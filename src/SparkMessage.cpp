/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkMessage.h"

SparkMessage::SparkMessage(){
	data = {};
	split_data8={};
	split_data7={};
	cmd=0;
	sub_cmd=0;
}

void SparkMessage::start_message (byte _cmd, byte _sub_cmd){
	cmd = _cmd;
	sub_cmd = _sub_cmd;
	data = {};
	split_data8 = {};
	split_data7 = {};
	final_message = {};
};

std::vector<ByteVector> SparkMessage::end_message(int dir, byte msg_number) {

	//TODO: split into sub functions

	int max_chunk_size;
	if (dir == DIR_TO_SPARK) {
		max_chunk_size = 0x80;
	} else {
		max_chunk_size = 0x19;
	}

	// determine how many chunks there are
	int data_len = data.size();
	int num_chunks = int((data_len + max_chunk_size - 1) / max_chunk_size);



	// split the data into chunks of maximum 0x80 bytes (still 8 bit bytes)
	// and add a chunk sub-header if a multi-chunk message

	for (int this_chunk=0; this_chunk <num_chunks; this_chunk++){
		int chunk_len = min(max_chunk_size,
				data_len - (this_chunk * max_chunk_size));
		ByteVector data8;
		if (num_chunks > 1){
			// we need the chunk sub-header
			data8.push_back(num_chunks);
			data8.push_back(this_chunk);
			data8.push_back(chunk_len);
		}
		else{
			data8 = {};
		}
		data8.insert(data8.end(), data.begin() + (this_chunk * max_chunk_size),
				data.begin() + (this_chunk * max_chunk_size + chunk_len));
		split_data8.push_back(data8);
	}

	// now we can convert this to 7-bit data format with the 8-bits byte at the front
	// so loop over each chunk
	// and in each chunk loop over every sequence of (max) 7 bytes
	// and extract the 8th bit and put in 'bit8'
	// and then add bit8 and the 7-bit sequence to data7
	for (auto chunk : split_data8){
		int chunk_len = chunk.size();
		//DEBUG_PRINTF("Chunk size 8bit: %d\n", chunk_len);
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
		//DEBUG_PRINTLN("7-bit chunk:");
		//SparkHelper::printByteVector(bytes7);
		//DEBUG_PRINTF("Chunk size 7bit: %d\n", bytes7.size());
		split_data7.push_back(bytes7);
	}


	int MAX_BLOCK_SIZE;
	if (dir == DIR_TO_SPARK) {
		MAX_BLOCK_SIZE = 0xAD;
	} else {
		MAX_BLOCK_SIZE = 0x6A;
	}
		
	

	// now we can create the final message with the message header and the chunk header
	ByteVector block_header = { '\x01', '\xfe', '\x00', '\x00' };
	ByteVector block_header_direction;
	if (dir == DIR_TO_SPARK) {
		block_header_direction = msg_to_spark;
	} else {
		block_header_direction = msg_from_spark;
	}
	ByteVector block_filler = {'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00'};
	ByteVector chunk_header = { '\xf0', '\x01' };
	byte msg_num;
	if (msg_number == 0) {
		msg_num = '\x01';
	} else {
		msg_num = (byte) msg_number;
	}
	byte trailer = '\xf7';

	std::vector<ByteVector> all_chunks;

	// build F0 01 chunks:
	for (auto data : split_data7) {
		byte checksum = calculate_checksum(data);

		ByteVector complete_chunk = chunk_header;
		complete_chunk.push_back(msg_num);
		complete_chunk.push_back(checksum);
		complete_chunk.push_back(cmd);
		complete_chunk.push_back(sub_cmd);
		complete_chunk.insert(complete_chunk.end(), data.begin(), data.end());
		complete_chunk.push_back(trailer);
		//DEBUG_PRINTLN("Pushing chunk");
		all_chunks.push_back(complete_chunk);

	}

	//Reverse order for iterating with pop
	std::reverse(all_chunks.begin(), all_chunks.end());

	int block_prefix_size = 16;
	int max_data_size = MAX_BLOCK_SIZE - block_prefix_size;

	int block_size = min(MAX_BLOCK_SIZE,
			SparkHelper::dataVectorNumOfBytes(all_chunks) + block_prefix_size);

	//create block
	ByteVector current_block;
	ByteVector data_remainder;

	bool new_block = true;

	// start filling with chunks and start new blocks if required
	//DEBUG_PRINTF("All chunks size: %d\n", all_chunks.size());
	while (all_chunks.size() > 0) {

		if (new_block) {

			//DEBUG_PRINTLN("Starting new block");
			block_size = min(MAX_BLOCK_SIZE,
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
		//DEBUG_PRINTF("All chunks size after pop: %d\n", all_chunks.size());

		//DEBUG_PRINTF("Size of current chunk: %d\n", current_chunk.size());

		if (data_remainder.size() > 0) {
			current_block.insert(current_block.end(), data_remainder.begin(),
					data_remainder.end());
			data_remainder.clear();
		}

		int remaining_block_indicator = MAX_BLOCK_SIZE - current_block.size()
				- current_chunk.size();
		//DEBUG_PRINTF("Remaining MAX, BLOCK, CHUNK: %d, %d, %d\n",
		//MAX_BLOCK_SIZE, current_block.size(), current_chunk.size());
		int remaining_space = MAX_BLOCK_SIZE - current_block.size();

		if (remaining_block_indicator > 0) {
			//DEBUG_PRINTLN("Adding new chunk to block");
			current_block.insert(current_block.end(), current_chunk.begin(),
					current_chunk.end());
			new_block = false;
		}
		if (remaining_block_indicator == 0) {
			//DEBUG_PRINTLN("Pushing block, exact match");
			current_block.insert(current_block.end(), current_chunk.begin(),
					current_chunk.end());
			final_message.push_back(current_block);
			current_block.clear();
			if (all_chunks.size() > 1) {
				new_block = true;
			}
		}
		if (remaining_block_indicator < 0) {
			current_block.insert(current_block.end(), current_chunk.begin(),
					current_chunk.begin() + remaining_space);
			data_remainder.assign(current_chunk.begin() + remaining_space,
					current_chunk.end());
			//DEBUG_PRINTLN("Pushing block, remainder");
			final_message.push_back(current_block);
			current_block.clear();
			new_block = true;
		}

	}
	
	if (data_remainder.size() > 0) {
		// New header needs to be created as only remainder is left if flag is set here
		if (new_block == true) {
			//DEBUG_PRINTLN("Starting new block");
			block_size = min(MAX_BLOCK_SIZE,
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
		//DEBUG_PRINTLN("Pushing last block");
		final_message.push_back(current_block);
		current_block.clear();
	}
	
	return final_message;

}

void SparkMessage::add_bytes(ByteVector bytes_8){
	for (byte by: bytes_8){
		data.push_back(by);
	}
}

void SparkMessage::add_byte(byte by){
	data.push_back(by);
}

void SparkMessage::add_prefixed_string(std::string pack_str){
	int str_length = pack_str.size();
	ByteVector byte_pack;
	byte_pack.push_back((byte)str_length);
	byte_pack.push_back((byte)(str_length + 0xa0));
	std::copy(pack_str.begin(), pack_str.end(), std::back_inserter<ByteVector>(byte_pack));
	add_bytes (byte_pack);
}


void SparkMessage::add_string(std::string pack_str){
	int str_length = pack_str.size();
	ByteVector byte_pack;
	byte_pack.push_back((byte)(str_length + 0xa0));
	std::copy(pack_str.begin(), pack_str.end(), std::back_inserter<ByteVector>(byte_pack));
	add_bytes (byte_pack);
}

void SparkMessage::add_long_string(std::string pack_str){
	int str_length = pack_str.size();
	ByteVector byte_pack;
	byte_pack.push_back(('\xd9'));
	byte_pack.push_back((byte)str_length);
	std::copy(pack_str.begin(), pack_str.end(), std::back_inserter<ByteVector>(byte_pack));
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
		b = '\xc3';
	}
	else{
		b = '\xc2';
	}
	add_byte(b);
}

std::vector<ByteVector> SparkMessage::get_current_preset_num(){
	// hardcoded message
	std::vector<ByteVector> msg;
	ByteVector msg_vec = {0x01, 0xfe, 0x00, 0x00, 0x53, 0xfe,
			0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0xf0, 0x01, 0x08, 0x00, 0x02, 0x10, 0xf7}; // had an extra 0x79 at the end, likely not needed
	msg.push_back(msg_vec);
	return msg;
}

std::vector<ByteVector> SparkMessage::get_current_preset(){
	// hardcoded message
	std::vector<ByteVector> msg;
	ByteVector msg_vec = {0x01, 0xfe, 0x00, 0x00, 0x53, 0xfe,
			0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0xf0, 0x01, 0x0a, 0x01, 0x02, 0x01, 0x00, 0x01, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0xf7};

	msg.push_back(msg_vec);
	return msg;
}

std::vector<ByteVector> SparkMessage::change_effect_parameter (std::string pedal, int param, float val){
	cmd = '\x01';
	sub_cmd = '\x04';

	start_message (cmd, sub_cmd);
	add_prefixed_string (pedal);
	add_byte((byte)param);
	add_float(val);
	return end_message();

}

std::vector<ByteVector> SparkMessage::change_effect (std::string pedal1, std::string pedal2){
	cmd = '\x01';
	sub_cmd = '\x06';

	start_message (cmd, sub_cmd);
	add_prefixed_string (pedal1);
	add_prefixed_string (pedal2);
	return end_message();


}

std::vector<ByteVector> SparkMessage::change_hardware_preset (int preset_num){
	cmd = '\x01';
	sub_cmd = '\x38';

	start_message (cmd, sub_cmd);
	add_byte((byte)0);
	add_byte((byte)preset_num-1);
	return end_message();

}

std::vector<ByteVector> SparkMessage::turn_effect_onoff (std::string pedal, boolean enable){
	cmd = '\x01';
	sub_cmd = '\x15';

	start_message (cmd, sub_cmd);
	add_prefixed_string (pedal);
	add_onoff(enable);
	return end_message();
}

std::vector<ByteVector> SparkMessage::send_serial_number(byte msg_number) {
	cmd = '\x03';
	sub_cmd = '\x23';

	start_message(cmd, sub_cmd);
	add_prefixed_string("S999C999B999");
	add_byte(0x01);
	add_byte(0x77);
	return end_message(DIR_FROM_SPARK, msg_number);
}

std::vector<ByteVector> SparkMessage::send_firmware_version(byte msg_number) {
	cmd = '\x03';
	sub_cmd = '\x2F';

	//TODO take version string as input
	start_message(cmd, sub_cmd);
	//add_byte(0x11);
	add_byte(0xCE);
	//Version string 1.6.5.160
	add_byte((byte) 1);
	add_byte((byte) 6);
	add_byte((byte) 5);
	add_byte((byte) 160 - 128);
	return end_message(DIR_FROM_SPARK, msg_number);
}

std::vector<ByteVector> SparkMessage::send_hw_checksums(byte msg_number) {
	cmd = '\x03';
	sub_cmd = '\x2A';

	// TODO : Take checksums as input
	ByteVector checksums = { 0x14, 0x50, 0x4C, 0x70, 0x5A, 0x58 };
	//TODO take version string as input
	start_message(cmd, sub_cmd);
	add_byte(0x94);
	add_byte(0x50);
	add_byte(0xCC);
	add_byte(0xF0);
	add_byte(0x5A);
	add_byte(0x58);
	return end_message(DIR_FROM_SPARK, msg_number);
}

std::vector<ByteVector> SparkMessage::send_hw_preset_number(byte msg_number) {
	cmd = '\x03';
	sub_cmd = '\x10';

	start_message(cmd, sub_cmd);
	add_byte(0x00);
	add_byte(0x00);
	add_byte(0x00);
	return end_message(DIR_FROM_SPARK, msg_number);
}

std::vector<ByteVector> SparkMessage::create_preset(Preset preset_data,
		int direction, byte msg_num) {

	if (direction == DIR_TO_SPARK) {
		cmd = '\x01';
	} else {
		cmd = '\x03';
	}
	sub_cmd = '\x01';

	start_message (cmd, sub_cmd);
	if (direction == DIR_TO_SPARK) {
		add_byte('\x00');
		add_byte('\x7f');
	} else {
		add_byte('\x01');
		add_byte('\x00');
	}
	add_long_string (preset_data.uuid);
	add_string (preset_data.name);
	add_string (preset_data.version);
	std::string descr = preset_data.description;
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
		std::vector<Parameter> curr_pedal_params = curr_pedal.parameters;
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


// This prepares a message to send an acknowledgement to Spark App
std::vector<ByteVector> SparkMessage::send_ack(byte seq, byte sub_cmd) {
	std::vector<ByteVector> ack_cmd;
	ack_cmd.clear();
	ByteVector ack = { 0x01, 0xfe, 0x00, 0x00, 0x41, 0xff, 0x17, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x01 };
	ack.push_back(seq);
	ack.push_back(0x00);
	ack.push_back(0x04);
	ack.push_back(sub_cmd);

	//	ack.push_back(0x00);
	//	ack.push_back(0x00);

	ack.push_back(0xf7);

	ack_cmd.push_back(ack);
	return ack_cmd;
}

byte SparkMessage::calculate_checksum(ByteVector chunk) {
	byte current_sum = 0x00;
	for (byte by : chunk) {
		current_sum ^= by;
	}
	return (byte) current_sum;
}

