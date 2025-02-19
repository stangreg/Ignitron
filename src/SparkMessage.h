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
    byte sub_cmd;
    byte cmd_detail = 0;
    // data types for transformation
    ByteVector data;
    vector<ByteVector> split_data8;
    vector<ByteVector> split_data7;
    vector<ByteVector> all_chunks;
    byte current_msg_number_ = 0x00;

    const ByteVector msg_from_spark = {0x41, 0xff};
    const ByteVector msg_to_spark = {0x53, 0xfe};

    // Default chunk sizes
    int maxChunkSizeToSpark_ = 0x80;
    int maxChunkSizeFromSpark_ = 0x19;

    // Default block sizes
    int maxBlockSizeToSpark_ = 0xAD;
    int maxBlockSizeFromSpark_ = 0x6A;

    bool with_header = true;

    void start_message(byte cmd_, byte sub_cmd_);
    vector<CmdData> end_message(int dir = DIR_TO_SPARK,
                                byte msg_number = 0x00);

    void splitDataToChunks(int dir);
    void convertDataTo7Bit();
    void buildChunkData(byte msg_number);
    vector<CmdData> buildMessage(int dir, byte msg_num = 0);
    // ByteVector end_message();
    void add_bytes(const ByteVector &bytes_8);
    void add_byte(byte by);

    void add_string(const string &pack_str);
    void add_long_string(const string &pack_str);
    void add_prefixed_string(const string &pack_str, int length_override = 0);
    void add_float(float flt);
    void add_onoff(boolean onoff);
    void add_int16(unsigned int number);
    byte calculate_checksum(const ByteVector &chunk);
    byte calculate_preset_checksum(const ByteVector &chunk);
    ByteVector build_preset_data(const Preset &preset, int direction = DIR_TO_SPARK);

public:
    SparkMessage();

    // Command messages to send to Spark
    vector<CmdData> change_effect_parameter(byte msg_num, const string &pedal, int param, float val);
    vector<CmdData> change_effect(byte msg_num, const string &pedal1, const string &pedal2);
    vector<CmdData> change_hardware_preset(byte msg_num, int preset_num);
    vector<CmdData> turn_effect_onoff(byte msg_num, const string &pedal, boolean enable);
    vector<CmdData> switchTuner(byte msg_num, boolean enable);
    vector<CmdData> change_preset(const Preset &preset_data, int dir = DIR_TO_SPARK, byte msg_num = 0x00);
    vector<CmdData> get_current_preset_num(byte msg_num);
    vector<CmdData> get_current_preset(byte msg_num, int hw_preset = -1);
    vector<CmdData> send_ack(byte seq, byte cmd_, int direction = DIR_TO_SPARK);

    vector<CmdData> get_amp_name(byte msg_num);
    vector<CmdData> get_serial_number(byte msg_num);
    vector<CmdData> get_hw_checksums(byte msg_num);
    vector<CmdData> get_hw_checksums_extended(byte msg_num);
    vector<CmdData> get_firmware_version(byte msg_num);

    vector<CmdData> send_serial_number(byte msg_number);
    vector<CmdData> send_firmware_version(byte msg_number);
    vector<CmdData> send_hw_checksums(byte msg_number, ByteVector checksums = {});
    vector<CmdData> send_hw_preset_number(byte msg_number);
    vector<CmdData> send_response_71(byte msg_number);
    vector<CmdData> send_response_72(byte msg_number);

    vector<CmdData> spark_looper_command(byte msg_number, byte command);
    vector<CmdData> spark_config_after_intro(byte msg_number, byte command);

    vector<CmdData> update_looper_settings(byte msg_number, const LooperSetting *setting);
    vector<CmdData> get_looper_status(byte msg_number);
    vector<CmdData> get_looper_config(byte msg_number);
    vector<CmdData> get_looper_record_status(byte msg_number); // To test if that is correct

    byte get_preset_checksum(const Preset &preset);

    int &
    maxChunkSizeToSpark() {
        return maxChunkSizeToSpark_;
    }

    int &maxChunkSizeFromSpark() {
        return maxChunkSizeFromSpark_;
    }

    int &maxBlockSizeToSpark() {
        return maxBlockSizeToSpark_;
    }

    int &maxBlockSizeFromSpark() {
        return maxBlockSizeFromSpark_;
    }

    bool &withHeader() {
        return with_header;
    }
    const int &maxChunkSizeToSpark() const {
        return maxChunkSizeToSpark_;
    }

    const int &maxChunkSizeFromSpark() const {
        return maxChunkSizeFromSpark_;
    }

    const int &maxBlockSizeToSpark() const {
        return maxBlockSizeToSpark_;
    }

    const int &maxBlockSizeFromSpark() const {
        return maxBlockSizeFromSpark_;
    }
};

#endif
