#include "SparkPresetBuilder.h"

SparkPresetBuilder::SparkPresetBuilder() {
	presetBanksNames = {};
}


preset SparkPresetBuilder::getPresetFromJson(char* json) {

	preset resultPreset;
	JSONVar jsonPreset = JSON.parse(json);

	// JSON.typeof(jsonVar) can be used to get the type of the variable
	if (JSON.typeof(jsonPreset) == "undefined") {
		Serial.println("Parsing input failed!");
		return resultPreset;
	}

	// Preset number is not used currently
	resultPreset.presetNumber = 0;
	// myObject.hasOwnProperty(key) checks if the object contains an entry for key
	// preset UUID
	if (jsonPreset.hasOwnProperty("UUID")) {
		std::string presetUUID = (std::string) jsonPreset["UUID"];
		resultPreset.uuid = presetUUID;
	}

	// preset NAME
	if (jsonPreset.hasOwnProperty("Name")) {
		std::string presetName = (std::string) jsonPreset["Name"];
		resultPreset.name = presetName;
	}

	// preset VERSION
	if (jsonPreset.hasOwnProperty("Version")) {
		std::string presetVersion = (std::string) jsonPreset["Version"];
		resultPreset.version = presetVersion;
	}

	// preset Description
	if (jsonPreset.hasOwnProperty("Description")) {
		std::string presetDescription = (std::string) jsonPreset["Description"];
		resultPreset.description = presetDescription;
	}

	// preset Icon
	if (jsonPreset.hasOwnProperty("Icon")) {
		std::string presetIcon = (std::string) jsonPreset["Icon"];
		resultPreset.icon = presetIcon;
	}

	// preset BPM
	if (jsonPreset.hasOwnProperty("BPM")) {
		float presetBpm = (float)((double) jsonPreset["BPM"]);
		resultPreset.bpm = presetBpm;
	}
	// all pedals
	if (jsonPreset.hasOwnProperty("Pedals")) {
		JSONVar pedalArray = jsonPreset["Pedals"];
		for ( int i = 0; i < pedalArray.length(); i++) {
			pedal currentPedal;
			currentPedal.name = (std::string) pedalArray[i]["Name"];
			currentPedal.isOn = (boolean) pedalArray[i]["IsOn"];
			if (pedalArray[i].hasOwnProperty("Parameters")) {
				JSONVar currentPedalParams = pedalArray[i]["Parameters"];
				for (int j = 0; j < currentPedalParams.length(); j++) {
					parameter currentParam;
					currentParam.number = j;
					currentParam.special = 0x91;
					currentParam.value = (float)((double)currentPedalParams[j]);
					currentPedal.parameters.push_back(currentParam);
				}
				resultPreset.pedals.push_back(currentPedal);
			}
			else{
				Serial.println("ERROR: Pedal has no paramters!");
			}
		}
	}
	else{
		Serial.println("ERROR: No pedals found in file");
	}

	// preset Filler
	if (jsonPreset.hasOwnProperty("Filler")) {
		std::string presetFillerString = (std::string) jsonPreset["Filler"];
		byte presetFiller = SparkHelper::HexToByte(presetFillerString);
		resultPreset.filler = presetFiller;
	}
	resultPreset.isEmpty=false;
	return resultPreset;

}

//std::string SparkPresetBuilder::getJsonFromPreset(preset pset){};

void SparkPresetBuilder::initializePresetListFromFS(){
	eSPIFFS fileSystem;
	presetBanksNames.clear();
	std::string allPresetsAsText;
	std::vector<std::string> tmpVector;
	//Serial.println("Trying to read file list");
	if(!fileSystem.openFromFile("/PresetList.txt", allPresetsAsText)){
		Serial.println("ERROR while trying to open presets list file");
		return;
	}

	std::stringstream stream(allPresetsAsText);
	std::string line;
	while (std::getline(stream, line)) {
		std::string presetFilename = line;
		// Lines starting with '-' and empty lines
		// are ignored and can be used for comments in the file
		if (line.rfind("-", 0) != 0 && !line.empty()) {
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

preset SparkPresetBuilder::getPreset(int bank, int pre){
	eSPIFFS fileSystem;
	preset retPreset;
	if(pre > PRESETS_PER_BANK){
		Serial.println("Requested preset out of bounds.");
		return retPreset;
	}

	if(bank > presetBanksNames.size()){
		Serial.println("Requested bank out of bounds.");
		return retPreset;
	}
	std::string presetFilename = "/"+presetBanksNames[bank-1][pre-1];
	std::string presetJsonString;
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

