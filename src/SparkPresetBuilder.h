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
#include "SparkStatus.h"
#include "SparkTypes.h"

const int PRESETS_PER_BANK = 4;

enum PresetStoreResult {
    STORE_PRESET_OK,
    STORE_PRESET_FILE_EXISTS,
    STORE_PRESET_ERROR_OPEN,
    STORE_PRESET_UNKNOWN_ERROR
};

enum PresetDeleteResult {
    DELETE_PRESET_OK,
    DELETE_PRESET_FILE_NOT_EXIST,
    DELETE_PRESET_ERROR_OPEN,
    DELETE_PRESET_UNKNOWN_ERROR
};

using namespace std;
using ByteVector = vector<byte>;

class SparkPresetBuilder {

private:
    vector<vector<string>> presetBanksNames;
    std::map<string, pair<int, int>> presetUUIDs;
    vector<Preset> hwPresets;

    int numberOfHWBanks_ = 1;
    int numberOfHWPresets_ = PRESETS_PER_BANK;

    const char *presetListFileName = "/PresetList.txt";
    const char *presetListUUIDFileName = "/PresetListUUIDs.txt";
    bool deletePresetFile(int bnk, int pre);
    void updatePresetListUUID(int bnk, int pre, string uuid);
    void initializePresetListFromFS();

    void buildPresetUUIDs();

public:
    SparkPresetBuilder();
    // string getJsonFromPreset(preset pset);
    void init();
    void initHWPresets();

    void resetHWPresets();

    const int numberOfHWbanks() const { return numberOfHWBanks_; }
    int &numberOfHWBanks() { return numberOfHWBanks_; }
    const int numberOfHWPresets() const { return numberOfHWPresets_; }

    void validateChecksums(vector<byte> checksums);
    Preset getPreset(int bank, int preset);
    pair<int, int> getBankPresetNumFromUUID(string uuid);
    const int getNumberOfBanks() const;
    Preset getPresetFromJson(char *json);
    Preset getPresetFromJson(File file);
    Preset getPresetFromJsonDocument(JsonDocument doc, string jsonString);
    PresetStoreResult storePreset(Preset newPreset, int bnk, int pre);
    PresetDeleteResult deletePreset(int bnk, int pre);

    void insertHWPreset(int number, const Preset &preset);
    string processFilename(string filename, const Preset &preset, bool overwrite = false);
    Preset readPresetFromFile(string filename);
    bool isHWPresetMissing(int num);
};

#endif
