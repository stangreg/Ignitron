/*
 * SparkHardwareManager.cpp
 *
 *  Created on: 30.08.2025
 *      Author: GitHub Copilot
 */

#include "SparkHardwareManager.h"

// Include NimBLEDevice.h first to get the library's definition of notify_callback
#include <NimBLEDevice.h>

// Include the other headers after
#include "../communication/SparkBTControl.h"
#include "../hardware/SparkBLEKeyboard.h"
#include "../hardware/SparkDisplayControl.h"
#include "../hardware/SparkLEDControl.h"
#include "SparkDataControl.h"

// Initialize static members
SparkBTControl *SparkHardwareManager::bleControl = nullptr;
SparkBLEKeyboard SparkHardwareManager::bleKeyboard = SparkBLEKeyboard();

SparkHardwareManager &SparkHardwareManager::getInstance() {
    static SparkHardwareManager instance;
    return instance;
}

SparkHardwareManager::SparkHardwareManager() {
    bleControl = new SparkBTControl();
}

SparkHardwareManager::~SparkHardwareManager() {
    // Cleanup, if necessary
    delete bleControl;
}

OperationMode SparkHardwareManager::initializeHardware(OperationMode opMode, SparkDataControl *dataControl) {

    switch (opMode) {
    case SPARK_MODE_APP:
        initializeKeyboard("Ignitron BLE");
        // Short delay to initialize then stop to save power
        delay(100);
        bleKeyboard.end();

        // Initialize BLE client for connecting to Spark Amp
        bleControl->initBLE(SparkDataControl::bleNotificationCallback);
        DEBUG_PRINTLN("SparkHardwareManager: Hardware initialized for APP mode");
        break;

    case SPARK_MODE_AMP:
        if (dataControl->getModeManager().currentBTMode() == BT_MODE_SPARK_BLE) {
            bleControl->startServer();
        } else if (dataControl->getModeManager().currentBTMode() == BT_MODE_SPARK_SERIAL) {
            bleControl->startBTSerial();
        }
        DEBUG_PRINTLN("SparkHardwareManager: Hardware initialized for AMP mode");
        break;

    case SPARK_MODE_KEYBOARD:
        initializeKeyboard("Ignitron BLE");
        DEBUG_PRINTLN("SparkHardwareManager: Hardware initialized for KEYBOARD mode");
        break;
    }

    if (!LittleFS.begin(true, "/littlefs", 10U, "spiffs")) {
        DEBUG_PRINTLN("SparkHardwareManager: Failed to mount LittleFS");
    } else {
        DEBUG_PRINTLN("SparkHardwareManager: LittleFS mounted successfully");
    }
    return opMode;
}

void SparkHardwareManager::initializeBluetooth(void (*notificationCallback)(NimBLERemoteCharacteristic *, uint8_t *, size_t, bool)) {
    bleControl->initBLE();
    if (notificationCallback) {
        bleControl->subscribeToNotifications(notificationCallback);
    }
}

void SparkHardwareManager::initializeKeyboard(const std::string &keyboardName) {
    // Set MAC address for BLE keyboard
    // esp_base_mac_addr_set(&macKeyboard[0]);

    // Initialize BLE
    Serial.print("Initializing Keyboard...");
    bleKeyboard.setName(keyboardName.c_str());
    bleKeyboard.begin();
    Serial.println("done");
}

void SparkHardwareManager::startBLEServer() {
    bleControl->startServer();
}

bool SparkHardwareManager::checkBLEConnection() {
    if (bleControl->isAmpConnected()) {
        return true;
    }
    if (bleControl->isConnectionFound()) {
        if (bleControl->connectToServer()) {
            // Use the callback from SparkDataControl directly
            bleControl->subscribeToNotifications(SparkDataControl::bleNotificationCallback);
            Serial.println("BLE connection to Spark established.");
            return true;
        } else {
            Serial.println("Failed to connect, starting scan");
            bleControl->startScan();
            return false;
        }
    }
    return false;
}

SparkBTControl *SparkHardwareManager::getBleControl() {
    return bleControl;
}

SparkBLEKeyboard &SparkHardwareManager::getBleKeyboard() {
    return bleKeyboard;
}

bool SparkHardwareManager::isAmpConnected() {
    return bleControl != nullptr && bleControl->isAmpConnected();
}

bool SparkHardwareManager::isAppConnected() {
    return bleControl != nullptr && bleControl->isAppConnected();
}
