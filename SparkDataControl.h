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

#define SWITCH_MODE_FX 1
#define SWITCH_MODE_PRESET 2

#define SPARK_MODE_APP 1
#define SPARK_MODE_AMP 2

using ByteVector = std::vector<byte>;

class SparkBLEControl;

class SparkDataControl {
public:
	static int operationMode;

	SparkDataControl();
	virtual ~SparkDataControl();

	void init(int op_mode);
	bool checkBLEConnection();
	bool isBLEConnected();
	void startBLEServer();

	// Callback function when Spark notifies about a changed characteristic
	static void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData,
			size_t length, bool isNotify);

	// Check if a preset has been updated (via ack or from Spark)
	void checkForUpdates();
	bool isActivePresetUpdated();
	bool isPresetNumberUpdated();
	// Retrieves the current preset from Spark (required for HW presets)
	void getCurrentPresetFromSpark();
	void updatePendingPreset(int bnk);
	void updatePendingWithActiveBank();
	// Switch to a selected preset of the current bank
	void switchPreset(int pre);
	// Switch effect on/off
	void switchEffectOnOff(std::string fx_name, bool enable);
	// get a preset from saved presets
	preset getPreset(int bank, int pre);
	// return the number of banks in the preset list
	int getNumberOfBanks();

	// Return active or pending preset/bank, set/get active preset number
	preset* activePreset() const {return &activePreset_;}
	preset* pendingPreset() const	{return &pendingPreset_;}
	const int& activePresetNum() const {return activePresetNum_;}
	int& activePresetNum() {return activePresetNum_;}
	const int& activeBank() const {return activeBank_;}
	const int& pendingBank() const {return pendingBank_;}
	int& pendingBank() {return pendingBank_;}
	const int numberOfBanks() const {return presetBuilder.getNumberOfBanks();}

	// Set/get button mode
	const int& buttonMode() const {return buttonMode_;}
	int& buttonMode() {return buttonMode_;}

	// Functions for Spark AMP (Server mode)
	void receiveSparkWrite(ByteVector blk);
	// method to process any data from Spark (process with SparkStreamReader and send ack if required)
	static void processSparkData(ByteVector blk);
	void triggerInitialBLENotifications();

private:
	static SparkBLEControl bleControl;
	static SparkStreamReader spark_ssr;
	static SparkMessage spark_msg;
	static SparkPresetBuilder presetBuilder;

	bool startup = true;
	static bool isActivePresetUpdatedByAck;

	//Button data
	int buttonMode_ = SWITCH_MODE_PRESET;


	//PRESET variables
	static preset activePreset_;
	static preset pendingPreset_;
	static int activeBank_;
	static int pendingBank_;
	int activePresetNum_ = 1;
	int selectedPresetNum = 1;

	// Messages to send to Spark
	std::vector<ByteVector> current_msg;
	static std::vector<ByteVector> ack_msg;



};

#endif /* SPARKDATACONTROL_H_ */
