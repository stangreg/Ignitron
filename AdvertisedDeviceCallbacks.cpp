#include "AdvertisedDeviceCallbacks.h"

	void AdvertisedDeviceCallbacks::setBLEControl(SparkBLEControl* ble) {
		bleControl = ble;
	}

	void AdvertisedDeviceCallbacks::onResult(NimBLEAdvertisedDevice *advertisedDevice) {
		//Serial.print("Advertised Device found: ");
		//Serial.println(advertisedDevice->toString().c_str());
		if (advertisedDevice->isAdvertisingService(NimBLEUUID("ffc0"))) {
			Serial.println("Found Spark");
			/** stop scan before connecting */
			NimBLEDevice::getScan()->stop();
			/** Save the device reference in a global for the client to use*/
			bleControl->setAdvertisedDevice(advertisedDevice);
			/** Ready to connect now */
			bleControl->setConnectionFound(true);
		}
	}
