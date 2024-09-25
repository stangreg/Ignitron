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

        delay(beatInterval_ / 2);
        beatOnOff_ = !(beatOnOff_);
        delay(beatInterval_ / 2);
        beatOnOff_ = !(beatOnOff_);
        if (looperStarted) {
            increaseBeat();
        }
    }
}

void SparkLooperControl::stop() {
    looperStarted = false;
}

void SparkLooperControl::start() {
    looperStarted = true;
}

void SparkLooperControl::setMeasure(float measure) {
    stop();
    int totalBeat = floor(measure * beatsPerBar_ * totalBars());
    currentBar_ = (totalBeat / beatsPerBar_) + 1;
    currentBeat_ = (totalBeat % beatsPerBar_) + 1;

    DEBUG_PRINTF("Current beat: %f, (%d) %d / %d \n", measure, totalBeat, currentBar_, currentBeat_);

    // 1    2       3      4       1    2      3     4     1    2     3      4     1     2     3    4      1
    // 0   0.0625  0.125  0.1875 0.25 0.3125 0.375 0.4375 0.5 0.5625 0.625 0.6875 0.75 0.8125 0.875 0.9375 1/0
    // 0   1        2      3       4    5      6     7     8    9    10     11    12    13    14   15      16
    // 0   0        0      0       1    1      1     1     2    2     2      2     3     3     3    3       4
    // 1   1        1      1       2    2      2     2     3    3     3      3     4     4     4    4       5
}

void SparkLooperControl::increaseBeat() {
    DEBUG_PRINTLN("Beat increase");
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
    currentBar_ = 1;
    increaseBar_ = false;
    resetTrigger_ = false;
}
