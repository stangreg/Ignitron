/*
 * SparkBLEControl.h
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARKBLECONTROL_H_
#define SPARKBLECONTROL_H_

#include <NimBLEDevice.h>
#include <Arduino.h>
#include <vector>

// Service and characteristics UUIDs of Spark Amp
#define SPARK_BLE_SERVICE_UUID "FFC0"
#define SPARK_BLE_WRITE_CHAR_UUID "FFC1"
#define SPARK_BLE_NOTIF_CHAR_UUID "FFC2"

using ByteVector = std::vector<byte>;

// Forward declaration of Callbacks classes, does nothing special, only default actions
class ClientCallbacks: public NimBLEClientCallbacks {};

class SparkBLEControl : public NimBLEAdvertisedDeviceCallbacks {
public:
	SparkBLEControl();
	virtual ~SparkBLEControl();
	const bool isConnectionFound() const { return isConnectionFound_;}
	const bool isConnected() const { return isConnected_;}
	bool connectToServer();
	bool subscribeToNotifications(notify_callback notifyCallback=nullptr);
	bool writeBLE(std::vector<ByteVector> cmd, bool response = false);
	void initBLE();
	void initScan();


private:

	/** Create a single global instance of the callback class to be used by all clients */
	static ClientCallbacks clientCB;
	NimBLEAdvertisedDevice *advDevice;
	NimBLEClient *pClient = nullptr;

	bool isConnected_ = false;
	bool isConnectionFound_ = false;
	uint32_t scanTime = 0; /** 0 = scan forever */
	const uint8_t notificationOn[2] = { 0x1, 0x0 };

	static void scanEndedCB(NimBLEScanResults results);
	void onResult(NimBLEAdvertisedDevice *advertisedDevice);
	void setAdvertisedDevice(NimBLEAdvertisedDevice *device);


};

#endif /* SPARKBLECONTROL_H_ */
