/*
 * SparkLooperControl.h
 *
 *  Created on: 22.09.2024
 *      Author: stangreg
 */

#ifndef SPARKLOOPERCONTROL_H_
#define SPARKLOOPERCONTROL_H_

#include "Config_Definitions.h"
#include "SparkTypes.h"
#include <Arduino.h>

using namespace std;

class SparkLooperControl {

public:
    SparkLooperControl();
    virtual ~SparkLooperControl();

    const LooperSetting *looperSetting() const { return &looperSetting_; }

    const int currentBeat() const { return currentBeat_; }
    const int currentBar() const { return currentBar_; }
    const int totalBars() const { return looperSetting_.bars; }
    const int bpm() const { return looperSetting_.bpm; }
    const int beatOnOff() const { return beatOnOff_; }
    void init();
    static void run(void *args);
    void stop();
    void start();
    static void reset();
    void triggerReset() { resetTrigger_ = true; }

    void changeSettingBpm(int bpm) { looperSetting_.setBpm(bpm); }
    void changeSettingBars() { looperSetting_.cycleBars(); }
    void toggleSettingClick() { looperSetting_.toggleClick(); }
    void toggleSettingCount() { looperSetting_.toggleCount(); }
    void resetSetting() { looperSetting_.reset(); }
    void setLooperSetting(LooperSetting setting) { looperSetting_ = setting; }
    void setCurrentBar(int bar) { currentBar_ = bar; }
    void resetChangePending() { looperSetting_.changePending = false; }
    void setMeasure(float measure);

private:
    static LooperSetting looperSetting_;

    static bool looperStarted;

    static int currentBeat_;
    static int currentBar_;

    static int beatsPerBar_;
    static bool increaseBar_;

    static int beatInterval_;
    static bool beatOnOff_;

    static bool resetTrigger_;

    static void increaseBeat();
};

#endif /* SPARKLOOPERTIMER_H_ */