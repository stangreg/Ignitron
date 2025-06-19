/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkDataControl.h"

SparkBTControl *SparkDataControl::bleControl = nullptr;
SparkStreamReader SparkDataControl::sparkSsr;
SparkStatus &SparkDataControl::statusObject = SparkStatus::getInstance();
SparkMessage SparkDataControl::sparkMsg;

SparkDisplayControl *SparkDataControl::sparkDisplay = nullptr;
SparkKeyboardControl *SparkDataControl::keyboardControl = nullptr;
SparkLooperControl SparkDataControl::looperControl_;
SparkBLEKeyboard SparkDataControl::bleKeyboard = SparkBLEKeyboard();

queue<ByteVector> SparkDataControl::msgQueue;
deque<CmdData> SparkDataControl::currentCommand;
deque<AckData> SparkDataControl::pendingLooperAcks;

byte SparkDataControl::nextMessageNum = 0x01;

bool SparkDataControl::ampNameReceived_ = false;

// LooperSetting *SparkDataControl::looperSetting_ = nullptr;
int SparkDataControl::tapEntrySize = 5;
CircularBuffer SparkDataControl::tapEntries(tapEntrySize);
bool SparkDataControl::recordStartFlag = false;

vector<CmdData>
    SparkDataControl::ackMsg;
vector<CmdData> SparkDataControl::currentMsg;

bool SparkDataControl::customPresetNumberChangePending = false;
OperationMode SparkDataControl::operationMode_ = SPARK_MODE_APP;
SubMode SparkDataControl::subMode_ = SUB_MODE_PRESET;

BTMode SparkDataControl::currentBTMode_ = BT_MODE_BLE;
OperationMode SparkDataControl::sparkModeAmp = SPARK_MODE_AMP;
OperationMode SparkDataControl::sparkModeApp = SPARK_MODE_APP;
AmpType SparkDataControl::sparkAmpType = AMP_TYPE_40;
string SparkDataControl::sparkAmpName = AMP_NAME_SPARK_40;
bool SparkDataControl::withDelay = false;
ByteVector SparkDataControl::checksums = {};

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
BatteryLevel SparkDataControl::batteryLevel_ = BATTERY_LEVEL_0;
#endif

bool SparkDataControl::isInitBoot_ = true;
byte SparkDataControl::specialMsgNum = 0xEE;

SparkDataControl::SparkDataControl() {
    // init();
    bleControl = new SparkBTControl(this);
    keyboardControl = new SparkKeyboardControl();
    keyboardControl->init();
    tapEntries = CircularBuffer(tapEntrySize);
}

SparkDataControl::~SparkDataControl() {
    if (bleControl)
        delete bleControl;
    if (sparkDisplay)
        delete sparkDisplay;
    if (keyboardControl)
        delete keyboardControl;
}

OperationMode SparkDataControl::init(OperationMode opModeInput) {
    operationMode_ = opModeInput;

    tapEntries = CircularBuffer(tapEntrySize);

    readOpModeFromFile();
    SparkPresetControl::getInstance().init();

    // Define MAC address required for keyboard
    uint8_t macKeyboard[] = {0xB4, 0xE6, 0x2D, 0xB2, 0x1B, 0x36}; //{0x36, 0x33, 0x33, 0x33, 0x33, 0x33};

    switch (operationMode_) {
    case SPARK_MODE_APP:
        // Set MAC address for BLE keyboard
        esp_base_mac_addr_set(&macKeyboard[0]);

        // initialize BLE
        bleKeyboard.setName("Ignitron BLE");
        bleKeyboard.begin();
        // delay(2000);
        bleKeyboard.end();
        bleControl->initBLE(&bleNotificationCallback);
        DEBUG_PRINTLN("Starting regular check for empty HW presets.");

        xTaskCreatePinnedToCore(
            startLooperTimer,
            "LooperTimer",
            10000,
            NULL,
            0,
            NULL,
            1);
        break;
    case SPARK_MODE_AMP:
        readBTModeFromFile();
        if (currentBTMode_ == BT_MODE_BLE) {
            bleControl->startServer();
        } else if (currentBTMode_ == BT_MODE_SERIAL) {
            bleControl->startBTSerial();
        }
        readPresetChecksums();
        break;
    case SPARK_MODE_KEYBOARD:
        // Set MAC address for BLE keyboard
        esp_base_mac_addr_set(&macKeyboard[0]);

        // initialize BLE
        bleKeyboard.setName("Ignitron BLE");
        bleKeyboard.begin();
        break;
    }

    return operationMode_;
}

void SparkDataControl::switchSubMode(SubMode subMode) {
    // TODO: Check if that works fine
    if (subMode == SUB_MODE_LOOPER) {
        bleKeyboard.start();
    } else {
        bleKeyboard.end();
    }
    // Switch off tuner mode at amp if was enabled before but is not matching current subMode
    if (subMode_ == SUB_MODE_TUNER && subMode_ != subMode) {
        switchTuner(false);
    }
    if (subMode == SUB_MODE_TUNER) {
        switchTuner(true);
    }
    subMode_ = subMode;
    SparkPresetControl::getInstance().updatePendingWithActive();
}

bool SparkDataControl::toggleSubMode() {

    if (!processAction() || operationMode_ == SPARK_MODE_AMP) {
        Serial.println("Spark Amp not connected or in AMP mode, doing nothing.");
        return false;
    }

    Serial.print("Switching to ");
    if (operationMode_ == SPARK_MODE_APP) {
        switch (subMode_) {
        case SUB_MODE_FX:
            Serial.println("PRESET mode");
            subMode_ = SUB_MODE_PRESET;
            break;
        case SUB_MODE_PRESET:
            Serial.println("FX mode");
            subMode_ = SUB_MODE_FX;
            SparkPresetControl::getInstance().updatePendingWithActive();
            break;
        case SUB_MODE_LOOP_CONTROL:
            Serial.println("Looper CONFIG mode");
            subMode_ = SUB_MODE_LOOP_CONFIG;
            break;
        case SUB_MODE_LOOP_CONFIG:
            Serial.println("Looper CONTROL mode");
            subMode_ = SUB_MODE_LOOP_CONTROL;
            SparkPresetControl::getInstance().updatePendingWithActive();
            break;
        default:
            Serial.println("Unexpected mode. Defaulting to PRESET mode");
            subMode_ = SUB_MODE_PRESET;
            break;
        } // SWITCH
    }
    return true;
}

bool SparkDataControl::toggleLooperAppMode() {

    if (operationMode_ == SPARK_MODE_AMP || !processAction()) {
        Serial.println("Spark Amp not connected or in AMP mode, doing nothing.");
        return false;
    }
    SubMode newSubMode;
    Serial.print("Switching to ");
    switch (subMode_) {
    case SUB_MODE_PRESET:
    case SUB_MODE_FX:
        Serial.println("LOOPER mode");
        if (sparkAmpName == AMP_NAME_SPARK_2) {
            newSubMode = SUB_MODE_LOOP_CONTROL;
            looperControl_.stop();
            looperControl_.reset();
            sparkLooperGetConfig();
            sparkLooperGetStatus();
        } else {
            newSubMode = SUB_MODE_LOOPER;
        }
        break;
    case SUB_MODE_LOOPER:
    case SUB_MODE_SPK_LOOPER:
        Serial.println("APP mode");
        newSubMode = SUB_MODE_PRESET;
        looperControl_.triggerReset();
        break;
    default:
        Serial.println("Unexpected mode. Defaulting to APP mode");
        newSubMode = SUB_MODE_PRESET;
        break;
    } // SWITCH
    switchSubMode(newSubMode);
    return true;
}

void SparkDataControl::setDisplayControl(SparkDisplayControl *display) {
    sparkDisplay = display;
}

void SparkDataControl::restartESP(bool resetSparkMode) {
    // RESET Ignitron
    Serial.print("!!! Restarting !!! ");
    if (resetSparkMode) {
        Serial.print("Resetting Spark mode");
        bool sparkModeFileExists = LittleFS.exists(sparkModeFileName.c_str());
        if (sparkModeFileExists) {
            LittleFS.remove(sparkModeFileName.c_str());
        }
    }
    Serial.println();
    ESP.restart();
}

void SparkDataControl::readOpModeFromFile() {
    OperationMode sparkModeInput;
    Serial.println("Reading opmode file.");
    if (!LittleFS.exists(sparkModeFileName.c_str())) {
        Serial.println("Spark mode config file does not exist.");
        return;
    }
    File file = LittleFS.open(sparkModeFileName.c_str());
    string line;

    if (!(file)) {
        Serial.println("Error reading Spark mode file.");
        return;
    }

    while (file.available()) {
        line += file.read();
    }
    file.close();
    Serial.printf("OPMode: %s\n", line.c_str());

    sparkModeInput = (OperationMode)(line[0] - '0'); // was: stoi(line);

    if (sparkModeInput != 0) {
        operationMode_ = sparkModeInput;
        Serial.printf("Reading operation mode from file: %d\n", sparkModeInput);
    }
}

void SparkDataControl::readBTModeFromFile() {
    string line;
    File file = LittleFS.open(btModeFileName.c_str());

    while (file.available()) {
        line += file.read();
    }
    Serial.printf("BTMode: %s\n", line.c_str());
    file.close();
    currentBTMode_ = (BTMode)(line[0] - '0'); // was: stoi(line);
}

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
void SparkDataControl::updateBatteryLevel() {
#if BATTERY_TYPE == BATTERY_TYPE_LI_ION || BATTERY_TYPE == BATTERY_TYPE_LI_FE_PO4
    const int analogReading = analogRead(BATTERY_VOLTAGE_ADC_PIN);
    // analogReading ranges from 0 to 4095
    // 0V = 0, 3.3V = 4095 (BATTERY_MAX_LEVEL)
    float analogVoltage = (analogReading / BATTERY_MAX_LEVEL) * 3.3;
    float batteryVoltage = analogVoltage / VOLTAGE_DIVIDER_R1 * (VOLTAGE_DIVIDER_R1 + VOLTAGE_DIVIDER_R2);
#elif BATTERY_TYPE == BATTERY_TYPE_AMP
    float batteryVoltage = SparkStatus::getInstance().ampBatteryLevel();
#endif

    // Set battery level
    batteryLevel_ = batteryVoltage < BATTERY_CAPACITY_VOLTAGE_THRESHOLD_10
                        ? BATTERY_LEVEL_0
                    : batteryVoltage < BATTERY_CAPACITY_VOLTAGE_THRESHOLD_50
                        ? BATTERY_LEVEL_1
                    : batteryVoltage < BATTERY_CAPACITY_VOLTAGE_THRESHOLD_90
                        ? BATTERY_LEVEL_2
                        : BATTERY_LEVEL_3;

#if BATTERY_TYPE == BATTERY_TYPE_AMP
    if (SparkStatus::getInstance().ampBatteryChargingStatus() == BATTERY_CHARGING_STATUS_CHARGING) {
        batteryLevel_ = BATTERY_LEVEL_CHARGING;
    }
#endif
}
#endif

void SparkDataControl::resetStatus() {
    Serial.println("Resetting Status");
    ampNameReceived_ = false;
    isInitBoot_ = true;
    operationMode_ = SPARK_MODE_APP;
    subMode_ = SUB_MODE_PRESET;
    nextMessageNum = 0x01;
    customPresetNumberChangePending = false;
    sparkAmpType = AMP_TYPE_40;
    sparkAmpName = "Spark 40";
    withDelay = false;
    lastAmpBatteryUpdate = 0;
    SparkPresetControl::getInstance().resetStatus();
    SparkStatus::getInstance().resetStatus();
}

/////////////////////////////////////////////////////////
// SPARK COMMUNICATION CONTROL
/////////////////////////////////////////////////////////

void SparkDataControl::setAmpParameters() {

    string ampName = sparkAmpName;
    DEBUG_PRINTF("Amp name: %s\n", ampName.c_str());
    if (ampName == AMP_NAME_SPARK_40 || ampName == AMP_NAME_SPARK_GO || ampName == AMP_NAME_SPARK_NEO) {
        sparkMsg.maxChunkSizeToSpark() = 0x80;
        sparkMsg.maxBlockSizeToSpark() = 0xAD;
        sparkMsg.withHeader() = true;
        bleControl->setMaxBleMsgSize(0xAD);
        withDelay = false;
    }
    if (ampName == AMP_NAME_SPARK_MINI || ampName == AMP_NAME_SPARK_2) { // || ampName == AMP_NAME_SPARK_NEO) {
        sparkMsg.maxChunkSizeToSpark() = 0x80;
        sparkMsg.maxBlockSizeToSpark() = 0xAD;
        sparkMsg.withHeader() = true;
        bleControl->setMaxBleMsgSize(0x64);
        withDelay = true;
    }
    sparkMsg.maxChunkSizeFromSpark() = 0x19;
    sparkMsg.maxBlockSizeFromSpark() = 0x6A;

    SparkPresetControl::getInstance().setAmpParameters(ampName);
}

void SparkDataControl::readPresetChecksums() {
    SparkPresetControl &presetControl = SparkPresetControl::getInstance();
    checksums.clear();
    for (int i = 1; i <= PRESETS_PER_BANK; i++) {
        Preset preset = presetControl.getPreset(1, i);
        byte checksum = sparkMsg.getPresetChecksum(preset);
        checksums.push_back(checksum);
    }
}

void SparkDataControl::checkForUpdates() {

    if (msgQueue.size() > 0) {
        processSparkData(msgQueue.front());
        msgQueue.pop();
    }

    SparkPresetControl::getInstance().checkForUpdates(operationMode_);

    if (recordStartFlag) {
        if (looperControl_.currentBar() != 0) {
            sparkLooperCommand(SPK_LOOPER_CMD_REC);
            recordStartFlag = false;
        }
    }

    if (statusObject.isLooperSettingUpdated()) {
        looperControl_.setLooperSetting(statusObject.currentLooperSetting());
        statusObject.resetLooperSettingUpdateFlag();
    }

    const LooperSetting &looperSetting = looperControl_.looperSetting();
    if (looperSetting.changePending) {
        updateLooperSettings();
        looperControl_.resetChangePending();
    }

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
#if BATTERY_TYPE == BATTERY_TYPE_AMP
    unsigned int currentTime = millis();
    if (lastAmpBatteryUpdate == 0 || (currentTime - lastAmpBatteryUpdate > updateAmpBatteryInterval)) {
        Serial.println("Reading current battery level");
        lastAmpBatteryUpdate = currentTime;
        currentMsg = sparkMsg.getAmpStatus(nextMessageNum);
        triggerCommand(currentMsg);
    }
#endif
#endif

    if (operationMode_ == SPARK_MODE_AMP) {

        // Read incoming (serial) Bluetooth data, if available
        while (bleControl && bleControl->byteAvailable()) {
            byte inputByte = bleControl->readByte();
            currentBTMsg.push_back(inputByte);
            int msgSize = currentBTMsg.size();
            if (msgSize > 0) {
                if (currentBTMsg[msgSize - 1] == 0xF7) {
                    // DEBUG_PRINTF("Free Heap size: %d\n", ESP.getFreeHeap()); DEBUG_PRINTF("Max free Heap block: %d\n",
                    //		ESP.getMaxAllocHeap());

                    DEBUG_PRINTLN("Received a message");
                    DEBUG_PRINTVECTOR(currentBTMsg);
                    DEBUG_PRINTLN();
                    // heap_caps_check_integrity_all(true) ;
                    processSparkData(currentBTMsg);

                    currentBTMsg.clear();
                }
            }
        }
    }
}

void SparkDataControl::processSparkData(ByteVector &blk) {

    /*DEBUG_PRINT("Received data: ");
    DEBUG_PRINTVECTOR(blk);
    DEBUG_PRINTLN();
    */
    // Check if incoming message requires sending an acknowledgment
    handleSendingAck(blk);

    MessageProcessStatus retCode = sparkSsr.processBlock(blk);
    if (retCode == MSG_PROCESS_RES_REQUEST && operationMode_ == SPARK_MODE_AMP) {
        handleAmpModeRequest();
    }
    if (retCode == MSG_PROCESS_RES_COMPLETE) {
        handleAppModeResponse();
    }

    handleIncomingAck();
}

bool SparkDataControl::processAction() {

    if (operationMode_ == SPARK_MODE_AMP) {
        return true;
    }
    return isAmpConnected();
}

bool SparkDataControl::getCurrentPresetFromSpark() {
    int hwPreset = -1;
    currentMsg = sparkMsg.getCurrentPreset(nextMessageNum, hwPreset);
    Serial.println("Getting current preset from Spark");

    return triggerCommand(currentMsg);
}

bool SparkDataControl::switchPreset(int pre, bool isInitial) {

    return SparkPresetControl::getInstance().switchPreset(pre, isInitial);
}

bool SparkDataControl::changeHWPreset(int preset) {

    currentMsg = sparkMsg.changeHardwarePreset(nextMessageNum, preset);
    return triggerCommand(currentMsg);
}

bool SparkDataControl::changePreset(Preset preset) {
    currentMsg = sparkMsg.changePreset(preset, DIR_TO_SPARK, nextMessageNum);
    if (triggerCommand(currentMsg)) {
        customPresetNumberChangePending = true;
        return true;
    }
    return false;
}

bool SparkDataControl::switchEffectOnOff(const string &fxName, bool enable) {

    SparkPresetControl::getInstance().switchFXOnOff(fxName, enable);
    currentMsg = sparkMsg.turnEffectOnOff(nextMessageNum, fxName, enable);

    return triggerCommand(currentMsg);
}

bool SparkDataControl::toggleEffect(int fxIdentifier) {

    Preset activePreset = SparkPresetControl::getInstance().activePreset();
    if (!processAction() || operationMode_ == SPARK_MODE_AMP) {
        Serial.println("Not connected to Spark Amp or in AMP mode, doing nothing.");
        return false;
    }
    if (activePreset.isEmpty) {
        return false;
    }
    string fxName = activePreset.pedals[fxIdentifier].name;
    bool fxIsOn = activePreset.pedals[fxIdentifier].isOn;

    return switchEffectOnOff(fxName, fxIsOn ? false : true);
}

bool SparkDataControl::getAmpName() {
    currentMsg = sparkMsg.getAmpName(nextMessageNum);
    DEBUG_PRINTLN("Getting amp name from Spark");

    return triggerCommand(currentMsg);
}

bool SparkDataControl::getCurrentPresetNum() {
    currentMsg = sparkMsg.getCurrentPresetNum(nextMessageNum);
    DEBUG_PRINTLN("Getting current preset num from Spark");

    return triggerCommand(currentMsg);
}

bool SparkDataControl::getSerialNumber() {
    currentMsg = sparkMsg.getSerialNumber(nextMessageNum);
    DEBUG_PRINTLN("Getting serial number from Spark");

    return triggerCommand(currentMsg);
}

bool SparkDataControl::getFirmwareVersion() {
    currentMsg = sparkMsg.getFirmwareVersion(nextMessageNum);
    DEBUG_PRINTLN("Getting firmware version from Spark");

    return triggerCommand(currentMsg);
}

bool SparkDataControl::getHWChecksums() {
    DEBUG_PRINTLN("Getting checksums from Spark");
    if (sparkAmpName == AMP_NAME_SPARK_2) {
        currentMsg = sparkMsg.getHWChecksumsExtended(nextMessageNum);
    } else {
        currentMsg = sparkMsg.getHwChecksums(nextMessageNum);
    }
    return triggerCommand(currentMsg);
}

bool SparkDataControl::getCurrentPreset(int num) {
    currentMsg = sparkMsg.getCurrentPreset(nextMessageNum, num);
    DEBUG_PRINTLN("Getting preset information from Spark");

    return triggerCommand(currentMsg);
}

bool SparkDataControl::triggerCommand(vector<CmdData> &msg) {
    nextMessageNum++;
    if (msg.size() > 0) {
        currentCommand.assign(msg.begin(), msg.end());
    }
    // sparkSsr.clearMessageBuffer();
    DEBUG_PRINTLN("Sending message via BT.");
    return sendNextRequest();
    // sparkSsr.clearMessageBuffer();
}

bool SparkDataControl::sendNextRequest() {
    if (currentCommand.size() > 0) {
        CmdData request = currentCommand.front();
        ByteVector firstBlock = request.data;
        AckData currRequest;
        currRequest.cmd = request.cmd;
        currRequest.subcmd = request.subcmd;
        currRequest.detail = request.detail;
        currRequest.msgNum = request.msgNum;

        if (sendMessageToBT(firstBlock)) {
            pendingLooperAcks.push_back(currRequest);
            if (currentCommand.size() > 0) {
                currentCommand.pop_front();
            }
            return true;
        }
    }
    return false;
}

void SparkDataControl::handleSendingAck(const ByteVector &blk) {
    bool ackNeeded;
    byte seq, subCmd;

    // Check if ack needed. In positive case the sequence number and command
    // are also returned to send back to requester
    tie(ackNeeded, seq, subCmd) = sparkSsr.needsAck(blk);
    if (ackNeeded) {
        DEBUG_PRINTLN("ACK required");
        if (operationMode_ == SPARK_MODE_AMP) {
            ackMsg = sparkMsg.sendAck(seq, subCmd, DIR_FROM_SPARK);
        } else {
            ackMsg = sparkMsg.sendAck(seq, subCmd, DIR_TO_SPARK);
        }

        DEBUG_PRINTLN("Sending acknowledgment");
        if (operationMode_ == SPARK_MODE_APP) {
            triggerCommand(ackMsg);
        } else if (operationMode_ == SPARK_MODE_AMP) {
            bleControl->notifyClients(ackMsg);
        }
    }
}

void SparkDataControl::handleAmpModeRequest() {

    vector<CmdData> msg;
    vector<CmdData> currentMessage = sparkSsr.lastMessage();
    byte currentMessageNum = statusObject.lastMessageNum();
    byte subCmd_ = currentMessage.back().subcmd;
    SparkPresetControl &presetControl = SparkPresetControl::getInstance();

    Preset preset;

    MessageType lastMessageType = statusObject.lastMessageType();
    bool sendMessage = true;
    switch (lastMessageType) {

    case MSG_REQ_SERIAL:
        DEBUG_PRINTLN("Found request for serial number");
        msg = sparkMsg.sendSerialNumber(
            currentMessageNum);
        break;
    case MSG_REQ_FW_VER:
        DEBUG_PRINTLN("Found request for firmware version");
        msg = sparkMsg.sendFirmwareVersion(
            currentMessageNum);
        break;
    case MSG_REQ_PRESET_CHK:
        DEBUG_PRINTLN("Found request for hw checksum");
        msg = sparkMsg.sendHWChecksums(currentMessageNum, checksums);
        break;
    case MSG_REQ_CURR_PRESET_NUM:
        DEBUG_PRINTLN("Found request for hw preset number");
        msg = sparkMsg.sendHWPresetNumber(currentMessageNum);
        break;
    case MSG_REQ_CURR_PRESET:
        DEBUG_PRINTLN("Found request for current preset");
        preset = presetControl.activePreset();
        preset.presetNumber = 127;
        msg = sparkMsg.changePreset(preset, DIR_FROM_SPARK,
                                    currentMessageNum);
        break;
    case MSG_REQ_PRESET1:
        DEBUG_PRINTLN("Found request for preset 1");
        preset = presetControl.getPreset(1, 1);
        preset.presetNumber = 0;
        msg = sparkMsg.changePreset(preset, DIR_FROM_SPARK, currentMessageNum);
        break;
    case MSG_REQ_PRESET2:
        DEBUG_PRINTLN("Found request for preset 2");
        preset = presetControl.getPreset(1, 2);
        preset.presetNumber = 1;
        DEBUG_PRINTF("Preset NUMBER after init: %02X\n", preset.presetNumber);
        msg = sparkMsg.changePreset(preset, DIR_FROM_SPARK, currentMessageNum);
        break;
    case MSG_REQ_PRESET3:
        DEBUG_PRINTLN("Found request for preset 3");
        preset = presetControl.getPreset(1, 3);
        preset.presetNumber = 2;
        msg = sparkMsg.changePreset(preset, DIR_FROM_SPARK, currentMessageNum);
        break;
    case MSG_REQ_PRESET4:
        DEBUG_PRINTLN("Found request for preset 4");
        preset = presetControl.getPreset(1, 4);
        preset.presetNumber = 3;
        msg = sparkMsg.changePreset(preset, DIR_FROM_SPARK, currentMessageNum);
        break;
    case MSG_REQ_AMP_STATUS:
        DEBUG_PRINTLN("Found request for amp status");
        msg = sparkMsg.sendAmpStatus(currentMessageNum);
        break;
    case MSG_REQ_72:
        DEBUG_PRINTLN("Found request for 02 72");
        msg = sparkMsg.sendResponse72(currentMessageNum);
        break;
    default:
        DEBUG_PRINTF("Found invalid request: %d \n", lastMessageType);
        sendMessage = false;
        break;
    }
    if (sendMessage) {
        bleControl->notifyClients(msg);
    }
}

void SparkDataControl::handleAppModeResponse() {

    string msgStr = sparkSsr.getJson();
    MessageType lastMessageType = statusObject.lastMessageType();
    byte lastMessageNumber = statusObject.lastMessageNum();
    // DEBUG_PRINTF("Last message number: %s\n", SparkHelper::intToHex(lastMessageNumber).c_str());

    if (operationMode_ == SPARK_MODE_APP) {
        bool printMessage = false;

        if (lastMessageType == MSG_TYPE_AMP_NAME) {
            DEBUG_PRINTLN("Last message was amp name.");
            sparkAmpName = statusObject.ampName();
            setAmpParameters();
            getHWChecksums();
            printMessage = true;
            // ampNameReceived_ = true;
        }

        if (lastMessageType == MSG_TYPE_AMP_SERIAL) {
            DEBUG_PRINTLN("Last message was serial number.");
            // reading HW checksums for cache
            getAmpName();
            printMessage = true;
        }

        if (lastMessageType == MSG_TYPE_HWCHECKSUM) {
            printMessage = true;
            SparkPresetControl &presetControl = SparkPresetControl::getInstance();
            presetControl.validateChecksums(statusObject.hwChecksums());
            // try to load last selected preset from filesystem,
            // if not available, read current preset from amp
            if (!presetControl.readLastPresetFromFile()) {
                getCurrentPresetFromSpark();
            };
        }

        if (lastMessageType == MSG_TYPE_HWPRESET) {
            DEBUG_PRINTLN("Received HW Preset response");

            int sparkPresetNumber = statusObject.currentPresetNumber();

            // only change active presetNumber if new number is between 1 and max HW presets,
            // otherwise it is a custom preset number and can be ignored
            SparkPresetControl &presetControl = SparkPresetControl::getInstance();
            int numberOfHWPresets = presetControl.numberOfHWBanks() * PRESETS_PER_BANK;
            if (lastMessageNumber != specialMsgNum && sparkPresetNumber >= 1 && sparkPresetNumber <= numberOfHWPresets) {
                // check if this improves behavior, updating activePreset when new HW preset received
                SparkPresetControl::getInstance().updateFromSparkResponseHWPreset(sparkPresetNumber);
            } else {
                DEBUG_PRINTLN("Received custom preset number (128), ignoring number change");
            }
            printMessage = true;
        }

        if (lastMessageType == MSG_TYPE_PRESET) {
            DEBUG_PRINTLN("Last message was a preset change.");
            printMessage = true;
            // This preset number is between 0 and 3!
            bool isSpecial = lastMessageNumber == specialMsgNum;
            SparkPresetControl::getInstance().updateFromSparkResponsePreset(isSpecial);
        }

        if (lastMessageType == MSG_TYPE_FX_ONOFF) {
            DEBUG_PRINTLN("Last message was a effect change.");
            Pedal receivedEffect = statusObject.currentEffect();
            SparkPresetControl::getInstance().toggleFX(receivedEffect);
            printMessage = true;
        }

        if (lastMessageType == MSG_TYPE_AMPSTATUS) {
            DEBUG_PRINTLN("Last message was amp status");
            int batteryLevel = SparkStatus::getInstance().ampBatteryLevel();
            Serial.printf("Battery level = %d\n", batteryLevel);
        }

        if (lastMessageType == MSG_TYPE_LOOPER_SETTING) {
            DEBUG_PRINTLN("New Looper setting received.");
            printMessage = true;
        }

        if (lastMessageType == MSG_TYPE_LOOPER_STATUS) {
            DEBUG_PRINTLN("New Looper status received (reading only number of loops)");
            int numOfLoops = statusObject.numberOfLoops();
            looperControl_.loopCount() = numOfLoops;
            if (numOfLoops > 0) {
                looperControl_.isRecAvailable() = true;
            }
            printMessage = true;
        }

        if (lastMessageType == MSG_TYPE_LOOPER_COMMAND) {
            DEBUG_PRINTLN("New Looper command received.");
            updateLooperCommand(statusObject.lastLooperCommand());
        }

        if (lastMessageType == MSG_TYPE_TAP_TEMPO) {
            DEBUG_PRINTLN("New BPM setting received.");
            printMessage = true;
        }

        if (lastMessageType == MSG_TYPE_MEASURE) {
            DEBUG_PRINTLN("Measure info received.");
            // float currentMeasure = sparkSsr.getMeasure();
            // looperControl_.setMeasure(currentMeasure);
        }

        if (lastMessageType == MSG_TYPE_TUNER_OUTPUT) {
            // Amp seems to be in tuner mode
            if (subMode_ != SUB_MODE_TUNER) {
                // Switch off tuner
                switchTuner(false);
            }
            // operationMode_ = SUB_MODE_TUNER;
        }

        // TODO: Check if this works
        if (lastMessageType == MSG_TYPE_TUNER_ON) {
            Serial.println("Tuner on received.");
            switchSubMode(SUB_MODE_TUNER);
        }

        // TODO: Check if this works
        if (lastMessageType == MSG_TYPE_TUNER_OFF) {
            Serial.println("Tuner off received.");
            switchSubMode(SUB_MODE_PRESET);
        }

        if (lastMessageType == MSG_TYPE_INPUT_VOLUME) {
            DEBUG_PRINTLN("Input volume received.");
            printMessage = true;
        }

        if (msgStr.length() > 0 && printMessage) {
            Serial.println("Message processed:");
            Serial.println(msgStr.c_str());
        }
    }

    if (operationMode_ == SPARK_MODE_AMP) {
        if (lastMessageType == MSG_TYPE_PRESET) {

            SparkPresetControl::getInstance().updateFromSparkResponseAmpPreset(&msgStr[0]);
            statusObject.resetPresetUpdateFlag();
            statusObject.resetPresetNumberUpdateFlag();
        }
    }
    statusObject.resetLastMessageType();
}

void SparkDataControl::handleIncomingAck() {

    // if last Ack was for preset change (0x01 / 0x38) or effect switch (0x15),
    // confirm pending preset into active
    SparkPresetControl &presetControl = SparkPresetControl::getInstance();

    AckData lastAck = sparkSsr.getLastAckAndEmpty();
    if (lastAck.cmd == 0x05) { // 05 is intermediate ack, not last message
        DEBUG_PRINTLN("Received intermediate ACK");
        if (lastAck.subcmd == 0x01) {
            // send next part of command on intermediate ACK
            sendNextRequest();
        }
    }
    if (lastAck.cmd == 0x04) {
        DEBUG_PRINTLN("Received final ACK");
        if (lastAck.subcmd == 0x01) {
            // only execute preset number change on last ack for preset change
            if (customPresetNumberChangePending) {
                currentMsg = sparkMsg.changeHardwarePreset(nextMessageNum, 128);
                triggerCommand(currentMsg);
                customPresetNumberChangePending = false;
                presetControl.updateActiveWithPendingPreset();
            }
        }
        if (lastAck.subcmd == 0x38) {
            DEBUG_PRINTLN("Received ACK for 0x38 command");
            // getCurrentPresetFromSpark();

            presetControl.updateFromSparkResponseACK();
            presetControl.writeCurrentPresetToFile();
            Serial.println("OK");
        }
        if (lastAck.subcmd == 0x15) {
            SparkPresetControl::getInstance().updateActiveWithPendingPreset();
            Serial.println("OK");
        }
        if (lastAck.subcmd == 0x75) {
            byte msgNum = lastAck.msgNum;
            byte looperCommand;
            for (auto it = pendingLooperAcks.begin(); it != pendingLooperAcks.end(); /*NOTE: no incrementation of the iterator here*/) {
                byte itMsgNum = (*it).msgNum;
                if (itMsgNum == msgNum) {
                    looperCommand = (*it).detail;
                    it = pendingLooperAcks.erase(it); // erase returns the next iterator
                } else {
                    ++it; // otherwise increment it by yourself
                }
            }
            updateLooperCommand(looperCommand);
            Serial.println(looperControl_.getLooperStatus().c_str());
        }
    }
}

void SparkDataControl::readHWPreset(int num) {

    // in case HW presets are missing from the cache, they can be requested
    Serial.printf("Reading missing HW preset %d\n", num);
    currentMsg = sparkMsg.getCurrentPreset(specialMsgNum, num);
    triggerCommand(currentMsg);
}

/////////////////////////////////////////////////////////
// BLUETOOTH RELATED
/////////////////////////////////////////////////////////

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
            // delay(2000);
            return true;
        } else {
            Serial.println("Failed to connect, starting scan");
            bleControl->startScan();
            return false;
        }
    }
    return false;
}

void SparkDataControl::toggleBTMode() {

    if (operationMode_ == SPARK_MODE_AMP) {
        Serial.print("Switching Bluetooth mode to ");
        if (currentBTMode_ == BT_MODE_BLE) {
            Serial.println("Serial");
            currentBTMode_ = BT_MODE_SERIAL;
        } else if (currentBTMode_ == BT_MODE_SERIAL) {
            Serial.println("BLE");
            currentBTMode_ = BT_MODE_BLE;
        }
        // Save new mode to file
        File file = LittleFS.open(btModeFileName.c_str(), FILE_WRITE);
        file.print(currentBTMode_);
        file = LittleFS.open(sparkModeFileName.c_str(), FILE_WRITE);
        file.print(SPARK_MODE_AMP);
        file.close();
        Serial.println("Restarting in new BT mode");
        restartESP(false);
    }
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

    // Triggered when data is received from Spark Amp in APP mode
    //  Transform data into ByteVetor and process
    ByteVector chunk(&pData[0], &pData[length]);
    // DEBUG_PRINT("Incoming block: ");
    // DEBUG_PRINTVECTOR(chunk);
    // DEBUG_PRINTLN();
    //  DEBUG_PRINTF("Is notify: %s\n", isNotify ? "true" : "false");
    //   Add incoming data to message queue for processing
    msgQueue.push(chunk);
    // DEBUG_PRINTF("Seding back data via notify.");
    // vector<ByteVector> notifyVector = { chunk };
    // bleControl->writeBLE(notifyVector, false, false);
}

void SparkDataControl::queueMessage(ByteVector &blk) {
    if (blk.size() > 0) {
        msgQueue.push(blk);
    }
}

bool SparkDataControl::sendMessageToBT(ByteVector &msg) {
    DEBUG_PRINTLN("Sending message via BT.");
    return bleControl->writeBLE(msg, withDelay);
}

/////////////////////////////////////////////////////////
// KEYBOARD RELATED
/////////////////////////////////////////////////////////

void SparkDataControl::sendButtonPressAsKeyboard(keyboardKeyDefinition k) {
    if (bleKeyboard.isConnected()) {

        Serial.printf("Sending button: %d - mod: %d - repeat: %d\n", k.key, k.modifier, k.repeat);
        if (k.modifier != 0)
            bleKeyboard.press(k.modifier);
        for (uint8_t i = 0; i <= k.repeat; i++) {
            bleKeyboard.write(k.key);
        }
        if (k.modifier != 0)
            bleKeyboard.release(k.modifier);
        lastKeyboardButtonPressed_ = k.keyUid;
        lastKeyboardButtonPressedString_ = k.display;
    } else {
        Serial.println("Keyboard not connected");
    }
}

void SparkDataControl::resetLastKeyboardButtonPressed() {
    lastKeyboardButtonPressed_ = 0;
    lastKeyboardButtonPressedString_ = "";
}

/////////////////////////////////////////////////////////
// LOOPER FUNCTIONS
/////////////////////////////////////////////////////////

/* Looper functions:
//
// 02 = RECORD count in (done)
// 04 = RECORD (done)
// 05 = STOPREC + PLAY(??)
// 06 = RETRY (followed by 02 and 04);
// 07 = RECORD finished??
// 08 = PLAY (done)
// 09 = STOP (done)
// 0b = RECORD (Dub) (done)
// 0c = STOP RECORD (done)
// 0d = UNDO (done)
// 0e = REDO (done)
// 0a = DELETE (done)
Looper commands end */

bool SparkDataControl::sparkLooperCommand(LooperCommand command) {

    currentMsg = sparkMsg.sparkLooperCommand(nextMessageNum, command);
    DEBUG_PRINTF("Spark Looper: %02x\n", command);

    return triggerCommand(currentMsg);
}

void SparkDataControl::tapTempoButton() {

    int bpm;
    unsigned long now = millis();
    unsigned long diff = now - lastTapButtonPressed_;
    DEBUG_PRINTF("Tap data: now = %u, lastTapButton = %u, diff = %u\n", now, lastTapButtonPressed_, diff);
    lastTapButtonPressed_ = now;

    if (diff > tapButtonThreshold_) {
        DEBUG_PRINTLN("Restarting tap calc");
        tapEntries.reset();
        bpm = 0;
    } else {
        tapEntries.add_element(diff);
        if (tapEntries.size() > 3) {
            int averageTime = tapEntries.averageValue();
            if (averageTime > 0) {
                bpm = 60000 / averageTime;
                bpm = min(bpm, 255);
                bpm = max(30, bpm);
                DEBUG_PRINTF("Tap tempo: %d\n", bpm);
                looperControl_.changeSettingBpm(bpm);
            }
        }
    }
}

bool SparkDataControl::switchTuner(bool on) {
    DEBUG_PRINTF("Switching Tuner %s\n", on ? "on" : "off");
    currentMsg = sparkMsg.switchTuner(nextMessageNum, on);
    return triggerCommand(currentMsg);
}

bool SparkDataControl::updateLooperSettings() {
    DEBUG_PRINTF("Updating looper settings: %s\n", looperControl_.looperSetting().getJson().c_str());
    currentMsg = sparkMsg.updateLooperSettings(nextMessageNum, looperControl_.looperSetting());
    return triggerCommand(currentMsg);
}

void SparkDataControl::startLooperTimer(void *args) {
    looperControl_.run(args);
}

void SparkDataControl::updateLooperCommand(byte lastCommand) {
    DEBUG_PRINTF("Last looper command: %02x\n", lastCommand);
    switch (lastCommand) {
    case 0x02:
        // Count in started
        break;
    case 0x04:
        // Record started
        looperControl_.isRecRunning() = true;
        break;
    case 0x05:
        // To be analyzed
        break;
    case 0x06:
        // Retry recording, maybe not required
        break;
    case 0x07:
        // Recording finished
        looperControl_.isRecRunning() = false;
        looperControl_.isRecAvailable() = true;
        break;
    case 0x08:
        // Play
        looperControl_.isPlaying() = true;
        break;
    case 0x09:
        // Stop
        looperControl_.isPlaying() = false;
        break;
    case 0x0A:
        // Delete
        looperControl_.resetStatus();
        break;
    case 0x0B:
        // Dub
        looperControl_.isRecRunning() = true;
        break;
    case 0x0C:
        // Stop Recording
        looperControl_.isRecRunning() = false;
        looperControl_.isRecAvailable() = true;
        break;
    case 0x0D:
        // UNDO
        looperControl_.canRedo() = true;
        break;
    case 0x0E:
        looperControl_.canRedo() = false;
        break;
    default:
        DEBUG_PRINTLN("Unknown looper command received.");
        break;
    }
    Serial.println(looperControl_.getLooperStatus().c_str());
}

bool SparkDataControl::sparkLooperStopAll() {
    bool stopReturn = sparkLooperStopPlaying();
    bool recStopReturn = sparkLooperStopRec();
    return stopReturn && recStopReturn;
}

bool SparkDataControl::sparkLooperStopPlaying() {
    // looperControl_.isPlaying() = false;
    bool retValue = sparkLooperCommand(SPK_LOOPER_CMD_STOP);
    if (retValue) {
        looperControl_.stop();
        looperControl_.reset();
        sparkLooperGetStatus();
    }
    return retValue;
}

bool SparkDataControl::sparkLooperPlay() {
    // looperControl_.isPlaying() = true;
    if (!(looperControl_.isPlaying())) {
        looperControl_.start();
        return sparkLooperCommand(SPK_LOOPER_CMD_PLAY);
    }
    return true;
}

bool SparkDataControl::sparkLooperRec() {
    bool countIn = looperControl_.looperSetting().click;
    if (countIn) {
        sparkLooperCommand(SPK_LOOPER_CMD_COUNTIN);
        looperControl_.setCurrentBar(0);
    }
    looperControl_.start();
    recordStartFlag = true;
    return true;
}

bool SparkDataControl::sparkLooperDub() {
    bool retValue = sparkLooperCommand(SPK_LOOPER_CMD_DUB);
    // looperControl_.reset();
    looperControl_.start();
    retValue = retValue && sparkLooperCommand(SPK_LOOPER_CMD_PLAY);
    looperControl_.isPlaying() = true;
    return retValue;
}

bool SparkDataControl::sparkLooperRetry() {
    sparkLooperCommand(SPK_LOOPER_CMD_RETRY);
    looperControl_.reset();
    return sparkLooperRec();
}
// TODO: Get Looper status on startup and set flags accordingly

bool SparkDataControl::sparkLooperStopRec() {
    bool isRecAvailable = looperControl_.isRecAvailable();
    bool retVal = false;
    if (isRecAvailable) {
        retVal = sparkLooperCommand(SPK_LOOPER_CMD_STOP_DUB);
    } else {
        retVal = sparkLooperCommand(SPK_LOOPER_CMD_STOP_REC);
        retVal = sparkLooperCommand(SPK_LOOPER_CMD_REC_COMPLETE);
        looperControl_.reset();
    }
    sparkLooperGetStatus();
    return retVal;
}

bool SparkDataControl::sparkLooperUndo() {
    if (looperControl_.canUndo()) {
        DEBUG_PRINTLN("UNDO possible");
        return sparkLooperCommand(SPK_LOOPER_CMD_UNDO);
    }
    return true;
}

bool SparkDataControl::sparkLooperRedo() {
    if (looperControl_.canRedo()) {
        DEBUG_PRINTLN("REDO possible");
        return sparkLooperCommand(SPK_LOOPER_CMD_REDO);
    }
    return true;
}

bool SparkDataControl::sparkLooperStopRecAndPlay() {
    // sparkLooperCommand(SPK_LOOPER_CMD_STOPREC);
    sparkLooperStopRec();
    return sparkLooperPlay();
}

bool SparkDataControl::sparkLooperDeleteAll() {
    return sparkLooperCommand(SPK_LOOPER_CMD_DELETE);
}

bool SparkDataControl::sparkLooperPlayStop() {
    bool isRecRunning = looperControl_.isRecRunning();
    bool isPlaying = looperControl_.isPlaying();
    bool result = false;
    if (isRecRunning) {
        result = sparkLooperStopAll();
    } else if (isPlaying) {
        result = sparkLooperStopPlaying();
    } else {
        result = sparkLooperPlay();
    }
    return result;
}

bool SparkDataControl::sparkLooperRecDub() {
    bool isRecRunning = looperControl_.isRecRunning();
    bool isRecAvailable = looperControl_.isRecAvailable();
    int status = 2 * isRecAvailable + isRecRunning;
    bool result = false;

    switch (status) {
    case 0:
        // not running and not available
        result = sparkLooperRec();
        break;
    case 1:
        // not available but running
        result = sparkLooperStopRecAndPlay();
        break;
    case 2:
        // available, not running
        result = sparkLooperDub();
        break;
    case 3:
        // running and available
        result = sparkLooperStopRec();
        break;
    }
    return result;
}

bool SparkDataControl::sparkLooperUndoRedo() {
    // bool isRecRunning = looperControl_.isRecRunning();
    bool isRecAvailable = looperControl_.isRecAvailable();
    bool canRedo = looperControl_.canRedo();
    if (canRedo) {
        DEBUG_PRINTLN("REDO");
        return sparkLooperRedo();
    }
    if (isRecAvailable) {
        DEBUG_PRINTLN("UNDO");
        return sparkLooperUndo();
    }
    return false;
}

bool SparkDataControl::sparkLooperGetStatus() {
    bool retValue;
    currentMsg = sparkMsg.getLooperStatus(nextMessageNum);
    return triggerCommand(currentMsg);
}

bool SparkDataControl::sparkLooperGetConfig() {
    currentMsg = sparkMsg.getLooperConfig(nextMessageNum);
    return triggerCommand(currentMsg);
}

bool SparkDataControl::sparkLooperGetRecordStatus() {
    currentMsg = sparkMsg.getLooperRecordStatus(nextMessageNum);
    return triggerCommand(currentMsg);
}
