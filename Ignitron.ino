

#include <Arduino.h>
#include <NimBLEDevice.h> // github NimBLE
#include <SPI.h>
#include <Wire.h>
#include <string>

#include "src/SparkButtonHandler.h"
#include "src/SparkDataControl.h"
#include "src/SparkDisplayControl.h"
#include "src/SparkLEDControl.h"
#include "src/SparkPresetControl.h"

using namespace std;

// Device Info Definitions
const string DEVICE_NAME = "Ignitron";

// Control classes
SparkDataControl spark_dc;
SparkButtonHandler spark_bh;
SparkLEDControl spark_led;
SparkDisplayControl spark_display;
SparkPresetControl &presetControl = SparkPresetControl::getInstance();

unsigned long lastInitialPresetTimestamp = 0;
unsigned long currentTimestamp = 0;
int initialRequestInterval = 3000;

// Check for initial boot
bool isInitBoot;
int operationMode = SPARK_MODE_APP;

/////////////////////////////////////////////////////////
//
// INIT AND RUN
//
/////////////////////////////////////////////////////////

void setup() {

    Serial.begin(115200);
    while (!Serial)
        ;

    Serial.println("Initializing");
    // spark_dc = new SparkDataControl();
    spark_bh.setDataControl(&spark_dc);
    operationMode = spark_bh.checkBootOperationMode();

    // Setting operation mode before initializing
    operationMode = spark_dc.init(operationMode);
    spark_bh.configureButtons();
    Serial.printf("Operation mode: %d\n", operationMode);

    switch (operationMode) {
    case SPARK_MODE_APP:
        Serial.println("======= Entering APP mode =======");
        break;
    case SPARK_MODE_AMP:
        Serial.println("======= Entering AMP mode =======");
        break;
    case SPARK_MODE_LOOPER:
        Serial.println("======= Entering Looper mode =======");
        break;
    case SPARK_MODE_KEYBOARD:
        Serial.println("======= Entering Keyboard mode =======");
        break;
    }

    spark_display.setDataControl(&spark_dc);
    spark_dc.setDisplayControl(&spark_display);
    spark_display.init(operationMode);
    // Assigning data control to buttons;
    spark_bh.setDataControl(&spark_dc);
    // Initializing control classes
    spark_led.setDataControl(&spark_dc);

    Serial.println("Initialization done.");
}

void loop() {

    // Methods to call only in APP mode
    if (operationMode == SPARK_MODE_APP || operationMode == SPARK_MODE_LOOPER) {
        while (!(spark_dc.checkBLEConnection())) {
            spark_display.update(spark_dc.isInitBoot());
            spark_bh.readButtons();
        }

        // After connection is established, continue.
        //  On first boot, get the amp type and set the preset to Hardware setting 1.

        if (spark_dc.isInitBoot()) { // && !spark_dc.isInitHWRead()) {
            // This is only done once after the connection has been established
            // Read AMP name to determine special parameters
            // TEST: spark_dc.getSerialNumber();
            spark_dc.getAmpName();
            DEBUG_PRINTLN("Trying to communicate");
            while (!(presetControl.allHWPresetsAvailable())) {
                spark_dc.checkForUpdates();
                // Reading button input
                spark_bh.readButtons();
                // Update LED status
                // spark_led.updateLEDs();
                // Update display
                spark_display.update(spark_dc.isInitBoot());
            }
            // DEBUG_PRINTLN("Initial boot, setting preset to HW 1");
            // spark_dc.switchPreset(spark_dc.pendingPresetNum(), true);
            spark_dc.getCurrentPresetFromSpark();
            // spark_dc.getCurrentPresetNum();
            spark_dc.isInitBoot() = false;
            // spark_dc.configureLooper();
        }
    }

    // Check if presets have been updated (not needed in Keyboard mode)
    if (operationMode != SPARK_MODE_KEYBOARD) {
        spark_dc.checkForUpdates();
    }
    // Reading button input
    spark_bh.readButtons();
    // Update LED status
    spark_led.updateLEDs();
    // Update display
    spark_display.update();
}
