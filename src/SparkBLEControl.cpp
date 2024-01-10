/*
 * SparkBLEControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkBLEControl.h"

bool SparkBLEControl::isAppConnectedSerial_ = false;

//ClientCallbacks SparkBLEControl::clientCB;

SparkBLEControl::SparkBLEControl() {
	//advDevCB = new AdvertisedDeviceCallbacks();
	advDevice = nullptr;
	spark_dc = nullptr;

}

SparkBLEControl::SparkBLEControl(SparkDataControl *dc) {
	//advDevCB = new AdvertisedDeviceCallbacks();
	advDevice = nullptr;
	spark_dc = dc;

}

SparkBLEControl::~SparkBLEControl() {
	Serial.println("Deleting BLE objects");
	if (advDevice) {
		delete advDevice;
		advDevice = nullptr;
	}
	if (btSerial) {
		delete btSerial;
		btSerial = nullptr;
	}
	
}

// Initializing BLE connection with NimBLE
void SparkBLEControl::initBLE(notify_callback notifyCallback) {
	//NimBLEDevice::init("");
	advDevice = new NimBLEAdvertisedDevice();
	notifyCB = notifyCallback;

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

void SparkBLEControl::startScan() {
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

		pClient->setClientCallbacks(this, false);
		/** Set initial connection parameters: These settings are 15ms interval, 0 latency, 120ms timout.
		 These settings are safe for 3 clients to connect reliably, can go faster if you have less
		 connections. Timeout should be a multiple of the interval, minimum is 100ms.
		 Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 51 * 10ms = 510ms timeout
		 */
		pClient->setConnectionParams(12, 12, 0, 51);
		//pClient->setConnectionParams(40, 80, 5, 51);
		/** Set how long we are willing to wait for the connection to complete (seconds), default is 30. */
		pClient->setConnectTimeout(10);
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
					if (!pChr->subscribe(true, notifyCB)) {
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
					for (auto block : cmd) {

						// This it to split messages into sizes of max. max_send_size.
						// As we have chosen 173, usually no further splitting is requried.
						// SparkMessage already creates messages split into 173 byte chunks
						DEBUG_PRINTLN("Sending block:");
						DEBUG_PRINTVECTOR(block);
						DEBUG_PRINTLN();
						if (pChr->writeValue(block.data(), block.size(),
								response)) {
							// Delay seems to be required in order to not lose any packages.
							// Seems to be more stable with a short delay
							// also seems to be not working for Spark Mini without a delay.s
							delay(50);
						} else {
							Serial.println("There was an error with writing!");
							// Disconnect if write failed
							pClient->disconnect();
							isAmpConnected_ = false;
							return false;
						}
					} //For each block
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
		DEBUG_PRINTLN("Done with this command!");
		return true;
	} else {
		isAmpConnected_ = false;
		return false;
	}
}

void SparkBLEControl::onResult(NimBLEAdvertisedDevice *advertisedDevice) {

	if (advertisedDevice->isAdvertisingService(
			NimBLEUUID(SPARK_BLE_SERVICE_UUID))) {
		Serial.println("Found Spark, connecting.");
		/** stop scan before connecting */
		//Commented as workaround, might need to get back here, is currently with DataControl;
		NimBLEDevice::getScan()->stop();
		/** Save the device reference in a global for the client to use*/
		setAdvertisedDevice(advertisedDevice);
		/** Ready to connect now */
		isConnectionFound_ = true;
		delay(500);
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

	pSparkService = pServer->createService(
	SPARK_BLE_SERVICE_UUID);
	pSparkWriteCharacteristic =
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

	pSparkNotificationCharacteristic =
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
	pAdvertising = NimBLEDevice::getAdvertising();

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
	DEBUG_PRINT(pCharacteristic->getUUID().toString().c_str());
	DEBUG_PRINTLN(": onWrite()");
	std::string rxValue = pCharacteristic->getValue();
	ByteVector byteVector;
	byteVector.clear();

	if (rxValue.length() > 0) {
		for (int i = 0; i < rxValue.length(); i++) {
			byteVector.push_back((byte) (rxValue[i]));
		}
	}
	spark_dc->processSparkData(byteVector);

}

void SparkBLEControl::onSubscribe(NimBLECharacteristic *pCharacteristic,
		ble_gap_conn_desc *desc, uint16_t subValue) {
	std::string str = "Address: ";
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
	str += std::string(pCharacteristic->getUUID());

	Serial.println(str.c_str());
}
;

void SparkBLEControl::notifyClients(std::vector<ByteVector> msg) {
	if (pServer) {
		NimBLEService *pSvc = pServer->getServiceByUUID(SPARK_BLE_SERVICE_UUID);
		if (pSvc) {
			NimBLECharacteristic *pChr = pSvc->getCharacteristic(
			SPARK_BLE_NOTIF_CHAR_UUID);
			if (pChr) {
				for (auto block : msg) {
					DEBUG_PRINTLN("Sending data:");
					DEBUG_PRINTVECTOR(block);
					DEBUG_PRINTLN();
					pChr->setValue(&block.data()[0], block.size());
					pChr->notify();
				}
				DEBUG_PRINTLN("Clients notified.");

			}
		}
	}

	if (btSerial && btSerial->hasClient()) {
		DEBUG_PRINTLN("Sending message via BT Serial:");
		for (auto chunk : msg) {
			for (auto by : chunk) {
				if (by < 16) {
					DEBUG_PRINT("0");
				} DEBUG_PRINT(by, HEX
				);
				btSerial->write(by);
			}
			DEBUG_PRINTLN();
			DEBUG_PRINTF("Free Heap size: %d\n", ESP.getFreeHeap());
		}
	}
}

void SparkBLEControl::onConnect(NimBLEServer *pServer_,
		ble_gap_conn_desc *desc) {
	isAppConnectedBLE_ = true;
	Serial.println("Multi-connect support: start advertising");
	//	pServer->updateConnParams(desc->conn_handle, 40, 80, 5, 51);
	NimBLEDevice::startAdvertising();
}

void SparkBLEControl::onDisconnect(NimBLEServer *pServer_) {
	Serial.println("Client disconnected");
	isAppConnectedBLE_ = false;
	notificationCount = 0;
	Serial.println("Start advertising");
	NimBLEDevice::startAdvertising();
}
;

void SparkBLEControl::onDisconnect(NimBLEClient *pClient_) {
	isAmpConnected_ = false;
	isConnectionFound_ = false;
	if (!(NimBLEDevice::getScan()->isScanning())) {
		startScan();
	}
	NimBLEClientCallbacks::onDisconnect(pClient_);
}

void SparkBLEControl::stopScan() {
	if (isScanning()) {
		Serial.println("Scan stopped");
		NimBLEDevice::getScan()->stop();
	} else {
		Serial.print("Scan is not running");
	}

}

void SparkBLEControl::serialCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial.println("Client Connected");
    isAppConnectedSerial_ = true;
  }
  if(event == ESP_SPP_CLOSE_EVT ){
      Serial.println("Client disconnected");
      isAppConnectedSerial_ = false;
    }
}

void SparkBLEControl::startBTSerial() {
	btSerial = new BluetoothSerial();
	btSerial->register_callback(serialCallback);
	//btStart();
	if (btSerial->begin(bt_name_serial.c_str(), false)) {
		Serial.printf("Started BT Serial with name %s \n",
				bt_name_serial.c_str());
		btSerial->flush();
	} else {
		Serial.println("BT Serial start failed!");
	}
}

void SparkBLEControl::stopBTSerial() {
	// Stop Bluetooth Serial
	btSerial->end();
	delete btSerial;
	btSerial = nullptr;
	Serial.println("BT Serial stopped");
}


void SparkBLEControl::stopBLEServer() {

	if (pServer) {
		Serial.println("Switching off BLE server");
		pServer->stopAdvertising();
	}

}

