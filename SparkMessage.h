#ifndef SPARK_MESSAGE_H // include guard
#define SPARK_MESSAGE_H

#include <array>
#include <vector>
#include <iomanip>
#include <sstream>
#include <Arduino.h>
#include "SparkHelper.h"

#include "SparkTypes.h"

using ByteVector = std::vector<byte>;

class SparkMessage {

private:

	byte cmd;
	byte sub_cmd;
	// data types to be confirmed
	std::vector<ByteVector> split_data8;
	std::vector<ByteVector> split_data7;
	ByteVector data;
	std::vector<ByteVector> final_message;

	void start_message(byte cmd, byte sub_cmd);
	std::vector<ByteVector> end_message();
	//ByteVector end_message();
	void add_bytes(ByteVector bytes_8);
	void add_byte(byte by);

	void add_string(std::string pack_str);
	void add_long_string(std::string pack_str);
	void add_prefixed_string(std::string pack_str);
	void add_float(float flt);
	void add_onoff(boolean onoff);

public:

	SparkMessage();

	// Command messages to send to Spark
	std::vector<ByteVector> change_effect_parameter(std::string pedal,
			int param, float val);
	std::vector<ByteVector> change_effect(std::string pedal1,
			std::string pedal2);
	std::vector<ByteVector> change_hardware_preset(int preset_num);
	std::vector<ByteVector> turn_effect_onoff(std::string pedal,
			boolean enable);
	std::vector<ByteVector> create_preset(preset preset_data);
	std::vector<ByteVector> get_current_preset_num();
	std::vector<ByteVector> get_current_preset();
	std::vector<ByteVector> send_ack(byte seq, byte cmd);

};

#endif
