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

#include "SparkDataControl.h"

// Service and characteristics UUIDs of Spark Amp
#define SPARK_BLE_SERVICE_UUID "FFC0"
#define SPARK_BLE_WRITE_CHAR_UUID "FFC1"
#define SPARK_BLE_NOTIF_CHAR_UUID "FFC2"

using ByteVector = std::vector<byte>;
class SparkDataControl;

// Forward declaration of Callbacks classes, does nothing special, only default actions
//class ClientCallbacks: public NimBLEClientCallbacks {};

class SparkBLEControl: public NimBLEAdvertisedDeviceCallbacks,
		NimBLECharacteristicCallbacks,
		NimBLEServerCallbacks,
		NimBLEClientCallbacks {
public:
	SparkBLEControl();
	SparkBLEControl(SparkDataControl *dc);
	virtual ~SparkBLEControl();
	const bool isConnectionFound() const {
		return isConnectionFound_;
	}
	const bool isAmpConnected() const {
		return isAmpConnected_;
	}
	const bool isAppConnected() const {
		return isAppConnected_;
	}
	bool connectToServer();
	bool subscribeToNotifications(notify_callback notifyCallback = nullptr);
	bool writeBLE(std::vector<ByteVector> cmd, bool response = false);
	void initBLE(notify_callback notifyCallback = nullptr);
	void initScan();
	const bool isScanning() const {
		return NimBLEDevice::getScan()->isScanning();
	}
	void startScan();
	void stopScan();

	void startServer();
	void notifyClients(ByteVector msg);
	void sendInitialNotification();

private:

	/** Create a single global instance of the callback class to be used by all clients */
	//static ClientCallbacks clientCB;
	NimBLEAdvertisedDevice *advDevice;
	NimBLEClient *pClient = nullptr;

	bool isAmpConnected_ = false;
	bool isConnectionFound_ = false;
	// isClientConnected will be set when a client is connected to ESP in AMP mode
	bool isAppConnected_ = false;
	notify_callback notifyCB;

	uint32_t scanTime = 0; /** 0 = scan forever */
	const uint8_t notificationOn[2] = { 0x1, 0x0 };

	static void scanEndedCB(NimBLEScanResults results);
	void onResult(NimBLEAdvertisedDevice *advertisedDevice);
	void setAdvertisedDevice(NimBLEAdvertisedDevice *device);
	void onDisconnect(NimBLEClient *pClient);

	// Server mode functions
	NimBLEServer *pServer = nullptr;
	SparkDataControl *spark_dc;
	void onWrite(NimBLECharacteristic *pCharacteristic);
	void onSubscribe(NimBLECharacteristic *pCharacteristic,
			ble_gap_conn_desc *desc, uint16_t subValue);
	void onConnect(NimBLEServer *pServer);
	void onDisconnect(NimBLEServer *pServer);

	int notificationCount = 0;
};

#endif /* SPARKBLECONTROL_H_ */
