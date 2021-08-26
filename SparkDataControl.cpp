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
SparkPresetBuilder SparkDataControl::presetBuilder;
std::vector<ByteVector> SparkDataControl::ack_msg;
SparkDisplayControl* SparkDataControl::spark_display = nullptr;
preset SparkDataControl::activePreset_;
preset SparkDataControl::pendingPreset_ = activePreset_;
bool SparkDataControl::isActivePresetUpdatedByAck = false;
int SparkDataControl::activeBank_ = 0;
int SparkDataControl::pendingBank_ = 0;
int SparkDataControl::operationMode_ = SPARK_MODE_AMP;
preset SparkDataControl::appReceivedPreset_;
bool SparkDataControl::presetReceivedFromApp_ = false;

SparkDataControl::SparkDataControl() {
	//init();
}

SparkDataControl::~SparkDataControl() {
	// TODO Auto-generated destructor stub
}

void SparkDataControl::init(int op_mode){
	// Creating vector of presets
	presetBuilder.initializePresetListFromFS();
	operationMode_ = op_mode;
	if(operationMode_ == SPARK_MODE_APP){
		// initialize BLE
		bleControl.initBLE();
	}
	else if (operationMode_ == SPARK_MODE_AMP){
		bleControl.startServer();
		spark_display->init(operationMode_);

	}
}

void SparkDataControl::setDisplayControl(SparkDisplayControl* display){
	spark_display = display;
}

void SparkDataControl::setActivePresetNum(int num){
	activePresetNum_ = num;
}

void SparkDataControl::checkForUpdates(){
	if(isActivePresetUpdated()){
		pendingPreset_ = activePreset_;
	}
	// When preset number is updated, we are usually in a HW preset, so update banks to 0
	if(isPresetNumberUpdated()){
		activeBank_ = 0;
		pendingBank_ = 0;
		pendingPreset_ = activePreset_;
	}
}

void SparkDataControl::startBLEServer(){
	bleControl.startServer();
}

bool SparkDataControl::checkBLEConnection(){
	if (bleControl.isConnected()){
		return true;
	}
	else{
		if(bleControl.isConnectionFound()){
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

preset SparkDataControl::getPreset(int bank, int pre){
	return presetBuilder.getPreset(bank, pre);
}

int SparkDataControl::getNumberOfBanks(){
	return presetBuilder.getNumberOfBanks();
}

bool SparkDataControl::isBLEConnected(){
	return bleControl.isConnected();
}

bool SparkDataControl::isBLEClientConnected(){
	return bleControl.isClientConnected();
}


void SparkDataControl::notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData,
		size_t length, bool isNotify) {

	// Transform data into ByteVetor and process
	ByteVector chunk(&pData[0], &pData[length]);
	processSparkData(chunk);

}

int SparkDataControl::processSparkData(ByteVector blk){

	bool ackNeeded;
	byte seq, cmd;

	// Check if ack needed. In positive case the sequence number and command
	// are also returned to send back to requester
	std::tie(ackNeeded, seq, cmd) = spark_ssr.needsAck(blk);
	if (ackNeeded){
		ack_msg = spark_msg.send_ack(seq, cmd);
		//Serial.println("Sending acknowledgement");
		if(operationMode_ == SPARK_MODE_APP){
			bleControl.writeBLE(ack_msg);
		}
		else if (operationMode_ == SPARK_MODE_AMP){
			bleControl.notifyClients(ack_msg[0]);
		}
	}
	int retCode = spark_ssr.processBlock(blk);
	if( retCode == MSG_PROCESS_RES_COMPLETE && operationMode_ == SPARK_MODE_AMP){
		Serial.println("Message processed:");
		std::string msgStr = spark_ssr.getJson();
		Serial.println(msgStr.c_str());
		if (spark_ssr.lastMessageType() == MSG_TYPE_PRESET){
			presetReceivedFromApp_ = true;
			appReceivedPreset_ = presetBuilder.getPresetFromJson(&msgStr[0]);
		}
	}
	// if last Ack was for preset change (0x38) or effect switch (0x15),
	// confirm pending preset into active
	byte lastAck = spark_ssr.getLastAckAndEmpty();
	if((lastAck == 0x38 && activeBank_ != 0) || lastAck == 0x15){
		activePreset_ = pendingPreset_;
		isActivePresetUpdatedByAck = true;
	}
	return retCode;
}


void SparkDataControl::getCurrentPresetFromSpark() {
	current_msg = spark_msg.get_current_preset();
	//Serial.println("Getting current preset from Spark");
	bleControl.writeBLE(current_msg);
}

void SparkDataControl::updatePendingPreset(int bnk){
	pendingPreset_ = getPreset(bnk, activePresetNum_);
}

void SparkDataControl::updatePendingWithActiveBank(){
	pendingBank_ = activeBank_;
}

void SparkDataControl::switchPreset(int pre) {
	int bnk = pendingBank_;

	if (pendingBank_ == 0) { // for bank 0 switch hardware presets
		current_msg = spark_msg.change_hardware_preset(pre);
		Serial.printf("Changing to HW preset %d\n", pre);
		bleControl.writeBLE(current_msg);
		// For HW presets we always need to get the preset from Spark
		// as we don't know the parameters
		getCurrentPresetFromSpark();
	} else {
		pendingPreset_ = presetBuilder.getPreset(pendingBank_, pre);
		current_msg = spark_msg.create_preset(pendingPreset_);
		Serial.printf("Changing to preset %2d-%d\n", pendingBank_, pre);
		bleControl.writeBLE(current_msg);
		// This is the final message with actually switches over to the
		//previously sent preset
		current_msg = spark_msg.change_hardware_preset(128);
		bleControl.writeBLE(current_msg);
	}
	activeBank_ = pendingBank_;
	setActivePresetNum(pre);
}

void SparkDataControl::switchEffectOnOff(std::string fx_name, bool enable){

	Serial.printf("Switching %s effect %s\n", enable ? "On" : "Off", fx_name.c_str());
	for(int i=0; i< pendingPreset_.pedals.size(); i++){
		pedal currentPedal = pendingPreset_.pedals[i];
		if (currentPedal.name == fx_name){
			pendingPreset_.pedals[i].isOn = enable;
			break;
		}
	}
	current_msg = spark_msg.turn_effect_onoff(fx_name,
				enable);
	bleControl.writeBLE(current_msg);
}

bool SparkDataControl::isActivePresetUpdated(){
	// Check if active preset has been updated
	// If so, update the preset variables
	if (spark_ssr.isPresetUpdated()) {
		activePreset_ = spark_ssr.currentSetting();
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

	// Check if the preset number has been updated. In that case we need to get
	// current settings from Spark
	if (spark_ssr.isPresetNumberUpdated()) {
		spark_ssr.resetPresetNumberUpdateFlag();
		selectedPresetNum = spark_ssr.currentPresetNumber();
		Serial.printf("Received new preset number: %d\n", selectedPresetNum);
		getCurrentPresetFromSpark();
		return true;
	}
	return false;
}

void SparkDataControl::triggerInitialBLENotifications(){
	bleControl.sendInitialNotification();
}

bool SparkDataControl::presetReceivedFromApp(){
	return presetReceivedFromApp_;
}
