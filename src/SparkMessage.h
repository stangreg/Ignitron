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
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <Arduino.h>
#include "Config_Definitions.h"
#include "SparkHelper.h"
#include "SparkTypes.h"



using ByteVector = std::vector<byte>;

class SparkMessage{

private:

	byte cmd;
	byte sub_cmd;
	// data types to be confirmed
	std::vector<ByteVector> split_data8;
	std::vector<ByteVector> split_data7;
	ByteVector data;
	std::vector<ByteVector> final_message;
	byte current_msg_number_ = 0x00;

	const ByteVector msg_from_spark = { '\x41', '\xff' };
	const ByteVector msg_to_spark = { '\x53', '\xfe' };


	void start_message (byte cmd_, byte sub_cmd_);
	std::vector<ByteVector> end_message(int dir = DIR_TO_SPARK,
			byte msg_number = 0x00);
	//ByteVector end_message();
	void add_bytes(ByteVector bytes_8);
	void add_byte(byte by);

	void add_string(std::string pack_str);
	void add_long_string(std::string pack_str);
	void add_prefixed_string(std::string pack_str, int length_override = 0);
	void add_float (float flt);
	void add_onoff (boolean onoff);
	byte calculate_checksum(ByteVector chunk);

public:

	SparkMessage();

	// Command messages to send to Spark
	std::vector<ByteVector> change_effect_parameter (byte msg_num, std::string pedal, int param, float val);
	std::vector<ByteVector> change_effect (byte msg_num, std::string pedal1, std::string pedal2);
	std::vector<ByteVector> change_hardware_preset (byte msg_num, int preset_num);
	std::vector<ByteVector> turn_effect_onoff (byte msg_num, std::string pedal, boolean enable);
	std::vector<ByteVector> create_preset(Preset preset_data, int dir =
	DIR_TO_SPARK, byte msg_num = 0x00);
	std::vector<ByteVector> get_current_preset_num(byte msg_num);
	std::vector<ByteVector> get_current_preset(byte msg_num, int hw_preset = -1);
	std::vector<ByteVector> send_ack(byte seq, byte cmd_, int direction =
			DIR_TO_SPARK);

	std::vector<ByteVector> send_serial_number(byte msg_number);
	std::vector<ByteVector> send_firmware_version(byte msg_number);
	std::vector<ByteVector> send_hw_checksums(byte msg_number);
	std::vector<ByteVector> send_hw_preset_number(byte msg_number);
	std::vector<ByteVector> send_response_71(byte msg_number);
	std::vector<ByteVector> send_response_72(byte msg_number);

};

#endif
