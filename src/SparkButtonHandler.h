/*
 * SparkButtonHandler.h
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#ifndef SPARKBUTTONHANDLER_H_
#define SPARKBUTTONHANDLER_H_

#include <BfButton.h> //https://github.com/mickey9801/ButtonFever
#include "Config_Definitions.h"
#include "SparkDataControl.h"
#include "SparkTypes.h"


class SparkButtonHandler {
public:
	SparkButtonHandler();
	SparkButtonHandler(SparkDataControl* dc);
	virtual ~SparkButtonHandler();

	void readButtons();
	int checkBootOperationMode();
	static void configureButtons();
	void setDataControl(SparkDataControl *dc) {
		spark_dc = dc;
	}

private:

	static BfButton btn_preset1;
	static BfButton btn_preset2;
	static BfButton btn_preset3;
	static BfButton btn_preset4;
	static BfButton btn_bank_up;
	static BfButton btn_bank_down;
	static SparkDataControl* spark_dc;

	static KeyboardMapping mapping;
	// BUTTON Handlers
	static void btnPresetHandler(BfButton *btn, BfButton::press_pattern_t pattern);
	static void btnLooperPresetHandler(BfButton *btn, BfButton::press_pattern_t pattern);
	static void btnBankHandler(BfButton *btn, BfButton::press_pattern_t pattern);
	static void btnSwitchModeHandler(BfButton *btn, BfButton::press_pattern_t pattern);
	static void btnDeletePresetHandler(BfButton *btn, BfButton::press_pattern_t pattern);
	static void btnToggleFXHandler(BfButton *btn, BfButton::press_pattern_t pattern);
	static void btnResetHandler(BfButton *btn,
			BfButton::press_pattern_t pattern);
	static void btnKeyboardHandler(BfButton *btn,
			BfButton::press_pattern_t pattern);

	static void btnToggleLoopHandler(BfButton *btn,
			BfButton::press_pattern_t pattern);
	static void btnToggleBTMode(BfButton *btn,
			BfButton::press_pattern_t pattern);



	static void configureLooperButtons();
	static void configureAppButtonsPreset();
	static void configureAppButtonsFX();
	static void configureAmpButtons();
	static void configureKeyboardButtons();

};

#endif /* SPARKBUTTONHANDLER_H_ */
