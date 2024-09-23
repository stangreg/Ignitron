#include "SparkLooperControl.h"

LooperSetting SparkLooperControl::looperSetting_;
int SparkLooperControl::currentBeat_ = 1;
int SparkLooperControl::currentBar_ = 1;
int SparkLooperControl::beatsPerBar_ = 4;
int SparkLooperControl::beatInterval_ = 500;
bool SparkLooperControl::beatOnOff_ = true;

SparkLooperControl::SparkLooperControl() {
    // looperSetting_ = new LooperSetting();
}

SparkLooperControl::~SparkLooperControl() {
}

void SparkLooperControl::init() {

    currentBar_ = 1;
    currentBeat_ = 1;
    beatInterval_ = 60000 / looperSetting_.bpm;
}

void SparkLooperControl::start(void *args) {

    while (true) {
        int bpm = looperSetting_.bpm;
        if (bpm > 0) {
            beatInterval_ = 60000 / bpm;
        }
        delay(beatInterval_ / 2);
        beatOnOff_ = !(beatOnOff_);
        delay(beatInterval_ / 2);
        beatOnOff_ = !(beatOnOff_);
        increaseBeat();
    }
}

void SparkLooperControl::increaseBeat() {
    int totalBars = looperSetting_.bars;
    currentBeat_ = currentBeat_ % beatsPerBar_ + 1;
    if (currentBeat_ == 1) {
        currentBar_ = currentBar_ % totalBars + 1;
    }
}

void SparkLooperControl::reset() {
    currentBeat_ = 1;
    currentBar_ = 1;
}
