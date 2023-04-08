/*
 * SparkLEDControl.cpp
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#include "SparkLEDControl.h"

SparkLEDControl::SparkLEDControl() {
	spark_dc = nullptr;
	activePreset = nullptr;
	init();
}

SparkLEDControl::SparkLEDControl(SparkDataControl* dc) {
	spark_dc = dc;
	init();

}

SparkLEDControl::~SparkLEDControl() {
}

void SparkLEDControl::init(){

	pinMode(LED_PRESET1_GPIO, OUTPUT);
	pinMode(LED_PRESET2_GPIO, OUTPUT);
	pinMode(LED_PRESET3_GPIO, OUTPUT);
	pinMode(LED_PRESET4_GPIO, OUTPUT);
	pinMode(LED_BANK_DOWN_GPIO, OUTPUT);
	pinMode(LED_BANK_UP_GPIO, OUTPUT);

	allLedOff();

}

void SparkLEDControl::updateLEDs() {

	operationMode = spark_dc->operationMode();
	activePreset = spark_dc->activePreset();
	activePresetNum = spark_dc->activePresetNum();

	switch (operationMode) {

	case SPARK_MODE_LOOPER:
	case SPARK_MODE_APP: {
		int buttonMode = spark_dc->buttonMode();
		// Show only active preset LED
		if (buttonMode == SWITCH_MODE_PRESET) {
			updateLED_APP_PresetMode();
			// For each effect, show which effect is active
		} else if (buttonMode == SWITCH_MODE_FX) {
			updateLED_APP_FXMode();
		}
	}
	break;

	case SPARK_MODE_AMP:
		updateLED_AMP();
		break;
	case SPARK_MODE_KEYBOARD:
		updateLED_KEYBOARD();
		break;
	}
}

void SparkLEDControl::updateLED_APP_PresetMode() {
	allLedOff();
	switchLED(activePresetNum, true);
}

void SparkLEDControl::updateLED_APP_FXMode() {
	if (!activePreset->isEmpty) {
		for (int btn_number = 1; btn_number <= 6; btn_number++) {
			int fxIndex = SparkHelper::getFXIndexFromButtonNumber(btn_number);
			Pedal current_fx = activePreset->pedals[fxIndex];
			switchLED(btn_number, current_fx.isOn);
		}
	}
}

void SparkLEDControl::updateLED_AMP() {
	unsigned long currentMillis = millis();

	int presetNumToEdit = spark_dc->presetNumToEdit();
	const int presetEditMode = spark_dc->presetEditMode();

	if (presetEditMode != PRESET_EDIT_NONE) {

		if (presetNumToEdit == 0) {
			for (int i = 1; i <= 4; i++) {
				switchLED(i, true);
			}
		}
		else if (currentMillis - previousMillis >= blinkInterval_ms) {
			// save the last time you blinked the LED
			previousMillis = currentMillis;

			allLedOff();
			// if the LED is off turn it on and vice-versa:
			if (ledState == LOW) {
				ledState = HIGH;
				switchLED(presetNumToEdit, true);
			} else {
				ledState = LOW;
			}
		}

	} else {
		allLedOff();
		switchLED(activePresetNum, true);
	}
}

void SparkLEDControl::updateLED_KEYBOARD(){

	std::string pressedKey = spark_dc->lastKeyboardButtonPressed();
	if ( pressedKey == "" ) {
		allLedOff();
		return;
	}

	// Map index of pressed key from 1 to 6
	int index = mapping.indexOfKey(pressedKey)+1;
	switchLED(index, HIGH);

}

void SparkLEDControl::allLedOff(){
	digitalWrite(LED_PRESET1_GPIO, LOW);
	digitalWrite(LED_PRESET2_GPIO, LOW);
	digitalWrite(LED_PRESET3_GPIO, LOW);
	digitalWrite(LED_PRESET4_GPIO, LOW);
	digitalWrite(LED_BANK_DOWN_GPIO, LOW);
	digitalWrite(LED_BANK_UP_GPIO, LOW);
}

void SparkLEDControl::switchLED(int num, bool on){
	int STATE = on ? HIGH : LOW;
	int ledGpio = SparkHelper::getLedGpio(num);
	digitalWrite(ledGpio, STATE);
}
