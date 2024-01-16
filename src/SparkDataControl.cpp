/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkDataControl.h"

SparkBLEControl *SparkDataControl::bleControl = nullptr;
SparkStreamReader SparkDataControl::spark_ssr;
SparkMessage SparkDataControl::spark_msg;
SparkPresetBuilder SparkDataControl::presetBuilder;
SparkDisplayControl *SparkDataControl::spark_display = nullptr;

Preset SparkDataControl::activePreset_;
Preset SparkDataControl::pendingPreset_ = activePreset_;

int SparkDataControl::activeBank_ = 0;
int SparkDataControl::pendingBank_ = 0;
Preset SparkDataControl::appReceivedPreset_;
int SparkDataControl::presetEditMode_ = PRESET_EDIT_NONE;

int SparkDataControl::presetNumToEdit_ = 0;
int SparkDataControl::presetBankToEdit_ = 0;


int SparkDataControl::activePresetNum_ = 1;
std::string SparkDataControl::responseMsg_ = "";

std::vector<ByteVector> SparkDataControl::ack_msg;
bool SparkDataControl::customPresetAckPending = false;
bool SparkDataControl::retrieveCurrentPreset = false;
int SparkDataControl::operationMode_ = SPARK_MODE_APP;

int SparkDataControl::currentBTMode_ = BT_MODE_BLE;
int SparkDataControl::sparkModeAmp = SPARK_MODE_AMP;
int SparkDataControl::sparkModeApp = SPARK_MODE_APP;

SparkDataControl::SparkDataControl() {
	//init();
	bleControl = new SparkBLEControl(this);
}

SparkDataControl::~SparkDataControl() {
	if (bleControl)
		delete bleControl;
	if (spark_display)
		delete spark_display;
}

int SparkDataControl::init(int opModeInput) {
	operationMode_ = opModeInput;

	std::string currentSparkModeFile;
	int sparkModeInput = 0;
		// Creating vector of presets
	presetBuilder.initializePresetListFromFS();

	fileSystem.openFromFile(sparkModeFileName.c_str(), currentSparkModeFile);

	std::stringstream sparkModeStream(currentSparkModeFile);
	std::string line;
	std::string currentBTModeFile;
	std::stringstream btModeStream;

	while (std::getline(sparkModeStream, line)) {
		sparkModeInput = (int)(line[0]-'0'); // was: stoi(line);
	}
	if (sparkModeInput != 0) {
		operationMode_ = sparkModeInput;
		//Serial.printf("Reading operation mode from file: %d.", sparkModeInput);
	}

	// Define MAC address required for keyboard
	uint8_t mac_keyboard[] = { 0xB4, 0xE6, 0x2D, 0xB2, 0x1B, 0x36 }; //{0x36, 0x33, 0x33, 0x33, 0x33, 0x33};


	switch(operationMode_){
	case SPARK_MODE_APP:
		// Set MAC address for BLE keyboard
		esp_base_mac_addr_set(&mac_keyboard[0]);

		// initialize BLE
		bleKeyboard.setName("Ignitron BLE");
		bleKeyboard.begin();
		//delay(2000);
		bleKeyboard.end();
		bleControl->initBLE(&bleNotificationCallback);
		break;
	case SPARK_MODE_AMP:
		pendingBank_ = 1;
		activeBank_ = 1;
		fileSystem.openFromFile(btModeFileName.c_str(), currentBTModeFile);
		btModeStream.str(currentBTModeFile);

		while (std::getline(btModeStream, line)) {
			currentBTMode_ = (int)(line[0]-'0'); // was: stoi(line);
		}

		if (currentBTMode_ == BT_MODE_BLE) {
			bleControl->startServer();
		} else if (currentBTMode_ == BT_MODE_SERIAL) {
			bleControl->startBTSerial();
		}
		activePreset_ = presetBuilder.getPreset(activePresetNum_, activeBank_);
		pendingPreset_ = presetBuilder.getPreset(activePresetNum_,
				pendingBank_);
		break;
	case SPARK_MODE_KEYBOARD:
		// Set MAC address for BLE keyboard
		esp_base_mac_addr_set(&mac_keyboard[0]);

		// initialize BLE
		bleKeyboard.setName("Ignitron BLE");
		bleKeyboard.begin();
		break;
	}

	return operationMode_;

}


void SparkDataControl::switchOperationMode(int opMode) {
	operationMode_ = opMode;
	buttonMode_ = SWITCH_MODE_PRESET;
	if (opMode == SPARK_MODE_APP) {
		bleKeyboard.end();
	} else if (opMode == SPARK_MODE_LOOPER) {
		bleKeyboard.start();
	}
	updatePendingWithActive();
}

void SparkDataControl::setDisplayControl(SparkDisplayControl *display) {
	spark_display = display;
}

void SparkDataControl::checkForUpdates() {


	if (spark_ssr.isPresetNumberUpdated() && (operationMode_ == SPARK_MODE_APP || operationMode_ == SPARK_MODE_LOOPER)) {
		DEBUG_PRINTLN("Preset number has changed, getting current preset from Spark");
		spark_ssr.resetPresetNumberUpdateFlag();
		getCurrentPresetFromSpark();
	}

	// Check if active preset has been updated
	// If so, update the preset variables
	if (spark_ssr.isPresetUpdated() && (operationMode_ == SPARK_MODE_APP || operationMode_ == SPARK_MODE_LOOPER)){
			pendingPreset_  = spark_ssr.currentSetting();
			activePreset_ = pendingPreset_;
			spark_ssr.resetPresetUpdateFlag();
	}

	if (retrieveCurrentPreset) {
		retrieveCurrentPreset = false;
		getCurrentPresetFromSpark();
	}
	if (operationMode_ == SPARK_MODE_AMP) {

		while (bleControl && bleControl->byteAvailable()) {
			byte inputByte = bleControl->readByte();
			currentBTMsg.push_back(inputByte);
			int msgSize = currentBTMsg.size();
			if (msgSize > 0) {
				if (currentBTMsg[msgSize - 1] == 0xF7) {
					DEBUG_PRINTF("Free Heap size: %d\n", ESP.getFreeHeap()); DEBUG_PRINTF("Max free Heap block: %d\n",
							ESP.getMaxAllocHeap());

					DEBUG_PRINTLN("Received a message");
					DEBUG_PRINTVECTOR(currentBTMsg);
					DEBUG_PRINTLN();
					heap_caps_check_integrity_all(true) ;
					processSparkData(currentBTMsg);

					currentBTMsg.clear();

				}
			}
		}

	}

}

void SparkDataControl::startBLEServer() {
	bleControl->startServer();
}

bool SparkDataControl::checkBLEConnection() {
	if (bleControl->isAmpConnected()) {
		return true;
	}
	if (bleControl->isConnectionFound()) {
		if (bleControl->connectToServer()) {
			bleControl->subscribeToNotifications(&bleNotificationCallback);
			Serial.println("BLE connection to Spark established.");
			return true;
		} else {
			Serial.println("Failed to connect, starting scan");
			bleControl->startScan();
			return false;
		}
	}
	return false;
}


Preset SparkDataControl::getPreset(int bank, int pre) {
	return presetBuilder.getPreset(bank, pre);
}

int SparkDataControl::getNumberOfBanks() {
	return presetBuilder.getNumberOfBanks();
}

bool SparkDataControl::isAmpConnected() {
	return bleControl->isAmpConnected();
}

bool SparkDataControl::isAppConnected() {
	return bleControl->isAppConnected();
}

void SparkDataControl::bleNotificationCallback(
		NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData,
		size_t length, bool isNotify) {
	//Triggered when data is received from Spark Amp in APP mode
	// Transform data into ByteVetor and process
	ByteVector chunk(&pData[0], &pData[length]);
	processSparkData(chunk);

}

int SparkDataControl::processSparkData(ByteVector blk) {

	bool ackNeeded;
	byte seq, sub_cmd;

	DEBUG_PRINTLN("Received data:");
	DEBUG_PRINTVECTOR(blk);
	DEBUG_PRINTLN();
	// Check if ack needed. In positive case the sequence number and command
	// are also returned to send back to requester
	std::tie(ackNeeded, seq, sub_cmd) = spark_ssr.needsAck(blk);
	if (ackNeeded) {
		if (operationMode_ == SPARK_MODE_AMP) {
			ack_msg = spark_msg.send_ack(seq, sub_cmd, DIR_FROM_SPARK);
		} else {
			ack_msg = spark_msg.send_ack(seq, sub_cmd, DIR_TO_SPARK);
		}

		DEBUG_PRINTLN("Sending acknowledgement");
		if (operationMode_ == SPARK_MODE_APP || operationMode_ == SPARK_MODE_LOOPER) {
			bleControl->writeBLE(ack_msg);
		} else if (operationMode_ == SPARK_MODE_AMP) {
			bleControl->notifyClients(ack_msg);
		}
	}
	int retCode = spark_ssr.processBlock(blk);
	if (retCode == MSG_PROCESS_RES_REQUEST && operationMode_ == SPARK_MODE_AMP) {

		std::vector<ByteVector> msg;
		std::vector<CmdData> currentMessage = spark_ssr.lastMessage();
		byte currentMessageNum = spark_ssr.lastMessageNum();
		byte sub_cmd_ = currentMessage.back().subcmd;

		switch (sub_cmd_) {

		case 0x23:
			DEBUG_PRINTLN("Found request for serial number");
			msg = spark_msg.send_serial_number(
					currentMessageNum);
			break;
		case 0x2F:
			DEBUG_PRINTLN("Found request for firmware version");
			msg = spark_msg.send_firmware_version(
					currentMessageNum);
			break;
		case 0x2A:
			DEBUG_PRINTLN("Found request for hw checksum");
			msg = spark_msg.send_hw_checksums(currentMessageNum);
			break;
		case 0x10:
			DEBUG_PRINTLN("Found request for hw preset number");
			msg = spark_msg.send_hw_preset_number(currentMessageNum);
			break;
		case 0x01:
			DEBUG_PRINTLN("Found request for current preset");
			msg = spark_msg.create_preset(activePreset_, DIR_FROM_SPARK,
					currentMessageNum);
			break;
		default:
			DEBUG_PRINTF("Found invalid request: %02x \n", sub_cmd_);
			break;
		}

		bleControl->notifyClients(msg);

	}
	if (retCode == MSG_PROCESS_RES_COMPLETE) {
		std::string msgStr = spark_ssr.getJson();
		if (operationMode_ == SPARK_MODE_APP) {

			if (spark_ssr.lastMessageType() == MSG_TYPE_HWPRESET) {
				DEBUG_PRINTLN("Received HW Preset response");
				activeBank_ = pendingBank_ = 0;
				activePresetNum_ = spark_ssr.currentPresetNumber();
				if (msgStr.length() > 0) {
					Serial.println("Message processed:");
					Serial.println(msgStr.c_str());
				}
			}

			if (spark_ssr.lastMessageType() == MSG_TYPE_PRESET) {
				DEBUG_PRINTLN("Last message was a preset change.");
				if (msgStr.length() > 0) {
					Serial.println("Message processed:");
					Serial.println(msgStr.c_str());
				}
			}
		}

		if (operationMode_ == SPARK_MODE_AMP) {
			if (spark_ssr.lastMessageType() == MSG_TYPE_PRESET) {
				presetEditMode_ = PRESET_EDIT_STORE;
				appReceivedPreset_ = presetBuilder.getPresetFromJson(&msgStr[0]);
				DEBUG_PRINTLN("received from app:");
				DEBUG_PRINTLN(appReceivedPreset_.json.c_str());
				spark_ssr.resetPresetUpdateFlag();
				spark_ssr.resetPresetNumberUpdateFlag();
				presetNumToEdit_ = 0;
			}
		}
		spark_ssr.resetLastMessageType();
	}

	// if last Ack was for preset change (0x38) or effect switch (0x15),
	// confirm pending preset into active
	byte lastAck = spark_ssr.getLastAckAndEmpty();
	//if (((lastAck == 0x38 || lastAck == 0x01) && activeBank_ != 0) || lastAck == 0x15) {
	if (((lastAck == 0x38 || lastAck == 0x01) && activeBank_ != 0) || lastAck == 0x15) {
		Serial.println("OK!");
		if (customPresetAckPending) {
			retrieveCurrentPreset = true;
			customPresetAckPending = false;
		}
		activePreset_ = pendingPreset_;
		// Should not be needed, as both are already set to the same value above
		pendingPreset_ = activePreset_;

	}
	return retCode;
}

bool SparkDataControl::getCurrentPresetFromSpark() {
	current_msg = spark_msg.get_current_preset();
	DEBUG_PRINTLN("Getting current preset from Spark");
	if (bleControl->writeBLE(current_msg)) {
		return true;
	}
	return false;
}

void SparkDataControl::updatePendingPreset(int bnk) {
	pendingPreset_ = getPreset(bnk, activePresetNum_);
}

void SparkDataControl::updatePendingWithActive() {
	pendingBank_ = activeBank_;
	pendingPreset_ = activePreset_;
}
bool SparkDataControl::switchPreset(int pre, bool isInitial) {
	bool retValue = false;
	int bnk = pendingBank_;
	if (operationMode_ == SPARK_MODE_APP || operationMode_ == SPARK_MODE_LOOPER) {
		if (pre == activePresetNum_ && activeBank_ == pendingBank_
				&& !(activePreset_.isEmpty) && !isInitial) {
			Pedal drivePedal = activePreset_.pedals[2];
			std::string drivePedalName = drivePedal.name;
			bool isDriveEnabled = drivePedal.isOn;
			if (switchEffectOnOff(drivePedalName, !isDriveEnabled)) {
				retValue = true;
			}
		} else {
			if (bnk == 0) { // for bank 0 switch hardware presets
				current_msg = spark_msg.change_hardware_preset(pre);
				Serial.printf("Changing to HW preset %d\n", pre);
				if (bleControl->writeBLE(current_msg) && getCurrentPresetFromSpark()) {
					// For HW presets we always need to get the preset from Spark
					// as we don't know the parameters
					retValue = true;
				}
			} else {
				pendingPreset_ = presetBuilder.getPreset(bnk, pre);

				if (pendingPreset_.isEmpty) {
					Serial.println("Empty preset, skipping further processing");
					return false;
				}

				current_msg = spark_msg.create_preset(pendingPreset_);
				Serial.printf("Changing to preset %2d-%d...", bnk, pre);
				if (bleControl->writeBLE(current_msg)) {
					// This is the final message with actually switches over to the
					//previously sent preset
					current_msg = spark_msg.change_hardware_preset(128);
					if (bleControl->writeBLE(current_msg)) {
						customPresetAckPending = true;
						retValue = true;
					}
				}
			}
		}
	}
	if (retValue == true) {
		activeBank_ = bnk;
		activePresetNum_ = pre;
	}

	return retValue;
}

bool SparkDataControl::switchEffectOnOff(std::string fx_name, bool enable) {

	Serial.printf("Switching %s effect %s...", enable ? "On" : "Off",
			fx_name.c_str());
	for (int i = 0; i < pendingPreset_.pedals.size(); i++) {
		Pedal currentPedal = pendingPreset_.pedals[i];
		if (currentPedal.name == fx_name) {
			pendingPreset_.pedals[i].isOn = enable;
			break;
		}
	}
	current_msg = spark_msg.turn_effect_onoff(fx_name, enable);
	if (bleControl->writeBLE(current_msg)) {
		return true;
	}
	return false;
}

void SparkDataControl::processPresetEdit(int presetNum) {
	if (presetNum == 0) {
		processDeletePresetRequest();
	} else if (presetEditMode_ == PRESET_EDIT_STORE) {
		processStorePresetRequest(presetNum);
	} else {
		resetPresetEdit(true, true);
		activePresetNum_ = presetNum;
		activePreset_ = presetBuilder.getPreset(activeBank_, activePresetNum_);
		pendingPreset_ = activePreset_;

	}
}

void SparkDataControl::processStorePresetRequest(int presetNum) {
	int responseCode;
	responseMsg_ = "";
	if (presetEditMode_ == PRESET_EDIT_STORE) {
		if (presetNumToEdit_ == presetNum
				&& presetBankToEdit_ == pendingBank_) {
			responseCode = presetBuilder.storePreset(appReceivedPreset_,
					pendingBank_, presetNum);
			if (responseCode == STORE_PRESET_OK) {
				Serial.println("Successfully stored preset");
				resetPresetEdit(true, true);
				appReceivedPreset_ = { };
				activePresetNum_ = presetNum;
				activePreset_ = presetBuilder.getPreset(activeBank_,
						activePresetNum_);
				pendingPreset_ = activePreset_;
				responseMsg_ = "SAVE OK";
			}
			if (responseCode == STORE_PRESET_FILE_EXISTS) {
				responseMsg_ = "PRST EXIST";
			}
			if (responseCode == STORE_PRESET_ERROR_OPEN
					|| responseCode == STORE_PRESET_UNKNOWN_ERROR) {
				responseMsg_ = "SAVE ERROR";
			}
		} else {
			activePresetNum_ = presetNum;
			activePreset_ = presetBuilder.getPreset(activeBank_,
					activePresetNum_);
			pendingPreset_ = activePreset_;
			presetNumToEdit_ = presetNum;
			presetBankToEdit_ = pendingBank_;
		}
	}

}

void SparkDataControl::resetPresetEdit(bool resetEditMode, bool resetPreset) {
	presetNumToEdit_ = 0;
	presetBankToEdit_ = 0;

	if (resetPreset) {
		appReceivedPreset_ = { };
	}
	if (resetEditMode) {
		presetEditMode_ = PRESET_EDIT_NONE;
	}

}

void SparkDataControl::resetPresetEditResponse() {
	responseMsg_ = "";
}

void SparkDataControl::processDeletePresetRequest() {
	int responseCode;
	responseMsg_ = "";
	if (presetEditMode_ == PRESET_EDIT_DELETE && activeBank_ > 0) {
		responseCode = presetBuilder.deletePreset(activeBank_,
				activePresetNum_);
		if (responseCode == DELETE_PRESET_OK
				|| responseCode == DELETE_PRESET_FILE_NOT_EXIST) {
			Serial.printf("Successfully deleted preset %d-%d\n", pendingBank_,
					activePresetNum_);
			presetNumToEdit_ = 0;
			presetBankToEdit_ = 0;
			activePreset_ = presetBuilder.getPreset(pendingBank_,
					activePresetNum_);
			pendingPreset_ = activePreset_;
			if (responseCode == DELETE_PRESET_OK) {
				responseMsg_ = "DELETE OK";
			} else {
				responseMsg_ = "FILE NOT EXITS";
			}
		}
		if (responseCode == DELETE_PRESET_ERROR_OPEN
				|| responseCode == STORE_PRESET_UNKNOWN_ERROR) {
			responseMsg_ = "DELETE ERROR";
		}
		resetPresetEdit(true, true);
	} else {
		setPresetDeletionFlag();
		presetNumToEdit_ = activePresetNum_;
		presetBankToEdit_ = activeBank_;
	}

}

void SparkDataControl::setPresetDeletionFlag() {
	presetEditMode_ = PRESET_EDIT_DELETE;
}

void SparkDataControl::updateActiveWithPendingPreset() {
	activePreset_ = pendingPreset_;
}

void SparkDataControl::sendButtonPressAsKeyboard(std::string str) {
	if (bleKeyboard.isConnected()) {
		Serial.printf("Sending button: %s\n", str.c_str());
		bleKeyboard.print(str.c_str());
		lastKeyboardButtonPressed_ = str;
	}
	else {
		Serial.println("Keyboard not connected");
	}

}

void SparkDataControl::resetLastKeyboardButtonPressed() {
	lastKeyboardButtonPressed_ = "";
}

void SparkDataControl::toggleBTMode() {

	if (operationMode_ == SPARK_MODE_AMP){
		Serial.print("Switching Bluetooth mode to ");
		if (currentBTMode_ == BT_MODE_BLE) {
			Serial.println("Serial");
			currentBTMode_ = BT_MODE_SERIAL;
		} else if (currentBTMode_ == BT_MODE_SERIAL) {
			Serial.println("BLE");
			currentBTMode_ = BT_MODE_BLE;
		}
		// Save new mode to file
		fileSystem.saveToFile(btModeFileName.c_str(), currentBTMode_);
		fileSystem.saveToFile(sparkModeFileName.c_str(), sparkModeAmp);
		Serial.println("Restarting in new BT mode");
		restartESP(false);
	}
}

void SparkDataControl::restartESP(bool resetSparkMode){
	//RESET Ignitron
	Serial.print("!!! Restarting !!! ");
	if (resetSparkMode) {
		Serial.print("Resetting Spark mode");
		bool sparkModeFileExists = SPIFFS.exists(sparkModeFileName.c_str());
		if(sparkModeFileExists){
			SPIFFS.remove(sparkModeFileName.c_str());
		}
	}
	Serial.println();
	ESP.restart();
}

void SparkDataControl::setBank(int i){
	if (i > presetBuilder.getNumberOfBanks() || i < 0) return;
	activeBank_ = pendingBank_ = i;

}

void SparkDataControl::increaseBank(){

	if (!processAction()) { return; }

	// Cycle around if at the end
	if (pendingBank_ == numberOfBanks()) {
		// Roll over to 0 when going beyond the last bank
		pendingBank_ = 0;
	} else {
		pendingBank_++;
	}
	if (operationMode_ == SPARK_MODE_AMP && pendingBank_ == 0) {
		pendingBank_ = std::min(1, numberOfBanks());
	}
	updatePendingBankStatus();
}

void SparkDataControl::decreaseBank(){

	if (!processAction()) { return; }

	// Roll over to last bank if going beyond the first bank
	if (pendingBank_ == 0) {
		pendingBank_ = numberOfBanks();
	} else {
		pendingBank_--;
	}
	// Don't go to bank 0 in AMP mode
	if (operationMode_ == SPARK_MODE_AMP && pendingBank_ == 0) {
		pendingBank_ = numberOfBanks();
	}
	updatePendingBankStatus();

}

void SparkDataControl::updatePendingBankStatus(){
	// Mark selected bank as the pending Bank to mark in display.
	// The device is configured so that it does not immediately
	// switch presets when a bank is changed, it does so only when
	// the preset button is pushed afterwards
	if (pendingBank_ != 0) {
		updatePendingPreset(pendingBank_);
	}
	if (operationMode_ == SPARK_MODE_AMP) {
		activeBank_ = pendingBank_;
		updateActiveWithPendingPreset();
		if (presetEditMode_ == PRESET_EDIT_DELETE) {
			resetPresetEdit(true, true);
		} else if (presetEditMode_ == PRESET_EDIT_STORE) {
			resetPresetEdit(false, false);
		}
	}

}

bool SparkDataControl::toggleEffect(int fx_identifier){
	if (!processAction() || operationMode_ == SPARK_MODE_AMP) {
		Serial.println("Not connected to Spark Amp or in AMP mode, doing nothing.");
		return false;
	}
	if (!activePreset_.isEmpty) {
		std::string fx_name = activePreset_.pedals[fx_identifier].name;
		bool fx_isOn = activePreset_.pedals[fx_identifier].isOn;

		return switchEffectOnOff(fx_name, fx_isOn ? false : true);
	}
	return false;
}

bool SparkDataControl::toggleButtonMode(){

	if (!processAction() || operationMode_ == SPARK_MODE_AMP ) {
		Serial.println("Spark Amp not connected or in AMP mode, doing nothing.");
		return false;
	}

	Serial.print("Switching to ");
	switch (buttonMode_) {
	case SWITCH_MODE_FX:
		Serial.println("PRESET mode");
		buttonMode_ = SWITCH_MODE_PRESET;
		break;
	case SWITCH_MODE_PRESET:
		Serial.println("FX mode");
		buttonMode_ = SWITCH_MODE_FX;
		updatePendingWithActive();
		break;
	default:
		Serial.println("Unexpected mode. Defaulting to PRESET mode");
		buttonMode_ = SWITCH_MODE_PRESET;
		break;
	} // SWITCH
	return true;
}

bool SparkDataControl::toggleLooperAppMode(){

	if (operationMode_ == SPARK_MODE_AMP || !processAction()) {
		Serial.println("Spark Amp not connected or in AMP mode, doing nothing.");
		return false;
	}
	int newOperationMode;
	Serial.print("Switching to ");
	switch (operationMode_) {
	case SPARK_MODE_APP:
		Serial.println("LOOPER mode");
		newOperationMode = SPARK_MODE_LOOPER;
		break;
	case SPARK_MODE_LOOPER:
		Serial.println("APP mode");
		newOperationMode = SPARK_MODE_APP;
		break;
	default:
		Serial.println("Unexpected mode. Defaulting to APP mode");
		newOperationMode = SPARK_MODE_APP;
		break;
	} // SWITCH
	switchOperationMode(newOperationMode);
	return true;
}

bool SparkDataControl::handleDeletePreset(){

	if (operationMode_ != SPARK_MODE_AMP) {
		Serial.println("Delete Preset: Not in AMP mode, doing nothing.");
		return false;
	}

	if (presetEditMode() == PRESET_EDIT_STORE) {
		resetPresetEdit(true, true);
	}
	else {
		processPresetEdit();
	}
	return true;
}

bool SparkDataControl::processAction(){

	if (operationMode_ == SPARK_MODE_AMP) {
		return true;
	}
	return isAmpConnected();

}

bool SparkDataControl::processPresetSelect(int presetNum){

	if (!processAction()) {
		Serial.println("Action not meeting requirements, ignoring.");
		return false;
	}

	if (operationMode_ == SPARK_MODE_APP
			|| operationMode_ == SPARK_MODE_LOOPER) {
		switchPreset(presetNum, false);
	} else if (operationMode_ == SPARK_MODE_AMP) { // AMP mode
		processPresetEdit(presetNum);
	}
	return true;
}

bool SparkDataControl::increasePresetLooper(){

	if (!processAction() || operationMode_ != SPARK_MODE_LOOPER) {
		Serial.println("Looper preset change: Spark Amp not connected or not in Looper mode, doing nothing");
		return false;
	}

	int selectedPresetNum;
	if (activePresetNum_ == 4) {
		selectedPresetNum = 1;
		if (activeBank_ == numberOfBanks()) {
			pendingBank_ = 0;
		} else {
			pendingBank_++;
		}
	} else {
		selectedPresetNum = activePresetNum_ + 1;
	}
	return switchPreset(selectedPresetNum, false);
}

bool SparkDataControl::decreasePresetLooper(){

	if (!processAction() || operationMode_ != SPARK_MODE_LOOPER) {
		Serial.println("Looper preset change: Spark Amp not connected or not in Looper mode, doing nothing");
		return false;
	}

	int selectedPresetNum;
	if (activePresetNum_== 1) {
		selectedPresetNum = 4;
		if (activeBank_ == 0) {
			pendingBank_ = numberOfBanks();
		} else {
			pendingBank_--;
		}
	} else {
		selectedPresetNum = activePresetNum_ - 1;
	}
	return switchPreset(selectedPresetNum, false);
}
