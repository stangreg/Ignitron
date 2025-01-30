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
#include <LittleFS.h>
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

using namespace std;
using ByteVector = vector<byte>;

class SparkPresetBuilder {

private:
    vector<vector<string>> presetBanksNames;
    std::map<string, pair<int, int>> presetUUIDs;
    vector<Preset> hwPresets;
    const char *presetListFileName = "/PresetList.txt";
    bool deletePresetFile(int bnk, int pre);
    void updatePresetListUUID(int bnk, int pre, string uuid);
    void initializePresetListFromFS();
    void buildPresetUUIDs();

public:
    SparkPresetBuilder();
    // string getJsonFromPreset(preset pset);

    void resetHWPresets();
    Preset getPreset(int bank, int preset);
    pair<int, int> getBankPresetNumFromUUID(string uuid);
    const int getNumberOfBanks() const;
    Preset getPresetFromJson(char *json);
    int storePreset(Preset newPreset, int bnk, int pre);
    int deletePreset(int bnk, int pre);

    void insertHWPreset(int number, const Preset &preset);
    bool isHWPresetMissing(int num);
};

#endif
