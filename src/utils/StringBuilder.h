#ifndef STRINGBUILDER_H
#define STRINGBUILDER_H

#include <string>

using namespace std;

class StringBuilder {

    // String representations of processed data
    string raw;
    string text;
    string indent;
    string json;

public:
    // Functions to create string representations of processed data
    void startStr();
    void endStr();
    void addIndent();
    void deleteIndent();
    void addSeparator();
    void addNewline();
    void addPython(string pythonStr);
    void addStr(const string &aTitle, const string &aStr, string nature = "all");
    void addInt(const string &aTitle, int anInt, string nature = "all");
    void addFloat(const string &aTitle, float aFloat, string nature = "all");
    void addFloatPure(float aFloat, string nature = "all");
    void addBool(const string &aTitle, bool aBool, string nature = "all");

    const string getJson() const { return json; }
    const string getRaw() const { return raw; }
    const string getText() const { return text; }
};

#endif