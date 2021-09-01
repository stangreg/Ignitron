/*
 * SparkBLEControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkBLEControl.h"

//ClientCallbacks SparkBLEControl::clientCB;

SparkBLEControl::SparkBLEControl() {
	//advDevCB = new AdvertisedDeviceCallbacks();
	advDevice = new NimBLEAdvertisedDevice();
	spark_dc = nullptr;

}

SparkBLEControl::SparkBLEControl(SparkDataControl *dc) {
	//advDevCB = new AdvertisedDeviceCallbacks();
	advDevice = new NimBLEAdvertisedDevice();
	spark_dc = dc;

}

SparkBLEControl::~SparkBLEControl() {
	//if(advDevCB) delete advDevCB;
	if (advDevice)
		delete advDevice;
}

// Initializing BLE connection with NimBLE
void SparkBLEControl::initBLE() {
	NimBLEDevice::init("");

	/** Optional: set the transmit power, default is 3db */
	NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */

	/** create new scan */
	NimBLEScan *pScan = NimBLEDevice::getScan();

	/** create a callback that gets called when advertisers are found */
	pScan->setAdvertisedDeviceCallbacks(this, false);

	/** Set scan interval (how often) and window (how long) in milliseconds */
	pScan->setInterval(45);
	pScan->setWindow(15);

	/** Active scan will gather scan response data from advertisers
	 but will use more energy from both devices
	 */
	pScan->setActiveScan(true);
	/** Start scanning for advertisers for the scan time specified (in seconds) 0 = forever
	 Optional callback for when scanning stops.
	 */
	Serial.println("Starting scan");
	pScan->start(scanTime, scanEndedCB);
}

void SparkBLEControl::setAdvertisedDevice(NimBLEAdvertisedDevice *device) {
	advDevice = device;
}

void SparkBLEControl::scanEndedCB(NimBLEScanResults results) {
	Serial.println("Scan ended.");
}

void SparkBLEControl::initScan() {
	NimBLEDevice::getScan()->start(scanTime, scanEndedCB);
	Serial.println("Scan initiated");
}

bool SparkBLEControl::connectToServer() {

	/** Check if we have a client we should reuse first **/
	if (NimBLEDevice::getClientListSize()) {
		/** Special case when we already know this device, we send false as the
		 second argument in connect() to prevent refreshing the service database.
		 This saves considerable time and power.
		 */
		pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
		if (pClient) {
			if (!pClient->connect(advDevice, false)) {
				Serial.println("Reconnect failed");
				isAmpConnected_ = false;
				return false;
			}
			Serial.println("Reconnected client");
		}
		/** We don't already have a client that knows this device,
		 we will check for a client that is disconnected that we can use.
		 */
		else {
			pClient = NimBLEDevice::getDisconnectedClient();
		}
	}

	/** No client to reuse? Create a new one. */
	if (!pClient) {
		if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
			Serial.println(
					"Max clients reached - no more connections available");
			isAmpConnected_ = false;
			return false;
		}

		pClient = NimBLEDevice::createClient();

		Serial.println("New client created");

		pClient->setClientCallbacks(this, false);
		/** Set initial connection parameters: These settings are 15ms interval, 0 latency, 120ms timout.
		 These settings are safe for 3 clients to connect reliably, can go faster if you have less
		 connections. Timeout should be a multiple of the interval, minimum is 100ms.
		 Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 51 * 10ms = 510ms timeout
		 */
		pClient->setConnectionParams(12, 12, 0, 51);
		/** Set how long we are willing to wait for the connection to complete (seconds), default is 30. */
		pClient->setConnectTimeout(5);

		if (!pClient->connect(advDevice)) {
			/** Created a client but failed to connect, don't need to keep it as it has no data */
			NimBLEDevice::deleteClient(pClient);
			Serial.println("Failed to connect, deleted client");
			isAmpConnected_ = false;
			return false;
		}
	}

	if (!pClient->isConnected()) {
		if (!pClient->connect(advDevice)) {
			Serial.println("Failed to connect");
			isAmpConnected_ = false;
			return false;
		}
	}

	Serial.print("Connected to: ");
	Serial.println(pClient->getPeerAddress().toString().c_str());
	isAmpConnected_ = true;
	return true;
}

bool SparkBLEControl::subscribeToNotifications(notify_callback notifyCallback) {

	// Subscribe to notifications from Spark
	NimBLERemoteService *pSvc = nullptr;
	NimBLERemoteCharacteristic *pChr = nullptr;
	NimBLERemoteDescriptor *pDsc = nullptr;

	if (pClient) {
		pSvc = pClient->getService(SPARK_BLE_SERVICE_UUID);
		if (pSvc) { // make sure it's not null
			pChr = pSvc->getCharacteristic(SPARK_BLE_NOTIF_CHAR_UUID);

			if (pChr) { // make sure it's not null
				if (pChr->canNotify()) {
					Serial.printf(
							"Subscribing to service notifications of %s\n",
							SPARK_BLE_NOTIF_CHAR_UUID);
					Serial.println("Notifications turned on");
					// Descriptor 2902 needs to be activated in order to receive notifications
					pChr->getDescriptor(BLEUUID((uint16_t) 0x2902))->writeValue(
							(uint8_t*) notificationOn, 2, true);
					// Subscribing to Spark characteristic
					if (!pChr->subscribe(true, notifyCallback)) {
						Serial.println("Subscribe failed, disconnecting");
						// Disconnect if subscribe failed
						pClient->disconnect();
						return false;
					}
				}

			} else {
				Serial.printf("%s characteristic not found.\n",
				SPARK_BLE_NOTIF_CHAR_UUID);
				return false;
			}

			Serial.println("Done with this device.");
			return true;
		} // pSrv
		else {
			Serial.printf("Service %s not found.\n", SPARK_BLE_SERVICE_UUID);
			return false;
		}
	} // pClient
	else {
		Serial.print("Client not found! Need reconnection");
		isAmpConnected_ = false;
		return false;
	}
}

// To send messages to Spark via Bluetooth LE
bool SparkBLEControl::writeBLE(std::vector<ByteVector> cmd, bool response) {

	if (pClient && pClient->isConnected()) {

		NimBLERemoteService *pSvc = nullptr;
		NimBLERemoteCharacteristic *pChr = nullptr;

		pSvc = pClient->getService(SPARK_BLE_SERVICE_UUID);
		if (pSvc) {
			pChr = pSvc->getCharacteristic(SPARK_BLE_WRITE_CHAR_UUID);

			if (pChr) {

				if (pChr->canWrite()) {

					std::vector<ByteVector> packets;
					// Spark messages are sent in chunks of 173 (0xAD) bytes, so we do the same.
					int max_send_size = 173;
					int curr_pos;

					for (auto block : cmd) {

						// This it to split messages into sizes of max. max_send_size.
						// As we have chosen 173, usually no further splitting is requried.
						// SparkMessage already creates messages split into 173 byte chunks
						curr_pos = 0;

						int packetsToSend = (int) ceil(
								(double) block.size() / max_send_size);
						ByteVector packet;
						packet.clear();

						auto start = block.begin();
						auto end = block.end();

						// Splitting the message
						while (start != end) {
							auto next =
									std::distance(start, end) >= max_send_size ?
											start + max_send_size : end;

							packet.assign(start, next);
							packets.push_back(packet);
							start = next;
						} // While not at
					}

					// Send each packet to Spark
					for (auto packet : packets) {
						//Serial.println("Trying to send package");
						if (pChr->writeValue(packet.data(), packet.size(),
								response)) {
							// Delay seems to be required in order to not lose any packages.
							// Seems to be more stable with a short delay
							delay(10);
						} else {
							Serial.println("There was an error with writing!");
							// Disconnect if write failed
							pClient->disconnect();
							isAmpConnected_ = false;
							return false;
						}
					} //For each packet

				}  // if can write
			} // if pChr
			else {
				Serial.printf("Characteristic %s not found.\n",
				SPARK_BLE_WRITE_CHAR_UUID);
			}
		} // if pSvc
		else {
			Serial.printf("%s service not found.\n", SPARK_BLE_SERVICE_UUID);
		}
		//Serial.println("Done with this command!");
		return true;
	} else {
		Serial.println("Client seems to be disconnected");
		isAmpConnected_ = false;
		return false;
	}
}

void SparkBLEControl::onResult(NimBLEAdvertisedDevice *advertisedDevice) {

	if (advertisedDevice->isAdvertisingService(
			NimBLEUUID(SPARK_BLE_SERVICE_UUID))) {
		Serial.println("Found Spark, connecting.");
		/** stop scan before connecting */
		NimBLEDevice::getScan()->stop();
		/** Save the device reference in a global for the client to use*/
		setAdvertisedDevice(advertisedDevice);
		/** Ready to connect now */
		isConnectionFound_ = true;
	}
}

void SparkBLEControl::startServer() {
	Serial.println("Starting NimBLE Server");

	/** sets device name */
	NimBLEDevice::init("Spark 40 BLE");

	/** Optional: set the transmit power, default is 3db */
	NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
	NimBLEDevice::setSecurityAuth(
			/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/BLE_SM_PAIR_AUTHREQ_SC);

	pServer = NimBLEDevice::createServer();
	pServer->setCallbacks(this);

	NimBLEService *pSparkService = pServer->createService(
	SPARK_BLE_SERVICE_UUID);
	NimBLECharacteristic *pSparkWriteCharacteristic =
			pSparkService->createCharacteristic(
			SPARK_BLE_WRITE_CHAR_UUID,
					NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
							| NIMBLE_PROPERTY::WRITE_NR);

	uint8_t initialWriteValue[] = { 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
			0x77, 0x77, 0x77 };

	pSparkWriteCharacteristic->setValue(initialWriteValue);
	pSparkWriteCharacteristic->setCallbacks(this);

	NimBLECharacteristic *pSparkNotificationCharacteristic =
			pSparkService->createCharacteristic(
			SPARK_BLE_NOTIF_CHAR_UUID,
					NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
	uint8_t initialNotificationValue[] = { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
			0x88, 0x88, 0x88, 0x88 };
	pSparkNotificationCharacteristic->setValue(initialNotificationValue);
	pSparkNotificationCharacteristic->setCallbacks(this);

	/** Start the services when finished creating all Characteristics and Descriptors */
	pSparkService->start();
	//pSparkNotificationService->start();
	NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();

	//uint8_t Adv_DATA[] = {0xee, 0x03, 0x08, 0xEB, 0xED, 0x78, 0x0A, 0x6E};
	//BLEAdvertisementData oAdvertisementCustom = BLEAdvertisementData();
	//oAdvertisementCustom.setManufacturerData(std::string((char *)&Adv_DATA[0], 8)); // 8 is length of Adv_DATA
	//oAdvertisementCustom.setPartialServices(NimBLEUUID(SPARK_BLE_SERVICE_UUID));
	//oAdvertisementCustom.setFlags(0x06);
	//pAdvertising->setAdvertisementData(oAdvertisementCustom);
	pAdvertising->addServiceUUID(pSparkService->getUUID());

	/** Add the services to the advertisment data **/
	/** If your device is battery powered you may consider setting scan response
	 *  to false as it will extend battery life at the expense of less data sent.
	 */
	pAdvertising->setScanResponse(true);
	pAdvertising->start();

	Serial.println("Advertising Started");

}

void SparkBLEControl::onWrite(NimBLECharacteristic *pCharacteristic) {
	//Serial.print(pCharacteristic->getUUID().toString().c_str());
	//Serial.println(": onWrite()");
	std::string rxValue = pCharacteristic->getValue();
	ByteVector byteVector;
	byteVector.clear();

	if (rxValue.length() > 0) {
		for (int i = 0; i < rxValue.length(); i++) {
			byteVector.push_back((byte) (rxValue[i]));
		}
	}

	if (spark_dc->processSparkData(byteVector) == MSG_PROCESS_RES_INITIAL) {
		sendInitialNotification();
	}
}

void SparkBLEControl::onSubscribe(NimBLECharacteristic *pCharacteristic,
		ble_gap_conn_desc *desc, uint16_t subValue) {
	String str = "Client ID: ";
	str += desc->conn_handle;
	str += " Address: ";
	str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
	if (subValue == 0) {
		str += " Unsubscribed to ";
	} else if (subValue == 1) {
		str += " Subscribed to notfications for ";
	} else if (subValue == 2) {
		str += " Subscribed to indications for ";
	} else if (subValue == 3) {
		str += " Subscribed to notifications and indications for ";
	}
	str += std::string(pCharacteristic->getUUID()).c_str();

	Serial.println(str);
	sendInitialNotification();

}
;

void SparkBLEControl::notifyClients(ByteVector msg) {
	NimBLEService *pSvc = pServer->getServiceByUUID(SPARK_BLE_SERVICE_UUID);
	if (pSvc) {
		NimBLECharacteristic *pChr = pSvc->getCharacteristic(
		SPARK_BLE_NOTIF_CHAR_UUID);
		if (pChr) {
			pChr->setValue(&msg.data()[0], msg.size());
			pChr->notify(true);
		}
	}
}

void SparkBLEControl::sendInitialNotification() {
	//This is the serial number of the Spark. Sending fake one.
	ByteVector preface1 = { 0x01, 0xFE, 0x00, 0x00, 0x41, 0xFF, 0x29, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x01, 0x05,
			0x32, 0x03, 0x23, 0x02, 0x0D, 0x2D, 0x53, 0x39, 0x39, 0x39, 0x43,
			0x00, 0x39, 0x39, 0x39, 0x42, 0x39, 0x39, 0x39, 0x01, 0x77, 0xF7 };

	// When connecting app, we need to send a set of notifications for a successful connection
	ByteVector preface2 = { 0x01, 0xFE, 0x00, 0x00, 0x41, 0xFF, 0x1D, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x01, 0x01,
			0x77, 0x03, 0x2F, 0x11, 0x4E, 0x01, 0x04, 0x03, 0x2E, 0xF7 };
	ByteVector preface3 = { 0x01, 0xFE, 0x00, 0x00, 0x41, 0xFF, 0x1E, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x01, 0x02,
			0x2D, 0x03, 0x2A, 0x0D, 0x14, 0x7D, 0x4C, 0x07, 0x5A, 0x58, 0xF7 };

	int delayValue = 0;
	if (pServer->getConnectedCount()) {
		//Serial.println("Sending notifications...");
		notificationCount++;
		switch (notificationCount) {
		default:
			notificationCount = 1;
			/* no break */
		case 1:
			notifyClients(preface1);
			delay(delayValue);
			break;
		case 2:
			notifyClients(preface2);
			delay(delayValue);
			break;
		case 3:
			notifyClients(preface3);
			delay(delayValue);
			break;

		}

	} // if server connected
}

void SparkBLEControl::onConnect(NimBLEServer *pServer) {
	Serial.println("Client connected");
	isAppConnected_ = true;
	Serial.println("Multi-connect support: start advertising");
	NimBLEDevice::startAdvertising();
}
;

void SparkBLEControl::onDisconnect(NimBLEServer *pServer) {
	Serial.println("Client disconnected");
	isAppConnected_ = false;
	notificationCount = 0;
	Serial.println("Start advertising");
	NimBLEDevice::startAdvertising();
}
;

void SparkBLEControl::onDisconnect(NimBLEClient *pClient) {
	isAmpConnected_ = false;
	isConnectionFound_ = false;
	NimBLEClientCallbacks::onDisconnect(pClient);
}
/*
 bool SparkBLEControl::isAmpConnected() {
 bool retValue = lastHeartBeat;
 if (!isAmpConnected_) {
 lastHeartBeat = false;
 return false;
 }
 unsigned long currentTime = millis();
 if (currentTime - lastHeartBeatTime >= heartBeatInterval) {
 if (pClient && pClient->isConnected()) {
 NimBLERemoteService *pSvc = nullptr;
 NimBLERemoteCharacteristic *pChr = nullptr;
 pSvc = pClient->getService(SPARK_BLE_SERVICE_UUID);
 if (pSvc) {
 pChr = pSvc->getCharacteristic(SPARK_BLE_WRITE_CHAR_UUID);
 if (pChr) {
 Serial.print("Checking heartBeat");
 const unsigned char heartBeat[] = "\xFF";
 if (pChr->writeValue(heartBeat, 1, false)) {
 retValue = true;
 } else {
 retValue = false;
 }
 }
 }
 }
 lastHeartBeatTime = currentTime;
 lastHeartBeat = retValue;
 }
 return retValue;

 }
 */
