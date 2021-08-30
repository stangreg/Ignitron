/*
 * SparkButtonHandler.cpp
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#include "SparkButtonHandler.h"

// Intialize buttons
BfButton SparkButtonHandler::btn_preset1(BfButton::STANDALONE_DIGITAL, BUTTON_PRESET1_GPIO, false,
		HIGH);
BfButton SparkButtonHandler::btn_preset2(BfButton::STANDALONE_DIGITAL, BUTTON_PRESET2_GPIO, false,
		HIGH);
BfButton SparkButtonHandler::btn_preset3(BfButton::STANDALONE_DIGITAL, BUTTON_PRESET3_GPIO, false,
		HIGH);
BfButton SparkButtonHandler::btn_preset4(BfButton::STANDALONE_DIGITAL, BUTTON_PRESET4_GPIO, false,
		HIGH);
BfButton SparkButtonHandler::btn_bank_down(BfButton::STANDALONE_DIGITAL, BUTTON_BANK_DOWN_GPIO,
		false, HIGH);
BfButton SparkButtonHandler::btn_bank_up(BfButton::STANDALONE_DIGITAL, BUTTON_BANK_UP_GPIO, false,
		HIGH);

// Initialize SparkDataControl;
SparkDataControl* SparkButtonHandler::spark_dc = nullptr;

SparkButtonHandler::SparkButtonHandler() {
	init();

}

SparkButtonHandler::SparkButtonHandler(SparkDataControl* dc) {
	spark_dc = dc;
	init();
}

SparkButtonHandler::~SparkButtonHandler() {}

void SparkButtonHandler::readButtons(){
	btn_preset1.read();
	btn_preset2.read();
	btn_preset3.read();
	btn_preset4.read();
	btn_bank_up.read();
	btn_bank_down.read();
}

// Buttons to change Preset (in preset mode) and control FX in FX mode
void SparkButtonHandler::btnPresetHandler(BfButton *btn, BfButton::press_pattern_t pattern) {
	if(spark_dc){

		const int buttonMode = spark_dc->buttonMode();
		const int operationMode = spark_dc->operationMode();
		preset* activePreset = spark_dc->activePreset();
		int selectedPresetNum;

		if (pattern == BfButton::SINGLE_PRESS) {
			int pressed_btn_gpio = btn->getID();
			// Debug
			//Serial.print("Button pressed: ");
			//Serial.println(pressed_btn_gpio);

			// Switching between FX in FX mode
			if (buttonMode == SWITCH_MODE_FX) {
				std::string fx_name;
				boolean fx_isOn;
				if (!activePreset->isEmpty) {
					switch (pressed_btn_gpio) {
					case BUTTON_DRIVE_GPIO:
						fx_name = activePreset->pedals[FX_DRIVE].name;
						fx_isOn = activePreset->pedals[FX_DRIVE].isOn;
						break;
					case BUTTON_MOD_GPIO:
						fx_name = activePreset->pedals[FX_MOD].name;
						fx_isOn = activePreset->pedals[FX_MOD].isOn;
						break;
					case BUTTON_DELAY_GPIO:
						fx_name = activePreset->pedals[FX_DELAY].name;
						fx_isOn = activePreset->pedals[FX_DELAY].isOn;
						break;
					case BUTTON_REVERB_GPIO:
						fx_name = activePreset->pedals[FX_REVERB].name;
						fx_isOn = activePreset->pedals[FX_REVERB].isOn;
						break;
					}
					// Switching corresponding effect
					spark_dc->switchEffectOnOff(fx_name,
							fx_isOn ? false : true);
				}
				// Preset mode
			} else if (buttonMode == SWITCH_MODE_PRESET) {

				if (pressed_btn_gpio == BUTTON_PRESET1_GPIO) {
					selectedPresetNum = 1;
				} else if (pressed_btn_gpio == BUTTON_PRESET2_GPIO) {
					selectedPresetNum = 2;
				} else if (pressed_btn_gpio == BUTTON_PRESET3_GPIO) {
					selectedPresetNum = 3;
				} else if (pressed_btn_gpio == BUTTON_PRESET4_GPIO) {
					selectedPresetNum = 4;
				}
				if(operationMode == SPARK_MODE_APP){
					spark_dc->switchPreset(selectedPresetNum);
				}
				else { // AMP mode
					spark_dc->processPresetEdit(selectedPresetNum);
				}

			}
		}
	}
	else {
		Serial.println("SparkDataControl not setup yet, ignoring button press.");
	}
}

// Buttons to change the preset banks in preset mode and some other FX in FX mode
void SparkButtonHandler::btnBankHandler(BfButton *btn, BfButton::press_pattern_t pattern) {
	if(spark_dc){
		int buttonMode = spark_dc->buttonMode();
		preset* activePreset = spark_dc->activePreset();
		int activePresetNum = spark_dc->activePresetNum();
		int pendingBank = spark_dc->pendingBank();
		int numberOfBanks = spark_dc->numberOfBanks();
		int opMode = spark_dc->operationMode();


		if (pattern == BfButton::SINGLE_PRESS) {
			int pressed_btn_gpio = btn->getID();
			// Debug
			//Serial.print("Button pressed: ");
			//Serial.println(pressed_btn_gpio);

			if (buttonMode == SWITCH_MODE_FX) {

				std::string fx_name;
				boolean fx_isOn;
				if (!activePreset->isEmpty) {
					if (pressed_btn_gpio == BUTTON_NOISEGATE_GPIO) {
						fx_name = activePreset->pedals[FX_NOISEGATE].name;
						fx_isOn = activePreset->pedals[FX_NOISEGATE].isOn;
					} else if (pressed_btn_gpio == BUTTON_COMP_GPIO) {
						fx_name = activePreset->pedals[FX_COMP].name;
						fx_isOn = activePreset->pedals[FX_COMP].isOn;
					}
					// Switch corresponding effect
					spark_dc->switchEffectOnOff(fx_name,
							fx_isOn ? false : true);
				}

				// Preset mode
			} else if (buttonMode == SWITCH_MODE_PRESET) {
				if (pressed_btn_gpio == BUTTON_BANK_DOWN_GPIO) {
					// Cycle around if at the start
					if (pendingBank == 0) {
						pendingBank = numberOfBanks;
					} else {
						pendingBank--;
					} // else
					// Don't go to bank 0 in AMP mode
					if (opMode == SPARK_MODE_AMP && pendingBank == 0){
						pendingBank = numberOfBanks;
					}
				} //if BUTTON_BANK_UP/DOWN
				else if (pressed_btn_gpio == BUTTON_BANK_UP_GPIO) {
					// Cycle around if at the end
					if (pendingBank == numberOfBanks) {
						pendingBank = 0;
					} else {
						pendingBank++;
					}
					if (opMode == SPARK_MODE_AMP && pendingBank == 0){
						pendingBank = std::min(1,numberOfBanks);
					}
				}
				// Mark selected bank as the pending Bank to mark in display.
				// The device is configured so that it does not immediately
				// switch presets when a bank is changed, it does so only when
				// the preset button is pushed afterwards
				spark_dc->pendingBank() = pendingBank;
				if(pendingBank != 0){
					spark_dc->updatePendingPreset(pendingBank);
				}
				if (opMode == SPARK_MODE_AMP) {
					spark_dc->activeBank() = pendingBank;
					spark_dc->updateActiveWithPendingPreset();
					spark_dc->resetPresetDeletionFlag();
				}
			} // SWITCH_MODE_PREFIX
		} // SINGLE PRESS
	}
	else {
		Serial.println("SparkDataControl not setup yet, ignoring button press.");
	}
}

void SparkButtonHandler::btnSwitchModeHandler(BfButton *btn, BfButton::press_pattern_t pattern) {
	if(spark_dc && spark_dc->operationMode() == SPARK_MODE_APP){
		// Long press to switch between preset mode FX mode where FX can be separately switched
		const int buttonMode = spark_dc->buttonMode();
		if (pattern == BfButton::LONG_PRESS) {
			int pressed_btn_gpio = btn->getID();
			// Debug
			//Serial.print("Button long pressed: ");
			//Serial.println(pressed_btn_gpio);
			//Up preset
			if (pressed_btn_gpio == BUTTON_BANK_UP_GPIO) {
				Serial.print("Switching to ");
				switch (buttonMode) {
				case SWITCH_MODE_FX:
					Serial.println("PRESET mode");
					spark_dc->buttonMode() = SWITCH_MODE_PRESET;
					break;
				case SWITCH_MODE_PRESET:
					Serial.println("FX mode");
					spark_dc->buttonMode() = SWITCH_MODE_FX;
					spark_dc->updatePendingWithActiveBank();
					break;
				default:
					Serial.println("Unexpected mode. Defaulting to PRESET mode");
					spark_dc->buttonMode() = SWITCH_MODE_PRESET;
					break;
				} // SWITCH
			} // IF BANK UP BUTTON
		} // LONG PRESS
	} // IF spark_dc and in APP mode
	else {
		Serial.println("SparkDataControl not setup yet or in AMP mode, ignoring button press.");
	}
}

void SparkButtonHandler::btnDeletePresetHandler(BfButton *btn, BfButton::press_pattern_t pattern){
	if(spark_dc && spark_dc->operationMode() == SPARK_MODE_AMP){
		Serial.println("Entering Delete preset handler");
		const int buttonMode = spark_dc->buttonMode();
		const bool isPresetMarkedForDeletion = spark_dc->isPresetMarkedForDeletion();
		const bool hasAppReceivedPreset = spark_dc->presetReceivedFromApp();
		const int presetNum = spark_dc->activePresetNum();

		if (pattern == BfButton::LONG_PRESS) {
			int pressed_btn_gpio = btn->getID();
			// Debug
			//Serial.print("Button long pressed: ");
			//Serial.println(pressed_btn_gpio);
			//Up preset
			if (pressed_btn_gpio == BUTTON_BANK_DOWN_GPIO) {
				Serial.println("Bank down pressed");
				if (hasAppReceivedPreset) {
					spark_dc->resetReceivedPreset();
				}
				else {
					spark_dc->processPresetEdit();
				}
			} // IF BANK UP BUTTON
		} // LONG PRESS
	} // IF spark_dc and in APP mode
	else {
		Serial.println("SparkDataControl not setup yet or in APP mode, ignoring button press.");
	}

}

int SparkButtonHandler::init() {
	// Setup the button event handler
	btn_preset1.onPress(btnPresetHandler);
	btn_preset2.onPress(btnPresetHandler);
	btn_preset3.onPress(btnPresetHandler);
	btn_preset4.onPress(btnPresetHandler);
	btn_bank_down.onPress(btnBankHandler);
	btn_bank_up.onPress(btnBankHandler);
	btn_bank_up.onPressFor(btnSwitchModeHandler, 1500);
	btn_bank_down.onPressFor(btnDeletePresetHandler, 1500);
	if (digitalRead(BUTTON_PRESET1_GPIO) == HIGH){
		return SPARK_MODE_AMP;
	}
	else{
		return SPARK_MODE_APP;
	}
}
