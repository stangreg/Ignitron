/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARK_MESSAGE_H // include guard
#define SPARK_MESSAGE_H

#include "Config_Definitions.h"
#include "SparkHelper.h"
#include "SparkTypes.h"
#include <Arduino.h>
#include <algorithm>
#include <array>
#include <deque>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using ByteVector = vector<byte>;

class SparkMessage {

private:
    byte cmd;
    byte subCmd;
    byte cmdDetail = 0;
    // data types for transformation
    ByteVector data;
    vector<ByteVector> splitData8;
    vector<ByteVector> splitData7;
    vector<ByteVector> allChunks;
    byte currentMsgNumber_ = 0x00;

    const ByteVector msgFromSpark = {0x41, 0xff};
    const ByteVector msgToSpark = {0x53, 0xfe};

    // Default chunk sizes
    int maxChunkSizeToSpark_ = 0x80;
    int maxChunkSizeFromSpark_ = 0x19;

    // Default block sizes
    int maxBlockSizeToSpark_ = 0xAD;
    int maxBlockSizeFromSpark_ = 0x6A;

    bool withHeader_ = true;

    void startMessage(byte cmd_, byte sub_cmd_);
    vector<CmdData> endMessage(MessageDirection dir = DIR_TO_SPARK,
                               byte msgNumber = 0x00);

    void splitDataToChunks(int dir);
    void convertDataTo7Bit();
    void buildChunkData(byte msgNumber);
    vector<CmdData> buildMessage(MessageDirection dir, byte msgNum = 0);
    // ByteVector endMessage();
    void addBytes(const ByteVector &bytes8);
    void addByte(byte by);

    void addString(const string &packStr);
    void addLongString(const string &packStr);
    void addPrefixedString(const string &packStr, int lengthOverride = 0);
    void addFloat(float flt);
    void addOnOff(boolean onoff);
    void addInt16(unsigned int number);
    byte calculateChecksum(const ByteVector &chunk);
    byte calculatePresetChecksum(const ByteVector &chunk);
    ByteVector buildPresetData(const Preset &preset, MessageDirection direction = DIR_TO_SPARK);

public:
    SparkMessage();

    // Command messages to send to Spark
    vector<CmdData> changeEffectParameter(byte msgNum, const string &pedal, int param, float val);
    vector<CmdData> changeEffect(byte msgNum, const string &pedal1, const string &pedal2);
    vector<CmdData> changeHardwarePreset(byte msgNum, int preset_num);
    vector<CmdData> turnEffectOnOff(byte msgNum, const string &pedal, boolean enable);
    vector<CmdData> switchTuner(byte msgNum, boolean enable);
    vector<CmdData> changePreset(const Preset &presetData, MessageDirection dir = DIR_TO_SPARK, byte msgNum = 0x00);
    vector<CmdData> getCurrentPresetNum(byte msgNum);
    vector<CmdData> getCurrentPreset(byte msgNum, int hwPreset = -1);
    vector<CmdData> sendAck(byte seq, byte cmd_, MessageDirection direction = DIR_TO_SPARK);

    vector<CmdData> getAmpName(byte msgNum);
    vector<CmdData> getSerialNumber(byte msgNum);
    vector<CmdData> getHwChecksums(byte msgNum);
    vector<CmdData> getHWChecksumsExtended(byte msgNum);
    vector<CmdData> getFirmwareVersion(byte msgNum);
    vector<CmdData> getAmpStatus(byte msgNum);

    vector<CmdData> sendSerialNumber(byte msgNumber);
    vector<CmdData> sendFirmwareVersion(byte msgNumber);
    vector<CmdData> sendHWChecksums(byte msgNumber, ByteVector checksums = {});
    vector<CmdData> sendHWPresetNumber(byte msgNumber);
    vector<CmdData> sendAmpStatus(byte msgNumber);
    vector<CmdData> sendResponse72(byte msgNumber);

    vector<CmdData> sparkLooperCommand(byte msgNumber, LooperCommand command);
    vector<CmdData> sparkConfigAfterIntro(byte msgNumber, byte command);

    vector<CmdData> updateLooperSettings(byte msgNumber, const LooperSetting &setting);
    vector<CmdData> getLooperStatus(byte msgNumber);
    vector<CmdData> getLooperConfig(byte msgNumber);
    vector<CmdData> getLooperRecordStatus(byte msgNumber); // To test if that is correct

    byte getPresetChecksum(const Preset &preset);

    int &maxChunkSizeToSpark() { return maxChunkSizeToSpark_; }
    const int &maxChunkSizeToSpark() const { return maxChunkSizeToSpark_; }

    int &maxChunkSizeFromSpark() { return maxChunkSizeFromSpark_; }
    const int &maxChunkSizeFromSpark() const { return maxChunkSizeFromSpark_; }

    int &maxBlockSizeToSpark() { return maxBlockSizeToSpark_; }
    const int &maxBlockSizeToSpark() const { return maxBlockSizeToSpark_; }

    int &maxBlockSizeFromSpark() { return maxBlockSizeFromSpark_; }
    const int &maxBlockSizeFromSpark() const { return maxBlockSizeFromSpark_; }

    bool &withHeader() { return withHeader_; }
};

#endif
