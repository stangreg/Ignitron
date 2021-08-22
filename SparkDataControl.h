/*
 * SparkDataControl.h
 *
 *  Created on: 19.08.2021
 *      Author: steffen
 */

#ifndef SPARKDATACONTROL_H_
#define SPARKDATACONTROL_H_

#include <vector>
#include <Arduino.h>

#include "SparkStreamReader.hh"
#include "SparkBLEControl.h"
#include "SparkMessage.hh"
#include "SparkTypes.h"
#include "SparkPresetBuilder.hh"


using ByteVector = std::vector<byte>;

class SparkDataControl {
public:

	SparkDataControl();
	virtual ~SparkDataControl();

	bool checkBLEConnection();

	static void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData,
			size_t length, bool isNotify);

	void init();
	preset getActivePreset();
	int getActiveBank();
	bool isActivePresetUpdated();
	bool isPresetNumberUpdated();
	void getCurrentPresetFromSpark();
	void switchPreset(int bnk, int pre);
	void switchEffectOnOff(std::string fx_name, bool enable);
	bool isBLEConnected();
	//std::vector<std::vector<preset>>* getPresetBanks();
	preset getPreset(int bank, int pre);
	int getNumberOfBanks();

private:
	static SparkBLEControl bleControl;
	static SparkStreamReader spark_ssr;
	static SparkMessage spark_msg;
	static SparkPresetBuilder presetBuilder;

	bool startup = true;
	static bool isActivePresetUpdatedByAck;

	//PRESET variables
	static preset activePreset;
	static preset pendingPreset;
	//std::vector<std::vector<preset>> *presetBanks;
	static int activeBank;
	int selectedPresetNum = 1;

	//ByteVector current_msg;
	std::vector<ByteVector> current_msg;
	static std::vector<ByteVector> ack_msg;

	static void processSparkNotification(ByteVector blk);


};

#endif /* SPARKDATACONTROL_H_ */
