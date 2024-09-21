/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkStreamReader.h"

SparkStreamReader::SparkStreamReader() : message{}, unstructured_data{}, msg{}, msg_pos(0) {
}

string SparkStreamReader::getJson() {
    return json;
}

void SparkStreamReader::setMessage(const vector<ByteVector> &msg_) {
    unstructured_data = msg_;
    message.clear();
}

void SparkStreamReader::resetPresetNumberUpdateFlag() {
    isPresetNumberUpdated_ = false;
}

void SparkStreamReader::resetPresetUpdateFlag() {
    isPresetUpdated_ = false;
}

void SparkStreamReader::resetLooperSettingUpdateFlag() {
    isLooperSettingUpdated_ = false;
}

void SparkStreamReader::resetLastMessageType() {
    last_message_type_ = 0;
}

byte SparkStreamReader::read_byte() {
    byte a_byte;
    a_byte = msg[msg_pos];
    msg_pos += 1;
    return a_byte;
}

string SparkStreamReader::read_prefixed_string() {

    (void)read_byte();
    // offset removed from string length byte to get real length
    int real_str_len = read_byte() - 0xa0;
    string a_str = "";
    // reading string
    for (int i = 0; i < real_str_len; i++) {
        a_str += char(read_byte());
    }
    return a_str;
}

string SparkStreamReader::read_string() {
    byte a_byte = read_byte();
    int str_len;
    if (a_byte == 0xd9) {
        a_byte = read_byte();
        str_len = a_byte;
    } else if (a_byte >= 0xa0) {
        str_len = a_byte - 0xa0;
    } else {
        a_byte = read_byte();
        str_len = a_byte - 0xa0;
    }

    string a_str = "";
    for (int i = 0; i < str_len; i++) {
        a_str += char(read_byte());
    }
    return a_str;
}

// floats are special - bit 7 is actually stored in the format byte and not in the data
float SparkStreamReader::read_float() {
    byte prefix = read_byte(); // should be ca

    // using union struct to share memory for easy transformation of bytes to float
    union {
        float f;
        unsigned long ul;
    } u;

    byte a, b, c, d;
    a = read_byte();
    b = read_byte();
    c = read_byte();
    d = read_byte();
    u.ul = (a << 24) | (b << 16) | (c << 8) | d;
    float val = u.f;
    return val;
}

bool SparkStreamReader::read_onoff() {
    byte a_byte = read_byte();
    switch (a_byte) {
    case 0xC3:
        return true;
        break;
    case 0xC2:
        return false;
        break;
    default:
        DEBUG_PRINTLN("Incorrect on/off state");
        return "?";
        break;
    }
}

void SparkStreamReader::start_str() {
    text = "";
    json = "{";
    raw = "";
    // dict={};
    indent = "";
}

void SparkStreamReader::end_str() {
    json += "}";
}

void SparkStreamReader::add_indent() {
    indent += "\t";
}

void SparkStreamReader::del_indent() {
    indent.resize(indent.size() - 1);
}

void SparkStreamReader::add_separator() {
    json += ", ";
}

void SparkStreamReader::add_newline() {
    json += "\n";
}

void SparkStreamReader::add_python(string python_str) {
    json += indent + python_str; // + "\n";
}

void SparkStreamReader::add_str(const string &a_title, const string &a_str, string nature) {
    raw += a_str;
    raw += " ";
    char string_add[200] = "";
    int size = sizeof string_add;
    snprintf(string_add, size, "%s%-20s: %s \n", indent.c_str(), a_title.c_str(), a_str.c_str());
    text += string_add;
    if (nature != "python") {
        json += indent + "\"" + a_title + "\": \"" + a_str + "\"";
    }
}

void SparkStreamReader::add_int(const string &a_title, int an_int, string nature) {
    char string_add[100] = "";
    int size = sizeof string_add;
    snprintf(string_add, size, "%d ", an_int);
    raw += string_add;
    snprintf(string_add, size, "%s%-20s: %d\n", indent.c_str(), a_title.c_str(), an_int);
    text += string_add;
    if (nature != "python") {
        snprintf(string_add, size, "%s\"%s\": %d", indent.c_str(), a_title.c_str(), an_int);
        json += string_add;
    }
}

void SparkStreamReader::add_float(const string &a_title, float a_float, string nature) {

    char string_add[100] = "";
    int size = sizeof string_add;
    snprintf(string_add, size, "%2.4f ", a_float);
    raw += string_add;
    snprintf(string_add, size, "%s%-20s: %2.4f\n", indent.c_str(), a_title.c_str(), a_float);
    text += string_add;
    if (nature != "python") {
        snprintf(string_add, size, "%s%2.4f", indent.c_str(), a_float);
        json += string_add;
    } else {
        snprintf(string_add, size, "%s\"%s\": %2.4f", indent.c_str(), a_title.c_str(), a_float);
        json += string_add;
    }
}

void SparkStreamReader::add_float_pure(float a_float, string nature) {
    char string_add[100] = "";
    int size = sizeof string_add;
    snprintf(string_add, size, "%2.4f ", a_float);
    raw += string_add;
    snprintf(string_add, size, "%2.4f ", a_float);
    text += string_add;
    snprintf(string_add, size, "%2.4f", a_float);
    json += string_add;
}

void SparkStreamReader::add_bool(const string &a_title, boolean a_bool, string nature) {
    char string_add[100] = "";
    int size = sizeof string_add;
    snprintf(string_add, size, "%s ", a_bool ? "true" : "false");
    raw += string_add;
    snprintf(string_add, size, "%s%s: %-20s\n", indent.c_str(), a_title.c_str(), a_bool ? "true" : "false");
    text += string_add;
    if (nature != "python") {
        snprintf(string_add, size, "%s\"%s\": %s", indent.c_str(), a_title.c_str(), a_bool ? "true" : "false");
        json += string_add;
    }
}

void SparkStreamReader::read_effect_parameter() {
    // Read object
    string effect = read_prefixed_string();
    byte param = read_byte();
    float val = read_float();

    // Build string representations
    start_str();
    add_str("Effect", effect);
    add_separator();
    add_int("Parameter", param);
    add_separator();
    add_float("Value", val);
    end_str();

    // Set values
    last_message_type_ = MSG_TYPE_FX_PARAM;
}

void SparkStreamReader::read_effect() {
    // Read object
    string effect1 = read_prefixed_string();
    string effect2 = read_prefixed_string();

    // Build string representations
    start_str();
    add_str("OldEffect", effect1);
    add_separator();
    add_newline();
    add_str("NewEffect", effect2);
    end_str();

    // Set values
    last_message_type_ = MSG_TYPE_FX_CHANGE;
}

void SparkStreamReader::read_hardware_preset() {
    // Read object
    read_byte();
    byte preset_num = read_byte() + 1;

    // Build string representations
    start_str();
    add_int("New HW Preset", preset_num);
    end_str();

    // Set values
    currentPresetNumber_ = preset_num;
    isPresetNumberUpdated_ = true;
    last_message_type_ = MSG_TYPE_HWPRESET;
}

void SparkStreamReader::read_store_hardware_preset() {
    // Read object
    read_byte();
    byte preset_num = read_byte() + 1;

    // Build string representations
    start_str();
    add_int("NewStoredPreset", preset_num);
    end_str();

    // Set values
    last_message_type_ = MSG_TYPE_HWPRESET;
}

void SparkStreamReader::read_effect_onoff() {
    // Read object
    string effect = read_prefixed_string();
    boolean isOn = read_onoff();

    // Build string representations
    start_str();
    add_str("Effect", effect);
    add_separator();
    add_bool("IsOn", isOn);
    end_str();

    // Set values
    last_message_type_ = MSG_TYPE_FX_ONOFF;
}

void SparkStreamReader::read_preset() {
    // Read object (Preset main data)
    // DEBUG_PRINTF("Free memory before reading preset: %d\n", xPortGetFreeHeapSize());

    DEBUG_PRINTLN("Parsing message:");
    DEBUG_PRINTVECTOR(msg);
    DEBUG_PRINTLN();

    read_byte();
    byte preset = read_byte();
    // DEBUG_PRINTF("Read PresetNumber: %d\n", preset);
    currentPreset_.presetNumber = preset;
    string uuid = read_string();
    // DEBUG_PRINTF("Read UUID: %s\n", uuid.c_str());
    currentPreset_.uuid = uuid;
    string name = read_string();
    // DEBUG_PRINTF("Read Name: %s\n", name.c_str());
    currentPreset_.name = name;
    string version = read_string();
    // DEBUG_PRINTF("Read Version: %s\n", version.c_str());
    currentPreset_.version = version;
    string descr = read_string();
    // DEBUG_PRINTF("Read Description: %s\n", descr.c_str());
    currentPreset_.description = descr;
    string icon = read_string();
    // DEBUG_PRINTF("Read Icon: %s\n", icon.c_str());
    currentPreset_.icon = icon;
    float bpm = read_float();
    // DEBUG_PRINTF("Read BPM: %f\n", bpm);
    currentPreset_.bpm = bpm;
    // Build string representations
    // DEBUG_PRINTF("Free memory before adds: %d\n", xPortGetFreeHeapSize());
    start_str();
    add_int("PresetNumber", preset);
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
    add_float("BPM", bpm, "python");
    add_separator();
    add_newline();
    // DEBUG_PRINTF("Free memory after adds: %d\n", xPortGetFreeHeapSize());
    //  Read Pedal data (including string representations)

    // !!! number of pedals not used currently, assumed constant as 7 !!!
    // int num_effects = read_byte() - 0x90;
    // DEBUG_PRINTF("Read Number of effects: %d\n", num_effects);
    add_python("\"Pedals\": [");
    add_newline();
    currentPreset_.pedals = {};
    for (int i = 0; i < currentPreset_.numberOfPedals; i++) { // Fixed to 7, but could maybe also be derived from num_effects?
        Pedal currentPedal = {};
        // DEBUG_PRINTF("Reading Pedal %d:\n", i);
        string e_str = read_string();
        // DEBUG_PRINTF("  Pedal name: %s\n", e_str.c_str());
        currentPedal.name = e_str;
        boolean e_onoff = read_onoff();
        // DEBUG_PRINTF("  Pedal state: %s\n", e_onoff);
        currentPedal.isOn = e_onoff;
        add_python("{");
        add_str("Name", e_str);
        add_separator();
        add_bool("IsOn", e_onoff);
        add_separator();
        int num_p = read_byte() - char(0x90);
        // DEBUG_PRINTF("  Number of Parameters: %d\n", num_p);
        // DEBUG_PRINTF("Free memory before parameters: %d\n", xPortGetFreeHeapSize());
        add_python("\"Parameters\":[");
        // Read parameters of current pedal
        currentPedal.parameters = {};
        for (int p = 0; p < num_p; p++) {
            Parameter currentParameter = {};
            // DEBUG_PRINTF("  Reading parameter %d:\n", p);
            byte num = read_byte();
            // DEBUG_PRINTF("    Parameter ID: %d\n", SparkHelper::intToHex(num));
            byte spec = read_byte();
            // DEBUG_PRINTF("    Parameter Special: %s\n",
            //	SparkHelper::intToHex(spec));
            // DEBUG_PRINTF("Free memory before reading float: %d\n", xPortGetFreeHeapSize());
            float val = read_float();
            // DEBUG_PRINTF("    Parameter Value: %f\n", val);
            currentParameter.number = num;
            currentParameter.special = spec;
            currentParameter.value = val;
            // add_int("Parameter", num, "python");
            // add_str("Special", SparkHelper::intToHex(spec), "python");
            // add_float("Value", val, "python");
            add_float_pure(val, "python");
            if (p < num_p - 1) {
                add_separator();
            }
            currentPedal.parameters.push_back(currentParameter);
            // DEBUG_PRINTF("Free memory after reading preset: %d\n", xPortGetFreeHeapSize());
        }

        add_python("]");
        // del_indent();
        add_python("}");
        if (i < currentPreset_.numberOfPedals - 1) {
            add_separator();
            add_newline();
        }
        currentPreset_.pedals.push_back(currentPedal);
    }
    add_python("],");
    add_newline();
    byte filler = read_byte();
    // DEBUG_PRINTF("Preset filler ID: %s\n", SparkHelper::intToHex(filler));
    currentPreset_.filler = filler;
    add_str("Filler", SparkHelper::intToHex(filler));
    add_newline();
    end_str();
    currentPreset_.text = text;
    currentPreset_.raw = raw;
    currentPreset_.json = json;
    currentPreset_.isEmpty = false;
    isPresetUpdated_ = true;
    last_message_type_ = MSG_TYPE_PRESET;
}

void SparkStreamReader::read_looper_settings() {

    DEBUG_PRINT("Reading looper settings:");
    DEBUG_PRINTVECTOR(msg);
    DEBUG_PRINTLN();

    int bpm = read_byte();
    // if the first byte is 0xCC, this is a prefix and the real BPM are in the next byte
    // CC is prefixed if the bpm is exceeding 128.
    if (bpm == 0xCC) {
        bpm = read_byte();
    }
    int count_byte = read_byte();
    string count_str = count_byte == 0x04 ? "straight" : "shuffle";
    int bars = read_byte();
    bool free_indicator = read_onoff();
    bool click = read_onoff();
    bool unknown_onoff = read_onoff();
    byte unknown_byte = read_byte();

    // Build string representations
    start_str();
    add_int("BPM", bpm);
    add_separator();
    add_str("Count", count_str);
    add_separator();
    add_int("Bars", bars);
    add_separator();
    add_bool("Free", free_indicator);
    add_separator();
    add_bool("Click", click);
    add_separator();
    add_bool("Unknown switch", unknown_onoff);
    add_separator();
    add_str("Unknown byte", SparkHelper::intToHex(unknown_byte));
    end_str();

    looperSetting_.bpm = bpm;
    looperSetting_.count_str = count_str;
    looperSetting_.count = count_byte;
    looperSetting_.bars = bars;
    looperSetting_.free_indicator = free_indicator;
    looperSetting_.click = click;
    looperSetting_.unknown_onoff = unknown_onoff;
    looperSetting_.unknown_byte = unknown_byte;

    looperSetting_.json = json;
    looperSetting_.text = text;
    looperSetting_.raw = raw;

    isLooperSettingUpdated_ = true;
    last_message_type_ = MSG_TYPE_LOOPER_SETTING;
}

void SparkStreamReader::read_tap_tempo() {
    float bpm = read_float();

    start_str();
    add_float("BPM", bpm, "python");
    end_str();
    last_message_type_ = MSG_TYPE_TAP_TEMPO;
}

boolean SparkStreamReader::structure_data(bool processHeader) {

    ByteVector block_content;
    block_content.clear();
    message.clear();

    DEBUG_PRINTLN("Unstructured data:");

    for (auto block : unstructured_data) {

        // DEBUG_PRINTVECTOR(block);

        int block_length;
        if (processHeader) {
            block_length = block[6];
        } else {
            block_length = block.size();
        }
        int data_size = block.size();
        // DEBUG_PRINTF("Read block size %d, %d\n", block_length, data_size);
        if (data_size != block_length) {
            DEBUG_PRINTF("Data is of size %d and reports %d\n", data_size, block_length);
            Serial.println("Corrupt block:");
            for (auto by : block) {
                Serial.print(SparkHelper::intToHex(by).c_str());
            }
            Serial.println();
        }
        // DEBUG_PRINTLN("Sizes match");
        ByteVector chunk;
        if (processHeader) {
            // Cut away header
            chunk.assign(block.begin() + 16, block.end());
        } else {
            chunk.assign(block.begin(), block.end());
        }

        for (auto chunk_byte : chunk) {
            block_content.push_back(chunk_byte);
        } // FOR chunk_byte

        // DEBUG_PRINTLN("Pushed chunk bytes to block content");
    } // FOR block
    DEBUG_PRINTLN();
    // DEBUG_PRINTLN("...Processed");

    if (block_content[0] != 0xF0 || block_content[1] != 0x01) {
        Serial.println("Invalid block start, ignoring all data");
        return false;
    }

    // DEBUG_PRINTLN("Data seems correct");
    //  and split them into chunks now, splitting on each f7
    vector<ByteVector> chunks;
    chunks.clear();
    ByteVector chunk_temp = {};
    for (byte by : block_content) {
        chunk_temp.push_back(by);
        if (by == 0xf7) {
            chunks.push_back(chunk_temp);
            chunk_temp = {};
        }
    }

    vector<CmdData> chunk_8bit = {};
    for (auto chunk : chunks) {
        last_message_num_ = chunk[2];
        byte this_cmd = chunk[4];
        byte this_sub_cmd = chunk[5];
        ByteVector data7bit = {};
        data7bit.assign(chunk.begin() + 6, chunk.end() - 1);

        int chunk_len = data7bit.size();
        // DEBUG_PRINT("Chunk_len:");
        // DEBUG_PRINTLN(chunk_len);
        int num_seq = int((chunk_len + 7) / 8);
        ByteVector data8bit = {};

        for (int this_seq = 0; this_seq < num_seq; this_seq++) {
            int seq_len = min(8, chunk_len - (this_seq * 8));
            ByteVector seq = {};
            byte bit8 = data7bit[this_seq * 8];
            for (int ind = 0; ind < seq_len - 1; ind++) {
                byte dat = data7bit[this_seq * 8 + ind + 1];
                if ((bit8 & (1 << ind)) == (1 << ind)) {
                    dat |= 0x80;
                }
                seq.push_back(dat);
            }
            for (auto by : seq) {
                data8bit.push_back(by);
            }
        }
        // DEBUG_PRINTLN("Converted to 8bit");
        struct CmdData curr_data = {this_cmd, this_sub_cmd, data8bit};
        chunk_8bit.push_back(curr_data);

        // now check for mult-chunk messages and collapse their data into a single message
        // multi-chunk messages are cmd/sub_cmd of 1,1 or 3,1

        message.clear();
        ByteVector concat_data;
        concat_data.clear();
        for (CmdData chunkData : chunk_8bit) {
            this_cmd = chunkData.cmd;
            this_sub_cmd = chunkData.subcmd;
            ByteVector this_data = chunkData.data;
            if ((this_cmd == 0x01 || this_cmd == 0x03) && this_sub_cmd == 0x01) {
                // DEBUG_PRINTLN("Multi message");
                // found a multi-message
                int num_chunks = this_data[0];
                int this_chunk = this_data[1];
                ByteVector this_data_suffix;
                this_data_suffix.assign(this_data.begin() + 3, this_data.end());
                for (auto by : this_data_suffix) {
                    concat_data.push_back(by);
                }
                // if at last chunk of multi-chunk
                if (this_chunk == num_chunks - 1) {
                    // DEBUG_PRINTLN("Last chunk to process");
                    curr_data = {this_cmd, this_sub_cmd, concat_data};
                    message.push_back(curr_data);
                    concat_data = {};
                }
            } else {
                // copy old one
                message.push_back(chunkData);
            } // else
        } // For all in 8-bit vector
    } // for all chunks

    return true;
}

void SparkStreamReader::set_interpreter(const ByteVector &_msg) {
    msg = _msg;
    msg_pos = 0;
}

int SparkStreamReader::run_interpreter(byte _cmd, byte _sub_cmd) {
    // Message from APP to AMP
    if (_cmd == 0x01) {
        switch (_sub_cmd) {
        case 0x01:
            DEBUG_PRINTLN("01 01 - Reading preset");
            read_preset();
            break;
        case 0x04:
            DEBUG_PRINTLN("01 04 - Reading effect param");
            read_effect_parameter();
            break;
        case 0x06:
            DEBUG_PRINTLN("01 06 - Reading effect");
            read_effect();
            break;
        case 0x15:
            DEBUG_PRINTLN("01 15 - Reading effect on/off");
            read_effect_onoff();
            break;
        case 0x38:
            DEBUG_PRINTLN("01 38 - Change to different preset");
            read_hardware_preset();
            break;
        default:
            DEBUG_PRINTF("%0x %0x - not handled: ", _cmd, _sub_cmd);
            DEBUG_PRINTVECTOR(msg);
            DEBUG_PRINTLN();
            break;
        }

    }
    // Request to AMP
    else if (_cmd == 0x02) {
        DEBUG_PRINTLN("Reading request from Amp");
    }
    // Message from AMP to APP
    else if (_cmd == 0x03) {
        switch (_sub_cmd) {
        case 0x01:
            DEBUG_PRINTLN("03 01 - Reading preset");
            read_preset();
            break;
        case 0x06:
            DEBUG_PRINTLN("03 06 - Reading effect");
            read_effect();
            break;
        case 0x11:
            DEBUG_PRINTLN("03 11 - Reading amp name");
            read_amp_name();
            break;
        case 0x27:
            DEBUG_PRINTLN("03 27 - Storing HW preset");
            read_store_hardware_preset();
            break;
        case 0x37:
            DEBUG_PRINTLN("03 37 - Reading effect param");
            read_effect_parameter();
            break;
        case 0x38:
        case 0x10:
            DEBUG_PRINTLN("03 38/10 - Reading HW preset");
            read_hardware_preset();
            break;
        case 0x63:
            DEBUG_PRINTLN("03 63 - Reading Tap Tempo");
            read_tap_tempo();
            break;
        case 0x76:
            DEBUG_PRINTLN("03 76 - Reading Looper settings");
            read_looper_settings();
            break;
        default:
            DEBUG_PRINTF("%0x %0x - not handled: ", _cmd, _sub_cmd);
            DEBUG_PRINTVECTOR(msg);
            DEBUG_PRINTLN();
            break;
        }
    }
    // Acknowledgement
    else if (_cmd == 0x04 || _cmd == 0x05) {
        DEBUG_PRINT("ACK number ");
        DEBUG_PRINTLN(last_message_num_);
        AckData ack = {last_message_num_, _cmd, _sub_cmd};
        acknowledgments.push_back(ack);
        DEBUG_PRINTF("Acknowledgment for command %0x %0x\n", _cmd, _sub_cmd);
    } else {
        // unprocessed command (likely the initial ones sent from the app
#ifdef DEBUG
        DEBUG_PRINTF("Unprocessed: %0x, %0x - ", _cmd,
                     _sub_cmd);
        DEBUG_PRINTVECTOR(msg);
        DEBUG_PRINTLN();
#endif
    }
    return 1;
}

tuple<bool, byte, byte> SparkStreamReader::needsAck(const ByteVector &blk) {

    if (blk.size() < 22) { // Block is too short, does not need acknowledgement
        return tuple<bool, byte, byte>(false, 0, 0);
    }
    byte direction[2] = {blk[4], blk[5]};
    last_message_num_ = blk[18];
    byte cmd = blk[20];
    byte sub_cmd = blk[21];

    byte msg_to_spark[2] = {0x53, 0xFE};
    int msg_to_spark_comp = memcmp(direction, msg_to_spark, sizeof(direction));
    if (msg_to_spark_comp == 0) {
        if (cmd == 0x01 && sub_cmd != 0x04) {
            // the app sent a message that needs a response
            return tuple<bool, byte, byte>(true, last_message_num_, sub_cmd);
        }
    }

    return tuple<bool, byte, byte>(false, 0, 0);
}

AckData SparkStreamReader::getLastAckAndEmpty() {
    AckData lastAck = {0x00, 0x00};
    if (acknowledgments.size() > 0) {
        lastAck = acknowledgments.back();
        acknowledgments.clear();
    }
    return lastAck;
}

void SparkStreamReader::preProcessBlock(ByteVector &blk) {

    // Special behavior: When receiving messages from Spark APP, blocks might be split into two.
    // This will reassemble the block by appending to the previous one.

    // Iterate through block and split into F001/F7 chunks into response.

    // If nothing in response yet or if last read byte was a F7, add block
    if (response.size() == 0 || last_read_byte == end_marker) {
        if (blockIsStarted(blk)) {
            response.push_back(blk);
            last_read_byte = blk.back();
        } else {
            DEBUG_PRINTLN("Incomplete fragment found, ignoring.");
        }
        return;
    }

    auto it = blk.begin();
    ByteVector segment = {};
    ByteVector currentChunk = {};

    // Search blk for each occurrence of F7 and append to response
    while (it != blk.end()) {
        it = find(blk.begin(), blk.end(), end_marker);
        if (it != blk.end()) {
            segment.assign(blk.begin(), it + 1);
            blk.assign(it + 1, blk.end());
            if (last_read_byte != end_marker) {
                currentChunk = response.back();
                currentChunk.insert(currentChunk.end(), segment.begin(), segment.end());
                response.pop_back();
                response.push_back(currentChunk);
                currentChunk = {};
            } else {
                response.push_back(segment);
            }
            last_read_byte = response.back().back();
        }
    }
    // If a remainder is left in blk, append to previous block
    if (blk.size() > 0) {

        if (last_read_byte != end_marker) {
            currentChunk = response.back();
            currentChunk.insert(currentChunk.end(), blk.begin(), blk.end());
            response.pop_back();
            response.push_back(currentChunk);
            currentChunk = {};
        } else {
            response.push_back(blk);
        }
        last_read_byte = response.back().back();
    }
}

bool SparkStreamReader::blockIsStarted(ByteVector &blk) {
    if (blk.size() < 2)
        return false;
    bool newStart = (blk[0] == 0xF0 && blk[1] == 0x01);

    return newStart;
}

int SparkStreamReader::processBlock(ByteVector &blk) {

    int retValue = MSG_PROCESS_RES_INCOMPLETE;
    bool msg_to_spark = false;
    bool msg_from_spark = true;

    DEBUG_PRINTLN("Processing block");
    DEBUG_PRINTVECTOR(blk);
    DEBUG_PRINTLN();

    // Process:
    // 1. Remove 01FE header if present
    // 2. Build command (response) vector by splitting blocks into F001...F7 blocks

    // Remove 01FE header
    if (blk[0] == 0x01 && blk[1] == 0xFE && blk.size() > 16) {
        // Block starts with 01FE and is long enough
        // Read meta data of block
        int blk_len = blk[6];
        byte dir[2] = {blk[4], blk[5]};

        msg_to_spark = dir[0] == 0x53 && dir[1] == 0xFE;
        msg_from_spark = dir[0] == 0x41 && dir[1] == 0xFF;

        // Cut off header after extracting information
        blk.assign(blk.begin() + 16, blk.end());
    }
    // FROM HERE NO HEADER IS PRESENT ANYMORE and blk should start with F001 (after preprocessing)

    // Cut blk into chunks and append to response
    preProcessBlock(blk);

    if (response.size() == 0) {
        return retValue;
    }

    // Check if last block is final and which command
    ByteVector currentBlock = response.back();
    byte seq = currentBlock[2];
    byte cmd = currentBlock[4];
    byte sub_cmd = currentBlock[5];

    if (!(isValidBlockWithoutHeader(currentBlock))) {
        /*
        DEBUG_PRINTLN("Block not ready for processing, skipping further processing.");
        DEBUG_PRINTVECTOR(currentBlock);
        DEBUG_PRINTLN();
        */
        return retValue;
    }

    // Check if currentBlock is last block of command
    // If we don't have a 01 or 03 command with 01/10/38 sub command, we are ready to process
    if ((cmd != 0x01 && cmd != 0x03) || (sub_cmd != 01 && sub_cmd != 10 && sub_cmd != 38)) {
        msg_last_block = true;
    }
    // Multi-chunk message
    else {
        int num_chunks = currentBlock[7];
        int this_chunk = currentBlock[8];
        if ((this_chunk + 1) == num_chunks) {
            msg_last_block = true;
        }
    }

    // Process data if the block just analyzed was the last
    if (msg_last_block) {
        msg_last_block = false;
        setMessage(response);
        DEBUG_PRINTLN("Response so far: ");
        for (auto chunk : response) {
            DEBUG_PRINTVECTOR(chunk);
            DEBUG_PRINTLN();
        }

        read_message(false);
        response.clear();
        last_read_byte = 0x00;
        retValue = MSG_PROCESS_RES_COMPLETE;
    } // msg_last_block

    // Message is not complete, has not been processed yet
    // if request was an initiating one from the app, return value for INITIAL message,
    // so SparkDataControl knows how to notify.
    // so notifications will be triggered
    if (cmd == 0x02) {
        retValue = MSG_PROCESS_RES_REQUEST;
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
    // message.clear();
}

vector<CmdData> SparkStreamReader::read_message(bool processHeader) {
    if (structure_data(processHeader)) {
        interpret_data();
    }
    return message;
}

bool SparkStreamReader::isValidBlockWithoutHeader(const ByteVector &blk) {

    // Checks done:
    // 1. Block has a length of at least 3
    // 2. Block starts with F0 01
    // 3. Block ends with F7

    if (blk.size() < 3)
        return false;
    if (blk[0] != 0xF0 || blk[1] != 0x01)
        return false;
    if (blk.back() != 0xF7)
        return false;

    return true;
}

void SparkStreamReader::clearMessageBuffer() {
    DEBUG_PRINTLN("Clearing response buffer.");
    response.clear();
}

void SparkStreamReader::read_amp_name() {
    string ampName = read_prefixed_string();

    // Build string representations
    start_str();
    add_str("Amp Name", ampName);
    end_str();

    // Set values
    last_message_type_ = MSG_TYPE_AMP_NAME;
    ampName_ = ampName;
}
