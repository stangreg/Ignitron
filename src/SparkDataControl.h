/*
 * SparkDataControl.h
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARKDATACONTROL_H_
#define SPARKDATACONTROL_H_

#include "CircularBuffer.h"
#include "Config_Definitions.h"
#include "SparkBLEKeyboard.h"
#include "SparkBTControl.h"
#include "SparkKeyboardControl.h"
#include "SparkLooperControl.h"

#include <Arduino.h>
#include <queue>
#include <stdexcept>
#include <vector>

#include "SparkDisplayControl.h"
#include "SparkMessage.h"
#include "SparkPresetBuilder.h"
#include "SparkPresetControl.h"
#include "SparkStreamReader.h"

#include "SparkTypes.h"

using namespace std;
using ByteVector = vector<byte>;

class SparkBTControl;
class SparkDisplayControl;

class SparkDataControl {
public:
    SparkDataControl();
    virtual ~SparkDataControl();

    int init(int op_mode);
    void resetStatus();
    void setDisplayControl(SparkDisplayControl *display);
    bool checkBLEConnection();
    static bool isAmpConnected();
    static bool isAppConnected(); // true if ESP in AMP mode and client is connected
    void startBLEServer();
    // static void onScanEnded(NimBLEScanResults results);

    // Callback function when Spark notifies about a changed characteristic
    static void bleNotificationCallback(NimBLERemoteCharacteristic *pRemoteCharacteristic,
                                        uint8_t *pData, size_t length, bool isNotify);

    // methods to process any data from Spark (process with SparkStreamReader and send ack if required)
    static void processSparkData(ByteVector &blk);

    // Check if a preset has been updated (via ack or from Spark)
    void checkForUpdates();

    bool getAmpName();
    static bool getCurrentPresetNum();
    bool getSerialNumber();
    bool getFirmwareVersion();
    static bool getHWChecksums();
    bool getCurrentPreset(int num);
    static bool getCurrentPresetFromSpark();
    static void readHWPreset(int num);

    // Switch to a selected preset of the current bank
    bool switchPreset(int pre, bool isInitial);
    bool changeHWPreset(int preset);
    bool changePreset(Preset preset);

    // Switch effect on/off
    static bool switchEffectOnOff(const string &fx_name, bool enable);
    static bool toggleEffect(int fx_identifier);

    bool toggleSubMode();
    bool toggleLooperAppMode();

    uint8_t lastKeyboardButtonPressed() const { return lastKeyboardButtonPressed_; }
    string lastKeyboardButtonPressedString() const { return lastKeyboardButtonPressedString_; }
    KeyboardMapping currentKeyboard() const { return keyboardControl->getCurrentKeyboard(); }
    KeyboardMapping nextKeyboard() {
        keyboardChanged_ = true;
        return keyboardControl->getNextKeyboard();
    }

    KeyboardMapping previousKeyboard() {
        keyboardChanged_ = true;
        return keyboardControl->getPreviousKeyboard();
    }

    void resetKeyboardChangeIndicator() { keyboardChanged_ = false; }

    const bool ampNameReceived() const { return ampNameReceived_; }

    const int &operationMode() const { return operationMode_; }
    int &operationMode() { return operationMode_; }

    SparkLooperControl *looperControl() { return looperControl_; }

    bool &isInitBoot() { return isInitBoot_; }

    const int currentBTMode() const { return currentBTMode_; }

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
    const int batteryLevel() const { return batteryLevel_; }
#endif

    // Set/get button mode
    const int &subMode() const { return subMode_; }
    int &subMode() { return subMode_; }

    bool &keyboardChanged() { return keyboardChanged_; }

    int getMaxChunkSize(int direction);
    int getMaxBlockSize(int direction);

    // Functions for Spark AMP (Server mode)
    // void receiveSparkWrite(const ByteVector& blk);
    void triggerInitialBLENotifications();
    void switchSubMode(int subMode);
    void toggleBTMode();

    bool sparkLooperCommand(byte command);
    bool sparkLooperStopAll();
    bool sparkLooperStopPlaying();
    bool sparkLooperPlay();

    bool sparkLooperRec();
    bool sparkLooperDub();
    bool sparkLooperRetry();
    bool sparkLooperStopRec();
    bool sparkLooperUndo();
    bool sparkLooperRedo();
    bool sparkLooperDeleteAll();
    bool sparkLooperStopRecAndPlay();

    bool sparkLooperPlayStop();
    bool sparkLooperRecDub();
    bool sparkLooperUndoRedo();

    bool sparkLooperGetStatus();
    bool sparkLooperGetConfig();
    bool sparkLooperGetRecordStatus();

    void tapTempoButton();

    static bool switchTuner(bool on);

    static bool processAction();
    const SparkStreamReader &getSSR() const { return spark_ssr; }
    // Functions for Looper/Keyboard mode
    void sendButtonPressAsKeyboard(keyboardKeyDefinition key);
    void resetLastKeyboardButtonPressed();

    void restartESP(bool resetSparkMode = false);

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
    // Battery level
    void updateBatteryLevel();
#endif

private:
    static int operationMode_;

    static SparkBTControl *bleControl;
    static SparkStreamReader spark_ssr;
    static SparkStatus &statusObject;
    static SparkMessage spark_msg;
    static SparkDisplayControl *spark_display;
    static SparkKeyboardControl *keyboardControl;
    static SparkLooperControl *looperControl_;

    SparkBLEKeyboard bleKeyboard;

    // TODO: Put settings into proper config file
    string btModeFileName = "/config/BTMode.config";
    string sparkModeFileName = "/config/SparkMode.config";

    // Button data
    static int subMode_;
    uint8_t lastKeyboardButtonPressed_ = 0;
    string lastKeyboardButtonPressedString_ = "";
    bool keyboardChanged_ = false;

    unsigned long lastTapButtonPressed_ = 0;
    const int tapButtonThreshold_ = 2000;
    static int tapEntrySize;
    static CircularBuffer tapEntries;
    static bool recordStartFlag;

    static bool ampNameReceived_;

    // static LooperSetting *looperSetting_;

    // Messages to send to Spark
    static vector<CmdData> current_msg;
    static vector<CmdData> ack_msg;
    static bool customPresetNumberChangePending;

    // Spark AMP mode

    static int currentBTMode_;
    static int sparkModeAmp;
    static int sparkModeApp;
    ByteVector currentBTMsg;
    static int sparkAmpType;
    static string sparkAmpName;
    static bool with_delay;

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
    // Battery level
    static int batteryLevel_;
#endif

    // keep track which HW presets have been read so far
    static bool isInitBoot_;
    static byte special_msg_num;

    static byte nextMessageNum;
    static queue<ByteVector> msgQueue;
    static deque<CmdData> currentCommand;
    static deque<AckData> pendingLooperAcks;

    static bool sendMessageToBT(ByteVector &msg);
    static bool triggerCommand(vector<CmdData> &msg);
    static bool sendNextRequest();

    // Retrieves the current preset from Spark (required for HW presets)
    static void setAmpParameters();

    // methods to process any data from Spark (process with SparkStreamReader and send ack if required)
    static void handleSendingAck(const ByteVector &blk);
    static void handleAmpModeRequest();
    static void handleAppModeResponse();
    static void handleIncomingAck();

    // Read in all HW presets
    void readOpModeFromFile();
    void readBTModeFromFile();

    static bool updateLooperSettings();
    static void startLooperTimer(void *args);
    static void updateLooperCommand(byte lastCommand);
};

#endif /* SPARKDATACONTROL_H_ */
