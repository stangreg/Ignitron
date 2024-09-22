/*
 * SparkLooperTimer.h
 *
 *  Created on: 22.09.2024
 *      Author: stangreg
 */

#ifndef SPARKLOOPERTIMER_H_
#define SPARKLOOPERTIMER_H_

#include "Config_Definitions.h"
#include "SparkDataControl.h"
#include "SparkTypes.h"
#include <Arduino.h>

using namespace std;

class SparkLooperTimer {

public:
    SparkLooperTimer();
    SparkLooperTimer(SparkDataControl *dc);
    virtual ~SparkLooperTimer();

    const int currentBeat() const { return currentBeat_; }
    const int currentBar() const { return currentBar_; }
    void init();
    static void start(void *args);
    void stop();
    void reset();

private:
    SparkDataControl *spark_dc;
    LooperSetting *looperSetting;

    int currentBeat_ = 1;
    int currentBar_ = 1;

    int totalBars_ = 4;
    int beatsPerBar_ = 4;

    int bpm = 120;

    unsigned long beatInterval;

    void increaseBeat();
};

#endif /* SPARKLOOPERTIMER_H_ */