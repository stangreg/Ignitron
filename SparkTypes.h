#ifndef SPARK_TYPES_H
#define SPARK_TYPES_H

#include <vector>
#include <Arduino.h>
#include "SparkHelper.h"

using ByteVector = std::vector<byte>;

// Positions of FX types in preset struct
#define FX_NOISEGATE 	0
#define FX_COMP  	 	1
#define FX_DRIVE 		2
#define FX_AMP 			3
#define FX_MOD 			4
#define FX_DELAY 		5
#define FX_REVERB 		6

struct parameter {

	int number;
	std::string special;
	float value;
};

struct pedal {

	std::string name;
	boolean isOn;
	std::vector<parameter> parameters;

};

struct preset {

	boolean isEmpty = true;

	std::string json;
	std::string raw;
	std::string text;
	std::string indent;

	int presetNumber;
	std::string uuid;
	std::string name;
	std::string version;
	std::string description;
	std::string icon;
	float bpm;
	std::vector<pedal> pedals;
	byte filler;

	void start_str() {
		text = "";
		json = "{";
		raw = "";
		indent = "";
	}

	void end_str() {
		json += "}";
	}

	void add_indent() {
		indent += "\t";
	}

	void del_indent() {
		indent = indent.substr(1);
	}

	void add_json(char* python_str) {
		json += indent + python_str + "\n";
	}

	void add_str(char* a_title, std::string a_str, char* nature = "all") {
		raw +=  a_str;
		raw += " ";
		char string_add[200] = "";
		sprintf(string_add, "%s%-20s: %s \n", indent.c_str(), a_title, a_str.c_str());
		text += string_add;
		if (nature != "python") {
			json += indent + "\"" + a_title + "\":\"" + a_str + "\",\n";
		}
	}


	void add_int(char* a_title, int an_int, char* nature = "all") {
		char string_add[200] ="";
		sprintf(string_add, "%d ", an_int);
		raw += string_add;
		sprintf(string_add, "%s%-20s: %d\n", indent.c_str(), a_title, an_int);
		text += string_add;
		if (nature != "python") {
			sprintf(string_add, "%s\"%-20s\": %d,\n", indent.c_str(), a_title, an_int);
			json += string_add;
		}
	}



	void add_float(char* a_title, float a_float, char* nature = "all") {
		char string_add[200] = "";
		sprintf(string_add, "%2.4f ", a_float);
		raw += string_add;
		sprintf(string_add, "%s%-20s: %2.4f\n", indent.c_str(), a_title, a_float);
		text += string_add;
		if (nature == "python") {
			sprintf(string_add, "%s%2.4f,\n", indent.c_str(), a_float);
			json += string_add;
		}
		else {
			sprintf(string_add, "%s\"%s\": %2.4f,\n", indent.c_str(), a_title, a_float);
			json += string_add;
		}
	}

	void add_bool(char* a_title, boolean a_bool, char* nature = "all") {
		char string_add[200] ="";
		sprintf(string_add, "%s ", a_bool ? "true" : "false");
		raw += string_add;
		sprintf(string_add, "%s%s: %-20s\n", indent.c_str(), a_title, a_bool ? "true" : "false");
		text += string_add;
		if (nature != "python") {
			sprintf(string_add, "%s\"%s\": %s,\n", indent.c_str(), a_title, a_bool ? "true" : "false");
			json += string_add;
		}
	}


	std::string getJson(){
		start_str();
		add_int ("PresetNumber", presetNumber);
		add_str("UUID", uuid);
		add_str("Name", name);
		add_str("Version", version);
		add_str("Description", description);
		add_str("Icon", icon);
		add_float("BPM", bpm);
		add_json("\"Pedals\": [");
		add_indent();
		for (int i = 0; i < pedals.size(); i++) { // Fixed to 7, but could maybe also be derived from num_effects?
			pedal currentPedal = pedals[i];
			add_json ("{");
			add_str("Name", currentPedal.name);
			add_bool("IsOn", currentPedal.isOn);
			add_json("\"Parameters\":[");
			add_indent();
			for (int p = 0; p < currentPedal.parameters.size(); p++) {
				parameter currentParam = currentPedal.parameters[p];
				add_int("Parameter", currentParam.number, "python");
				add_str("Special", currentParam.special, "python");
				add_float("Value", currentParam.value, "python");
			}

			add_json("],");
			del_indent();
			add_json("},");
		}
		add_json("],");
		del_indent();
		add_str("Filler", SparkHelper::intToHex(filler));
		end_str();

		return json;
	}

};

struct cmd_data {
	byte cmd;
	byte subcmd;
	ByteVector data;

	void print() {
		Serial.print("[");
		Serial.print(cmd);
		Serial.print("], [");
		Serial.print(subcmd);
		Serial.print("], [");
		for (byte by : data) {
			Serial.print(SparkHelper::intToHex(by).c_str());
			Serial.print(", ");
		}
		Serial.print("]");
	}
};


#endif
