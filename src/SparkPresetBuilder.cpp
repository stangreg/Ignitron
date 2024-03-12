/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkPresetBuilder.h"

SparkPresetBuilder::SparkPresetBuilder() {
	presetBanksNames = {};
	resetHWPresets();
}


Preset SparkPresetBuilder::getPresetFromJson(char* json) {

	Preset resultPreset;
	string jsonString(json);

	const int capacity = JSON_OBJECT_SIZE(
					10) + JSON_ARRAY_SIZE(8) + 8 * JSON_OBJECT_SIZE(4) + 8*JSON_OBJECT_SIZE(8);
	DynamicJsonDocument jsonPreset(capacity);
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
	// preset Filler
	byte presetFiller = jsonPreset["Filler"].as<unsigned char>();
	//byte presetFiller = SparkHelper::HexToByte(presetFillerString);
	resultPreset.filler = presetFiller;

	resultPreset.isEmpty=false;
	resultPreset.json = jsonString;
	DEBUG_PRINTLN("JSON:");
	DEBUG_PRINTLN(resultPreset.json.c_str());
	return resultPreset;

}

//string SparkPresetBuilder::getJsonFromPreset(preset pset){};

void SparkPresetBuilder::initializePresetListFromFS(){

	presetBanksNames.clear();
	string allPresetsAsText;
	vector<string> tmpVector;
	DEBUG_PRINTLN("Trying to read file list");
	if(!fileSystem.openFromFile(presetListFileName, allPresetsAsText)){
		Serial.println("ERROR while trying to open presets list file");
		return;
	}

	stringstream stream(allPresetsAsText);
	string line;
	while (getline(stream, line)) {
		line.erase(remove(line.begin(), line.end(), '\r' ), line.end());
		line.erase(remove(line.begin(), line.end(), '\n' ), line.end());

		// Lines starting with '-' and empty lines
		// are ignored and can be used for comments in the file
		if (line.rfind("-", 0) != 0 && !line.empty()) {
			string presetFilename = line;
			tmpVector.push_back(presetFilename);
			if(tmpVector.size() == PRESETS_PER_BANK){
				presetBanksNames.push_back(tmpVector);
				tmpVector.clear();
			}
		}
	}
	if(tmpVector.size() > 0){
		while(tmpVector.size() < 4){
			Serial.println("Last bank not full, filling with last preset to get bank complete");
			tmpVector.push_back(tmpVector.back());
		}
		presetBanksNames.push_back(tmpVector);
	}

}

Preset SparkPresetBuilder::getPreset(int bank, int pre){
	Preset retPreset{};
	if(pre > PRESETS_PER_BANK){
		Serial.println("Requested preset out of bounds.");
		return retPreset;
	}

	if(bank > presetBanksNames.size()){
		Serial.println("Requested bank out of bounds.");
		return retPreset;
	}
	if (bank == 0){
		return hwPresets.at(pre-1);
	}
	string presetFilename = "/"+presetBanksNames[bank-1][pre-1];
	string presetJsonString;
	if(fileSystem.openFromFile(&presetFilename[0], presetJsonString)){
		retPreset = getPresetFromJson(&presetJsonString[0]);
		return retPreset;
	}
	else{
		Serial.printf("Error while opening file %s, returning empty preset.", presetFilename.c_str());
		return retPreset;
	}
}

int SparkPresetBuilder::getNumberOfBanks(){
	return presetBanksNames.size();
}

int SparkPresetBuilder::storePreset(Preset newPreset, int bnk, int pre){
	string presetNamePrefix = newPreset.name;
	if (presetNamePrefix == "null" || presetNamePrefix.empty()) {
		presetNamePrefix = "Preset";
	}
	Serial.println("Saving preset:");
	Serial.println(newPreset.json.c_str());
	string presetNameWithPath;
	// remove any blanks from the name for a new filename
	presetNamePrefix.erase(remove_if(presetNamePrefix.begin(),
									presetNamePrefix.end(),
									[](char chr){
										return not(regex_match(string(1,chr), regex("[A-z0-9_]")));
										}
									),
									presetNamePrefix.end());
	//cut down name to 24 characters (a potential counter + .json will then increase to 30);
	const int nameLength = presetNamePrefix.length();
	presetNamePrefix.resize(min(24, nameLength));

	string presetFileName = presetNamePrefix + ".json";
	Serial.printf("Store preset with filename %s\n", presetFileName.c_str());
	int counter = 0;

	while (fileSystem.getFileSize(("/" + presetFileName).c_str()) != 0) {
		counter++;
		Serial.printf("ERROR: File '%s' already exists! Saving as copy.\n", presetFileName.c_str());
		char counterStr[2];
		sprintf(counterStr, "%d", counter);
		presetFileName = presetNamePrefix + counterStr + ".json";
	}
	presetNameWithPath = "/" + presetFileName;
	// First store the json string to a new file
	fileSystem.saveToFile(presetNameWithPath.c_str(), newPreset.json.c_str());

	// Then insert the preset into the right position
	string filestr = "";
	string oldListFile;
	int lineCount = 1;
	int insertPosition = 4 * (bnk-1) + pre;

	if(!fileSystem.openFromFile(presetListFileName, oldListFile)){
		Serial.println("ERROR while trying to open presets list file");
		return STORE_PRESET_ERROR_OPEN;
	}

	stringstream stream(oldListFile);
	string line;
	while (getline(stream, line)) {
		if (lineCount != insertPosition) {
			// Lines starting with '-' and empty lines
			// are ignored and can be used for comments in the file
			if (line.rfind("-", 0) != 0 && !line.empty()) {
				if (((lineCount-1) % 4) == 0){
					// New bank separator addd to file for better readability
					char bank_string[20] ="";
					sprintf(bank_string, "%d ", ((lineCount-1)/4)+1);
					filestr += "-- Bank ";
					filestr += bank_string;
					filestr += "\n";
				}
				lineCount++;
				filestr += line + "\n";
			}
		}
		else {
			filestr += presetFileName + "\n";
			// Adding old line so it is not lost.
			filestr += line + "\n";
			lineCount++;
		}
	}
	if(fileSystem.saveToFile(presetListFileName, filestr.c_str())){
		Serial.printf("Successfully stored new preset to %d-%d\n", bnk, pre);
		initializePresetListFromFS();
		return STORE_PRESET_OK;
	}
	return STORE_PRESET_UNKNOWN_ERROR;
}

int SparkPresetBuilder::deletePreset(int bnk, int pre){

	// Then insert the preset into the right position
	string filestr = "";
	string oldListFile;
	string presetFileToDelete = "";
	int lineCount = 1;
	int presetCount = 1;
	int deletePosition = 4 * (bnk-1) + pre;

	if(!fileSystem.openFromFile(presetListFileName, oldListFile)){
		Serial.println("ERROR while trying to open presets list file");
		return STORE_PRESET_ERROR_OPEN;
	}

	stringstream stream(oldListFile);
	string line;
	while (getline(stream, line)) {
		if (lineCount != deletePosition) {
			// Lines starting with '-' and empty lines
			// are ignored and can be used for comments in the file
			if (line.rfind("-", 0) != 0 && !line.empty()) {
				if (((lineCount-1) % 4) == 0){
					// New bank separator added to file for better readability
					char bank_string[20] ="";
					sprintf(bank_string, "%d ", ((lineCount-1)/4)+1);
					filestr += "-- Bank ";
					filestr += bank_string;
					filestr += "\n";
				}
				filestr += line + "\n";
				lineCount++;
				presetCount++;
			}
		}
		else {
			// Just increase the line counter, so deleted line
			// does not get into new content.
			// presetCounter not increased to count presets properly
			presetFileToDelete = "/" + line;
			lineCount++;
		}
	}
	if(fileSystem.saveToFile(presetListFileName, filestr.c_str())){
		initializePresetListFromFS();
		//SPIFFS.begin();
		if(SPIFFS.remove(presetFileToDelete.c_str())){
			return DELETE_PRESET_OK;
		}
		else {
			return DELETE_PRESET_FILE_NOT_EXIST;
		}
	}
	return DELETE_PRESET_UNKNOWN_ERROR;
}

void SparkPresetBuilder::insertHWPreset(int number, Preset preset) {

	if (number <0 || number > 3){
		Serial.println("ERROR: HW Preset not inserted, preset number out of bounds.");
		return;
	}
	hwPresets.at(number) = preset;

}

void SparkPresetBuilder::resetHWPresets() {
	hwPresets.clear();
	Preset examplePreset;
	hwPresets = {examplePreset, examplePreset, examplePreset, examplePreset };
}
