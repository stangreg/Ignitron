/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkMessage.h"

SparkMessage::SparkMessage() : data{}, split_data8{}, split_data7{}, cmd(0), sub_cmd(0) {
}

void SparkMessage::start_message(byte _cmd, byte _sub_cmd) {
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

    for (int this_chunk = 0; this_chunk < num_chunks; this_chunk++) {
        int chunk_len = min(MAX_CHUNK_SIZE,
                            data_len - (this_chunk * MAX_CHUNK_SIZE));
        ByteVector data8;
        if (num_chunks > 1) {
            // we need the chunk sub-header
            data8.push_back(num_chunks);
            data8.push_back(this_chunk);
            data8.push_back(chunk_len);
        } else {
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
    for (auto chunk : split_data8) {

        int chunk_len = chunk.size();
        int num_seq = int((chunk_len + 6) / 7);
        ByteVector bytes7 = {};

        for (int this_seq = 0; this_seq < num_seq; this_seq++) {
            int seq_len = min(7, chunk_len - (this_seq * 7));
            byte bit8 = 0;
            ByteVector seq = {};
            for (int ind = 0; ind < seq_len; ind++) {
                byte dat = chunk[this_seq * 7 + ind];
                if ((dat & 0x80) == 0x80) {
                    bit8 |= (1 << ind);
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

    ByteVector chunk_header = {0xF0, 0x01};
    byte msg_num;
    if (msg_number == 0) {
        msg_num = 0x01;
    } else {
        msg_num = (byte)msg_number;
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

vector<CmdData> SparkMessage::buildMessage(int dir, byte msg_num) {

    vector<CmdData> final_message = {};

    int total_bytes_to_process = SparkHelper::dataVectorNumOfBytes(all_chunks);
    int block_prefix_size = with_header ? 16 : 0;

    // Maximum block size depending on direction.
    int MAX_BLOCK_SIZE = (dir == DIR_TO_SPARK) ? maxBlockSizeToSpark() : maxBlockSizeFromSpark();

    // now we can create the final message with the message header and the chunk header
    ByteVector block_header = {0x01, 0xFE, 0x00, 0x00};
    ByteVector block_header_direction = (dir == DIR_TO_SPARK) ? msg_to_spark : msg_from_spark;
    ByteVector block_filler = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00};

    // read all chunks into one big chunk for splitting
    std::deque<byte> data_to_split = {};
    for (auto chunk : all_chunks) {
        data_to_split.insert(data_to_split.end(), chunk.begin(), chunk.end());
    }
    all_chunks.clear();

    // create block
    ByteVector current_block = {};
    int data_size;

    // create blocks from all data with headers.
    while (data_to_split.size() > 0) {
        DEBUG_PRINTVECTOR(current_block);
        DEBUG_PRINTLN();
        data_size = (int)data_to_split.size();
        if (with_header) {

            int block_size = min(MAX_BLOCK_SIZE,
                                 data_size + block_prefix_size);
            current_block = block_header;
            current_block.insert(current_block.end(),
                                 block_header_direction.begin(),
                                 block_header_direction.end());
            current_block.push_back(block_size);
            current_block.insert(current_block.end(), block_filler.begin(),
                                 block_filler.end());
        }
        int remaining_size = MAX_BLOCK_SIZE - current_block.size();
        int bytes_to_insert = min(remaining_size, data_size);
        std::deque<byte>::iterator num_bytes = data_to_split.begin() + bytes_to_insert;
        current_block.insert(current_block.end(), data_to_split.begin(), num_bytes);
        data_to_split.erase(data_to_split.begin(), num_bytes);
        DEBUG_PRINTVECTOR(current_block);
        DEBUG_PRINTLN();
        CmdData data_item;
        data_item.data = current_block;
        data_item.cmd = cmd;
        data_item.subcmd = sub_cmd;
        data_item.detail = cmd_detail;
        data_item.msg_num = msg_num;
        final_message.push_back(data_item);
        current_block.clear();
    }

    DEBUG_PRINTLN("COMPLETE MESSAGE: ");
    for (auto block : final_message) {
        DEBUG_PRINTVECTOR(block.data);
        DEBUG_PRINTLN();
    }

    return final_message;
}

vector<CmdData> SparkMessage::end_message(int dir, byte msg_number) {

    DEBUG_PRINT("MESSAGE NUMBER: ");
    DEBUG_PRINT(msg_number);
    DEBUG_PRINTLN();

    // First split all data into chunks of defined maximum size
    splitDataToChunks(dir);
    // now we can convert this to 7-bit data format with the 8-bits byte at the front
    convertDataTo7Bit();
    // build F001 chunks
    buildChunkData(msg_number);
    // build final message (with 01FE header if required)
    return buildMessage(dir, msg_number);
}

void SparkMessage::add_bytes(const ByteVector &bytes_8) {
    for (byte by : bytes_8) {
        data.push_back(by);
    }
}

void SparkMessage::add_byte(byte by) {
    data.push_back(by);
}

void SparkMessage::add_prefixed_string(const string &pack_str, int length_override) {
    int str_length = length_override;
    if (str_length == 0)
        str_length = pack_str.size();
    ByteVector byte_pack;
    byte_pack.push_back((byte)str_length);
    byte_pack.push_back((byte)(str_length + 0xa0));
    copy(pack_str.begin(), pack_str.end(), back_inserter<ByteVector>(byte_pack));
    add_bytes(byte_pack);
}

void SparkMessage::add_string(const string &pack_str) {
    int str_length = pack_str.size();
    ByteVector byte_pack;
    byte_pack.push_back((byte)(str_length + 0xa0));
    copy(pack_str.begin(), pack_str.end(), back_inserter<ByteVector>(byte_pack));
    add_bytes(byte_pack);
}

void SparkMessage::add_long_string(const string &pack_str) {
    int str_length = pack_str.size();
    ByteVector byte_pack;
    byte_pack.push_back((0xD9));
    byte_pack.push_back((byte)str_length);
    copy(pack_str.begin(), pack_str.end(), back_inserter<ByteVector>(byte_pack));
    add_bytes(byte_pack);
}

void SparkMessage::add_float(float flt) {

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
    for (int i = 3; i >= 0; i--) {
        byte_pack.push_back(u.temp_array[i]);
    }
    // DEBUG_PRINTF("Converting float %f to HEX: ", u.float_variable);
    // for (auto byte : byte_pack) {
    //	DEBUG_PRINTF("%s", SparkHelper::intToHex(byte).c_str());
    // }
    // DEBUG_PRINTLN();
    add_bytes(byte_pack);
}

void SparkMessage::add_onoff(boolean enable) {
    byte b;
    if (enable == true) {
        b = 0xC3;
    } else {
        b = 0xC2;
    }
    add_byte(b);
}

void SparkMessage::add_int16(unsigned int number) {
    ByteVector bytepack;
    // Converting number to 16 bit INT
    bytepack.push_back(0xCD);
    bytepack.push_back(number >> 8);
    bytepack.push_back(number & 0xFF);

    add_bytes(bytepack);
}

vector<CmdData> SparkMessage::get_current_preset_num(byte msg_num) {
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

    start_message(cmd, sub_cmd);
    return end_message(DIR_TO_SPARK, msg_num);
}

vector<CmdData> SparkMessage::get_current_preset(byte msg_num, int hw_preset) {

    cmd = 0x02;
    sub_cmd = 0x01;
    DEBUG_PRINTF("Getting preset with message number %s\n", SparkHelper::intToHex(msg_num).c_str());
    start_message(cmd, sub_cmd);
    if (hw_preset == -1) {
        add_byte(0x10);
        add_byte(0x00);
    } else {
        add_byte(0x00);
        add_byte((byte)hw_preset - 1);
    }

    // add trailing 30 bytes of 00
    /* for (int i = 0; i < 30; i++) {
         add_byte(0x00);
     }*/

    return end_message(DIR_TO_SPARK, msg_num);
}

vector<CmdData> SparkMessage::change_effect_parameter(byte msg_num, const string &pedal, int param, float val) {
    cmd = 0x01;
    sub_cmd = 0x04;

    start_message(cmd, sub_cmd);
    add_prefixed_string(pedal);
    add_byte((byte)param);
    add_float(val);
    return end_message(DIR_TO_SPARK, msg_num);
}

vector<CmdData> SparkMessage::change_effect(byte msg_num, const string &pedal1, const string &pedal2) {
    cmd = 0x01;
    sub_cmd = 0x06;

    start_message(cmd, sub_cmd);
    add_prefixed_string(pedal1);
    add_prefixed_string(pedal2);
    return end_message(DIR_TO_SPARK, msg_num);
}

vector<CmdData> SparkMessage::change_hardware_preset(byte msg_num, int preset_num) {
    cmd = 0x01;
    sub_cmd = 0x38;

    start_message(cmd, sub_cmd);
    add_byte((byte)0);
    add_byte((byte)preset_num - 1);
    return end_message(DIR_TO_SPARK, msg_num);
}

vector<CmdData> SparkMessage::get_amp_name(byte msg_num) {

    cmd = 0x02;
    sub_cmd = 0x11;

    start_message(cmd, sub_cmd);
    return end_message(DIR_TO_SPARK, msg_num);
}

vector<CmdData> SparkMessage::get_serial_number(byte msg_num) {
    cmd = 0x02;
    sub_cmd = 0x23;

    start_message(cmd, sub_cmd);
    return end_message(DIR_TO_SPARK, msg_num);
}

vector<CmdData> SparkMessage::get_hw_checksums(byte msg_num) {
    cmd = 0x02;
    sub_cmd = 0x2a;

    start_message(cmd, sub_cmd);
    return end_message(DIR_TO_SPARK, msg_num);
}

vector<CmdData> SparkMessage::get_firmware_version(byte msg_num) {
    cmd = 0x02;
    sub_cmd = 0x2f;

    start_message(cmd, sub_cmd);
    return end_message(DIR_TO_SPARK, msg_num);
}

vector<CmdData> SparkMessage::turn_effect_onoff(byte msg_num, const string &pedal, boolean enable) {
    cmd = 0x01;
    sub_cmd = 0x15;

    start_message(cmd, sub_cmd);
    add_prefixed_string(pedal);
    add_onoff(enable);
    add_byte(0x00);
    return end_message(DIR_TO_SPARK, msg_num);
}

vector<CmdData> SparkMessage::send_serial_number(byte msg_number) {
    cmd = 0x03;
    sub_cmd = 0x23;

    start_message(cmd, sub_cmd);
    string serial_num = "S999C999B999";
    // Spark App seems to send the F7 byte as part of the string in addition to the final F7 byte,
    // so we need to have a flexible add_prefixed_string method to increase lenght information by one
    add_prefixed_string(serial_num, serial_num.length() + 1);
    add_byte(0xF7);
    return end_message(DIR_FROM_SPARK, msg_number);
}

vector<CmdData> SparkMessage::send_firmware_version(byte msg_number) {
    cmd = 0x03;
    sub_cmd = 0x2F;

    // TODO take version string as input
    start_message(cmd, sub_cmd);

    add_byte(0xCE);
    // Version string 1.7.5.182 (old: 1.6.5.160 (160-128))
    add_byte((byte)1);
    add_byte((byte)10);
    add_byte((byte)8);
    add_byte((byte)25);
    return end_message(DIR_FROM_SPARK, msg_number);
}

vector<CmdData> SparkMessage::send_hw_checksums(byte msg_number) {
    cmd = 0x03;
    sub_cmd = 0x2A;

    start_message(cmd, sub_cmd);

    // 0D 147D 4C07 5A58
    /*
    add_byte(0x94);
    add_byte(0x7D);
    add_byte(0xCC);
    add_byte(0x87);
    add_byte(0x5A);
    add_byte(0x58);
     */

    // Spark 40: 94 CC 8e 75 67 2a

    // Spark GO: f001 0779 032a   07 14 4c 1e 75 67 2a       f7

    add_byte(0x94);
    add_byte(0xCC);
    add_byte(0x9e);
    add_byte(0x75);
    add_byte(0x67);
    add_byte(0x2a);

    return end_message(DIR_FROM_SPARK, msg_number);
}

vector<CmdData> SparkMessage::send_hw_preset_number(byte msg_number) {
    cmd = 0x03;
    sub_cmd = 0x10;

    start_message(cmd, sub_cmd);
    add_byte(0x00);
    add_byte(0x00);
    add_byte(0x00);
    return end_message(DIR_FROM_SPARK, msg_number);
}

vector<CmdData> SparkMessage::create_preset(const Preset &preset_data,
                                            int direction, byte msg_num) {

    if (direction == DIR_TO_SPARK) {
        cmd = 0x01;
    } else {
        cmd = 0x03;
    }
    sub_cmd = 0x01;

    start_message(cmd, sub_cmd);
    if (direction == DIR_TO_SPARK) {
        add_byte(0x00);
        add_byte(0x7F);
    } else {
        add_byte(0x01);
        add_byte(0x00);
    }
    add_long_string(preset_data.uuid);
    string name = preset_data.name;
    if (name.size() > 31) {
        add_long_string(name);
    } else {
        add_string(name);
    }
    add_string(preset_data.version);
    string descr = preset_data.description;
    if (descr.size() > 31) {
        add_long_string(descr);
    } else {
        add_string(descr);
    }
    add_string(preset_data.icon);
    add_float(preset_data.bpm);
    add_byte((byte)(0x90 + 7)); // 7 pedals
    for (int i = 0; i < 7; i++) {
        Pedal curr_pedal = preset_data.pedals[i];
        add_string(curr_pedal.name);
        add_onoff(curr_pedal.isOn);
        vector<Parameter> curr_pedal_params = curr_pedal.parameters;
        int num_p = curr_pedal_params.size();
        add_byte((byte)(num_p + 0x90));
        for (int p = 0; p < num_p; p++) {
            add_byte((byte)p);
            add_byte((byte)'\x91');
            add_float(curr_pedal_params[p].value);
        }
    }
    byte checksum = calculate_checksum(data) ^ 0x7F;
    add_byte(checksum);
    return end_message(direction, msg_num);
}

// This prepares a message to send an acknowledgement
vector<CmdData> SparkMessage::send_ack(byte msg_num, byte sub_cmd_,
                                       int dir) {

    byte cmd_ = 0x04;

    start_message(cmd_, sub_cmd_);
    if (sub_cmd_ == 0x70) {
        add_byte(0x00);
        add_byte(0x00);
    }
    return end_message(dir, msg_num);
}

byte SparkMessage::calculate_checksum(const ByteVector &chunk) {
    byte current_sum = 0x00;
    for (byte by : chunk) {
        current_sum ^= by;
    }
    return (byte)current_sum;
}

vector<CmdData> SparkMessage::send_response_71(byte msg_number) {

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

vector<CmdData> SparkMessage::send_response_72(byte msg_number) {

    // f0 01 02 53 03 72 01 43 1e 00 0f f7
    cmd = 0x03;
    sub_cmd = 0x72;

    start_message(cmd, sub_cmd);
    add_byte(0xC3);
    add_byte(0x1E);
    add_byte(0x00);
    add_byte(0x0F);
    return end_message(DIR_FROM_SPARK, msg_number);
}

vector<CmdData> SparkMessage::spark_looper_command(byte msg_number, byte command) {

    cmd = 0x01;
    sub_cmd = 0x75;
    cmd_detail = command;

    start_message(cmd, sub_cmd);
    add_byte(command);
    return end_message(DIR_TO_SPARK, msg_number);
}

vector<CmdData> SparkMessage::spark_config_after_intro(byte msg_number, byte command) {

    cmd = 0x02;
    sub_cmd = command;

    start_message(cmd, sub_cmd);
    if (command == 0x33) {
        add_byte(0x00);
        add_byte(0x0a);
    }
    return end_message(DIR_TO_SPARK, msg_number);
}

vector<CmdData> SparkMessage::update_looper_settings(byte msg_number, const LooperSetting *setting) {

    cmd = 0x01;
    sub_cmd = 0x76;
    vector<CmdData> message;

    if (setting == nullptr) {
        Serial.println("Looper not set, ignoring");
        return message;
    }

    DEBUG_PRINTF("LPSetting: BPM: %d, Count: %02x, Bars: %d, Free?: %d, Click: %d, Max duration: %d\n ", setting->bpm, setting->count, setting->bars, setting->free_indicator, setting->click, setting->max_duration);
    start_message(cmd, sub_cmd);
    if (setting->bpm >= 128) {
        add_byte(0xCC);
    }
    add_byte(setting->bpm);
    add_byte(setting->count);
    add_byte(setting->bars);
    add_onoff(setting->free_indicator);
    add_onoff(setting->click);
    add_onoff(setting->unknown_onoff);
    add_int16(setting->max_duration);

    message = end_message(DIR_TO_SPARK, msg_number);
    return message;
}

vector<CmdData> SparkMessage::get_looper_status(byte msg_number) {
    cmd = 0x02;
    sub_cmd = 0x78;
    start_message(cmd, sub_cmd);
    return end_message(DIR_TO_SPARK, msg_number);
}

vector<CmdData> SparkMessage::get_looper_config(byte msg_number) {
    cmd = 0x02;
    sub_cmd = 0x76;
    start_message(cmd, sub_cmd);
    return end_message(DIR_TO_SPARK, msg_number);
}

vector<CmdData> SparkMessage::get_looper_record_status(byte msg_number) {
    cmd = 0x02;
    sub_cmd = 0x75;
    start_message(cmd, sub_cmd);
    return end_message(DIR_TO_SPARK, msg_number);
}