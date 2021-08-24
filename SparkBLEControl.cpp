/*
 * SparkBLEControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkBLEControl.h"

ClientCallbacks SparkBLEControl::clientCB;

SparkBLEControl::SparkBLEControl() {
	//advDevCB = new AdvertisedDeviceCallbacks();
	advDevice = new NimBLEAdvertisedDevice();

}

SparkBLEControl::~SparkBLEControl() {
	//if(advDevCB) delete advDevCB;
	if(advDevice) delete advDevice;
}

// Initializing BLE connection with NimBLE
void SparkBLEControl::initBLE() {
	NimBLEDevice::init("");

	/** Optional: set the transmit power, default is 3db */
	NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */

	/** create new scan */
	NimBLEScan *pScan = NimBLEDevice::getScan();

	/** create a callback that gets called when advertisers are found */
	//advDevCB->setBLEControl(this);
	pScan->setAdvertisedDeviceCallbacks(this,false);

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

void SparkBLEControl::setAdvertisedDevice(NimBLEAdvertisedDevice *device){
	advDevice  = device;
}

void SparkBLEControl::scanEndedCB(NimBLEScanResults results){
	Serial.println("Scan ended.");
}

void SparkBLEControl::initScan(){
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
				isConnected_ = false;
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
			isConnected_ = false;
			return false;
		}

		pClient = NimBLEDevice::createClient();

		Serial.println("New client created");

		pClient->setClientCallbacks(&clientCB, false);
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
			isConnected_ = false;
			return false;
		}
	}

	if (!pClient->isConnected()) {
		if (!pClient->connect(advDevice)) {
			Serial.println("Failed to connect");
			isConnected_ = false;
			return false;
		}
	}

	Serial.print("Connected to: ");
	Serial.println(pClient->getPeerAddress().toString().c_str());
	isConnected_ = true;
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
							"Subscribing to service notifications of %s\n", SPARK_BLE_NOTIF_CHAR_UUID);
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
				Serial.printf("%s characteristic not found.\n", SPARK_BLE_NOTIF_CHAR_UUID);
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
		isConnected_ = false;
		return false;
	}
}

// To send messages to Spark via Bluetooth LE
bool SparkBLEControl::writeBLE(std::vector<ByteVector> cmd, boolean response) {

	if (pClient && pClient->isConnected()) {

		NimBLERemoteService *pSvc = nullptr;
		NimBLERemoteCharacteristic *pChr = nullptr;
		NimBLERemoteDescriptor *pDsc = nullptr;

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
						if (pChr->writeValue(packet.data(), packet.size(),
								response)) {
							// Delay seems to be required in order to not lose any packages.
							// Seems to be more stable with a short delay
							delay(10);
						}
						else {
							Serial.println("There was an error with writing!");
							// Disconnect if write failed
							pClient->disconnect();
							isConnected_ = false;
							return false;
						}
					} //For each packet

				}  // if can write
			} // if pChr
			else {
				Serial.printf("Characteristic %s not found.\n", SPARK_BLE_WRITE_CHAR_UUID);
			}
		} // if pSvc
		else {
			Serial.printf("%s service not found.\n", SPARK_BLE_SERVICE_UUID);
		}
		//Serial.println("Done with this command!");
		return true;
	} else {
		Serial.println("Client seems to be disconnected");
		isConnected_ = false;
		return false;
	}
}

void SparkBLEControl::onResult(NimBLEAdvertisedDevice *advertisedDevice) {

	if (advertisedDevice->isAdvertisingService(NimBLEUUID(SPARK_BLE_SERVICE_UUID))) {
		Serial.println("Found Spark, connecting.");
		/** stop scan before connecting */
		NimBLEDevice::getScan()->stop();
		/** Save the device reference in a global for the client to use*/
		setAdvertisedDevice(advertisedDevice);
		/** Ready to connect now */
		isConnectionFound_ = true;
	}
}
