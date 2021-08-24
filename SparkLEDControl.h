/*
 * SparkLEDControl.h
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#ifndef SPARKLEDCONTROL_H_
#define SPARKLEDCONTROL_H_

#include "SparkDataControl.h"

// GPIOs
#define LED_PRESET1_GPIO 13
#define LED_DRIVE_GPIO 13

#define LED_PRESET2_GPIO 12
#define LED_MOD_GPIO 12

#define LED_PRESET3_GPIO 27
#define LED_DELAY_GPIO 27

#define LED_PRESET4_GPIO 16
#define LED_REVERB_GPIO 16

#define LED_BANK_DOWN_GPIO 17
#define LED_NOISEGATE_GPIO 17

#define LED_BANK_UP_GPIO 14
#define LED_COMP_GPIO 14

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
};

#endif /* SPARKLEDCONTROL_H_ */
