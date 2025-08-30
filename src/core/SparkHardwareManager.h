/*
 * SparkHardwareManager.h
 *
 *  Created on: 30.08.2025
 *      Author: GitHub Copilot
 */

#ifndef SPARKHARDWAREMANAGER_H_
#define SPARKHARDWAREMANAGER_H_

#include <Arduino.h>
// Forward declarations instead of includes
class SparkBTControl;
class SparkBLEKeyboard;
class SparkDisplayControl;
class SparkLEDControl;
class NimBLERemoteCharacteristic;

#include "SparkTypes.h"

// We'll use the library's definition of notify_callback
// Instead of defining our own, we'll use the full function pointer type

class SparkDataControl; // Forward declaration

class SparkHardwareManager {
public:
    /**
     * Get the singleton instance of SparkHardwareManager
     * @return Reference to the SparkHardwareManager instance
     */
    static SparkHardwareManager &getInstance();

    /**
     * Initialize hardware components for the given operation mode
     * @param opMode The operation mode (APP, AMP, KEYBOARD)
     * @param dataControl Pointer to the SparkDataControl instance
     * @return The operation mode after initialization
     */
    OperationMode initializeHardware(OperationMode opMode, SparkDataControl *dataControl);

    /**
     * Initialize the Bluetooth interface
     * @param callback The notification callback function
     */
    void initializeBluetooth(void (*notificationCallback)(NimBLERemoteCharacteristic *, uint8_t *, size_t, bool));

    /**
     * Initialize the BLE keyboard
     * @param keyboardName The name for the BLE keyboard
     */
    void initializeKeyboard(const std::string &keyboardName = "Ignitron BLE");

    /**
     * Start the BLE server for AMP mode
     */
    void startBLEServer();

    /**
     * Check BLE connection to Spark Amp
     * @return True if connected or connection established successfully
     */
    bool checkBLEConnection();

    /**
     * Check if Amp is connected
     * @return True if connected
     */
    static bool isAmpConnected();

    /**
     * Check if App is connected (ESP in AMP mode and client is connected)
     * @return True if connected
     */
    static bool isAppConnected();

    /**
     * Get the BLE Control instance
     * @return Pointer to the SparkBTControl instance
     */
    static SparkBTControl *getBleControl();

    /**
     * Get the BLE Keyboard instance
     * @return Pointer to the SparkBLEKeyboard instance
     */
    static SparkBLEKeyboard *getBleKeyboard();

private:
    SparkHardwareManager(); // Private constructor for singleton
    ~SparkHardwareManager();

    // Disable copy constructor and assignment operator
    SparkHardwareManager(const SparkHardwareManager &) = delete;
    SparkHardwareManager &operator=(const SparkHardwareManager &) = delete;

    // MAC address for BLE keyboard
    uint8_t macKeyboard[6] = {0xB4, 0xE6, 0x2D, 0xB2, 0x1B, 0x36};

    // Reference to other components
    static SparkBTControl *bleControl;
    static SparkBLEKeyboard *bleKeyboard;
};

#endif /* SPARKHARDWAREMANAGER_H_ */
