/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARK_TYPES_H
#define SPARK_TYPES_H

#include "Config_Definitions.h"
#include "SparkHelper.h"
#include "StringBuilder.h"
#include <Arduino.h>
#include <array>
#include <vector>

using namespace std;
using ByteVector = vector<byte>;

enum MessageDirection {
    DIR_TO_SPARK,
    DIR_FROM_SPARK
};

struct keyboardKeyDefinition {
    // Quick reference from https://github.com/T-vK/ESP32-BLE-Keyboard/blob/master/BleKeyboard.h
    // KEY_LEFT_ARROW = 0xD8;
    // KEY_RIGHT_ARROW = 0xD7;

    // KEY_LEFT_CTRL = 0x80;
    // KEY_LEFT_SHIFT = 0x81;
    // KEY_LEFT_ALT = 0x82;

    uint8_t keyUid;   // UID for key 1 to 6 and 11 to 16 for long press
    uint8_t key;      // see key code in BleKeyboard.h
    uint8_t modifier; // 0: no mod / 0x80 to 0x87 in BleKeyboard.h
    uint8_t repeat;   // 0: no repeat, just once / >0 repeat n times

    string display; // what to display in LCD
};

struct KeyboardMapping {

    // int mappingSize = 6;
    string mappingName;
    // Mapping for short press
    vector<keyboardKeyDefinition> keyboardShortPress;
    // Mapping for long press
    vector<keyboardKeyDefinition> keyboardLongPress;

    int indexOfKey(uint8_t ki) {

        int index = (ki < 10) ? ki : ki - 10;

        return index;
    }
};

struct Parameter {

    int number = 0;
    string special = "";
    float value = 0.0;
};

struct Pedal {

    string name = "";
    boolean isOn = false;
    vector<Parameter> parameters;
};

struct Preset {

    boolean isEmpty = true;
    static const int numberOfPedals = 7;

    Preset() {
        json = raw = text = "";
        uuid = name = "";
        version = description = icon = "";
        bpm = 0.0;
        checksum = 0x00;
        isEmpty = true;
    }

    bool isEqual(Preset *otherPreset) {
        string compareStringThis = name + uuid;
        string compareStringOther = otherPreset->name + otherPreset->uuid;

        if (compareStringThis.compare(compareStringOther) == 0) {
            return true;
        }
        return false;
    }

    string json;
    // Raw and text might not be filled (when read from the stored presets).
    string raw;
    string text;

    int presetNumber = -1;
    string uuid;
    string name;
    string version;
    string description;
    string icon;
    float bpm;
    vector<Pedal> pedals;
    byte checksum;
};

struct CmdData {
    byte msgNum = 0x00;
    byte cmd = 0x00;
    byte subcmd = 0x00;
    ByteVector data = {};
    byte detail = 0;

    string toString() {
        string cmdStr;
        cmdStr = "[" + SparkHelper::intToHex(cmd) + "], [" + SparkHelper::intToHex(subcmd) + "], [" + SparkHelper::intToHex(detail) + "], [";
        for (byte by : data) {
            cmdStr += SparkHelper::intToHex(by).c_str();
        }
        cmdStr += "]";
        return cmdStr;
    }
};

struct AckData {
    byte msgNum = 0x00;
    byte cmd = 0x00;
    byte subcmd = 0x00;
    byte detail = 0x00;
};

struct LooperSetting {

    int barsConfig[7] = {1, 2, 4, 8, 12, 16, -1};
    int sizeOfBarsConfig = sizeof(barsConfig) / sizeof(int);

    bool changePending = false;

    int bpm = 120;
    string countStr = "Straight";
    byte count = 0x04;
    int bars = 4;
    int barsIndex = 2;
    bool freeIndicator = false;
    bool click = true;

    bool unknownOnOff = false;
    unsigned int maxDuration = 60000;

    string json = "";
    string text = "";
    string raw = "";

    void reset() {
        bpm = 120;
        count = 0x04;
        countStr = "Straight";
        barsIndex = 2;
        bars = barsConfig[barsIndex];
        freeIndicator = false;
        click = true;
        unknownOnOff = false;
        maxDuration = 60000;

        changePending = true;
    }

    void cycleBars() {
        barsIndex = (barsIndex + 1) % sizeOfBarsConfig;
        bars = barsConfig[barsIndex];
        freeIndicator = (bars == -1) ? true : false;
        if (freeIndicator) {
            bars = bpm / 4;
        }
        changePending = true;
    }

    void toggleCount() {
        if (count == 0x04) {
            count = 0x03;
            countStr = "Shuffle";
        } else {
            count = 0x04;
            countStr = "Straight";
        }
        changePending = true;
    }

    void toggleClick() {
        click = !(click);
        changePending = true;
    }

    void setBpm(int _bpm) {
        bpm = _bpm;
        if (freeIndicator) {
            bars = bpm / 4;
        }
        changePending = true;
    }

    string getJson() const {
        StringBuilder sb;
        sb.startStr();
        sb.addInt("BPM", bpm);
        sb.addSeparator();
        sb.addStr("Count", countStr);
        sb.addSeparator();
        sb.addInt("Bars", bars);
        sb.addSeparator();
        sb.addBool("Free", freeIndicator);
        sb.addSeparator();
        sb.addBool("Click", click);
        sb.addSeparator();
        sb.addBool("Unknown switch", unknownOnOff);
        sb.addSeparator();
        sb.addInt("Max duration", maxDuration);
        sb.endStr();

        return sb.getJson();
    }
};

#endif
