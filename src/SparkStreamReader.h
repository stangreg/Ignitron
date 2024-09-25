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
#include "SparkTypes.h"
#include "StringBuilder.h"

using namespace std;
using ByteVector = vector<byte>;

#define MSG_TYPE_PRESET 1
#define MSG_TYPE_HWPRESET 2
#define MSG_TYPE_FX_ONOFF 3
#define MSG_TYPE_FX_CHANGE 4
#define MSG_TYPE_FX_PARAM 5
#define MSG_TYPE_AMP_NAME 6
#define MSG_TYPE_LOOPER_SETTING 7
#define MSG_TYPE_TAP_TEMPO 8
#define MSG_TYPE_MEASURE 9

#define MSG_REQ_SERIAL = 11
#define MSG_REQ_FW_VER = 12
#define MSG_REQ_PRESET_CHK = 13
#define MSG_REQ_CURR_PRESET_NUM = 14
#define MSG_REQ_CURR_PRESET = 15

#define MSG_PROCESS_RES_COMPLETE 1
#define MSG_PROCESS_RES_INCOMPLETE 2
#define MSG_PROCESS_RES_REQUEST 3

class SparkStreamReader {
    // Parser for Spark messages (from App or Amp)
    // -------------------------------------------

private:
    StringBuilder sb;
    // Vector containing struct of cmd, sub_cmd, and payload
    vector<CmdData> message = {};
    // Unstructured input data, needs to go through structure_data first
    vector<ByteVector> unstructured_data = {};

    // payload of a CmdData object to be interpreted. msg_pos is pointing at the next byte to read
    ByteVector msg;
    int msg_pos;
    // indicator if a block received is the last one
    bool msg_last_block = false;
    vector<ByteVector> response;

    // In case a preset was received from Spark, it is saved here. Can then be read by main program
    Preset currentPreset_;
    // Preset number. Can be retrieved by main program in case it has been updated by Spark Amp.
    int currentPresetNumber_ = 0;
    // Flags to indicate that either preset or presetNumber have been updated
    boolean isPresetUpdated_ = false;
    boolean isPresetNumberUpdated_ = false;
    vector<AckData> acknowledgments;
    int last_message_type_ = 0;
    byte last_message_num_ = 0x00;
    byte last_requested_preset = 0x00;
    string ampName_ = "";
    float measure_;
    byte last_read_byte;
    const byte end_marker = 0xF7;

    LooperSetting looperSetting_;
    boolean isLooperSettingUpdated_ = false;

    // Functions to process calls based on identified cmd/sub_cmd.
    void read_amp_name();
    void read_effect_parameter();
    void read_effect();
    void read_effect_onoff();
    void read_hardware_preset();
    void read_store_hardware_preset();
    void read_preset();
    void read_looper_settings();
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

    boolean isValidBlockWithoutHeader(const ByteVector &blk);

public:
    // Constructors
    SparkStreamReader();
    // setting the messag so it can be structured and interpreted
    void setMessage(const vector<ByteVector> &msg_);
    string getJson();

    // Preset related methods to make information public
    const Preset currentPreset() const { return currentPreset_; }
    const int currentPresetNumber() const { return currentPresetNumber_; }
    const boolean isPresetNumberUpdated() const { return isPresetNumberUpdated_; }
    const boolean isPresetUpdated() const { return isPresetUpdated_; }
    const boolean isLooperSettingUpdated() const { return isLooperSettingUpdated_; }
    const LooperSetting currentLooperSetting() const { return looperSetting_; }

    const int lastMessageType() const { return last_message_type_; }
    const byte lastMessageNum() const {
        return last_message_num_;
    }
    const vector<CmdData> lastMessage() const {
        return message;
    }
    string getAmpName() { return ampName_; }
    float getMeasure() { return measure_; }

    void resetPresetNumberUpdateFlag();
    void resetPresetUpdateFlag();
    void resetLooperSettingUpdateFlag();
    void resetLastMessageType();
    tuple<boolean, byte, byte> needsAck(const ByteVector &block);
    int processBlock(ByteVector &block);
    AckData getLastAckAndEmpty();
    void clearMessageBuffer();
};

#endif
