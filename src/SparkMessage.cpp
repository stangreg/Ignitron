/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkMessage.h"

SparkMessage::SparkMessage() :
data{}, split_data8{}, split_data7{}, cmd(0), sub_cmd(0)
{
}

void SparkMessage::start_message (byte _cmd, byte _sub_cmd){
	cmd = _cmd;
	sub_cmd = _sub_cmd;
	data = {};
	split_data8 = {};
	split_data7 = {};
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
	int data_len = data.size();
	// minimum is 1 chunk
	int num_chunks = 1;
	if (data_len > 0) {
		num_chunks = int((data_len + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE);
	}

	// split the data into chunks of maximum 0x80 bytes (still 8 bit bytes)
	// and add a chunk sub-header if a multi-chunk message

	for (int this_chunk=0; this_chunk <num_chunks; this_chunk++){
		int chunk_len = min(MAX_CHUNK_SIZE,
				data_len - (this_chunk * MAX_CHUNK_SIZE));
		ByteVector data8;
		if (num_chunks > 1){
			// we need the chunk sub-header
			// TODO: validate that num_chunks and this_chunk are in correct order
			data8.push_back(num_chunks);
			data8.push_back(this_chunk);
			data8.push_back(chunk_len);
		}
		else{
			data8 = {};
		}
		data8.insert(data8.end(), data.begin() + (this_chunk * MAX_CHUNK_SIZE),
				data.begin() + (this_chunk * MAX_CHUNK_SIZE + chunk_len));
		split_data8.push_back(data8);
	}

}

void SparkMessage::convertDataTo7Bit() {

	// so loop over each chunk
	// and in each chunk loop over every sequence of (max) 7 bytes
	// and extract the 8th bit and put in 'bit8'
	// and then add bit8 and the 7-bit sequence to data7
	for (auto chunk : split_data8){

		int chunk_len = chunk.size();
		int num_seq = int ((chunk_len + 6) / 7);
		ByteVector bytes7 = {};

		for (int this_seq = 0; this_seq < num_seq; this_seq++){
			int seq_len = min (7, chunk_len - (this_seq * 7));
			byte bit8 = 0;
			ByteVector seq = {};
			for (int ind = 0; ind < seq_len; ind++){
				byte dat = chunk[this_seq * 7 + ind];
				if ((dat & 0x80) == 0x80){
					bit8 |= (1<<ind);
				}
				dat &= 0x7f;

				seq.push_back((byte)dat);
			}

			bytes7.push_back(bit8);
			bytes7.insert(bytes7.end(), seq.begin(), seq.end());
		}
		split_data7.push_back(bytes7);
	}

}

void SparkMessage::buildChunkData(byte msg_number) {

	ByteVector chunk_header = { 0xF0, 0x01 };
	byte msg_num;
	if (msg_number == 0) {
		msg_num = 0x01;
	} else {
		msg_num = (byte) msg_number;
	}
	byte trailer = 0xF7;


	// build F0 01 chunks:
	for (auto data7bit : split_data7) {
		byte checksum = calculate_checksum(data7bit);

		ByteVector complete_chunk = chunk_header;
		complete_chunk.push_back(msg_num);
		complete_chunk.push_back(checksum);
		complete_chunk.push_back(cmd);
		complete_chunk.push_back(sub_cmd);
		complete_chunk.insert(complete_chunk.end(), data7bit.begin(), data7bit.end());
		complete_chunk.push_back(trailer);

		all_chunks.push_back(complete_chunk);

	}

}

vector<ByteVector> SparkMessage::buildMessage(int dir) {

	vector<ByteVector> final_message;

	int MAX_BLOCK_SIZE;
	if (dir == DIR_TO_SPARK) {
		// Maximum block size for messages to Spark Amp
		MAX_BLOCK_SIZE = maxBlockSizeToSpark();
	} else {
		// Maximum block size for messages sent to Spark App
		MAX_BLOCK_SIZE = maxBlockSizeFromSpark();
	}


	//Reverse order for iterating with pop
	reverse(all_chunks.begin(), all_chunks.end());


	// now we can create the final message with the message header and the chunk header
	ByteVector block_header = { 0x01, 0xFE, 0x00, 0x00 };
	ByteVector block_header_direction;
	if (dir == DIR_TO_SPARK) {
		block_header_direction = msg_to_spark;
	} else {
		block_header_direction = msg_from_spark;
	}
	ByteVector block_filler = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00 };

	int block_prefix_size = 0;
	if (with_header) {
		block_prefix_size = 16;
	}

	//create block
	ByteVector current_block = { };
	ByteVector data_remainder;

	bool new_block = true;

	// start filling with chunks and start new blocks if required
	while (all_chunks.size() > 0) {

		if (new_block && with_header) {
			int block_size = min(MAX_BLOCK_SIZE,
					SparkHelper::dataVectorNumOfBytes(all_chunks)
					+ (int) data_remainder.size() + block_prefix_size);
			current_block = block_header;
			current_block.insert(current_block.end(),
					block_header_direction.begin(),
					block_header_direction.end());
			current_block.push_back(block_size);
			current_block.insert(current_block.end(), block_filler.begin(),
					block_filler.end());
		}

		ByteVector current_chunk = all_chunks.back();
		all_chunks.pop_back();

		int remaining_block_indicator;
		int remaining_space;
		// Put remaining data to chunks as long as some left
		while (data_remainder.size() > 0) {

			remaining_space = MAX_BLOCK_SIZE - current_block.size();
			int data_to_insert = min(remaining_space, (int)data_remainder.size());
			current_block.insert(current_block.end(), data_remainder.begin(),
					data_remainder.begin() + data_to_insert);
			final_message.push_back(current_block);
			current_block.clear();
			data_remainder.assign(data_remainder.begin() + data_to_insert, data_remainder.end());
		}

		data_remainder.clear();
		// Calculating if next chunk fits into current block or if needs to be split
		remaining_block_indicator = MAX_BLOCK_SIZE - current_block.size()
											- current_chunk.size();
		remaining_space = MAX_BLOCK_SIZE - current_block.size();

		// Chunk fits and space is left
		if (remaining_block_indicator > 0) {
			current_block.insert(current_block.end(), current_chunk.begin(),
					current_chunk.end());
			new_block = false;
		}
		// Chunk fits exactly into the remaining space
		if (remaining_block_indicator == 0) {
			current_block.insert(current_block.end(), current_chunk.begin(),
					current_chunk.end());
			final_message.push_back(current_block);
			current_block.clear();
			if (all_chunks.size() > 1) {
				new_block = true;
			}
		}
		// Chunk does not fit, needs to be split between blocks
		if (remaining_block_indicator < 0) {
			//DEBUG_PRINTF("Remaining indicator = %i\n", remaining_block_indicator);
			//DEBUG_PRINTF("Remaining space = %i\n", remaining_space);
			//DEBUG_PRINTF("Current block size = %i\n", current_block.size());
			//DEBUG_PRINTF("Current chunk size = %i\n", current_chunk.size());

			current_block.insert(current_block.end(), current_chunk.begin(),
					current_chunk.begin() + remaining_space);
			data_remainder.assign(current_chunk.begin() + remaining_space,
					current_chunk.end());

			final_message.push_back(current_block);

			current_block.clear();
			new_block = true;
		}

	}
	// If there is a remainder after all chunks have been added, add to block
	if (data_remainder.size() > 0) {
		// New header needs to be created as only remainder is left if flag is set here
		if (new_block && with_header) {
			int block_size = min(MAX_BLOCK_SIZE,
					SparkHelper::dataVectorNumOfBytes(all_chunks)
					+ (int) data_remainder.size() + block_prefix_size);

			current_block = block_header;
			current_block.insert(current_block.end(),
					block_header_direction.begin(),
					block_header_direction.end());
			current_block.push_back(block_size);
			current_block.insert(current_block.end(), block_filler.begin(),
					block_filler.end());
		}
		current_block.insert(current_block.end(), data_remainder.begin(),
				data_remainder.end());
		data_remainder.clear();
	}

	if (current_block.size() > 0) {
		final_message.push_back(current_block);
		current_block.clear();
	}

	return final_message;
}

vector<ByteVector> SparkMessage::end_message(int dir, byte msg_number) {

	//TODO: split into sub functions
	DEBUG_PRINT("MESSAGE NUMBER: ");
	DEBUG_PRINT(msg_number);
	DEBUG_PRINTLN();

	vector<ByteVector> finalMessage;

	// First split all data into chunks of defined maximum size
	splitDataToChunks(dir);
	// now we can convert this to 7-bit data format with the 8-bits byte at the front
	convertDataTo7Bit();
	// build F001 chunks
	buildChunkData(msg_number);
	// build final message (with 01FE header if required)
	return buildMessage(dir);

}

void SparkMessage::add_bytes(const ByteVector& bytes_8){
	for (byte by: bytes_8){
		data.push_back(by);
	}
}

void SparkMessage::add_byte(byte by){
	data.push_back(by);
}

void SparkMessage::add_prefixed_string(const string& pack_str, int length_override){
	int str_length = length_override;
	if (str_length == 0) str_length = pack_str.size();
	ByteVector byte_pack;
	byte_pack.push_back((byte)str_length);
	byte_pack.push_back((byte)(str_length + 0xa0));
	copy(pack_str.begin(), pack_str.end(), back_inserter<ByteVector>(byte_pack));
	add_bytes (byte_pack);
}


void SparkMessage::add_string(const string& pack_str){
	int str_length = pack_str.size();
	ByteVector byte_pack;
	byte_pack.push_back((byte)(str_length + 0xa0));
	copy(pack_str.begin(), pack_str.end(), back_inserter<ByteVector>(byte_pack));
	add_bytes (byte_pack);
}

void SparkMessage::add_long_string(const string& pack_str){
	int str_length = pack_str.size();
	ByteVector byte_pack;
	byte_pack.push_back((0xD9));
	byte_pack.push_back((byte)str_length);
	copy(pack_str.begin(), pack_str.end(), back_inserter<ByteVector>(byte_pack));
	add_bytes(byte_pack);
}


void SparkMessage::add_float (float flt){

	union {
		float float_variable;
		byte temp_array[4];
	} u;
	// Overwrite bytes of union with float variable
	// ROundign float to 4 digits before converting to avoid rounding errors during conversion
	u.float_variable = roundf(flt * 10000) / 10000;
	ByteVector byte_pack;
	byte_pack.push_back((byte)0xca);
	// Assign bytes to input array
	for(int i=3; i>=0; i--){
		byte_pack.push_back(u.temp_array[i]);
	}
	//DEBUG_PRINTF("Converting float %f to HEX: ", u.float_variable);
	//for (auto byte : byte_pack) {
	//	DEBUG_PRINTF("%s", SparkHelper::intToHex(byte).c_str());
	//}
	//DEBUG_PRINTLN();
	add_bytes(byte_pack);
}

void SparkMessage::add_onoff (boolean enable){
	byte b;
	if (enable == true){
		b = 0xC3;
	}
	else{
		b = 0xC2;
	}
	add_byte(b);
}

vector<ByteVector> SparkMessage::get_current_preset_num(byte msg_num){
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
	sub_cmd = 0x10;

	start_message (cmd, sub_cmd);
	return end_message(DIR_TO_SPARK, msg_num);


}

vector<ByteVector> SparkMessage::get_current_preset(byte msg_num, int hw_preset){

	cmd = 0x02;
	sub_cmd = 0x01;
	DEBUG_PRINTF("Getting preset with message number %s\n", SparkHelper::intToHex(msg_num).c_str());
	start_message (cmd, sub_cmd);
	if (hw_preset == -1) {
		add_byte(0x10);
		add_byte(0x00);
	} else {
		add_byte(0x00);
		add_byte((byte)hw_preset-1);
	}

	// add trailing 30 bytes of 00
	for (int i=0; i<30; i++){
		add_byte(0x00);
	}


	return end_message(DIR_TO_SPARK, msg_num);
}

vector<ByteVector> SparkMessage::change_effect_parameter (byte msg_num, const string& pedal, int param, float val){
	cmd = 0x01;
	sub_cmd = 0x04;

	start_message (cmd, sub_cmd);
	add_prefixed_string (pedal);
	add_byte((byte)param);
	add_float(val);
	return end_message(DIR_TO_SPARK, msg_num);

}

vector<ByteVector> SparkMessage::change_effect (byte msg_num, const string& pedal1, const string& pedal2){
	cmd = 0x01;
	sub_cmd = 0x06;

	start_message (cmd, sub_cmd);
	add_prefixed_string (pedal1);
	add_prefixed_string (pedal2);
	return end_message(DIR_TO_SPARK, msg_num);


}

vector<ByteVector> SparkMessage::change_hardware_preset (byte msg_num, int preset_num, int dir){

	if (dir == DIR_TO_SPARK) {
		cmd = 0x01;
	} else {
		cmd = 0x03;
	}
	sub_cmd = 0x38;

	start_message (cmd, sub_cmd);
	add_byte((byte)0);
	add_byte((byte)preset_num-1);
	return end_message(dir, msg_num);

}

vector<ByteVector> SparkMessage::get_amp_name(byte msg_num){

	cmd = 0x02;
	sub_cmd = 0x11;

	start_message (cmd, sub_cmd);
	return end_message(DIR_TO_SPARK, msg_num);


}

vector<ByteVector> SparkMessage::turn_effect_onoff (byte msg_num, const string& pedal, boolean enable){
	cmd = 0x01;
	sub_cmd = 0x15;

	start_message (cmd, sub_cmd);
	add_prefixed_string (pedal);
	add_onoff(enable);
	return end_message(DIR_TO_SPARK, msg_num);
}

vector<ByteVector> SparkMessage::send_serial_number(byte msg_number) {
	cmd = 0x03;
	sub_cmd = 0x23;

	start_message(cmd, sub_cmd);
	string serial_num = "S999C999B999";
	// Spark App seems to send the F7 byte as part of the string in addition to the final F7 byte,
	// so we need to have a flexible add_prefixed_string method to increase lenght information by one
	add_prefixed_string(serial_num, serial_num.length()+1);
	add_byte(0xF7);
	return end_message(DIR_FROM_SPARK, msg_number);
}

vector<ByteVector> SparkMessage::send_firmware_version(byte msg_number) {
	cmd = 0x03;
	sub_cmd = 0x2F;

	//TODO take version string as input
	start_message(cmd, sub_cmd);

	add_byte(0xCE);
	//Version string 1.7.5.182 (old: 1.6.5.160 (160-128))
	add_byte((byte) 1);
	add_byte((byte) 7);
	add_byte((byte) 5);
	add_byte((byte) 182);
	return end_message(DIR_FROM_SPARK, msg_number);
}

vector<ByteVector> SparkMessage::send_hw_checksums(byte msg_number) {
	cmd = 0x03;
	sub_cmd = 0x2A;

	start_message(cmd, sub_cmd);

	//0D 147D 4C07 5A58
	/*
	add_byte(0x94);
	add_byte(0x7D);
	add_byte(0xCC);
	add_byte(0x87);
	add_byte(0x5A);
	add_byte(0x58);
	 */

	// 94 CC 8e 75 67 2a
	add_byte(0x94);
	add_byte(0xCC);
	add_byte(0x9e);
	add_byte(0x75);
	add_byte(0x67);
	add_byte(0x2a);

	return end_message(DIR_FROM_SPARK, msg_number);
}

vector<ByteVector> SparkMessage::send_hw_preset_number(byte msg_number) {
	cmd = 0x03;
	sub_cmd = 0x10;

	start_message(cmd, sub_cmd);
	add_byte(0x00);
	add_byte(0x00);
	add_byte(0x00);
	return end_message(DIR_FROM_SPARK, msg_number);
}

vector<ByteVector> SparkMessage::create_preset(const Preset& preset_data,
		int direction, byte msg_num) {

	if (direction == DIR_TO_SPARK) {
		cmd = 0x01;
	} else {
		cmd = 0x03;
	}
	sub_cmd = 0x01;

	start_message (cmd, sub_cmd);
	if (direction == DIR_TO_SPARK) {
		add_byte(0x00);
		add_byte(0x7F);
	} else {
		add_byte(0x01);
		add_byte(0x00);
	}
	add_long_string (preset_data.uuid);
	add_string (preset_data.name);
	add_string (preset_data.version);
	string descr = preset_data.description;
	if (descr.size() > 31){
		add_long_string (descr);
	}
	else{
		add_string (descr);
	}
	add_string (preset_data.icon);
	add_float (preset_data.bpm);
	add_byte ((byte)(0x90 + 7));        // 7 pedals
	for (int i=0; i<7; i++){
		Pedal curr_pedal = preset_data.pedals[i];
		add_string (curr_pedal.name);
		add_onoff(curr_pedal.isOn);
		vector<Parameter> curr_pedal_params = curr_pedal.parameters;
		int num_p = curr_pedal_params.size();
		add_byte ((byte)(num_p + 0x90));
		for (int p=0; p<num_p; p++){
			add_byte((byte)p);
			add_byte((byte)'\x91');
			add_float (curr_pedal_params[p].value);
		}
	}
	byte checksum = calculate_checksum(data) ^ 0x7F;
	add_byte(checksum);
	return end_message(direction, msg_num);

}


// This prepares a message to send an acknowledgement
vector<ByteVector> SparkMessage::send_ack(byte msg_num, byte sub_cmd_,
		int dir) {

	byte cmd_ = 0x04;

	start_message(cmd_, sub_cmd_);
	if (sub_cmd_ == 0x70) {
		add_byte(0x00);
		add_byte(0x00);
	}
	return end_message(dir, msg_num);

}

byte SparkMessage::calculate_checksum(const ByteVector& chunk) {
	byte current_sum = 0x00;
	for (byte by : chunk) {
		current_sum ^= by;
	}
	return (byte) current_sum;
}

vector<ByteVector> SparkMessage::send_response_71(byte msg_number) {

	// f0 01 03 13 03 71 10 0c 04 01 02 4d 0e 51 05 4d 06 49 1d f7
	// 10 = 0 0 0 1 0 0 0 0 ==> 0 0 0 0 1 0 0 0
	// 05 = 0 0 0 0 0 1 0 1 ==> 1 0 1 0 0 0 0 0
	cmd = 0x03;
	sub_cmd = 0x71;

	start_message(cmd, sub_cmd);
	add_byte(0x0C);
	add_byte(0x04);
	add_byte(0x01);
	add_byte(0x02);
	add_byte(0xCD);
	add_byte(0x0E);
	add_byte(0x51);
	add_byte(0xCD);
	add_byte(0x06);
	add_byte(0xC9);
	add_byte(0x1D);
	return end_message(DIR_FROM_SPARK, msg_number);
}

vector<ByteVector> SparkMessage::send_response_72(byte msg_number) {

	//f0 01 02 53 03 72 01 43 1e 00 0f f7
	cmd = 0x03;
	sub_cmd = 0x72;

	start_message(cmd, sub_cmd);
	add_byte(0xC3);
	add_byte(0x1E);
	add_byte(0x00);
	add_byte(0x0F);
	return end_message(DIR_FROM_SPARK, msg_number);
}
