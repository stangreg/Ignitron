/*
 * SparkDisplayControl.cpp
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#include "SparkDisplayControl.h"

Adafruit_SSD1306 SparkDisplayControl::display(SCREEN_WIDTH, SCREEN_HEIGHT,
                                              &Wire, OLED_RESET);

SparkDisplayControl::SparkDisplayControl() : SparkDisplayControl(nullptr) {
}

SparkDisplayControl::SparkDisplayControl(SparkDataControl *dc) {
    // Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    spark_dc = dc;
    sparkLooperControl = nullptr;
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
    display.clearDisplay(); // No Adafruit splash
    display.display();
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);
    opMode = spark_dc->operationMode();
    sparkLooperControl = spark_dc->looperControl();

    showInitialMessage();
    display.display();
    if (opMode == SPARK_MODE_KEYBOARD) {
        // Allow the initial screen to show for some time
        delay(2000);
    }
}

void SparkDisplayControl::showInitialMessage() {
    display.drawBitmap(0, 0, epd_bitmap_Ignitron_Logo, 128, 47,
                       SSD1306_WHITE);
    display.setTextSize(2);

    string modeText;
    switch (opMode) {
    case SPARK_MODE_APP:
        modeText = "APP";
        break;
    case SPARK_MODE_AMP:
        modeText = "AMP";
        break;
    case SPARK_MODE_LOOPER:
    case SPARK_MODE_SPK_LOOPER:
        modeText = "LPR";
        break;
    case SPARK_MODE_KEYBOARD:
        modeText = "KB";
        break;
    }
    modeText += " v" + VERSION;

    drawCentreString(modeText.c_str(), 50);
}

void SparkDisplayControl::showBankAndPresetNum() {

    // Display the bank and preset number
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(4);

    // Configure numbers as strings
    ostringstream selBankStr;
    selBankStr << pendingBank;

    ostringstream selPresetStr;
    selPresetStr << activePresetNum;

    // Bank number display
    string bankDisplay = "";
    if (pendingBank < 10) {
        bankDisplay = "0";
    }
    bankDisplay += selBankStr.str();
    if (pendingBank == 0) {
        bankDisplay = "HW";
    }

    display.setCursor(0, 0);
    display.print(bankDisplay.c_str());

    // Preset display
    display.setCursor(display.width() - 24, 0);
    string presetText = "";
    presetText = selPresetStr.str();
    display.print(presetText.c_str());
}

void SparkDisplayControl::showPresetName() {

    const string msg = SparkPresetControl::getInstance().responseMsg();

    // Rectangle color for preset name
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
        SparkPresetControl::getInstance().resetPresetEditResponse();
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
        if (activeBank != pendingBank) {
            primaryLinePreset = pendingPreset;
        } else {
            primaryLinePreset = activePreset;
        }
        primaryLineText = primaryLinePreset.name;

        // Reset scroll timer
        if (primaryLineText != previous_text1) {
            text_row_1_timestamp = millis();
            previous_text1 = primaryLineText;
            display_x1 = 0;
        }
        // double the preset text for scrolling if longer than threshold
        //  if text is too long and will be scrolled, double to "wrap around"
        if (primaryLineText.length() > text_scroll_limit) {
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
        // displayPreset = presetFromApp;
        if (!(presetFromApp.isEmpty)) {
            secondaryLineText = presetFromApp.name;
            // Reset scroll timer
            if (secondaryLineText != previous_text2) {
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
        if (secondaryLineText.length() > text_scroll_limit) {
            secondaryLineText = secondaryLineText + text_filler + secondaryLineText;
        }
    } else if (opMode == SPARK_MODE_APP || opMode == SPARK_MODE_LOOPER || opMode == SPARK_MODE_SPK_LOOPER) {
        // Build string to show active FX
        secondaryLinePreset = primaryLinePreset;

        // When we switched to FX mode, we always show the current selected preset
        if (buttonMode == BUTTON_MODE_FX) {
            secondaryLinePreset = activePreset;
        }

        if (!(secondaryLinePreset.isEmpty) || pendingBank > 0) {
            // Iterate through the corresponding preset's pedals and show indicators if switched on
            for (int i = 0; i < 7; i++) { // 7 pedals, amp to be ignored
                if (i != 3) {             // Amp is on position 3, ignore
                    // blank placeholder for Amp
                    string fx_indicators_on[] =
                        {"N", "C ", "D ", " ", "M ", "D ", "R"};
                    string fx_indicators_off[] = {" ", "  ", "  ", " ", "  ", "  ", " "};

                    string currPedalStatus;
                    Pedal currPedal = secondaryLinePreset.pedals[i];
                    currPedalStatus =
                        currPedal.isOn ? fx_indicators_on[i] : fx_indicators_off[i];
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
    if (opMode == SPARK_MODE_APP || opMode == SPARK_MODE_LOOPER || opMode == SPARK_MODE_SPK_LOOPER) {
        drawCentreString(secondaryLineText.c_str(), secondaryLinePosY);

    } else if (opMode == SPARK_MODE_AMP) {
        display.setCursor(display_x2, secondaryLinePosY);
        display.print(secondaryLineText.c_str());
    }
}

void SparkDisplayControl::showLooperTimer() {
    int currentBeat = sparkLooperControl->currentBeat();
    int currentBar = sparkLooperControl->currentBar();
    int totalBars = sparkLooperControl->totalBars();
    int bpm = sparkLooperControl->bpm();

    display.setTextSize(4);
    display.setCursor(0, 0);
    display.print(currentBar);
    display.setCursor(display.width() - 24, 0);
    display.print(currentBeat);

    // Progress bar
    int barHeight = 16;
    int charWidth = 12;
    int xPos = 0;
    int xOffset = 16;
    int yPos = 47;
    // Reserve 2 characters for totalBars, 3 for BPM
    int maxBarWidth = display.width() - 2 * charWidth;
    int barWidth = (double)(currentBar)*maxBarWidth / totalBars;
    int drawColor = SSD1306_WHITE;
    display.fillRect(xPos, yPos, barWidth, barHeight, drawColor);

    // Show BPM
    int bpmXPos = display.width() - 60;
    int bpmYPos = 17;
    display.setCursor(bpmXPos, bpmYPos);
    display.setTextSize(2);
    display.print(bpm);

    // Show total bars
    display.setCursor(maxBarWidth + 2, yPos);
    display.setTextSize(2);
    display.print(totalBars);

    // lines to separate the bars
    int stepSize = 1;
    if (totalBars > 25) {
        stepSize = 2;
    }
    for (int i = 0; i < totalBars; i = i + stepSize) {
        int xPos = (double)i * maxBarWidth / totalBars;
        drawColor = xPos < barWidth ? SSD1306_BLACK : SSD1306_WHITE;
        display.drawFastVLine(xPos, yPos, barHeight, drawColor);
    }
}

void SparkDisplayControl::showModeModifier() {

    display.setCursor(display.width() - 48, 0);

    // Preset display
    display.setTextSize(4);
    string presetText = " ";

    if (opMode == SPARK_MODE_APP && buttonMode == BUTTON_MODE_FX) {
        // If in FX mode, show an "M" for manual mode
        presetText = "M";
    }
    if (opMode == SPARK_MODE_AMP && presetEditMode != PRESET_EDIT_NONE) {
        presetText = "*";
    }
    // Spark 2 built-in Looper
    if (opMode == SPARK_MODE_SPK_LOOPER) {
        // If in Looper mode, show an "L" for Looper mode
        display.setTextSize(2);
        if (buttonMode == BUTTON_MODE_LOOP_CONTROL) {
            presetText = "L";
        } else if (buttonMode == BUTTON_MODE_LOOP_CONFIG) {
            presetText = "C";
        }
    }
    // Looper app
    if (opMode == SPARK_MODE_LOOPER) {
        // If in Looper mode, show an "L" for Looper mode
        presetText = "L";
    }
    display.print(presetText.c_str());
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

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
    // If enable battery SOC, move the BT symbol to the left
    xPosSymbol -= 5;
#endif

    if (isBTConnected) {
        display.drawBitmap(xPosSymbol, yPosSymbol, epd_bitmap_bt_logo, symbolWidth, symbolHeight, color);
    }
}

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
// Show battery status indicator with a voltage divider
// 0-4095 for 0v-3.3v
void SparkDisplayControl::showBatterySymbol() {
    // Display the Battery symbols
    int xPosSymbol = (display.width() / 2.0);
    int yPosSymbol = 11;
    int symbolWidth = 9;
    int symbolHeight = 15;

    uint16_t color = SSD1306_WHITE;
    const unsigned char *battery_icon;
    switch (batteryLevel) {
    case BATTERY_LEVEL_0:
        battery_icon = epd_bitmap_battery_level_0;
        break;
    case BATTERY_LEVEL_1:
        battery_icon = epd_bitmap_battery_level_1;
        break;
    case BATTERY_LEVEL_2:
        battery_icon = epd_bitmap_battery_level_2;
        break;
    case BATTERY_LEVEL_3:
    default:
        battery_icon = epd_bitmap_battery_level_3;
        break;
    }

    display.drawBitmap(xPosSymbol, yPosSymbol, battery_icon, symbolWidth, symbolHeight, color);
}
#endif

void SparkDisplayControl::showPressedKey() {

    short int pressedButtonPosX = 20;
    short int pressedButtonPosY = 0;

    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(4);

    display.setCursor(pressedButtonPosX, pressedButtonPosY);

    unsigned long currentMillis = millis();
    if (lastKeyboardButtonPressedString != "" && showKeyboardPressedFlag == false) {
        keyboardPressedTimestamp = millis();
        showKeyboardPressedFlag = true;
    }
    if (showKeyboardPressedFlag) {
        if (currentMillis - keyboardPressedTimestamp >= showKeyboardPressedInterval) {
            // reset the show message flag to show preset data again
            showKeyboardPressedFlag = false;
            spark_dc->resetLastKeyboardButtonPressed();
            lastKeyboardButtonPressedString = "";
        }
    }

    display.print(lastKeyboardButtonPressedString.c_str());
}

void SparkDisplayControl::initKeyboardLayoutStrings() {

    currentKeyboard = spark_dc->currentKeyboard();
    string spacerText = "  ";

    lowerButtonsShort = currentKeyboard.keyboardShortPress[0].display.append(spacerText).append(currentKeyboard.keyboardShortPress[1].display).append(spacerText).append(currentKeyboard.keyboardShortPress[2].display).append(spacerText).append(currentKeyboard.keyboardShortPress[3].display);

    upperButtonsShort = currentKeyboard.keyboardShortPress[4].display.append(spacerText).append(currentKeyboard.keyboardShortPress[5].display);

    lowerButtonsLong = currentKeyboard.keyboardLongPress[0].display.append(spacerText).append(currentKeyboard.keyboardLongPress[1].display).append(spacerText).append(currentKeyboard.keyboardLongPress[2].display).append(spacerText).append(currentKeyboard.keyboardLongPress[3].display);

    upperButtonsLong = currentKeyboard.keyboardLongPress[4].display
                           //.append(spacerText)
                           .append(currentKeyboard.keyboardLongPress[5].display);
}

void SparkDisplayControl::showKeyboardLayout() {

    currentKeyboard = spark_dc->currentKeyboard();

    short int upperButtonsLongY = 1;
    short int upperButtonsShortY = 17;

    short int lowerButtonsLongY = 33;
    short int lowerButtonsShortY = 50;

    short int upperPositionOffset = -4;

    short int displayMid = display.width() / 2;

    display.fillRect(0, 48, 128, 16, SSD1306_BLACK);

    // Rectangle color for preset name
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
    display.fillRect(0, lowerButtonsLongY - 1, 128, 16, rectColor);
    drawCentreString(lowerButtonsLong.c_str(), lowerButtonsLongY);

    display.fillRect(displayMid - upperPositionOffset, 0, displayMid + upperPositionOffset, 16, rectColor);
    drawRightAlignedString(upperButtonsLong.c_str(), upperButtonsLongY, upperPositionOffset);
}

void SparkDisplayControl::showTunerNote() {

    display.setTextColor(SSD1306_WHITE);

    // text sizes: 1 = 6x8, 2= 12x16, 3=18x24, 4=24x36
    char sharpSymbol = currentNote.at(1);
    display.setTextSize(3);

    int displayMidY = display.height() / 2.0;
    int displayMidX = display.width() / 2.0;
    int notePosX = displayMidX - 6;
    if (sharpSymbol == '#') {
        notePosX = displayMidX - 12;
    }
    display.setCursor(notePosX, displayMidY - 12);
    char baseNote = currentNote.at(0);
    display.print(baseNote);

    if (sharpSymbol == '#') {
        display.setTextSize(2);
        display.setCursor(displayMidX + 8, displayMidY - 8);
        display.print(sharpSymbol);
    }
}

void SparkDisplayControl::showTunerOffset() {

    int tunerPosX = 0;
    int tunerPosY = 0;

    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(tunerPosX, tunerPosY);
    if (noteOffsetCents >= -50 && noteOffsetCents <= 50) {
        display.print(to_string(noteOffsetCents).c_str());
    }
}

void SparkDisplayControl::showTunerGraphic() {

    int triangleSize = 12;
    int color = SSD1306_WHITE;
    int triPosHigher3X = 0;
    int triPosHigher2X = 15;
    int triPosHigher1X = 30;
    int triPosLower1X = 98;
    int triPosLower2X = 113;
    int triPosLower3X = 128;

    int centsTolerance = 5;

    // draw triangles to point to the note
    if (noteOffsetCents > -50 && noteOffsetCents < -35) {
        drawTunerTriangleCentre(triPosHigher3X, triangleSize, true, color);
    }
    if (noteOffsetCents > -50 && noteOffsetCents < -20) {
        drawTunerTriangleCentre(triPosHigher2X, triangleSize, true, color);
    }
    if (noteOffsetCents > -50 && noteOffsetCents < centsTolerance) {
        drawTunerTriangleCentre(triPosHigher1X, triangleSize, true, color);
    }

    if (noteOffsetCents > -centsTolerance) {
        drawTunerTriangleCentre(triPosLower1X, triangleSize, false, color);
    }
    if (noteOffsetCents > 20) {
        drawTunerTriangleCentre(triPosLower2X, triangleSize, false, color);
    }
    if (noteOffsetCents > 35) {
        drawTunerTriangleCentre(triPosLower3X, triangleSize, false, color);
    }
    // Draw circle if in tune
    if (-centsTolerance <= noteOffsetCents && noteOffsetCents <= centsTolerance) {
        display.drawCircle(display.width() / 2.0, display.height() / 2.0, 20, color);
        display.invertDisplay(true);
    } else {
        display.invertDisplay(false);
    }
}

void SparkDisplayControl::update(bool isInitBoot) {

    opMode = spark_dc->operationMode();
    display.clearDisplay();
    if ((opMode == SPARK_MODE_APP || opMode == SPARK_MODE_LOOPER || opMode == SPARK_MODE_SPK_LOOPER) && isInitBoot) {
        showInitialMessage();
    } else if (opMode == SPARK_MODE_KEYBOARD) {
        if (spark_dc->keyboardChanged()) {
            initKeyboardLayoutStrings();
            spark_dc->resetKeyboardChangeIndicator();
        }
        lastKeyboardButtonPressedString = spark_dc->lastKeyboardButtonPressedString();
        showPressedKey();
        showKeyboardLayout();
    } else if (opMode == SPARK_MODE_TUNER) {
        SparkStatus &statusObject = SparkStatus::getInstance();
        currentNote = statusObject.noteString();
        noteOffsetCents = statusObject.note_offset_cents();

        showTunerNote();
        showTunerOffset();
        showTunerGraphic();

    } else {
        SparkPresetControl &presetControl = SparkPresetControl::getInstance();
        display.setTextWrap(false);
        activeBank = presetControl.activeBank();
        pendingBank = presetControl.pendingBank();
        activePreset = presetControl.activePreset();
        pendingPreset = presetControl.pendingPreset();
        buttonMode = spark_dc->buttonMode();
        activePresetNum = presetControl.activePresetNum();
        presetFromApp = presetControl.appReceivedPreset();
        presetEditMode = presetControl.presetEditMode();
        isBTConnected = spark_dc->isAmpConnected() || spark_dc->isAppConnected();
        currentBTMode = spark_dc->currentBTMode();
        sparkLooperControl = spark_dc->looperControl();

        showConnection();
#ifdef ENABLE_BATTERY_STATUS_INDICATOR
        batteryLevel = spark_dc->batteryLevel();
        showBatterySymbol();
#endif
        showModeModifier();
        updateTextPositions();
        showPresetName();
        if (opMode == SPARK_MODE_SPK_LOOPER) {
            showLooperTimer();
        } else {
            showBankAndPresetNum();
            showFX_SecondaryName();

            // in FX mode (manual mode) invert display
            if (buttonMode == BUTTON_MODE_FX) {
                display.invertDisplay(true);
            } else {
                display.invertDisplay(false);
            }
        }
    }
    // logDisplay();
    display.display();
}

void SparkDisplayControl::updateTextPositions() {
    // This is for the primary preset name line
    display_minX1 = DISPLAY_MIN_X_FACTOR * ((primaryLineText.length() - text_filler.length()) / 2.0 + text_filler.length());
    if (primaryLineText.length() <= text_scroll_limit) {
        display_x1 = 0;
    }
    // long preset names are scrolled right to left and back
    else {
        if (millis() - text_row_1_timestamp > text_scroll_delay) {
            display_x1 -= display_scroll_num1;
            if (display_x1 < display_minX1) {
                // Reset text
                display_x1 = 0;
                text_row_1_timestamp = millis();
            }
        }
    }

    // This is for the secondary FX display line (show preset name in AMP mode)
    display_minX2 = DISPLAY_MIN_X_FACTOR * ((secondaryLineText.length() - text_filler.length()) / 2.0 + text_filler.length());
    if (secondaryLineText.length() <= text_scroll_limit) {
        display_x2 = 0;
    } else {
        if (millis() - text_row_2_timestamp > text_scroll_delay) {
            display_x2 -= display_scroll_num2;
            if (display_x2 < display_minX2) {
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
    int displayMid = display.width() / 2;

    display.getTextBounds(buf, displayMid, y, &x1, &y1, &w, &h); // calc width of new string
    display.setCursor(displayMid - w / 2 + offset, y);
    display.print(buf);
}

void SparkDisplayControl::drawRightAlignedString(const char *buf,
                                                 int y, int offset) {
    int16_t x1, y1;
    uint16_t w, h;
    int x = 0;
    int displayWidth = display.width();

    display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); // calc width of new string
    display.setCursor(displayWidth - w + offset, y);
    display.print(buf);
}

void SparkDisplayControl::drawTunerTriangleCentre(int x, int size, bool dir, int color) {

    int displayMidY = display.height() / 2.0;
    int yPos1 = displayMidY - size / 2.0;
    int xPos1 = x;

    int yPos2 = displayMidY + size / 2.0;
    int xPos2 = x;

    int yPos3 = displayMidY;
    int xPos3;
    if (dir) {
        xPos3 = x + sqrt(0.75 * size * size);
    } else {
        xPos3 = x - sqrt(0.75 * size * size);
    }

    display.fillTriangle(xPos1, yPos1, xPos2, yPos2, xPos3, yPos3, color);
}

void SparkDisplayControl::drawInvertBitmapColor(int16_t x, int16_t y,
                                                const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {

    int16_t i, j, byteWidth = (w + 7) / 8;

    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            if ((pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) == 0) {
                display.drawPixel(x + i, y + j, color);
            }
        }
    }
}

void SparkDisplayControl::logDisplay() {
    if (millis() - lastLogTimestamp > logInterval) {
        DEBUG_PRINTLN("Display status:");
        DEBUG_PRINTLN("Current Preset settings:");
        DEBUG_PRINTF("Primary line   : %s\n", primaryLineText.c_str());
        DEBUG_PRINTF("Secondary line : %s\n", secondaryLineText.c_str());
        if (!(activePreset.isEmpty) && !(pendingPreset.isEmpty)) {
            DEBUG_PRINTF("Act Preset empty? : %s\n", activePreset.isEmpty ? "true" : "false");
            DEBUG_PRINTF("Pen Preset empty? : %s\n", pendingPreset.isEmpty ? "true" : "false");
        }
        lastLogTimestamp = millis();
    }
}
