#ifndef SPARKCURRENTSTATUS_H
#define SPARKCURRENTSTATUS_H

#include "SparkTypes.h"

const int MSG_TYPE_PRESET = 1;
const int MSG_TYPE_HWPRESET = 2;
const int MSG_TYPE_FX_ONOFF = 3;
const int MSG_TYPE_FX_CHANGE = 4;
const int MSG_TYPE_FX_PARAM = 5;
const int MSG_TYPE_AMP_NAME = 6;
const int MSG_TYPE_LOOPER_SETTING = 7;
const int MSG_TYPE_TAP_TEMPO = 8;
const int MSG_TYPE_MEASURE = 9;
const int MSG_TYPE_LOOPER_COMMAND = 10;
const int MSG_TYPE_LOOPER_STATUS = 11;
const int MSG_TYPE_TUNER_OUTPUT = 12;
const int MSG_TYPE_TUNER_ON = 13;
const int MSG_TYPE_TUNER_OFF = 14;
const int MSG_TYPE_HWCHECKSUM = 15;
const int MSG_TYPE_HWCHECKSUM_EXT = 16;
const int MSG_TYPE_AMPSTATUS = 17;

const int MSG_REQ_SERIAL = 21;
const int MSG_REQ_FW_VER = 22;
const int MSG_REQ_PRESET_CHK = 23;
const int MSG_REQ_CURR_PRESET_NUM = 24;
const int MSG_REQ_CURR_PRESET = 25;
const int MSG_REQ_AMP_STATUS = 26;
const int MSG_REQ_72 = 27;
const int MSG_REQ_PRESET1 = 28;
const int MSG_REQ_PRESET2 = 29;
const int MSG_REQ_PRESET3 = 30;
const int MSG_REQ_PRESET4 = 31;
const int MSG_REQ_INVALID = 99;

class SparkStatus {
public:
    static SparkStatus &getInstance();
    ~SparkStatus();

    SparkStatus(const SparkStatus &) = delete; // Disable copy constructor
    SparkStatus &operator=(const SparkStatus &) = delete;

    // Preset related methods to make information public
    const Preset currentPreset() const { return currentPreset_; }
    Preset &currentPreset() { return currentPreset_; }

    const int currentPresetNumber() const { return currentPresetNumber_; }
    int &currentPresetNumber() { return currentPresetNumber_; }

    const boolean isPresetNumberUpdated() const { return isPresetNumberUpdated_; }
    boolean &isPresetNumberUpdated() { return isPresetNumberUpdated_; }

    const boolean isPresetUpdated() const { return isPresetUpdated_; }
    boolean &isPresetUpdated() { return isPresetUpdated_; }

    const boolean isLooperSettingUpdated() const { return isLooperSettingUpdated_; }
    boolean &isLooperSettingUpdated() { return isLooperSettingUpdated_; }

    const LooperSetting currentLooperSetting() const { return looperSetting_; }
    LooperSetting &currentLooperSetting() { return looperSetting_; }

    const Pedal currentEffect() const { return currentEffect_; }
    Pedal &currentEffect() { return currentEffect_; }

    const boolean isEffectUpdated() const { return isEffectUpdated_; }
    boolean &isEffectUpdated() { return isEffectUpdated_; }

    const byte lastLooperCommand() const { return lastLooperCommand_; }
    byte &lastLooperCommand() { return lastLooperCommand_; }

    const int numberOfLoops() const { return numberOfLoops_; }
    int &numberOfLoops() { return numberOfLoops_; }

    const int lastMessageType() const { return lastMessageType_; }
    int &lastMessageType() { return lastMessageType_; }

    const byte lastMessageNum() const { return lastMessageNum_; }
    byte &lastMessageNum() { return lastMessageNum_; }

    const string ampName() const { return ampName_; }
    string &ampName() { return ampName_; }

    const BatteryLevel ampBatteryLevel() const { return ampBatteryLevel_; }
    BatteryLevel &ampBatteryLevel() { return ampBatteryLevel_; }

    const bool isAmpBatteryPowered() const { return isAmpBatteryPowered_; }
    bool &isAmpBatteryPowered() { return isAmpBatteryPowered_; }

    const int ampBatteryChargingStatus() const { return ampBatteryChargingStatus_; }
    BatteryChargingStatus &ampBatteryChargingStatus() { return ampBatteryChargingStatus_; }

    const float measure() const { return measure_; }
    float &measure() { return measure_; }

    const string noteString() const {
        if (note_ == 0x0e)
            return " ";
        return notes[note_ % 12];
    }

    byte &note() { return note_; }

    const float noteOffset() const { return noteOffset_; }
    float &noteOffset() { return noteOffset_; }

    const vector<byte> hwChecksums() const { return hwChecksums_; }
    vector<byte> &hwChecksums() { return hwChecksums_; }

    const int noteOffsetCents() const { return (noteOffset_ * 100) - 50; }

    const vector<AckData> acknowledgments() const { return acknowledgments_; }
    vector<AckData> &acknowledgments() { return acknowledgments_; }

    void resetPresetNumberUpdateFlag();
    void resetPresetUpdateFlag();
    void resetLooperSettingUpdateFlag();
    void resetLastMessageType();
    void resetAcknowledgments();

    void resetStatus();

private:
    SparkStatus();

    LooperSetting looperSetting_;
    boolean isLooperSettingUpdated_ = false;

    byte lastLooperCommand_;
    int numberOfLoops_;

    string ampName_ = "";
    float measure_;

    byte note_;
    float noteOffset_;
    string notes[12] = {"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "};

    // In case a preset was received from Spark, it is saved here. Can then be read by main program
    Preset currentPreset_;
    Pedal currentEffect_;
    // Preset number. Can be retrieved by main program in case it has been updated by Spark Amp.
    int currentPresetNumber_ = 0;
    // Flags to indicate that either preset or presetNumber have been updated
    boolean isPresetUpdated_ = false;
    boolean isPresetNumberUpdated_ = false;
    boolean isEffectUpdated_ = false;

    vector<AckData> acknowledgments_;
    int lastMessageType_ = 0;
    byte lastMessageNum_ = 0x00;
    byte lastRequestedPreset = 0x00;

    BatteryLevel ampBatteryLevel_ = BATTERY_LEVEL_0;
    bool isAmpBatteryPowered_ = false;
    BatteryChargingStatus ampBatteryChargingStatus_ = BATTERY_CHARGING_STATUS_DISCHARGING;

    vector<byte> hwChecksums_ = {};
};

#endif