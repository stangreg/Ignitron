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

class SparkDataControl;

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
    static SparkDataControl *spark_dc;
    static LooperSetting *looperSetting;

    static int currentBeat_;
    static int currentBar_;

    static int totalBars_;
    static int beatsPerBar_;

    static int bpm;

    static int beatInterval_;

    static void increaseBeat();
};

#endif /* SPARKLOOPERTIMER_H_ */