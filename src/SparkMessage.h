/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARK_MESSAGE_H // include guard
#define SPARK_MESSAGE_H

#include <array>
#include <vector>
#include <deque>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <Arduino.h>
#include "Config_Definitions.h"
#include "SparkHelper.h"
#include "SparkTypes.h"


using namespace std;
using ByteVector = vector<byte>;


class SparkMessage{

private:

	byte cmd;
	byte sub_cmd;
	// data types for transformation
	ByteVector data;
	vector<ByteVector> split_data8;
	vector<ByteVector> split_data7;
	vector<ByteVector> all_chunks;
	byte current_msg_number_ = 0x00;

	const ByteVector msg_from_spark = { 0x41, 0xff };
	const ByteVector msg_to_spark = { 0x53, 0xfe };

	// Default chunk sizes
	int maxChunkSizeToSpark_ = 0x80;
	int maxChunkSizeFromSpark_ = 0x19;

	// Default block sizes
	int maxBlockSizeToSpark_ = 0xAD;
	int maxBlockSizeFromSpark_ = 0x6A;

	bool with_header = true;


	void start_message (byte cmd_, byte sub_cmd_);
	vector<ByteVector> end_message(int dir = DIR_TO_SPARK,
			byte msg_number = 0x00);

	void splitDataToChunks(int dir);
	void convertDataTo7Bit();
	void buildChunkData(byte msg_number);
	vector<ByteVector> buildMessage(int dir);
	//ByteVector end_message();
	void add_bytes(const ByteVector& bytes_8);
	void add_byte(byte by);

	void add_string(const string& pack_str);
	void add_long_string(const string& pack_str);
	void add_prefixed_string(const string& pack_str, int length_override = 0);
	void add_float (float flt);
	void add_onoff (boolean onoff);
	byte calculate_checksum(const ByteVector& chunk);

public:

	SparkMessage();

	// Command messages to send to Spark
	vector<ByteVector> change_effect_parameter (byte msg_num, const string& pedal, int param, float val);
	vector<ByteVector> change_effect (byte msg_num, const string& pedal1, const string& pedal2);
	vector<ByteVector> change_hardware_preset (byte msg_num, int preset_num);
	vector<ByteVector> turn_effect_onoff (byte msg_num, const string& pedal, boolean enable);
	vector<ByteVector> create_preset(const Preset& preset_data, int dir =
			DIR_TO_SPARK, byte msg_num = 0x00);
	vector<ByteVector> get_current_preset_num(byte msg_num);
	vector<ByteVector> get_current_preset(byte msg_num, int hw_preset = -1);
	vector<ByteVector> send_ack(byte seq, byte cmd_, int direction =
			DIR_TO_SPARK);

	vector<ByteVector> get_amp_name(byte msg_num);
	vector<ByteVector> get_serial_number(byte msg_num);
	vector<ByteVector> get_hw_checksums(byte msg_num);
	vector<ByteVector> get_firmware_version(byte msg_num);
	
	
	vector<ByteVector> send_serial_number(byte msg_number);
	vector<ByteVector> send_firmware_version(byte msg_number);
	vector<ByteVector> send_hw_checksums(byte msg_number);
	vector<ByteVector> send_hw_preset_number(byte msg_number);
	vector<ByteVector> send_response_71(byte msg_number);
	vector<ByteVector> send_response_72(byte msg_number);


	int& maxChunkSizeToSpark() {
		return maxChunkSizeToSpark_;
	}

	int& maxChunkSizeFromSpark() {
		return maxChunkSizeFromSpark_;
	}

	int& maxBlockSizeToSpark() {
		return maxBlockSizeToSpark_;
	}

	int& maxBlockSizeFromSpark() {
		return maxBlockSizeFromSpark_;
	}

	bool& withHeader() {
		return with_header;
	}
	const int& maxChunkSizeToSpark() const {
		return maxChunkSizeToSpark_;
	}

	const int& maxChunkSizeFromSpark() const{
		return maxChunkSizeFromSpark_;
	}

	const int& maxBlockSizeToSpark() const{
		return maxBlockSizeToSpark_;
	}

	const int& maxBlockSizeFromSpark() const {
		return maxBlockSizeFromSpark_;
	}
};

#endif
