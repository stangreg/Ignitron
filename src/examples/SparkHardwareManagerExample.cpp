/*
 * SparkHardwareManagerExample.cpp
 *
 * Example of how to use the SparkHardwareManager for hardware access
 *
 * Created on: 30.08.2025
 * Author: GitHub Copilot
 */

#include "../core/SparkDataControl.h"
#include "../core/SparkHardwareManager.h"
#include <Arduino.h>

/**
 * Example function showing how to access BLE functionality
 */
void exampleBLEAccess() {
    // Get access to the BLE control instance
    SparkBTControl *bleControl = SparkHardwareManager::getInstance().getBleControl();

    // Check if the amp is connected
    if (SparkHardwareManager::isAmpConnected()) {
        Serial.println("Amp is connected!");

        // Create a message to send to the amp
        std::vector<byte> message;
        // ...fill message with data...

        // Send the message to the amp
        bleControl->writeBLE(message, false);
    } else {
        // Check BLE connection - will attempt to connect if not connected
        bool connected = SparkHardwareManager::getInstance().checkBLEConnection();
        if (connected) {
            Serial.println("Connected to amp successfully!");
        } else {
            Serial.println("Failed to connect to amp.");
        }
    }
}

/**
 * Example function showing how to access keyboard functionality
 */
void exampleKeyboardAccess() {
    // Get access to the BLE keyboard instance
    SparkBLEKeyboard *keyboard = SparkHardwareManager::getInstance().getBleKeyboard();

    // Check if the keyboard is connected
    if (keyboard->isConnected()) {
        Serial.println("Keyboard is connected!");

        // Send a key press
        keyboard->press(KEY_LEFT_CTRL);
        keyboard->press('c');
        delay(100);
        keyboard->releaseAll();

        Serial.println("Sent Ctrl+C to connected device");
    } else {
        Serial.println("Keyboard is not connected");

        // Initialize the keyboard if needed
        SparkHardwareManager::getInstance().initializeKeyboard("Ignitron Keyboard");
    }
}

/**
 * Example function showing how to initialize hardware based on operation mode
 */
void exampleHardwareInitialization(SparkDataControl *dataControl) {
    // Get the current operation mode from the mode manager
    SparkModeManager &modeManager = dataControl->getModeManager();
    OperationMode currentMode = modeManager.operationMode();

    // Initialize hardware based on the operation mode
    SparkHardwareManager::getInstance().initializeHardware(currentMode, dataControl);

    // Mode-specific hardware actions
    switch (currentMode) {
    case SPARK_MODE_APP:
        Serial.println("Hardware initialized for APP mode");
        break;

    case SPARK_MODE_AMP:
        Serial.println("Hardware initialized for AMP mode");
        // Start the BLE server in AMP mode
        SparkHardwareManager::getInstance().startBLEServer();
        break;

    case SPARK_MODE_KEYBOARD:
        Serial.println("Hardware initialized for KEYBOARD mode");
        break;
    }
}
