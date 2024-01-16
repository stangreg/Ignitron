/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkStreamReader.h"


SparkStreamReader::SparkStreamReader(){
	message = {};
	unstructured_data = {};
	msg = {};
	msg_pos = 0;
}

std::string SparkStreamReader::getJson(){
	return json;
}

void SparkStreamReader::setMessage(std::vector<ByteVector> msg_){
	unstructured_data = msg_;
	message.clear();
}


void SparkStreamReader::resetPresetNumberUpdateFlag(){
	isPresetNumberUpdated_ = false;
}

void SparkStreamReader::resetPresetUpdateFlag(){
	isPresetUpdated_ = false;
}

void SparkStreamReader::resetLastMessageType(){
	last_message_type_ = 0;
}


byte SparkStreamReader::read_byte() {
	byte a_byte;
	a_byte = msg[msg_pos];
	msg_pos += 1;
	return a_byte;
}

std::string SparkStreamReader::read_prefixed_string() {
	int str_len = read_byte();
	// offset removed from string length byte to get real length
	int real_str_len = read_byte() - 0xa0;
	std::string a_str = "";
	// reading string
	for (int i = 0; i < real_str_len; i++) {
		a_str += char(read_byte());
	}
	return a_str;
}

std::string SparkStreamReader::read_string() {
	byte a_byte = read_byte();
	int str_len;
	if (a_byte == 0xd9) {
		a_byte = read_byte();
		str_len = a_byte;
	}
	else if (a_byte >= 0xa0) {
		str_len = a_byte - 0xa0;
	}
	else {
		a_byte = read_byte();
		str_len = a_byte - 0xa0;
	}

	std::string a_str = "";
	for (int i = 0; i < str_len; i++) {
		a_str += char(read_byte());
	}
	return a_str;
}

// floats are special - bit 7 is actually stored in the format byte and not in the data
float SparkStreamReader::read_float () {
	byte prefix = read_byte(); // should be ca

	// using union struct to share memory for easy transformation of bytes to float
	union {
		float f;
		unsigned long ul;
	} u;

	byte a, b, c, d;
	a= read_byte();
	b= read_byte();
	c= read_byte();
	d= read_byte();
	u.ul = (a << 24) | (b << 16) | (c << 8) | d;
	float val = u.f;
	return val;
}

boolean SparkStreamReader::read_onoff() {
	byte a_byte = read_byte();
	if (a_byte == 0xc3) {
		return true;
	}
	else if (a_byte == 0xc2) {
		return false;
	}
	else {
		return "?";
	}
}

void SparkStreamReader::start_str() {
	text = "";
	json = "{";
	raw = "";
	//dict={};
	indent = "";
}

void SparkStreamReader::end_str() {
	json += "}";
}

void SparkStreamReader::add_indent() {
	indent += "\t";
}

void SparkStreamReader::del_indent() {
	indent = indent.substr(1);
}

void SparkStreamReader::add_separator() {
	json += ", ";
}

void SparkStreamReader::add_newline(){
	json += "\n";
}

void SparkStreamReader::add_python(char* python_str) {
	json += indent + python_str;// + "\n";
}

void SparkStreamReader::add_str(char* a_title, std::string a_str, char* nature) {
	raw +=  a_str;
	raw += " ";
	char string_add[200] = "";
	sprintf(string_add, "%s%-20s: %s \n", indent.c_str(), a_title, a_str.c_str());
	text += string_add;
	if (nature != "python") {
		json += indent + "\"" + a_title + "\": \"" + a_str + "\"";
	}
}


void SparkStreamReader::add_int(char* a_title, int an_int, char* nature) {
	char string_add[100] = "";
	sprintf(string_add, "%d ", an_int);
	raw += string_add;
	sprintf(string_add, "%s%-20s: %d\n", indent.c_str(), a_title, an_int);
	text += string_add;
	if (nature != "python") {
		sprintf(string_add, "%s\"%s\": %d", indent.c_str(), a_title, an_int);
		json += string_add;
	}
}



void SparkStreamReader::add_float(char* a_title, float a_float, char* nature) {

	char string_add[100] = "";
	sprintf(string_add, "%2.4f ", a_float);
	raw += string_add;
	sprintf(string_add, "%s%-20s: %2.4f\n", indent.c_str(), a_title, a_float);
	text += string_add;
	if (nature == "python") {
		sprintf(string_add, "%s%2.4f", indent.c_str(), a_float);
		json += string_add;
	}
	else {
		sprintf(string_add, "%s\"%s\": %2.4f",  indent.c_str(), a_title, a_float);
		json += string_add;
	}
}

void SparkStreamReader::add_float_pure(float a_float, char* nature) {
	char string_add[100] = "";
	sprintf(string_add, "%2.4f ", a_float);
	raw += string_add;
	sprintf(string_add, "%2.4f ", a_float);
	text += string_add;
	sprintf(string_add, "%2.4f", a_float);
	json += string_add;
}

void SparkStreamReader::add_bool(char* a_title, boolean a_bool, char* nature) {
	char string_add[100] = "";
	sprintf(string_add, "%s ", a_bool ? "true" : "false");
	raw += string_add;
	sprintf(string_add, "%s%s: %-20s\n", indent.c_str(), a_title, a_bool ? "true" : "false");
	text += string_add;
	if (nature != "python") {
		sprintf(string_add, "%s\"%s\": %s", indent.c_str(), a_title, a_bool ? "true" : "false");
		json += string_add;
	}
}


void SparkStreamReader::read_effect_parameter() {
	// Read object
	std::string effect = read_prefixed_string ();
	byte param = read_byte ();
	float val = read_float();

	// Build string representations
	start_str();
	add_str ("Effect", effect);
	add_separator();
	add_int ("Parameter", param);
	add_separator();
	add_float ("Value", val);
	end_str();

	// Set values
	last_message_type_ = MSG_TYPE_FX_PARAM;
}

void SparkStreamReader::read_effect() {
	//Read object
	std::string effect1 = read_prefixed_string ();
	std::string effect2 = read_prefixed_string ();

	// Build string representations
	start_str();
	add_str ("OldEffect", effect1);
	add_separator();
	add_newline();
	add_str ("NewEffect", effect2);
	end_str();

	// Set values
	last_message_type_ = MSG_TYPE_FX_CHANGE;

}

void SparkStreamReader::read_hardware_preset() {
	// Read object
	read_byte ();
	byte preset_num = read_byte () + 1;

	// Build string representations
	start_str();
	add_int ("New HW Preset", preset_num);
	end_str();

	// Set values
	currentPresetNumber_ = preset_num;
	isPresetNumberUpdated_ = true;
	last_message_type_ = MSG_TYPE_HWPRESET;

}

void SparkStreamReader::read_store_hardware_preset() {
	// Read object
	read_byte ();
	byte preset_num = read_byte () + 1;

	// Build string representations
	start_str();
	add_int ("NewStoredPreset", preset_num);
	end_str();

	// Set values
	last_message_type_ = MSG_TYPE_HWPRESET;

}

void SparkStreamReader::read_effect_onoff() {
	// Read object
	std::string effect = read_prefixed_string ();
	boolean isOn = read_onoff ();

	// Build string representations
	start_str();
	add_str ("Effect", effect);
	add_separator();
	add_bool ("IsOn", isOn);
	end_str();

	// Set values
	last_message_type_ = MSG_TYPE_FX_ONOFF;

}

void SparkStreamReader::read_preset() {
	// Read object (Preset main data)
	//DEBUG_PRINTF("Free memory before reading preset: %d\n", xPortGetFreeHeapSize());
	read_byte();
	byte preset = read_byte();
	//DEBUG_PRINTF("Read PresetNumber: %d\n", preset);
	currentSetting_.presetNumber = preset;
	std::string uuid = read_string();
	//DEBUG_PRINTF("Read UUID: %s\n", uuid.c_str());
	currentSetting_.uuid = uuid;
	std::string name = read_string();
	//DEBUG_PRINTF("Read Name: %s\n", name.c_str());
	currentSetting_.name = name;
	std::string version = read_string();
	//DEBUG_PRINTF("Read Version: %s\n", version.c_str());
	currentSetting_.version = version;
	std::string descr = read_string();
	//DEBUG_PRINTF("Read Description: %s\n", descr.c_str());
	currentSetting_.description = descr;
	std::string icon = read_string();
	//DEBUG_PRINTF("Read Icon: %s\n", icon.c_str());
	currentSetting_.icon = icon;
	float bpm = read_float();
	//DEBUG_PRINTF("Read BPM: %f\n", bpm);
	currentSetting_.bpm = bpm;
	// Build string representations
	//DEBUG_PRINTF("Free memory before adds: %d\n", xPortGetFreeHeapSize());
	start_str();
	add_int ("PresetNumber", preset);
	add_separator();
	add_str("UUID", uuid);
	add_separator();
	add_newline();
	add_str("Name", name);
	add_separator();
	add_str("Version", version);
	add_separator();
	add_str("Description", descr);
	add_separator();
	add_str("Icon", icon);
	add_separator();
	add_float("BPM", bpm);
	add_separator();
	add_newline();
	//DEBUG_PRINTF("Free memory after adds: %d\n", xPortGetFreeHeapSize());
	// Read Pedal data (including string representations)
	int num_effects = read_byte() - 0x90;
	//DEBUG_PRINTF("Read Number of effects: %d\n", num_effects);
	add_python("\"Pedals\": [");
	add_newline();
	currentSetting_.pedals = {};
	for (int i = 0; i < currentSetting_.numberOfPedals; i++) { // Fixed to 7, but could maybe also be derived from num_effects?
		Pedal currentPedal = {};
		//DEBUG_PRINTF("Reading Pedal %d:\n", i);
		std::string e_str = read_string();
		//DEBUG_PRINTF("  Pedal name: %s\n", e_str.c_str());
		currentPedal.name = e_str;
		boolean e_onoff = read_onoff();
		//DEBUG_PRINTF("  Pedal state: %s\n", e_onoff);
		currentPedal.isOn = e_onoff;
		add_python ("{");
		add_str("Name", e_str);
		add_separator();
		add_bool("IsOn", e_onoff);
		add_separator();
		int num_p = read_byte() - char(0x90);
		//DEBUG_PRINTF("  Number of Parameters: %d\n", num_p);
		//DEBUG_PRINTF("Free memory before parameters: %d\n", xPortGetFreeHeapSize());
		add_python("\"Parameters\":[");
		// Read parameters of current pedal
		currentPedal.parameters = {};
		for (int p = 0; p < num_p; p++) {
			Parameter currentParameter = {};
			//DEBUG_PRINTF("  Reading parameter %d:\n", p);
			byte num = read_byte();
			//DEBUG_PRINTF("    Parameter ID: %d\n", SparkHelper::intToHex(num));
			byte spec = read_byte();
			//DEBUG_PRINTF("    Parameter Special: %s\n",
			//	SparkHelper::intToHex(spec));
			//DEBUG_PRINTF("Free memory before reading float: %d\n", xPortGetFreeHeapSize());
			float val = read_float();
			//DEBUG_PRINTF("    Parameter Value: %f\n", val);
			currentParameter.number = num;
			currentParameter.special = spec;
			currentParameter.value = val;
			//add_int("Parameter", num, "python");
			//add_str("Special", SparkHelper::intToHex(spec), "python");
			//add_float("Value", val, "python");
			add_float_pure(val, "python");
			if(p < num_p - 1){
				add_separator();
			}
			currentPedal.parameters.push_back(currentParameter);
			//DEBUG_PRINTF("Free memory after reading preset: %d\n", xPortGetFreeHeapSize());
		}
		
		add_python("]");
		//del_indent();
		add_python("}");
		if (i < currentSetting_.numberOfPedals - 1) {
			add_separator();
			add_newline();
		}
		currentSetting_.pedals.push_back(currentPedal);
	}
	add_python("],");
	add_newline();
	byte filler = read_byte();
	//DEBUG_PRINTF("Preset filler ID: %s\n", SparkHelper::intToHex(filler));
	currentSetting_.filler = filler;
	add_str("Filler", SparkHelper::intToHex(filler));
	add_newline();
	end_str();
	currentSetting_.text = text;
	currentSetting_.raw = raw;
	currentSetting_.json = json;
	currentSetting_.isEmpty = false;
	isPresetUpdated_ = true;
	last_message_type_ = MSG_TYPE_PRESET;

}

boolean SparkStreamReader::structure_data(bool processHeader) {

	ByteVector block_content;
	block_content.clear();
	message.clear();

	//DEBUG_PRINTLN("Structuring data..:");
	//SparkHelper::printDataAsHexString(unstructured_data);
	for (auto block : unstructured_data) {
		//DEBUG_PRINTLN("Processing block");
		//SparkHelper::printByteVector(block);
		//DEBUG_PRINTLN();
		//DEBUG_PRINTF("Current heap size: %d\n", ESP.getFreeHeap());
		int block_length;
		if(processHeader){
			block_length = block[6];
		}
		else {
			block_length = block.size();
		}
		int data_size = block.size();
		//DEBUG_PRINTF("Read block size %d, %d\n", block_length, data_size);
		if ( data_size != block_length) {
			DEBUG_PRINTF("Data is of size %d and reports %d\n", data_size, block_length );
			Serial.println("Corrupt block:");
			for (auto by : block){
				Serial.print(SparkHelper::intToHex(by).c_str());
			}
			Serial.println();
		}
		//DEBUG_PRINTLN("Sizes match");
		ByteVector chunk;
		if(processHeader){
			//Cut away header
			chunk.assign(block.begin() + 16, block.end());
		}
		else {
			chunk.assign(block.begin(), block.end());
		}
		//DEBUG_PRINTF("Assigned block of size %d", chunk.size());

		for (auto chunk_byte : chunk) {
			//DEBUG_PRINT(SparkHelper::intToHex(chunk_byte).c_str());
			block_content.push_back(chunk_byte);
		} // FOR chunk_byte

		//DEBUG_PRINTLN("Pushed chunk bytes to block content");
	} // FOR block
	  //DEBUG_PRINTLN("...Processed");


	if (block_content[0] != 0xF0 || block_content[1] != 0x01){
		Serial.println("Invalid block start, ignoring all data");
		return false;
	}
	else
	{
		//DEBUG_PRINTLN("Data seems correct");
		// and split them into chunks now, splitting on each f7
		std::vector<ByteVector> chunks;
		chunks.clear();
		ByteVector chunk_temp = {};
		for (byte by : block_content) {
			chunk_temp.push_back(by);
			if (by == 0xf7) {
				chunks.push_back(chunk_temp);
				chunk_temp = {};
			}
		}
		//DEBUG_PRINTLN("Split at F7");


		std::vector<CmdData> chunk_8bit = {};
		for (auto chunk : chunks) {
			byte this_cmd = chunk[4];
			byte this_sub_cmd = chunk[5];
			ByteVector data7bit = {};
			data7bit.assign(chunk.begin() + 6, chunk.end() - 1);

			int chunk_len = data7bit.size();
			//DEBUG_PRINT("Chunk_len:");
			//DEBUG_PRINTLN(chunk_len);
			int num_seq = int ((chunk_len + 7) / 8);
			ByteVector data8bit = {};

			for (int this_seq = 0; this_seq < num_seq; this_seq++) {
				int seq_len = min (8, chunk_len - (this_seq * 8));
				ByteVector seq = {};
				byte bit8 = data7bit[this_seq * 8];
				for (int ind = 0; ind < seq_len - 1; ind++) {
					byte dat = data7bit[this_seq * 8 + ind + 1];
					if ((bit8 & (1<<ind)) == (1<<ind)) {
						dat |= 0x80;
					}
					seq.push_back(dat);
				}
				for (auto by : seq) {
					data8bit.push_back(by);
				}
			}
			//DEBUG_PRINTLN("Converted to 8bit");
			struct CmdData curr_data = {this_cmd, this_sub_cmd, data8bit};
			chunk_8bit.push_back(curr_data);

			// now check for mult-chunk messages and collapse their data into a single message
			// multi-chunk messages are cmd/sub_cmd of 1,1 or 3,1

			message.clear();
			ByteVector concat_data;
			concat_data.clear();
			for (CmdData chunkData : chunk_8bit) {
				this_cmd     = chunkData.cmd;
				this_sub_cmd = chunkData.subcmd;
				ByteVector this_data = chunkData.data;
				if ((this_cmd == 1 || this_cmd == 3) && this_sub_cmd == 1) {
					//DEBUG_PRINTLN("Multi message");
					//found a multi-message
					int num_chunks = this_data[0];
					int this_chunk = this_data[1];
					ByteVector this_data_suffix;
					this_data_suffix.assign(this_data.begin() + 3, this_data.end());
					for (auto by : this_data_suffix) {
						concat_data.push_back(by);
					}
					// if at last chunk of multi-chunk
					if (this_chunk == num_chunks - 1) {
						//DEBUG_PRINTLN("Last chunk to process");
						curr_data = {this_cmd, this_sub_cmd, concat_data};
						message.push_back(curr_data);
						concat_data = {};
						curr_data = {};
					}
				}
				else {
					// copy old one
					message.push_back(chunkData);
				} // else
			} // For all in 8-bit vector
		} // for all chunks
	} // else (Block starts with F001)
	return true;
}


void SparkStreamReader::set_interpreter (ByteVector _msg) {
	msg = _msg;
	msg_pos = 0;
}

int SparkStreamReader::run_interpreter (byte _cmd, byte _sub_cmd) {
	if (_cmd == 0x01) {
		if (_sub_cmd == 0x01) {
			DEBUG_PRINTLN("Reading preset");
			read_preset();
		}
		else if (_sub_cmd == 0x04) {
			DEBUG_PRINTLN("Reading effect param");
			read_effect_parameter();
		}
		else if (_sub_cmd == 0x06) {
			DEBUG_PRINTLN("Reading effect");
			read_effect();
		}
		else if (_sub_cmd == 0x15) {
			DEBUG_PRINTLN("Reading effect on off");
			read_effect_onoff();
		}
		else if (_sub_cmd == 0x38) {
			read_hardware_preset();
		}
		else {
			DEBUG_PRINT(SparkHelper::intToHex(_cmd).c_str());
			DEBUG_PRINT(SparkHelper::intToHex(_sub_cmd).c_str());
			DEBUG_PRINTLN(" not handled");
			DEBUG_PRINTVECTOR(msg);DEBUG_PRINTLN();
		}
	}
	else if (_cmd == 0x02) {
		DEBUG_PRINTLN("Reading request from Amp");
		/*if (_sub_cmd == 0x38 || _sub_cmd == 0x10 ) {
			DEBUG_PRINTLN("Reading HW preset");
			read_hardware_preset();
		 }*/
	}
	else if (_cmd == 0x03) {
		if (_sub_cmd == 0x01) {
			DEBUG_PRINTLN("Reading preset");
			read_preset();
		}
		else if (_sub_cmd == 0x06) {
			DEBUG_PRINTLN("Reading effect");
			read_effect();
		}
		else if (_sub_cmd == 0x27) {
			DEBUG_PRINTLN("Storing HW preset");
			read_store_hardware_preset();
		}
		else if (_sub_cmd == 0x37) {
			DEBUG_PRINTLN("Reading effect param");
			read_effect_parameter();
		}
		else if (_sub_cmd == 0x38 || _sub_cmd == 0x10) {
			DEBUG_PRINTLN("Reading HW preset");
			read_hardware_preset();
		}
		else {
			DEBUG_PRINT(SparkHelper::intToHex(_cmd).c_str());
			DEBUG_PRINT(SparkHelper::intToHex(_sub_cmd).c_str());
			DEBUG_PRINTLN(" not handled");
			DEBUG_PRINTVECTOR(msg);DEBUG_PRINTLN();
		}
	}
	else if (_cmd == 0x04 || _cmd == 0x05 ) {
		acknowledgements.push_back(_sub_cmd);
		DEBUG_PRINTF("Acknowledgement for command %s\n", SparkHelper::intToHex(_sub_cmd).c_str());
	}
	else {
		// unprocessed command (likely the initial ones sent from the app

		std::string cmd_str = SparkHelper::intToHex(_cmd);
		std::string sub_cmd_str = SparkHelper::intToHex(_sub_cmd);
		DEBUG_PRINTF("Unprocessed: %s, %s - ", cmd_str.c_str(),
				sub_cmd_str.c_str());
		DEBUG_PRINTVECTOR(msg); DEBUG_PRINTLN();
	}
	return 1;
}

std::tuple<bool, byte, byte> SparkStreamReader::needsAck(ByteVector blk){

	if(blk.size() < 22){ // Block is too short, does not need acknowledgement
		return std::tuple<bool, byte, byte>(false, 0, 0);
	}
	byte direction[2] = { blk[4], blk[5] };
	last_message_num_ = blk[18];
	byte cmd = blk[20];
	byte sub_cmd = blk[21];

	byte msg_to_spark[2] = { '\x53', '\xfe' };
	int msg_to_spark_comp = memcmp(direction, msg_to_spark, sizeof(direction));
	if (msg_to_spark_comp == 0 && cmd == 0x01 && sub_cmd != 0x04) {
		// the app sent a message that needs a response
		return std::tuple<bool, byte, byte>(true, last_message_num_, sub_cmd);
	}
	return std::tuple<bool, byte, byte>(false, 0, 0);
}

byte SparkStreamReader::getLastAckAndEmpty(){
	byte lastAck = 0;
	if (acknowledgements.size() > 0){
		lastAck = acknowledgements.back();
		acknowledgements.clear();
	}
	return lastAck;
}

int SparkStreamReader::processBlock(ByteVector blk){

	int retValue = MSG_PROCESS_RES_INCOMPLETE;

	// Special behavior: When receiving messages from Spark APP, blocks might be split into two.
	// This will reassemble the block by appending to the previous one.
	if (!(blk[0] == '\x01' && blk[1] == '\xFE')){ // check if block needs to be appended to earlier block
		DEBUG_PRINTLN("Received block with no header start, appending to previous (if present)");
		if (response.size() > 0){
			ByteVector lastChunk = response.back();
			response.pop_back();
			for (auto by: blk){
				lastChunk.push_back(by);
			}
			blk = lastChunk;
		}
		//if current block starts with 01FE, check if last block was complete, otherwise remove from response
	} else {
		if (response.size() > 0) {
			bool removeLastChunk = false;
			ByteVector lastChunk = response.back();
			if (lastChunk.size() < 2) {
				DEBUG_PRINTLN("Last message too short and incomplete, ignoring");
				removeLastChunk = true;
			}

			byte lastByte = lastChunk.back();
			if (lastByte != 0xF7) {
				DEBUG_PRINTLN("Last chunk incomplete, ignoring");
				removeLastChunk = true;
			}
			if(removeLastChunk) {
				response.pop_back();
			}
		}
	}

	// Process:
	// Read a block
	// Check length of 01FE message (found in byte pos 6)
	// If length <6A => last block read
	// else
	// Check if the block ends with F7
	// If it ends with F7, check backwards for F001 and see what the message counter is
	// if counter == number_of_messages -1 => last block read
	// else read on.
	// Then pass everything to StreamReader
	response.push_back(blk);

	// Block with header needs to start with 01FE and to be longer than 22 bytes.
	// If sent without header, check for validity (starting with F001 and ending with F7) for immediate processing
	if ( (blk.size() < 22) || (blk[0] != 0x01 || blk[1] != 0xFE)){
		// Check if block came without header (01FE) and apply process special handling
		// Case implemented for Spark Mini/ (Go(?))
		if (!(isValidBlockWithoutHeader(blk))){
			DEBUG_PRINTLN("Block not ready for processing, skipping further processing.");
			return retValue;
		}
		setMessage(response);
		read_message(false);
		response.clear();
		retValue = MSG_PROCESS_RES_COMPLETE;

	} else {

		int blk_len = blk[6];
		byte direction[2] = { blk[4], blk[5] };
		byte seq = blk[18];
		byte cmd = blk[20];
		byte sub_cmd = blk[21];

		//Check if announced size matches real size, otherwise skip (and wait for more data);
		if(blk_len == blk.size()){
			byte msg_to_spark[2] = { '\x53', '\xfe' };
			int msg_to_spark_comp = memcmp(direction, msg_to_spark, sizeof(direction));

			// now we need to see if this is the last block

			// if the block length is less than the max size then
			// definitely last block
			// otherwise we check either directly for the message counter,
			// or we check if the block end with 0xF7. Then we can search for the last
			// 0xF001 occurrence and check the message count.

			if (msg_to_spark_comp == 0) {
				if (blk_len < 0xad) {
					msg_last_block = true;
				} else {
					// this is sent to Spark so will have a chunk header at top
					int num_chunks = blk[23];
					int this_chunk = blk[24];
					if ((this_chunk + 1) == num_chunks) {
						msg_last_block = true;
					}
				}
			}

			byte msg_from_spark[2] = { '\x41', '\xff' };
			int msg_from_spark_comp = memcmp(direction, msg_from_spark,
					sizeof(direction));
			if (msg_from_spark_comp == 0) {
				if (blk_len < 0x6a) {
					// if the message is smaller than the largest size possible for block, definitely the last block
					msg_last_block = true;
				}
				// if message ends with F7 we can check if that was the last block. Otherwise there will be more.
				else if (blk[blk.size() - 1] == '\xf7') {
					// this is from Spark so chunk header could be anywhere
					// so search from the end
					int pos = -1;
					for (int i = blk.size() - 2; i >= 0; i--) {
						if (blk[i] == '\xf0' && blk[i + 1] == '\x01') {
							pos = i;
							break;
						}
					}
					if (pos >= 0) {
						int num_chunks = blk[pos + 7];
						int this_chunk = blk[pos + 8];
						if ((this_chunk + 1) == num_chunks) {
							msg_last_block = true;
						} // if this_chunk+1 == num_chunks
					} //pos >= 0
					else {
						Serial.println("Chunk Header not found");
					}
				} //if message ends with F7
			} // Message is from Spark
			//Process data if the block just analyzed was the last
			if (msg_last_block) {
				msg_last_block = false;
				setMessage(response);
				read_message(true);
				response.clear();
				retValue = MSG_PROCESS_RES_COMPLETE;
			} // msg_last_block
		} // If length not equal to announced length

		// Message is not complete, has not been processed yet
		// if request was an initiating one from the app, return value for INITIAL message,
		// so SparkDataControl knows how to notify.
		// so notifications will be triggered

		if(cmd == 0x02){
			retValue = MSG_PROCESS_RES_REQUEST;
		}
	}
	return retValue;
}

void SparkStreamReader::interpret_data() {
	for (auto msgData : message) {
		int this_cmd = msgData.cmd;
		int this_sub_cmd = msgData.subcmd;
		ByteVector this_data = msgData.data;

		set_interpreter(this_data);
		run_interpreter(this_cmd, this_sub_cmd);
	}
	//message.clear();
}

std::vector<CmdData> SparkStreamReader::read_message(bool processHeader) {
	if(structure_data(processHeader)){
		interpret_data();
	}
	return message;
}

bool SparkStreamReader::isValidBlockWithoutHeader(ByteVector blk){

	if (blk.size() < 3) return false;

	byte blk_start[2] = { blk[0], blk[1]};
	byte valid_start[2] = { 0xF0, 0x01 };

	int valid_start_check = memcmp(blk_start, valid_start,
			sizeof(valid_start));

	if ( valid_start_check != 0) return false;

	byte blk_end = blk[blk.size()-1];
	if (blk_end != 0xF7) return false;

	return true;

}
