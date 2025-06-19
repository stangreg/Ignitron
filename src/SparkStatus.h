#ifndef SPARKCURRENTSTATUS_H
#define SPARKCURRENTSTATUS_H

#include "SparkTypes.h"

enum MessageType {
    MSG_TYPE_NONE,
    MSG_TYPE_PRESET,
    MSG_TYPE_HWPRESET,
    MSG_TYPE_FX_ONOFF,
    MSG_TYPE_FX_CHANGE,
    MSG_TYPE_FX_PARAM,
    MSG_TYPE_AMP_NAME,
    MSG_TYPE_AMP_SERIAL,
    MSG_TYPE_LOOPER_SETTING,
    MSG_TYPE_TAP_TEMPO,
    MSG_TYPE_MEASURE,
    MSG_TYPE_LOOPER_COMMAND,
    MSG_TYPE_LOOPER_STATUS,
    MSG_TYPE_TUNER_OUTPUT,
    MSG_TYPE_TUNER_ON,
    MSG_TYPE_TUNER_OFF,
    MSG_TYPE_HWCHECKSUM,
    MSG_TYPE_HWCHECKSUM_EXT,
    MSG_TYPE_AMPSTATUS,
    MSG_TYPE_INPUT_VOLUME,
    MSG_REQ_SERIAL,
    MSG_REQ_FW_VER,
    MSG_REQ_PRESET_CHK,
    MSG_REQ_CURR_PRESET_NUM,
    MSG_REQ_CURR_PRESET,
    MSG_REQ_AMP_STATUS,
    MSG_REQ_72,
    MSG_REQ_PRESET1,
    MSG_REQ_PRESET2,
    MSG_REQ_PRESET3,
    MSG_REQ_PRESET4,
    MSG_REQ_INVALID,
};

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

    const MessageType lastMessageType() const { return lastMessageType_; }
    MessageType &lastMessageType() { return lastMessageType_; }

    const byte lastMessageNum() const { return lastMessageNum_; }
    byte &lastMessageNum() { return lastMessageNum_; }

    const string ampName() const { return ampName_; }
    string &ampName() { return ampName_; }

    const string ampSerialNumber() const { return ampSerialNumber_; }
    string &ampSerialNumber() { return ampSerialNumber_; }

    const BatteryLevel ampBatteryLevel() const { return ampBatteryLevel_; }
    BatteryLevel &ampBatteryLevel() { return ampBatteryLevel_; }

    const bool isAmpBatteryPowered() const { return isAmpBatteryPowered_; }
    bool &isAmpBatteryPowered() { return isAmpBatteryPowered_; }

    const int ampBatteryChargingStatus() const { return ampBatteryChargingStatus_; }
    BatteryChargingStatus &ampBatteryChargingStatus() { return ampBatteryChargingStatus_; }

    const float inputVolume() const { return inputVolume_; }
    float &inputVolume() { return inputVolume_; }

    const bool isVolumeChanged() const { return isVolumeChanged_; }
    bool &isVolumeChanged() { return isVolumeChanged_; }

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
    void resetVolumeUpdateFlag();

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
    MessageType lastMessageType_ = MSG_TYPE_NONE;
    byte lastMessageNum_ = 0x00;
    byte lastRequestedPreset = 0x00;

    BatteryLevel ampBatteryLevel_ = BATTERY_LEVEL_0;
    bool isAmpBatteryPowered_ = false;
    BatteryChargingStatus ampBatteryChargingStatus_ = BATTERY_CHARGING_STATUS_DISCHARGING;

    vector<byte> hwChecksums_ = {};
    string ampSerialNumber_ = "";

    float inputVolume_ = 0.0;
    bool isVolumeChanged_ = false;
};

#endif