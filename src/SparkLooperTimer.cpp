#include "SparkLooperTimer.h"

SparkDataControl *SparkLooperTimer::spark_dc = nullptr;
LooperSetting *SparkLooperTimer::looperSetting = nullptr;
int SparkLooperTimer::currentBeat_ = 1;
int SparkLooperTimer::currentBar_ = 1;
int SparkLooperTimer::totalBars_ = 4;
int SparkLooperTimer::bpm = 120;
int SparkLooperTimer::beatsPerBar_ = 4;
int SparkLooperTimer::beatInterval_ = 500;

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
    beatInterval_ = 60000 / bpm;
}

void SparkLooperTimer::start(void *args) {

    while (true) {
        bpm = looperSetting->bpm;
        if (bpm > 0) {
            beatInterval_ = 60000 / bpm;
        }
        delay(beatInterval_);
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
