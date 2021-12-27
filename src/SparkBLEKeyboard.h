/*
 * SparkBLEKeyboard.h
 *
 *  Created on: 27.12.2021
 *      Author: steffen
 */

#ifndef SPARKBLEKEYBOARD_H_
#define SPARKBLEKEYBOARD_H_

#define USE_NIMBLE

#include <BleKeyboard.h>
#include <NimBLEDevice.h>

class SparkBLEKeyboard: public BleKeyboard {
public:
	SparkBLEKeyboard();
	virtual ~SparkBLEKeyboard();

	void start();
	void end();
};

#endif /* SPARKBLEKEYBOARD_H_ */
