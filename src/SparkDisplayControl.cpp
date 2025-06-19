/*
 * SparkDisplayControl.cpp
 *
 *  Created on: 23.08.2021
 *      Author: stangreg
 */

#include "SparkDisplayControl.h"

const int SparkDisplayControl::SCREEN_WIDTH = 128;         // Display width
const int SparkDisplayControl::SCREEN_HEIGHT = 64;         // Display height
const int SparkDisplayControl::OLED_RESET = -1;            // Reset pin # (or -1 if sharing Arduino reset pin)
const int SparkDisplayControl::DISPLAY_MIN_X_FACTOR = -12; // for text size 2, scales linearly with text size

#if defined(OLED_DRIVER_SSD1306)
Adafruit_SSD1306 SparkDisplayControl::display_(SCREEN_WIDTH, SCREEN_HEIGHT,
                                               &Wire, OLED_RESET);
SparkDisplayControl::SparkDisplayControl() : SparkDisplayControl(nullptr) {
}
#elif defined(OLED_DRIVER_SH1106)
Adafruit_SH1106G SparkDisplayControl::display_(SCREEN_WIDTH, SCREEN_HEIGHT,
                                               &Wire, OLED_RESET);
SparkDisplayControl::SparkDisplayControl() : SparkDisplayControl(nullptr) {
}
#elif defined(OLED_DRIVER_SH1107)
Adafruit_SH1107 SparkDisplayControl::display_(SCREEN_HEIGHT, SCREEN_WIDTH,
                                              &Wire, OLED_RESET);
SparkDisplayControl::SparkDisplayControl() : SparkDisplayControl(nullptr) {
}
#endif

SparkDisplayControl::SparkDisplayControl(SparkDataControl *dc) {
    // Adafruit_SSD1306 display_(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    sparkDC_ = dc;
}

SparkDisplayControl::~SparkDisplayControl() {
}

void SparkDisplayControl::init(int mode) {
#if defined(OLED_DRIVER_SSD1306)
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display_.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C required for this display
        Serial.println(F("SSD1306 initialization failed"));
        for (;;)
            ; // Loop forever
    }
#elif defined(OLED_DRIVER_SH1106)
    if (!display_.begin(0x3C, true)) { // 0x3C required for this display
        Serial.println(F("SH1106 initialization failed"));
        for (;;)
            ; // Loop forever
    }
#elif defined(OLED_DRIVER_SH1107)
    if (!display_.begin(0x3C, true)) { // 0x3C required for this display
        Serial.println(F("SH1107 initialization failed"));
        for (;;)
            ; // Loop forever
    }
    display_.setRotation(1);
#endif
    initKeyboardLayoutStrings();
    // Clear the buffer
    display_.clearDisplay(); // No Adafruit splash
    display_.display();
    display_.setTextColor(WHITE);
    display_.setTextWrap(false);

    showInitialMessage();
    display_.display();
    if (sparkDC_->operationMode() == SPARK_MODE_KEYBOARD) {
        // Allow the initial screen to show for some time
        delay(2000);
    }
}

void SparkDisplayControl::showInitialMessage() {
    display_.drawBitmap(0, 0, epdBitmapIgnitronLogo, kSplashImageWidth, kSplashImageHeight,
                        WHITE);
    display_.setTextSize(2);

    string modeText;
    switch (sparkDC_->operationMode()) {
    case SPARK_MODE_APP:
        modeText = "APP";
        break;
    case SPARK_MODE_AMP:
        modeText = "AMP";
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
    display_.setTextColor(WHITE);
    display_.setTextSize(4);

    SparkPresetControl &presetControl = SparkPresetControl::getInstance();
    int pendingBank = presetControl.pendingBank();

    // Configure numbers as strings
    ostringstream selBankStr;
    selBankStr << pendingBank;

    ostringstream selPresetStr;
    selPresetStr << presetControl.activePresetNum();

    // Bank number display
    string bankDisplay = "";
    if (pendingBank < 10) {
        bankDisplay = "0";
    }
    bankDisplay += selBankStr.str();
    if (pendingBank == 0) {
        if (presetControl.numberOfHWBanks() == 1) {
            bankDisplay = "HW";
        } else {
            bankDisplay = "H" + to_string(presetControl.pendingHWBank() + 1);
        }
    }

    display_.setCursor(0, 0);
    display_.print(bankDisplay.c_str());

    // Preset display
    display_.setCursor(display_.width() - 24, 0);
    string presetText = "";
    presetText = selPresetStr.str();
    display_.print(presetText.c_str());
}

void SparkDisplayControl::showPresetName() {

    SparkPresetControl &presetControl = SparkPresetControl::getInstance();
    const string msg = presetControl.responseMsg();
    int pendingBank = presetControl.pendingBank();
    int activeBank = presetControl.activeBank();
    int pendingHWBank = presetControl.pendingHWBank();
    int activeHWBank = presetControl.activeHWBank();

    PresetEditMode presetEditMode = presetControl.presetEditMode();

    // Rectangle color for preset name
    int rectColor;
    int textColor;
    // Show preset name inverted if it is not the currently selected one
    if (pendingBank == activeBank && presetEditMode != PRESET_EDIT_DELETE) {
        rectColor = BLACK;
        textColor = WHITE;
    } else {
        rectColor = WHITE;
        textColor = BLACK;
    }

    display_.setTextColor(textColor);
    display_.fillRect(0, 31, 128, 16, rectColor);

    display_.setTextSize(2);
    unsigned long currentMillis = millis();

    if (msg != "") { // message to show for some time
        previousMillis = millis();
        primaryLineText = msg;
        presetControl.resetPresetEditResponse();
        showMsgFlag = true;
    }
    if (showMsgFlag) {
        display_.setCursor(0, 32);
        if (currentMillis - previousMillis >= showMessageInterval) {
            // reset the show message flag to show preset data again
            showMsgFlag = false;
        }
    } else { // no preset save message to display
        display_.setCursor(displayX1_, 32);
        // If bank is not HW preset bank and the currently selected bank
        // is not the active one, show the pending preset name
        if (activeBank != pendingBank || activeHWBank != pendingHWBank) {
            primaryLinePreset = presetControl.pendingPreset();
        } else {
            primaryLinePreset = presetControl.activePreset();
        }
        primaryLineText = primaryLinePreset.name;

        // Reset scroll timer
        if (primaryLineText != previousText1_) {
            textRow1Timestamp_ = millis();
            previousText1_ = primaryLineText;
            displayX1_ = 0;
        }
        // double the preset text for scrolling if longer than threshold
        //  if text is too long and will be scrolled, double to "wrap around"
        if (primaryLineText.length() > textScrollLimit_) {
            primaryLineText = primaryLineText + textFiller_ + primaryLineText;
        }
    }
    display_.print(primaryLineText.c_str());
}
void SparkDisplayControl::showFX_SecondaryName() {
    // The last line shows either the FX setup (in APP mode)
    // or the new received preset from the app (in AMP mode)
    int secondaryLinePosY = 50;

    OperationMode opMode = sparkDC_->operationMode();
    SparkPresetControl &presetControl = SparkPresetControl::getInstance();
    Preset presetFromApp = presetControl.appReceivedPreset();

    secondaryLineText = "";
    if (opMode == SPARK_MODE_AMP) {
        // displayPreset = presetFromApp;
        if (!(presetFromApp.isEmpty)) {
            secondaryLineText = presetFromApp.name;
            // Reset scroll timer
            if (secondaryLineText != previousText2_) {
                textRow2Timestamp_ = millis();
                previousText2_ = secondaryLineText;
                displayX2_ = 0;
            }
        } else if (presetControl.presetEditMode() == PRESET_EDIT_DELETE) {
            secondaryLineText = "DELETE ?";
        } else {
            secondaryLineText = "Select preset";
        }
        // if text is too long and will be scrolled, double to "wrap around"
        if (secondaryLineText.length() > textScrollLimit_) {
            secondaryLineText = secondaryLineText + textFiller_ + secondaryLineText;
        }
    } else if (opMode == SPARK_MODE_APP) {

        // Build string to show active FX
        SubMode subMode = sparkDC_->subMode();
#ifndef DEDICATED_PRESET_LEDS
        secondaryLinePreset = primaryLinePreset;

        // When we switched to FX mode, we always show the current selected preset
        if (subMode == SUB_MODE_FX) {
            secondaryLinePreset = presetControl.activePreset();
        }

        if (!(secondaryLinePreset.isEmpty) || presetControl.pendingBank() > 0) {
            // Iterate through the corresponding preset's pedals and show indicators if switched on
            for (int i = 0; i < 7; i++) { // 7 pedals, amp to be ignored
                if (i != 3) {             // Amp is on position 3, ignore
                    // blank placeholder for Amp
                    string fxIndicatorsOn[] =
                        {"N", "C ", "D ", " ", "M ", "D ", "R"};
                    string fxIndicatorsOff[] = {" ", "  ", "  ", " ", "  ", "  ", " "};

                    string currPedalStatus;
                    Pedal currPedal = secondaryLinePreset.pedals[i];
                    currPedalStatus =
                        currPedal.isOn ? fxIndicatorsOn[i] : fxIndicatorsOff[i];
                    secondaryLineText += currPedalStatus;
                }
            }
        } else {
            secondaryLineText = "";
        }
    }

    display_.fillRect(0, 48, 128, 16, BLACK);
    display_.setTextColor(WHITE);
#else
        secondaryLinePosY -= 1; // move up one pixel, looks better with inverted
        switch (subMode) {
        case SUB_MODE_FX:
            secondaryLineText = " Manual/FX ";
            break;
        case SUB_MODE_PRESET:
            secondaryLineText = "  Preset   ";
            break;
        case SUB_MODE_LOOP_CONFIG:
            secondaryLineText = " Loop Cnfg ";
            break;
        case SUB_MODE_LOOP_CONTROL:
        case SUB_MODE_LOOPER:
            secondaryLineText = "  Looper   ";
            break;
        }
    }

    display_.fillRect(0, 48, 128, 16, WHITE); // Invert line
    display_.setTextColor(BLACK);
#endif

    display_.setTextSize(2);
    if (opMode == SPARK_MODE_APP) {
        drawCentreString(secondaryLineText.c_str(), secondaryLinePosY);

    } else if (opMode == SPARK_MODE_AMP) {
        display_.setCursor(displayX2_, secondaryLinePosY);
        display_.print(secondaryLineText.c_str());
    }
}

void SparkDisplayControl::showLooperTimer() {

    SparkLooperControl &sparkLooperControl = sparkDC_->looperControl();
    int currentBeat = sparkLooperControl.currentBeat();
    int currentBar = sparkLooperControl.currentBar();
    int totalBars = sparkLooperControl.totalBars();
    int bpm = sparkLooperControl.bpm();

    display_.setTextSize(4);
    display_.setCursor(0, 0);
    display_.print(currentBar);
    display_.setCursor(display_.width() - 24, 0);
    display_.print(currentBeat);

    // Progress bar
    int barHeight = 16;
    int charWidth = 12;
    int xPos = 0;
    int xOffset = 16;
    int yPos = 47;
    // Reserve 2 characters for totalBars, 3 for BPM
    int maxBarWidth = display_.width() - 2 * charWidth;
    int barWidth = (double)(currentBar)*maxBarWidth / totalBars;
    int drawColor = WHITE;
    display_.fillRect(xPos, yPos, barWidth, barHeight, drawColor);

    // Show BPM
    int bpmXPos = display_.width() - 60;
    int bpmYPos = 17;
    display_.setCursor(bpmXPos, bpmYPos);
    display_.setTextSize(2);
    display_.print(bpm);

    // Show total bars
    display_.setCursor(maxBarWidth + 2, yPos);
    display_.setTextSize(2);
    display_.print(totalBars);

    // lines to separate the bars
    int stepSize = 1;
    if (totalBars > 25) {
        stepSize = 2;
    }
    for (int i = 0; i < totalBars; i = i + stepSize) {
        int xPos = (double)i * maxBarWidth / totalBars;
        drawColor = xPos < barWidth ? BLACK : WHITE;
        display_.drawFastVLine(xPos, yPos, barHeight, drawColor);
    }
}

void SparkDisplayControl::showModeModifier() {

    display_.setCursor(display_.width() - 48, 0);

    // Preset display
    display_.setTextSize(4);
    string presetText = " ";

    OperationMode opMode = sparkDC_->operationMode();
    SubMode subMode = sparkDC_->subMode();
    SparkPresetControl &presetControl = SparkPresetControl::getInstance();

    // Change to subMode
    if (opMode == SPARK_MODE_APP && subMode == SUB_MODE_FX) {
        // If in FX mode, show an "M" for manual mode
        presetText = "M";
    }
    if (opMode == SPARK_MODE_AMP && presetControl.presetEditMode() != PRESET_EDIT_NONE) {
        presetText = "*";
    }
    // Spark 2 built-in Looper
    if (subMode == SUB_MODE_LOOP_CONTROL) {
        // If in Looper mode, show an "L" for Looper mode
        display_.setTextSize(2);
        presetText = "L";
    }
    if (subMode == SUB_MODE_LOOP_CONFIG) {
        display_.setTextSize(2);
        presetText = "C";
    }
    // Looper app
    if (subMode == SUB_MODE_LOOPER) {
        // If in Looper mode, show an "L" for Looper mode
        presetText = "L";
    }
    display_.print(presetText.c_str());
}

void SparkDisplayControl::showConnection() {
    // Display the Connection symbols
    int xPosSymbol = (display_.width() / 2.0) - 10;
    int yPosSymbol = 9;
    int symbolWidth = 15;
    int symbolHeight = 17;

    int xPosText = xPosSymbol;
    int yPosText = 0;

    // Bluetooth
    switch (sparkDC_->currentBTMode()) {
    case BT_MODE_BLE:
        currentBTModeText = "BLE";
        break;
    case BT_MODE_SERIAL:
        currentBTModeText = "SRL";
        break;
    }
    display_.setTextColor(WHITE);
    display_.setTextSize(1);
    display_.setCursor(xPosText, yPosText);
    display_.print(currentBTModeText.c_str());

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
    // If enable battery SOC, move the BT symbol to the left
    xPosSymbol -= 5;
#endif

    bool isBTConnected = sparkDC_->isAmpConnected() || sparkDC_->isAppConnected();
    if (isBTConnected) {
        display_.drawBitmap(xPosSymbol, yPosSymbol, epdBitmapBTLogo, symbolWidth, symbolHeight, WHITE);
    }
}

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
// Show battery status indicator with a voltage divider
// 0-4095 for 0v-3.3v
void SparkDisplayControl::showBatterySymbol() {

    SparkStatus &statusObject = SparkStatus::getInstance();
    // Display the Battery symbols
    int xPosSymbol = (display_.width() / 2.0);
    int yPosSymbol = 11;
    int symbolWidth = 9;
    int symbolHeight = 15;

    BatteryLevel batteryLevel = sparkDC_->batteryLevel();

    uint16_t color = WHITE;
    const unsigned char *battery_icon;
    switch (batteryLevel) {
    case BATTERY_LEVEL_CHARGING:
        battery_icon = rotateBatteryIcons();
        break;
    case BATTERY_LEVEL_0:
        battery_icon = epdBitmapBatteryLevel0;
        break;
    case BATTERY_LEVEL_1:
        battery_icon = epdBitmapBatteryLevel1;
        break;
    case BATTERY_LEVEL_2:
        battery_icon = epdBitmapBatteryLevel2;
        break;
    case BATTERY_LEVEL_3:
    default:
        battery_icon = epdBitmapBatteryLevel3;
        break;
    }

    if (BATTERY_TYPE == BATTERY_TYPE_AMP) {
        if (statusObject.ampBatteryChargingStatus() == BATTERY_CHARGING_STATUS_POWERED) {
            battery_icon = epdBitmapBatteryPlug;
        }
    }

    display_.drawBitmap(xPosSymbol, yPosSymbol, battery_icon, symbolWidth, symbolHeight, color);
}
const unsigned char *SparkDisplayControl::rotateBatteryIcons() {

    unsigned long currentTime = millis();
    if (currentTime - lastBatteryRotationTimestamp > changeBatterySymbolInteral) {
        lastBatteryRotationTimestamp = currentTime;
        currentBatterySymbolIndex = (currentBatterySymbolIndex + 1) % 4;
    }
    switch (currentBatterySymbolIndex) {
    case 0:
        return epdBitmapBatteryLevel0;
        break;
    case 1:
        return epdBitmapBatteryLevel1;
        break;
    case 2:
        return epdBitmapBatteryLevel2;
        break;
    case 3:
    default:
        return epdBitmapBatteryLevel3;
        break;
    }
}
#endif

void SparkDisplayControl::showPressedKey() {

    short int pressedButtonPosX = 20;
    short int pressedButtonPosY = 0;

    display_.setTextColor(WHITE);
    display_.setTextSize(4);

    display_.setCursor(pressedButtonPosX, pressedButtonPosY);

    unsigned long currentMillis = millis();
    if (lastKeyboardButtonPressedString != "" && showKeyboardPressedFlag == false) {
        keyboardPressedTimestamp = millis();
        showKeyboardPressedFlag = true;
    }
    if (showKeyboardPressedFlag) {
        if (currentMillis - keyboardPressedTimestamp >= showKeyboardPressedInterval) {
            // reset the show message flag to show preset data again
            showKeyboardPressedFlag = false;
            sparkDC_->resetLastKeyboardButtonPressed();
            lastKeyboardButtonPressedString = "";
        }
    }

    display_.print(lastKeyboardButtonPressedString.c_str());
}

void SparkDisplayControl::initKeyboardLayoutStrings() {

    KeyboardMapping currentKeyboard = sparkDC_->currentKeyboard();
    string spacerText = "  ";

    lowerButtonsShort = currentKeyboard.keyboardShortPress[0].display.append(spacerText).append(currentKeyboard.keyboardShortPress[1].display).append(spacerText).append(currentKeyboard.keyboardShortPress[2].display).append(spacerText).append(currentKeyboard.keyboardShortPress[3].display);

    upperButtonsShort = currentKeyboard.keyboardShortPress[4].display.append(spacerText).append(currentKeyboard.keyboardShortPress[5].display);

    lowerButtonsLong = currentKeyboard.keyboardLongPress[0].display.append(spacerText).append(currentKeyboard.keyboardLongPress[1].display).append(spacerText).append(currentKeyboard.keyboardLongPress[2].display).append(spacerText).append(currentKeyboard.keyboardLongPress[3].display);

    upperButtonsLong = currentKeyboard.keyboardLongPress[4].display
                           //.append(spacerText)
                           .append(currentKeyboard.keyboardLongPress[5].display);
}

void SparkDisplayControl::showKeyboardLayout() {

    KeyboardMapping &currentKeyboard = sparkDC_->currentKeyboard();

    short int upperButtonsLongY = 1;
    short int upperButtonsShortY = 17;

    short int lowerButtonsLongY = 33;
    short int lowerButtonsShortY = 50;

    short int upperPositionOffset = -4;

    short int displayMid = display_.width() / 2;

    display_.fillRect(0, 48, 128, 16, BLACK);

    // Rectangle color for preset name
    int rectColor;
    int textColor;
    int textColorInv;
    // Show preset name inverted if it is not the currently selected one
    rectColor = WHITE;
    textColorInv = BLACK;
    textColor = WHITE;

    display_.setTextColor(textColor);
    display_.setTextSize(2);

    drawCentreString(lowerButtonsShort.c_str(), lowerButtonsShortY);
    drawRightAlignedString(upperButtonsShort.c_str(), upperButtonsShortY, upperPositionOffset);

    display_.setTextColor(textColorInv);
    display_.fillRect(0, lowerButtonsLongY - 1, 128, 16, rectColor);
    drawCentreString(lowerButtonsLong.c_str(), lowerButtonsLongY);

    display_.fillRect(displayMid - upperPositionOffset, 0, displayMid + upperPositionOffset, 16, rectColor);
    drawRightAlignedString(upperButtonsLong.c_str(), upperButtonsLongY, upperPositionOffset);
}

void SparkDisplayControl::showTunerNote() {

    display_.setTextColor(WHITE);

    // text sizes: 1 = 6x8, 2= 12x16, 3=18x24, 4=24x36
    char sharpSymbol = currentNote.at(1);
    display_.setTextSize(3);

    int displayMidY = display_.height() / 2.0;
    int displayMidX = display_.width() / 2.0;
    int notePosX = displayMidX - 6;
    if (sharpSymbol == '#') {
        notePosX = displayMidX - 12;
    }
    display_.setCursor(notePosX, displayMidY - 12);
    char baseNote = currentNote.at(0);
    display_.print(baseNote);

    if (sharpSymbol == '#') {
        display_.setTextSize(2);
        display_.setCursor(displayMidX + 8, displayMidY - 8);
        display_.print(sharpSymbol);
    }
}

void SparkDisplayControl::showTunerOffset() {

    display_.setTextColor(WHITE);
    if (noteOffsetCents >= -50 && noteOffsetCents <= 50) {
        // draw dynamic note offset at the bottom
        int tunerPosCenter = 128 / 2; // the middle lines are 63 and 64, choose the right side as it visually matches the other visible items
        int barHeight = 12;
        int tunerPosBottomY = 64 - barHeight;
        display_.drawFastVLine(tunerPosCenter - 2, tunerPosBottomY + barHeight / 3, barHeight / 3, WHITE);
        display_.drawFastVLine(tunerPosCenter + 2, tunerPosBottomY + barHeight / 3, barHeight / 3, WHITE);

        tunerPosCenter += noteOffsetCents;
        display_.drawFastVLine(tunerPosCenter, tunerPosBottomY, barHeight, WHITE);
    }
}

void SparkDisplayControl::showTunerGraphic() {

    int triangleSize = 12;
    int color = WHITE;
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
        display_.drawCircle(display_.width() / 2.0, display_.height() / 2.0, 20, color);
        invertedDisplay = true;
    } else {
        invertedDisplay = false;
    }
}

void SparkDisplayControl::showVolumeBar() {
    SparkStatus &statusObject = SparkStatus::getInstance();
    float volume = statusObject.inputVolume();

    display_.setTextSize(1);
    display_.setTextColor(WHITE);
    display_.setCursor(0, 0);

    // Draw volume bar
    int barWidth = 110;
    int barHeight = 16;
    int xPos = (display_.width() - barWidth) / 2;
    int yPos = (display_.height() - barHeight) / 2;

    display_.drawRect(xPos, yPos, barWidth, barHeight, WHITE);

    // Draw volume level
    int volumeBarWidth = volume * barWidth;
    display_.fillRect(xPos + 1, yPos + 1, volumeBarWidth - 2, barHeight - 2, WHITE);

    int stringYPos = (display_.height() + barHeight) / 2 + 5;
    drawCentreString("INPUT VOLUME", stringYPos);

    // Draw volume number
    display_.setTextSize(2);
    int volumeNumberYPos = (display_.height() - barHeight) / 2 - 20;
    int volumeNumber = (int)(volume * 100); // Convert to percentage
    drawCentreString(to_string(volumeNumber).c_str(), volumeNumberYPos);
}

void SparkDisplayControl::checkInvertDisplay(int subMode) {
    switch (subMode) {
    case SUB_MODE_FX:
        invertedDisplay = true;
        break;
    case SUB_MODE_TUNER:
        // don't change value as it is set in showTunerGraphics()
        break;
    default:
        invertedDisplay = false;
        break;
    }

    display_.invertDisplay(invertedDisplay);
}

void SparkDisplayControl::update(bool isInitBoot) {

    OperationMode opMode = sparkDC_->operationMode();
    SubMode subMode = sparkDC_->subMode();
    display_.clearDisplay();
    checkInvertDisplay(subMode);

    if ((opMode == SPARK_MODE_APP) && isInitBoot) {
        showInitialMessage();
    } else if (opMode == SPARK_MODE_KEYBOARD) {
        if (sparkDC_->keyboardChanged()) {
            initKeyboardLayoutStrings();
            sparkDC_->resetKeyboardChangeIndicator();
        }
        lastKeyboardButtonPressedString = sparkDC_->lastKeyboardButtonPressedString();
        showPressedKey();
        showKeyboardLayout();
    } else if (subMode == SUB_MODE_TUNER) {
        SparkStatus &statusObject = SparkStatus::getInstance();
        currentNote = statusObject.noteString();
        noteOffsetCents = statusObject.noteOffsetCents();

        showTunerNote();
        showTunerOffset();
        showTunerGraphic();

    } else {
        // SparkPresetControl &presetControl = SparkPresetControl::getInstance();
        SparkStatus &statusObject = SparkStatus::getInstance();
        isVolumeChanged = statusObject.isVolumeChanged();
        if (isVolumeChanged) {
            volumeChangedTimestamp = millis();
            statusObject.resetVolumeUpdateFlag();
        }
        unsigned int now = millis();
        if (now - volumeChangedTimestamp <= showVolumeChangedInterval) {
            showVolumeBar();
        } else {
            display_.setTextWrap(false);
            showConnection();

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
            showBatterySymbol();
#endif

            showModeModifier();
            updateTextPositions();
            showPresetName();
            if (subMode == SUB_MODE_LOOP_CONTROL || subMode == SUB_MODE_LOOP_CONFIG) {
                showLooperTimer();
            } else {
                showBankAndPresetNum();
                showFX_SecondaryName();
            }
        }
    }
    // logDisplay();
    display_.display();
}

void SparkDisplayControl::updateTextPositions() {
    // This is for the primary preset name line
    displayMinX1_ = DISPLAY_MIN_X_FACTOR * ((primaryLineText.length() - textFiller_.length()) / 2.0 + textFiller_.length());
    if (primaryLineText.length() <= textScrollLimit_) {
        displayX1_ = 0;
    }
    // long preset names are scrolled right to left and back
    else {
        if (millis() - textRow1Timestamp_ > textScrollDelay_) {
            displayX1_ -= displayScrollNum1_;
            if (displayX1_ < displayMinX1_) {
                // Reset text
                displayX1_ = 0;
                textRow1Timestamp_ = millis();
            }
        }
    }

    // This is for the secondary FX display line (show preset name in AMP mode)
    displayMinX2_ = DISPLAY_MIN_X_FACTOR * ((secondaryLineText.length() - textFiller_.length()) / 2.0 + textFiller_.length());
    if (secondaryLineText.length() <= textScrollLimit_) {
        displayX2_ = 0;
    } else {
        if (millis() - textRow2Timestamp_ > textScrollDelay_) {
            displayX2_ -= displayScrollNum2_;
            if (displayX2_ < displayMinX2_) {
                // Reset text
                displayX2_ = 0;
                textRow2Timestamp_ = millis();
            }
        }
    }
}

void SparkDisplayControl::drawCentreString(const char *buf,
                                           int y, int offset) {
    int16_t x1, y1;
    uint16_t w, h;
    int displayMid = display_.width() / 2;

    display_.getTextBounds(buf, displayMid, y, &x1, &y1, &w, &h); // calc width of new string
    display_.setCursor(displayMid - w / 2 + offset, y);
    display_.print(buf);
}

void SparkDisplayControl::drawRightAlignedString(const char *buf,
                                                 int y, int offset) {
    int16_t x1, y1;
    uint16_t w, h;
    int x = 0;
    int displayWidth = display_.width();

    display_.getTextBounds(buf, x, y, &x1, &y1, &w, &h); // calc width of new string
    display_.setCursor(displayWidth - w + offset, y);
    display_.print(buf);
}

void SparkDisplayControl::drawTunerTriangleCentre(int x, int size, bool dir, int color) {

    int displayMidY = display_.height() / 2.0;
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

    display_.fillTriangle(xPos1, yPos1, xPos2, yPos2, xPos3, yPos3, color);
}

void SparkDisplayControl::drawInvertBitmapColor(int16_t x, int16_t y,
                                                const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {

    int16_t i, j, byteWidth = (w + 7) / 8;

    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            if ((pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) == 0) {
                display_.drawPixel(x + i, y + j, color);
            }
        }
    }
}

void SparkDisplayControl::logDisplay() {
    if (millis() - lastLogTimestamp > logInterval) {
        SparkPresetControl &presetControl = SparkPresetControl::getInstance();

        DEBUG_PRINTLN("Display status:");
        DEBUG_PRINTLN("Current Preset settings:");
        DEBUG_PRINTF("Primary line   : %s\n", primaryLineText.c_str());
        DEBUG_PRINTF("Secondary line : %s\n", secondaryLineText.c_str());
        if (!(presetControl.activePreset().isEmpty) && !(presetControl.pendingPreset().isEmpty)) {
            DEBUG_PRINTF("Act Preset empty? : %s\n", presetControl.activePreset().isEmpty ? "true" : "false");
            DEBUG_PRINTF("Pen Preset empty? : %s\n", presetControl.pendingPreset().isEmpty ? "true" : "false");
        }
        lastLogTimestamp = millis();
    }
}
