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
SparkDataControl *SparkButtonHandler::spark_dc = nullptr;
KeyboardMapping SparkButtonHandler::currentKeyboard;

SparkButtonHandler::SparkButtonHandler() {
}

SparkButtonHandler::SparkButtonHandler(SparkDataControl *dc) {
    spark_dc = dc;
}

SparkButtonHandler::~SparkButtonHandler() {}

void SparkButtonHandler::readButtons() {
    btn_preset1.read();
    btn_preset2.read();
    btn_preset3.read();
    btn_preset4.read();
    btn_bank_up.read();
    btn_bank_down.read();
}

int SparkButtonHandler::checkBootOperationMode() {

    int operationMode;
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

    int operationMode = spark_dc->operationMode();
    int buttonMode = spark_dc->buttonMode();

    // Mode specific button config
    switch (operationMode) {
    case SPARK_MODE_LOOPER:
        configureLooperButtons();
        break;
    case SPARK_MODE_SPK_LOOPER:
        if (buttonMode == BUTTON_MODE_LOOP_CONTROL) {
            configureSpark2LooperControlButtons();
        } else if (buttonMode == BUTTON_MODE_LOOP_CONFIG) {
            configureSpark2LooperConfigButtons();
        }
        break;
    case SPARK_MODE_APP:
        if (buttonMode == BUTTON_MODE_PRESET) {
            configureAppButtonsPreset();
        } else if (buttonMode == BUTTON_MODE_FX) {
            configureAppButtonsFX();
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
    btn_preset1.onPress(btnKeyboardHandler);
    btn_preset2.onPress(btnKeyboardHandler);
    btn_preset3.onPress(btnKeyboardHandler);
    btn_preset4.onPress(btnKeyboardHandler);
    btn_bank_down.onPress(btnKeyboardHandler);
    btn_bank_up.onPress(btnKeyboardHandler);

    // Long press: switch Spark presets (will move across banks)
    btn_bank_down.onPressFor(btnLooperPresetHandler, LONG_BUTTON_PRESS_TIME);
    btn_bank_up.onPressFor(btnLooperPresetHandler, LONG_BUTTON_PRESS_TIME);

    // Switch between APP and Looper mode
    btn_preset4.onPressFor(btnToggleLoopHandler, LONG_BUTTON_PRESS_TIME);

    // Reset Ignitron
    btn_preset2.onPressFor(btnResetHandler, LONG_BUTTON_PRESS_TIME);
}

void SparkButtonHandler::configureSpark2LooperControlButtons() {

    // Short press preset buttons: Looper functionality
    btn_preset1.onPress(btnSpark2LooperHandler);
    btn_preset2.onPress(btnSpark2LooperHandler);
    btn_preset3.onPress(btnSpark2LooperHandler);
    btn_preset4.onPress(btnSpark2LooperHandler);
    btn_bank_down.onPress(btnSpark2LooperHandler);
    btn_bank_up.onPress(btnSpark2LooperHandler);
    btn_preset3.onPressFor(btnSpark2LooperHandler, LONG_BUTTON_PRESS_TIME);

    btn_bank_down.onPressFor(btnSpark2LooperHandler, LONG_BUTTON_PRESS_TIME);

    // Switch between APP and Looper mode
    btn_preset4.onPressFor(btnToggleLoopHandler, LONG_BUTTON_PRESS_TIME);
    btn_bank_up.onPressFor(btnSwitchModeHandler, LONG_BUTTON_PRESS_TIME);

    // Reset Ignitron
    btn_preset2.onPressFor(btnResetHandler, LONG_BUTTON_PRESS_TIME);
}

void SparkButtonHandler::configureSpark2LooperConfigButtons() {

    // Short press preset buttons: Looper functionality
    btn_preset1.onPress(btnSpark2LooperConfigHandler);
    btn_preset2.onPress(btnSpark2LooperConfigHandler);
    btn_preset3.onPress(btnSpark2LooperConfigHandler);
    btn_preset4.onPress(btnSpark2LooperConfigHandler);
    btn_bank_down.onPress(btnSpark2LooperConfigHandler);
    btn_bank_up.onPress(btnSpark2LooperConfigHandler);
    btn_bank_down.onPressFor(btnSpark2LooperConfigHandler, LONG_BUTTON_PRESS_TIME);

    // Switch between APP and Looper mode
    btn_preset4.onPressFor(btnToggleLoopHandler, LONG_BUTTON_PRESS_TIME);
    btn_bank_up.onPressFor(btnSwitchModeHandler, LONG_BUTTON_PRESS_TIME);

    // Reset Ignitron
    btn_preset2.onPressFor(btnResetHandler, LONG_BUTTON_PRESS_TIME);
}

void SparkButtonHandler::configureAppButtonsPreset() {

    // Short press handlers
    btn_preset1.onPress(btnPresetHandler);
    btn_preset2.onPress(btnPresetHandler);
    btn_preset3.onPress(btnPresetHandler);
    btn_preset4.onPress(btnPresetHandler);

    // Setup the button event handler
    btn_bank_down.onPress(btnBankHandler);
    btn_bank_up.onPress(btnBankHandler);

    // Switch between FX / Preset mode
    btn_bank_up.onPressFor(btnSwitchModeHandler, LONG_BUTTON_PRESS_TIME);
    // Switch between APP and Looper mode
    btn_preset4.onPressFor(btnToggleLoopHandler, LONG_BUTTON_PRESS_TIME);

    // Reset Ignitron
    btn_preset2.onPressFor(btnResetHandler, LONG_BUTTON_PRESS_TIME);
}
void SparkButtonHandler::configureAppButtonsFX() {

    // Short press handlers
    btn_preset1.onPress(btnToggleFXHandler);
    btn_preset2.onPress(btnToggleFXHandler);
    btn_preset3.onPress(btnToggleFXHandler);
    btn_preset4.onPress(btnToggleFXHandler);
    btn_bank_down.onPress(btnToggleFXHandler);
    btn_bank_up.onPress(btnToggleFXHandler);

    // Switch between FX / Preset mode
    btn_bank_up.onPressFor(btnSwitchModeHandler, LONG_BUTTON_PRESS_TIME);
    // Switch between APP and Looper mode
    btn_preset4.onPressFor(btnToggleLoopHandler, LONG_BUTTON_PRESS_TIME);

    // Reset Ignitron
    btn_preset2.onPressFor(btnResetHandler, LONG_BUTTON_PRESS_TIME);
}
void SparkButtonHandler::configureAmpButtons() {

    // Short press handlers
    btn_preset1.onPress(btnPresetHandler);
    btn_preset2.onPress(btnPresetHandler);
    btn_preset3.onPress(btnPresetHandler);
    btn_preset4.onPress(btnPresetHandler);

    // Setup the button event handler
    btn_bank_down.onPress(btnBankHandler);
    btn_bank_up.onPress(btnBankHandler);

    // Delete stored preset
    btn_bank_down.onPressFor(btnDeletePresetHandler, LONG_BUTTON_PRESS_TIME);
    // Switch between BT / BLE mode (only works once, then will cause crash)
    btn_bank_up.onPressFor(btnToggleBTMode, LONG_BUTTON_PRESS_TIME);

    // Reset Ignitron
    btn_preset2.onPressFor(btnResetHandler, LONG_BUTTON_PRESS_TIME);
}
void SparkButtonHandler::configureKeyboardButtons() {
    // Short press: Keyboard/Looper functionality
    btn_preset1.onPress(btnKeyboardHandler);
    btn_preset2.onPress(btnKeyboardHandler);
    btn_preset3.onPress(btnKeyboardHandler);
    btn_preset4.onPress(btnKeyboardHandler);
    btn_bank_down.onPress(btnKeyboardHandler);
    btn_bank_up.onPress(btnKeyboardHandler);

    // Long press: Keyboard/Looper functionality
    btn_preset1.onPressFor(btnKeyboardHandler, LONG_BUTTON_PRESS_TIME);
    btn_preset2.onPressFor(btnKeyboardHandler, LONG_BUTTON_PRESS_TIME);
    btn_preset3.onPressFor(btnKeyboardHandler, LONG_BUTTON_PRESS_TIME);
    btn_preset4.onPressFor(btnKeyboardHandler, LONG_BUTTON_PRESS_TIME);
    btn_bank_down.onPressFor(btnKeyboardSwitchHandler, LONG_BUTTON_PRESS_TIME);
    btn_bank_up.onPressFor(btnKeyboardSwitchHandler, LONG_BUTTON_PRESS_TIME);

    // btn_bank_down.onPressFor(btnKeyboardHandler, LONG_BUTTON_PRESS_TIME);
    // btn_bank_up.onPressFor(btnKeyboardHandler, LONG_BUTTON_PRESS_TIME);
}

void SparkButtonHandler::btnKeyboardHandler(BfButton *btn,
                                            BfButton::press_pattern_t pattern) {

    if (!spark_dc) {
        Serial.println("SparkDataControl not setup yet,ignoring button press.");
        return;
    }

    // Debug
    int pressed_btn_gpio = btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    string keyToSend;
    currentKeyboard = spark_dc->currentKeyboard();

    // Button configuration is set in the SparkTypes.h file
    int buttonIndex = SparkHelper::getButtonNumber(pressed_btn_gpio);

    vector<keyboardKeyDefinition> keyMap;

    switch (pattern) {
    case BfButton::SINGLE_PRESS:
        keyMap = currentKeyboard.keyboardShortPress;
        break;
    case BfButton::LONG_PRESS:
        keyMap = currentKeyboard.keyboardLongPress;
        break;
    }

    int keyMapSize = keyMap.size();

    if (buttonIndex > keyMapSize || buttonIndex < 1) {
        Serial.println("ERROR: Keyboard index out of bounds");
        return;
    }

    // Keyboard mapping array index starts at 0
    spark_dc->sendButtonPressAsKeyboard(keyMap[buttonIndex - 1]);
}

void SparkButtonHandler::btnToggleLoopHandler(BfButton *btn,
                                              BfButton::press_pattern_t pattern) {

    if (!spark_dc) {
        Serial.println("SparkDataControl not setup yet,ignoring button press.");
        return;
    }
    // Switch between APP mode and LOOPER mode
    // Debug
#ifdef DEBUG
    int pressed_btn_gpio = btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);
#endif

    // Switch between APP and LOOPER mode
    spark_dc->toggleLooperAppMode();
    Serial.println("Re-initializing button config");
    configureButtons();
}

void SparkButtonHandler::btnToggleBTMode(BfButton *btn,
                                         BfButton::press_pattern_t pattern) {

    if (!spark_dc) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }
#ifdef DEBUG
    int pressed_btn_gpio = btn->getID();
    // Debug
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);
#endif

    spark_dc->toggleBTMode();
}

void SparkButtonHandler::btnLooperPresetHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }
    // Debug
    int pressed_btn_gpio = btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    switch (pressed_btn_gpio) {
    case BUTTON_BANK_DOWN_GPIO:
        spark_dc->decreasePresetLooper();
        break;
    case BUTTON_BANK_UP_GPIO:
        spark_dc->increasePresetLooper();
        break;
    }
}

// Spark 2 Looper controls
void SparkButtonHandler::btnSpark2LooperHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }
    // Debug
    int pressed_btn_gpio = btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    // Check if button has been pressed short or long
    switch (pattern) {
    case BfButton::SINGLE_PRESS:
        switch (pressed_btn_gpio) {
        case BUTTON_PRESET1_GPIO:
            spark_dc->sparkLooperCommand(SPK_LOOPER_CMD_COUNTIN);
            break;
        case BUTTON_PRESET2_GPIO:
            spark_dc->sparkLooperCommand(SPK_LOOPER_CMD_PLAY);
            break;
        case BUTTON_PRESET3_GPIO:
            spark_dc->sparkLooperCommand(SPK_LOOPER_CMD_STOP);
            break;
        case BUTTON_PRESET4_GPIO:
            spark_dc->sparkLooperCommand(SPK_LOOPER_CMD_DUB);
            break;
        case BUTTON_BANK_DOWN_GPIO:
            spark_dc->sparkLooperCommand(SPK_LOOPER_CMD_UNDO);
            break;
        case BUTTON_BANK_UP_GPIO:
            spark_dc->sparkLooperCommand(SPK_LOOPER_CMD_REDO);
            break;
        }
        break;
    case BfButton::LONG_PRESS:
        switch (pressed_btn_gpio) {
        case BUTTON_BANK_DOWN_GPIO:
            spark_dc->sparkLooperCommand(SPK_LOOPER_CMD_DELETE);
            break;
        }
        break;
    }
}

void SparkButtonHandler::btnSpark2LooperConfigHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }
    // Debug
    int pressed_btn_gpio = btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    SparkLooperControl *looperControl = spark_dc->looperControl();

    // Check if button has been pressed short or long
    switch (pattern) {
    case BfButton::SINGLE_PRESS:
        switch (pressed_btn_gpio) {
        case BUTTON_PRESET1_GPIO:
            spark_dc->tapTempoButton();
            break;
        case BUTTON_PRESET2_GPIO:
            looperControl->changeSettingBars();
            break;
        case BUTTON_PRESET3_GPIO:
            looperControl->toggleSettingCount();
            break;
        case BUTTON_PRESET4_GPIO:
            looperControl->toggleSettingClick();
            break;
        }
        break;
    case BfButton::LONG_PRESS:
        looperControl->resetSetting();
        break;
    default:
        Serial.println("Unknown button press.");
    }
}

// Buttons to change Preset (in preset mode) and control FX in FX mode
void SparkButtonHandler::btnPresetHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }

    // Debug
    int pressed_btn_gpio = btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    // Change selected preset
    int selectedPresetNum = SparkHelper::getButtonNumber(pressed_btn_gpio);
    spark_dc->processPresetSelect(selectedPresetNum);
}

// Buttons to change the preset banks in preset mode and some other FX in FX mode
void SparkButtonHandler::btnBankHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }

    // Debug
    int pressed_btn_gpio = btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    if (pressed_btn_gpio == BUTTON_BANK_DOWN_GPIO) {
        spark_dc->decreaseBank();
    } else if (pressed_btn_gpio == BUTTON_BANK_UP_GPIO) {
        spark_dc->increaseBank();
    }
}

void SparkButtonHandler::btnSwitchModeHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }
    // Switch between preset mode FX mode where FX can be separately switched
    // Debug
#ifdef DEBUG
    int pressed_btn_gpio = btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);
#endif

    // Switch mode in APP mode
    spark_dc->toggleButtonMode();
    Serial.println("Re-initializing button config");
    configureButtons();
}

void SparkButtonHandler::btnDeletePresetHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc) {
        Serial.println("SparkDataControl not setup yet or in APP mode, ignoring button press.");
        return;
    }

#ifdef DEBUG
    int pressed_btn_gpio = btn->getID();
    // Debug
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);
#endif

    spark_dc->handleDeletePreset();
}

void SparkButtonHandler::btnResetHandler(BfButton *btn,
                                         BfButton::press_pattern_t pattern) {

    if (!spark_dc) {
        Serial.println("SparkDataControl not setup yet,ignoring button press.");
        return;
    }
    // DEBUG
#ifdef DEBUG
    int pressed_btn_gpio = btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);
#endif

    spark_dc->restartESP(true);
}

void SparkButtonHandler::btnToggleFXHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

    if (!spark_dc) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }

    // Debug
    int pressed_btn_gpio = btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    int fxIndex = SparkHelper::getFXIndexFromBtnGpio(pressed_btn_gpio);
    spark_dc->toggleEffect(fxIndex);
}

void SparkButtonHandler::btnKeyboardSwitchHandler(BfButton *btn, BfButton::press_pattern_t pattern) {
    if (!spark_dc) {
        Serial.println("SparkDataControl not setup yet, ignoring button press.");
        return;
    }

    // Debug
    int pressed_btn_gpio = btn->getID();
    DEBUG_PRINT("Button pressed: ");
    DEBUG_PRINTLN(pressed_btn_gpio);

    if (pressed_btn_gpio == BUTTON_BANK_DOWN_GPIO) {
        currentKeyboard = spark_dc->previousKeyboard();
    } else if (pressed_btn_gpio == BUTTON_BANK_UP_GPIO) {
        currentKeyboard = spark_dc->nextKeyboard();
    }
}
