/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARK_HELPER_H // include guard
#define SPARK_HELPER_H

#include <array>
#include <vector>
#include <iomanip>
#include <sstream>

#include <Arduino.h>
#include "Config_Definitions.h"


using ByteVector = std::vector<byte>;

class SparkHelper{

private:


public:
	// Convert a string to a ByteVector
	//static ByteVector HexToBytes(const std::string& hex);
	// Convert a string hex item to a byte
	static byte HexToByte(const std::string& hex);

	// Convert a byte array to a string
	//static std::string hexStr(byte *data, int len);
	// convert a byte to a string hex representation
	static std::string intToHex(byte by);
	// print a vector of byte vectors;
	static void printDataAsHexString(std::vector<ByteVector>data);
	// Print a byte vector
	static void printByteVector(ByteVector vec);
	// get a byteVector from int value
	//static ByteVector bytes(int value);

	static int dataVectorNumOfBytes(std::vector<ByteVector> data);

	static int getButtonNumber(int btn_gpio);
	static int getFXIndexFromBtnGpio(int btn_gpio);
	static int getLedGpio(int btn_number);
	static int getFXIndexFromButtonNumber(int btn_number);
};

#endif
