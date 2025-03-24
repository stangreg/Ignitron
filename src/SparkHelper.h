/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARK_HELPER_H // include guard
#define SPARK_HELPER_H

#include <array>
#include <iomanip>
#include <vector>
// #include <sstream>
#include <cstdio>

#include "Config_Definitions.h"
#include <Arduino.h>

using namespace std;
using ByteVector = vector<byte>;

class SparkHelper {

private:
public:
    // Convert a string to a ByteVector
    // static ByteVector HexToBytes(const string& hex);
    // Convert a string hex item to a byte
    static byte HexToByte(const string &hex);
    static ByteVector hexStringToByteVector(const string &hexString);
    // Internal test function
    static ByteVector stripHeader(ByteVector input);

    // Convert a byte array to a string
    // static string hexStr(byte *data, int len);
    // convert a byte to a string hex representation
    static string intToHex(byte by);
    // print a vector of byte vectors;
    static void printDataAsHexString(const vector<ByteVector> &data);
    // Print a byte vector
    static void printByteVector(const ByteVector &vec);
    // get a byteVector from int value
    // static ByteVector bytes(int value);

    static int dataVectorNumOfBytes(const vector<ByteVector> &data);

    static PresetLedButtonNum getButtonNumber(ButtonGpio btn_gpio);
    static FxType getFXIndexFromBtnGpio(ButtonGpio btn_gpio);
    static LedGpio getLedGpio(int btn_number, bool fxMode);
    static FxType getFXIndexFromButtonNumber(FxLedButtonNumber btn_number);

    static int searchSubVector(const ByteVector &vectorToSearchIn, const ByteVector &vectorToFind);
};

#endif
