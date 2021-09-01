/*
 * SparkDisplayControl.cpp
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#include "SparkDisplayControl.h"

Adafruit_SSD1306 SparkDisplayControl::display(SCREEN_WIDTH, SCREEN_HEIGHT,
		&Wire, OLED_RESET);

SparkDisplayControl::SparkDisplayControl() :
		SparkDisplayControl(nullptr) {

}

SparkDisplayControl::SparkDisplayControl(SparkDataControl *dc) {
	Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
	spark_dc = dc;
	presetFromApp = nullptr;
	secondaryLinePreset = nullptr;
	primaryLinePreset = nullptr;
	pendingPreset = nullptr;
	activePreset = nullptr;
	presetEditMode = PRESET_EDIT_NONE;
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
	display.setTextWrap(false);
	opMode = spark_dc->operationMode();

	showInitialMessage();
	display.display();
	if (opMode == SPARK_MODE_AMP) {
		// Allow the initial screen to show for some time
		delay(5000);
	}
}

void SparkDisplayControl::showInitialMessage() {
	display.drawBitmap(0, 0, epd_bitmap_Sparky_Logo, 128, 47,
	SSD1306_WHITE);
	display.setTextSize(2);

	std::string modeText;
	if (opMode == SPARK_MODE_APP) {
		modeText = "APP mode";
	} else if (opMode == SPARK_MODE_AMP) {
		modeText = "AMP mode";
	}
	drawCentreString(modeText.c_str(), display.width() / 2, 49);
}

void SparkDisplayControl::showBankAndPresetNum() {
	// Display the bank and preset number
	display.setTextColor(SSD1306_WHITE);
	//Configure numbers as strings
	std::ostringstream selBankStr;
	selBankStr << pendingBank;

	std::ostringstream selPresetStr;
	selPresetStr << activePresetNum;

	display.setCursor(display.width() - 48, 1);

	// Preset display
	display.setTextSize(4);
	std::string presetText = " ";
	if (opMode == SPARK_MODE_APP && buttonMode == SWITCH_MODE_FX) {
		// If in FX mode, show an "M" for manual mode
		presetText = "M";

	}
	if (opMode == SPARK_MODE_AMP && presetEditMode != PRESET_EDIT_NONE) {
		presetText = "*";
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

}

void SparkDisplayControl::showPresetName() {

	const std::string msg = spark_dc->responseMsg();
	std::ostringstream selPresetStr;
	selPresetStr << activePresetNum;

	//Rectangle color for preset name
	int rectColor;
	int textColor;
	// Show preset name inverted if it is not the currently selected one
	if (pendingBank == activeBank && presetEditMode != PRESET_EDIT_DELETE) {
		rectColor = SSD1306_BLACK;
		textColor = SSD1306_WHITE;
	} else {
		rectColor = SSD1306_WHITE;
		textColor = SSD1306_BLACK;
	}

	display.setTextColor(textColor);
	display.fillRect(0, 31, 128, 16, rectColor);

	display.setTextSize(2);
	unsigned long currentMillis = millis();

	if (msg != "") { // message to show for some time
		previousMillis = millis();
		primaryLineText = msg;
		spark_dc->resetPresetEditResponse();
		showMsgFlag = true;
	}
	if (showMsgFlag) {
		display.setCursor(0, 32);
		if (currentMillis - previousMillis >= showMessageInterval) {
			// reset the show message flag to show preset data again
			showMsgFlag = false;

		}
	} else { // no preset save message to display
		display.setCursor(display_x1, 32);
		// If bank is not HW preset bank and the currently selected bank
		// is not the active one, show the pending preset name
		if (pendingBank != 0 && activeBank != pendingBank) {
			primaryLinePreset = pendingPreset;
			primaryLineText = primaryLinePreset->name;
			// if the bank is the HW one and is not the active one
			// show only a generic name as we don't know the HW preset name upfront
		} else if (pendingBank == 0 && activeBank != pendingBank) {
			primaryLinePreset = activePreset;
			primaryLineText = "Hardware Preset " + selPresetStr.str();
			// Otherwise just show the active preset name
		} else {
			primaryLinePreset = activePreset;
			primaryLineText = primaryLinePreset->name;
		}
	}
	display.print(primaryLineText.c_str());

}
void SparkDisplayControl::showFX_SecondaryName() {
	// The last line shows either the FX setup (in APP mode)
	// or the new received preset from the app (in AMP mode)
	secondaryLineText = "";
	if (opMode == SPARK_MODE_AMP) {
		//displayPreset = presetFromApp;
		if (!(presetFromApp->isEmpty)) {
			secondaryLineText = presetFromApp->name;
		} else if (presetEditMode == PRESET_EDIT_DELETE) {
			secondaryLineText = "DELETE ?";
		} else {
			secondaryLineText = "Select preset";
		}
	} else if (opMode == SPARK_MODE_APP) {
		// Build string to show active FX
		secondaryLinePreset = primaryLinePreset;
		std::string currPedalStatus;
		// blank placeholder for Amp
		std::string fx_indicators_on[] =
				{ "N", "C ", "D ", " ", "M ", "D ",
				"R" };
		std::string fx_indicators_off[] = { " ", "  ", "  ", " ", "  ", "  ",
				" " };

		// When we switched to FX mode, we always show the current selected preset
		if (buttonMode == SWITCH_MODE_FX) {
			secondaryLinePreset = activePreset;
		}
		if (!(secondaryLinePreset->isEmpty) || pendingBank > 0) {
			// Iterate through the corresponding preset's pedals and show indicators if switched on
			for (int i = 0; i < 7; i++) { // 7 pedals, amp to be ignored
				if (i != 3) { // Amp is on position 3, ignore
					pedal currPedal = secondaryLinePreset->pedals[i];
					currPedalStatus =
							secondaryLinePreset->pedals[i].isOn ?
									fx_indicators_on[i] : fx_indicators_off[i];
					secondaryLineText += currPedalStatus;
				}
			}
		} else {
			secondaryLineText = "";
		}
	}
	display.fillRect(0, 48, 128, 16, SSD1306_BLACK);

	display.setTextColor(SSD1306_WHITE);
	display.setTextSize(2);
	if (opMode == SPARK_MODE_APP) {
		drawCentreString(secondaryLineText.c_str(), display.width() / 2, 49);
		//display.setCursor(-6, 49);
	} else {
		display.setCursor(display_x2, 49);
		display.print(secondaryLineText.c_str());
	}
	

}

void SparkDisplayControl::showConnection() {
	// Display the bank and preset number
	int xPos = display.width() / 2.0;
	int yPos = 15;
	int radius = 4;
	uint16_t color = SSD1306_WHITE;
	if (isConnected) {
		display.fillCircle(xPos, yPos, radius, color);
	} else {
		display.drawCircle(xPos, yPos, radius, color);
	}

}

void SparkDisplayControl::update(bool isInitBoot) {

	display.clearDisplay();
	if (opMode == SPARK_MODE_APP && isInitBoot) {
		showInitialMessage();
	} else {
		display.setTextWrap(false);
		activeBank = spark_dc->activeBank();
		pendingBank = spark_dc->pendingBank();
		activePreset = spark_dc->activePreset();
		pendingPreset = spark_dc->pendingPreset();
		buttonMode = spark_dc->buttonMode();
		activePresetNum = spark_dc->activePresetNum();
		presetFromApp = spark_dc->appReceivedPreset();
		presetEditMode = spark_dc->presetEditMode();
		isConnected = spark_dc->isAmpConnected() || spark_dc->isAppConnected();

		showConnection();
		showBankAndPresetNum();
		showPresetName();
		showFX_SecondaryName();

		// in FX mode (manual mode) invert display
		if (buttonMode == SWITCH_MODE_FX) {
			display.invertDisplay(true);
		} else {
			display.invertDisplay(false);
		}
		updateTextPositions();
	}
	display.display();
}

void SparkDisplayControl::updateTextPositions() {
	//This is for the primary preset name line
	display_minX1 = DISPLAY_MIN_X_FACTOR * primaryLineText.length()
			+ display.width();
	if (primaryLineText.length() <= 12) {
		display_x1 = 0;
	}
	// long preset names are scrolled right to left and back
	else {
		display_x1 += display_scroll_num1;
		if (display_x1 < display_minX1 || display_x1 > 1) {
			// the two ifs are required in case the preset name length is
			// longer than the previous one and has scrolled already too far
			if (display_x1 < display_minX1) {
				display_x1 = display_minX1;
			}
			if (display_x1 > 1) {
				display_x1 = 1;
			}
			// Invert scrolling direction and scroll
			display_scroll_num1 = -display_scroll_num1;
			display_x1 += display_scroll_num1;
		}
	}

	// This is for the secondary FX display line (show preset name in AMP mode)
	display_minX2 = DISPLAY_MIN_X_FACTOR * secondaryLineText.length()
			+ display.width();
	if (secondaryLineText.length() <= 12) {
		display_x2 = 0;
	} else {
		display_x2 += display_scroll_num2;
		if (display_x2 < display_minX2 || display_x2 > 1) {
			// the two ifs are required in case the preset name length is
			// longer than the previous one and has scrolled already too far
			if (display_x2 < display_minX2) {
				display_x2 = display_minX2;
			}
			if (display_x2 > 1) {
				display_x2 = 1;
			}
			// Invert scrolling direction and scroll
			display_scroll_num2 = -display_scroll_num2;
			display_x2 += display_scroll_num2;
		}
	}
}

void SparkDisplayControl::drawCentreString(const char *buf, int x,
		int y) {
	int16_t x1, y1;
	uint16_t w, h;
	display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
	display.setCursor(x - w / 2, y);
	display.print(buf);
}
