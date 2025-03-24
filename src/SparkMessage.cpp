/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkMessage.h"

SparkMessage::SparkMessage() : data{}, splitData8{}, splitData7{}, cmd(0), subCmd(0) {
}

void SparkMessage::startMessage(byte _cmd, byte _subCmd) {
    cmd = _cmd;
    subCmd = _subCmd;
    data = {};
    splitData8 = {};
    splitData7 = {};
};

void SparkMessage::splitDataToChunks(int dir) {

    int MAX_CHUNK_SIZE;
    if (dir == DIR_TO_SPARK) {
        // maximum chunk size for messages to Spark Amp
        MAX_CHUNK_SIZE = maxChunkSizeToSpark();
    } else {
        // maximum chunk size for messages sent from Spark Amp to App
        MAX_CHUNK_SIZE = maxChunkSizeFromSpark();
    }

    // determine how many chunks there are
    int dataLen = data.size();
    // minimum is 1 chunk
    int numChunks = 1;
    if (dataLen > 0) {
        numChunks = int((dataLen + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE);
    }

    // split the data into chunks of maximum 0x80 bytes (still 8 bit bytes)
    // and add a chunk sub-header if a multi-chunk message

    for (int thisChunk = 0; thisChunk < numChunks; thisChunk++) {
        int chunkLen = min(MAX_CHUNK_SIZE,
                           dataLen - (thisChunk * MAX_CHUNK_SIZE));
        ByteVector data8;
        if (numChunks > 1) {
            // we need the chunk sub-header
            data8.push_back(numChunks);
            data8.push_back(thisChunk);
            data8.push_back(chunkLen);
        } else {
            data8 = {};
        }
        data8.insert(data8.end(), data.begin() + (thisChunk * MAX_CHUNK_SIZE),
                     data.begin() + (thisChunk * MAX_CHUNK_SIZE + chunkLen));
        splitData8.push_back(data8);
    }
}

void SparkMessage::convertDataTo7Bit() {

    // so loop over each chunk
    // and in each chunk loop over every sequence of (max) 7 bytes
    // and extract the 8th bit and put in 'bit8'
    // and then add bit8 and the 7-bit sequence to data7
    for (auto chunk : splitData8) {

        int chunkLen = chunk.size();
        int numberOfSequences = int((chunkLen + 6) / 7);
        ByteVector bytes7 = {};

        for (int thisSeq = 0; thisSeq < numberOfSequences; thisSeq++) {
            int seqLen = min(7, chunkLen - (thisSeq * 7));
            byte bit8 = 0;
            ByteVector seq = {};
            for (int ind = 0; ind < seqLen; ind++) {
                byte dat = chunk[thisSeq * 7 + ind];
                if ((dat & 0x80) == 0x80) {
                    bit8 |= (1 << ind);
                }
                dat &= 0x7f;

                seq.push_back((byte)dat);
            }

            bytes7.push_back(bit8);
            bytes7.insert(bytes7.end(), seq.begin(), seq.end());
        }
        splitData7.push_back(bytes7);
    }
}

void SparkMessage::buildChunkData(byte msgNumber) {

    ByteVector chunkHeader = {0xF0, 0x01};
    byte msgNum;
    if (msgNumber == 0) {
        msgNum = 0x01;
    } else {
        msgNum = (byte)msgNumber;
    }
    byte trailer = 0xF7;

    // build F0 01 chunks:
    for (auto data7bit : splitData7) {
        byte checksum = calculateChecksum(data7bit);

        ByteVector completeChunk = chunkHeader;
        completeChunk.push_back(msgNum);
        completeChunk.push_back(checksum);
        completeChunk.push_back(cmd);
        completeChunk.push_back(subCmd);
        completeChunk.insert(completeChunk.end(), data7bit.begin(), data7bit.end());
        completeChunk.push_back(trailer);

        allChunks.push_back(completeChunk);
    }
}

vector<CmdData> SparkMessage::buildMessage(MessageDirection dir, byte msgNum) {

    vector<CmdData> finalMessage = {};

    int totalBytesToProcess = SparkHelper::dataVectorNumOfBytes(allChunks);
    int blockPrefixSize = withHeader_ ? 16 : 0;

    // Maximum block size depending on direction.
    int MAX_BLOCK_SIZE = (dir == DIR_TO_SPARK) ? maxBlockSizeToSpark() : maxBlockSizeFromSpark();

    // now we can create the final message with the message header and the chunk header
    ByteVector blockHeader = {0x01, 0xFE, 0x00, 0x00};
    ByteVector blockHeaderDirection = (dir == DIR_TO_SPARK) ? msgToSpark : msgFromSpark;
    ByteVector blockFiller = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00};

    // read all chunks into one big chunk for splitting
    std::deque<byte> dataToSplit = {};
    for (auto chunk : allChunks) {
        dataToSplit.insert(dataToSplit.end(), chunk.begin(), chunk.end());
    }
    allChunks.clear();

    // create block
    ByteVector currentBlock = {};
    int dataSize;

    // create blocks from all data with headers.
    while (dataToSplit.size() > 0) {
        dataSize = (int)dataToSplit.size();
        if (withHeader_) {

            int blockSize = min(MAX_BLOCK_SIZE,
                                dataSize + blockPrefixSize);
            currentBlock = blockHeader;
            currentBlock.insert(currentBlock.end(),
                                blockHeaderDirection.begin(),
                                blockHeaderDirection.end());
            currentBlock.push_back(blockSize);
            currentBlock.insert(currentBlock.end(), blockFiller.begin(),
                                blockFiller.end());
        }
        int remainingSize = MAX_BLOCK_SIZE - currentBlock.size();
        int bytesToInsert = min(remainingSize, dataSize);
        std::deque<byte>::iterator numOfBytes = dataToSplit.begin() + bytesToInsert;
        currentBlock.insert(currentBlock.end(), dataToSplit.begin(), numOfBytes);
        dataToSplit.erase(dataToSplit.begin(), numOfBytes);
        CmdData dataItem;
        dataItem.data = currentBlock;
        dataItem.cmd = cmd;
        dataItem.subcmd = subCmd;
        dataItem.detail = cmdDetail;
        dataItem.msgNum = msgNum;
        finalMessage.push_back(dataItem);
        currentBlock.clear();
    }

    DEBUG_PRINTLN("COMPLETE MESSAGE: ");
    for (auto block : finalMessage) {
        DEBUG_PRINTVECTOR(block.data);
        DEBUG_PRINTLN();
    }

    return finalMessage;
}

vector<CmdData> SparkMessage::endMessage(MessageDirection dir, byte msgNumber) {

    DEBUG_PRINT("MESSAGE NUMBER: ");
    DEBUG_PRINT(msgNumber);
    DEBUG_PRINTLN();

    // First split all data into chunks of defined maximum size
    splitDataToChunks(dir);
    // now we can convert this to 7-bit data format with the 8-bits byte at the front
    convertDataTo7Bit();
    // build F001 chunks
    buildChunkData(msgNumber);
    // build final message (with 01FE header if required)
    return buildMessage(dir, msgNumber);
}

void SparkMessage::addBytes(const ByteVector &bytes_8) {
    for (byte by : bytes_8) {
        data.push_back(by);
    }
}

void SparkMessage::addByte(byte by) {
    data.push_back(by);
}

void SparkMessage::addPrefixedString(const string &packStr, int lengthOverride) {
    int strLength = lengthOverride;
    if (strLength == 0)
        strLength = packStr.size();
    ByteVector bytePack;
    bytePack.push_back((byte)strLength);
    bytePack.push_back((byte)(strLength + 0xa0));
    copy(packStr.begin(), packStr.end(), back_inserter<ByteVector>(bytePack));
    addBytes(bytePack);
}

void SparkMessage::addString(const string &packStr) {
    int strLength = packStr.size();
    ByteVector bytePack;
    bytePack.push_back((byte)(strLength + 0xa0));
    copy(packStr.begin(), packStr.end(), back_inserter<ByteVector>(bytePack));
    addBytes(bytePack);
}

void SparkMessage::addLongString(const string &packStr) {
    int strLength = packStr.size();
    ByteVector bytePack;
    bytePack.push_back((0xD9));
    bytePack.push_back((byte)strLength);
    copy(packStr.begin(), packStr.end(), back_inserter<ByteVector>(bytePack));
    addBytes(bytePack);
}

void SparkMessage::addFloat(float flt) {

    union {
        float floatVariable;
        byte tempArray[4];
    } u;
    // Overwrite bytes of union with float variable
    // ROundign float to 4 digits before converting to avoid rounding errors during conversion
    u.floatVariable = roundf(flt * 10000) / 10000;
    ByteVector bytePack;
    bytePack.push_back((byte)0xca);
    // Assign bytes to input array
    for (int i = 3; i >= 0; i--) {
        bytePack.push_back(u.tempArray[i]);
    }
    // DEBUG_PRINTF("Converting float %f to HEX: ", u.floatVariable);
    // for (auto byte : bytePack) {
    //	DEBUG_PRINTF("%s", SparkHelper::intToHex(byte).c_str());
    // }
    // DEBUG_PRINTLN();
    addBytes(bytePack);
}

void SparkMessage::addOnOff(boolean enable) {
    byte b;
    if (enable == true) {
        b = 0xC3;
    } else {
        b = 0xC2;
    }
    addByte(b);
}

void SparkMessage::addInt16(unsigned int number) {
    ByteVector bytepack;
    // Converting number to 16 bit INT
    bytepack.push_back(0xCD);
    bytepack.push_back(number >> 8);
    bytepack.push_back(number & 0xFF);

    addBytes(bytepack);
}

vector<CmdData> SparkMessage::getCurrentPresetNum(byte msgNum) {
    // hardcoded message
    /*
    vector<ByteVector> msg;
    ByteVector msg_vec = {0x01, 0xfe, 0x00, 0x00, 0x53, 0xfe,
            0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xf0, 0x01, 0x08, 0x00, 0x02, 0x10, 0xf7}; // had an extra 0x79 at the end, likely not needed
    msg.push_back(msg_vec);
    return msg;
     */
    cmd = 0x02;
    subCmd = 0x10;

    startMessage(cmd, subCmd);
    return endMessage(DIR_TO_SPARK, msgNum);
}

vector<CmdData> SparkMessage::getCurrentPreset(byte msgNum, int hw_preset) {

    cmd = 0x02;
    subCmd = 0x01;
    DEBUG_PRINTF("Getting preset with message number %s\n", SparkHelper::intToHex(msgNum).c_str());
    startMessage(cmd, subCmd);
    if (hw_preset == -1) {
        addByte(0x01);
        addByte(0x00);
    } else {
        addByte(0x00);
        addByte((byte)hw_preset - 1);
    }

    // add trailing 30 bytes of 00, seems to be optional
    /* for (int i = 0; i < 30; i++) {
         addByte(0x00);
     }*/

    return endMessage(DIR_TO_SPARK, msgNum);
}

vector<CmdData> SparkMessage::changeEffectParameter(byte msgNum, const string &pedal, int param, float val) {
    cmd = 0x01;
    subCmd = 0x04;

    startMessage(cmd, subCmd);
    addPrefixedString(pedal);
    addByte((byte)param);
    addFloat(val);
    return endMessage(DIR_TO_SPARK, msgNum);
}

vector<CmdData> SparkMessage::changeEffect(byte msgNum, const string &pedal1, const string &pedal2) {
    cmd = 0x01;
    subCmd = 0x06;

    startMessage(cmd, subCmd);
    addPrefixedString(pedal1);
    addPrefixedString(pedal2);
    return endMessage(DIR_TO_SPARK, msgNum);
}

vector<CmdData> SparkMessage::changeHardwarePreset(byte msgNum, int preset_num) {
    cmd = 0x01;
    subCmd = 0x38;

    startMessage(cmd, subCmd);
    addByte((byte)0);
    addByte((byte)preset_num - 1);
    return endMessage(DIR_TO_SPARK, msgNum);
}

vector<CmdData> SparkMessage::getAmpName(byte msgNum) {

    cmd = 0x02;
    subCmd = 0x11;

    startMessage(cmd, subCmd);
    return endMessage(DIR_TO_SPARK, msgNum);
}

vector<CmdData> SparkMessage::getSerialNumber(byte msgNum) {
    cmd = 0x02;
    subCmd = 0x23;

    startMessage(cmd, subCmd);
    return endMessage(DIR_TO_SPARK, msgNum);
}

vector<CmdData> SparkMessage::getHwChecksums(byte msgNum) {
    cmd = 0x02;
    subCmd = 0x2a;

    startMessage(cmd, subCmd);
    // Array of size 4
    addByte(0x94);
    addByte(0x00);
    addByte(0x01);
    addByte(0x02);
    addByte(0x03);
    return endMessage(DIR_TO_SPARK, msgNum);
}

vector<CmdData> SparkMessage::getHWChecksumsExtended(byte msgNum) {
    cmd = 0x02;
    subCmd = 0x2b;

    startMessage(cmd, subCmd);
    addByte(0x00);
    return endMessage(DIR_TO_SPARK, msgNum);
}

vector<CmdData> SparkMessage::getFirmwareVersion(byte msgNum) {
    cmd = 0x02;
    subCmd = 0x2f;

    startMessage(cmd, subCmd);
    return endMessage(DIR_TO_SPARK, msgNum);
}

vector<CmdData> SparkMessage::getAmpStatus(byte msgNum) {
    cmd = 0x02;
    subCmd = 0x71;

    startMessage(cmd, subCmd);
    return endMessage(DIR_TO_SPARK, msgNum);
}

vector<CmdData> SparkMessage::turnEffectOnOff(byte msgNum, const string &pedal, boolean enable) {
    cmd = 0x01;
    subCmd = 0x15;

    startMessage(cmd, subCmd);
    addPrefixedString(pedal);
    addOnOff(enable);
    addByte(0x00);
    return endMessage(DIR_TO_SPARK, msgNum);
}

vector<CmdData> SparkMessage::switchTuner(byte msgNum, boolean enable) {
    cmd = 0x01;
    subCmd = 0x65;

    startMessage(cmd, subCmd);
    addOnOff(enable);
    return endMessage(DIR_TO_SPARK, msgNum);
}

vector<CmdData> SparkMessage::sendSerialNumber(byte msgNumber) {
    cmd = 0x03;
    subCmd = 0x23;

    startMessage(cmd, subCmd);
    string serialNum = "S999C999B999";
    // Spark App seems to send the F7 byte as part of the string in addition to the final F7 byte,
    // so we need to have a flexible addPrefixedString method to increase lenght information by one
    addPrefixedString(serialNum, serialNum.length() + 1);
    addByte(0xF7);
    return endMessage(DIR_FROM_SPARK, msgNumber);
}

vector<CmdData> SparkMessage::sendFirmwareVersion(byte msgNumber) {
    cmd = 0x03;
    subCmd = 0x2F;

    // TODO take version string as input
    startMessage(cmd, subCmd);

    addByte(0xCE);
    // Version string 1.7.5.182 (old: 1.6.5.160 (160-128))
    addByte((byte)1);
    addByte((byte)10);
    addByte((byte)8);
    addByte((byte)25);
    return endMessage(DIR_FROM_SPARK, msgNumber);
}

vector<CmdData> SparkMessage::sendHWChecksums(byte msgNumber, ByteVector checksums) {
    cmd = 0x03;
    subCmd = 0x2A;

    if (checksums.size() == 0) {
        checksums = {0x50, 0x16, 0x8F, 0x58};
        // checksums = {0x8e, 0x75, 0x67, 0x2a};
        // checksums = {0xA5, 0xAE, 0xC2, 0x8E};
    }

    startMessage(cmd, subCmd);

    // 0D 147D 4C07 5A58
    /*
    addByte(0x94);
    addByte(0x7D);
    addByte(0xCC);
    addByte(0x87);
    addByte(0x5A);
    addByte(0x58);
     */

    // Spark 40: 94 CC 8e 75 67 2a
    // Spark GO: f001 0779 032a   07 14 4c 1e 75 67 2a       f7

    // Vector with size
    addByte(0x94);
    addByte(0xCC);
    for (byte bt : checksums) {
        addByte(bt);
    }

    return endMessage(DIR_FROM_SPARK, msgNumber);
}

vector<CmdData> SparkMessage::sendHWPresetNumber(byte msgNumber) {
    cmd = 0x03;
    subCmd = 0x10;

    startMessage(cmd, subCmd);
    addByte(0x00);
    addByte(0x00);
    addByte(0x00);
    return endMessage(DIR_FROM_SPARK, msgNumber);
}

vector<CmdData> SparkMessage::changePreset(const Preset &presetData,
                                           MessageDirection direction, byte msgNum) {

    if (direction == DIR_TO_SPARK) {
        cmd = 0x01;
    } else {
        cmd = 0x03;
    }
    subCmd = 0x01;

    startMessage(cmd, subCmd);
    buildPresetData(presetData, direction);
    return endMessage(direction, msgNum);
}

// This prepares a message to send an acknowledgement
vector<CmdData> SparkMessage::sendAck(byte msgNum, byte subCmd,
                                      MessageDirection dir) {

    byte cmd = 0x04;

    startMessage(cmd, subCmd);
    if (subCmd == 0x70) {
        addByte(0x00);
        addByte(0x00);
    }
    return endMessage(dir, msgNum);
}

byte SparkMessage::calculateChecksum(const ByteVector &chunk) {
    byte currentSum = 0x00;
    for (byte by : chunk) {
        currentSum ^= by;
    }
    return (byte)currentSum;
}

byte SparkMessage::calculatePresetChecksum(const ByteVector &chunk) {
    long int currentSum = 0;
    for (byte by : chunk) {
        currentSum += by;
    }
    return (byte)(currentSum % 256);
}

ByteVector SparkMessage::buildPresetData(const Preset &presetData, MessageDirection direction) {
    if (direction == DIR_TO_SPARK) {
        addByte(0x00);
        addByte(0x7F);
    } else {
        if (presetData.presetNumber == 127) {
            addByte(0x01);
            addByte(0x00);
        } else {
            addByte(0x00);
            addByte(presetData.presetNumber);
        }
    }
    addLongString(presetData.uuid);
    string name = presetData.name;
    if (name.size() > 31) {
        addLongString(name);
    } else {
        addString(name);
    }
    addString(presetData.version);
    string descr = presetData.description;
    if (descr.size() > 31) {
        addLongString(descr);
    } else {
        addString(descr);
    }
    addString(presetData.icon);
    addFloat(presetData.bpm);
    addByte((byte)(0x90 + 7)); // 7 pedals
    for (int i = 0; i < 7; i++) {
        Pedal currPedal = presetData.pedals[i];
        addString(currPedal.name);
        addOnOff(currPedal.isOn);
        vector<Parameter> currPedalParams = currPedal.parameters;
        int numberOfParameters = currPedalParams.size();
        addByte((byte)(numberOfParameters + 0x90));
        for (int p = 0; p < numberOfParameters; p++) {
            addByte((byte)p);
            addByte((byte)0x91);
            addFloat(currPedalParams[p].value);
        }
    }
    ByteVector presetOnly;
    presetOnly.assign(data.begin() + 2, data.end());
    byte checksum = calculatePresetChecksum(presetOnly);

    addByte(checksum);
    return data;
}

vector<CmdData> SparkMessage::sendAmpStatus(byte msgNumber) {

    // f0 01 03 13 03 71 10 0c 04 01 02 4d 0e 51 05 4d 06 49 1d f7
    // 10 = 0 0 0 1 0 0 0 0 ==> 0 0 0 0 1 0 0 0
    // 05 = 0 0 0 0 0 1 0 1 ==> 1 0 1 0 0 0 0 0
    cmd = 0x03;
    subCmd = 0x71;

    startMessage(cmd, subCmd);
    /*addByte(0x0C);
    addByte(0x04);
    addByte(0x01);
    addByte(0x02);
    addByte(0xCD);
    addByte(0x0E);
    addByte(0x51);
    addByte(0xCD);
    addByte(0x06);
    addByte(0xC9);
    addByte(0x1D);*/

    addByte(0x0F);
    addByte(0x02);
    addByte(0x00);
    addByte(0x00);
    addByte(0xCD);
    addByte(0x0F);
    addByte(0xA9);
    addByte(0xCD);
    addByte(0x08);
    addByte(0x33);
    addByte(0x14);
    return endMessage(DIR_FROM_SPARK, msgNumber);
}

vector<CmdData> SparkMessage::sendResponse72(byte msgNumber) {

    // f0 01 02 53 03 72 01 43 1e 00 0f f7
    cmd = 0x03;
    subCmd = 0x72;

    startMessage(cmd, subCmd);
    addByte(0xC3);
    addByte(0x1E);
    addByte(0x00);
    addByte(0x0F);
    return endMessage(DIR_FROM_SPARK, msgNumber);
}

vector<CmdData> SparkMessage::sparkLooperCommand(byte msgNumber, LooperCommand command) {

    cmd = 0x01;
    subCmd = 0x75;
    cmdDetail = command;

    startMessage(cmd, subCmd);
    addByte(command);
    return endMessage(DIR_TO_SPARK, msgNumber);
}

vector<CmdData> SparkMessage::sparkConfigAfterIntro(byte msgNumber, byte command) {

    cmd = 0x02;
    subCmd = command;

    startMessage(cmd, subCmd);
    if (command == 0x33) {
        addByte(0x00);
        addByte(0x0a);
    }
    return endMessage(DIR_TO_SPARK, msgNumber);
}

vector<CmdData> SparkMessage::updateLooperSettings(byte msgNumber, const LooperSetting &setting) {

    cmd = 0x01;
    subCmd = 0x76;
    vector<CmdData> message;

    DEBUG_PRINTF("LPSetting: BPM: %d, Count: %02x, Bars: %d, Free?: %d, Click: %d, Max duration: %d\n ", setting.bpm, setting.count, setting.bars, setting.freeIndicator, setting.click, setting.maxDuration);
    startMessage(cmd, subCmd);
    if (setting.bpm >= 128) {
        addByte(0xCC);
    }
    addByte(setting.bpm);
    addByte(setting.count);
    addByte(setting.bars);
    addOnOff(setting.freeIndicator);
    addOnOff(setting.click);
    addOnOff(setting.unknownOnOff);
    addInt16(setting.maxDuration);

    message = endMessage(DIR_TO_SPARK, msgNumber);
    return message;
}

vector<CmdData> SparkMessage::getLooperStatus(byte msgNumber) {
    cmd = 0x02;
    subCmd = 0x78;
    startMessage(cmd, subCmd);
    return endMessage(DIR_TO_SPARK, msgNumber);
}

vector<CmdData> SparkMessage::getLooperConfig(byte msgNumber) {
    cmd = 0x02;
    subCmd = 0x76;
    startMessage(cmd, subCmd);
    return endMessage(DIR_TO_SPARK, msgNumber);
}

vector<CmdData> SparkMessage::getLooperRecordStatus(byte msgNumber) {
    cmd = 0x02;
    subCmd = 0x75;
    startMessage(cmd, subCmd);
    return endMessage(DIR_TO_SPARK, msgNumber);
}

byte SparkMessage::getPresetChecksum(const Preset &preset) {
    ByteVector data = buildPresetData(preset);
    return data.back();
}
