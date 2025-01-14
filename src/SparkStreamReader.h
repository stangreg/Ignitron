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

#define MSG_PROCESS_RES_COMPLETE 1
#define MSG_PROCESS_RES_INCOMPLETE 2
#define MSG_PROCESS_RES_REQUEST 3

class SparkStreamReader {
    // Parser for Spark messages (from App or Amp)
    // -------------------------------------------

private:
    StringBuilder sb;
    SparkStatus &statusObject = SparkStatus::getInstance();
    // Vector containing struct of cmd, sub_cmd, and payload
    vector<CmdData> message = {};
    // Unstructured input data, needs to go through structure_data first
    vector<ByteVector> unstructured_data = {};

    // payload of a CmdData object to be interpreted. msg_pos is pointing at the next byte to read
    ByteVector msg_data;
    int msg_pos;
    // indicator if a block received is the last one
    bool msg_last_block = false;
    vector<ByteVector> response;

    byte last_read_byte;
    const byte end_marker = 0xF7;

    // Functions to process calls based on identified cmd/sub_cmd.
    void read_amp_name();
    void read_effect_parameter();
    void read_effect();
    void read_effect_onoff();
    void read_hardware_preset();
    void read_store_hardware_preset();
    void read_preset();
    void read_looper_settings();
    void read_looper_command();
    void read_looper_status();
    void read_tap_tempo();
    void read_measure();

    void preProcessBlock(ByteVector &blk);
    bool blockIsStarted(ByteVector &blk);

    // Functions to structure and process input data (high level)
    vector<CmdData> read_message(bool processHeader = true);
    boolean structure_data(bool processHeader = true);
    void interpret_data();
    void set_interpreter(const ByteVector &_msg);
    int run_interpreter(byte _cmd, byte _sub_cmd);

    // Low level functions to read single elements from structured payload
    byte read_byte();
    string read_prefixed_string();
    string read_string();
    float read_float();
    boolean read_onoff();
    unsigned int read_int16();

    boolean isValidBlockWithoutHeader(const ByteVector &blk);

public:
    SparkStreamReader();

    // setting the messag so it can be structured and interpreted
    void setMessage(const vector<ByteVector> &msg_);
    const vector<CmdData> lastMessage() const { return message; }

    string getJson();

    tuple<boolean, byte, byte> needsAck(const ByteVector &block);
    int processBlock(ByteVector &block);
    AckData getLastAckAndEmpty();
    void clearMessageBuffer();
};

#endif
