/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkStreamReader.h"

SparkStreamReader::SparkStreamReader() : message{}, unstructured_data{}, msg_data{}, msg_pos(0) {
}

string SparkStreamReader::getJson() {
    return sb.getJson();
}

void SparkStreamReader::setMessage(const vector<ByteVector> &msg_) {
    unstructured_data = msg_;
    message.clear();
}

byte SparkStreamReader::read_byte() {
    byte a_byte;
    a_byte = msg_data[msg_pos];
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

void SparkStreamReader::read_effect_parameter() {
    // Read object
    string effect = read_prefixed_string();
    byte param = read_byte();
    float val = read_float();

    // Build string representations
    sb.start_str();
    sb.add_str("Effect", effect);
    sb.add_separator();
    sb.add_int("Parameter", param);
    sb.add_separator();
    sb.add_float("Value", val);
    sb.end_str();

    // Set values
    statusObject.lastMessageType() = MSG_TYPE_FX_PARAM;
}

void SparkStreamReader::read_effect() {
    // Read object
    string effect1 = read_prefixed_string();
    string effect2 = read_prefixed_string();

    // Build string representations
    sb.start_str();
    sb.add_str("OldEffect", effect1);
    sb.add_separator();
    sb.add_newline();
    sb.add_str("NewEffect", effect2);
    sb.end_str();

    // Set values
    statusObject.lastMessageType() = MSG_TYPE_FX_CHANGE;
}

void SparkStreamReader::read_hardware_preset() {
    // Read object
    read_byte();
    byte preset_num = read_byte() + 1;

    // Build string representations
    sb.start_str();
    sb.add_int("New HW Preset number", preset_num);
    sb.end_str();

    // Set values
    if (preset_num != statusObject.currentPresetNumber()) {
        statusObject.isPresetNumberUpdated() = true;
    }
    statusObject.currentPresetNumber() = preset_num;
    statusObject.lastMessageType() = MSG_TYPE_HWPRESET;
}

void SparkStreamReader::read_hw_checksums(byte sub_cmd) {

    vector<byte> checksums;
    sb.start_str();

    // determine number of HW presets based on amp type (sub_cmd)
    int number_of_presets;
    if (sub_cmd == 0x2a) {
        number_of_presets = 4;
    } else if (sub_cmd == 0x2b) {
        number_of_presets = 8;
    }

    // Array prefix byte
    read_byte();
    for (int i = 0; i < number_of_presets; i++) {
        int sum = read_int();
        checksums.push_back(sum);
        sb.add_str("Checksum Preset " + to_string(i + 1), SparkHelper::intToHex(sum));
        if (i < number_of_presets - 1) {
            sb.add_separator();
        }
    }
    sb.end_str();

    statusObject.hwChecksums() = checksums;
    statusObject.lastMessageType() = MSG_TYPE_HWCHECKSUM;
}

void SparkStreamReader::read_store_hardware_preset() {
    // Read object
    read_byte();
    byte preset_num = read_byte() + 1;

    // Build string representations
    sb.start_str();
    sb.add_int("NewStoredPreset", preset_num);
    sb.end_str();

    // Set values
    statusObject.lastMessageType() = MSG_TYPE_HWPRESET;
}

void SparkStreamReader::read_effect_onoff() {
    // Read object
    string effect = read_prefixed_string();
    boolean isOn = read_onoff();

    statusObject.currentEffect().name = effect;
    statusObject.currentEffect().isOn = isOn;
    // Build string representations
    sb.start_str();
    sb.add_str("Effect", effect);
    sb.add_separator();
    sb.add_bool("IsOn", isOn);
    sb.end_str();

    // Set values
    statusObject.lastMessageType() = MSG_TYPE_FX_ONOFF;
    statusObject.isEffectUpdated() = true;
}

void SparkStreamReader::read_preset() {
    // Read object (Preset main data)
    // DEBUG_PRINTF("Free memory before reading preset: %d\n", xPortGetFreeHeapSize());

    DEBUG_PRINTLN("Parsing message:");
    DEBUG_PRINTVECTOR(msg_data);
    DEBUG_PRINTLN();

    Preset &currentPreset = statusObject.currentPreset();

    read_byte();
    byte preset = read_byte();
    // DEBUG_PRINTF("Read PresetNumber: %d\n", preset);
    currentPreset.presetNumber = preset;
    string uuid = read_string();
    // DEBUG_PRINTF("Read UUID: %s\n", uuid.c_str());
    currentPreset.uuid = uuid;
    string name = read_string();
    // DEBUG_PRINTF("Read Name: %s\n", name.c_str());
    currentPreset.name = name;
    string version = read_string();
    // DEBUG_PRINTF("Read Version: %s\n", version.c_str());
    currentPreset.version = version;
    string descr = read_string();
    // DEBUG_PRINTF("Read Description: %s\n", descr.c_str());
    currentPreset.description = descr;
    string icon = read_string();
    // DEBUG_PRINTF("Read Icon: %s\n", icon.c_str());
    currentPreset.icon = icon;
    float bpm = read_float();
    // DEBUG_PRINTF("Read BPM: %f\n", bpm);
    currentPreset.bpm = bpm;
    // Build string representations
    // DEBUG_PRINTF("Free memory before adds: %d\n", xPortGetFreeHeapSize());
    sb.start_str();
    sb.add_int("PresetNumber", preset);
    sb.add_separator();
    sb.add_str("UUID", uuid);
    sb.add_separator();
    sb.add_newline();
    sb.add_str("Name", name);
    sb.add_separator();
    sb.add_str("Version", version);
    sb.add_separator();
    sb.add_str("Description", descr);
    sb.add_separator();
    sb.add_str("Icon", icon);
    sb.add_separator();
    sb.add_float("BPM", bpm, "python");
    sb.add_separator();
    sb.add_newline();
    // DEBUG_PRINTF("Free memory after adds: %d\n", xPortGetFreeHeapSize());
    //  Read Pedal data (including string representations)

    // !!! number of pedals not used currently, assumed constant as 7 !!!
    // int num_effects = read_byte() - 0x90;
    // DEBUG_PRINTF("Read Number of effects: %d\n", num_effects);
    sb.add_python("\"Pedals\": [");
    sb.add_newline();
    currentPreset.pedals = {};
    int numberOfPedals = currentPreset.numberOfPedals;
    for (int i = 0; i < numberOfPedals; i++) { // Fixed to 7, but could maybe also be derived from num_effects?
        Pedal currentPedal = {};
        // DEBUG_PRINTF("Reading Pedal %d:\n", i);
        string e_str = read_string();
        // DEBUG_PRINTF("  Pedal name: %s\n", e_str.c_str());
        currentPedal.name = e_str;
        boolean e_onoff = read_onoff();
        // DEBUG_PRINTF("  Pedal state: %s\n", e_onoff);
        currentPedal.isOn = e_onoff;
        sb.add_python("{");
        sb.add_str("Name", e_str);
        sb.add_separator();
        sb.add_bool("IsOn", e_onoff);
        sb.add_separator();
        int num_p = read_byte() - char(0x90);
        // DEBUG_PRINTF("  Number of Parameters: %d\n", num_p);
        // DEBUG_PRINTF("Free memory before parameters: %d\n", xPortGetFreeHeapSize());
        sb.add_python("\"Parameters\":[");
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
            // sb.add_int("Parameter", num, "python");
            // sb.add_str("Special", SparkHelper::intToHex(spec), "python");
            // sb.add_float("Value", val, "python");
            sb.add_float_pure(val, "python");
            if (p < num_p - 1) {
                sb.add_separator();
            }
            currentPedal.parameters.push_back(currentParameter);
            // DEBUG_PRINTF("Free memory after reading preset: %d\n", xPortGetFreeHeapSize());
        }

        sb.add_python("]");
        // del_indent();
        sb.add_python("}");
        if (i < numberOfPedals - 1) {
            sb.add_separator();
            sb.add_newline();
        }
        currentPreset.pedals.push_back(currentPedal);
    }
    sb.add_python("],");
    sb.add_newline();
    byte chksum = read_byte();
    currentPreset.checksum = chksum;
    sb.add_str("Checksum", SparkHelper::intToHex(chksum));
    sb.add_newline();
    sb.end_str();
    currentPreset.text = sb.getText();
    currentPreset.raw = sb.getRaw();
    currentPreset.json = sb.getJson();
    currentPreset.isEmpty = false;

    statusObject.isPresetUpdated() = true;
    statusObject.lastMessageType() = MSG_TYPE_PRESET;
}

void SparkStreamReader::read_looper_settings() {

    DEBUG_PRINT("Reading looper settings:");
    DEBUG_PRINTVECTOR(msg_data);
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
    unsigned int max_duration = read_int16();

    // Build string representations
    sb.start_str();
    sb.add_int("BPM", bpm);
    sb.add_separator();
    sb.add_str("Count", count_str);
    sb.add_separator();
    sb.add_int("Bars", bars);
    sb.add_separator();
    sb.add_bool("Free", free_indicator);
    sb.add_separator();
    sb.add_bool("Click", click);
    sb.add_separator();
    sb.add_bool("Unknown switch", unknown_onoff);
    sb.add_separator();
    sb.add_int("Max duration", max_duration);
    sb.end_str();

    LooperSetting &looperSetting = statusObject.currentLooperSetting();
    looperSetting.bpm = bpm;
    looperSetting.count_str = count_str;
    looperSetting.count = count_byte;
    looperSetting.bars = bars;
    looperSetting.free_indicator = free_indicator;
    looperSetting.click = click;
    looperSetting.unknown_onoff = unknown_onoff;
    looperSetting.max_duration = max_duration;

    looperSetting.json = sb.getJson();
    looperSetting.text = sb.getText();
    looperSetting.raw = sb.getRaw();

    statusObject.isLooperSettingUpdated() = true;
    statusObject.lastMessageType() = MSG_TYPE_LOOPER_SETTING;
}

void SparkStreamReader::read_looper_command() {

    statusObject.lastLooperCommand() = read_byte();
    DEBUG_PRINT("Received looper command: ");
    DEBUG_PRINTVECTOR(msg_data);
    DEBUG_PRINTLN();
    statusObject.lastMessageType() = MSG_TYPE_LOOPER_COMMAND;
}

void SparkStreamReader::read_looper_status() {
    // 4C0404004242
    int bpm = read_byte();
    byte count = read_byte();
    byte bars = read_byte();
    int numberOfLoops = read_byte();
    statusObject.numberOfLoops() = numberOfLoops;
    bool unknownOnOff1 = read_byte();
    bool unknownOnOff2 = read_byte();

    sb.start_str();
    sb.add_int("BPM", bpm);
    sb.add_separator();
    sb.add_int("Count", count);
    sb.add_separator();
    sb.add_int("Bars", bars);
    sb.add_separator();
    sb.add_int("Loops", numberOfLoops);
    sb.add_separator();
    sb.add_str("Unknown OnOff1", SparkHelper::intToHex(unknownOnOff1));
    sb.add_separator();
    sb.add_str("Unknown OnOff2", SparkHelper::intToHex(unknownOnOff2));
    sb.end_str();

    statusObject.lastMessageType() = MSG_TYPE_LOOPER_STATUS;
}

void SparkStreamReader::read_tap_tempo() {
    float bpm = read_float();

    sb.start_str();
    sb.add_float("BPM", bpm, "python");
    sb.end_str();
    statusObject.lastMessageType() = MSG_TYPE_TAP_TEMPO;
}

void SparkStreamReader::read_measure() {
    float measure = read_float();
    statusObject.measure() = measure;

    sb.start_str();
    sb.add_float("Measure", measure, "python");
    sb.end_str();
    statusObject.lastMessageType() = MSG_TYPE_MEASURE;
}

void SparkStreamReader::read_tuner() {
    byte note = read_byte();
    float offset = read_float();
    statusObject.note() = note;
    statusObject.note_offset() = offset;

    sb.start_str();
    sb.add_int("Note", note);
    sb.add_float("Offset", offset, "python");
    sb.end_str();
    statusObject.lastMessageType() = MSG_TYPE_TUNER_OUTPUT;
}

void SparkStreamReader::read_tuner_onoff() {
    // Read object
    boolean isOn = read_onoff();

    // Build string representations
    sb.start_str();
    sb.add_bool("Tuner mode", isOn);
    sb.end_str();

    // Set values
    if (isOn) {
        statusObject.lastMessageType() = MSG_TYPE_TUNER_ON;
    } else {
        statusObject.lastMessageType() = MSG_TYPE_TUNER_OFF;
    }
}

boolean SparkStreamReader::structure_data(bool processHeader) {

    ByteVector block_content;
    block_content.clear();
    message.clear();

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
        statusObject.lastMessageNum() = chunk[2];
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
        CmdData curr_data;
        curr_data.cmd = this_cmd;
        curr_data.subcmd = this_sub_cmd;
        curr_data.data = data8bit;

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
                    curr_data.cmd = this_cmd;
                    curr_data.subcmd = this_sub_cmd;
                    curr_data.data = concat_data;

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
    DEBUG_PRINTVECTOR(_msg);
    DEBUG_PRINTLN();
    msg_data = _msg;
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
            DEBUG_PRINTF("%02x %02x - not handled: ", _cmd, _sub_cmd);
            DEBUG_PRINTVECTOR(msg_data);
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
        case 0x2a:
            DEBUG_PRINTLN("03 2A - Reading HW checksums");
            read_hw_checksums(0x2a);
            break;
        case 0x2b:
            DEBUG_PRINTLN("03 2B - Reading HW checksums");
            read_hw_checksums(0x2b);
            break;
        case 0x06:
            DEBUG_PRINTLN("03 06 - Reading effect");
            read_effect();
            break;
        case 0x11:
            DEBUG_PRINTLN("03 11 - Reading amp name");
            read_amp_name();
            break;
        case 0x15:
            DEBUG_PRINTLN("03 15 - Reading effect on/off");
            read_effect_onoff();
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
        case 0x64:
            DEBUG_PRINTLN("03 64 - Reading Tuner Output");
            read_tuner();
            break;
        case 0x65:
            DEBUG_PRINTLN("03 65 - Tuner On/Off");
            read_tuner_onoff();
            break;
        case 0x75:
            DEBUG_PRINTLN("03 75 - Reading Looper Record Status");
            read_looper_command();
            break;
        case 0x76:
            DEBUG_PRINTLN("03 76 - Reading Looper settings");
            read_looper_settings();
            break;
        case 0x77:
            DEBUG_PRINTLN("03 77 - Reading current measure");
            read_measure();
            break;
        case 0x78:
            DEBUG_PRINTLN("03 78 - Reading current Looper status");
            read_looper_status();
            break;
        default:
            DEBUG_PRINTF("%02x %02x - not handled: ", _cmd, _sub_cmd);
            DEBUG_PRINTVECTOR(msg_data);
            DEBUG_PRINTLN();
            break;
        }
    }
    // Acknowledgement
    else if (_cmd == 0x04 || _cmd == 0x05) {
        DEBUG_PRINT("ACK number ");
        byte lastMessageNum = statusObject.lastMessageNum();
        DEBUG_PRINTLN(lastMessageNum);
        byte detail = 0x00; // detail is only used for Acks received from Amp
        AckData ack;
        ack.msg_num = lastMessageNum;
        ack.cmd = _cmd;
        ack.subcmd = _sub_cmd;
        statusObject.acknowledgments().push_back(ack);
        DEBUG_PRINTF("Acknowledgment for command %02x %02x\n", _cmd, _sub_cmd);
    } else {
        // unprocessed command (likely the initial ones sent from the app
        DEBUG_PRINTF("Unprocessed: %02x, %02x - ", _cmd,
                     _sub_cmd);
        DEBUG_PRINTVECTOR(msg_data);
        DEBUG_PRINTLN();
    }
    return 1;
}

tuple<bool, byte, byte> SparkStreamReader::needsAck(const ByteVector &blk) {

    if (blk.size() < 22) { // Block is too short, does not need acknowledgement
        return tuple<bool, byte, byte>(false, 0, 0);
    }
    byte direction[2] = {blk[4], blk[5]};
    byte lastMessageNum = blk[18];
    statusObject.lastMessageNum() = lastMessageNum;
    byte cmd = blk[20];
    byte sub_cmd = blk[21];

    byte msg_to_spark[2] = {0x53, 0xFE};
    int msg_to_spark_comp = memcmp(direction, msg_to_spark, sizeof(direction));
    if (msg_to_spark_comp == 0) {
        if (cmd == 0x01 && sub_cmd != 0x04) {
            // the app sent a message that needs a response
            return tuple<bool, byte, byte>(true, lastMessageNum, sub_cmd);
        }
    }

    return tuple<bool, byte, byte>(false, 0, 0);
}

AckData SparkStreamReader::getLastAckAndEmpty() {
    AckData lastAck;
    vector<AckData> acknowledgments = statusObject.acknowledgments();
    if (acknowledgments.size() > 0) {
        lastAck = acknowledgments.back();
        statusObject.resetAcknowledgments();
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

    /*
        DEBUG_PRINTLN("Processing block");
        DEBUG_PRINTVECTOR(blk);
        DEBUG_PRINTLN();
    */
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
        DEBUG_PRINT("Response received: ");
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

int SparkStreamReader::read_int() {

    byte first = read_byte();
    // If int value is greater than 128 it is prefixed with 0xCC
    if (first == 0xcc) {
        return read_byte();
    } else {
        return first;
    }
}

unsigned int SparkStreamReader::read_int16() {
    // Read the following two bytes as INT
    // INT is prefixed with 0xCD
    byte prefix = read_byte();
    byte major = read_byte();
    byte minor = read_byte();
    unsigned int result = (major << 8 | minor);
    return result;
}

void SparkStreamReader::clearMessageBuffer() {
    DEBUG_PRINTLN("Clearing response buffer.");
    response.clear();
}

void SparkStreamReader::read_amp_name() {
    string ampName = read_prefixed_string();

    // Build string representations
    sb.start_str();
    sb.add_str("Amp Name", ampName);
    sb.end_str();

    // Set values
    statusObject.lastMessageType() = MSG_TYPE_AMP_NAME;
    statusObject.ampName() = ampName;
}
