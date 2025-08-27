#include "StringBuilder.h"

void StringBuilder::startStr() {
    text = "";
    json = "{";
    raw = "";
    // dict={};
    indent = "";
}

void StringBuilder::endStr() {
    json += "}";
}

void StringBuilder::addIndent() {
    indent += "\t";
}

void StringBuilder::deleteIndent() {
    indent.resize(indent.size() - 1);
}

void StringBuilder::addSeparator() {
    json += ", ";
}

void StringBuilder::addNewline() {
    json += "\n";
}

void StringBuilder::addPython(string python_str) {
    json += indent + python_str; // + "\n";
}

void StringBuilder::addStr(const string &aTitle, const string &aStr, string nature) {
    raw += aStr;
    raw += " ";
    char stringAdd[200] = "";
    int size = sizeof stringAdd;
    snprintf(stringAdd, size, "%s%-20s: %s \n", indent.c_str(), aTitle.c_str(), aStr.c_str());
    text += stringAdd;
    if (nature != "python") {
        json += indent + "\"" + aTitle + "\": \"" + aStr + "\"";
    }
}

void StringBuilder::addInt(const string &aTitle, int anInt, string nature) {
    char stringAdd[100] = "";
    int size = sizeof stringAdd;
    snprintf(stringAdd, size, "%d ", anInt);
    raw += stringAdd;
    snprintf(stringAdd, size, "%s%-20s: %d\n", indent.c_str(), aTitle.c_str(), anInt);
    text += stringAdd;
    if (nature != "python") {
        snprintf(stringAdd, size, "%s\"%s\": %d", indent.c_str(), aTitle.c_str(), anInt);
        json += stringAdd;
    }
}

void StringBuilder::addFloat(const string &aTitle, float aFloat, string nature) {

    char stringAdd[100] = "";
    int size = sizeof stringAdd;
    snprintf(stringAdd, size, "%2.4f ", aFloat);
    raw += stringAdd;
    snprintf(stringAdd, size, "%s%-20s: %2.4f\n", indent.c_str(), aTitle.c_str(), aFloat);
    text += stringAdd;
    if (nature != "python") {
        snprintf(stringAdd, size, "%s%2.4f", indent.c_str(), aFloat);
        json += stringAdd;
    } else {
        snprintf(stringAdd, size, "%s\"%s\": %2.4f", indent.c_str(), aTitle.c_str(), aFloat);
        json += stringAdd;
    }
}

void StringBuilder::addFloatPure(float aFloat, string nature) {
    char stringAdd[100] = "";
    int size = sizeof stringAdd;
    snprintf(stringAdd, size, "%2.4f ", aFloat);
    raw += stringAdd;
    snprintf(stringAdd, size, "%2.4f ", aFloat);
    text += stringAdd;
    snprintf(stringAdd, size, "%2.4f", aFloat);
    json += stringAdd;
}

void StringBuilder::addBool(const string &aTitle, bool aBool, string nature) {
    char stringAdd[100] = "";
    int size = sizeof stringAdd;
    snprintf(stringAdd, size, "%s ", aBool ? "true" : "false");
    raw += stringAdd;
    snprintf(stringAdd, size, "%s%s: %-20s\n", indent.c_str(), aTitle.c_str(), aBool ? "true" : "false");
    text += stringAdd;
    if (nature != "python") {
        snprintf(stringAdd, size, "%s\"%s\": %s", indent.c_str(), aTitle.c_str(), aBool ? "true" : "false");
        json += stringAdd;
    }
}
