/*
 * SparkBLEKeyboard.cpp
 *
 *  Created on: 27.12.2021
 *      Author: steffen
 */

#include "SparkBLEKeyboard.h"

SparkBLEKeyboard::SparkBLEKeyboard() {
	// TODO Auto-generated constructor stub

}

SparkBLEKeyboard::~SparkBLEKeyboard() {
	// TODO Auto-generated destructor stub
}

void SparkBLEKeyboard::end() {
	BLEServer *pServer = BLEDevice::getServer();
	for (int i = 0; i < pServer->getConnectedCount(); i++) {
		pServer->disconnect(pServer->getPeerInfo(i).getConnHandle());
	}
	pServer->stopAdvertising();
}

void SparkBLEKeyboard::start() {
	BLEServer *pServer = BLEDevice::getServer();
	pServer->startAdvertising();

}
