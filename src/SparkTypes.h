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
#include "Common.h"


using ByteVector = std::vector<byte>;

// Positions of FX types in Preset struct
#define FX_NOISEGATE 	0
#define FX_COMP  	 	1
#define FX_DRIVE 		2
#define FX_AMP 			3
#define FX_MOD 			4
#define FX_DELAY 		5
#define FX_REVERB 		6

#define DIR_TO_SPARK	0
#define DIR_FROM_SPARK  1

struct KeyboardMapping {
	std::string keyboardShortPress[6] = {"1", "2", "3", "4", "5", "6"};
	std::string keyboardLongPress[6] = {"A", "B", "C", "D", "E", "F"};


	int indexOfKey(std::string key){

		int sizeOfArrayShort = sizeof(keyboardShortPress)/sizeof(keyboardShortPress[0]);
		int sizeOfArrayLong = sizeof(keyboardLongPress)/sizeof(keyboardLongPress[0]);

		int index = indexOfValue(keyboardShortPress, sizeOfArrayShort, key);
		// only if not found in first array, search second array
		if (index == -1) {
			index = indexOfValue(keyboardLongPress, sizeOfArrayLong, key);
			return index;
		} else {
			return index;
		}

	}

	private:
	int indexOfValue(std::string* arr, int arrSize, std::string val){

		for (int i = 0; i < arrSize; i++){
			if (arr[i] == val) {
				return i;
			}
		}

		return -1;

	}

};

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
	static const int numberOfPedals = 7;

	std::string json;
	// Raw and text might not be filled (when read from the stored presets).
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
		cmdStr = "[" + SparkHelper::intToHex(cmd) + "], ["
				+ SparkHelper::intToHex(subcmd) + "], [";
		for (byte by : data) {
			cmdStr += SparkHelper::intToHex(by).c_str();
		}
		cmdStr += "]";
		return cmdStr;
	}
};


#endif
