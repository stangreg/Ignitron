/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkPresetBuilder.h"

void SparkPresetBuilder::updatePresetListUUID(int bnk, int pre, string uuid) {
    presetUUIDs[uuid] = make_pair(bnk, pre);
}

SparkPresetBuilder::SparkPresetBuilder() : presetBanksNames{} {
}

void SparkPresetBuilder::init() {
    // removing for now until further investigation
    // SPIFFS.begin(true);
    //  Creating vector of presets
    Serial.println("Initializing PresetBuilder");
    resetHWPresets();

    initializePresetListFromFS();
    /*xTaskCreatePinnedToCore(
        buildPresetUUIDs, // Function to implement the task
        "PresetUUIDs",    // Name of the task
        10000,            // Stack size in words
        this,             // Task input parameter
        0,                // Priority of the task
        NULL,             // Task handle.
        1                 // Core where the task should run
    );*/
}

Preset SparkPresetBuilder::getPresetFromJson(char *json) {

    Preset resultPreset;
    string jsonString(json);

    JsonDocument jsonPreset;
    DeserializationError err = deserializeJson(jsonPreset, json);

    if (err) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.f_str());
    }

    // Preset number is not used currently
    resultPreset.presetNumber = 0;
    // myObject.hasOwnProperty(key) checks if the object contains an entry for key
    // preset UUID
    string presetUUID = jsonPreset["UUID"].as<string>();
    resultPreset.uuid = presetUUID;

    // preset NAME
    string presetName = jsonPreset["Name"].as<string>();
    resultPreset.name = presetName;

    // preset VERSION
    string presetVersion = jsonPreset["Version"].as<string>();
    resultPreset.version = presetVersion;

    // preset Description
    string presetDescription = jsonPreset["Description"].as<string>();
    resultPreset.description = presetDescription;

    // preset Icon
    string presetIcon = jsonPreset["Icon"].as<string>();
    resultPreset.icon = presetIcon;

    // preset BPM
    float presetBpm = jsonPreset["BPM"].as<float>();
    resultPreset.bpm = presetBpm;

    // all pedals
    JsonArray pedalArray = jsonPreset["Pedals"];
    for (JsonObject currentJsonPedal : pedalArray) {
        Pedal currentPedal;
        currentPedal.name = currentJsonPedal["Name"].as<string>();
        currentPedal.isOn = currentJsonPedal["IsOn"].as<bool>();

        JsonArray pedalParamArray = currentJsonPedal["Parameters"];
        int i = 0;
        for (float currentJsonPedalParam : pedalParamArray) {
            Parameter currentParam;
            currentParam.number = i;
            currentParam.special = 0x91;
            currentParam.value = currentJsonPedalParam;
            currentPedal.parameters.push_back(currentParam);
            i++;
        }
        resultPreset.pedals.push_back(currentPedal);
    }
    // preset checksum
    string presetChecksum = jsonPreset["Checksum"].as<string>();
    if (presetChecksum == "null") {
        Serial.println("Checksum not found, trying legacy format.");
        presetChecksum = jsonPreset["Filler"].as<string>();
    }
    resultPreset.checksum = stoi(presetChecksum, 0, 16);

    resultPreset.isEmpty = false;
    resultPreset.json = jsonString;
    // DEBUG_PRINTLN("JSON:");
    // DEBUG_PRINTLN(resultPreset.json.c_str());
    return resultPreset;
}

// string SparkPresetBuilder::getJsonFromPreset(preset pset){};

void SparkPresetBuilder::initializePresetListFromFS() {

    presetUUIDs.clear();
    Serial.println("Reading custom presets from filesystem.");
    presetBanksNames.clear();
    string allPresetsAsText;
    vector<string> tmpVector;
    bool createUUIDFile = false;
    DEBUG_PRINTLN("Trying to read preset list file");
    File file = LittleFS.open(presetListUUIDFileName);

    if (!file) {
        Serial.println("ERROR while trying to open presets list file");
        createUUIDFile = true;
    }

    if (createUUIDFile) {
        file = LittleFS.open(presetListFileName);
    }

    string fileContent;
    while (file.available()) {
        fileContent += file.read();
    }
    DEBUG_PRINTF("File read: %s\n", fileContent.c_str());
    stringstream fileStream(fileContent);
    file.close();

    string line;
    int bank = 1;
    int preset = 1;
    while (getline(fileStream, line)) {
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(remove(line.begin(), line.end(), '\n'), line.end());

        // Lines starting with '-' and empty lines
        // are ignored and can be used for comments in the file
        if (line.rfind("-", 0) != 0 && !line.empty()) {
            // store UUIDs into filename
            string presetFilename;
            string uuid;
            stringstream lineStream = stringstream(line);
            lineStream >> presetFilename;
            if (!createUUIDFile) {
                lineStream >> uuid;
                presetUUIDs[uuid] = make_pair(bank, preset);
            }
            tmpVector.push_back(presetFilename);
            preset++;
            if (tmpVector.size() == PRESETS_PER_BANK) {
                presetBanksNames.push_back(tmpVector);
                tmpVector.clear();
                bank++;
                preset = 1;
            }
        }
    }
    if (tmpVector.size() > 0) {
        while (tmpVector.size() < 4) {
            Serial.println("Last bank not full, filling with last preset to get bank complete");
            tmpVector.push_back(tmpVector.back());
        }
        presetBanksNames.push_back(tmpVector);
    }

    // Read HWPresets from file, if present
    for (int presetNum = 1; presetNum <= 4; presetNum++) {
        string filename = "HWPreset" + to_string(presetNum) + ".json";
        Preset hwPreset = readPresetFromFile(filename);
        if (!(hwPreset.isEmpty)) {
            hwPresets.at(presetNum - 1) = hwPreset;
            string uuid = hwPreset.uuid;
            updatePresetListUUID(0, presetNum, uuid);
        }
    }

    if (createUUIDFile) {
        buildPresetUUIDs();
    }
}

void SparkPresetBuilder::buildPresetUUIDs() {

    Serial.print("Building UUID file from scratch...");
    File presetUUIDFile = LittleFS.open(presetListUUIDFileName, FILE_WRITE);
    if (!presetUUIDFile) {
        Serial.println("ERROR: Could not open preset UUID file");
    }

    for (int bnk = 1; bnk <= getNumberOfBanks(); bnk++) {
        for (int pre = 1; pre <= PRESETS_PER_BANK; pre++) {
            Preset tmpPreset = getPreset(bnk, pre);
            string uuid = tmpPreset.uuid;
            string presetName = presetBanksNames[bnk - 1][pre - 1];
            string printLine = presetName + " " + uuid + "\n";
            presetUUIDFile.print(printLine.c_str());
            presetUUIDs[uuid] = make_pair(bnk, pre);
        }
    }
    presetUUIDFile.close();
    Serial.println("done.");
}

Preset SparkPresetBuilder::getPreset(int bank, int pre) {
    Preset retPreset;
    if (pre > PRESETS_PER_BANK) {
        Serial.println("Requested preset out of bounds.");
        return retPreset;
    }

    if (bank > presetBanksNames.size()) {
        Serial.println("Requested bank out of bounds.");
        return retPreset;
    }
    if (bank == 0) {
        return hwPresets.at(pre - 1);
    }

    string presetFilename = presetBanksNames[bank - 1][pre - 1];
    retPreset = readPresetFromFile(presetFilename);

    return retPreset;
}

pair<int, int> SparkPresetBuilder::getBankPresetNumFromUUID(string uuid) {
    pair<int, int> result;
    try {
        result = presetUUIDs.at(uuid);
        DEBUG_PRINTF("Found UUID %s as preset %d - %d\n", uuid.c_str(), std::get<0>(result), std::get<1>(result));
    } catch (std::out_of_range exception) {
        Serial.print("Preset not found.");
        result = make_pair(0, 0);
    }
    return result;
}

const int SparkPresetBuilder::getNumberOfBanks() const {
    return presetBanksNames.size();
}

int SparkPresetBuilder::storePreset(Preset newPreset, int bnk, int pre) {
    string presetNamePrefix = newPreset.name;
    string presetUUID = newPreset.uuid;
    if (presetNamePrefix == "null" || presetNamePrefix.empty()) {
        presetNamePrefix = "Preset";
    }

    string presetFileName = savePresetToFile(presetNamePrefix, newPreset);

    // Then insert the preset into the right position
    string filestrPreset = "";
    string filestrPresetUUID = "";
    string oldListFile;
    int lineCount = 1;
    int insertPosition = 4 * (bnk - 1) + pre;

    File presetListFile;
    File presetUUIDListFile;
    presetUUIDListFile = LittleFS.open(presetListUUIDFileName);
    if (!presetUUIDListFile) {
        Serial.println("ERROR while trying to open presets list file");
        return STORE_PRESET_ERROR_OPEN;
    }

    string fileContent;
    while (presetUUIDListFile.available()) {
        fileContent += presetUUIDListFile.read();
    }
    stringstream stream(fileContent);
    string line;

    while (getline(stream, line)) {
        string preset;
        string uuid;
        stringstream(line) >> preset >> uuid;
        if (lineCount != insertPosition) {
            // Lines starting with '-' and empty lines
            // are ignored and can be used for comments in the file
            if (line.rfind("-", 0) != 0 && !line.empty()) {
                if (((lineCount - 1) % 4) == 0) {
                    // New bank separator addd to file for better readability
                    char bank_string[20] = "";
                    int size = sizeof bank_string;
                    snprintf(bank_string, size, "%d ", ((lineCount - 1) / 4) + 1);
                    filestrPreset += "-- Bank ";
                    filestrPreset += bank_string;
                    filestrPreset += "\n";
                }
                lineCount++;
                filestrPreset += preset + "\n";
                filestrPresetUUID += preset + " " + uuid + "\n";
            }
        } else {
            filestrPreset += presetFileName + "\n";
            filestrPresetUUID += presetFileName + " " + presetUUID + "\n";
            // Adding old line so it is not lost.
            filestrPreset += preset + "\n";
            filestrPresetUUID += preset + " " + uuid + "\n";
            lineCount++;
        }
    }
    presetListFile = LittleFS.open(presetListFileName, FILE_WRITE);
    presetUUIDListFile = LittleFS.open(presetListUUIDFileName, FILE_WRITE);
    if (!presetListFile || !presetUUIDListFile) {
        Serial.println("ERROR while trying to open presets list files for writing.");
        return STORE_PRESET_ERROR_OPEN;
    }
    bool success = presetListFile.print(filestrPreset.c_str()) && presetUUIDListFile.print(filestrPresetUUID.c_str());
    presetListFile.close();
    presetUUIDListFile.close();
    if (success) {
        Serial.printf("Successfully stored new preset to %d-%d\n", bnk, pre);
        initializePresetListFromFS();
        return STORE_PRESET_OK;
    }
    return STORE_PRESET_UNKNOWN_ERROR;
}

int SparkPresetBuilder::deletePreset(int bnk, int pre) {

    // Remove the preset
    string filestrPreset = "";
    string filestrPresetUUID = "";
    File presetListFile;
    File presetListUUIDFile;

    string presetFileToDelete = "";
    int lineCount = 1;
    int presetCount = 1;
    int deletePosition = 4 * (bnk - 1) + pre;
    presetListUUIDFile = LittleFS.open(presetListUUIDFileName);
    if (!presetListUUIDFile) {
        Serial.println("ERROR while trying to open presets list file");
        return STORE_PRESET_ERROR_OPEN;
    }

    // Read file content into stream
    string fileContent;
    while (presetListUUIDFile.available()) {
        fileContent += presetListUUIDFile.read();
    }
    presetListUUIDFile.close();
    stringstream stream(fileContent);
    string line;

    DEBUG_PRINTF("DELETE - File content: %s\n", fileContent.c_str());
    string preset;
    string uuid;
    while (getline(stream, line)) {
        if (lineCount != deletePosition) {
            // Lines starting with '-' and empty lines
            // are ignored and can be used for comments in the file
            if (line.rfind("-", 0) != 0 && !line.empty()) {
                stringstream(line) >> preset >> uuid;
                if (((lineCount - 1) % 4) == 0) {
                    // New bank separator added to file for better readability
                    char bank_string[20] = "";
                    int size = sizeof bank_string;
                    snprintf(bank_string, size, "%d ", ((lineCount - 1) / 4) + 1);
                    filestrPreset += "-- Bank ";
                    filestrPreset += bank_string;
                    filestrPreset += "\n";
                }
                filestrPreset += preset + "\n";
                filestrPresetUUID += preset + " " + uuid + "\n";
                lineCount++;
                presetCount++;
            }
        } else {
            // Just increase the line counter, so deleted line
            // does not get into new content.
            // presetCounter not increased to count presets properly
            stringstream(line) >> preset >> uuid;
            presetFileToDelete = "/" + preset;
            lineCount++;
        }
    }
    presetListFile = LittleFS.open(presetListFileName, FILE_WRITE);
    presetListUUIDFile = LittleFS.open(presetListUUIDFileName, FILE_WRITE);
    if (!presetListFile || !presetListUUIDFile) {
        Serial.println("ERROR opening preset files for writing.");
        return STORE_PRESET_ERROR_OPEN;
    }
    bool success = presetListFile.print(filestrPreset.c_str()) && presetListUUIDFile.print(filestrPresetUUID.c_str());
    presetListFile.close();
    presetListUUIDFile.close();
    if (success) {
        initializePresetListFromFS();

        if (LittleFS.remove(presetFileToDelete.c_str())) {
            return DELETE_PRESET_OK;
        } else {
            return DELETE_PRESET_FILE_NOT_EXIST;
        }
    }
    return DELETE_PRESET_UNKNOWN_ERROR;
}

void SparkPresetBuilder::insertHWPreset(int number, const Preset &preset) {

    if (number < 0 || number > 3) {
        Serial.println("ERROR: HW Preset not inserted, preset number out of bounds.");
        return;
    }
    hwPresets.at(number) = preset;
    string filename = "HWPreset" + to_string(number + 1);
    savePresetToFile(filename, preset, true);
    string uuid = preset.uuid;
    updatePresetListUUID(0, number + 1, uuid);
}

string SparkPresetBuilder::savePresetToFile(string filename, const Preset &preset, bool overwrite) {

    Serial.println("Saving preset:");
    Serial.println(preset.json.c_str());
    string presetNameWithPath;
    // remove any blanks from the name for a new filename
    filename.erase(remove_if(filename.begin(),
                             filename.end(),
                             [](char chr) {
                                 return not(regex_match(string(1, chr), regex("[A-z0-9_]")));
                             }),
                   filename.end());
    // cut down name to 24 characters (a potential counter + .json will then increase to 30);
    const int nameLength = filename.length();
    filename.resize(min(24, nameLength));

    string presetFileName = filename + ".json";
    Serial.printf("Store preset with filename %s\n", presetFileName.c_str());
    int counter = 0;

    presetFileName = "/" + presetFileName;
    File presetFile = LittleFS.open(presetFileName.c_str());

    if (!overwrite) {
        while (presetFile && presetFile.size() != 0) {
            counter++;
            Serial.printf("ERROR: File '%s' already exists! Saving as copy.\n", presetFileName.c_str());
            char counterStr[2];
            int size = sizeof counterStr;
            snprintf(counterStr, size, "%d", counter);
            presetFileName = filename + counterStr + ".json";
            presetFile.close();
            presetFile = LittleFS.open(presetFileName.c_str());
        }
    }
    presetFile.close();
    presetFile = LittleFS.open(presetFileName.c_str(), FILE_WRITE);
    // Store the json string to a new file
    presetFile.print(preset.json.c_str());
    presetFile.close();

    return presetFileName;
}

Preset SparkPresetBuilder::readPresetFromFile(string filename) {

    Preset retPreset;
    string presetJsonString;

    string fullFilename = "/" + filename;
    DEBUG_PRINTF("Trying to read preset %s...", fullFilename.c_str());
    File file = LittleFS.open(fullFilename.c_str());

    if (file) {
        while (file.available()) {
            presetJsonString += (char)file.read();
        }
        file.close();
        retPreset = getPresetFromJson(&presetJsonString[0]);
        DEBUG_PRINTLN("done.");
        DEBUG_PRINTF("Read preset: %s\n", retPreset.json.c_str());
        return retPreset;
    } else {
        Serial.printf("Error while opening file %s, returning empty preset.\n", fullFilename.c_str());
        return retPreset;
    }
}

void SparkPresetBuilder::resetHWPresets() {
    hwPresets.clear();
    Preset examplePreset;
    hwPresets = {examplePreset, examplePreset, examplePreset, examplePreset};
}

void SparkPresetBuilder::validateChecksums(vector<byte> checksums) {

    // TODO: Once all 8 Spark 2 HW presets are supported, this needs to be adjusted
    if (hwPresets.size() < 4 || checksums.size() < 4) {
        Serial.println("ERROR: Vector HW Presets or Checksums not in the right size.");
        return;
    }

    bool success = true;

    // TODO: Change limit to variable
    for (int presetNum = 0; presetNum < 4; presetNum++) {
        byte presetChk = hwPresets.at(presetNum).checksum;
        byte check = (byte)checksums.at(presetNum);
        if (presetChk != check) {
            Serial.printf("HW checksum for preset %d changed (Cache: %02x / Amp: %02x), invalidating cache.\n", presetNum + 1, presetChk, check);
            Preset emptyPreset;
            hwPresets.at(presetNum) = emptyPreset;
            success = false;
        }
    }
    if (success) {
        Serial.println("Cached HW presets are valid.");
    }
}

bool SparkPresetBuilder::isHWPresetMissing(int num) {
    if (num < 1 || num > 4) {
        return false;
    }
    if (hwPresets.at(num - 1).isEmpty) {
        return true;
    }
    return false;
}
