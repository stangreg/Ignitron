/*
 * SparkBLEControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: steffen
 */

#include "SparkBLEControl.h"


ClientCallbacks SparkBLEControl::clientCB;

SparkBLEControl::SparkBLEControl() {
	advDevCB = new AdvertisedDeviceCallbacks();
	advDevice = new NimBLEAdvertisedDevice();

}

SparkBLEControl::~SparkBLEControl() {
	if(advDevCB) delete advDevCB;
	if(advDevice) delete advDevice;
}

void SparkBLEControl::initBLE() {
	NimBLEDevice::init("");
	//NimBLEDevice::setMTU(175);
	/** Optional: set the transmit power, default is 3db */
	NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */

	/** create new scan */
	NimBLEScan *pScan = NimBLEDevice::getScan();

	/** create a callback that gets called when advertisers are found */
	advDevCB->setBLEControl(this);
	pScan->setAdvertisedDeviceCallbacks(advDevCB,false);

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

void SparkBLEControl::setConnectionFound(bool _connect){
	connection_found = _connect;
}

void SparkBLEControl::initScan(){
	NimBLEDevice::getScan()->start(scanTime, scanEndedCB);
	Serial.println("Scan initiated");
}

bool SparkBLEControl::connectionFound(){ return connection_found; }

bool SparkBLEControl::isConnected(){ return isClientConnected;}

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
				isClientConnected = false;
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
			isClientConnected = false;
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
			isClientConnected = false;
			return false;
		}
	}

	if (!pClient->isConnected()) {
		if (!pClient->connect(advDevice)) {
			Serial.println("Failed to connect");
			isClientConnected = false;
			return false;
		}
	}

	Serial.print("Connected to: ");
	Serial.println(pClient->getPeerAddress().toString().c_str());
	//Serial.print("RSSI: ");
	//Serial.println(pClient->getRssi());
	isClientConnected = true;
	return true;
}

bool SparkBLEControl::subscribeToNotifications(notify_callback notifyCallback) {
	/** Now we can read/write/subscribe the charateristics of the services we are interested in */
	NimBLERemoteService *pSvc = nullptr;
	NimBLERemoteCharacteristic *pChr = nullptr;
	NimBLERemoteDescriptor *pDsc = nullptr;

	if (pClient) {
		pSvc = pClient->getService("ffc0");
		if (pSvc) { /** make sure it's not null */
			pChr = pSvc->getCharacteristic("ffc2");

			if (pChr) { /** make sure it's not null */
				if (pChr->canNotify()) {
					Serial.println(
							"Subscribing to service notifications of ffc2");
					Serial.println("Notifications turned on");
					pChr->getDescriptor(BLEUUID((uint16_t) 0x2902))->writeValue(
							(uint8_t*) notificationOn, 2, true);
					if (!pChr->subscribe(true, notifyCallback)) {
						Serial.println("Subscribe failed, disconnecting");
						// Disconnect if subscribe failed
						pClient->disconnect();
						return false;
					}
				}

			} else {
				Serial.println("ffc2 characteristic not found.");
				return false;
			}

			Serial.println("Done with this device!");
			return true;
		} // pSrv
		else {
			Serial.println("Service not found");
			return false;
		}
	} // pClient
	else {
		Serial.print("Client not found! Need reconnection");
		isClientConnected = false;
		return false;
	}
}

/** Handles the provisioning of clients and connects / interfaces with the server */
bool SparkBLEControl::writeBLE(std::vector<ByteVector> cmd, boolean response) {
	NimBLERemoteService *pSvc = nullptr;
	NimBLERemoteCharacteristic *pChr = nullptr;
	NimBLERemoteDescriptor *pDsc = nullptr;

	//Serial.printf("cmd size: %d\n", cmd.size());
	//Serial.printf("MTU: %d\n", NimBLEDevice::getMTU());
	if (pClient && pClient->isConnected()) {

		/** Now we can read/write/subscribe the charateristics of the services we are interested in */
		NimBLERemoteService *pSvc = nullptr;
		NimBLERemoteCharacteristic *pChr = nullptr;
		NimBLERemoteDescriptor *pDsc = nullptr;

		pSvc = pClient->getService("ffc0");
		if (pSvc) { /** make sure it's not null */
			pChr = pSvc->getCharacteristic("ffc1");

			if (pChr) { /** make sure it's not null */

				if (pChr->canWrite()) {

					//Serial.printf("Sending request(size %d)\n", cmd.size());
					//SparkHelper::printDataAsHexString(cmd);
					//Serial.println();
					std::vector<ByteVector> packets;
					int max_send_size = 173;
					int curr_pos;

					for (auto block : cmd) {

						//Serial.printf("Processing block into sending packets...\n"); // %s\n", SparkHelper::hexStr(block.data(), block.size()).c_str());
						curr_pos = 0;

						int packetsToSend = (int) ceil(
								(double) block.size() / max_send_size);
						//Serial.printf("Packets to send: %d\n", packetsToSend);
						ByteVector packet;
						packet.clear();

						auto start = block.begin();
						auto end = block.end();

						while (start != end) {
							auto next =
									std::distance(start, end) >= max_send_size ?
											start + max_send_size : end;

							packet.assign(start, next);
							//Serial.printf("Packet size: %d\n", packet.size());

							packets.push_back(packet);
							start = next;
						} // While not at
					}

					//Serial.println("Packets to be sent:");
					//SparkHelper::printDataAsHexString(packets);
					//Serial.println();

					for (auto packet : packets) {
						if (pChr->writeValue(packet.data(), packet.size(),
								response)) {
							//Serial.println("Packet sent to Spark");
							delay(10);
							//Serial.println(pChr->getUUID().toString().c_str());
						} else {
							Serial.println("There was an error with writing!");
							/** Disconnect if write failed */
							pClient->disconnect();
							isClientConnected = false;
							return false;
						}
					} //For each packet

				}  // if can write
			} // if pChr
			else {
				Serial.println("Characteristic ffc1 not found");
			}
		} // if pSvc
		else {
			Serial.println("ffc0 service not found.");
		}
		//Serial.println("Done with this command!");
		return true;
	} else {
		Serial.println("Client seems to be disconnected");
		isClientConnected = false;
		return false;
	}
}
