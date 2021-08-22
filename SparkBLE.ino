//#include <splash.h>
//#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> //https://github.com/adafruit/Adafruit_SSD1306
#include <BfButton.h> //https://github.com/mickey9801/ButtonFever
#include <Regexp.h>
#include <string>
#include <NimBLEDevice.h> // github NimBLE

#include "SparkDataControl.h"


// Device Info Definitions
const String DEVICE_NAME = "SparkPal";
const String VERSION = "0.3.1";

// GPIO Buttons/LEDs
#define BUTTON_CHANNEL1_GPIO 19
#define BUTTON_DRIVE_GPIO 19

#define BUTTON_CHANNEL2_GPIO 18
#define BUTTON_MOD_GPIO 18

#define BUTTON_CHANNEL3_GPIO 32
#define BUTTON_DELAY_GPIO 32

#define BUTTON_CHANNEL4_GPIO 25
#define BUTTON_REVERB_GPIO 25

#define BUTTON_BANK_DOWN_GPIO 26
#define BUTTON_NOISEGATE_GPIO 26

#define BUTTON_BANK_UP_GPIO 33
#define BUTTON_COMP_GPIO 33

#define LED_CHANNEL1_GPIO 13
#define LED_DRIVE_GPIO 13

#define LED_CHANNEL2_GPIO 12
#define LED_MOD_GPIO 12

#define LED_CHANNEL3_GPIO 14
#define LED_DELAY_GPIO 14

#define LED_CHANNEL4_GPIO 16
#define LED_REVERB_GPIO 16

#define LED_BANK_DOWN_GPIO 17
#define LED_NOISEGATE_GPIO 17

#define LED_BANK_UP_GPIO 15
#define LED_COMP_GPIO 15

BfButton btn_preset1(BfButton::STANDALONE_DIGITAL, BUTTON_CHANNEL1_GPIO, false,
		HIGH);
BfButton btn_preset2(BfButton::STANDALONE_DIGITAL, BUTTON_CHANNEL2_GPIO, false,
		HIGH);
BfButton btn_preset3(BfButton::STANDALONE_DIGITAL, BUTTON_CHANNEL3_GPIO, false,
		HIGH);
BfButton btn_preset4(BfButton::STANDALONE_DIGITAL, BUTTON_CHANNEL4_GPIO, false,
		HIGH);
BfButton btn_bank_down(BfButton::STANDALONE_DIGITAL, BUTTON_BANK_DOWN_GPIO,
		false, HIGH);
BfButton btn_bank_up(BfButton::STANDALONE_DIGITAL, BUTTON_BANK_UP_GPIO, false,
		HIGH);

// OLED Screen config
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ESP32 Bluetooth Serial Object
// only needed for BT classic
//BluetoothSerial SerialBT;

//PRESET variables
preset activePreset;
preset selectedPreset;
std::vector<std::vector<preset>>* presetBanks;
int numberOfBanks = 0;
int selectedBank = 0;
int activeBank = 0;
int activePresetNum = 1;

boolean bankChanged = false;

//COMMS variables
SparkMessage spark_msg;
SparkStreamReader spark_ssr;
SparkDataControl *spark_dc;

//ByteVector rd_data = {};

//BT classic variables
//const char SPARK_BT_NAME[] = "Spark 40 Audio";
//const char SPARK_BT_NAME[] = "Spark BLE";

// CONNECTION / CONTROL variables
bool isBTConnected;
bool isInitBoot;
bool startup;

//DISPLAY variables
int display_x;
int display_minX;
const int DISPLAY_MIN_X_FACTOR = -12; // for text size 2, scales linearly with text size
int display_scroll_num = -1; // scroll speed, make more positive to speed up the scroll

// BUTTON MODES
const int SWITCH_MODE_FX = 1;
const int SWITCH_MODE_CHANNEL = 2;
int buttonMode = SWITCH_MODE_CHANNEL;

// Forward declarations (required for default values)
void updateLEDs();

/////////////////////////////////////////////////////////
//
// NimBLE STUFF - Check if that can be moved to other classes later
//
/////////////////////////////////////////////////////////









/////////////////////////////////////////////////////////
//
// DISPLAY
//
/////////////////////////////////////////////////////////

void initDisplay() {
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
	//displayMessage("SparkPal v0.2", 2);
	display_x = 0;
	display_minX = DISPLAY_MIN_X_FACTOR * 10; //initialize, will be updated with new preset
	display.drawBitmap(0, 0, sweaty_logo_bmp, 128, 47, SSD1306_WHITE);
	display.setTextSize(2);
	display.setCursor(6, 48);
	display.print("Connecting");
	display.display();
	//delay(2000);
}

void displayMessage(String message, int fontsize) {
	display.setCursor(0, 0);
	display.setTextSize(fontsize);
	display.print(message);
}

void updateDisplay() {

	display.setTextColor(SSD1306_WHITE);
	//Configure numbers as strings
	std::ostringstream selBankStr;
	selBankStr << selectedBank;

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
	if (selectedBank < 10) {
		bankDisplay = "0";
	}
	bankDisplay += selBankStr.str();
	if (selectedBank == 0) {
		bankDisplay = "HW";
	}

	display.setCursor(1, 1);
	display.print(bankDisplay.c_str());

	//RECTANGLE COLOR FOR PRESET NAME
	int rectColor;
	int textColor;
	if (selectedBank == activeBank) {
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
	std::string displayPresetName;
	preset displayPreset;

	if (selectedBank != 0 && activeBank != selectedBank) {
		//displayPreset = (*presetBanks)[selectedBank - 1][selectedPresetNum - 1];
		displayPreset = selectedPreset;
		displayPresetName = displayPreset.name;
	} else if (selectedBank == 0 && activeBank != selectedBank) {
		displayPreset = activePreset;
		displayPresetName = "Hardware Preset " + selPresetStr.str();
	} else {
		displayPreset = activePreset;
		displayPresetName = displayPreset.name;
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
	if (!displayPreset.isEmpty || selectedBank > 0) {
		if (bankChanged) {
			//Serial.println(displayPreset.getPython().c_str());
			bankChanged = false;
		}
		for (int i = 0; i < 7; i++) { // 7 pedals, amp to be ignored
			if (i != 3) {// && i != 0) { // Noise Reduction position 0 Amp is on position 3
				pedal currPedal = displayPreset.pedals[i];
				currPedalStatus =
						displayPreset.pedals[i].isOn ? fx_indicators_on[i] : fx_indicators_off[i];
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


/////////////////////////////////////////////////////////
//
// HW BUTTONS, LED, BLUETOOTH
//
/////////////////////////////////////////////////////////
void btnPresetHandler(BfButton *btn, BfButton::press_pattern_t pattern) {
	if (pattern == BfButton::SINGLE_PRESS) {
		int pressed_btn_gpio = btn->getID();
		// Debug
		Serial.println("");
		Serial.print("Button pressed: ");
		Serial.println(pressed_btn_gpio);

		if (buttonMode == SWITCH_MODE_FX) {
			std::string fx_name;
			boolean fx_isOn;
			if (!activePreset.isEmpty) {
				switch (pressed_btn_gpio) {
				case BUTTON_DRIVE_GPIO:
					fx_name = activePreset.pedals[FX_DRIVE].name;
					fx_isOn = activePreset.pedals[FX_DRIVE].isOn;
					break;
				case BUTTON_MOD_GPIO:
					fx_name = activePreset.pedals[FX_MOD].name;
					fx_isOn = activePreset.pedals[FX_MOD].isOn;
					break;
				case BUTTON_DELAY_GPIO:
					fx_name = activePreset.pedals[FX_DELAY].name;
					fx_isOn = activePreset.pedals[FX_DELAY].isOn;
					break;
				case BUTTON_REVERB_GPIO:
					fx_name = activePreset.pedals[FX_REVERB].name;
					fx_isOn = activePreset.pedals[FX_REVERB].isOn;
					break;
				}
				spark_dc->switchEffectOnOff(fx_name,
						fx_isOn ? false : true);
				updateLEDs();
				//delay(150);
				//TODO Check if needed (ACK mode)
				//spark_dc->getCurrentPresetFromSpark();
			}
		} else if (buttonMode == SWITCH_MODE_CHANNEL) {

			if (pressed_btn_gpio == BUTTON_CHANNEL1_GPIO) {
				activePresetNum = 1;
			} else if (pressed_btn_gpio == BUTTON_CHANNEL2_GPIO) {
				activePresetNum = 2;
			} else if (pressed_btn_gpio == BUTTON_CHANNEL3_GPIO) {
				activePresetNum = 3;
			} else if (pressed_btn_gpio == BUTTON_CHANNEL4_GPIO) {
				activePresetNum = 4;
			}
			// Send the preset command
			spark_dc->switchPreset(selectedBank, activePresetNum);
			//getCurrentPreset();
		}
	}
}

void btnBankHandler(BfButton *btn, BfButton::press_pattern_t pattern) {
	if (pattern == BfButton::SINGLE_PRESS) {
		int pressed_btn_gpio = btn->getID();
		// Debug
		Serial.println("");
		Serial.print("Button pressed: ");
		Serial.println(pressed_btn_gpio);

		if (buttonMode == SWITCH_MODE_FX) {

			std::string fx_name;
			boolean fx_isOn;
			if (!activePreset.isEmpty) {
				if (pressed_btn_gpio == BUTTON_NOISEGATE_GPIO) {
					fx_name = activePreset.pedals[FX_NOISEGATE].name;
					fx_isOn = activePreset.pedals[FX_NOISEGATE].isOn;
				} else if (pressed_btn_gpio == BUTTON_COMP_GPIO) {
					fx_name = activePreset.pedals[FX_COMP].name;
					fx_isOn = activePreset.pedals[FX_COMP].isOn;
				}

				spark_dc->switchEffectOnOff(fx_name,
										fx_isOn ? false : true);
				//delay(150);
				//TODO check if needed (ACK mode)
				//spark_dc->getCurrentPresetFromSpark();
			}
		} else if (buttonMode == SWITCH_MODE_CHANNEL) {
			if (pressed_btn_gpio == BUTTON_BANK_DOWN_GPIO) {
				if (selectedBank == 0) {
					selectedBank = numberOfBanks;
				} else {
					selectedBank--;
				} // else
			} //if BUTTON_BANK_UP/DOWN
			else if (pressed_btn_gpio == BUTTON_BANK_UP_GPIO) {
				if (selectedBank == numberOfBanks) {
					selectedBank = 0;
				} else {
					selectedBank++;
				}
			}
		} // SWITCH_MODE_CHANNEL
		if(selectedBank != 0){
			selectedPreset = spark_dc->getPreset(selectedBank, activePresetNum);
		}
		bankChanged = true;
	} // SINGLE PRESS
}

void btnSwitchModeHandler(BfButton *btn, BfButton::press_pattern_t pattern) {

	if (pattern == BfButton::LONG_PRESS) {
		int pressed_btn_gpio = btn->getID();
		// Debug
		Serial.println("");
		Serial.print("Button long pressed: ");
		Serial.println(pressed_btn_gpio);
		//Up preset
		if (pressed_btn_gpio == BUTTON_BANK_UP_GPIO) {
			Serial.print("Switching mode to ");
			switch (buttonMode) {
			case SWITCH_MODE_FX:
				Serial.println("channel mode");
				buttonMode = SWITCH_MODE_CHANNEL;
				updateLEDs();
				break;
			case SWITCH_MODE_CHANNEL:
				Serial.println("FX mode");
				buttonMode = SWITCH_MODE_FX;
				selectedBank = activeBank;
				//getCurrentPreset();
				//delay(1000);
				updateLEDs();
				break;
			default:
				Serial.println("Unexpected mode. Defaulting to CHANNEL mode");
				buttonMode = SWITCH_MODE_CHANNEL;
				//getCurrentPreset();
				break;
			} // SWITCH
		} // IF BANK UP BUTTON
	}
}

void btnSetup() {
	// Setup the button event handler
	btn_preset1.onPress(btnPresetHandler);
	btn_preset2.onPress(btnPresetHandler);
	btn_preset3.onPress(btnPresetHandler);
	btn_preset4.onPress(btnPresetHandler);
	btn_bank_down.onPress(btnBankHandler);
	btn_bank_up.onPress(btnBankHandler);
	btn_bank_up.onPressFor(btnSwitchModeHandler, 1500);

}

void updateLEDs() {

	if (buttonMode == SWITCH_MODE_CHANNEL) {
		switch (activePresetNum) {
		case 1:
			digitalWrite(LED_CHANNEL1_GPIO, HIGH);
			digitalWrite(LED_CHANNEL2_GPIO, LOW);
			digitalWrite(LED_CHANNEL3_GPIO, LOW);
			digitalWrite(LED_CHANNEL4_GPIO, LOW);
			digitalWrite(LED_BANK_DOWN_GPIO, LOW);
			digitalWrite(LED_BANK_UP_GPIO, LOW);
			break;
		case 2:
			digitalWrite(LED_CHANNEL1_GPIO, LOW);
			digitalWrite(LED_CHANNEL2_GPIO, HIGH);
			digitalWrite(LED_CHANNEL3_GPIO, LOW);
			digitalWrite(LED_CHANNEL4_GPIO, LOW);
			digitalWrite(LED_BANK_DOWN_GPIO, LOW);
			digitalWrite(LED_BANK_UP_GPIO, LOW);
			break;
		case 3:
			digitalWrite(LED_CHANNEL1_GPIO, LOW);
			digitalWrite(LED_CHANNEL2_GPIO, LOW);
			digitalWrite(LED_CHANNEL3_GPIO, HIGH);
			digitalWrite(LED_CHANNEL4_GPIO, LOW);
			digitalWrite(LED_BANK_DOWN_GPIO, LOW);
			digitalWrite(LED_BANK_UP_GPIO, LOW);
			break;
		case 4:
			digitalWrite(LED_CHANNEL1_GPIO, LOW);
			digitalWrite(LED_CHANNEL2_GPIO, LOW);
			digitalWrite(LED_CHANNEL3_GPIO, LOW);
			digitalWrite(LED_CHANNEL4_GPIO, HIGH);
			digitalWrite(LED_BANK_DOWN_GPIO, LOW);
			digitalWrite(LED_BANK_UP_GPIO, LOW);
			break;
		}
	} else if (buttonMode == SWITCH_MODE_FX) {
		if (!activePreset.isEmpty) {

			pedal fx_noisegate = activePreset.pedals[FX_NOISEGATE];
			if (fx_noisegate.isOn) {
				digitalWrite(LED_NOISEGATE_GPIO, HIGH);
			} else {
				digitalWrite(LED_NOISEGATE_GPIO, LOW);
			}

			pedal fx_comp = activePreset.pedals[FX_COMP];
			if (fx_comp.isOn) {
				digitalWrite(LED_COMP_GPIO, HIGH);
			} else {
				digitalWrite(LED_COMP_GPIO, LOW);
			}

			pedal fx_dist = activePreset.pedals[FX_DRIVE];
			if (fx_dist.isOn) {
				digitalWrite(LED_DRIVE_GPIO, HIGH);
			} else {
				digitalWrite(LED_DRIVE_GPIO, LOW);
			}

			pedal fx_mod = activePreset.pedals[FX_MOD];
			if (fx_mod.isOn) {
				digitalWrite(LED_MOD_GPIO, HIGH);
			} else {
				digitalWrite(LED_MOD_GPIO, LOW);
			}

			pedal fx_delay = activePreset.pedals[FX_DELAY];
			if (fx_delay.isOn) {
				digitalWrite(LED_DELAY_GPIO, HIGH);
			} else {
				digitalWrite(LED_DELAY_GPIO, LOW);
			}

			pedal fx_reverb = activePreset.pedals[FX_REVERB];
			if (fx_reverb.isOn) {
				digitalWrite(LED_REVERB_GPIO, HIGH);
			} else {
				digitalWrite(LED_REVERB_GPIO, LOW);
			}
		}
	}

}


/////////////////////////////////////////////////////////
//
// INIT AND RUN
//
/////////////////////////////////////////////////////////

void setup() {
	isInitBoot = true;
	startup = true;

	pinMode(LED_CHANNEL1_GPIO, OUTPUT);
	pinMode(LED_CHANNEL2_GPIO, OUTPUT);
	pinMode(LED_CHANNEL3_GPIO, OUTPUT);
	pinMode(LED_CHANNEL4_GPIO, OUTPUT);
	pinMode(LED_BANK_DOWN_GPIO, OUTPUT);
	pinMode(LED_BANK_UP_GPIO, OUTPUT);

	// Start serial debug console monitoring
	Serial.begin(115200);
	while (!Serial);
	Serial.println("Starting");

	isBTConnected = false;
	selectedBank = 0;
	activeBank = 0;
	activePresetNum = 1;


	btnSetup();
	spark_dc = new SparkDataControl();
	spark_dc->init();
	numberOfBanks = spark_dc->getNumberOfBanks();
	Serial.printf("Number of Banks = %d\n", numberOfBanks);
	initDisplay();

}



void loop() {

	while (!spark_dc->checkBLEConnection()){ ;}
	//After connection is established, continue as planned.
	btn_preset1.read();
	btn_preset2.read();
	btn_preset3.read();
	btn_preset4.read();
	btn_bank_up.read();
	btn_bank_down.read();

	if (isInitBoot == true) {
		Serial.println("Initial boot, setting preset to HW 1");
		spark_dc->switchPreset(0,1);
		isInitBoot = false;
		//Serial.println("Trying to get current preset");
		//spark_dc->getCurrentPresetFromSpark();
	}

	if(spark_dc->isActivePresetUpdated()){
		Serial.println("Preset updated!");
		activePreset = spark_dc->getActivePreset();
		activeBank = spark_dc->getActiveBank();
		selectedPreset = activePreset;
		//Serial.println("Active preset:"),
		//Serial.println(activePreset.getJson().c_str());
		updateLEDs();
	}
	if(spark_dc->isPresetNumberUpdated()){
		Serial.println("Preset number updated!");
		activeBank = 0;
		selectedBank = 0;
		selectedPreset = activePreset;
		updateLEDs();
	}
	updateDisplay();

}
