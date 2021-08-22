#include "SparkPresetBuilder.hh"

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

  /*Serial.print("JSON.typeof(myObject) = ");
    Serial.println(JSON.typeof(jsonObject)); // prints: object
  */

  // TODO: Think if we need the number to display.
  resultPreset.presetNumber = 0;
  // myObject.hasOwnProperty(key) checks if the object contains an entry for key
  // preset UUID
  if (jsonPreset.hasOwnProperty("UUID")) {
    std::string presetUUID = (std::string) jsonPreset["UUID"];
    resultPreset.uuid = presetUUID;
    //Serial.print("jsonPreset[\"UUID\"] = ");
    //Serial.println(presetUUID.c_str());
  }

  // preset NAME
  if (jsonPreset.hasOwnProperty("Name")) {
    std::string presetName = (std::string) jsonPreset["Name"];
    resultPreset.name = presetName;
    //Serial.print("jsonPreset[\"Name\"] = ");
    //Serial.println(presetName.c_str());
  }

  // preset VERSION
  if (jsonPreset.hasOwnProperty("Version")) {
    std::string presetVersion = (std::string) jsonPreset["Version"];
    resultPreset.version = presetVersion;
    //Serial.print("jsonPreset[\"Version\"] = ");
    //Serial.println(presetVersion.c_str());
  }

  // preset Description
  if (jsonPreset.hasOwnProperty("Description")) {
    std::string presetDescription = (std::string) jsonPreset["Description"];
    resultPreset.description = presetDescription;
    //Serial.print("jsonPreset[\"Description\"] = ");
    //Serial.println(presetDescription.c_str());
  }

  // preset Icon
  if (jsonPreset.hasOwnProperty("Icon")) {
    std::string presetIcon = (std::string) jsonPreset["Icon"];
    resultPreset.icon = presetIcon;
    //Serial.print("jsonPreset[\"Icon\"] = ");
    //Serial.println(presetIcon.c_str());
  }

  // preset BPM
  if (jsonPreset.hasOwnProperty("BPM")) {
    float presetBpm = (float)((double) jsonPreset["BPM"]);
    resultPreset.bpm = presetBpm;
    //Serial.print("jsonPreset[\"BPM\"] = ");
    //Serial.println(presetBpm);
  }

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
    //Serial.print("jsonPreset[\"Filler\"] = ");
    //Serial.println(presetFiller);
  }
  resultPreset.isEmpty=false;
  //Serial.println(resultPreset.getPython().c_str());
  return resultPreset;

}
//std::string SparkPresetBuilder::getJsonFromPreset(preset pset){};
/*
std::vector<std::vector<preset>>* SparkPresetBuilder::getPresetBanks(){
	Serial.printf("Returning presetBanks of size %d\n", presetBanks.size());
	return &presetBanks;
}
*/
void SparkPresetBuilder::initializePresetListFromFS(){
	eSPIFFS fileSystem;
	presetBanksNames.clear();
	std::string allPresetsAsText;
	std::vector<std::string> tmpVector;
	Serial.println("Trying to read file list");
	if(fileSystem.openFromFile("/PresetList.txt", allPresetsAsText)){
		Serial.println("Successfully opened presets list file");
	}
	else{
		Serial.println("ERROR while trying to open presets list file");
	}

	//Serial.printf("FileSystem read file /PresetList.txt with size %d", allPresetsAsText.size());
	std::stringstream stream(allPresetsAsText);
	std::string line;
	while (std::getline(stream, line)) {
		std::string presetFilename = line;
		//Serial.printf("Pushing back file name %s\n", presetFilename.c_str());
		tmpVector.push_back(presetFilename);
		if(tmpVector.size() == PRESETS_PER_BANK){
			//Serial.println("Bank is full, pushing bank");
			presetBanksNames.push_back(tmpVector);
			tmpVector.clear();
		}
	}
	if(tmpVector.size() > 0){
		while(tmpVector.size() < 4){
			Serial.println("Last bank not full, adding last preset again to get bank complete");
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
	Serial.printf("Trying to retrieve saved preset %d, %d\n", bank, pre);
	std::string presetFilename = "/"+presetBanksNames[bank-1][pre-1];
	std::string presetJsonString;
	if(fileSystem.openFromFile(&presetFilename[0], presetJsonString)){
		retPreset = getPresetFromJson(&presetJsonString[0]);
		//Serial.println("Retrieved preset:");
		//Serial.println(retPreset.getJson().c_str());
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

