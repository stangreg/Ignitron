#include "StringBuilder.h"

void StringBuilder::start_str() {
    text = "";
    json = "{";
    raw = "";
    // dict={};
    indent = "";
}

void StringBuilder::end_str() {
    json += "}";
}

void StringBuilder::add_indent() {
    indent += "\t";
}

void StringBuilder::del_indent() {
    indent.resize(indent.size() - 1);
}

void StringBuilder::add_separator() {
    json += ", ";
}

void StringBuilder::add_newline() {
    json += "\n";
}

void StringBuilder::add_python(string python_str) {
    json += indent + python_str; // + "\n";
}

void StringBuilder::add_str(const string &a_title, const string &a_str, string nature) {
    raw += a_str;
    raw += " ";
    char string_add[200] = "";
    int size = sizeof string_add;
    snprintf(string_add, size, "%s%-20s: %s \n", indent.c_str(), a_title.c_str(), a_str.c_str());
    text += string_add;
    if (nature != "python") {
        json += indent + "\"" + a_title + "\": \"" + a_str + "\"";
    }
}

void StringBuilder::add_int(const string &a_title, int an_int, string nature) {
    char string_add[100] = "";
    int size = sizeof string_add;
    snprintf(string_add, size, "%d ", an_int);
    raw += string_add;
    snprintf(string_add, size, "%s%-20s: %d\n", indent.c_str(), a_title.c_str(), an_int);
    text += string_add;
    if (nature != "python") {
        snprintf(string_add, size, "%s\"%s\": %d", indent.c_str(), a_title.c_str(), an_int);
        json += string_add;
    }
}

void StringBuilder::add_float(const string &a_title, float a_float, string nature) {

    char string_add[100] = "";
    int size = sizeof string_add;
    snprintf(string_add, size, "%2.4f ", a_float);
    raw += string_add;
    snprintf(string_add, size, "%s%-20s: %2.4f\n", indent.c_str(), a_title.c_str(), a_float);
    text += string_add;
    if (nature != "python") {
        snprintf(string_add, size, "%s%2.4f", indent.c_str(), a_float);
        json += string_add;
    } else {
        snprintf(string_add, size, "%s\"%s\": %2.4f", indent.c_str(), a_title.c_str(), a_float);
        json += string_add;
    }
}

void StringBuilder::add_float_pure(float a_float, string nature) {
    char string_add[100] = "";
    int size = sizeof string_add;
    snprintf(string_add, size, "%2.4f ", a_float);
    raw += string_add;
    snprintf(string_add, size, "%2.4f ", a_float);
    text += string_add;
    snprintf(string_add, size, "%2.4f", a_float);
    json += string_add;
}

void StringBuilder::add_bool(const string &a_title, bool a_bool, string nature) {
    char string_add[100] = "";
    int size = sizeof string_add;
    snprintf(string_add, size, "%s ", a_bool ? "true" : "false");
    raw += string_add;
    snprintf(string_add, size, "%s%s: %-20s\n", indent.c_str(), a_title.c_str(), a_bool ? "true" : "false");
    text += string_add;
    if (nature != "python") {
        snprintf(string_add, size, "%s\"%s\": %s", indent.c_str(), a_title.c_str(), a_bool ? "true" : "false");
        json += string_add;
    }
}
