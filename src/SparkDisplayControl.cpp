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
	//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
	spark_dc = dc;
	presetFromApp = nullptr;
	secondaryLinePreset = nullptr;
	primaryLinePreset = nullptr;
	pendingPreset = nullptr;
	activePreset = nullptr;
	presetEditMode = PRESET_EDIT_NONE;
}
SparkDisplayControl::~SparkDisplayControl() {
}

void SparkDisplayControl::init(int mode) {
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C required for this display
		Serial.println(F("SSD1306 initialization failed"));
		for (;;)
			; // Loop forever
	}
	initKeyboardLayoutStrings();
	// Clear the buffer
	display.clearDisplay(); //No Adafruit splash
	display.display();
	display.setTextColor(SSD1306_WHITE);
	display.setTextWrap(false);
	opMode = spark_dc->operationMode();

	showInitialMessage();
	display.display();
	if (opMode == SPARK_MODE_KEYBOARD) {
		//Allow the initial screen to show for some time
		delay(3000);
	}
}

void SparkDisplayControl::showInitialMessage() {
	display.drawBitmap(0, 0, epd_bitmap_Ignitron_Logo, 128, 47,
	SSD1306_WHITE);
	display.setTextSize(2);

	std::string modeText;
	switch(opMode){
	case SPARK_MODE_APP:
		modeText = "APP mode";
		break;
	case SPARK_MODE_AMP:
		modeText = "AMP mode";
		break;
	case SPARK_MODE_LOOPER:
		modeText = "LOOPER";
		break;
	case SPARK_MODE_KEYBOARD:
		modeText = "KEYBOARD";
		break;
	}

	drawCentreString(modeText.c_str(), 50);
}

void SparkDisplayControl::showBankAndPresetNum() {
	// Display the bank and preset number
	display.setTextColor(SSD1306_WHITE);
	//Configure numbers as strings
	std::ostringstream selBankStr;
	selBankStr << pendingBank;

	std::ostringstream selPresetStr;
	selPresetStr << activePresetNum;

	display.setCursor(display.width() - 48, 0);

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
	if (opMode == SPARK_MODE_LOOPER) {
		// If in Looper mode, show an "L" for Looper mode
		presetText = "L";

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

	display.setCursor(0, 0);
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
		// Reset scroll timer
		if (primaryLineText != previous_text1){
			text_row_1_timestamp = millis();
			previous_text1 = primaryLineText;
			display_x1 = 0;
		}
		//double the preset text for scrolling if longer than threshold
		// if text is too long and will be scrolled, double to "wrap around"
		if(primaryLineText.length() > text_scroll_limit){
			primaryLineText = primaryLineText + text_filler + primaryLineText;
		}
	}
	display.print(primaryLineText.c_str());

}
void SparkDisplayControl::showFX_SecondaryName() {
	// The last line shows either the FX setup (in APP mode)
	// or the new received preset from the app (in AMP mode)
	int secondaryLinePosY = 50;

	secondaryLineText = "";
	if (opMode == SPARK_MODE_AMP) {
		//displayPreset = presetFromApp;
		if (!(presetFromApp->isEmpty)) {
			secondaryLineText = presetFromApp->name;
			// Reset scroll timer
			if (secondaryLineText != previous_text2){
				text_row_2_timestamp = millis();
				previous_text2 = secondaryLineText;
				display_x2 = 0;
			}
		} else if (presetEditMode == PRESET_EDIT_DELETE) {
			secondaryLineText = "DELETE ?";
		} else {
			secondaryLineText = "Select preset";
		}
		// if text is too long and will be scrolled, double to "wrap around"
		if (secondaryLineText.length() > text_scroll_limit){
			secondaryLineText = secondaryLineText + text_filler + secondaryLineText;
		}
	} else if (opMode == SPARK_MODE_APP || opMode == SPARK_MODE_LOOPER) {
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
					Pedal currPedal = secondaryLinePreset->pedals[i];
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
	if (opMode == SPARK_MODE_APP || opMode == SPARK_MODE_LOOPER) {
		drawCentreString(secondaryLineText.c_str(), secondaryLinePosY);

	} else if (opMode == SPARK_MODE_AMP) {
		display.setCursor(display_x2, secondaryLinePosY);
		display.print(secondaryLineText.c_str());
	}
	

}

void SparkDisplayControl::showConnection() {
	// Display the Connection symbols
	int xPosSymbol = (display.width() / 2.0) - 10;
	int yPosSymbol = 9;
	int symbolWidth = 15;
	int symbolHeight = 17;

	int xPosText = xPosSymbol;
	int yPosText = 0;

	uint16_t color = SSD1306_WHITE;
	// Bluetooth
	switch (currentBTMode) {
	case BT_MODE_BLE:
		currentBTModeText = "BLE";
		break;
	case BT_MODE_SERIAL:
		currentBTModeText = "SRL";
		break;
	}
	display.setTextSize(1);
	display.setCursor(xPosText, yPosText);
	display.print(currentBTModeText.c_str());

	if (isBTConnected) {
		display.drawBitmap(xPosSymbol, yPosSymbol, epd_bitmap_bt_logo, symbolWidth, symbolHeight, color);
	}

}

void SparkDisplayControl::showPressedKey(){

	short int pressedButtonPosX = 20;
	short int pressedButtonPosY = 0;

	display.setTextColor(SSD1306_WHITE);
	display.setTextSize(4);

	display.setCursor(pressedButtonPosX, pressedButtonPosY);

	unsigned long currentMillis = millis();
	if (lastKeyboardButtonPressed != "" && showKeyboardPressedFlag == false) {
		keyboardPressedTimestamp = millis();
		showKeyboardPressedFlag = true;
	}
	if (showKeyboardPressedFlag) {
		if (currentMillis - keyboardPressedTimestamp >= showKeyboardPressedInterval) {
			// reset the show message flag to show preset data again
			showKeyboardPressedFlag = false;
			spark_dc->resetLastKeyboardButtonPressed();
			lastKeyboardButtonPressed = "";
		}
	}

	display.print(lastKeyboardButtonPressed.c_str());
}

void SparkDisplayControl::initKeyboardLayoutStrings(){

	std::string spacerText = "  ";

	lowerButtonsShort = mapping.keyboardShortPress[0]
										.append(spacerText)
										.append(mapping.keyboardShortPress[1])
										.append(spacerText)
										.append(mapping.keyboardShortPress[2])
										.append(spacerText)
										.append(mapping.keyboardShortPress[3]);

		upperButtonsShort = mapping.keyboardShortPress[4]
										.append(spacerText)
										.append(mapping.keyboardShortPress[5]);

		lowerButtonsLong = mapping.keyboardLongPress[0]
										.append(spacerText)
										.append(mapping.keyboardLongPress[1])
										.append(spacerText)
										.append(mapping.keyboardLongPress[2])
										.append(spacerText)
										.append(mapping.keyboardLongPress[3]);

		upperButtonsLong = mapping.keyboardLongPress[4]
										.append(spacerText)
										.append(mapping.keyboardLongPress[5]);

}

void SparkDisplayControl::showKeyboardLayout(){

	short int upperButtonsLongY = 1;
	short int upperButtonsShortY = 17;

	short int lowerButtonsLongY = 33;
	short int lowerButtonsShortY = 50;

	short int upperPositionOffset = -4;

	short int displayMid = display.width()/2;

	display.fillRect(0, 48, 128, 16, SSD1306_BLACK);

	//Rectangle color for preset name
	int rectColor;
	int textColor;
	int textColorInv;
	// Show preset name inverted if it is not the currently selected one
	rectColor = SSD1306_WHITE;
	textColorInv = SSD1306_BLACK;
	textColor = SSD1306_WHITE;

	display.setTextColor(textColor);
	display.setTextSize(2);

	drawCentreString(lowerButtonsShort.c_str(), lowerButtonsShortY);
	drawRightAlignedString(upperButtonsShort.c_str(), upperButtonsShortY, upperPositionOffset);

	display.setTextColor(textColorInv);
	display.fillRect(0, lowerButtonsLongY-1, 128, 16, rectColor);
	drawCentreString(lowerButtonsLong.c_str(), lowerButtonsLongY);

	display.fillRect(displayMid-upperPositionOffset, 0, displayMid+upperPositionOffset, 16, rectColor);
	drawRightAlignedString(upperButtonsLong.c_str(), upperButtonsLongY, upperPositionOffset);

}

void SparkDisplayControl::update(bool isInitBoot) {

	display.clearDisplay();
	if ((opMode == SPARK_MODE_APP || opMode == SPARK_MODE_LOOPER) && isInitBoot) {
		showInitialMessage();
	} else if (opMode == SPARK_MODE_KEYBOARD) {
		lastKeyboardButtonPressed = spark_dc->lastKeyboardButtonPressed();
		showPressedKey();
		showKeyboardLayout();
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
		isBTConnected = spark_dc->isAmpConnected() || spark_dc->isAppConnected();
		opMode = spark_dc->operationMode();
		currentBTMode = spark_dc->currentBTMode();

		showConnection();
		showBankAndPresetNum();
		updateTextPositions();
		showPresetName();
		showFX_SecondaryName();

		// in FX mode (manual mode) invert display
		if (buttonMode == SWITCH_MODE_FX) {
			display.invertDisplay(true);
		} else {
			display.invertDisplay(false);
		}

	}
	display.display();
}

void SparkDisplayControl::updateTextPositions() {
	//This is for the primary preset name line
	display_minX1 = DISPLAY_MIN_X_FACTOR * ((primaryLineText.length() - text_filler.length())/2.0 + text_filler.length());
	if (primaryLineText.length() <= text_scroll_limit) {
		display_x1 = 0;
	}
	// long preset names are scrolled right to left and back
	else {
		if (millis() - text_row_1_timestamp > text_scroll_delay){
			display_x1 -= display_scroll_num1;
			if (display_x1 < display_minX1) {
				// the two ifs are required in case the preset name length is
				// longer than the previous one and has scrolled already too far
				if (display_x1 < display_minX1) {
					display_x1 = display_minX1;
				}
				// Reset text
				display_x1 = 0;
				text_row_1_timestamp = millis();
			}
		}
	}

	// This is for the secondary FX display line (show preset name in AMP mode)
	display_minX2 = DISPLAY_MIN_X_FACTOR * ((secondaryLineText.length() - text_filler.length())/2.0 + text_filler.length());
	if (secondaryLineText.length() <= text_scroll_limit) {
		display_x2 = 0;
	} else {
		if (millis() - text_row_2_timestamp > text_scroll_delay){
			display_x2 -= display_scroll_num2;
			if (display_x2 < display_minX2) {
				// the two ifs are required in case the preset name length is
				// longer than the previous one and has scrolled already too far
				if (display_x2 < display_minX2) {
					display_x2 = display_minX2;
				}
				// Reset text
				display_x2 = 0;
				text_row_2_timestamp = millis();
			}
		}
	}
}

void SparkDisplayControl::drawCentreString(const char *buf,
		int y, int offset) {
	int16_t x1, y1;
	uint16_t w, h;
	int displayMid = display.width()/2;

	display.getTextBounds(buf, displayMid, y, &x1, &y1, &w, &h); //calc width of new string
	display.setCursor(displayMid - w / 2 + offset, y);
	display.print(buf);
}

void SparkDisplayControl::drawRightAlignedString(const char *buf,
		int y, int offset) {
	int16_t x1, y1;
	uint16_t w, h;
	int x = 0;
	int displayWidth = display.width();

	display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
	display.setCursor(displayWidth - w + offset, y);
	display.print(buf);
}

void SparkDisplayControl::drawInvertBitmapColor(int16_t x, int16_t y,
		const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {

	int16_t i, j, byteWidth = (w + 7) / 8;

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			if ((pgm_read_byte(bitmap + j * byteWidth + i / 8)
					& (128 >> (i & 7))) == 0) {
				display.drawPixel(x + i, y + j, color);
			}
		}
	}
}
