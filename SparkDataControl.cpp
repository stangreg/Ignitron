/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkDataControl.h"

SparkBLEControl SparkDataControl::bleControl;
SparkStreamReader SparkDataControl::spark_ssr;
SparkMessage SparkDataControl::spark_msg;
//SparkPresetBuilder *SparkDataControl::presetBuilder = new SparkPresetBuilder();
SparkPresetBuilder SparkDataControl::presetBuilder;
std::vector<ByteVector> SparkDataControl::ack_msg;
preset SparkDataControl::activePreset_;
preset SparkDataControl::pendingPreset_ = activePreset_;
bool SparkDataControl::isActivePresetUpdatedByAck = false;
int SparkDataControl::activeBank_ = 0;
int SparkDataControl::pendingBank_ = 0;

SparkDataControl::SparkDataControl() {
	//init();
}

SparkDataControl::~SparkDataControl() {
	// TODO Auto-generated destructor stub
}

void SparkDataControl::init(){
	//Serial.println("Initializing DataControl");
	//presetBuilder->initializePresetBanks();
	presetBuilder.initializePresetListFromFS();
	//Serial.println("Done initializing preset banks");
	bleControl.initBLE();
	//presetBanks = presetBuilder->getPresetBanks();
	//Serial.printf("DataControl preset banks size is %d\n", presetBanks->size());
}

void SparkDataControl::checkForUpdates(){
	if(isActivePresetUpdated()){
		//Serial.println("Preset updated!");
		pendingPreset_ = activePreset_;
		//Serial.println("Active preset:"),
		//Serial.println(activePreset.getJson().c_str());
	}
	if(isPresetNumberUpdated()){
		//Serial.println("Preset number updated!");
		activeBank_ = 0;
		pendingBank_ = 0;
		pendingPreset_ = activePreset_;
	}
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
	if((lastAck == 0x38 && activeBank_ != 0) || lastAck == 0x15){
		activePreset_ = pendingPreset_;
		isActivePresetUpdatedByAck = true;
	}
}


void SparkDataControl::getCurrentPresetFromSpark() {
	//delay(1000);
	current_msg = spark_msg.get_current_preset();
	Serial.println("Getting current preset from Spark");
	bleControl.writeBLE(current_msg);
}

void SparkDataControl::updatePendingPreset(int bnk){
	pendingPreset_ = getPreset(bnk, activePresetNum_);
}

void SparkDataControl::readPendingBank(){
	pendingBank_ = activeBank_;
}

void SparkDataControl::switchPreset(int pre) {
	int bnk = pendingBank_;
	Serial.printf("Called switchPreset with params %d and %d\n", pendingBank_, pre);
	if (pendingBank_ == 0) { // for bank 0 choose hardware presets
		current_msg = spark_msg.change_hardware_preset(pre);
		Serial.printf("Changing to HW preset %d\n", pre);
		bleControl.writeBLE(current_msg);
		getCurrentPresetFromSpark();
	} else {
		pendingPreset_ = presetBuilder.getPreset(pendingBank_, pre);
		current_msg = spark_msg.create_preset(pendingPreset_);
		Serial.printf("Switching preset to %d, %d\n", pendingBank_, pre);
		bleControl.writeBLE(current_msg);
		current_msg = spark_msg.change_hardware_preset(128);
		bleControl.writeBLE(current_msg);
	}
	activeBank_ = pendingBank_;
	activePresetNum_ = pre;
}

void SparkDataControl::switchEffectOnOff(std::string fx_name, bool enable){
	current_msg = spark_msg.turn_effect_onoff(fx_name,
			enable);
	Serial.printf("Switching effect %s to status %s\n", fx_name.c_str(), enable ? "On" : "Off");
	for(int i=0; i< pendingPreset_.pedals.size(); i++){
		pedal currentPedal = pendingPreset_.pedals[i];
		if (currentPedal.name == fx_name){
			pendingPreset_.pedals[i].isOn = enable;
			break;
		}
	}
	bleControl.writeBLE(current_msg);
}

bool SparkDataControl::isActivePresetUpdated(){
	if (spark_ssr.isPresetUpdated()) {
		activePreset_ = spark_ssr.getCurrentSetting();
		pendingPreset_ = activePreset_;
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
