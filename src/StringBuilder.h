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
    // TODO Extract String building into own class to use it outside of this class.
    // Functions to create string representations of processed data
    void start_str();
    void end_str();
    void add_indent();
    void del_indent();
    void add_separator();
    void add_newline();
    void add_python(string python_str);
    void add_str(const string &a_title, const string &a_str, string nature = "all");
    void add_int(const string &a_title, int an_int, string nature = "all");
    void add_float(const string &a_title, float a_float, string nature = "all");
    void add_float_pure(float a_float, string nature = "all");
    void add_bool(const string &a_title, bool a_bool, string nature = "all");

    const string getJson() const { return json; }
    const string getRaw() const { return raw; }
    const string getText() const { return text; }
};

#endif