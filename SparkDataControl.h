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
#define SWITCH_MODE_CHANNEL 2

using ByteVector = std::vector<byte>;

class SparkDataControl {
public:

	SparkDataControl();
	virtual ~SparkDataControl();

	void init();
	bool checkBLEConnection();

	static void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData,
			size_t length, bool isNotify);

	void checkForUpdates();
	bool isActivePresetUpdated();
	bool isPresetNumberUpdated();
	void getCurrentPresetFromSpark();
	void updatePendingPreset(int bnk);
	void readPendingBank();
	void switchPreset(int pre);
	void switchEffectOnOff(std::string fx_name, bool enable);
	bool isBLEConnected();
	//std::vector<std::vector<preset>>* getPresetBanks();
	preset getPreset(int bank, int pre);
	int getNumberOfBanks();

	preset* activePreset() const {return &activePreset_;}
	preset* pendingPreset() const	{return &pendingPreset_;}
	const int& activePresetNum() const {return activePresetNum_;}
	int& activePresetNum() {return activePresetNum_;}

	const int& activeBank() const {return activeBank_;}
	const int& pendingBank() const {return pendingBank_;}
	int& pendingBank() {return pendingBank_;}
	const int numberOfBanks() const {return presetBuilder.getNumberOfBanks();}


	const int& buttonMode() const {return buttonMode_;}
	int& buttonMode() {return buttonMode_;}

private:
	static SparkBLEControl bleControl;
	static SparkStreamReader spark_ssr;
	static SparkMessage spark_msg;
	static SparkPresetBuilder presetBuilder;

	bool startup = true;
	static bool isActivePresetUpdatedByAck;

	//Button data
	int buttonMode_ = SWITCH_MODE_CHANNEL;

	//PRESET variables
	static preset activePreset_;
	static preset pendingPreset_;
	//std::vector<std::vector<preset>> *presetBanks;
	static int activeBank_;
	static int pendingBank_;
	int activePresetNum_ = 1;
	int selectedPresetNum = 1;

	//ByteVector current_msg;
	std::vector<ByteVector> current_msg;
	static std::vector<ByteVector> ack_msg;

	static void processSparkNotification(ByteVector blk);


};

#endif /* SPARKDATACONTROL_H_ */
