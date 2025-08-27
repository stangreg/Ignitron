#include "SparkStatus.h"

SparkStatus::SparkStatus() {
}

SparkStatus::~SparkStatus() {
}

SparkStatus &SparkStatus::getInstance() {
    static SparkStatus INSTANCE;
    return INSTANCE;
}

void SparkStatus::resetPresetNumberUpdateFlag() {
    isPresetNumberUpdated_ = false;
}

void SparkStatus::resetPresetUpdateFlag() {
    isPresetUpdated_ = false;
}

void SparkStatus::resetLooperSettingUpdateFlag() {
    isLooperSettingUpdated_ = false;
}

void SparkStatus::resetLastMessageType() {
    lastMessageType_ = MSG_TYPE_NONE;
}

void SparkStatus::resetAcknowledgments() {
    acknowledgments_.clear();
}

void SparkStatus::resetVolumeUpdateFlag() {
    isVolumeChanged_ = false;
}

void SparkStatus::resetStatus() {
    isLooperSettingUpdated_ = false;

    lastLooperCommand_ = 0;

    ampName_ = "";
    // Preset number. Can be retrieved by main program in case it has been updated by Spark Amp.
    currentPresetNumber_ = 0;
    // Flags to indicate that either preset or presetNumber have been updated
    isPresetUpdated_ = false;
    isPresetNumberUpdated_ = false;
    isEffectUpdated_ = false;

    acknowledgments_.clear();
    lastMessageType_ = MSG_TYPE_NONE;
    lastMessageNum_ = 0x00;
    lastRequestedPreset = 0x00;

    ampBatteryLevel_ = BATTERY_LEVEL_0;
    isAmpBatteryPowered_ = false;
    ampBatteryChargingStatus_ = BATTERY_CHARGING_STATUS_DISCHARGING;

    hwChecksums_.clear();
}
