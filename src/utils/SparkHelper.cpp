/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkHelper.h"

using ByteVector = vector<byte>;

byte SparkHelper::HexToByte(const string &hex) {
    byte ret_byte;

    if (hex.length() <= 2) {
        ret_byte = (byte)strtol(hex.c_str(), NULL, 16);
    } else {
        Serial.println("Error in HexToByte: String too long");
        ret_byte = 0x00;
    }
    return ret_byte;
}

ByteVector SparkHelper::hexStringToByteVector(const string &hexString) {
    ByteVector outputVector = {};
    for (unsigned int i = 0; i < hexString.length(); i += 2) {
        string byteString = hexString.substr(i, 2);
        byte by = HexToByte(byteString);
        outputVector.push_back(by);
    }
    return outputVector;
}

ByteVector SparkHelper::stripHeader(ByteVector input) {
    int numberOfChunks = input[0];
    int length = 0;
    int start = 0;
    int end = 0;
    ByteVector currentVector;
    ByteVector output;
    for (int currentPos = 0; currentPos < input.size();) {
        length = input[currentPos + 2];
        start = currentPos + 3;
        end = min(start + length, (int)input.size());
        currentPos = end;
        currentVector.assign(input.begin() + start, input.begin() + end);
        for (auto by : currentVector) {
            output.push_back(by);
        }
        currentVector.clear();
    }
    DEBUG_PRINTLN("Pure Preset data:");
    DEBUG_PRINTVECTOR(output);
    DEBUG_PRINTLN();
    return output;
}

string SparkHelper::intToHex(byte by) {
    char hexString[20];
    int size = sizeof hexString;
    snprintf(hexString, size, "%02X", by);
    return hexString;
}

void SparkHelper::printDataAsHexString(const vector<ByteVector> &data) {
    for (auto elements : data) {
        for (auto by : elements) {
            char hexString[20];
            int size = sizeof hexString;
            snprintf(hexString, size, "%02X", by);
            Serial.print(hexString);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void SparkHelper::printByteVector(const ByteVector &vec) {
    for (auto by : vec) {
        Serial.print(SparkHelper::intToHex(by).c_str());
    }
}

int SparkHelper::dataVectorNumOfBytes(const vector<ByteVector> &data) {
    int count = 0;
    for (auto vec : data) {
        count += vec.size();
    }
    return count;
}

PresetLedButtonNum SparkHelper::getButtonNumber(ButtonGpio btnGpio) {

    switch (btnGpio) {
    case BUTTON_PRESET1_GPIO:
        return PRESET1_NUM;
        break;
    case BUTTON_PRESET2_GPIO:
        return PRESET2_NUM;
        break;
    case BUTTON_PRESET3_GPIO:
        return PRESET3_NUM;
        break;
    case BUTTON_PRESET4_GPIO:
        return PRESET4_NUM;
        break;
    case BUTTON_BANK_DOWN_GPIO:
        return BANK_DOWN_NUM;
        break;
    case BUTTON_BANK_UP_GPIO:
        return BANK_UP_NUM;
        break;
    default:
        return INVALID_PRESET_BUTTON_NUM;
    }
}

FxType SparkHelper::getFXIndexFromBtnGpio(ButtonGpio btnGpio) {
    switch (btnGpio) {
    case BUTTON_NOISEGATE_GPIO:
        return INDEX_FX_NOISEGATE;
        break;
    case BUTTON_COMP_GPIO:
        return INDEX_FX_COMP;
        break;
    case BUTTON_DRIVE_GPIO:
        return INDEX_FX_DRIVE;
        break;
    case BUTTON_MOD_GPIO:
        return INDEX_FX_MOD;
        break;
    case BUTTON_DELAY_GPIO:
        return INDEX_FX_DELAY;
        break;
    case BUTTON_REVERB_GPIO:
        return INDEX_FX_REVERB;
        break;
    default:
        return INDEX_FX_INVALID;
    }
}

LedGpio SparkHelper::getLedGpio(int btnNumber, bool fxMode) {
    if (!fxMode) { // Preset mode LEDs
        switch (btnNumber) {
        case 1:
            return LED_PRESET1_GPIO;
            break;
        case 2:
            return LED_PRESET2_GPIO;
            break;
        case 3:
            return LED_PRESET3_GPIO;
            break;
        case 4:
            return LED_PRESET4_GPIO;
            break;
        case 5:
            return LED_BANK_DOWN_GPIO;
            break;
        case 6:
            return LED_BANK_UP_GPIO;
            break;
        default:
            return LED_GPIO_INVALID;
        }
    } else { // FX Mode LEDs
        switch (btnNumber) {
        case 1:
            return LED_DRIVE_GPIO;
            break;
        case 2:
            return LED_MOD_GPIO;
            break;
        case 3:
            return LED_DELAY_GPIO;
            break;
        case 4:
            return LED_REVERB_GPIO;
            break;
        case 5:
            return LED_NOISEGATE_GPIO;
            break;
        case 6:
            return LED_COMP_GPIO;
            break;
        default:
            return LED_GPIO_INVALID;
        }
    }
}

FxType SparkHelper::getFXIndexFromButtonNumber(FxLedButtonNumber btnNumber) {
    switch (btnNumber) {
    case NOISEGATE_NUM:
        return INDEX_FX_NOISEGATE;
        break;
    case COMP_NUM:
        return INDEX_FX_COMP;
        break;
    case DRIVE_NUM:
        return INDEX_FX_DRIVE;
        break;
    case MOD_NUM:
        return INDEX_FX_MOD;
        break;
    case DELAY_NUM:
        return INDEX_FX_DELAY;
        break;
    case REVERB_NUM:
        return INDEX_FX_REVERB;
        break;
    default:
        return INDEX_FX_INVALID;
    }
}

int SparkHelper::searchSubVector(const ByteVector &vectorToSearchIn,
                                 const ByteVector &vectorToFind) {
    auto it = std::search(vectorToSearchIn.begin(), vectorToSearchIn.end(), vectorToFind.begin(), vectorToFind.end());
    return it - vectorToSearchIn.begin();
}
