/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkDataControl.h"

SparkBTControl *SparkDataControl::bleControl = nullptr;
SparkStreamReader SparkDataControl::spark_ssr;
SparkStatus &SparkDataControl::statusObject = SparkStatus::getInstance();
SparkMessage SparkDataControl::spark_msg;

SparkDisplayControl *SparkDataControl::spark_display = nullptr;
SparkKeyboardControl *SparkDataControl::keyboardControl = nullptr;
SparkLooperControl *SparkDataControl::looperControl_ = nullptr;

eSPIFFS SparkDataControl::fileSystem;

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
    SparkDataControl::ack_msg;
vector<CmdData> SparkDataControl::current_msg;

bool SparkDataControl::customPresetNumberChangePending = false;
int SparkDataControl::operationMode_ = SPARK_MODE_APP;

int SparkDataControl::currentBTMode_ = BT_MODE_BLE;
int SparkDataControl::sparkModeAmp = SPARK_MODE_AMP;
int SparkDataControl::sparkModeApp = SPARK_MODE_APP;
int SparkDataControl::sparkAmpType = AMP_TYPE_40;
string SparkDataControl::sparkAmpName = "Spark 2";
bool SparkDataControl::with_delay = false;

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
int SparkDataControl::batteryLevel_ = BATTERY_LEVEL_0;
#endif

bool SparkDataControl::isInitBoot_ = true;
byte SparkDataControl::special_msg_num = 0xEE;

SparkDataControl::SparkDataControl() {
    // init();
    bleControl = new SparkBTControl(this);
    keyboardControl = new SparkKeyboardControl();
    keyboardControl->init();
    tapEntries = CircularBuffer(tapEntrySize);
    looperControl_ = new SparkLooperControl();
}

SparkDataControl::~SparkDataControl() {
    if (bleControl)
        delete bleControl;
    if (spark_display)
        delete spark_display;
    if (keyboardControl)
        delete keyboardControl;
    if (looperControl_)
        delete looperControl_;
}

int SparkDataControl::init(int opModeInput) {
    operationMode_ = opModeInput;

    tapEntries = CircularBuffer(tapEntrySize);

    SparkPresetControl::getInstance().init();
    readOpModeFromFile();

    // Define MAC address required for keyboard
    uint8_t mac_keyboard[] = {0xB4, 0xE6, 0x2D, 0xB2, 0x1B, 0x36}; //{0x36, 0x33, 0x33, 0x33, 0x33, 0x33};

    switch (operationMode_) {
    case SPARK_MODE_APP:
        // Set MAC address for BLE keyboard
        esp_base_mac_addr_set(&mac_keyboard[0]);

        // initialize BLE
        bleKeyboard.setName("Ignitron BLE");
        bleKeyboard.begin();
        // delay(2000);
        bleKeyboard.end();
        bleControl->initBLE(&bleNotificationCallback);
        DEBUG_PRINTLN("Starting regular check for empty HW presets.");

        // TODO Check if this is working. In worst case the start function is called once and exits
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

    if (opMode == SPARK_MODE_APP) {
        bleKeyboard.end();
        buttonMode_ = BUTTON_MODE_PRESET;
    } else if (opMode == SPARK_MODE_LOOPER) {
        bleKeyboard.start();
    } else if (opMode == SPARK_MODE_SPK_LOOPER) {
        buttonMode_ = BUTTON_MODE_LOOP_CONTROL;
    }
    SparkPresetControl::getInstance().updatePendingWithActive();
}

bool SparkDataControl::toggleButtonMode() {

    if (!processAction() || operationMode_ == SPARK_MODE_AMP) {
        Serial.println("Spark Amp not connected or in AMP mode, doing nothing.");
        return false;
    }

    Serial.print("Switching to ");
    if (operationMode_ == SPARK_MODE_APP) {
        switch (buttonMode_) {
        case BUTTON_MODE_FX:
            Serial.println("PRESET mode");
            buttonMode_ = BUTTON_MODE_PRESET;
            break;
        case BUTTON_MODE_PRESET:
            Serial.println("FX mode");
            buttonMode_ = BUTTON_MODE_FX;
            SparkPresetControl::getInstance().updatePendingWithActive();
            break;
        default:
            Serial.println("Unexpected mode. Defaulting to PRESET mode");
            buttonMode_ = BUTTON_MODE_PRESET;
            break;
        } // SWITCH
    } else if (operationMode_ == SPARK_MODE_SPK_LOOPER) {
        switch (buttonMode_) {
        case BUTTON_MODE_LOOP_CONTROL:
            Serial.println("CONFIG mode");
            buttonMode_ = BUTTON_MODE_LOOP_CONFIG;
            break;
        case BUTTON_MODE_LOOP_CONFIG:
            Serial.println("CONTROL mode");
            buttonMode_ = BUTTON_MODE_LOOP_CONTROL;
            SparkPresetControl::getInstance().updatePendingWithActive();
            break;
        default:
            Serial.println("Unexpected mode. Defaulting to PRESET mode");
            buttonMode_ = BUTTON_MODE_LOOP_CONTROL;
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
    int newOperationMode;
    Serial.print("Switching to ");
    switch (operationMode_) {
    case SPARK_MODE_APP:
        Serial.println("LOOPER mode");
        if (sparkAmpName == AMP_NAME_SPARK_2) {
            newOperationMode = SPARK_MODE_SPK_LOOPER;
            buttonMode_ = BUTTON_MODE_LOOP_CONTROL;
            looperControl_->stop();
            looperControl_->reset();
            sparkLooperGetConfig();
            sparkLooperGetStatus();

        } else {
            newOperationMode = SPARK_MODE_LOOPER;
        }
        break;
    case SPARK_MODE_LOOPER:
    case SPARK_MODE_SPK_LOOPER:
        Serial.println("APP mode");
        newOperationMode = SPARK_MODE_APP;
        buttonMode_ = BUTTON_MODE_PRESET;
        looperControl_->triggerReset();
        break;
    default:
        Serial.println("Unexpected mode. Defaulting to APP mode");
        newOperationMode = SPARK_MODE_APP;
        buttonMode_ = BUTTON_MODE_PRESET;
        break;
    } // SWITCH
    switchOperationMode(newOperationMode);
    return true;
}

void SparkDataControl::setDisplayControl(SparkDisplayControl *display) {
    spark_display = display;
}

void SparkDataControl::restartESP(bool resetSparkMode) {
    // RESET Ignitron
    Serial.print("!!! Restarting !!! ");
    if (resetSparkMode) {
        Serial.print("Resetting Spark mode");
        bool sparkModeFileExists = SPIFFS.exists(sparkModeFileName.c_str());
        if (sparkModeFileExists) {
            SPIFFS.remove(sparkModeFileName.c_str());
        }
    }
    Serial.println();
    ESP.restart();
}

void SparkDataControl::readOpModeFromFile() {
    string currentSparkModeFile;
    int sparkModeInput = 0;

    fileSystem.openFromFile(sparkModeFileName.c_str(), currentSparkModeFile);

    stringstream sparkModeStream(currentSparkModeFile);
    string line;

    while (getline(sparkModeStream, line)) {
        sparkModeInput = (int)(line[0] - '0'); // was: stoi(line);
    }
    if (sparkModeInput != 0) {
        operationMode_ = sparkModeInput;
        // Serial.printf("Reading operation mode from file: %d.", sparkModeInput);
    }
}

void SparkDataControl::readBTModeFromFile() {
    string currentBTModeFile;
    stringstream btModeStream;
    string line;
    fileSystem.openFromFile(btModeFileName.c_str(), currentBTModeFile);
    btModeStream.str(currentBTModeFile);

    while (getline(btModeStream, line)) {
        currentBTMode_ = (int)(line[0] - '0'); // was: stoi(line);
    }
}

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
void SparkDataControl::updateBatteryLevel() {
    const int analogReading = analogRead(BATTERY_VOLTAGE_ADC_PIN);
    // analogReading ranges from 0 to 4095
    // 0V = 0, 3.3V = 4095
    float analogVoltage = (analogReading / 4095.0) * 3.3;
    float batteryVoltage = analogVoltage / VOLTAGE_DIVIDER_R1 * (VOLTAGE_DIVIDER_R1 + VOLTAGE_DIVIDER_R2);

    // Set battery level
    batteryLevel_ = batteryVoltage < BATTERY_CAPACITY_VOLTAGE_THRESHOLD_10
                        ? BATTERY_LEVEL_0
                    : batteryVoltage < BATTERY_CAPACITY_VOLTAGE_THRESHOLD_50
                        ? BATTERY_LEVEL_1
                    : batteryVoltage < BATTERY_CAPACITY_VOLTAGE_THRESHOLD_90
                        ? BATTERY_LEVEL_2
                        : BATTERY_LEVEL_3;
}
#endif

void SparkDataControl::resetStatus() {
    // isInitBoot_ = true;
    operationMode_ = SPARK_MODE_APP;
    buttonMode_ = BUTTON_MODE_PRESET;
    nextMessageNum = 0x01;
    customPresetNumberChangePending = false;
    sparkAmpType = AMP_TYPE_40;
    sparkAmpName = "Spark 40";
    with_delay = false;
    SparkPresetControl::getInstance().resetStatus();
}

/////////////////////////////////////////////////////////
// SPARK COMMUNICATION CONTROL
/////////////////////////////////////////////////////////

void SparkDataControl::setAmpParameters() {

    string ampName = sparkAmpName;
    DEBUG_PRINTF("Amp name: %s\n", ampName.c_str());
    if (ampName == AMP_NAME_SPARK_40 || ampName == AMP_NAME_SPARK_GO) {
        spark_msg.maxChunkSizeToSpark() = 0x80;
        spark_msg.maxBlockSizeToSpark() = 0xAD;
        spark_msg.withHeader() = true;
        bleControl->setMaxBleMsgSize(0xAD);
        with_delay = false;
    }
    if (ampName == AMP_NAME_SPARK_MINI || ampName == AMP_NAME_SPARK_2) {
        spark_msg.maxChunkSizeToSpark() = 0x80;
        spark_msg.maxBlockSizeToSpark() = 0xAD;
        spark_msg.withHeader() = true;
        bleControl->setMaxBleMsgSize(0x64);
        with_delay = false;
    }
    spark_msg.maxChunkSizeFromSpark() = 0x19;
    spark_msg.maxBlockSizeFromSpark() = 0x6A;
}

void SparkDataControl::checkForUpdates() {

    while (msgQueue.size() > 0) {
        processSparkData(msgQueue.front());
        msgQueue.pop();
    }

    SparkPresetControl::getInstance().checkForUpdates(operationMode_);

    if (recordStartFlag) {
        if (looperControl_->currentBar() != 0) {
            sparkLooperCommand(SPK_LOOPER_CMD_REC);
            recordStartFlag = false;
        }
    }

    if (statusObject.isLooperSettingUpdated()) {
        looperControl_->setLooperSetting(statusObject.currentLooperSetting());
        statusObject.resetLooperSettingUpdateFlag();
    }

    const LooperSetting *looperSetting = looperControl_->looperSetting();
    if (looperSetting->changePending) {
        updateLooperSettings();
        looperControl_->resetChangePending();
    }

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

    // Check if incoming message requires sending an acknowledgment
    handleSendingAck(blk);

    int retCode = spark_ssr.processBlock(blk);
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
    int hw_preset = -1;
    current_msg = spark_msg.get_current_preset(nextMessageNum, hw_preset);
    Serial.println("Getting current preset from Spark");

    return triggerCommand(current_msg);
}

bool SparkDataControl::switchPreset(int pre, bool isInitial) {

    DEBUG_PRINTLN("Entering switchPreset()");
    return SparkPresetControl::getInstance().switchPreset(pre, isInitial);
}

bool SparkDataControl::changeHWPreset(int preset) {

    current_msg = spark_msg.change_hardware_preset(nextMessageNum, preset);
    return triggerCommand(current_msg);
}

bool SparkDataControl::changePreset(Preset preset) {
    current_msg = spark_msg.change_preset(preset, DIR_TO_SPARK, nextMessageNum);
    if (triggerCommand(current_msg)) {
        customPresetNumberChangePending = true;
        return true;
    }
    return false;
}

bool SparkDataControl::switchEffectOnOff(const string &fx_name, bool enable) {

    SparkPresetControl::getInstance().switchFXOnOff(fx_name, enable);
    current_msg = spark_msg.turn_effect_onoff(nextMessageNum, fx_name, enable);

    return triggerCommand(current_msg);
}

bool SparkDataControl::toggleEffect(int fx_identifier) {

    Preset activePreset = SparkPresetControl::getInstance().activePreset();
    if (!processAction() || operationMode_ == SPARK_MODE_AMP) {
        Serial.println("Not connected to Spark Amp or in AMP mode, doing nothing.");
        return false;
    }
    if (activePreset.isEmpty) {
        return false;
    }
    string fx_name = activePreset.pedals[fx_identifier].name;
    bool fx_isOn = activePreset.pedals[fx_identifier].isOn;

    return switchEffectOnOff(fx_name, fx_isOn ? false : true);
}

bool SparkDataControl::getAmpName() {
    current_msg = spark_msg.get_amp_name(nextMessageNum);
    DEBUG_PRINTLN("Getting amp name from Spark");

    return triggerCommand(current_msg);
}

bool SparkDataControl::getCurrentPresetNum() {
    current_msg = spark_msg.get_current_preset_num(nextMessageNum);
    DEBUG_PRINTLN("Getting current preset num from Spark");

    return triggerCommand(current_msg);
}

bool SparkDataControl::getSerialNumber() {
    current_msg = spark_msg.get_serial_number(nextMessageNum);
    DEBUG_PRINTLN("Getting serial number from Spark");

    return triggerCommand(current_msg);
}

bool SparkDataControl::getFirmwareVersion() {
    current_msg = spark_msg.get_firmware_version(nextMessageNum);
    DEBUG_PRINTLN("Getting firmware version from Spark");

    return triggerCommand(current_msg);
}

bool SparkDataControl::getHWChecksums() {
    current_msg = spark_msg.get_hw_checksums(nextMessageNum);
    DEBUG_PRINTLN("Getting checksums from Spark");

    return triggerCommand(current_msg);
}

bool SparkDataControl::getCurrentPreset(int num) {
    current_msg = spark_msg.get_current_preset(nextMessageNum, num);
    DEBUG_PRINTLN("Getting preset information from Spark");

    return triggerCommand(current_msg);
}

bool SparkDataControl::triggerCommand(vector<CmdData> &msg) {
    nextMessageNum++;
    if (msg.size() > 0) {
        currentCommand.assign(msg.begin(), msg.end());
    }
    // spark_ssr.clearMessageBuffer();
    DEBUG_PRINTLN("Sending message via BT.");
    return sendNextRequest();
    // spark_ssr.clearMessageBuffer();
}

bool SparkDataControl::sendNextRequest() {
    if (currentCommand.size() > 0) {
        CmdData request = currentCommand.front();
        ByteVector firstBlock = request.data;
        AckData curr_request;
        curr_request.cmd = request.cmd;
        curr_request.subcmd = request.subcmd;
        curr_request.detail = request.detail;
        curr_request.msg_num = request.msg_num;

        if (sendMessageToBT(firstBlock)) {
            pendingLooperAcks.push_back(curr_request);
            currentCommand.pop_front();
            return true;
        }
    }
    return false;
}

void SparkDataControl::handleSendingAck(const ByteVector &blk) {
    bool ackNeeded;
    byte seq, sub_cmd;

    // Check if ack needed. In positive case the sequence number and command
    // are also returned to send back to requester
    tie(ackNeeded, seq, sub_cmd) = spark_ssr.needsAck(blk);
    if (ackNeeded) {
        DEBUG_PRINTLN("ACK required");
        if (operationMode_ == SPARK_MODE_AMP) {
            ack_msg = spark_msg.send_ack(seq, sub_cmd, DIR_FROM_SPARK);
        } else {
            ack_msg = spark_msg.send_ack(seq, sub_cmd, DIR_TO_SPARK);
        }

        DEBUG_PRINTLN("Sending acknowledgment");
        if (operationMode_ == SPARK_MODE_APP || operationMode_ == SPARK_MODE_LOOPER || operationMode_ == SPARK_MODE_SPK_LOOPER) {
            triggerCommand(ack_msg);
        } else if (operationMode_ == SPARK_MODE_AMP) {
            bleControl->notifyClients(ack_msg);
        }
    }
}

void SparkDataControl::handleAmpModeRequest() {

    vector<CmdData> msg;
    vector<CmdData> currentMessage = spark_ssr.lastMessage();
    byte currentMessageNum = statusObject.lastMessageNum();
    byte sub_cmd_ = currentMessage.back().subcmd;
    Preset activePreset = SparkPresetControl::getInstance().activePreset();

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
        msg = spark_msg.change_preset(activePreset, DIR_FROM_SPARK,
                                      currentMessageNum);
        break;
    case 0x71:
        DEBUG_PRINTLN("Found request for 02 71");
        msg = spark_msg.send_response_71(currentMessageNum);
        break;
    case 0x72:
        DEBUG_PRINTLN("Found request for 02 72");
        msg = spark_msg.send_response_72(currentMessageNum);
        break;
    default:
        DEBUG_PRINTF("Found invalid request: %02x \n", sub_cmd_);
        break;
    }

    bleControl->notifyClients(msg);
}

void SparkDataControl::handleAppModeResponse() {

    string msgStr = spark_ssr.getJson();
    int lastMessageType = statusObject.lastMessageType();
    byte lastMessageNumber = statusObject.lastMessageNum();
    // DEBUG_PRINTF("Last message number: %s\n", SparkHelper::intToHex(lastMessageNumber).c_str());

    if (operationMode_ == SPARK_MODE_APP || operationMode_ == SPARK_MODE_SPK_LOOPER) {
        bool printMessage = false;

        if (lastMessageType == MSG_TYPE_HWPRESET) {
            DEBUG_PRINTLN("Received HW Preset response");

            int spark_presetNumber = statusObject.currentPresetNumber();

            // only change active presetNumber if new number is between 1 and 4,
            // otherwise it is a custom preset number and can be ignored
            if (lastMessageNumber != special_msg_num && spark_presetNumber >= 1 && spark_presetNumber <= 4) {
                // check if this improves behavior, updating activePreset when new HW preset received
                SparkPresetControl::getInstance().updateFromSparkResponseHWPreset(spark_presetNumber);
            } else {
                DEBUG_PRINTLN("Received custom preset number (128), ignoring number change");
            }
            printMessage = true;
        }

        if (lastMessageType == MSG_TYPE_PRESET) {
            DEBUG_PRINTLN("Last message was a preset change.");
            printMessage = true;
            // This preset number is between 0 and 3!
            bool isSpecial = lastMessageNumber == special_msg_num;
            SparkPresetControl::getInstance().updateFromSparkResponsePreset(isSpecial);
        }

        if (lastMessageType == MSG_TYPE_FX_ONOFF) {
            DEBUG_PRINTLN("Last message was a effect change.");
            Pedal receivedEffect = statusObject.currentEffect();
            SparkPresetControl::getInstance().toggleFX(receivedEffect);
            printMessage = true;
        }

        if (lastMessageType == MSG_TYPE_AMP_NAME) {
            DEBUG_PRINTLN("Last message was amp name.");
            sparkAmpName = statusObject.ampName();
            setAmpParameters();
            printMessage = true;
            // ampNameReceived_ = true;
        }

        if (lastMessageType == MSG_TYPE_LOOPER_SETTING) {
            DEBUG_PRINTLN("New Looper setting received.");
            printMessage = true;
        }

        if (lastMessageType == MSG_TYPE_LOOPER_STATUS) {
            DEBUG_PRINTLN("New Looper status received (reading only number of loops)");
            int numOfLoops = statusObject.numberOfLoops();
            looperControl_->loopCount() = numOfLoops;
            if (numOfLoops > 0) {
                looperControl_->isRecAvailable() = true;
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
            // float currentMeasure = spark_ssr.getMeasure();
            // looperControl_->setMeasure(currentMeasure);
        }

        if (msgStr.length() > 0 && printMessage) {
            Serial.println("Message processed:");
            Serial.println(msgStr.c_str());
        }
    }

    if (operationMode_ == SPARK_MODE_AMP) {
        if (lastMessageType == MSG_TYPE_PRESET) {

            SparkPresetControl::getInstance().updateFromSparkResponseAmpPreset(msgStr);
            statusObject.resetPresetUpdateFlag();
            statusObject.resetPresetNumberUpdateFlag();
        }
    }
    statusObject.resetLastMessageType();
}

void SparkDataControl::handleIncomingAck() {

    // if last Ack was for preset change (0x01 / 0x38) or effect switch (0x15),
    // confirm pending preset into active
    AckData lastAck = spark_ssr.getLastAckAndEmpty();
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
                current_msg = spark_msg.change_hardware_preset(nextMessageNum, 128);
                triggerCommand(current_msg);
                customPresetNumberChangePending = false;
                SparkPresetControl::getInstance().updateActiveWithPendingPreset();
            }
        }
        if (lastAck.subcmd == 0x38) {
            DEBUG_PRINTLN("Received ACK for 0x38 command");
            // getCurrentPresetFromSpark();

            SparkPresetControl::getInstance().updateFromSparkResponseACK();
            Serial.println("OK");
        }
        if (lastAck.subcmd == 0x15) {
            // TODO: Move to own method?
            SparkPresetControl::getInstance().updateActiveWithPendingPreset();
            Serial.println("OK");
        }
        if (lastAck.subcmd == 0x75) {
            byte msg_num = lastAck.msg_num;
            byte looperCommand;
            for (auto it = pendingLooperAcks.begin(); it != pendingLooperAcks.end(); /*NOTE: no incrementation of the iterator here*/) {
                byte it_msg_num = (*it).msg_num;
                if (it_msg_num == msg_num) {
                    looperCommand = (*it).detail;
                    it = pendingLooperAcks.erase(it); // erase returns the next iterator
                } else {
                    ++it; // otherwise increment it by yourself
                }
            }
            updateLooperCommand(looperCommand);
            Serial.println(looperControl_->getLooperStatus().c_str());
        }
    }
}

void SparkDataControl::readHWPreset(int num) {

    // in case HW presets are missing from the cache, they can be requested
    Serial.printf("Reading missing HW preset %d\n", num);
    current_msg = spark_msg.get_current_preset(special_msg_num, num);
    triggerCommand(current_msg);
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
        fileSystem.saveToFile(btModeFileName.c_str(), currentBTMode_);
        fileSystem.saveToFile(sparkModeFileName.c_str(), sparkModeAmp);
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

bool SparkDataControl::sendMessageToBT(ByteVector &msg) {
    DEBUG_PRINTLN("Sending message via BT.");
    return bleControl->writeBLE(msg, with_delay);
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
        lastKeyboardButtonPressed_ = k.key_uid;
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

bool SparkDataControl::sparkLooperCommand(byte command) {

    current_msg = spark_msg.spark_looper_command(nextMessageNum, command);
    DEBUG_PRINTF("Spark Looper: %02x\n", command);

    return triggerCommand(current_msg);
}

bool SparkDataControl::increasePresetLooper() {
    // TODO: Wrapper, might not be needed in the end.
    return SparkPresetControl::getInstance().increasePresetLooper();
}

bool SparkDataControl::decreasePresetLooper() {
    // TODO: Wrapper, might not be needed in the end.
    return SparkPresetControl::getInstance().decreasePresetLooper();
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
                looperControl_->changeSettingBpm(bpm);
            }
        }
    }
}

bool SparkDataControl::updateLooperSettings() {
    DEBUG_PRINTF("Updating looper settings: %s\n", looperControl_->looperSetting()->getJson().c_str());
    current_msg = spark_msg.update_looper_settings(nextMessageNum, looperControl_->looperSetting());
    return triggerCommand(current_msg);
}

void SparkDataControl::startLooperTimer(void *args) {
    looperControl_->run(args);
}

void SparkDataControl::updateLooperCommand(byte lastCommand) {
    DEBUG_PRINTF("Last looper command: %02x\n", lastCommand);
    switch (lastCommand) {
    case 0x02:
        // Count in started
        break;
    case 0x04:
        // Record started
        looperControl_->isRecRunning() = true;
        break;
    case 0x05:
        // To be analyzed
        break;
    case 0x06:
        // Retry recording, maybe not required
        break;
    case 0x07:
        // Recording finished
        looperControl_->isRecRunning() = false;
        looperControl_->isRecAvailable() = true;
        break;
    case 0x08:
        // Play
        looperControl_->isPlaying() = true;
        break;
    case 0x09:
        // Stop
        looperControl_->isPlaying() = false;
        break;
    case 0x0A:
        // Delete
        looperControl_->resetStatus();
        break;
    case 0x0B:
        // Dub
        looperControl_->isRecRunning() = true;
        break;
    case 0x0C:
        // Stop Recording
        looperControl_->isRecRunning() = false;
        looperControl_->isRecAvailable() = true;
        break;
    case 0x0D:
        // UNDO
        looperControl_->canRedo() = true;
        break;
    case 0x0E:
        looperControl_->canRedo() = false;
        break;
    default:
        DEBUG_PRINTLN("Unknown looper command received.");
        break;
    }
    Serial.println(looperControl_->getLooperStatus().c_str());
}

bool SparkDataControl::sparkLooperStopAll() {
    bool stopReturn = sparkLooperStopPlaying();
    bool recStopReturn = sparkLooperStopRec();
    return stopReturn && recStopReturn;
}

bool SparkDataControl::sparkLooperStopPlaying() {
    // looperControl_->isPlaying() = false;
    bool retValue = sparkLooperCommand(SPK_LOOPER_CMD_STOP);
    if (retValue) {
        looperControl_->stop();
        looperControl_->reset();
        sparkLooperGetStatus();
    }
    return retValue;
}

bool SparkDataControl::sparkLooperPlay() {
    // looperControl_->isPlaying() = true;
    if (!(looperControl_->isPlaying())) {
        looperControl_->start();
        return sparkLooperCommand(SPK_LOOPER_CMD_PLAY);
    }
    return true;
}

bool SparkDataControl::sparkLooperRec() {
    bool countIn = looperControl_->looperSetting()->click;
    if (countIn) {
        sparkLooperCommand(SPK_LOOPER_CMD_COUNTIN);
        looperControl_->setCurrentBar(0);
    }
    looperControl_->start();
    recordStartFlag = true;
    return true;
}

bool SparkDataControl::sparkLooperDub() {
    bool retValue = sparkLooperCommand(SPK_LOOPER_CMD_DUB);
    // looperControl_->reset();
    looperControl_->start();
    retValue = retValue && sparkLooperCommand(SPK_LOOPER_CMD_PLAY);
    looperControl_->isPlaying() = true;
    return retValue;
}

bool SparkDataControl::sparkLooperRetry() {
    sparkLooperCommand(SPK_LOOPER_CMD_RETRY);
    looperControl_->reset();
    return sparkLooperRec();
}
// TODO: Get Looper status on startup and set flags accordingly

bool SparkDataControl::sparkLooperStopRec() {
    bool isRecAvailable = looperControl_->isRecAvailable();
    bool retVal = false;
    if (isRecAvailable) {
        retVal = sparkLooperCommand(SPK_LOOPER_CMD_STOP_DUB);
    } else {
        retVal = sparkLooperCommand(SPK_LOOPER_CMD_STOP_REC);
        retVal = sparkLooperCommand(SPK_LOOPER_CMD_REC_COMPLETE);
        looperControl_->reset();
    }
    sparkLooperGetStatus();
    return retVal;
}

bool SparkDataControl::sparkLooperUndo() {
    if (looperControl_->canUndo()) {
        DEBUG_PRINTLN("UNDO possible");
        return sparkLooperCommand(SPK_LOOPER_CMD_UNDO);
    }
    return true;
}

bool SparkDataControl::sparkLooperRedo() {
    if (looperControl_->canRedo()) {
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
    bool isRecRunning = looperControl_->isRecRunning();
    bool isPlaying = looperControl_->isPlaying();
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
    bool isRecRunning = looperControl_->isRecRunning();
    bool isRecAvailable = looperControl_->isRecAvailable();
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
    // bool isRecRunning = looperControl_->isRecRunning();
    bool isRecAvailable = looperControl_->isRecAvailable();
    bool canRedo = looperControl_->canRedo();
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
    current_msg = spark_msg.get_looper_status(nextMessageNum);
    return triggerCommand(current_msg);
}

bool SparkDataControl::sparkLooperGetConfig() {
    current_msg = spark_msg.get_looper_config(nextMessageNum);
    return triggerCommand(current_msg);
}

bool SparkDataControl::sparkLooperGetRecordStatus() {
    current_msg = spark_msg.get_looper_record_status(nextMessageNum);
    return triggerCommand(current_msg);
}
