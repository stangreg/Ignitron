#include "SparkLooperControl.h"
// TODO Reset bar and currnet beat count when switching to Looper mode
LooperSetting SparkLooperControl::looperSetting_;
int SparkLooperControl::currentBeat_ = 1;
int SparkLooperControl::currentBar_ = 1;
int SparkLooperControl::beatsPerBar_ = 4;
int SparkLooperControl::beatInterval_ = 500;
bool SparkLooperControl::beatOnOff_ = true;
bool SparkLooperControl::increaseBar_ = false;
bool SparkLooperControl::resetTrigger_ = true;
bool SparkLooperControl::looperStarted = false;

SparkLooperControl::SparkLooperControl() {
    // looperSetting_ = new LooperSetting();
}

SparkLooperControl::~SparkLooperControl() {
}

void SparkLooperControl::init() {

    currentBar_ = 1;
    currentBeat_ = 1;
    increaseBar_ = false;
    beatInterval_ = 60000 / looperSetting_.bpm;
}

void SparkLooperControl::run(void *args) {

    while (true) {
        int bpm = looperSetting_.bpm;
        if (bpm > 0) {
            beatInterval_ = 60000 / bpm;
        }
        if (resetTrigger_) {
            reset();
        }
        if (looperStarted) {
            delay(beatInterval_ / 2);
            beatOnOff_ = !(beatOnOff_);
            delay(beatInterval_ / 2);
            beatOnOff_ = !(beatOnOff_);
            increaseBeat();
        } else {
            reset();
        }
    }
}

void SparkLooperControl::stop() {
    reset();
    looperStarted = false;
}

void SparkLooperControl::start() {
    reset();
    looperStarted = true;
}

void SparkLooperControl::increaseBeat() {
    int totalBars = looperSetting_.bars;
    currentBeat_ = currentBeat_ % beatsPerBar_ + 1;
    if (increaseBar_) {
        currentBar_ = currentBar_ % totalBars + 1;
        increaseBar_ = false;
    }
    if (currentBeat_ == beatsPerBar_) {
        increaseBar_ = true;
    }
}

void SparkLooperControl::reset() {
    currentBeat_ = 1;
    currentBar_ = 0;
    increaseBar_ = false;
    resetTrigger_ = false;
}


