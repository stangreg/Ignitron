#include "SparkStreamReader.hh"


SparkStreamReader::SparkStreamReader(){
	message = {};
	unstructured_data = {};
	msg = {};
	msg_pos = 0;
}

void SparkStreamReader::setMessage(std::vector<ByteVector> msg){
	unstructured_data = msg;
	message.clear();
}

preset SparkStreamReader::getCurrentSetting(){
	return currentSetting;
}

int SparkStreamReader::getCurrentPresetNumber(){
	return currentPresetNumber;
}

boolean SparkStreamReader::isPresetUpdated(){
	return presetUpdated;
}

boolean SparkStreamReader::isPresetNumberUpdated(){
	return presetNumberUpdated;
}

void SparkStreamReader::resetPresetNumberUpdateFlag(){
	presetNumberUpdated = false;
}

void SparkStreamReader::resetPresetUpdateFlag(){
	presetUpdated = false;
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
	python = "{";
	raw = "";
	//dict={};
	indent = "";
}

void SparkStreamReader::end_str() {
	python += "}";
}

void SparkStreamReader::add_indent() {
	indent += "\t";
}

void SparkStreamReader::del_indent() {
	indent = indent.substr(1);
}

void SparkStreamReader::add_python(char* python_str) {
	python += indent + python_str + "\n";
}

void SparkStreamReader::add_str(char* a_title, std::string a_str, char* nature) {
	raw +=  a_str;
	raw += " ";
	char string_add[200] = "";
	sprintf(string_add, "%s%-20s: %s \n", indent.c_str(), a_title, a_str.c_str());
	text += string_add;
	if (nature != "python") {
		python += indent + "\"" + a_title + "\":\"" + a_str + "\",\n";
	}
}


void SparkStreamReader::add_int(char* a_title, int an_int, char* nature) {
	char string_add[200] ="";
	sprintf(string_add, "%d ", an_int);
	raw += string_add;
	sprintf(string_add, "%s%-20s: %d\n", indent.c_str(), a_title, an_int);
	text += string_add;
	if (nature != "python") {
		sprintf(string_add, "%s\"%-20s\": %d,\n", indent.c_str(), a_title, an_int);
		python += string_add;
	}
}



void SparkStreamReader::add_float(char* a_title, float a_float, char* nature) {
	char string_add[200] = "";
	sprintf(string_add, "%2.4f ", a_float);
	raw += string_add;
	sprintf(string_add, "%s%-20s: %2.4f\n", indent.c_str(), a_title, a_float);
	text += string_add;
	if (nature == "python") {
		sprintf(string_add, "%s%2.4f,\n", indent.c_str(), a_float);
		python += string_add;
	}
	else {
		sprintf(string_add, "%s\"%s\": %2.4f,\n", indent.c_str(), a_title, a_float);
		python += string_add;
	}
}

void SparkStreamReader::add_bool(char* a_title, boolean a_bool, char* nature) {
	char string_add[200] ="";
	sprintf(string_add, "%s ", a_bool ? "true" : "false");
	raw += string_add;
	sprintf(string_add, "%s%s: %-20s\n", indent.c_str(), a_title, a_bool ? "true" : "false");
	text += string_add;
	if (nature != "python") {
		sprintf(string_add, "%s\"%s\": %s,\n", indent.c_str(), a_title, a_bool ? "true" : "false");
		python += string_add;
	}
}


void SparkStreamReader::read_effect_parameter() {
	start_str();
	std::string effect = read_prefixed_string ();
	byte param = read_byte ();
	float val = read_float();
	add_str ("Effect", effect);
	add_int ("Parameter", param);
	add_float ("Value", val);
	end_str();
}

void SparkStreamReader::read_effect() {
	start_str();
	std::string effect1 = read_prefixed_string ();
	std::string effect2 = read_prefixed_string ();
	add_str ("OldEffect", effect1);
	add_str ("NewEffect", effect2);
	end_str();
}

void SparkStreamReader::read_hardware_preset() {
	start_str();
	read_byte ();
	byte preset_num = read_byte () + 1;
	add_int ("NewPreset", preset_num);
	end_str();
	Serial.print("Preset number received: ");
	Serial.println(preset_num);
	currentPresetNumber = preset_num;
	presetNumberUpdated = true;

}

void SparkStreamReader::read_store_hardware_preset() {
	start_str();
	read_byte ();
	byte preset_num = read_byte () + 1;
	add_int ("NewStoredPreset", preset_num);
	end_str();
}

void SparkStreamReader::read_effect_onoff() {
	start_str();
	std::string effect = read_prefixed_string ();
	boolean isOn = read_onoff ();
	add_str ("Effect", effect);
	add_bool ("IsOn", isOn);
	end_str();
}

void SparkStreamReader::read_preset() {
	start_str();
	read_byte();

	byte preset = read_byte();
	currentSetting.presetNumber = preset;
	add_int ("PresetNumber", preset);

	std::string uuid = read_string();
	currentSetting.uuid = uuid;
	add_str("UUID", uuid);

	std::string name = read_string();
	//Serial.printf("Read name: %s\n", name.c_str());
	currentSetting.name = name;
	add_str("Name", name);

	std::string version = read_string();
	currentSetting.version = version;
	add_str("Version", version);

	std::string descr = read_string();
	currentSetting.description = descr;
	add_str("Description", descr);

	std::string icon = read_string();
	currentSetting.icon = icon;
	add_str("Icon", icon);

	float bpm = read_float();
	currentSetting.bpm = bpm;
	add_float("BPM", bpm);

	int num_effects = read_byte() - 0x90;
	add_python("\"Pedals\": [");
	add_indent();
	currentSetting.pedals = {};
	for (int i = 0; i < 7; i++) { // Fixed to 7, but could maybe also be derived from num_effects?
		pedal currentPedal = {};
		std::string e_str = read_string();
		currentPedal.name = e_str;
		boolean e_onoff = read_onoff();
		currentPedal.isOn = e_onoff;
		add_python ("{");
		add_str("Name", e_str);
		add_bool("IsOn", e_onoff);
		int num_p = read_byte() - char(0x90);
		add_python("\"Parameters\":[");
		add_indent();
		currentPedal.parameters = {};
		for (int p = 0; p < num_p; p++) {
			parameter currentParameter = {};
			byte num = read_byte();
			byte spec = read_byte();
			float val = read_float();
			currentParameter.number = num;
			currentParameter.special = spec;
			currentParameter.value = val;
			add_int("Parameter", num, "python");
			add_str("Special", SparkHelper::intToHex(spec), "python");
			//add_str("Special", spec, "python");
			add_float("Value", val, "python");

			currentPedal.parameters.push_back(currentParameter);
		}

		add_python("],");
		del_indent();
		add_python("},");
		currentSetting.pedals.push_back(currentPedal);
	}
	add_python("],");
	del_indent();
	byte filler = read_byte();
	currentSetting.filler = filler;
	add_str("Filler", SparkHelper::intToHex(filler));
	end_str();
	currentSetting.text = text;
	currentSetting.raw = raw;
	currentSetting.python = python;
	currentSetting.isEmpty = false;
	presetUpdated = true;
}

boolean SparkStreamReader::structure_data() {

	ByteVector block_content;
	block_content.clear();
	message.clear();

	//Serial.println("Structuring data..:");
	//SparkHelper::printDataAsHexString(unstructured_data);
	for (auto block : unstructured_data) {
		//Serial.println("Processing block");
		//SparkHelper::printByteVector(block);
		//Serial.println();
		//Serial.printf("Current heap size: %d\n", ESP.getFreeHeap());
		int block_length = block[6];
		int data_size = block.size();
		//Serial.printf("Read block size %d, %d\n", block_length, data_size);
		if ( data_size != block_length) {
			Serial.printf("Data is of size %d and reports %d\n", data_size, block_length );
			Serial.println("Corrupt block:");
			for (auto by : block){
				Serial.print(SparkHelper::intToHex(by).c_str());
			}
			Serial.println();
		}
		//Serial.println("Sizes match");
		ByteVector chunk;
		chunk.assign(block.begin() + 16, block.end());
		//Serial.printf("Assigned block of size %d", chunk.size());
		for (auto chunk_byte : chunk) {
			//Serial.print(SparkHelper::intToHex(chunk_byte).c_str());
			block_content.push_back(chunk_byte);
		} // FOR chunk_byte
		//Serial.println("Pushed chunk bytes to block content");
	} // FOR block
	//Serial.println("...Processed");


	if (block_content[0] != 0xF0 || block_content[1] != 0x01){
		Serial.println("Invalid block start, ignoring all data");
		return false;
	}
	else
	{
		//Serial.println("Data seems correct");
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
		//Serial.println("Split at F7");


		std::vector<cmd_data> chunk_8bit = {};
		for (auto chunk : chunks) {
			byte this_cmd = chunk[4];
			byte this_sub_cmd = chunk[5];
			ByteVector data7bit = {};
			data7bit.assign(chunk.begin() + 6, chunk.end() - 1);

			int chunk_len = data7bit.size();
			//Serial.print("Chunk_len:");
			//Serial.println(chunk_len);
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
			//Serial.println("Converted to 8bit");
			struct cmd_data curr_data = {this_cmd, this_sub_cmd, data8bit};
			chunk_8bit.push_back(curr_data);

			// now check for mult-chunk messages and collapse their data into a single message
			// multi-chunk messages are cmd/sub_cmd of 1,1 or 3,1

			message.clear();
			ByteVector concat_data;
			concat_data.clear();
			for (cmd_data chunk : chunk_8bit) {
				this_cmd     = chunk.cmd;
				this_sub_cmd = chunk.subcmd;
				ByteVector this_data = chunk.data;
				if ((this_cmd == 1 || this_cmd == 3) && this_sub_cmd == 1) {
					//Serial.println("Multi message");
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
						//Serial.println("Last chunk to process");
						curr_data = {this_cmd, this_sub_cmd, concat_data};
						message.push_back(curr_data);
						concat_data = {};
						curr_data = {};
					}
				}
				else {
					// copy old one
					//Serial.print("Copying old one");
					message.push_back(chunk);
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
			Serial.println("Reading preset");
			read_preset();
		}
		else if (_sub_cmd == 0x04) {
			Serial.println("Reading effect param");
			read_effect_parameter();
		}
		else if (_sub_cmd == 0x06) {
			Serial.println("Reading effect");
			read_effect();
		}
		else if (_sub_cmd == 0x15) {
			Serial.println("Reading effect on off");
			read_effect_onoff();
		}
		else if (_sub_cmd == 0x38) {
			read_hardware_preset();
		}
		else {
			Serial.print(SparkHelper::intToHex(_cmd).c_str());
			Serial.print(SparkHelper::intToHex(_sub_cmd).c_str());
			Serial.println(" not handled");
		}
	}
	else if (_cmd == 0x03) {
		if (_sub_cmd == 0x01) {
			Serial.println("Reading preset");
			read_preset();
		}
		else if (_sub_cmd == 0x06) {
			Serial.println("Reading effect");
			read_effect();
		}
		else if (_sub_cmd == 0x27) {
			Serial.println("Storing HW preset");
			read_store_hardware_preset();
		}
		else if (_sub_cmd == 0x37) {
			Serial.println("Reading effect param");
			read_effect_parameter();
		}
		else if (_sub_cmd == 0x38 || _sub_cmd == 0x10) {
			Serial.println("Reading HW preset");
			read_hardware_preset();
		}
		else {
			Serial.print(SparkHelper::intToHex(_cmd).c_str());
			Serial.print(SparkHelper::intToHex(_sub_cmd).c_str());
			Serial.println(" not handled");
		}
	}
	else if (_cmd == 0x04) {
		acknowledgements.push_back(_sub_cmd);
		//Serial.printf("Acknowledgement for command %s\n", SparkHelper::intToHex(_sub_cmd).c_str());
	}
	else {
		Serial.println ("Unprocessed");
	}
	//Serial.println("Analyzed response");
	//Serial.println(python.c_str());
	return 1;
}

std::tuple<bool, byte, byte> SparkStreamReader::needsAck(ByteVector blk){

	if(blk.size() < 22){ // Block is too short, does not need acknowledgement
		return std::tuple<bool, byte, byte>(false, 0, 0);
	}
	byte direction[2] = { blk[4], blk[5] };
	byte seq = blk[18];
	byte cmd = blk[20];
	byte sub_cmd = blk[21];
	//Check if this is needed at all!

	byte msg_to_spark[2] = { '\x53', '\xfe' };
	int msg_to_spark_comp = memcmp(direction, msg_to_spark, sizeof(direction));
	if (msg_to_spark_comp == 0 && cmd == 0x01 && sub_cmd != 0x04) {
		// the app sent a message that needs a response
		return std::tuple<bool, byte, byte>(true, seq, cmd);
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

void SparkStreamReader::processBlock(ByteVector blk){

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
	//SparkHelper::printByteVector(blk);
	//Serial.println();

	int blk_len = blk[6];
	byte direction[2] = { blk[4], blk[5] };
	byte seq = blk[18];
	byte cmd = blk[20];
	byte sub_cmd = blk[21];

	//Check if this is needed at all!

	byte msg_to_spark[2] = { '\x53', '\xfe' };
	int msg_to_spark_comp = memcmp(direction, msg_to_spark, sizeof(direction));

	// now we need to see if this is the last block

	// if the block length is less than the max size then
	// definitely last block
	// could be a full block and still last one
	// but to be full surely means it is a multi-block as
	// other messages are always small
	// so need to check the chunk counts - in different places
	// depending on whether

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
			//Serial.println("Last message, shorter");
			// if the message is smaller than the largest size possible for block, definitely the last block
			msg_last_block = true;
		}
		// if message ends with F7 we can check if that was the last block. Otherwise there will be more.
		else if (blk[blk.size() - 1] == '\xf7') {
			// this is from Spark so chunk header could be anywhere
			// so search from the end
			int pos = -1;
			//Serial.println("Searching backwards for chunk header");
			for (int i = blk.size() - 2; i >= 0; i--) {
				if (blk[i] == '\xf0' && blk[i + 1] == '\x01') {
					Serial.println("Found F001");
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
		setMessage(response);
		//Serial.println("Reading message");
		read_message();
		msg_last_block = false;
		response.clear();
	} // msg_last_block

}

void SparkStreamReader::interpret_data() {
	for (auto msg : message) {
		int this_cmd = msg.cmd;
		int this_sub_cmd = msg.subcmd;
		ByteVector this_data = msg.data;

		set_interpreter(this_data);
		run_interpreter(this_cmd, this_sub_cmd);
	}
	message.clear();
}

std::vector<cmd_data> SparkStreamReader::read_message() {
	if(structure_data()){
		interpret_data();
	}
	return message;
}
