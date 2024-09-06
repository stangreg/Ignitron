/*
 * SparkBLEKeyboard.h
 *
 *  Created on: 27.12.2021
 *      Author: steffen
 */

#ifndef SPARKBLEKEYBOARD_H_
#define SPARKBLEKEYBOARD_H_


#include <Arduino.h>
#include <BleKeyboard.h>
#include <NimBLEDevice.h>

using namespace std;

class SparkBLEKeyboard: public BleKeyboard {
public:
	SparkBLEKeyboard();
	virtual ~SparkBLEKeyboard();

	void start();
	void end();
};

#endif /* SPARKBLEKEYBOARD_H_ */
