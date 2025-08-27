/*
 * SparkModeManager.cpp
 *
 *  Created on: 27.08.2025
 *      Author: GitHub Copilot
 */

#include "SparkModeManager.h"
#include <FS.h>
#include <LittleFS.h>

SparkModeManager::SparkModeManager() {
    // Initialize with default values
    operationMode_ = SPARK_MODE_APP;
    subMode_ = SUB_MODE_PRESET;
    currentBTMode_ = BT_MODE_BLE;
}

SparkModeManager::~SparkModeManager() {
    // Nothing to clean up
}

OperationMode SparkModeManager::init(OperationMode opModeInput) {
    operationMode_ = opModeInput;
    readOpModeFromFile();
    readBTModeFromFile();
    return operationMode_;
}

void SparkModeManager::switchSubMode(SubMode subMode) {
    // Switch off tuner mode at amp if was enabled before but is not matching current subMode
    if (subMode_ == SUB_MODE_TUNER && subMode_ != subMode) {
        // Need to notify external components to switch tuner off
    }

    if (subMode == SUB_MODE_TUNER) {
        // Need to notify external components to switch tuner on
    }

    subMode_ = subMode;
}

bool SparkModeManager::toggleSubMode() {
    if (operationMode_ == SPARK_MODE_AMP) {
        Serial.println("In AMP mode, doing nothing.");
        return false;
    }

    Serial.print("Switching to ");
    if (operationMode_ == SPARK_MODE_APP) {
        switch (subMode_) {
        case SUB_MODE_FX:
            Serial.println("PRESET mode");
            subMode_ = SUB_MODE_PRESET;
            break;
        case SUB_MODE_PRESET:
            Serial.println("FX mode");
            subMode_ = SUB_MODE_FX;
            break;
        case SUB_MODE_LOOP_CONTROL:
            Serial.println("Looper CONFIG mode");
            subMode_ = SUB_MODE_LOOP_CONFIG;
            break;
        case SUB_MODE_LOOP_CONFIG:
            Serial.println("Looper CONTROL mode");
            subMode_ = SUB_MODE_LOOP_CONTROL;
            break;
        default:
            Serial.println("Unexpected mode. Defaulting to PRESET mode");
            subMode_ = SUB_MODE_PRESET;
            break;
        } // SWITCH
    }
    return true;
}

bool SparkModeManager::toggleLooperAppMode() {
    if (operationMode_ == SPARK_MODE_AMP) {
        Serial.println("In AMP mode, doing nothing.");
        return false;
    }

    SubMode newSubMode;
    Serial.print("Switching to ");

    switch (subMode_) {
    case SUB_MODE_PRESET:
    case SUB_MODE_FX:
        Serial.println("LOOPER mode");
        newSubMode = SUB_MODE_LOOPER;
        break;
    case SUB_MODE_LOOPER:
        Serial.println("PRESET mode");
        newSubMode = SUB_MODE_PRESET;
        break;
    case SUB_MODE_LOOP_CONTROL:
    case SUB_MODE_LOOP_CONFIG:
        Serial.println("PRESET mode");
        newSubMode = SUB_MODE_PRESET;
        break;
    default:
        Serial.println("Unexpected mode. Defaulting to PRESET mode");
        newSubMode = SUB_MODE_PRESET;
        break;
    }

    subMode_ = newSubMode;
    return true;
}

void SparkModeManager::toggleBTMode() {
    if (currentBTMode_ == BT_MODE_BLE) {
        currentBTMode_ = BT_MODE_SERIAL;
        DEBUG_PRINTLN("Setting BT mode to SERIAL");
    } else {
        currentBTMode_ = BT_MODE_BLE;
        DEBUG_PRINTLN("Setting BT mode to BLE");
    }
    saveBTModeToFile();
}

void SparkModeManager::readOpModeFromFile() {
    if (LittleFS.begin()) {
        if (LittleFS.exists(sparkModeFileName.c_str())) {
            File file = LittleFS.open(sparkModeFileName.c_str(), "r");
            if (file) {
                String fileContent = file.readString();
                file.close();

                if (fileContent == "APP") {
                    operationMode_ = SPARK_MODE_APP;
                    DEBUG_PRINTLN("Setting operation mode to APP from file");
                } else if (fileContent == "AMP") {
                    operationMode_ = SPARK_MODE_AMP;
                    DEBUG_PRINTLN("Setting operation mode to AMP from file");
                } else if (fileContent == "KEYBOARD") {
                    operationMode_ = SPARK_MODE_KEYBOARD;
                    DEBUG_PRINTLN("Setting operation mode to KEYBOARD from file");
                }
            }
        }
        LittleFS.end();
    }
}

void SparkModeManager::readBTModeFromFile() {
    if (LittleFS.begin()) {
        if (LittleFS.exists(btModeFileName.c_str())) {
            File file = LittleFS.open(btModeFileName.c_str(), "r");
            if (file) {
                String fileContent = file.readString();
                file.close();

                if (fileContent == "BLE") {
                    currentBTMode_ = BT_MODE_BLE;
                    DEBUG_PRINTLN("Setting BT mode to BLE from file");
                } else if (fileContent == "SERIAL") {
                    currentBTMode_ = BT_MODE_SERIAL;
                    DEBUG_PRINTLN("Setting BT mode to SERIAL from file");
                }
            }
        }
        LittleFS.end();
    }
}

void SparkModeManager::saveOpModeToFile() {
    if (LittleFS.begin()) {
        File file = LittleFS.open(sparkModeFileName.c_str(), "w");
        if (file) {
            switch (operationMode_) {
            case SPARK_MODE_APP:
                file.print("APP");
                break;
            case SPARK_MODE_AMP:
                file.print("AMP");
                break;
            case SPARK_MODE_KEYBOARD:
                file.print("KEYBOARD");
                break;
            }
            file.close();
            DEBUG_PRINTLN("Saved operation mode to file");
        }
        LittleFS.end();
    }
}

void SparkModeManager::saveBTModeToFile() {
    if (LittleFS.begin()) {
        File file = LittleFS.open(btModeFileName.c_str(), "w");
        if (file) {
            if (currentBTMode_ == BT_MODE_BLE) {
                file.print("BLE");
            } else {
                file.print("SERIAL");
            }
            file.close();
            DEBUG_PRINTLN("Saved BT mode to file");
        }
        LittleFS.end();
    }
}
