/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARK_PRESET_BUILDER_H // include guard
#define SPARK_PRESET_BUILDER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <Effortless_SPIFFS.h>
#include <algorithm>
#include <regex>

#include "Config_Definitions.h"

#include "SparkHelper.h"
#include "SparkTypes.h"

#define PRESETS_PER_BANK 4

#define STORE_PRESET_OK 1
#define STORE_PRESET_FILE_EXISTS 2
#define STORE_PRESET_ERROR_OPEN 3
#define STORE_PRESET_UNKNOWN_ERROR 4

#define DELETE_PRESET_OK 1
#define DELETE_PRESET_FILE_NOT_EXIST 2
#define DELETE_PRESET_ERROR_OPEN 3
#define DELETE_PRESET_UNKNOWN_ERROR 4


using ByteVector = std::vector<byte>;

class SparkPresetBuilder{

private:
	std::vector<std::vector<std::string>> presetBanksNames;
	eSPIFFS fileSystem;
	const char* presetListFileName = "/PresetList.txt";
	bool deletePresetFile(int bnk, int pre);

public:
	SparkPresetBuilder();
	//std::string getJsonFromPreset(preset pset);
	void initializePresetListFromFS();
	Preset getPreset(int bank, int preset);
	int getNumberOfBanks();
	Preset getPresetFromJson(char* json);
	int storePreset(Preset newPreset, int bnk, int pre);
	int deletePreset(int bnk, int pre);

};

#endif
