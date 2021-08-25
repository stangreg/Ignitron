/*
 * SparkDisplayControl.cpp
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#include "SparkDisplayControl.h"

Adafruit_SSD1306 SparkDisplayControl::display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

SparkDisplayControl::SparkDisplayControl() : SparkDisplayControl(nullptr) {

}

SparkDisplayControl::SparkDisplayControl(SparkDataControl* dc) {
	Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
	spark_dc = dc;
}
SparkDisplayControl::~SparkDisplayControl() {
	// TODO Auto-generated destructor stub
}

void SparkDisplayControl::init(int mode) {
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C required for this display
		Serial.println(F("SSD1306 initialization failed"));
		for (;;)
			; // Loop forever
	}
	// Clear the buffer
	display.clearDisplay(); //No Adafruit splash
	display.display();
	display.setTextColor(SSD1306_WHITE);

	if(mode == SPARK_MODE_APP){
		display.setTextWrap(false);
		display_x = 0;
		display_minX = DISPLAY_MIN_X_FACTOR * 10; //initialize, will be updated with new preset
		// Display the splash logo
		display.drawBitmap(0, 0, epd_bitmap_Sparky_Logo, 128, 47, SSD1306_WHITE);
		display.setTextSize(2);
		display.setCursor(6, 48);
		display.print("Connecting");
	}
	else{
		showInitialMessage();
	}
	display.display();
}

void SparkDisplayControl::showInitialMessage(){
	// Clear the buffer
	display.clearDisplay(); //No Adafruit splash
	display.display();
	display.setTextColor(SSD1306_WHITE);
	display.setTextSize(2);
	display.setCursor(18,0);
	display.print("SparkBLE");
	display.setCursor(18,24);
	display.print("AMP mode");
	display.setTextSize(1);
	display.setCursor(24,44);
	display.print("Please connect");
	display.setCursor(36,56);
	display.print("Spark App");
}

void SparkDisplayControl::showMessage(std::string* msg, int numLines, int size, int x, int y ){

	display.clearDisplay();
	display.display();
	display.clearDisplay();

	display.setTextWrap(false);
	display.setTextSize(size);
	int distance = 24 * size;
	for (int i = 0; i < numLines; i++){
		Serial.printf("Setting cursor to %d", i*distance);
		display.setCursor(0, i * distance);
		display.print(msg[i].substr(0, 18).c_str());
	}
	display.display();
}

void SparkDisplayControl::update() {
	if ( spark_dc->operationMode() == SPARK_MODE_APP){
		int activeBank = spark_dc->activeBank();
		int pendingBank = spark_dc->pendingBank();
		preset* activePreset = spark_dc->activePreset();
		preset* pendingPreset = spark_dc->pendingPreset();
		int buttonMode = spark_dc->buttonMode();
		int activePresetNum = spark_dc->activePresetNum();

		std::string displayPresetName;
		preset* displayPreset;

		// Display the bank and preset number
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
			// If in FX mode, show an "M" for manual mode
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


		//Rectangle color for preset name
		int rectColor;
		int textColor;
		// Show preset name inverted if it is not the currently selected one
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

		// If bank is not HW preset bank and the currently selected bank
		// is not the active one, show the pending preset name
		if (pendingBank != 0 && activeBank != pendingBank) {
			displayPreset = pendingPreset;
			displayPresetName = displayPreset->name;
			// if the bank is the HW one and is not the active one
			// show only a generic name as we don't know the HW preset name upfront
		} else if (pendingBank == 0 && activeBank != pendingBank) {
			displayPreset = activePreset;
			displayPresetName = "Hardware Preset " + selPresetStr.str();
			// Otherwise just show the active preset name
		} else {
			displayPreset = activePreset;
			displayPresetName = displayPreset->name;
		}

		// Calculate how far the name can scroll left before bouncing
		display_minX = DISPLAY_MIN_X_FACTOR * displayPresetName.length()
											+ display.width();
		display.print(displayPresetName.c_str());

		// Build string to show active FX
		std::string fx_display = " ";
		std::string currPedalStatus;
		std::string fx_indicators_on[] = { "N", "C ", "D ", "  ", "M ", "D ", "R" }; // blank placeholder for Amp
		std::string fx_indicators_off[] = { " ", "  ", "  ", "  ", "  ", "  ", " " }; // blank placeholder for Amp

		// When we switched to FX mode, we always show the current selected preset
		if (buttonMode == SWITCH_MODE_FX) {
			displayPreset = activePreset;
		}
		if (!displayPreset->isEmpty || pendingBank > 0) {
			// Iterate through the corresponding preset's pedals and show indicators if switched on
			for (int i = 0; i < 7; i++) { // 7 pedals, amp to be ignored
				if (i != 3) {// Amp is on position 3, ignore
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

		// in FX mode (manual mode) invert display
		if (buttonMode == SWITCH_MODE_FX) {
			display.invertDisplay(true);
		} else {
			display.invertDisplay(false);
		}

		// If preset name is short, don't scroll
		if(displayPresetName.length()<=12){
			display_x = 0;
		}
		// long preset names are scrolled right to left and back
		else{
			display_x += display_scroll_num;
			if (display_x < display_minX || display_x > 1) {
				// the two ifs are required in case the preset name length is
				// longer than the previous one and has scrolled already too far
				if( display_x < display_minX){
					display_x=display_minX;
				}
				if(display_x > 1){
					display_x=1;
				}
				// Invert scrolling direction and scroll
				display_scroll_num = -display_scroll_num;
				display_x += display_scroll_num;
			}
		}
		display.display();
	} // IF MODE = APP
	else {
		if(!(spark_dc->isBLEClientConnected() && spark_dc->presetReceivedFromApp())){
			showInitialMessage();
		}
		else {
			const preset* currentPreset = spark_dc->appReceivedPreset();
			display.setTextColor(SSD1306_WHITE);
			display.setTextWrap(false);
			display.clearDisplay();
			display.setCursor(1,0);
			display.setTextSize(1);
			display.print("Received preset:");
			display.setCursor(1,24);
			display.print(currentPreset->name.c_str());
			display.display();

		}
	}
}
