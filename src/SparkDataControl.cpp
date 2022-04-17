/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkDataControl.h"

SparkBLEControl SparkDataControl::bleControl;
SparkOTAServer SparkDataControl::otaServer;
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
int SparkDataControl::operationMode_ = SPARK_MODE_APP;


SparkDataControl::SparkDataControl() {
	//init();
}

SparkDataControl::~SparkDataControl() {
	// TODO Auto-generated destructor stub
}

void SparkDataControl::init(int opMode) {
	operationMode_ = opMode;
	// Creating vector of presets
	presetBuilder.initializePresetListFromFS();
	if (operationMode_ == SPARK_MODE_APP) {
		// initialize BLE
		bleKeyboard.setName("Ignitron BLE");
		bleKeyboard.begin();
		delay(1000);
		bleKeyboard.end();
		bleControl.initBLE(&notifyCB);

	} else if (operationMode_ == SPARK_MODE_AMP) {
		pendingBank_ = 1;
		activeBank_ = 1;
		//bleControl.startServer();
		bleControl.startBTClassic();
		activePreset_ = presetBuilder.getPreset(activePresetNum_, activeBank_);
		pendingPreset_ = presetBuilder.getPreset(activePresetNum_,
				pendingBank_);
	}

}

void SparkDataControl::connectToWifi() {
	if (otaServer.init()) {
		isWifiConnected_ = true;
	} else {
		isWifiConnected_ = false;
	}
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
	if (spark_ssr.isPresetNumberUpdated()) {
		spark_ssr.resetPresetNumberUpdateFlag();
		getCurrentPresetFromSpark();
	}
	// Check if active preset has been updated
	// If so, update the preset variables
	if (spark_ssr.isPresetUpdated()) {
		activePreset_ = spark_ssr.currentSetting();
		pendingPreset_ = activePreset_;
		spark_ssr.resetPresetUpdateFlag();
	}
	// Checking if OTA server has been requested
	if (operationMode_ == SPARK_MODE_AMP) {
		otaServer.handleClient();
		while (bleControl.byteAvailable()) {
			byte inputByte = bleControl.readByte();
			if (inputByte < 16)
				Serial.print("0");
			Serial.print(inputByte, HEX);
			currentBTMsg.push_back(inputByte);
			int msgSize = currentBTMsg.size();
			if (msgSize >= 2) {
				if (currentBTMsg[msgSize - 1] == 0xF7) {
					/*if (processSparkData(
							currentBTMsg) == MSG_PROCESS_RES_INITIAL) {
						Serial.println("Received initial request");
						bleControl.sendInitialNotification();
					 }*/
					processSparkData(currentBTMsg);
					bleControl.sendInitialNotification();
					currentBTMsg.clear();

				}
			}
		}
	}

}

void SparkDataControl::startBLEServer() {
	bleControl.startServer();
}

bool SparkDataControl::checkBLEConnection() {
	if (bleControl.isAmpConnected()) {
		return true;
	} else if (bleControl.isConnectionFound()) {
		if (bleControl.connectToServer()) {
			bleControl.subscribeToNotifications(&notifyCB);
			Serial.println("BLE connection to Spark established.");
			return true;
		} else {
			Serial.println("Failed to connect, starting scan");
			bleControl.startScan();
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
	return bleControl.isAmpConnected();
}

bool SparkDataControl::isAppConnected() {
	return bleControl.isAppConnected();
}

void SparkDataControl::notifyCB(
		NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData,
		size_t length, bool isNotify) {

	// Transform data into ByteVetor and process
	ByteVector chunk(&pData[0], &pData[length]);
	processSparkData(chunk);

}

int SparkDataControl::processSparkData(ByteVector blk) {

	bool ackNeeded;
	byte seq, cmd;

	DEBUG_PRINTLN("Received data:");
	DEBUG_PRINTVECTOR(blk);
	DEBUG_PRINTLN();

	// Check if ack needed. In positive case the sequence number and command
	// are also returned to send back to requester
	std::tie(ackNeeded, seq, cmd) = spark_ssr.needsAck(blk);
	if (ackNeeded) {
		ack_msg = spark_msg.send_ack(seq, cmd);
		DEBUG_PRINTLN("Sending acknowledgement");
		if (operationMode_ == SPARK_MODE_APP) {
			bleControl.writeBLE(ack_msg);
		} else if (operationMode_ == SPARK_MODE_AMP) {
			bleControl.notifyClients(ack_msg[0]);
		}
	}
	int retCode = spark_ssr.processBlock(blk);
	if (retCode == MSG_PROCESS_RES_COMPLETE && operationMode_ == SPARK_MODE_AMP) {
		std::string msgStr = spark_ssr.getJson();
		if (msgStr.length() > 0) {
			Serial.println("Message processed:");
			Serial.println(msgStr.c_str());
		}
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
	// if last Ack was for preset change (0x38) or effect switch (0x15),
	// confirm pending preset into active
	byte lastAck = spark_ssr.getLastAckAndEmpty();
	if ((lastAck == 0x38 && activeBank_ != 0) || lastAck == 0x15) {
		Serial.println("OK!");
		activePreset_ = pendingPreset_;
		pendingPreset_ = activePreset_;
	}
	return retCode;
}

bool SparkDataControl::getCurrentPresetFromSpark() {
	current_msg = spark_msg.get_current_preset();
	DEBUG_PRINTLN("Getting current preset from Spark");
	if (bleControl.writeBLE(current_msg)) {
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
				if (bleControl.writeBLE(current_msg)
						&& getCurrentPresetFromSpark()) {
					// For HW presets we always need to get the preset from Spark
					// as we don't know the parameters
					retValue = true;
				}
			} else {
				pendingPreset_ = presetBuilder.getPreset(bnk, pre);
				current_msg = spark_msg.create_preset(pendingPreset_);
				Serial.printf("Changing to preset %2d-%d...", bnk, pre);
				if (bleControl.writeBLE(current_msg)) {
					// This is the final message with actually switches over to the
					//previously sent preset
					current_msg = spark_msg.change_hardware_preset(128);
					if (bleControl.writeBLE(current_msg)) {
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
	if (bleControl.writeBLE(current_msg)) {
		return true;
	}
	return false;
}

void SparkDataControl::triggerInitialBLENotifications() {
	bleControl.sendInitialNotification();
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

void SparkDataControl::sendButtonPressAsKeyboard(uint8_t c) {
	if (bleKeyboard.isConnected()) {
		Serial.printf("Sending button: %d\n", c);
		bleKeyboard.write(c);
	}
	else {
		Serial.println("Keyboard not connected");
	}

}
