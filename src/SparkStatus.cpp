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
    last_message_type_ = 0;
}

void SparkStatus::resetAcknowledgments() {
    acknowledgments_.clear();
}
