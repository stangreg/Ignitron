#define CONFIG_LITTLEFS_SPIFFS_COMPAT

#include <Arduino.h>
#include <LittleFS.h>
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
SparkDisplayControl sparkDisplay;
SparkPresetControl &presetControl = SparkPresetControl::getInstance();

unsigned long lastInitialPresetTimestamp = 0;
unsigned long currentTimestamp = 0;
int initialRequestInterval = 3000;

// Check for initial boot
bool isInitBoot;
OperationMode operationMode = SPARK_MODE_APP;

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
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount failed");
        return;
    }
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
    case SPARK_MODE_KEYBOARD:
        Serial.println("======= Entering Keyboard mode =======");
        break;
    }

    sparkDisplay.setDataControl(&spark_dc);
    spark_dc.setDisplayControl(&sparkDisplay);
    sparkDisplay.init(operationMode);
    // Assigning data control to buttons;
    spark_bh.setDataControl(&spark_dc);
    // Initializing control classes
    spark_led.setDataControl(&spark_dc);

    Serial.println("Initialization done.");
}

void loop() {

    // Methods to call only in APP mode
    if (operationMode == SPARK_MODE_APP) {
        while (!(spark_dc.checkBLEConnection())) {
            sparkDisplay.update(spark_dc.isInitBoot());
            spark_led.updateLEDs();
            spark_bh.readButtons();
        }

        // After connection is established, continue.
        //  On first boot, get the amp type and set the preset to Hardware setting 1.

        if (spark_dc.isInitBoot()) { // && !spark_dc.isInitHWRead()) {
            // This is only done once after the connection has been established
            // Read AMP name to determine special parameters
            spark_dc.getSerialNumber();
            // spark_dc.getAmpName();
            //  delay(100);
            //  spark_dc.getCurrentPresetFromSpark();
            spark_dc.isInitBoot() = false;
            // spark_dc.configureLooper();
        }
    }

    // Check if presets have been updated (not needed in Keyboard mode)
    if (operationMode != SPARK_MODE_KEYBOARD) {
        spark_dc.checkForUpdates();
    }
    // Reading button input
    spark_bh.configureButtons();
    spark_bh.readButtons();
#ifdef ENABLE_BATTERY_STATUS_INDICATOR
    // Update battery level
    spark_dc.updateBatteryLevel();
#endif
    // test ignitron preset tools
    handleSerialCommands();
    // Update LED status
    spark_led.updateLEDs();
    // Update display
    sparkDisplay.update();
}

// === BEGIN: LISTPRESETS serial support =======================================

// Case-insensitive “.json” check
static bool hasJsonExt(const char *name) {
    if (!name)
        return false;
    size_t len = strlen(name);
    if (len < 5)
        return false;
    const char *ext = name + (len - 5);
    return ext[0] == '.' &&
           (ext[1] == 'j' || ext[1] == 'J') &&
           (ext[2] == 's' || ext[2] == 'S') &&
           (ext[3] == 'o' || ext[3] == 'O') &&
           (ext[4] == 'n' || ext[4] == 'N');
}

// Dump entire JSON file to a single line (removes CR/LF/TAB)
static void printJsonFileSingleLine(File &f) {
    Serial.print("JSON STRING: ");
    while (f.available()) {
        char c = (char)f.read();
        if (c == '\r' || c == '\n' || c == '\t')
            continue;
        Serial.write(c);
    }
    Serial.println();
}

// List every *.json at the LittleFS root
static void listAllPresets() {
    Serial.println("LISTPRESETS_START");

    File root = LittleFS.open("/");
    if (!root) {
        Serial.println("⚠️ Could not open LittleFS root");
        Serial.println("LISTPRESETS_DONE");
        return;
    }

    while (true) {
        File f = root.openNextFile();
        if (!f)
            break;

        if (!f.isDirectory()) {
            const char *name = f.name();
            if (name && hasJsonExt(name)) {
                Serial.print("Reading preset filename: ");
                Serial.println(name);
                printJsonFileSingleLine(f);
            }
        }
        f.close();
    }

    Serial.println("LISTPRESETS_DONE");
}

// Robust line-buffered serial command reader
static void handleSerialCommands() {
    static String buf;

    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\r')
            continue;

        if (c == '\n') {
            String cmd = buf;
            buf = "";
            cmd.trim();
            if (cmd.length() == 0)
                return;

            String u = cmd;
            u.toUpperCase();

            if (u == "LISTPRESETS") {
                listAllPresets();
            }
            if (u == "LISTBANKS") {
                File f = LittleFS.open("/PresetList.txt");
                if (f) {
                    Serial.println("LISTBANKS_START");
                    while (f.available()) {
                        char c = f.read();
                        if (c == '\r')
                            continue;
                        Serial.write(c);
                    }
                    Serial.println("LISTBANKS_DONE");
                    f.close();
                } else {
                    Serial.println("⚠️ PresetList.txt not found");
                    Serial.println("LISTBANKS_DONE");
                }
            }
        } else {
            buf += c;
            if (buf.length() > 256) {
                buf.remove(0, buf.length() - 256);
            }
        }
    }
}

// === END: LISTPRESETS serial support =========================================
