/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkHelper.h"

using ByteVector = vector<byte>;


byte SparkHelper::HexToByte(const string& hex) {
	byte ret_byte;

	if (hex.length() <= 2){
		ret_byte = (byte) strtol(hex.c_str(), NULL, 16);
	}
	else{
		Serial.println("Error in HexToByte: String too long");
		ret_byte = 0x00;
	}
	return ret_byte;
}


string SparkHelper::intToHex(byte by) {
	char hex_string[20];
	int size = sizeof hex_string;
	snprintf(hex_string, size, "%02X", by);
	return hex_string;
}

void SparkHelper::printDataAsHexString(const vector<ByteVector>& data) {
	for (auto elements : data) {
		for (auto by : elements) {
			char hex_string[20];
			int size = sizeof hex_string;
			snprintf(hex_string, size, "%02X", by);
			Serial.print(hex_string);
			Serial.print(" ");
		}
		Serial.println();
	}
}

void SparkHelper::printByteVector(const ByteVector& vec){
	for (auto by: vec){
		Serial.print(SparkHelper::intToHex(by).c_str());
	}
}

int SparkHelper::dataVectorNumOfBytes(const vector<ByteVector>& data) {
	int count = 0;
	for (auto vec : data) {
		count += vec.size();
	}
	return count;
}

int SparkHelper::getButtonNumber(int btn_gpio){

	switch (btn_gpio) {
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
		return -1;
	}
}

int SparkHelper::getFXIndexFromBtnGpio(int btn_gpio){
	switch (btn_gpio) {
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
		return -1;
	}

}

int SparkHelper::getLedGpio(int btn_number){
	switch(btn_number) {
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
		return -1;
	}

}

int SparkHelper::getFXIndexFromButtonNumber(int btn_number){
	switch (btn_number) {
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
	}
}

int SparkHelper::searchSubVector(const ByteVector& vectorToSearchIn,
		const ByteVector& vectorToFind) {
	auto it = std::search(vectorToSearchIn.begin(), vectorToSearchIn.end(), vectorToFind.begin(), vectorToFind.end());
	return it - vectorToSearchIn.begin();

}
