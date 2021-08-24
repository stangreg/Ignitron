/*
 * SparkButtonHandler.h
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#ifndef SPARKBUTTONHANDLER_H_
#define SPARKBUTTONHANDLER_H_

#include <BfButton.h> //https://github.com/mickey9801/ButtonFever
#include "SparkDataControl.h"

// GPIO Buttons/LEDs
#define BUTTON_PRESET1_GPIO 19
#define BUTTON_DRIVE_GPIO 19

#define BUTTON_PRESET2_GPIO 18
#define BUTTON_MOD_GPIO 18

#define BUTTON_PRESET3_GPIO 32
#define BUTTON_DELAY_GPIO 32

#define BUTTON_PRESET4_GPIO 25
#define BUTTON_REVERB_GPIO 25

#define BUTTON_BANK_DOWN_GPIO 26
#define BUTTON_NOISEGATE_GPIO 26

#define BUTTON_BANK_UP_GPIO 33
#define BUTTON_COMP_GPIO 33



class SparkButtonHandler {
public:
	SparkButtonHandler();
	SparkButtonHandler(SparkDataControl* dc);
	virtual ~SparkButtonHandler();

	void readButtons();
	SparkDataControl* dataControl() {return spark_dc;}

private:

	static BfButton btn_preset1;
	static BfButton btn_preset2;
	static BfButton btn_preset3;
	static BfButton btn_preset4;
	static BfButton btn_bank_up;
	static BfButton btn_bank_down;
	static SparkDataControl* spark_dc;
	void init();
	// BUTTON Handlers
	static void btnPresetHandler(BfButton *btn, BfButton::press_pattern_t pattern);
	static void btnBankHandler(BfButton *btn, BfButton::press_pattern_t pattern);
	static void btnSwitchModeHandler(BfButton *btn, BfButton::press_pattern_t pattern);
};

#endif /* SPARKBUTTONHANDLER_H_ */
