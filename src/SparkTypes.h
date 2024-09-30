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

#define DIR_TO_SPARK 0
#define DIR_FROM_SPARK 1

struct keyboardKeyDefinition {
    // Quick reference from https://github.com/T-vK/ESP32-BLE-Keyboard/blob/master/BleKeyboard.h
    // KEY_LEFT_ARROW = 0xD8;
    // KEY_RIGHT_ARROW = 0xD7;

    // KEY_LEFT_CTRL = 0x80;
    // KEY_LEFT_SHIFT = 0x81;
    // KEY_LEFT_ALT = 0x82;

    uint8_t key_uid;  // UID for key 1 to 6 and 11 to 16 for long press
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

    int number;
    string special;
    float value;
};

struct Pedal {

    string name;
    boolean isOn;
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
        filler = 0x00;
        isEmpty = true;
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
    byte filler;
};

struct CmdData {
    byte cmd;
    byte subcmd;
    ByteVector data;

    string toString() {
        string cmdStr;
        cmdStr = "[" + SparkHelper::intToHex(cmd) + "], [" + SparkHelper::intToHex(subcmd) + "], [";
        for (byte by : data) {
            cmdStr += SparkHelper::intToHex(by).c_str();
        }
        cmdStr += "]";
        return cmdStr;
    }
};

struct AckData {
    byte msg_num;
    byte cmd;
    byte subcmd;
};

struct LooperSetting {

    int barsConfig[7] = {1, 2, 4, 8, 12, 16, -1};
    int sizeOfBarsConfig = sizeof(barsConfig) / sizeof(int);

    bool changePending = false;

    int bpm = 120;
    string count_str = "Straight";
    byte count = 0x04;
    int bars = 4;
    int barsIndex = 2;
    bool free_indicator = false;
    bool click = true;

    bool unknown_onoff = false;
    unsigned int max_duration = 60000;

    string json;
    string text;
    string raw;

    void reset() {
        bpm = 120;
        count = 0x04;
        count_str = "Straight";
        barsIndex = 2;
        bars = barsConfig[barsIndex];
        free_indicator = false;
        click = true;
        unknown_onoff = false;
        max_duration = 60000;

        changePending = true;
    }

    void cycleBars() {
        barsIndex = (barsIndex + 1) % sizeOfBarsConfig;
        bars = barsConfig[barsIndex];
        free_indicator = (bars == -1) ? true : false;
        if (free_indicator) {
            bars = bpm / 4;
        }
        changePending = true;
    }

    void toggleCount() {
        if (count == 0x04) {
            count = 0x03;
            count_str = "Shuffle";
        } else {
            count = 0x04;
            count_str = "Straight";
        }
        changePending = true;
    }

    void toggleClick() {
        click = !(click);
        changePending = true;
    }

    void setBpm(int _bpm) {
        bpm = _bpm;
        if (free_indicator) {
            bars = bpm / 4;
        }
        changePending = true;
    }

    string getJson() const {
        StringBuilder sb;
        sb.start_str();
        sb.add_int("BPM", bpm);
        sb.add_separator();
        sb.add_str("Count", count_str);
        sb.add_separator();
        sb.add_int("Bars", bars);
        sb.add_separator();
        sb.add_bool("Free", free_indicator);
        sb.add_separator();
        sb.add_bool("Click", click);
        sb.add_separator();
        sb.add_bool("Unknown switch", unknown_onoff);
        sb.add_separator();
        sb.add_int("Max duration", max_duration);
        sb.end_str();

        return sb.getJson();
    }
};

#endif
