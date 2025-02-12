/*
 * SparkLEDControl.h
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#ifndef SPARKLEDCONTROL_H_
#define SPARKLEDCONTROL_H_

#include "Config_Definitions.h"
#include "SparkDataControl.h"
#include "SparkLooperControl.h"
#include "SparkPresetControl.h"
#include <Arduino.h>

using namespace std;

class SparkLEDControl {
public:
    SparkLEDControl();
    SparkLEDControl(SparkDataControl *dc);
    virtual ~SparkLEDControl();

    void updateLEDs();
    // SparkDataControl* dataControl() {return spark_dc;}
    void setDataControl(SparkDataControl *dc) {
        spark_dc = dc;
    }

private:
    void init();
    SparkDataControl *spark_dc;
    KeyboardMapping mapping;

    int operationMode = SPARK_MODE_APP;
    Preset activePreset;
    int activePresetNum = 1;

    // For blinking mode
    int ledState = LOW; // ledState used to set the LED
    // Generally, you should use "unsigned long" for variables that hold time
    // The value will quickly become too large for an int to store
    unsigned long previousMillis = 0; // will store last time LED was updated
    // constants won't change:
    const long blinkInterval_ms = 200; // blinkInterval_ms at which to blink (milliseconds)

    long tap_blinkInterval_ms = 250;

    void updateLED_APP_PresetMode();
    void updateLED_APP_FXMode();
    void updateLED_AMP();
    void updateLED_KEYBOARD();
    void updateLED_LooperMode();
    void updateLED_TUNER();

    void allLedOff();
    void switchLED(int num, bool on = true, bool fxMode = false);
};

#endif /* SPARKLEDCONTROL_H_ */
