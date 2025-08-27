/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARK_STREAM_READER_H // include guard
#define SPARK_STREAM_READER_H

#include <string>
#include <tuple>
#include <vector>

#include "Config_Definitions.h"
#include "SparkHelper.h"
#include "SparkStatus.h"
#include "SparkTypes.h"
#include "StringBuilder.h"

using namespace std;
using ByteVector = vector<byte>;

enum MessageProcessStatus {
    MSG_PROCESS_RES_COMPLETE,
    MSG_PROCESS_RES_INCOMPLETE,
    MSG_PROCESS_RES_REQUEST
};

class SparkStreamReader {
    // Parser for Spark messages (from App or Amp)
    // -------------------------------------------

private:
    StringBuilder sb;
    SparkStatus &statusObject = SparkStatus::getInstance();
    // Vector containing struct of cmd, sub_cmd, and payload
    vector<CmdData> message = {};
    // Unstructured input data, needs to go through structureData first
    vector<ByteVector> unstructuredData = {};

    // payload of a CmdData object to be interpreted. msgPos is pointing at the next byte to read
    ByteVector msgData;
    int msgPos;
    // indicator if a block received is the last one
    bool msgLastBlock = false;
    vector<ByteVector> response;

    byte lastReadByte;
    const byte endMarker = 0xF7;

    // Functions to process calls based on identified cmd/sub_cmd.
    void readAmpName();
    void readEffectParameter();
    void readEffect();
    void readEffectOnOff();
    void readHardwarePreset();
    void readHWChecksums(byte sub_cmd = 0x2a);
    void readStoreHardwarePreset();
    void readPreset();
    void readLooperSettings();
    void readLooperCommand();
    void readLooperStatus();
    void readTapTempo();
    void readMeasure();
    void readTuner();
    void readTunerOnOff();
    void readPresetRequest();
    void readAmpStatus();
    void readSerialNumber();
    void readInputVolume();

    void preProcessBlock(ByteVector &blk);
    bool blockIsStarted(ByteVector &blk);

    // Functions to structure and process input data (high level)
    vector<CmdData> readMessage(bool processHeader = true);
    boolean structureData(bool processHeader = true);
    void interpretData();
    ByteVector convertDataTo8bit(ByteVector input);
    void setInterpreter(const ByteVector &_msg);
    int runInterpreter(byte _cmd, byte _sub_cmd);

    // Low level functions to read single elements from structured payload
    byte readByte();
    string readPrefixedString();
    string readString();
    float readFloat();
    boolean readOnOff();
    unsigned int readInt16();
    int readInt();

    boolean isValidBlockWithoutHeader(const ByteVector &blk);

public:
    SparkStreamReader();

    // setting the messag so it can be structured and interpreted
    void setMessage(const vector<ByteVector> &msg_);
    const vector<CmdData> lastMessage() const { return message; }

    string getJson();

    tuple<boolean, byte, byte> needsAck(const ByteVector &block);
    MessageProcessStatus processBlock(ByteVector &block);
    AckData getLastAckAndEmpty();
    void clearMessageBuffer();
};

#endif
