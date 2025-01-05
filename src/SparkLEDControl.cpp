/*
 * SparkLEDControl.cpp
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#include "SparkLEDControl.h"

SparkLEDControl::SparkLEDControl() {
    spark_dc = nullptr;
    init();
}

SparkLEDControl::SparkLEDControl(SparkDataControl *dc) {
    spark_dc = dc;
    init();
}

SparkLEDControl::~SparkLEDControl() {
}

void SparkLEDControl::init() {

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
    activePreset = SparkPresetControl::getInstance().activePreset();
    activePresetNum = SparkPresetControl::getInstance().activePresetNum();

    switch (operationMode) {

    case SPARK_MODE_LOOPER:
    case SPARK_MODE_SPK_LOOPER:
    case SPARK_MODE_APP: {
        int buttonMode = spark_dc->buttonMode();
        // Show only active preset LED
        if (buttonMode == BUTTON_MODE_PRESET) {
            updateLED_APP_PresetMode();
            // For each effect, show which effect is active
        } else if (buttonMode == BUTTON_MODE_FX) {
            updateLED_APP_FXMode();
        } else if (buttonMode == BUTTON_MODE_LOOP_CONFIG || buttonMode == BUTTON_MODE_LOOP_CONTROL) {
            updateLED_LooperMode();
        }
    } break;

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
    if (!activePreset.isEmpty) {
        for (int btn_number = 1; btn_number <= 6; btn_number++) {
            int fxIndex = SparkHelper::getFXIndexFromButtonNumber(btn_number);
            /*if( fxIndex == -1){
                Serial.println("Invalid FX index found!");
                return;
            }*/
            Pedal current_fx = activePreset.pedals[fxIndex];
            switchLED(btn_number, current_fx.isOn);
        }
    }
}

void SparkLEDControl::updateLED_AMP() {
    unsigned long currentMillis = millis();

    int presetNumToEdit = SparkPresetControl::getInstance().presetNumToEdit();
    const int presetEditMode = SparkPresetControl::getInstance().presetEditMode();

    if (presetEditMode != PRESET_EDIT_NONE) {

        if (presetNumToEdit == 0) {
            for (int i = 1; i <= 4; i++) {
                switchLED(i, true);
            }
        } else if (currentMillis - previousMillis >= blinkInterval_ms) {
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

void SparkLEDControl::updateLED_KEYBOARD() {

    uint8_t pressedKey = spark_dc->lastKeyboardButtonPressed();
    if (pressedKey == 0) {
        allLedOff();
        return;
    }

    // Map index of pressed key from 1 to 6
    int index = mapping.indexOfKey(pressedKey);
    switchLED(index, HIGH);
}

void SparkLEDControl::updateLED_LooperMode() {

    const bool onOff = spark_dc->looperControl()->beatOnOff();
    allLedOff();
    // if the LED is off turn it on and vice-versa:
    if (onOff) {
        ledState = HIGH;
        switchLED(SPK_LOOPER_BPM_LED_ID, true);
    } else {
        ledState = LOW;
    }

    const bool isRecRunning = spark_dc->looperControl()->isRecRunning();
    if (isRecRunning) {
        switchLED(SPK_LOOPER_REC_DUB_LED_ID, true);
    } else {
        switchLED(SPK_LOOPER_REC_DUB_LED_ID, false);
    }

    const bool isPlaying = spark_dc->looperControl()->isPlaying();
    const bool isRecAvailable = spark_dc->looperControl()->isRecAvailable();

    if (isPlaying) {
        switchLED(SPK_LOOPER_PLAY_STOP_LED_ID, true);
    } else if (isRecAvailable) {
        switchLED(SPK_LOOPER_PLAY_STOP_LED_ID, onOff);
    } else {
        switchLED(SPK_LOOPER_PLAY_STOP_LED_ID, false);
    }

    // TODO: Check why LED is not on when Undo is available during Dub/Play. Seems that 0278 is ignored during play.
    const bool canUndo = spark_dc->looperControl()->canUndo();
    const bool canRedo = spark_dc->looperControl()->canRedo();
    if (canRedo) {
        switchLED(SPK_LOOPER_UNDO_REDO_LED_ID, false);
    } else if (canUndo) {
        switchLED(SPK_LOOPER_UNDO_REDO_LED_ID, true);
    } else {
        switchLED(SPK_LOOPER_UNDO_REDO_LED_ID, false);
    }
}

void SparkLEDControl::allLedOff() {
    digitalWrite(LED_PRESET1_GPIO, LOW);
    digitalWrite(LED_PRESET2_GPIO, LOW);
    digitalWrite(LED_PRESET3_GPIO, LOW);
    digitalWrite(LED_PRESET4_GPIO, LOW);
    digitalWrite(LED_BANK_DOWN_GPIO, LOW);
    digitalWrite(LED_BANK_UP_GPIO, LOW);
}

void SparkLEDControl::switchLED(int num, bool on) {
    int STATE = on ? HIGH : LOW;
    int ledGpio = SparkHelper::getLedGpio(num);
    if (ledGpio != -1) {
        digitalWrite(ledGpio, STATE);
    }
}
