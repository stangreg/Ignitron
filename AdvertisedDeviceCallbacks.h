/*
 * AdvertisedDeviceCallbacks.h
 *
 *  Created on: 19.08.2021
 *      Author: steffen
 */

#ifndef ADVERTISEDDEVICECALLBACKS_H_
#define ADVERTISEDDEVICECALLBACKS_H_

#include <NimBLEDevice.h>
#include "SparkBLEControl.h"

class SparkBLEControl;

class AdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {

private:
	SparkBLEControl* bleControl;

public:
	void setBLEControl(SparkBLEControl* ble);
	void onResult(NimBLEAdvertisedDevice *advertisedDevice);

};


#endif /* ADVERTISEDDEVICECALLBACKS_H_ */
