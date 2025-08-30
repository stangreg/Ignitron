/*
 * SparkModeManager.h
 *
 *  Created on: 27.08.2025
 *      Author: GitHub Copilot
 */

#ifndef SPARKMODEMANAGER_H_
#define SPARKMODEMANAGER_H_

/* *******************************************************************************
 * IMPORTANT: SparkModeManager Usage
 *
 * This class is the central place for managing all mode-related functionality.
 * To access SparkModeManager methods, get the instance from SparkDataControl:
 *
 *   SparkModeManager& modeManager = sparkDataControl.getModeManager();
 *
 * Then use direct calls to the modeManager:
 *   if (modeManager.operationMode() == SPARK_MODE_APP) {
 *     // ...
 *   }
 *
 *   modeManager.toggleSubMode();
 * *******************************************************************************/

#include "Config_Definitions.h"
#include "SparkTypes.h"
#include <Arduino.h>
#include <string>

using namespace std;

class SparkModeManager {
public:
    SparkModeManager();
    virtual ~SparkModeManager();

    /**
     * Initialize the mode manager with the provided operation mode
     * @param opModeInput Initial operation mode
     * @return The current operation mode after initialization
     */
    OperationMode init(OperationMode opModeInput);

    /**
     * Toggle between sub-modes (PRESET, FX, LOOP_CONTROL, LOOP_CONFIG)
     * @return True if successful, false otherwise
     */
    bool toggleSubMode();

    /**
     * Toggle between regular mode and looper app mode
     * @return True if successful, false otherwise
     */
    bool toggleLooperAppMode();

    /**
     * Switch to a specific sub-mode
     * @param subMode The sub-mode to switch to
     */
    void switchSubMode(SubMode subMode);

    /**
     * Toggle between BLE and Serial BT modes
     */
    void toggleBTMode();

    // Getters
    const OperationMode &operationMode() const { return operationMode_; }
    OperationMode &operationMode() { return operationMode_; }
    const SubMode &subMode() const { return subMode_; }
    SubMode &subMode() { return subMode_; }
    const BTMode currentBTMode() const { return currentBTMode_; }

    // Read modes from file system
    void resetOpModeFile();
    void readOpModeFromFile();
    void readBTModeFromFile();
    void saveOpModeToFile();
    void saveBTModeToFile();

private:
    OperationMode operationMode_ = SPARK_MODE_APP;
    SubMode subMode_ = SUB_MODE_PRESET;
    BTMode currentBTMode_ = BT_MODE_BLE;

    // File paths for configuration files
    string btModeFileName = "/config/BTMode.config";
    string sparkModeFileName = "/config/SparkMode.config";
};

#endif /* SPARKMODEMANAGER_H_ */
