/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARK_TYPES_H
#define SPARK_TYPES_H

#include <vector>
#include <Arduino.h>
#include "SparkHelper.h"

using ByteVector = std::vector<byte>;

// Positions of FX types in Preset struct
#define FX_NOISEGATE 	0
#define FX_COMP  	 	1
#define FX_DRIVE 		2
#define FX_AMP 			3
#define FX_MOD 			4
#define FX_DELAY 		5
#define FX_REVERB 		6

struct Parameter {

	int number;
	std::string special;
	float value;
};

struct Pedal {

	std::string name;
	boolean isOn;
	std::vector<Parameter> parameters;

};

struct Preset {

	boolean isEmpty = true;
	const int numberOfPedals = 7;

	std::string json;
	std::string raw;
	std::string text;

	int presetNumber;
	std::string uuid;
	std::string name;
	std::string version;
	std::string description;
	std::string icon;
	float bpm;
	std::vector<Pedal> pedals;
	byte filler;

};

struct CmdData {
	byte cmd;
	byte subcmd;
	ByteVector data;

	std::string toString() {
		std::string cmdStr;
		cmdStr = "[" + cmd + "], [" + subcmd + "], [";
		for (byte by : data) {
			cmdStr += SparkHelper::intToHex(by).c_str();
		}
		cmdStr += "]";
		return cmdStr;
	}
};


#endif
