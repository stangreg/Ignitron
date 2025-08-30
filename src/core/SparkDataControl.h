/*
 * SparkDataControl.h
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARKDATACONTROL_H_
#define SPARKDATACONTROL_H_

/* *******************************************************************************
 * IMPORTANT: Mode Management Using SparkModeManager
 * 
 * To access SparkModeManager methods, use the following pattern:
 * 
 *   SparkModeManager& modeManager = sparkDataControl.getModeManager();
 *   
 *   // Then use direct calls to the modeManager
 *   if (modeManager.operationMode() == SPARK_MODE_APP) {
 *     // ...
 *   }
 *   
 *   modeManager.toggleSubMode();
 *   
 * The delegated methods in SparkDataControl are marked as deprecated
 * and should be replaced with direct calls to SparkModeManager.
 * *******************************************************************************/

#include "../communication/SparkBTControl.h"
#include "../core/Config_Definitions.h"
#include "../hardware/SparkBLEKeyboard.h"
#include "../hardware/SparkKeyboardControl.h"
#include "../utils/CircularBuffer.h"
#include "SparkLooperControl.h"
#include "SparkModeManager.h"

#include <Arduino.h>
#include <queue>
#include <stdexcept>
#include <vector>

#include "../communication/SparkMessage.h"
#include "../communication/SparkStreamReader.h"
#include "../hardware/SparkDisplayControl.h"
#include "../preset/SparkPresetBuilder.h"
#include "../preset/SparkPresetControl.h"

#include "SparkTypes.h"

using namespace std;
using ByteVector = vector<byte>;

class SparkBTControl;
class SparkDisplayControl;

class SparkDataControl {
public:
    SparkDataControl();
    virtual ~SparkDataControl();

    OperationMode init(OperationMode opMode);
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

    static void queueMessage(ByteVector &blk);
    // methods to process any data from Spark (process with SparkStreamReader and send ack if required)
    static void processSparkData(ByteVector &blk);

    // Check if a preset has been updated (via ack or from Spark)
    void checkForUpdates();

    static bool getAmpName();
    static bool getCurrentPresetNum();
    static bool getSerialNumber();
    static bool getFirmwareVersion();
    static bool getHWChecksums();
    bool getCurrentPreset(int num);
    static bool getCurrentPresetFromSpark();
    static void readHWPreset(int num);

    // Switch to a selected preset of the current bank
    bool switchPreset(int pre, bool isInitial);
    bool changeHWPreset(int preset);
    bool changePreset(Preset preset);

    // Switch effect on/off
    static bool switchEffectOnOff(const string &fxName, bool enable);
    static bool toggleEffect(int fxIdentifier);

    uint8_t lastKeyboardButtonPressed() const { return lastKeyboardButtonPressed_; }
    string lastKeyboardButtonPressedString() const { return lastKeyboardButtonPressedString_; }
    KeyboardMapping &currentKeyboard() const { return keyboardControl->getCurrentKeyboard(); }
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

    /**
     * Get direct access to the SparkModeManager instance
     * @return Reference to the SparkModeManager
     */
    static SparkModeManager &getModeManager() {
        return modeManager;
    }

    // DEPRECATED: Mode-related methods delegated to SparkModeManager
    // Use getModeManager().<method>() instead
    [[deprecated("Use getModeManager().operationMode() instead")]]
    const OperationMode &operationMode() const { return modeManager.operationMode(); }
    [[deprecated("Use getModeManager().operationMode() instead")]]
    OperationMode &operationMode() { return modeManager.operationMode(); }
    [[deprecated("Use getModeManager().subMode() instead")]]
    const SubMode &subMode() const { return modeManager.subMode(); }
    [[deprecated("Use getModeManager().subMode() instead")]]
    SubMode &subMode() { return modeManager.subMode(); }
    [[deprecated("Use getModeManager().currentBTMode() instead")]]
    const BTMode currentBTMode() const { return modeManager.currentBTMode(); }
    [[deprecated("Use getModeManager().toggleSubMode() instead")]]
    bool toggleSubMode() { return modeManager.toggleSubMode(); }
    [[deprecated("Use getModeManager().toggleLooperAppMode() instead")]]
    bool toggleLooperAppMode() { return modeManager.toggleLooperAppMode(); }
    [[deprecated("Use getModeManager().toggleBTMode() instead")]]
    void toggleBTMode() { modeManager.toggleBTMode(); }

    SparkLooperControl &looperControl() { return looperControl_; }

    bool &isInitBoot() { return isInitBoot_; }

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
    const BatteryLevel batteryLevel() const { return batteryLevel_; }
#endif

    bool &keyboardChanged() { return keyboardChanged_; }

    int getMaxChunkSize(int direction);
    int getMaxBlockSize(int direction);

    // Functions for Spark AMP (Server mode)
    // void receiveSparkWrite(const ByteVector& blk);
    static void switchSubMode(SubMode subMode);

    bool sparkLooperCommand(LooperCommand command);
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
    const SparkStreamReader &getSSR() const { return sparkSsr; }
    // Functions for Looper/Keyboard mode
    void sendButtonPressAsKeyboard(keyboardKeyDefinition key);
    void resetLastKeyboardButtonPressed();

    void restartESP(bool resetSparkMode = false);

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
    // Battery level
    void updateBatteryLevel();
#endif

private:
    static SparkBTControl *bleControl;
    static SparkStreamReader sparkSsr;
    static SparkStatus &statusObject;
    static SparkMessage sparkMsg;
    static SparkDisplayControl *sparkDisplay;
    static SparkKeyboardControl *keyboardControl;
    static SparkLooperControl looperControl_;
    static SparkModeManager modeManager;

    static SparkBLEKeyboard bleKeyboard;

    // Button data
    uint8_t lastKeyboardButtonPressed_ = 0;
    string lastKeyboardButtonPressedString_ = "";
    bool keyboardChanged_ = false;

    unsigned long lastTapButtonPressed_ = 0;
    const int tapButtonThreshold_ = 2000;
    static int tapEntrySize;
    static CircularBuffer tapEntries;
    static bool recordStartFlag;

    static bool ampNameReceived_;
    const unsigned int updateAmpBatteryInterval = 60000; // Update battery status every minute
    unsigned int lastAmpBatteryUpdate = 0;               // When battery level was last updated

    // static LooperSetting *looperSetting_;

    // Messages to send to Spark
    static vector<CmdData> currentMsg;
    static vector<CmdData> ackMsg;
    static bool customPresetNumberChangePending;

    // Spark AMP mode
    ByteVector currentBTMsg;
    static AmpType sparkAmpType;
    static string sparkAmpName;
    static bool withDelay;
    static ByteVector checksums;

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
    // Battery level
    static BatteryLevel batteryLevel_;
#endif

    // keep track which HW presets have been read so far
    static bool isInitBoot_;
    static byte specialMsgNum;

    static byte nextMessageNum;
    static queue<ByteVector> msgQueue;
    static deque<CmdData> currentCommand;
    static deque<AckData> pendingLooperAcks;

    static bool sendMessageToBT(ByteVector &msg);
    static bool triggerCommand(vector<CmdData> &msg);
    static bool sendNextRequest();

    // Retrieves the current preset from Spark (required for HW presets)
    static void setAmpParameters();
    static void readPresetChecksums();

    // methods to process any data from Spark (process with SparkStreamReader and send ack if required)
    static void handleSendingAck(const ByteVector &blk);
    static void handleAmpModeRequest();
    static void handleAppModeResponse();
    static void handleIncomingAck();

    // Read in all HW presets
    static bool updateLooperSettings();
    static void startLooperTimer(void *args);
    static void updateLooperCommand(byte lastCommand);
};

#endif /* SPARKDATACONTROL_H_ */
