/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkStreamReader.h"

SparkStreamReader::SparkStreamReader() : message{}, unstructuredData{}, msgData{}, msgPos(0) {
}

string SparkStreamReader::getJson() {
    return sb.getJson();
}

void SparkStreamReader::setMessage(const vector<ByteVector> &msg_) {
    unstructuredData = msg_;
    message.clear();
}

byte SparkStreamReader::readByte() {
    byte aByte;
    aByte = msgData[msgPos];
    msgPos += 1;
    return aByte;
}

string SparkStreamReader::readPrefixedString() {

    (void)readByte();
    // offset removed from string length byte to get real length
    int realStrLength = readByte() - 0xa0;
    string aStr = "";
    // reading string
    for (int i = 0; i < realStrLength; i++) {
        aStr += char(readByte());
    }
    return aStr;
}

string SparkStreamReader::readString() {
    byte aByte = readByte();
    int strLength;
    if (aByte == 0xd9) {
        aByte = readByte();
        strLength = aByte;
    } else if (aByte >= 0xa0) {
        strLength = aByte - 0xa0;
    } else {
        aByte = readByte();
        strLength = aByte - 0xa0;
    }

    string aStr = "";
    for (int i = 0; i < strLength; i++) {
        aStr += char(readByte());
    }
    return aStr;
}

// floats are special - bit 7 is actually stored in the format byte and not in the data
float SparkStreamReader::readFloat() {
    byte prefix = readByte(); // should be ca

    // using union struct to share memory for easy transformation of bytes to float
    union {
        float f;
        unsigned long ul;
    } u;

    byte a, b, c, d;
    a = readByte();
    b = readByte();
    c = readByte();
    d = readByte();
    u.ul = (a << 24) | (b << 16) | (c << 8) | d;
    float val = u.f;
    return val;
}

bool SparkStreamReader::readOnOff() {
    byte aByte = readByte();
    switch (aByte) {
    case 0xC3:
        return true;
        break;
    case 0xC2:
        return false;
        break;
    default:
        DEBUG_PRINTLN("Incorrect on/off state");
        return "?";
        break;
    }
}

void SparkStreamReader::readEffectParameter() {
    // Read object
    string effect = readPrefixedString();
    byte param = readByte();
    float val = readFloat();

    // Build string representations
    sb.startStr();
    sb.addStr("Effect", effect);
    sb.addSeparator();
    sb.addInt("Parameter", param);
    sb.addSeparator();
    sb.addFloat("Value", val);
    sb.endStr();

    // Set values
    statusObject.lastMessageType() = MSG_TYPE_FX_PARAM;
}

void SparkStreamReader::readEffect() {
    // Read object
    string effect1 = readPrefixedString();
    string effect2 = readPrefixedString();

    // Build string representations
    sb.startStr();
    sb.addStr("OldEffect", effect1);
    sb.addSeparator();
    sb.addNewline();
    sb.addStr("NewEffect", effect2);
    sb.endStr();

    // Set values
    statusObject.lastMessageType() = MSG_TYPE_FX_CHANGE;
}

void SparkStreamReader::readHardwarePreset() {
    // Read object
    readByte();
    byte presetNum = readByte() + 1;

    // Build string representations
    sb.startStr();
    sb.addInt("New HW Preset number", presetNum);
    sb.endStr();

    // Set values
    if (presetNum != statusObject.currentPresetNumber()) {
        statusObject.isPresetNumberUpdated() = true;
    }
    statusObject.currentPresetNumber() = presetNum;
    statusObject.lastMessageType() = MSG_TYPE_HWPRESET;
}

void SparkStreamReader::readHWChecksums(byte subCmd) {

    vector<byte> checksums;
    sb.startStr();

    // determine number of HW presets based on amp type (subCmd)
    int numberOfPresets;
    if (subCmd == 0x2a) {
        numberOfPresets = 4;
    } else if (subCmd == 0x2b) {
        numberOfPresets = 8;
    }

    // Array prefix byte
    readByte();
    for (int i = 0; i < numberOfPresets; i++) {
        int sum = readInt();
        checksums.push_back(sum);
        sb.addStr("Checksum Preset " + to_string(i + 1), SparkHelper::intToHex(sum));
        if (i < numberOfPresets - 1) {
            sb.addSeparator();
        }
    }
    sb.endStr();

    statusObject.hwChecksums() = checksums;
    statusObject.lastMessageType() = MSG_TYPE_HWCHECKSUM;
}

void SparkStreamReader::readStoreHardwarePreset() {
    // Read object
    readByte();
    byte presetNum = readByte() + 1;

    // Build string representations
    sb.startStr();
    sb.addInt("NewStoredPreset", presetNum);
    sb.endStr();

    // Set values
    statusObject.lastMessageType() = MSG_TYPE_HWPRESET;
}

void SparkStreamReader::readEffectOnOff() {
    // Read object
    string effect = readPrefixedString();
    boolean isOn = readOnOff();

    statusObject.currentEffect().name = effect;
    statusObject.currentEffect().isOn = isOn;
    // Build string representations
    sb.startStr();
    sb.addStr("Effect", effect);
    sb.addSeparator();
    sb.addBool("IsOn", isOn);
    sb.endStr();

    // Set values
    statusObject.lastMessageType() = MSG_TYPE_FX_ONOFF;
    statusObject.isEffectUpdated() = true;
}

void SparkStreamReader::readPreset() {
    // Read object (Preset main data)
    // DEBUG_PRINTF("Free memory before reading preset: %d\n", xPortGetFreeHeapSize());

    /*DEBUG_PRINTLN("Parsing message:");
    DEBUG_PRINTVECTOR(msgData);
    DEBUG_PRINTLN();
    */

    Preset &currentPreset = statusObject.currentPreset();

    readByte();
    byte preset = readByte();
    // DEBUG_PRINTF("Read PresetNumber: %d\n", preset);
    currentPreset.presetNumber = preset;
    string uuid = readString();
    // DEBUG_PRINTF("Read UUID: %s\n", uuid.c_str());
    currentPreset.uuid = uuid;
    string name = readString();
    // DEBUG_PRINTF("Read Name: %s\n", name.c_str());
    currentPreset.name = name;
    string version = readString();
    // DEBUG_PRINTF("Read Version: %s\n", version.c_str());
    currentPreset.version = version;
    string descr = readString();
    // DEBUG_PRINTF("Read Description: %s\n", descr.c_str());
    currentPreset.description = descr;
    string icon = readString();
    // DEBUG_PRINTF("Read Icon: %s\n", icon.c_str());
    currentPreset.icon = icon;
    float bpm = readFloat();
    // DEBUG_PRINTF("Read BPM: %f\n", bpm);
    currentPreset.bpm = bpm;
    // Build string representations
    // DEBUG_PRINTF("Free memory before adds: %d\n", xPortGetFreeHeapSize());
    sb.startStr();
    sb.addInt("PresetNumber", preset);
    sb.addSeparator();
    sb.addStr("UUID", uuid);
    sb.addSeparator();
    sb.addNewline();
    sb.addStr("Name", name);
    sb.addSeparator();
    sb.addStr("Version", version);
    sb.addSeparator();
    sb.addStr("Description", descr);
    sb.addSeparator();
    sb.addStr("Icon", icon);
    sb.addSeparator();
    sb.addFloat("BPM", bpm, "python");
    sb.addSeparator();
    sb.addNewline();
    // DEBUG_PRINTF("Free memory after adds: %d\n", xPortGetFreeHeapSize());
    //  Read Pedal data (including string representations)

    // !!! number of pedals not used currently, assumed constant as 7 !!!
    // int num_effects = readByte() - 0x90;
    // DEBUG_PRINTF("Read Number of effects: %d\n", num_effects);
    sb.addPython("\"Pedals\": [");
    sb.addNewline();
    currentPreset.pedals = {};
    int numberOfPedals = currentPreset.numberOfPedals;
    for (int i = 0; i < numberOfPedals; i++) { // Fixed to 7, but could maybe also be derived from num_effects?
        Pedal currentPedal = {};
        // DEBUG_PRINTF("Reading Pedal %d:\n", i);
        string eStr = readString();
        // DEBUG_PRINTF("  Pedal name: %s\n", eStr.c_str());
        currentPedal.name = eStr;
        boolean eOnOff = readOnOff();
        // DEBUG_PRINTF("  Pedal state: %s\n", eOnOff);
        currentPedal.isOn = eOnOff;
        sb.addPython("{");
        sb.addStr("Name", eStr);
        sb.addSeparator();
        sb.addBool("IsOn", eOnOff);
        sb.addSeparator();
        int numOfParameters = readByte() - char(0x90);
        // DEBUG_PRINTF("  Number of Parameters: %d\n", numOfParameters);
        // DEBUG_PRINTF("Free memory before parameters: %d\n", xPortGetFreeHeapSize());
        sb.addPython("\"Parameters\":[");
        // Read parameters of current pedal
        currentPedal.parameters = {};
        for (int p = 0; p < numOfParameters; p++) {
            Parameter currentParameter = {};
            // DEBUG_PRINTF("  Reading parameter %d:\n", p);
            byte num = readByte();
            // DEBUG_PRINTF("    Parameter ID: %d\n", SparkHelper::intToHex(num));
            byte spec = readByte();
            // DEBUG_PRINTF("    Parameter Special: %s\n",
            //	SparkHelper::intToHex(spec));
            // DEBUG_PRINTF("Free memory before reading float: %d\n", xPortGetFreeHeapSize());
            float val = readFloat();
            // DEBUG_PRINTF("    Parameter Value: %f\n", val);
            currentParameter.number = num;
            currentParameter.special = spec;
            currentParameter.value = val;
            // sb.addInt("Parameter", num, "python");
            // sb.addStr("Special", SparkHelper::intToHex(spec), "python");
            // sb.addFloat("Value", val, "python");
            sb.addFloatPure(val, "python");
            if (p < numOfParameters - 1) {
                sb.addSeparator();
            }
            currentPedal.parameters.push_back(currentParameter);
            // DEBUG_PRINTF("Free memory after reading preset: %d\n", xPortGetFreeHeapSize());
        }

        sb.addPython("]");
        // deleteIndent();
        sb.addPython("}");
        if (i < numberOfPedals - 1) {
            sb.addSeparator();
            sb.addNewline();
        }
        currentPreset.pedals.push_back(currentPedal);
    }
    sb.addPython("],");
    sb.addNewline();
    byte chksum = readByte();
    currentPreset.checksum = chksum;
    sb.addStr("Checksum", SparkHelper::intToHex(chksum));
    sb.addNewline();
    sb.endStr();
    currentPreset.text = sb.getText();
    currentPreset.raw = sb.getRaw();
    currentPreset.json = sb.getJson();
    currentPreset.isEmpty = false;

    statusObject.isPresetUpdated() = true;
    statusObject.lastMessageType() = MSG_TYPE_PRESET;
}

void SparkStreamReader::readLooperSettings() {

    DEBUG_PRINT("Reading looper settings:");
    DEBUG_PRINTVECTOR(msgData);
    DEBUG_PRINTLN();

    int bpm = readByte();
    // if the first byte is 0xCC, this is a prefix and the real BPM are in the next byte
    // CC is prefixed if the bpm is exceeding 128.
    if (bpm == 0xCC) {
        bpm = readByte();
    }
    int countByte = readByte();
    string countStr = countByte == 0x04 ? "straight" : "shuffle";
    int bars = readByte();
    bool freeIndicator = readOnOff();
    bool click = readOnOff();
    bool unknownOnOff = readOnOff();
    unsigned int maxDuration = readInt16();

    // Build string representations
    sb.startStr();
    sb.addInt("BPM", bpm);
    sb.addSeparator();
    sb.addStr("Count", countStr);
    sb.addSeparator();
    sb.addInt("Bars", bars);
    sb.addSeparator();
    sb.addBool("Free", freeIndicator);
    sb.addSeparator();
    sb.addBool("Click", click);
    sb.addSeparator();
    sb.addBool("Unknown switch", unknownOnOff);
    sb.addSeparator();
    sb.addInt("Max duration", maxDuration);
    sb.endStr();

    LooperSetting &looperSetting = statusObject.currentLooperSetting();
    looperSetting.bpm = bpm;
    looperSetting.countStr = countStr;
    looperSetting.count = countByte;
    looperSetting.bars = bars;
    looperSetting.freeIndicator = freeIndicator;
    looperSetting.click = click;
    looperSetting.unknownOnOff = unknownOnOff;
    looperSetting.maxDuration = maxDuration;

    looperSetting.json = sb.getJson();
    looperSetting.text = sb.getText();
    looperSetting.raw = sb.getRaw();

    statusObject.isLooperSettingUpdated() = true;
    statusObject.lastMessageType() = MSG_TYPE_LOOPER_SETTING;
}

void SparkStreamReader::readLooperCommand() {

    statusObject.lastLooperCommand() = readByte();
    DEBUG_PRINT("Received looper command: ");
    DEBUG_PRINTVECTOR(msgData);
    DEBUG_PRINTLN();
    statusObject.lastMessageType() = MSG_TYPE_LOOPER_COMMAND;
}

void SparkStreamReader::readLooperStatus() {
    // 4C0404004242
    int bpm = readByte();
    byte count = readByte();
    byte bars = readByte();
    int numberOfLoops = readByte();
    statusObject.numberOfLoops() = numberOfLoops;
    bool unknownOnOff1 = readByte();
    bool unknownOnOff2 = readByte();

    sb.startStr();
    sb.addInt("BPM", bpm);
    sb.addSeparator();
    sb.addInt("Count", count);
    sb.addSeparator();
    sb.addInt("Bars", bars);
    sb.addSeparator();
    sb.addInt("Loops", numberOfLoops);
    sb.addSeparator();
    sb.addStr("Unknown OnOff1", SparkHelper::intToHex(unknownOnOff1));
    sb.addSeparator();
    sb.addStr("Unknown OnOff2", SparkHelper::intToHex(unknownOnOff2));
    sb.endStr();

    statusObject.lastMessageType() = MSG_TYPE_LOOPER_STATUS;
}

void SparkStreamReader::readTapTempo() {
    float bpm = readFloat();

    sb.startStr();
    sb.addFloat("BPM", bpm, "python");
    sb.endStr();
    statusObject.lastMessageType() = MSG_TYPE_TAP_TEMPO;
}

void SparkStreamReader::readMeasure() {
    float measure = readFloat();
    statusObject.measure() = measure;

    sb.startStr();
    sb.addFloat("Measure", measure, "python");
    sb.endStr();
    statusObject.lastMessageType() = MSG_TYPE_MEASURE;
}

void SparkStreamReader::readTuner() {
    byte note = readByte();
    float offset = readFloat();
    statusObject.note() = note;
    statusObject.noteOffset() = offset;

    sb.startStr();
    sb.addInt("Note", note);
    sb.addFloat("Offset", offset, "python");
    sb.endStr();
    statusObject.lastMessageType() = MSG_TYPE_TUNER_OUTPUT;
}

void SparkStreamReader::readTunerOnOff() {
    // Read object
    boolean isOn = readOnOff();

    // Build string representations
    sb.startStr();
    sb.addBool("Tuner mode", isOn);
    sb.endStr();

    // Set values
    if (isOn) {
        statusObject.lastMessageType() = MSG_TYPE_TUNER_ON;
    } else {
        statusObject.lastMessageType() = MSG_TYPE_TUNER_OFF;
    }
}

void SparkStreamReader::readPresetRequest() {
    int type = readByte();
    if (type == 1) {
        statusObject.lastMessageType() = MSG_REQ_CURR_PRESET;
    } else {
        int presetNum = readByte();
        DEBUG_PRINTF("Request for preset %d\n", presetNum + 1);
        switch (presetNum) {
        case 0:
            statusObject.lastMessageType() = MSG_REQ_PRESET1;
            break;
        case 1:
            statusObject.lastMessageType() = MSG_REQ_PRESET2;
            break;
        case 2:
            statusObject.lastMessageType() = MSG_REQ_PRESET3;
            break;
        case 3:
            statusObject.lastMessageType() = MSG_REQ_PRESET4;
            break;
        default:
            DEBUG_PRINTF("Unknown preset number request: %d\n", presetNum);
            break;
        }
    }
}

void SparkStreamReader::readAmpStatus() {

    // 0a ?
    readByte();
    // 01 (always 01?)
    readByte();
    // 01 (powered(?))
    bool isBatteryPowered = readByte() == 0x01 ? true : false;
    // 00 (discharging) 01 (constant power) 02 (charging) 03 (full charged)
    int chargingStatus = readByte();
    // CD 0f CD (4045) (Battery level)
    // CD 06 73 (1651) (?)
    // 20  (?)
    // Default value when power cable is connected
    int batteryLevel = readInt();
    // Unknown UINT16/int and last byte
    readInt();
    readInt();

    SparkStatus &statusObject = SparkStatus::getInstance();
    statusObject.isAmpBatteryPowered() = isBatteryPowered;
    statusObject.ampBatteryLevel() = (BatteryLevel)batteryLevel;
    statusObject.ampBatteryChargingStatus() = (BatteryChargingStatus)chargingStatus;
    statusObject.lastMessageType() = MSG_TYPE_AMPSTATUS;
}

void SparkStreamReader::readSerialNumber() {
    string serialNumber = readString();
    // Serial number seems to come with additional F7 character at the end
    serialNumber = serialNumber.substr(0, serialNumber.length() - 1);

    // Build string representations
    sb.startStr();
    sb.addStr("Serial Number", serialNumber);
    sb.endStr();

    // Set values
    statusObject.lastMessageType() = MSG_TYPE_AMP_SERIAL;
    statusObject.ampSerialNumber() = serialNumber;
}

void SparkStreamReader::readInputVolume() {
    // Read object
    float volume = readFloat();
    // Build string representations
    sb.startStr();
    sb.addFloat("Input Volume", volume);
    sb.endStr();

    // Set values
    statusObject.lastMessageType() = MSG_TYPE_INPUT_VOLUME;
    statusObject.inputVolume() = volume;
    statusObject.isVolumeChanged() = true;
}

boolean SparkStreamReader::structureData(bool processHeader) {

    ByteVector blockContent;
    blockContent.clear();
    message.clear();

    for (auto block : unstructuredData) {

        // DEBUG_PRINTVECTOR(block);

        int blockLength;
        if (processHeader) {
            blockLength = block[6];
        } else {
            blockLength = block.size();
        }
        int dataSize = block.size();
        // DEBUG_PRINTF("Read block size %d, %d\n", blockLength, dataSize);
        if (dataSize != blockLength) {
            DEBUG_PRINTF("Data is of size %d and reports %d\n", dataSize, blockLength);
            Serial.println("Corrupt block:");
            for (auto by : block) {
                Serial.print(SparkHelper::intToHex(by).c_str());
            }
            Serial.println();
        }
        // DEBUG_PRINTLN("Sizes match");
        ByteVector chunk;
        if (processHeader) {
            // Cut away header
            chunk.assign(block.begin() + 16, block.end());
        } else {
            chunk.assign(block.begin(), block.end());
        }

        for (auto chunkByte : chunk) {
            blockContent.push_back(chunkByte);
        } // FOR chunkByte

        // DEBUG_PRINTLN("Pushed chunk bytes to block content");
    } // FOR block
    DEBUG_PRINTLN();
    // DEBUG_PRINTLN("...Processed");

    if (blockContent[0] != 0xF0 || blockContent[1] != 0x01) {
        Serial.println("Invalid block start, ignoring all data");
        return false;
    }

    // DEBUG_PRINTLN("Data seems correct");
    //  and split them into chunks now, splitting on each f7
    vector<ByteVector> chunks;
    chunks.clear();
    ByteVector chunkTemp = {};
    for (byte by : blockContent) {
        chunkTemp.push_back(by);
        if (by == 0xf7) {
            chunks.push_back(chunkTemp);
            chunkTemp = {};
        }
    }

    vector<CmdData> chunk_8bit = {};
    for (auto chunk : chunks) {
        statusObject.lastMessageNum() = chunk[2];
        byte thisCmd = chunk[4];
        byte thisSubCmd = chunk[5];
        ByteVector data7bit = {};
        data7bit.assign(chunk.begin() + 6, chunk.end() - 1);

        // DEBUG_PRINTLN("Converted to 8bit");
        CmdData currData;
        currData.cmd = thisCmd;
        currData.subcmd = thisSubCmd;
        currData.data = convertDataTo8bit(data7bit);

        chunk_8bit.push_back(currData);

        // now check for mult-chunk messages and collapse their data into a single message
        // multi-chunk messages are cmd/subCmd of 1,1 or 3,1

        message.clear();
        ByteVector concatData;
        concatData.clear();
        for (CmdData chunkData : chunk_8bit) {
            thisCmd = chunkData.cmd;
            thisSubCmd = chunkData.subcmd;
            ByteVector thisData = chunkData.data;
            if ((thisCmd == 0x01 || thisCmd == 0x03) && thisSubCmd == 0x01) {
                // DEBUG_PRINTLN("Multi message");
                // found a multi-message
                int numChunks = thisData[0];
                int thisChunk = thisData[1];
                ByteVector thisDataSuffix;
                thisDataSuffix.assign(thisData.begin() + 3, thisData.end());
                for (auto by : thisDataSuffix) {
                    concatData.push_back(by);
                }
                // if at last chunk of multi-chunk
                if (thisChunk == numChunks - 1) {
                    // DEBUG_PRINTLN("Last chunk to process");
                    currData.cmd = thisCmd;
                    currData.subcmd = thisSubCmd;
                    currData.data = concatData;

                    message.push_back(currData);
                    concatData = {};
                }
            } else {
                // copy old one
                message.push_back(chunkData);
            } // else
        } // For all in 8-bit vector
    } // for all chunks

    return true;
}

void SparkStreamReader::setInterpreter(const ByteVector &_msg) {
    msgData = _msg;
    msgPos = 0;
}

int SparkStreamReader::runInterpreter(byte _cmd, byte _subCmd) {
    // Message from APP to AMP
    if (_cmd == 0x01) {
        switch (_subCmd) {
        case 0x01:
            DEBUG_PRINTLN("01 01 - Reading preset");
            readPreset();
            break;
        case 0x04:
            DEBUG_PRINTLN("01 04 - Reading effect param");
            readEffectParameter();
            break;
        case 0x06:
            DEBUG_PRINTLN("01 06 - Reading effect");
            readEffect();
            break;
        case 0x15:
            DEBUG_PRINTLN("01 15 - Reading effect on/off");
            readEffectOnOff();
            break;
        case 0x38:
            DEBUG_PRINTLN("01 38 - Change to different preset");
            readHardwarePreset();
            break;
        default:
            DEBUG_PRINTF("%02x %02x - not handled: ", _cmd, _subCmd);
            DEBUG_PRINTVECTOR(msgData);
            DEBUG_PRINTLN();
            break;
        }

    }
    // Request to AMP
    else if (_cmd == 0x02) {
        DEBUG_PRINTLN("Reading request from Amp");
        switch (_subCmd) {
        case 0x23:
            DEBUG_PRINTLN("Found request for serial number");
            statusObject.lastMessageType() = MSG_REQ_SERIAL;
            break;
        case 0x2F:
            DEBUG_PRINTLN("Found request for firmware version");
            statusObject.lastMessageType() = MSG_REQ_FW_VER;
            break;
        case 0x2A:
            DEBUG_PRINTLN("Found request for hw checksum");
            statusObject.lastMessageType() = MSG_REQ_PRESET_CHK;
            break;
        case 0x10:
            DEBUG_PRINTLN("Found request for hw preset number");
            statusObject.lastMessageType() = MSG_REQ_CURR_PRESET_NUM;
            break;
        case 0x01:
            DEBUG_PRINTLN("Found request for current preset");
            readPresetRequest();
            break;
        case 0x71:
            DEBUG_PRINTLN("Found request for 02 71");
            statusObject.lastMessageType() = MSG_REQ_AMP_STATUS;
            break;
        case 0x72:
            DEBUG_PRINTLN("Found request for 02 72");
            statusObject.lastMessageType() = MSG_REQ_72;
            break;
        default:
            DEBUG_PRINTF("Found invalid request: %02x \n", _subCmd);
            statusObject.lastMessageType() = MSG_REQ_INVALID;
            break;
        }
    }
    // Message from AMP to APP
    else if (_cmd == 0x03) {
        switch (_subCmd) {
        case 0x01:
            DEBUG_PRINTLN("03 01 - Reading preset");
            readPreset();
            break;
        case 0x2a:
            DEBUG_PRINTLN("03 2A - Reading HW checksums");
            readHWChecksums(0x2a);
            break;
        case 0x2b:
            DEBUG_PRINTLN("03 2B - Reading HW checksums");
            readHWChecksums(0x2b);
            break;
        case 0x06:
            DEBUG_PRINTLN("03 06 - Reading effect");
            readEffect();
            break;
        case 0x11:
            DEBUG_PRINTLN("03 11 - Reading amp name");
            readAmpName();
            break;
        case 0x15:
            DEBUG_PRINTLN("03 15 - Reading effect on/off");
            readEffectOnOff();
            break;
        case 0x23:
            DEBUG_PRINTLN("03 23 - Reading serial number");
            readSerialNumber();
            break;
        case 0x27:
            DEBUG_PRINTLN("03 27 - Storing HW preset");
            readStoreHardwarePreset();
            break;
        case 0x37:
            DEBUG_PRINTLN("03 37 - Reading effect param");
            readEffectParameter();
            break;
        case 0x38:
        case 0x10:
            DEBUG_PRINTLN("03 38/10 - Reading HW preset");
            readHardwarePreset();
            break;
        case 0x63:
            DEBUG_PRINTLN("03 63 - Reading Tap Tempo");
            readTapTempo();
            break;
        case 0x64:
            DEBUG_PRINTLN("03 64 - Reading Tuner Output");
            readTuner();
            break;
        case 0x65:
            DEBUG_PRINTLN("03 65 - Tuner On/Off");
            readTunerOnOff();
            break;
        case 0x6B:
            DEBUG_PRINTLN("03 6B - Reading Input Volume");
            readInputVolume();
            break;
        case 0x71:
            DEBUG_PRINTLN("03 71 - Reading Amp Status");
            readAmpStatus();
            break;
        case 0x75:
            DEBUG_PRINTLN("03 75 - Reading Looper Record Status");
            readLooperCommand();
            break;
        case 0x76:
            DEBUG_PRINTLN("03 76 - Reading Looper settings");
            readLooperSettings();
            break;
        case 0x77:
            DEBUG_PRINTLN("03 77 - Reading current measure");
            readMeasure();
            break;
        case 0x78:
            DEBUG_PRINTLN("03 78 - Reading current Looper status");
            readLooperStatus();
            break;
        default:
            DEBUG_PRINTF("%02x %02x - not handled: ", _cmd, _subCmd);
            DEBUG_PRINTVECTOR(msgData);
            DEBUG_PRINTLN();
            break;
        }
    }
    // Acknowledgement
    else if (_cmd == 0x04 || _cmd == 0x05) {
        DEBUG_PRINT("ACK number ");
        byte lastMessageNum = statusObject.lastMessageNum();
        DEBUG_PRINTLN(lastMessageNum);
        byte detail = 0x00; // detail is only used for Acks received from Amp
        AckData ack;
        ack.msgNum = lastMessageNum;
        ack.cmd = _cmd;
        ack.subcmd = _subCmd;
        statusObject.acknowledgments().push_back(ack);
        DEBUG_PRINTF("Acknowledgment for command %02x %02x\n", _cmd, _subCmd);
    } else {
        // unprocessed command (likely the initial ones sent from the app
        DEBUG_PRINTF("Unprocessed: %02x, %02x - ", _cmd,
                     _subCmd);
        DEBUG_PRINTVECTOR(msgData);
        DEBUG_PRINTLN();
    }

    return 1;
}

tuple<bool, byte, byte> SparkStreamReader::needsAck(const ByteVector &blk) {

    if (blk.size() < 22) { // Block is too short, does not need acknowledgement
        return tuple<bool, byte, byte>(false, 0, 0);
    }
    byte direction[2] = {blk[4], blk[5]};
    byte lastMessageNum = blk[18];
    statusObject.lastMessageNum() = lastMessageNum;
    byte cmd = blk[20];
    byte subCmd = blk[21];

    byte msgToSpark[2] = {0x53, 0xFE};
    int msgToSparkCompare = memcmp(direction, msgToSpark, sizeof(direction));
    if (msgToSparkCompare == 0) {
        if (cmd == 0x01 && subCmd != 0x04) {
            // the app sent a message that needs a response
            return tuple<bool, byte, byte>(true, lastMessageNum, subCmd);
        }
    }

    return tuple<bool, byte, byte>(false, 0, 0);
}

AckData SparkStreamReader::getLastAckAndEmpty() {
    AckData lastAck;
    vector<AckData> acknowledgments = statusObject.acknowledgments();
    if (acknowledgments.size() > 0) {
        lastAck = acknowledgments.back();
        statusObject.resetAcknowledgments();
    }
    return lastAck;
}

void SparkStreamReader::preProcessBlock(ByteVector &blk) {

    // Special behavior: When receiving messages from Spark APP, blocks might be split into two.
    // This will reassemble the block by appending to the previous one.

    // Iterate through block and split into F001/F7 chunks into response.

    // If nothing in response yet or if last read byte was a F7, add block
    if (response.size() == 0 || lastReadByte == endMarker) {
        if (blockIsStarted(blk)) {
            response.push_back(blk);
            lastReadByte = blk.back();
        } else {
            DEBUG_PRINTLN("Incomplete fragment found, ignoring.");
        }
        return;
    }

    auto it = blk.begin();
    ByteVector segment = {};
    ByteVector currentChunk = {};

    // Search blk for each occurrence of F7 and append to response
    while (it != blk.end()) {
        it = find(blk.begin(), blk.end(), endMarker);
        if (it != blk.end()) {
            segment.assign(blk.begin(), it + 1);
            blk.assign(it + 1, blk.end());
            if (lastReadByte != endMarker) {
                currentChunk = response.back();
                currentChunk.insert(currentChunk.end(), segment.begin(), segment.end());
                response.pop_back();
                response.push_back(currentChunk);
                currentChunk = {};
            } else {
                response.push_back(segment);
            }
            lastReadByte = response.back().back();
        }
    }
    // If a remainder is left in blk, append to previous block
    if (blk.size() > 0) {

        if (lastReadByte != endMarker) {
            currentChunk = response.back();
            currentChunk.insert(currentChunk.end(), blk.begin(), blk.end());
            response.pop_back();
            response.push_back(currentChunk);
            currentChunk = {};
        } else {
            response.push_back(blk);
        }
        lastReadByte = response.back().back();
    }
}

bool SparkStreamReader::blockIsStarted(ByteVector &blk) {
    if (blk.size() < 2)
        return false;
    bool newStart = (blk[0] == 0xF0 && blk[1] == 0x01);

    return newStart;
}

MessageProcessStatus SparkStreamReader::processBlock(ByteVector &blk) {

    MessageProcessStatus retValue = MSG_PROCESS_RES_INCOMPLETE;
    bool msgToSpark = false;
    bool msgFromSpark = true;

    /*
        DEBUG_PRINTLN("Processing block");
        DEBUG_PRINTVECTOR(blk);
        DEBUG_PRINTLN();
    */
    // Process:
    // 1. Remove 01FE header if present
    // 2. Build command (response) vector by splitting blocks into F001...F7 blocks

    // Remove 01FE header
    if (blk[0] == 0x01 && blk[1] == 0xFE && blk.size() > 16) {
        // Block starts with 01FE and is long enough
        // Read meta data of block
        int blkLength = blk[6];
        byte dir[2] = {blk[4], blk[5]};

        msgToSpark = dir[0] == 0x53 && dir[1] == 0xFE;
        msgFromSpark = dir[0] == 0x41 && dir[1] == 0xFF;

        // Cut off header after extracting information
        blk.assign(blk.begin() + 16, blk.end());
    }
    // FROM HERE NO HEADER IS PRESENT ANYMORE and blk should start with F001 (after preprocessing)

    // Cut blk into chunks and append to response
    preProcessBlock(blk);

    if (response.size() == 0) {
        return retValue;
    }

    // Check if last block is final and which command
    ByteVector currentBlock = response.back();
    byte seq = currentBlock[2];
    byte cmd = currentBlock[4];
    byte subCmd = currentBlock[5];

    if (!(isValidBlockWithoutHeader(currentBlock))) {
        /*
        DEBUG_PRINTLN("Block not ready for processing, skipping further processing.");
        DEBUG_PRINTVECTOR(currentBlock);
        DEBUG_PRINTLN();
        */
        return retValue;
    }

    // Check if currentBlock is last block of command
    // If we don't have a 01 or 03 command with 01/10/38 sub command, we are ready to process
    if ((cmd != 0x01 && cmd != 0x03) || (subCmd != 01 && subCmd != 10 && subCmd != 38)) {
        msgLastBlock = true;
    }
    // Multi-chunk message
    else {
        int numChunks = currentBlock[7];
        int thisChunk = currentBlock[8];
        if ((thisChunk + 1) == numChunks) {
            msgLastBlock = true;
        }
    }

    // Process data if the block just analyzed was the last
    if (msgLastBlock) {
        msgLastBlock = false;
        setMessage(response);
        DEBUG_PRINT("Message received: ");
        for (auto chunk : response) {
            DEBUG_PRINTVECTOR(chunk);
            DEBUG_PRINTLN();
        }

        readMessage(false);
        response.clear();
        lastReadByte = 0x00;
        retValue = MSG_PROCESS_RES_COMPLETE;
    } // msgLastBlock

    // Message is not complete, has not been processed yet
    // if request was an initiating one from the app, return value for INITIAL message,
    // so SparkDataControl knows how to notify.
    // so notifications will be triggered
    if (cmd == 0x02) {
        retValue = MSG_PROCESS_RES_REQUEST;
    }

    return retValue;
}

void SparkStreamReader::interpretData() {
    for (auto msgData : message) {
        int thisCmd = msgData.cmd;
        int thisSubCmd = msgData.subcmd;
        ByteVector thisData = msgData.data;

        setInterpreter(thisData);
        runInterpreter(thisCmd, thisSubCmd);
    }
    // message.clear();
}

vector<CmdData> SparkStreamReader::readMessage(bool processHeader) {
    if (structureData(processHeader)) {
        interpretData();
    }
    return message;
}

bool SparkStreamReader::isValidBlockWithoutHeader(const ByteVector &blk) {

    // Checks done:
    // 1. Block has a length of at least 3
    // 2. Block starts with F0 01
    // 3. Block ends with F7

    if (blk.size() < 3)
        return false;
    if (blk[0] != 0xF0 || blk[1] != 0x01)
        return false;
    if (blk.back() != 0xF7)
        return false;

    return true;
}

int SparkStreamReader::readInt() {

    int result = 0;
    byte first = readByte();
    byte major = 0;
    byte minor = 0;
    // If int value is greater than 128 it is prefixed with 0xCC
    switch (first) {
    case 0xCC:
    case 0xD0:
        result = readByte();
        break;
    case 0xCD:
        major = readByte();
        minor = readByte();
        result = (major << 8 | minor);
        break;
    default:
        result = first;
        break;
    }

    return result;
}

unsigned int SparkStreamReader::readInt16() {
    // Read the following two bytes as INT
    // INT is prefixed with 0xCD
    byte prefix = readByte();
    byte major = readByte();
    byte minor = readByte();
    unsigned int result = (major << 8 | minor);
    return result;
}

void SparkStreamReader::clearMessageBuffer() {
    DEBUG_PRINTLN("Clearing response buffer.");
    response.clear();
}

ByteVector SparkStreamReader::convertDataTo8bit(ByteVector input) {
    int chunkLength = input.size();
    // DEBUG_PRINT("Chunk_len:");
    // DEBUG_PRINTLN(chunkLength);
    int numOfSequences = int((chunkLength + 7) / 8);
    ByteVector data8bit = {};

    for (int thisSequence = 0; thisSequence < numOfSequences; thisSequence++) {
        int seqLength = min(8, chunkLength - (thisSequence * 8));
        ByteVector seq = {};
        byte bit8 = input[thisSequence * 8];
        for (int ind = 0; ind < seqLength - 1; ind++) {
            byte dat = input[thisSequence * 8 + ind + 1];
            if ((bit8 & (1 << ind)) == (1 << ind)) {
                dat |= 0x80;
            }
            seq.push_back(dat);
        }
        for (auto by : seq) {
            data8bit.push_back(by);
        }
    }
    return data8bit;
}

void SparkStreamReader::readAmpName() {
    string ampName = readPrefixedString();

    // Build string representations
    sb.startStr();
    sb.addStr("Amp Name", ampName);
    sb.endStr();

    // Set values
    statusObject.lastMessageType() = MSG_TYPE_AMP_NAME;
    statusObject.ampName() = ampName;
}
