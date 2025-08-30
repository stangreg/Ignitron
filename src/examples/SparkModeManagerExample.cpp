/*
 * Example.cpp - Demonstrates how to use SparkModeManager correctly
 *
 * This file serves as an example to show the correct way to interact with SparkModeManager
 * It is not meant to be compiled or used in production.
 */

#include "core/SparkDataControl.h"
#include "core/SparkModeManager.h"

// Example class that needs to interact with mode management
class ExampleClass {
private:
    SparkDataControl &sparkDataControl;

public:
    ExampleClass(SparkDataControl &dataControl) : sparkDataControl(dataControl) {}

    void exampleMethod() {
        // CORRECT: Get a reference to the SparkModeManager
        SparkModeManager &modeManager = sparkDataControl.getModeManager();

        // CORRECT: Use direct access to SparkModeManager methods
        if (modeManager.operationMode() == SPARK_MODE_APP) {
            // App mode logic
            if (modeManager.subMode() == SUB_MODE_PRESET) {
                // Handle preset mode
            }

            // Switch to another sub-mode
            modeManager.toggleSubMode();
        } else if (modeManager.operationMode() == SPARK_MODE_KEYBOARD) {
            // Keyboard mode logic
            modeManager.toggleBTMode();
        }

        // DEPRECATED: Do not use these methods on SparkDataControl
        // if (sparkDataControl.operationMode() == SPARK_MODE_APP) // Wrong!
        // sparkDataControl.toggleSubMode(); // Wrong!
    }
};
