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

	digitalWrite(LED_PRESET1_GPIO, LOW);
	digitalWrite(LED_PRESET2_GPIO, LOW);
	digitalWrite(LED_PRESET3_GPIO, LOW);
	digitalWrite(LED_PRESET4_GPIO, LOW);
	digitalWrite(LED_BANK_DOWN_GPIO, LOW);
	digitalWrite(LED_BANK_UP_GPIO, LOW);

}

void SparkLEDControl::updateLEDs() {

	operationMode = spark_dc->operationMode();
	activePreset = spark_dc->activePreset();
	activePresetNum = spark_dc->activePresetNum();

	switch (operationMode) {
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
	case SPARK_MODE_LOOPER:
		updateLED_APP_PresetMode();
		break;
	}
}

void SparkLEDControl::updateLED_APP_PresetMode() {
	switch (activePresetNum) {
	case 1:
		digitalWrite(LED_PRESET1_GPIO, HIGH);
		digitalWrite(LED_PRESET2_GPIO, LOW);
		digitalWrite(LED_PRESET3_GPIO, LOW);
		digitalWrite(LED_PRESET4_GPIO, LOW);
		digitalWrite(LED_BANK_DOWN_GPIO, LOW);
		digitalWrite(LED_BANK_UP_GPIO, LOW);
		break;
	case 2:
		digitalWrite(LED_PRESET1_GPIO, LOW);
		digitalWrite(LED_PRESET2_GPIO, HIGH);
		digitalWrite(LED_PRESET3_GPIO, LOW);
		digitalWrite(LED_PRESET4_GPIO, LOW);
		digitalWrite(LED_BANK_DOWN_GPIO, LOW);
		digitalWrite(LED_BANK_UP_GPIO, LOW);
		break;
	case 3:
		digitalWrite(LED_PRESET1_GPIO, LOW);
		digitalWrite(LED_PRESET2_GPIO, LOW);
		digitalWrite(LED_PRESET3_GPIO, HIGH);
		digitalWrite(LED_PRESET4_GPIO, LOW);
		digitalWrite(LED_BANK_DOWN_GPIO, LOW);
		digitalWrite(LED_BANK_UP_GPIO, LOW);
		break;
	case 4:
		digitalWrite(LED_PRESET1_GPIO, LOW);
		digitalWrite(LED_PRESET2_GPIO, LOW);
		digitalWrite(LED_PRESET3_GPIO, LOW);
		digitalWrite(LED_PRESET4_GPIO, HIGH);
		digitalWrite(LED_BANK_DOWN_GPIO, LOW);
		digitalWrite(LED_BANK_UP_GPIO, LOW);
		break;
	}
}

void SparkLEDControl::updateLED_APP_FXMode() {
	if (!activePreset->isEmpty) {

		Pedal fx_noisegate = activePreset->pedals[FX_NOISEGATE];
		if (fx_noisegate.isOn) {
			digitalWrite(LED_NOISEGATE_GPIO, HIGH);
		} else {
			digitalWrite(LED_NOISEGATE_GPIO, LOW);
		}

		Pedal fx_comp = activePreset->pedals[FX_COMP];
		if (fx_comp.isOn) {
			digitalWrite(LED_COMP_GPIO, HIGH);
		} else {
			digitalWrite(LED_COMP_GPIO, LOW);
		}

		Pedal fx_dist = activePreset->pedals[FX_DRIVE];
		if (fx_dist.isOn) {
			digitalWrite(LED_DRIVE_GPIO, HIGH);
		} else {
			digitalWrite(LED_DRIVE_GPIO, LOW);
		}

		Pedal fx_mod = activePreset->pedals[FX_MOD];
		if (fx_mod.isOn) {
			digitalWrite(LED_MOD_GPIO, HIGH);
		} else {
			digitalWrite(LED_MOD_GPIO, LOW);
		}

		Pedal fx_delay = activePreset->pedals[FX_DELAY];
		if (fx_delay.isOn) {
			digitalWrite(LED_DELAY_GPIO, HIGH);
		} else {
			digitalWrite(LED_DELAY_GPIO, LOW);
		}

		Pedal fx_reverb = activePreset->pedals[FX_REVERB];
		if (fx_reverb.isOn) {
			digitalWrite(LED_REVERB_GPIO, HIGH);
		} else {
			digitalWrite(LED_REVERB_GPIO, LOW);
		}
	}
}

void SparkLEDControl::updateLED_AMP() {
	unsigned long currentMillis = millis();
	int led_gpio = 0;

	int presetNumToEdit = spark_dc->presetNumToEdit();
	const int presetEditMode = spark_dc->presetEditMode();

	if (presetEditMode != PRESET_EDIT_NONE) {
		switch (presetNumToEdit) {
		case 1:
			led_gpio = LED_PRESET1_GPIO;
			digitalWrite(LED_PRESET2_GPIO, LOW);
			digitalWrite(LED_PRESET3_GPIO, LOW);
			digitalWrite(LED_PRESET4_GPIO, LOW);
			break;
		case 2:
			led_gpio = LED_PRESET2_GPIO;
			digitalWrite(LED_PRESET1_GPIO, LOW);
			digitalWrite(LED_PRESET3_GPIO, LOW);
			digitalWrite(LED_PRESET4_GPIO, LOW);
			break;
		case 3:
			led_gpio = LED_PRESET3_GPIO;
			digitalWrite(LED_PRESET1_GPIO, LOW);
			digitalWrite(LED_PRESET2_GPIO, LOW);
			digitalWrite(LED_PRESET4_GPIO, LOW);
			break;
		case 4:
			led_gpio = LED_PRESET4_GPIO;
			digitalWrite(LED_PRESET1_GPIO, LOW);
			digitalWrite(LED_PRESET2_GPIO, LOW);
			digitalWrite(LED_PRESET3_GPIO, LOW);
			break;
		default:
			digitalWrite(LED_PRESET1_GPIO, HIGH);
			digitalWrite(LED_PRESET2_GPIO, HIGH);
			digitalWrite(LED_PRESET3_GPIO, HIGH);
			digitalWrite(LED_PRESET4_GPIO, HIGH);
			break;
		}
		if (currentMillis - previousMillis >= interval) {
			// save the last time you blinked the LED
			previousMillis = currentMillis;

			// if the LED is off turn it on and vice-versa:
			if (ledState == LOW) {
				ledState = HIGH;
			} else {
				ledState = LOW;
			}
			// set the LED with the ledState of the variable:
			digitalWrite(led_gpio, ledState);
		}

	} else {
		switch (activePresetNum) {
		case 1:
			digitalWrite(LED_PRESET1_GPIO, HIGH);
			digitalWrite(LED_PRESET2_GPIO, LOW);
			digitalWrite(LED_PRESET3_GPIO, LOW);
			digitalWrite(LED_PRESET4_GPIO, LOW);
			digitalWrite(LED_BANK_DOWN_GPIO, LOW);
			digitalWrite(LED_BANK_UP_GPIO, LOW);
			break;
		case 2:
			digitalWrite(LED_PRESET1_GPIO, LOW);
			digitalWrite(LED_PRESET2_GPIO, HIGH);
			digitalWrite(LED_PRESET3_GPIO, LOW);
			digitalWrite(LED_PRESET4_GPIO, LOW);
			digitalWrite(LED_BANK_DOWN_GPIO, LOW);
			digitalWrite(LED_BANK_UP_GPIO, LOW);
			break;
		case 3:
			digitalWrite(LED_PRESET1_GPIO, LOW);
			digitalWrite(LED_PRESET2_GPIO, LOW);
			digitalWrite(LED_PRESET3_GPIO, HIGH);
			digitalWrite(LED_PRESET4_GPIO, LOW);
			digitalWrite(LED_BANK_DOWN_GPIO, LOW);
			digitalWrite(LED_BANK_UP_GPIO, LOW);
			break;
		case 4:
			digitalWrite(LED_PRESET1_GPIO, LOW);
			digitalWrite(LED_PRESET2_GPIO, LOW);
			digitalWrite(LED_PRESET3_GPIO, LOW);
			digitalWrite(LED_PRESET4_GPIO, HIGH);
			digitalWrite(LED_BANK_DOWN_GPIO, LOW);
			digitalWrite(LED_BANK_UP_GPIO, LOW);
			break;

		}
	}
}

