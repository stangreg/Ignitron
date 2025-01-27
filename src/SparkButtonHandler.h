/*
 * SparkButtonHandler.h
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#ifndef SPARKBUTTONHANDLER_H_
#define SPARKBUTTONHANDLER_H_

#include "Config_Definitions.h"
#include "SparkDataControl.h"
#include "SparkPresetControl.h"
#include "SparkTypes.h"
#include <BfButton.h> //https://github.com/mickey9801/ButtonFever

using namespace std;

class SparkButtonHandler {
public:
    SparkButtonHandler();
    SparkButtonHandler(SparkDataControl *dc);
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

    static SparkDataControl *spark_dc;
    static SparkPresetControl &presetControl;
    static KeyboardMapping currentKeyboard;

    // BUTTON Handlers
    static void btnPresetHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnLooperPresetHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnSpark2LooperHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnSpark2LooperConfigHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnBankHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnSwitchModeHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnDeletePresetHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnToggleFXHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnResetHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnKeyboardHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnKeyboardSwitchHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnToggleLoopHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnToggleBTModeHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnTESTHandler(BfButton *btn, BfButton::press_pattern_t pattern);
    static void btnSwitchTunerModeHandler(BfButton *btn, BfButton::press_pattern_t pattern);

    static void configureLooperButtons();
    static void configureSpark2LooperControlButtons();
    static void configureSpark2LooperConfigButtons();
    static void configureAppButtonsPreset();
    static void configureAppButtonsFX();
    static void configureAmpButtons();
    static void configureKeyboardButtons();
    static void configureTunerButtons();

    static void doNothing(BfButton *btn,
                                            BfButton::press_pattern_t pattern);
};

#endif /* SPARKBUTTONHANDLER_H_ */
