#ifndef SPARK_PRESET_BUILDER_H // include guard
#define SPARK_PRESET_BUILDER_H

#include <array>
#include <vector>
#include <iomanip>
#include <sstream>
#include <stdio.h>


#include <Arduino.h>
#include <Arduino_JSON.h>


#include "SparkTypes.h"
#include "SparkHelper.hh"
//#include "SparkPresets.hh"

#include <FS.h>
#include <Effortless_SPIFFS.h>

#define PRESETS_PER_BANK 4

using ByteVector = std::vector<byte>;

class SparkPresetBuilder{

private:
	//SparkPresets presets;
	//std::vector<std::vector<preset>> presetBanks;
	std::vector<std::vector<std::string>> presetBanksNames;
	std::vector<char*> allPresetsJSON;

	//preset getPresetFromJson(std::string json);
	preset getPresetFromJson(char* json);


public:
	SparkPresetBuilder();
	//std::string getJsonFromPreset(preset pset);
	//void initializePresetBanks();
	void initializePresetListFromFS();
	//std::vector<std::vector<preset>>* getPresetBanks();
	preset getPreset(int preset, int bank);
	int getNumberOfBanks();
};

#endif
