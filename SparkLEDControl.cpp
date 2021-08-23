/*
 * SparkLEDControl.cpp
 *
 *  Created on: 23.08.2021
 *      Author: steffen
 */

#include "SparkLEDControl.h"

SparkLEDControl::SparkLEDControl() {
	spark_dc = null;

}

SparkLEDControl::SparkLEDControl(SparkDataControl* dc) {
	spark_dc = dc;
	init();

}

SparkLEDControl::~SparkLEDControl() {
	// TODO Auto-generated destructor stub
}

void SparkLEDControl::init(){
	pinMode(LED_CHANNEL1_GPIO, OUTPUT);
	pinMode(LED_CHANNEL2_GPIO, OUTPUT);
	pinMode(LED_CHANNEL3_GPIO, OUTPUT);
	pinMode(LED_CHANNEL4_GPIO, OUTPUT);
	pinMode(LED_BANK_DOWN_GPIO, OUTPUT);
	pinMode(LED_BANK_UP_GPIO, OUTPUT);

}

void SparkLEDControl::updateLEDs() {

	preset* activePreset = spark_dc->activePreset();
	int activePresetNum = spark_dc->activePresetNum();
	int buttonMode = spark_dc->buttonMode();

	if (buttonMode == SWITCH_MODE_CHANNEL) {
		switch (activePresetNum) {
		case 1:
			digitalWrite(LED_CHANNEL1_GPIO, HIGH);
			digitalWrite(LED_CHANNEL2_GPIO, LOW);
			digitalWrite(LED_CHANNEL3_GPIO, LOW);
			digitalWrite(LED_CHANNEL4_GPIO, LOW);
			digitalWrite(LED_BANK_DOWN_GPIO, LOW);
			digitalWrite(LED_BANK_UP_GPIO, LOW);
			break;
		case 2:
			digitalWrite(LED_CHANNEL1_GPIO, LOW);
			digitalWrite(LED_CHANNEL2_GPIO, HIGH);
			digitalWrite(LED_CHANNEL3_GPIO, LOW);
			digitalWrite(LED_CHANNEL4_GPIO, LOW);
			digitalWrite(LED_BANK_DOWN_GPIO, LOW);
			digitalWrite(LED_BANK_UP_GPIO, LOW);
			break;
		case 3:
			digitalWrite(LED_CHANNEL1_GPIO, LOW);
			digitalWrite(LED_CHANNEL2_GPIO, LOW);
			digitalWrite(LED_CHANNEL3_GPIO, HIGH);
			digitalWrite(LED_CHANNEL4_GPIO, LOW);
			digitalWrite(LED_BANK_DOWN_GPIO, LOW);
			digitalWrite(LED_BANK_UP_GPIO, LOW);
			break;
		case 4:
			digitalWrite(LED_CHANNEL1_GPIO, LOW);
			digitalWrite(LED_CHANNEL2_GPIO, LOW);
			digitalWrite(LED_CHANNEL3_GPIO, LOW);
			digitalWrite(LED_CHANNEL4_GPIO, HIGH);
			digitalWrite(LED_BANK_DOWN_GPIO, LOW);
			digitalWrite(LED_BANK_UP_GPIO, LOW);
			break;
		}
	} else if (buttonMode == SWITCH_MODE_FX) {
		if (!activePreset->isEmpty) {

			pedal fx_noisegate = activePreset->pedals[FX_NOISEGATE];
			if (fx_noisegate.isOn) {
				digitalWrite(LED_NOISEGATE_GPIO, HIGH);
			} else {
				digitalWrite(LED_NOISEGATE_GPIO, LOW);
			}

			pedal fx_comp = activePreset->pedals[FX_COMP];
			if (fx_comp.isOn) {
				digitalWrite(LED_COMP_GPIO, HIGH);
			} else {
				digitalWrite(LED_COMP_GPIO, LOW);
			}

			pedal fx_dist = activePreset->pedals[FX_DRIVE];
			if (fx_dist.isOn) {
				digitalWrite(LED_DRIVE_GPIO, HIGH);
			} else {
				digitalWrite(LED_DRIVE_GPIO, LOW);
			}

			pedal fx_mod = activePreset->pedals[FX_MOD];
			if (fx_mod.isOn) {
				digitalWrite(LED_MOD_GPIO, HIGH);
			} else {
				digitalWrite(LED_MOD_GPIO, LOW);
			}

			pedal fx_delay = activePreset->pedals[FX_DELAY];
			if (fx_delay.isOn) {
				digitalWrite(LED_DELAY_GPIO, HIGH);
			} else {
				digitalWrite(LED_DELAY_GPIO, LOW);
			}

			pedal fx_reverb = activePreset->pedals[FX_REVERB];
			if (fx_reverb.isOn) {
				digitalWrite(LED_REVERB_GPIO, HIGH);
			} else {
				digitalWrite(LED_REVERB_GPIO, LOW);
			}
		}
	}

}
