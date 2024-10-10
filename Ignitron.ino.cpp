# 1 "C:\\Users\\mikes\\AppData\\Local\\Temp\\tmpdbq7nlg7"
#include <Arduino.h>
# 1 "C:/Users/mikes/Projects/Ignitron/Ignitron.ino"


#include <Arduino.h>
#include <NimBLEDevice.h>
#include <SPI.h>
#include <Wire.h>
#include <string>

#include "src/SparkButtonHandler.h"
#include "src/SparkDataControl.h"
#include "src/SparkDisplayControl.h"
#include "src/SparkLEDControl.h"

using namespace std;


const string DEVICE_NAME = "Ignitron";


SparkDataControl spark_dc;
SparkButtonHandler spark_bh;
SparkLEDControl spark_led;
SparkDisplayControl spark_display;

unsigned long lastInitialPresetTimestamp = 0;
unsigned long currentTimestamp = 0;
int initialRequestInterval = 3000;


bool isInitBoot;
int operationMode = SPARK_MODE_APP;
void setup();
void loop();
#line 39 "C:/Users/mikes/Projects/Ignitron/Ignitron.ino"
void setup() {

    Serial.begin(115200);
    while (!Serial)
        ;

    Serial.println("Initializing");

    spark_bh.setDataControl(&spark_dc);
    operationMode = spark_bh.checkBootOperationMode();


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

    spark_bh.setDataControl(&spark_dc);

    spark_led.setDataControl(&spark_dc);

    Serial.println("Initialization done.");
}

void loop() {


    if (operationMode == SPARK_MODE_APP || operationMode == SPARK_MODE_LOOPER) {
        while (!(spark_dc.checkBLEConnection())) {
            spark_display.update(spark_dc.isInitBoot());
            spark_bh.readButtons();
        }




        if (spark_dc.isInitBoot()) {



            DEBUG_PRINTLN("Trying to communicate");
            spark_dc.getAmpName();

            spark_dc.switchPreset(1, true);

            spark_dc.isInitBoot() = false;
        }
    }


    if (operationMode != SPARK_MODE_KEYBOARD) {
        spark_dc.checkForUpdates();
    }

    spark_bh.readButtons();

    spark_led.updateLEDs();

    spark_display.update();
}