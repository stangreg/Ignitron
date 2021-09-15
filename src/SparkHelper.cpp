/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "../src/SparkHelper.h"

using ByteVector = std::vector<byte>;


byte SparkHelper::HexToByte(const std::string& hex) {
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


std::string SparkHelper::intToHex(byte by) {
	char hex_string[20];
	sprintf(hex_string, "%02X", by);
	return hex_string;
}

void SparkHelper::printDataAsHexString(std::vector<ByteVector>data) {
	for (auto elements : data) {
		for (auto by : elements) {
			char hex_string[20];

			sprintf(hex_string, "%02X", by);
			Serial.print(hex_string);
			Serial.print(" ");
		}
		Serial.println();
	}
}

void SparkHelper::printByteVector(ByteVector vec){
	for (auto by: vec){
		Serial.print(SparkHelper::intToHex(by).c_str());
	}
}

