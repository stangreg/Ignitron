/*
 * SparkDataControl.h
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARKDATACONTROL_H_
#define SPARKDATACONTROL_H_

#include <vector>
#include <Arduino.h>

#include "SparkBLEControl.h"
#include "SparkMessage.h"
#include "SparkPresetBuilder.h"
#include "SparkStreamReader.h"
#include "SparkTypes.h"
#include "SparkDisplayControl.h"

#define SWITCH_MODE_FX 1
#define SWITCH_MODE_PRESET 2

#define SPARK_MODE_APP 1
#define SPARK_MODE_AMP 2

#define PRESET_EDIT_NONE 0
#define PRESET_EDIT_STORE 1
#define PRESET_EDIT_DELETE 2

using ByteVector = std::vector<byte>;

class SparkBLEControl;
class SparkDisplayControl;

class SparkDataControl {
public:

	SparkDataControl();
	virtual ~SparkDataControl();

	void init(int op_mode);
	void setDisplayControl(SparkDisplayControl *display);
	bool checkBLEConnection();
	bool isAmpConnected();
	bool isAppConnected(); // true if ESP in AMP mode and client is connected
	void startBLEServer();
	//static void onScanEnded(NimBLEScanResults results);

	// Callback function when Spark notifies about a changed characteristic
	static void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic,
			uint8_t *pData, size_t length, bool isNotify);

	// Check if a preset has been updated (via ack or from Spark)
	void checkForUpdates();
	void updatePendingPreset(int bnk);
	void updatePendingWithActiveBank();
	void updateActiveWithPendingPreset();
	// Retrieves the current preset from Spark (required for HW presets)
	bool getCurrentPresetFromSpark();
	// Switch to a selected preset of the current bank
	bool switchPreset(int pre);
	// Switch effect on/off
	bool switchEffectOnOff(std::string fx_name, bool enable);
	// get a preset from saved presets
	preset getPreset(int bank, int pre);
	// return the number of banks in the preset list
	int getNumberOfBanks();

	// Return active or pending preset/bank, set/get active preset number
	preset* activePreset() const {
		return &activePreset_;
	}
	preset* pendingPreset() const {
		return &pendingPreset_;
	}
	const int& activePresetNum() const {
		return activePresetNum_;
	}
	//int& activePresetNum() {return activePresetNum_;}
	const int& activeBank() const {
		return activeBank_;
	}
	int& activeBank() {
		return activeBank_;
	}

	const int& pendingBank() const {
		return pendingBank_;
	}

	int& pendingBank() {
		return pendingBank_;
	}
	const int numberOfBanks() const {
		return presetBuilder.getNumberOfBanks();
	}
	const preset* appReceivedPreset() const {
		return &appReceivedPreset_;
	}
	const int operationMode() const {
		return operationMode_;
	}
	const int presetNumToEdit() const {
		return presetNumToEdit_;
	}
	const std::string responseMsg() const {
		return responseMsg_;
	}
	const int presetEditMode() const {
		return presetEditMode_;
	}

	// Set/get button mode
	const int& buttonMode() const {
		return buttonMode_;
	}
	int& buttonMode() {
		return buttonMode_;
	}

	// Functions for Spark AMP (Server mode)
	void receiveSparkWrite(ByteVector blk);
	// method to process any data from Spark (process with SparkStreamReader and send ack if required)
	static int processSparkData(ByteVector blk);
	void triggerInitialBLENotifications();
	void processPresetEdit(int presetNum = 0);
	void resetPresetEdit(bool resetEditMode, bool resetPreset = false);
	void resetPresetEditResponse();

private:
	static int operationMode_;

	static SparkBLEControl bleControl;
	static SparkStreamReader spark_ssr;
	static SparkMessage spark_msg;
	static SparkPresetBuilder presetBuilder;
	static SparkDisplayControl *spark_display;

	//Button data
	int buttonMode_ = SWITCH_MODE_PRESET;

	//PRESET variables
	static preset activePreset_;
	static preset pendingPreset_;
	static int activeBank_;
	static int pendingBank_;
	static int activePresetNum_;

	// Messages to send to Spark
	std::vector<ByteVector> current_msg;
	static std::vector<ByteVector> ack_msg;

	//Spark AMP mode
	static preset appReceivedPreset_;
	static int presetNumToEdit_;
	static int presetBankToEdit_;
	static int presetEditMode_;

	static std::string responseMsg_;

	void processStorePresetRequest(int presetNum);
	void processDeletePresetRequest();
	void setPresetDeletionFlag();

};

#endif /* SPARKDATACONTROL_H_ */
