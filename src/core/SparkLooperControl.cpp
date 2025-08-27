#include "SparkLooperControl.h"
// TODO Reset bar and current beat count when switching to Looper mode
LooperSetting SparkLooperControl::looperSetting_;
int SparkLooperControl::currentBeat_ = 1;
int SparkLooperControl::currentBar_ = 1;
int SparkLooperControl::beatsPerBar_ = 4;
float SparkLooperControl::beatInterval_ = 500;
bool SparkLooperControl::beatOnOff_ = true;
bool SparkLooperControl::increaseBar_ = false;
bool SparkLooperControl::resetTrigger_ = true;
bool SparkLooperControl::looperStarted = false;
bool SparkLooperControl::isRecRunning_ = false;
bool SparkLooperControl::isRecAvailable_ = false;
bool SparkLooperControl::isPlaying_ = false;
bool SparkLooperControl::canRedo_ = false;
bool SparkLooperControl::canUndo_ = false;
int SparkLooperControl::loopCount_ = 0;
unsigned long SparkLooperControl::lastBeatTimestamp = 0;

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
            beatInterval_ = (float)60000 / bpm;
        }
        if (resetTrigger_) {
            reset();
        }

        unsigned long now = millis();
        if ((now - lastBeatTimestamp) >= (beatInterval_ / 2)) {
            lastBeatTimestamp = now;
            beatOnOff_ = !(beatOnOff_);
            if (looperStarted && beatOnOff_) {
                increaseBeat();
            }
        }
    }
}

void SparkLooperControl::stop() {
    looperStarted = false;
}

void SparkLooperControl::start() {
    looperStarted = true;
    lastBeatTimestamp = millis();
}

void SparkLooperControl::resetStatus() {
    isRecRunning_ = false;
    isRecAvailable_ = false;
    isPlaying_ = false;
    canUndo_ = false;
    canRedo_ = false;
    loopCount_ = 0;
    reset();
}

void SparkLooperControl::setMeasure(float measure) {
    stop();
    int totalBeat = floor(measure * beatsPerBar_ * totalBars());
    currentBar_ = (totalBeat / beatsPerBar_) + 1;
    currentBeat_ = (totalBeat % beatsPerBar_) + 1;

    // DEBUG_PRINTF("Current beat: %f, (%d) %d / %d \n", measure, totalBeat, currentBar_, currentBeat_);
}

string SparkLooperControl::getLooperStatus() {
    string str = "{ ";
    str = str + "Recording running: " + (isRecRunning_ ? "true" : "false");
    str += ", ";
    str = str + "Recording available: " + (isRecAvailable_ ? "true" : "false");
    str += ", ";
    str = str + "Is Playing: " + (isPlaying_ ? "true" : "false");
    str += ", ";
    str = str + "Redo available: " + (canRedo_ ? "true" : "false");
    str += " }";
    return str;
}

void SparkLooperControl::increaseBeat() {
    // DEBUG_PRINTLN("Beat increase");
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
    lastBeatTimestamp = millis();
}
