/*
 * SparkDisplayControl.cpp
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#include "SparkDisplayControl.h"

Adafruit_SSD1306 SparkDisplayControl::display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

SparkDisplayControl::SparkDisplayControl() {
	spark_dc = null;
	init();

}

SparkDisplayControl::SparkDisplayControl(SparkDataControl* dc) {
	Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
	spark_dc = dc;
	init();

}
SparkDisplayControl::~SparkDisplayControl() {
	// TODO Auto-generated destructor stub
}

void SparkDisplayControl::init() {
	Serial.println("Init display");
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
		Serial.println(F("SSD1306 allocation failed"));
		for (;;)
			; // Don't proceed, loop forever
	}
	// Clear the buffer
	display.clearDisplay(); //Clear the buffer first so we don't see Adafruit splash
	display.display();

	display.setTextColor(SSD1306_WHITE);
	display.setTextWrap(false);
	display_x = 0;
	display_minX = DISPLAY_MIN_X_FACTOR * 10; //initialize, will be updated with new preset
	display.drawBitmap(0, 0, sweaty_logo_bmp, 128, 47, SSD1306_WHITE);
	display.setTextSize(2);
	display.setCursor(6, 48);
	display.print("Connecting");
	display.display();
}


void SparkDisplayControl::update() {
	int activeBank = spark_dc->activeBank();
	int pendingBank = spark_dc->pendingBank();
	preset* activePreset = spark_dc->activePreset();
	preset* pendingPreset = spark_dc->pendingPreset();
	int buttonMode = spark_dc->buttonMode();
	int activePresetNum = spark_dc->activePresetNum();

	std::string displayPresetName;
	preset* displayPreset;


	display.setTextColor(SSD1306_WHITE);
	//Configure numbers as strings
	std::ostringstream selBankStr;
	selBankStr << pendingBank;

	std::ostringstream selPresetStr;
	selPresetStr << activePresetNum;

	display.clearDisplay();
	display.setCursor(display.width() - 48, 1);

	// Preset display
	display.setTextSize(4);
	std::string presetText = " ";
	if (buttonMode == SWITCH_MODE_FX) {
		//display.setCursor((display.width() / 2) + 18, 1);
		//display.print("M");
		presetText = "M";

	}
	presetText += selPresetStr.str();
	display.print(presetText.c_str());

	// Bank number display
	std::string bankDisplay = "";
	if (pendingBank < 10) {
		bankDisplay = "0";
	}
	bankDisplay += selBankStr.str();
	if (pendingBank == 0) {
		bankDisplay = "HW";
	}

	display.setCursor(1, 1);
	display.print(bankDisplay.c_str());

	//RECTANGLE COLOR FOR PRESET NAME
	int rectColor;
	int textColor;
	if (pendingBank == activeBank) {
		rectColor = SSD1306_BLACK;
		textColor = SSD1306_WHITE;
	} else {
		rectColor = SSD1306_WHITE;
		textColor = SSD1306_BLACK;
	}

	display.setTextColor(textColor);
	display.fillRect(0, 31, 128, 16, rectColor);

	display.setTextSize(2);
	display.setCursor(display_x, 32);

	if (pendingBank != 0 && activeBank != pendingBank) {
		//displayPreset = (*presetBanks)[selectedBank - 1][selectedPresetNum - 1];
		displayPreset = pendingPreset;
		displayPresetName = displayPreset->name;
	} else if (pendingBank == 0 && activeBank != pendingBank) {
		displayPreset = activePreset;
		displayPresetName = "Hardware Preset " + selPresetStr.str();
	} else {
		displayPreset = activePreset;
		displayPresetName = displayPreset->name;
	}

	display_minX = DISPLAY_MIN_X_FACTOR * displayPresetName.length()
					+ display.width();
	display.print(displayPresetName.c_str());

	std::string fx_display = " ";
	std::string currPedalStatus;
	std::string fx_indicators_on[] = { "N", "C ", "D ", "  ", "M ", "D ", "R" }; // blank placeholder for Amp
	std::string fx_indicators_off[] = { " ", "  ", "  ", "  ", "  ", "  ", " " }; // blank placeholder for Amp

	if (buttonMode == SWITCH_MODE_FX) {
		displayPreset = activePreset;
	}
	if (!displayPreset->isEmpty || pendingBank > 0) {
		for (int i = 0; i < 7; i++) { // 7 pedals, amp to be ignored
			if (i != 3) {// && i != 0) { // Noise Reduction position 0 Amp is on position 3
				pedal currPedal = displayPreset->pedals[i];
				currPedalStatus =
						displayPreset->pedals[i].isOn ? fx_indicators_on[i] : fx_indicators_off[i];
				fx_display += currPedalStatus;
			}
		}
	} else {
		fx_display = "";
	}

	display.fillRect(0, 48, 128, 16, SSD1306_BLACK);

	display.setTextColor(SSD1306_WHITE);
	display.setTextSize(2);
	display.setCursor(-6, 49);
	display.print(fx_display.c_str());

	if (buttonMode == SWITCH_MODE_FX) {
		display.invertDisplay(true);
	} else {
		display.invertDisplay(false);
	}


	if(displayPresetName.length()<=12){
		display_x = 0;
	}
	else{
		display_x += display_scroll_num;
		if (display_x < display_minX || display_x > 1) {
			if( display_x < display_minX){
				display_x=display_minX;
			}
			else{
				display_x=1;
			}
			display_scroll_num = -display_scroll_num;
			display_x += display_scroll_num;
		}
	}
	display.display();
}
