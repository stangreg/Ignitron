/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: steffen
 */

#include "SparkDataControl.h"

	SparkBLEControl SparkDataControl::bleControl;
	SparkStreamReader SparkDataControl::spark_ssr;
	SparkMessage SparkDataControl::spark_msg;
	//SparkPresetBuilder *SparkDataControl::presetBuilder = new SparkPresetBuilder();
	SparkPresetBuilder SparkDataControl::presetBuilder;
	std::vector<ByteVector> SparkDataControl::ack_msg;
	preset SparkDataControl::activePreset;
	preset SparkDataControl::pendingPreset;
	bool SparkDataControl::isActivePresetUpdatedByAck = false;
	int SparkDataControl::activeBank = 0;

SparkDataControl::SparkDataControl() {
	//delete presetBuilder;
}

SparkDataControl::~SparkDataControl() {
	// TODO Auto-generated destructor stub
}

void SparkDataControl::init(){
	Serial.println("Initializing DataControl");
	//presetBuilder->initializePresetBanks();
	presetBuilder.initializePresetListFromFS();
	Serial.println("Done initializing preset banks");
	bleControl.initBLE();
	//presetBanks = presetBuilder->getPresetBanks();
	//Serial.printf("DataControl preset banks size is %d\n", presetBanks->size());
}

bool SparkDataControl::checkBLEConnection(){
	if (bleControl.isConnected()){
		return true;
	}
	else{
		if(bleControl.connectionFound()){
			if(bleControl.connectToServer()){
				bleControl.subscribeToNotifications(&notifyCB);
				Serial.println("BLE connection to Spark established.");
				startup = false;
				return true;
			}
			else {
				Serial.println("Failed to connect, starting scan");
				bleControl.initScan();
				startup = true;
				return false;
			}
		}
	}
	return false;
}

/*
std::vector<std::vector<preset>>* SparkDataControl::getPresetBanks(){
	return (std::vector<std::vector<preset>>*)presetBanks;
}
*/
preset SparkDataControl::getPreset(int bank, int pre){
	return presetBuilder.getPreset(bank, pre);
}

int SparkDataControl::getNumberOfBanks(){
	return presetBuilder.getNumberOfBanks();
}

preset SparkDataControl::getActivePreset(){
	return activePreset;
}

int SparkDataControl::getActiveBank(){
	return activeBank;
}

bool SparkDataControl::isBLEConnected(){
	return bleControl.isConnected();
}

void SparkDataControl::notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData,
			size_t length, bool isNotify) {

		ByteVector chunk(&pData[0], &pData[length]);
		SparkDataControl::processSparkNotification(chunk);

	}

void SparkDataControl::processSparkNotification(ByteVector blk){

	bool ackNeeded;
	byte seq, cmd;

	std::tie(ackNeeded, seq, cmd) = spark_ssr.needsAck(blk);
	if (ackNeeded){
		ack_msg = spark_msg.send_ack(seq, cmd);
		Serial.println("Sending acknowledgement");
		bleControl.writeBLE(ack_msg);
	}
	spark_ssr.processBlock(blk);
	byte lastAck = spark_ssr.getLastAckAndEmpty();
	if((lastAck == 0x38 && activeBank != 0) || lastAck == 0x15){
		activePreset = pendingPreset;
		isActivePresetUpdatedByAck = true;
	}
}


void SparkDataControl::getCurrentPresetFromSpark() {
	//delay(1000);
	current_msg = spark_msg.get_current_preset();
	Serial.println("Getting current preset from Spark");
	bleControl.writeBLE(current_msg);
}

void SparkDataControl::switchPreset(int bnk, int pre) {
	Serial.printf("Called switchPreset with params %d and %d\n", bnk, pre);
	if (bnk == 0) { // for bank 0 choose hardware presets
		current_msg = spark_msg.change_hardware_preset(pre);
		Serial.printf("Changing to HW preset %d‘\n", pre);
		bleControl.writeBLE(current_msg);
		getCurrentPresetFromSpark();
	} else {
		pendingPreset = presetBuilder.getPreset(bnk, pre);
		current_msg = spark_msg.create_preset(pendingPreset);
		Serial.printf("Switching preset to %d, %d\n", bnk, pre);
		bleControl.writeBLE(current_msg);
		current_msg = spark_msg.change_hardware_preset(128);
		bleControl.writeBLE(current_msg);
		//getCurrentPresetFromSpark();
	}
	activeBank = bnk;
}

void SparkDataControl::switchEffectOnOff(std::string fx_name, bool enable){
	current_msg = spark_msg.turn_effect_onoff(fx_name,
							enable);
	Serial.printf("Switching effect %s to status %s\n", fx_name.c_str(), enable ? "On" : "Off");
	for(int i=0; i< pendingPreset.pedals.size(); i++){
		pedal currentPedal = pendingPreset.pedals[i];
		if (currentPedal.name == fx_name){
			pendingPreset.pedals[i].isOn = enable;
			break;
		}
	}
	bleControl.writeBLE(current_msg);
}

bool SparkDataControl::isActivePresetUpdated(){
	if (spark_ssr.isPresetUpdated()) {
			activePreset = spark_ssr.getCurrentSetting();
			pendingPreset = activePreset;
			spark_ssr.resetPresetUpdateFlag();
			isActivePresetUpdatedByAck = false;
			return true;
	}
	// if preset is not updated by message from Spark, it can be updated by acknlowledging a previous preset change
	else if (isActivePresetUpdatedByAck){
		isActivePresetUpdatedByAck = false;
		return true;
	}
	return false;
}

bool SparkDataControl::isPresetNumberUpdated(){
	if (spark_ssr.isPresetNumberUpdated()) {
		spark_ssr.resetPresetNumberUpdateFlag();
		selectedPresetNum = spark_ssr.getCurrentPresetNumber();
		Serial.printf("Received new preset. Number: %d\n", selectedPresetNum);
		getCurrentPresetFromSpark();
		return true;
	}
	return false;
}
