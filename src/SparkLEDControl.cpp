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

		Pedal fx_noisegate = activePreset->pedals[FX_NOISEGATE];
		switch(LED_NOISEGATE_NUM, fx_noisegate.isOn);

		Pedal fx_comp = activePreset->pedals[FX_COMP];
		switch(LED_COMP_NUM, fx_comp.isOn);

		Pedal fx_drive = activePreset->pedals[FX_DRIVE];
		switch(LED_DRIVE_NUM, fx_drive.isOn);

		Pedal fx_mod = activePreset->pedals[FX_MOD];
		switch(LED_MOD_NUM, fx_mod.isOn);

		Pedal fx_delay = activePreset->pedals[FX_DELAY];
		switch(LED_DELAY_NUM, fx_delay.isOn);

		Pedal fx_reverb = activePreset->pedals[FX_REVERB];
		switch(LED_REVERB_NUM, fx_reverb.isOn);

	}
}

void SparkLEDControl::updateLED_AMP() {
	unsigned long currentMillis = millis();

	int presetNumToEdit = spark_dc->presetNumToEdit();
	const int presetEditMode = spark_dc->presetEditMode();

	if (presetEditMode != PRESET_EDIT_NONE) {

		if (currentMillis - previousMillis >= interval) {
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
	switch(num){
	case 1:
		digitalWrite(LED_PRESET1_GPIO, STATE);
		break;
	case 2:
		digitalWrite(LED_PRESET2_GPIO, STATE);
		break;
	case 3:
		digitalWrite(LED_PRESET3_GPIO, STATE);
		break;
	case 4:
		digitalWrite(LED_PRESET4_GPIO, STATE);
		break;
	case 5:
		digitalWrite(LED_BANK_DOWN_GPIO, STATE);
		break;
	case 6:
		digitalWrite(LED_BANK_UP_GPIO, STATE);
		break;
	}
}
