/*
 * SparkButtonHandler.cpp
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#include "SparkButtonHandler.h"

// Intialize buttons
BfButton SparkButtonHandler::btn_preset1_(BfButton::STANDALONE_DIGITAL, BUTTON_PRESET1_GPIO, false, HIGH);
BfButton SparkButtonHandler::btn_preset2_(BfButton::STANDALONE_DIGITAL, BUTTON_PRESET2_GPIO, false, HIGH);
BfButton SparkButtonHandler::btn_preset3_(BfButton::STANDALONE_DIGITAL, BUTTON_PRESET3_GPIO, false, HIGH);
BfButton SparkButtonHandler::btn_preset4_(BfButton::STANDALONE_DIGITAL, BUTTON_PRESET4_GPIO, false, HIGH);
BfButton SparkButtonHandler::btn_bank_down_(BfButton::STANDALONE_DIGITAL, BUTTON_BANK_DOWN_GPIO, false, HIGH);
BfButton SparkButtonHandler::btn_bank_up_(BfButton::STANDALONE_DIGITAL, BUTTON_BANK_UP_GPIO, false, HIGH);

// Initialize SparkDataControl;
SparkDataControl *SparkButtonHandler::spark_dc_ = nullptr;
SparkPresetControl &SparkButtonHandler::presetControl_ = SparkPresetControl::getInstance();
KeyboardMapping SparkButtonHandler::currentKeyboard_;

SparkButtonHandler::SparkButtonHandler() {
}

SparkButtonHandler::SparkButtonHandler(SparkDataControl *dc) {
    spark_dc_ = dc;
}

SparkButtonHandler::~SparkButtonHandler() {}

void SparkButtonHandler::readButtons() {
    btn_preset1_.read();
    btn_preset2_.read();
    btn_preset3_.read();
    btn_preset4_.read();
    btn_bank_up_.read();
    btn_bank_down_.read();
}

OperationMode SparkButtonHandler::checkBootOperationMode() {

    OperationMode operationMode;
    // AMP mode when Preset 1 is pressed during startup
    if (digitalRead(BUTTON_PRESET1_GPIO) == HIGH) {
        operationMode = SPARK_MODE_AMP;
    } else if (digitalRead(BUTTON_PRESET3_GPIO) == HIGH) {
        operationMode = SPARK_MODE_KEYBOARD;
    } else { // default mode: APP
        operationMode = SPARK_MODE_APP;
    }

    Serial.printf("Operation mode (boot): %d\n", operationMode);
    return operationMode;
}

void SparkButtonHandler::configureButtons() {

    OperationMode operationMode = spark_dc_->operationMode();
    SubMode subMode = spark_dc_->subMode();

    // Mode specific button config
    switch (operationMode) {
    case SPARK_MODE_APP:
        switch (subMode) {
        case SUB_MODE_PRESET:
            configureAppButtonsPreset();
            break;
        case SUB_MODE_FX:
            configureAppButtonsFX();
            break;
        case SUB_MODE_LOOPER:
            configureLooperButtons();
            break;
        case SUB_MODE_LOOP_CONFIG:
            configureSpark2LooperConfigButtons();
            break;
        case SUB_MODE_LOOP_CONTROL:
            configureSpark2LooperControlButtons();
            break;
        case SUB_MODE_TUNER:
            configureTunerButtons();
            break;
        }
        break;
    case SPARK_MODE_AMP:
        configureAmpButtons();
        break;
    case SPARK_MODE_KEYBOARD:
        configureKeyboardButtons();
        break;
    }
}

void SparkButtonHandler::configureLooperButtons() {

    // Short press preset buttons: Looper functionality
    btn_preset1_.onPress(btnKeyboardHandler);
    btn_preset2_.onPress(btnKeyboardHandler);
    btn_preset3_.onPress(btnKeyboardHandler);
    btn_preset4_.onPress(btnKeyboardHandler);
    btn_bank_down_.onPress(btnKeyboardHandler);
    btn_bank_up_.onPress(btnKeyboardHandler);

    // Long press: switch Spark presets (will move across banks)
    btn_bank_down_.onPressFor(btnLooperPresetHandler, LONG_BUTTON_PRESS_TIME);
    btn_bank_up_.onPressFor(btnLooperPresetHandler, LONG_BUTTON_PRESS_TIME);

    // Switch between APP and Looper mode
    btn_preset4_.onPressFor(btnToggleLoopHandler, LONG_BUTTON_PRESS_TIME);

    // Reset Ignitron
    btn_preset2_.onPressFor(btnResetHandler, LONG_BUTTON_PRESS_TIME);
}

void SparkButtonHandler::configureSpark2LooperControlButtons() {

    // Short press preset buttons: Looper functionality
    btn_preset1_.onPress(btnSpark2LooperHandler);
    btn_preset2_.onPress(btnSpark2LooperHandler);
    btn_preset3_.onPress(btnSpark2LooperHandler);
    btn_preset4_.onPress(btnSpark2LooperHandler);
    btn_bank_down_.onPress(btnSpark2LooperHandler);
    btn_bank_up_.onPress(btnSpark2LooperHandler);

    btn_preset3_.onPressFor(btnSpark2LooperHandler, LONG_BUTTON_PRESS_TIME);

    // Switch between APP and Looper mode
    btn_preset4_.onPressFor(btnToggleLoopHandler, LONG_BUTTON_PRESS_TIME);
    btn_bank_up_.onPressFor(btnSwitchModeHandler, LONG_BUTTON_PRESS_TIME);

    // Reset Ignitron
    btn_preset2_.onPressFor(btnResetHandler, LONG_BUTTON_PRESS_TIME);
}

void SparkButtonHandler::configureSpark2LooperConfigButtons() {

    // Short press preset buttons: Looper functionality
    btn_preset1_.onPress(btnSpark2LooperConfigHandler);
    btn_preset2_.onPress(btnSpark2LooperConfigHandler);
    btn_preset3_.onPress(btnSpark2LooperConfigHandler);
    btn_preset4_.onPress(btnSpark2LooperConfigHandler);
    btn_bank_down_.onPress(btnSpark2LooperConfigHandler);
    btn_bank_up_.onPress(btnSpark2LooperConfigHandler);
    btn_bank_down_.onPressFor(btnSpark2LooperConfigHandler, LONG_BUTTON_PRESS_TIME);

    // Switch between APP and Looper mode
    btn_preset4_.onPressFor(btnToggleLoopHandler, LONG_BUTTON_PRESS_TIME);
    btn_bank_up_.onPressFor(btnSwitchModeHandler, LONG_BUTTON_PRESS_TIME);

    // Reset Ignitron
    btn_preset2_.onPressFor(btnResetHandler, LONG_BUTTON_PRESS_TIME);
}

void SparkButtonHandler::configureAppButtonsPreset() {

    // Short press handlers
    btn_preset1_.onPress(btnPresetHandler);
    btn_preset2_.onPress(btnPresetHandler);
    btn_preset3_.onPress(btnPresetHandler);
    btn_preset4_.onPress(btnPresetHandler);

    // Setup the button event handler
    btn_bank_down_.onPress(btnBankHandler);
    btn_bank_up_.onPress(btnBankHandler);

    // Switch between FX / Preset mode
    btn_bank_up_.onPressFor(btnSwitchModeHandler, LONG_BUTTON_PRESS_TIME);
    // Switch between APP and Looper mode
    btn_preset4_.onPressFor(btnToggleLoopHandler, LONG_BUTTON_PRESS_TIME);
    btn_bank_down_.onPressFor(btnSwitchTunerModeHandler, LONG_BUTTON_PRESS_TIME);

    // Reset Ignitron
    btn_preset2_.onPressFor(btnResetHandler, LONG_BUTTON_PRESS_TIME);
}
void SparkButtonHandler::configureAppButtonsFX() {

    // Short press handlers
    btn_preset1_.onPress(btnToggleFXHandler);
    btn_preset2_.onPress(btnToggleFXHandler);
    btn_preset3_.onPress(btnToggleFXHandler);
    btn_preset4_.onPress(btnToggleFXHandler);
    btn_bank_down_.onPress(btnToggleFXHandler);
    btn_bank_up_.onPress(btnToggleFXHandler);

    // Switch between FX / Preset mode
    btn_bank_up_.onPressFor(btnSwitchModeHandler, LONG_BUTTON_PRESS_TIME);
    // Switch between APP and Looper mode
    btn_preset4_.onPressFor(btnToggleLoopHandler, LONG_BUTTON_PRESS_TIME);

    // Reset Ignitron
    btn_preset2_.onPressFor(btnResetHandler, LONG_BUTTON_PRESS_TIME);
}
void SparkButtonHandler::configureAmpButtons() {

    // Short press handlers
    btn_preset1_.onPress(btnPresetHandler);
    btn_preset2_.onPress(btnPresetHandler);
    btn_preset3_.onPress(btnPresetHandler);
    btn_preset4_.onPress(btnPresetHandler);

    // Setup the button event handler
    btn_bank_down_.onPress(btnBankHandler);
    btn_bank_up_.onPress(btnBankHandler);

    // Delete stored preset
    btn_bank_down_.onPressFor(btnDeletePresetHandler, LONG_BUTTON_PRESS_TIME);
    // Switch between BT / BLE mode (only works once, then will cause crash)
    btn_bank_up_.onPressFor(btnToggleBTModeHandler, LONG_BUTTON_PRESS_TIME);

    // Reset Ignitron
    btn_preset2_.onPressFor(btnResetHandler, LONG_BUTTON_PRESS_TIME);
}
void SparkButtonHandler::configureKeyboardButtons() {
    // Short press: Keyboard/Looper functionality
    btn_preset1_.onPress(btnKeyboardHandler);
    btn_preset2_.onPress(btnKeyboardHandler);
    btn_preset3_.onPress(btnKeyboardHandler);
    btn_preset4_.onPress(btnKeyboardHandler);
    btn_bank_down_.onPress(btnKeyboardHandler);
    btn_bank_up_.onPress(btnKeyboardHandler);

    // Long press: Keyboard/Looper functionality
    btn_preset1_.onPressFor(btnKeyboardHandler, LONG_BUTTON_PRESS_TIME);
    btn_preset2_.onPressFor(btnKeyboardHandler, LONG_BUTTON_PRESS_TIME);
    btn_preset3_.onPressFor(btnKeyboardHandler, LONG_BUTTON_PRESS_TIME);
    btn_preset4_.onPressFor(btnKeyboardHandler, LONG_BUTTON_PRESS_TIME);
    btn_bank_down_.onPressFor(btnKeyboardSwitchHandler, LONG_BUTTON_PRESS_TIME);
    btn_bank_up_.onPressFor(btnKeyboardSwitchHandler, LONG_BUTTON_PRESS_TIME);

    // btn_bank_down.onPressFor(btnKeyboardHandler, LONG_BUTTON_PRESS_TIME);
    // btn_bank_up_.onPressFor(btnKeyboardHandler, LONG_BUTTON_PRESS_TIME);
}

void SparkButtonHandler::configureTunerButtons() {
    // Toggle Tuner mode
    // Short press: Keyboard/Looper functionality
    btn_preset1_.onPress(doNothing);
    btn_preset2_.onPress(doNothing);
    btn_preset3_.onPress(doNothing);
    btn_bank_down_.onPress(doNothing);
    btn_bank_up_.onPress(doNothing);

    // Long press: Keyboard/Looper functionality
    btn_preset1_.onPressFor(doNothing, LONG_BUTTON_PRESS_TIME);
    btn_preset3_.onPressFor(doNothing, LONG_BUTTON_PRESS_TIME);
    btn_preset4_.onPressFor(doNothing, LONG_BUTTON_PRESS_TIME);
    btn_bank_down_.onPressFor(doNothing, LONG_BUTTON_PRESS_TIME);
    btn_bank_up_.onPressFor(doNothing, LONG_BUTTON_PRESS_TIME);
    btn_bank_down_.onPress(btnSwitchTunerModeHandler);
}

void SparkButtonHandler::doNothing(BfButton *btn,
                                   BfButton::press_pattern_t pattern) {
    ;
}

void SparkButtonHandler::btnKeyboardHandler(BfButton *btn,
                                            BfButton::press_pattern_t pattern) {

    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet,ignoring button press.");
        return;
    }

    // Debug
    ButtonGpio pressed_btn_gpio = (ButtonGpio)btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    string keyToSend;
    currentKeyboard_ = spark_dc_->currentKeyboard();

    // Button configuration is set in the SparkTypes.h file
    PresetLedButtonNum buttonIndex = SparkHelper::getButtonNumber(pressed_btn_gpio);

    vector<keyboardKeyDefinition> keyMap;

    switch (pattern) {
    case BfButton::SINGLE_PRESS:
        keyMap = currentKeyboard_.keyboardShortPress;
        break;
    case BfButton::LONG_PRESS:
        keyMap = currentKeyboard_.keyboardLongPress;
        break;
    }

    int keyMapSize = keyMap.size();

    if (buttonIndex > keyMapSize || buttonIndex < 1) {
        Serial.println("ERROR: Keyboard index out of bounds");
        return;
    }

    // Keyboard mapping array index starts at 0
    spark_dc_->sendButtonPressAsKeyboard(keyMap[buttonIndex - 1]);
}

void SparkButtonHandler::btnToggleLoopHandler(BfButton *btn,
                                              BfButton::press_pattern_t pattern) {

    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet,ignoring button press.");
        return;
    }
    // Switch between APP mode and LOOPER mode
    // Debug
#ifdef DEBUG
    ButtonGpio pressed_btn_gpio = (ButtonGpio)btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);
#endif

    // Switch between APP and LOOPER mode
    spark_dc_->toggleLooperAppMode();
    Serial.println("Re-initializing button config");
    configureButtons();
}

void SparkButtonHandler::btnToggleBTModeHandler(BfButton *btn,
                                                BfButton::press_pattern_t pattern) {

    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }
#ifdef DEBUG
    ButtonGpio pressed_btn_gpio = (ButtonGpio)btn->getID();
    // Debug
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);
#endif

    spark_dc_->toggleBTMode();
}

void SparkButtonHandler::btnSwitchTunerModeHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }
    // Switch between tuner mode where tuner can be separately switched
    // Debug
#ifdef DEBUG
    ButtonGpio pressed_btn_gpio = (ButtonGpio)btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);
#endif

    // Switch mode in APP mode
    SubMode currentSubMode = spark_dc_->subMode();
    DEBUG_PRINTF("Old SubMode = %d", currentSubMode);
    SubMode newMode;
    if (currentSubMode == SUB_MODE_TUNER) {
        DEBUG_PRINTLN("Old mode: Tuner");
        newMode = SUB_MODE_PRESET;
    } else {
        DEBUG_PRINTLN("Old mode: Other");
        newMode = SUB_MODE_TUNER;
    }
    spark_dc_->switchSubMode(newMode);
    Serial.println("Re-initializing button config");
    configureButtons();
}

void SparkButtonHandler::btnLooperPresetHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }
    // Debug
    ButtonGpio pressed_btn_gpio = (ButtonGpio)btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    switch (pressed_btn_gpio) {
    case BUTTON_BANK_DOWN_GPIO:
        presetControl_.decreasePresetLooper();
        break;
    case BUTTON_BANK_UP_GPIO:
        presetControl_.increasePresetLooper();
        break;
    }
}

// Spark 2 Looper controls
void SparkButtonHandler::btnSpark2LooperHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }
    // Debug
    ButtonGpio pressed_btn_gpio = (ButtonGpio)btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    // Check if button has been pressed short or long
    switch (pattern) {
    case BfButton::SINGLE_PRESS:
        switch (pressed_btn_gpio) {
        case BUTTON_PRESET1_GPIO:
            spark_dc_->sparkLooperRecDub();
            break;
        case BUTTON_PRESET2_GPIO:
            spark_dc_->sparkLooperPlayStop();
            break;
        case BUTTON_PRESET3_GPIO:
            spark_dc_->sparkLooperUndoRedo();
            break;
        case BUTTON_PRESET4_GPIO:
            spark_dc_->sparkLooperGetRecordStatus();
            break;
        case BUTTON_BANK_DOWN_GPIO:
            spark_dc_->tapTempoButton();
            break;
        case BUTTON_BANK_UP_GPIO:
            spark_dc_->sparkLooperGetStatus();
            break;
        }
        break;
    case BfButton::LONG_PRESS:
        switch (pressed_btn_gpio) {
        case BUTTON_PRESET3_GPIO:
            spark_dc_->sparkLooperDeleteAll();
            break;
        }
    }
}

void SparkButtonHandler::btnSpark2LooperConfigHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }
    // Debug
    ButtonGpio pressed_btn_gpio = (ButtonGpio)btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    SparkLooperControl &looperControl = spark_dc_->looperControl();

    // Check if button has been pressed short or long
    switch (pattern) {
    case BfButton::SINGLE_PRESS:
        switch (pressed_btn_gpio) {
        case BUTTON_PRESET1_GPIO:
            looperControl.changeSettingBars();
            break;
        case BUTTON_PRESET2_GPIO:
            looperControl.toggleSettingCount();
            break;
        case BUTTON_PRESET3_GPIO:
            looperControl.toggleSettingClick();
            break;
        case BUTTON_BANK_DOWN_GPIO:
            spark_dc_->tapTempoButton();
            break;
        }
        break;
    case BfButton::LONG_PRESS:
        looperControl.resetSetting();
        break;
    default:
        Serial.println("Unknown button press.");
    }
}

// Buttons to change Preset (in preset mode) and control FX in FX mode
void SparkButtonHandler::btnPresetHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }

    // Debug
    ButtonGpio pressed_btn_gpio = (ButtonGpio)btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    // Change selected preset
    PresetLedButtonNum selectedPresetNum = SparkHelper::getButtonNumber(pressed_btn_gpio);
    presetControl_.processPresetSelect(selectedPresetNum);
}

// Buttons to change the preset banks in preset mode and some other FX in FX mode
void SparkButtonHandler::btnBankHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }

    // Debug
    ButtonGpio pressed_btn_gpio = (ButtonGpio)btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    if (pressed_btn_gpio == BUTTON_BANK_DOWN_GPIO) {
        presetControl_.decreaseBank();
    } else if (pressed_btn_gpio == BUTTON_BANK_UP_GPIO) {
        presetControl_.increaseBank();
    }
}

void SparkButtonHandler::btnSwitchModeHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }
    // Switch between preset mode FX mode where FX can be separately switched
    // Debug
#ifdef DEBUG
    ButtonGpio pressed_btn_gpio = (ButtonGpio)btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);
#endif

    // Switch mode in APP mode
    spark_dc_->toggleSubMode();
    Serial.println("Re-initializing button config");
    configureButtons();
}

void SparkButtonHandler::btnDeletePresetHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet or in APP mode, ignoring button press.");
        return;
    }

#ifdef DEBUG
    ButtonGpio pressed_btn_gpio = (ButtonGpio)btn->getID();
    // Debug
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);
#endif

    presetControl_.handleDeletePreset();
}

void SparkButtonHandler::btnResetHandler(BfButton *btn,
                                         BfButton::press_pattern_t pattern) {

    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet,ignoring button press.");
        return;
    }
    // DEBUG
#ifdef DEBUG
    ButtonGpio pressed_btn_gpio = (ButtonGpio)btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);
#endif

    spark_dc_->restartESP(true);
}

void SparkButtonHandler::btnToggleFXHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }

    // Debug
    ButtonGpio pressed_btn_gpio = (ButtonGpio)btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    FxType fxIndex = SparkHelper::getFXIndexFromBtnGpio(pressed_btn_gpio);
    spark_dc_->toggleEffect(fxIndex);
}

void SparkButtonHandler::btnKeyboardSwitchHandler(BfButton *btn, BfButton::press_pattern_t pattern) {
    if (!spark_dc_) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }

    // Debug
    ButtonGpio pressed_btn_gpio = (ButtonGpio)(ButtonGpio)btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    if (pressed_btn_gpio == BUTTON_BANK_DOWN_GPIO) {
        currentKeyboard_ = spark_dc_->previousKeyboard();
    } else if (pressed_btn_gpio == BUTTON_BANK_UP_GPIO) {
        currentKeyboard_ = spark_dc_->nextKeyboard();
    }
}
