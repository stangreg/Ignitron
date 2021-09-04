/*
 * SparkLEDControl.h
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#ifndef SPARKLEDCONTROL_H_
#define SPARKLEDCONTROL_H_

#include <Arduino.h>
#include "SparkDataControl.h"

// GPIOs
#define LED_PRESET1_GPIO 23
#define LED_DRIVE_GPIO 23

#define LED_PRESET2_GPIO 17
#define LED_MOD_GPIO 17

#define LED_PRESET3_GPIO 16
#define LED_DELAY_GPIO 16

#define LED_PRESET4_GPIO 14
#define LED_REVERB_GPIO 14

#define LED_BANK_DOWN_GPIO 27
#define LED_NOISEGATE_GPIO 27

#define LED_BANK_UP_GPIO 13
#define LED_COMP_GPIO 13

class SparkLEDControl {
public:
	SparkLEDControl();
	SparkLEDControl(SparkDataControl* dc);
	virtual ~SparkLEDControl();

	void updateLEDs();
	SparkDataControl* dataControl() {return spark_dc;}

private:

	void init();
	SparkDataControl* spark_dc;

	int delayTimeMs = 100;
	// For blinking mode
	int ledState = LOW;             // ledState used to set the LED
	// Generally, you should use "unsigned long" for variables that hold time
	// The value will quickly become too large for an int to store
	unsigned long previousMillis = 0;        // will store last time LED was updated
	// constants won't change:
	const long interval = 100;           // interval at which to blink (milliseconds)


};

#endif /* SPARKLEDCONTROL_H_ */
