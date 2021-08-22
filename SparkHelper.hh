#ifndef SPARK_HELPER_H // include guard
#define SPARK_HELPER_H

#include <array>
#include <vector>
#include <iomanip>
#include <sstream>

#include <Arduino.h>

using ByteVector = std::vector<byte>;

class SparkHelper{

    private:


  public:
    static ByteVector HexToBytes(const std::string& hex);
    static byte HexToByte(const std::string& hex);

    //static ByteVector HexToBytes(const ByteVector hex);
    static std::string hexStr(byte *data, int len);
    static std::string intToHex(byte by);
    static void printDataAsHexString(std::vector<ByteVector>data);
    static void printByteVector(ByteVector vec);
    static ByteVector bytes(int value);
};

#endif
