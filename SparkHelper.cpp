#include "SparkHelper.h"

using ByteVector = std::vector<byte>;

ByteVector SparkHelper::HexToBytes(const std::string& hex) {
	ByteVector bytes;

	for (unsigned int i = 0; i < hex.length(); i += 2) {
		std::string byteString = hex.substr(i, 2);
		byte by = (byte) strtol(byteString.c_str(), NULL, 16);
		bytes.push_back(by);
	}

	return bytes;
}

std::string SparkHelper::hexStr(byte *data, int len){
	std::stringstream ss;
	ss << std::hex;

	for ( int i(0) ; i < len; ++i )
		ss << std::setw(2) << std::setfill('0') << (int)data[i];

	return ss.str();
}

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

ByteVector SparkHelper::bytes(int value){

	ByteVector value_bytes;
	int val = value;
	while (val != 0)
	{
		std::uint8_t byte = val & 0xFF;
		value_bytes.insert(value_bytes.begin(), byte);

		val >>= 8;
	}

	return value_bytes;
	/*
    Serial.print("Converted ");
    Serial.print(value);
    Serial.print(" to ");
    printByteVector(value_bytes);
    Serial.println();
    return value_bytes;
	 */
}

