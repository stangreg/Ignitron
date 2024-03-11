/*
 * SparkDataControl.h
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARKDATACONTROL_H_
#define SPARKDATACONTROL_H_


#include <vector>
#include <queue>
#include <stdexcept>
#include <Arduino.h>
#include "Config_Definitions.h"
#include "SparkBLEControl.h"
#include "SparkBLEKeyboard.h"
#include "SparkKeyboardControl.h"

#include "SparkDisplayControl.h"
#include "SparkMessage.h"
#include "SparkPresetBuilder.h"
#include "SparkStreamReader.h"

#include "SparkTypes.h"

#define PRESET_EDIT_NONE 0
#define PRESET_EDIT_STORE 1
#define PRESET_EDIT_DELETE 2

using namespace std;
using ByteVector = vector<byte>;

class SparkBLEControl;
class SparkDisplayControl;

class SparkDataControl {
public:

	SparkDataControl();
	virtual ~SparkDataControl();

	int init(int op_mode);
	void resetStatus();
	void setDisplayControl(SparkDisplayControl *display);
	bool checkBLEConnection();
	bool isAmpConnected();
	bool isAppConnected(); // true if ESP in AMP mode and client is connected
	void startBLEServer();
	//static void onScanEnded(NimBLEScanResults results);

	// Callback function when Spark notifies about a changed characteristic
	static void bleNotificationCallback(NimBLERemoteCharacteristic *pRemoteCharacteristic,
			uint8_t *pData, size_t length, bool isNotify);

	// methods to process any data from Spark (process with SparkStreamReader and send ack if required)
	static void processSparkData(ByteVector blk);

	// Check if a preset has been updated (via ack or from Spark)
	void checkForUpdates();
	void updatePendingPreset(int bnk);
	void updatePendingWithActive();
	void updateActiveWithPendingPreset();

	bool getAmpName();
	// Switch to a selected preset of the current bank
	bool switchPreset(int pre, bool isInitial);

	// Read in all HW presets
	static void readHWPresets();
	// Switch effect on/off
	bool switchEffectOnOff(string fx_name, bool enable);
	bool toggleEffect(int fx_identifier);
	bool toggleButtonMode();
	bool toggleLooperAppMode();
	bool handleDeletePreset();
	bool processPresetSelect(int presetNum);
	// get a preset from saved presets
	Preset getPreset(int bank, int pre);
	// return the number of banks in the preset list
	int getNumberOfBanks();

	uint8_t lastKeyboardButtonPressed() const {
		return lastKeyboardButtonPressed_;
	}

	string lastKeyboardButtonPressedString() const {
			return lastKeyboardButtonPressedString_;
		}

	KeyboardMapping currentKeyboard() const {
		return keyboardControl->getCurrentKeyboard();
	}

	KeyboardMapping nextKeyboard() {
			keyboardChanged_ = true;
			return keyboardControl->getNextKeyboard();
	}

	KeyboardMapping previousKeyboard(){
			keyboardChanged_ = true;
			return keyboardControl->getPreviousKeyboard();
	}

	void resetKeyboardChangeIndicator() { keyboardChanged_ = false; }
	// Return active or pending preset/bank, set/get active preset number
	Preset* activePreset() const {
		return &activePreset_;
	}
	Preset* pendingPreset() const {
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
	const Preset* appReceivedPreset() const {
		return &appReceivedPreset_;
	}
	const int& operationMode() const {
		return operationMode_;
	}
	int& operationMode() {
		return operationMode_;
	}

	bool& isInitBoot() {
		return isInitBoot_;
	}

	const bool& isInitHWRead() const{
		return isInitHWRead_;
	}


	const int currentBTMode() const {
		return currentBTMode_;
	}
	const int presetNumToEdit() const {
		return presetNumToEdit_;
	}
	const string responseMsg() const {
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

	bool& keyboardChanged(){
		return keyboardChanged_;
	}

	int getMaxChunkSize(int direction);
	int getMaxBlockSize(int direction);

	// Functions for Spark AMP (Server mode)
	void receiveSparkWrite(ByteVector blk);
	void triggerInitialBLENotifications();
	void processPresetEdit(int presetNum = 0);
	void resetPresetEdit(bool resetEditMode, bool resetPreset = false);
	void resetPresetEditResponse();
	void switchOperationMode(int opMode);
	void toggleBTMode();

	void setBank(int i);
	void increaseBank();
	void decreaseBank();
	bool increasePresetLooper();
	bool decreasePresetLooper();

	// Functions for Looper/Keyboard mode
	void sendButtonPressAsKeyboard(keyboardKeyDefinition key);
	void resetLastKeyboardButtonPressed();


	void restartESP(bool resetSparkMode=false);

private:
	static int operationMode_;

	static SparkBLEControl *bleControl;
	static SparkStreamReader spark_ssr;
	static SparkMessage spark_msg;
	static SparkPresetBuilder presetBuilder;
	static SparkDisplayControl *spark_display;
	static SparkKeyboardControl *keyboardControl;



	SparkBLEKeyboard bleKeyboard;
	eSPIFFS fileSystem;

	string btModeFileName = "/config/BTMode.config";
	string sparkModeFileName = "/config/SparkMode.config";

	//Button data
	int buttonMode_ = SWITCH_MODE_PRESET;
	uint8_t lastKeyboardButtonPressed_ = 0;
	string lastKeyboardButtonPressedString_= "";
	bool keyboardChanged_ = false;

	//PRESET variables
	static Preset activePreset_;
	static Preset pendingPreset_;
	static int activeBank_;
	static int pendingBank_;
	static int activePresetNum_;

	// Messages to send to Spark
	static vector<ByteVector> current_msg;
	static vector<ByteVector> ack_msg;
	static bool customPresetAckPending;
	static bool retrieveCurrentPreset;
	static bool customPresetNumberChangePending;

	//Spark AMP mode
	static Preset appReceivedPreset_;
	static int presetNumToEdit_;
	static int presetBankToEdit_;
	static int presetEditMode_;

	static string responseMsg_;

	static int currentBTMode_;
	static int sparkModeAmp;
	static int sparkModeApp;
	ByteVector currentBTMsg;
	static int sparkAmpType;
	static string sparkAmpName;
	static bool with_delay;

	// keep track which HW presets have been read so far
	static int initialHWpreset;
	static int currentRequestedHWPreset;
	static bool isInitBoot_;
	static bool isInitHWRead_;

	int lastUpdateCheck = 0;
	static byte nextMessageNum;
	static queue<ByteVector> msgQueue;

	void processStorePresetRequest(int presetNum);
	void processDeletePresetRequest();
	void setPresetDeletionFlag();
	void updatePendingBankStatus();
	static bool sendMessageToBT(vector<ByteVector> msg);

	// Retrieves the current preset from Spark (required for HW presets)
	static bool getCurrentPresetFromSpark();
	static void setAmpParameters();

	bool processAction();

	// methods to process any data from Spark (process with SparkStreamReader and send ack if required)
	static void handleSendingAck(ByteVector blk);
	static void handleAmpModeRequest();
	static void handleAppModeResponse();
	static void handleIncomingAck();


};

#endif /* SPARKDATACONTROL_H_ */
