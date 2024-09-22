#include "SparkLooperTimer.h"

SparkLooperTimer::SparkLooperTimer() {
    spark_dc = nullptr;
    looperSetting = nullptr;
}

SparkLooperTimer::SparkLooperTimer(SparkDataControl *dc) {
    spark_dc = dc;
    looperSetting = spark_dc->looperSetting();
}

SparkLooperTimer::~SparkLooperTimer() {
}

void SparkLooperTimer::init() {

    if (looperSetting == nullptr) {
        Serial.println("looperSetting not initialized.");
        return;
    }

    totalBars_ = looperSetting->bars;
    currentBar_ = 1;
    currentBeat_ = 1;
    bpm = looperSetting->bpm;
    beatInterval = 60000 / bpm;
}

void SparkLooperTimer::start() {

    while (true) {
        bpm = looperSetting->bpm;
        if (bpm > 0) {
            beatInterval = 60000 / bpm;
        }
        delay(beatInterval);
        increaseBeat();
    }
}

void SparkLooperTimer::increaseBeat() {
    currentBeat_ = currentBeat_ % beatsPerBar_ + 1;
    if (currentBeat_ == 1) {
        currentBar_ = currentBar_ % totalBars_ + 1;
    }
}

void SparkLooperTimer::reset() {
    currentBeat_ = 1;
    currentBar_ = 1;
}
