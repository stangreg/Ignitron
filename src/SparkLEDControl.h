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
    // SparkDataControl* dataControl() {return sparkDC;}
    void setDataControl(SparkDataControl *dc) {
        sparkDC = dc;
    }

private:
    void init();
    SparkDataControl *sparkDC;
    KeyboardMapping mapping;

    OperationMode operationMode = SPARK_MODE_APP;
    Preset activePreset;
    int activePresetNum = 1;

    // For blinking mode
    int ledState = LOW; // ledState used to set the LED
    // Generally, you should use "unsigned long" for variables that hold time
    // The value will quickly become too large for an int to store
    unsigned long previousMillis = 0; // will store last time LED was updated
    // constants won't change:
    const long blinkInterval_ms = 200; // blinkInterval_ms at which to blink (milliseconds)
    long tapBlinkInterval_ms = 250;
    bool blinkInvert = false; // Remember state of blink inversion

    void updateLedAppPresetMode();
    void updateLedAppFXMode();
    void updateLedAmp();
    void updateLedKeyboard();
    void updateLedLooperMode();
    void updateLedTuner();

    void allLedOff();
    void switchLed(int num, bool on = true, bool fxMode = false);
};

#endif /* SPARKLEDCONTROL_H_ */
