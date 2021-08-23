#ifndef SPARK_STREAM_READER_H // include guard
#define SPARK_STREAM_READER_H

#include <array>
#include <vector>
#include <iomanip>
#include <sstream>
#include <tuple>

#include "SparkHelper.hh"
#include "SparkTypes.h"

#//include <SPI.h>
#//include <Arduino.h>

using ByteVector = std::vector<byte>;

class SparkStreamReader{
	// Parser for Spark messages (from App or Amp)
	// -------------------------------------------

private:

	// String representations of processed data
	std::string raw;
	std::string text;
	std::string indent;
	std::string python;

	// Vector containing struct of cmd, sub_cmd, and payload
	std::vector<cmd_data> message = {};
	// Unstructured input data, needs to go through structure_data first
	std::vector<ByteVector> unstructured_data = {};
	// payload of a cmd_data object to be interpreted. msg_pos is pointing at the next byte to read
	ByteVector msg;
	int msg_pos;

	bool msg_last_block = false;
	std::vector<ByteVector> response;


	// In case a preset was received from Spark, it is saved here. Can then be read by main program
	preset currentSetting;
	// Preset number. Can be retrieved by main program in case it has been updated by Spark Amp.
	int currentPresetNumber = 0;
	//Flags to indicate that either preset or presetNumber have been updated
	boolean presetUpdated = false;
	boolean presetNumberUpdated = false;
	ByteVector acknowledgements;

public:

	//Constructors
	SparkStreamReader();
	// setting the messag so it can be structured and interpreted
	void setMessage(std::vector<ByteVector> msg);

	// Preset related methods to make information public
	preset getCurrentSetting();
	int getCurrentPresetNumber();
	void resetPresetNumberUpdateFlag();
	void resetPresetUpdateFlag();
	boolean isPresetNumberUpdated();
	boolean isPresetUpdated();
	std::tuple<boolean, byte, byte> needsAck(ByteVector block);
	void processBlock(ByteVector block);
	byte getLastAckAndEmpty();

	// Functions to structure and process input data (high level)
	std::vector<cmd_data> read_message();
	boolean structure_data();
	void interpret_data();
	void set_interpreter (ByteVector _msg);
	int run_interpreter (byte _cmd, byte _sub_cmd);

	// Functions to process calls based on identified cmd/sub_cmd.
	void read_effect_parameter();
	void read_effect();
	void read_hardware_preset();
	void read_store_hardware_preset();
	void read_effect_onoff();
	void read_preset();


	// Low level functions to read single elements from structured payload
	byte read_byte();
	std::string read_prefixed_string();
	std::string read_string();
	float read_float ();
	boolean read_onoff();

	// Functions to create string representations of processed data
	void start_str();
	void end_str();
	void add_indent();
	void del_indent();
	void add_python(char* python_str);
	void add_str(char* a_title, std::string a_str, char* nature = "all");
	void add_int(char* a_title, int an_int, char* nature = "all");
	void add_float(char* a_title, float a_float, char* nature = "all");
	void add_bool(char* a_title, boolean a_bool, char* nature = "all");


};

#endif 
