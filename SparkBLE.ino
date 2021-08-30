#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <string>
#include <NimBLEDevice.h> // github NimBLE

#include "SparkDataControl.h"
#include "SparkButtonHandler.h"
#include "SparkLEDControl.h"
#include "SparkDisplayControl.h"

// Device Info Definitions
const std::string DEVICE_NAME = "SparkBLE";
const std::string VERSION = "0.5";

// Control classes
SparkDataControl *spark_dc;
SparkButtonHandler *spark_bh;
SparkLEDControl *spark_led;
SparkDisplayControl *spark_display;

// Check for initial boot
bool isInitBoot;
int operationMode = SPARK_MODE_APP;

/////////////////////////////////////////////////////////
//
// INIT AND RUN
//
/////////////////////////////////////////////////////////

void setup() {
	isInitBoot = true;
	Serial.begin(115200);
	while (!Serial)
		;

	Serial.println("Initializing");
	spark_dc = new SparkDataControl();

	spark_bh = new SparkButtonHandler(spark_dc);

	operationMode = spark_bh->init();
	if (operationMode == SPARK_MODE_APP) {
		Serial.println("======= Entering APP mode =======");
	} else {
		Serial.println("======= Entering AMP mode =======");
	}
	spark_display = new SparkDisplayControl(spark_dc);
	spark_display->init(operationMode);
	if (operationMode == SPARK_MODE_AMP) {
		// Allow for the button to be unpressed to not register button in normal operation
		delay(3000);
	}
	spark_dc->setDisplayControl(spark_display);
	// Setting operation mode before initializing
	spark_dc->init(operationMode);

	// Assigning data control to buttons;
	spark_bh->dataControl(spark_dc);

	// Initializing control classes
	spark_led = new SparkLEDControl(spark_dc);
	Serial.println("Initialization done.");

}

void loop() {

	if (operationMode == SPARK_MODE_APP) {
		while (!spark_dc->checkBLEConnection()) {
			;
		}

		//After connection is established, continue.
		// On first boot, set the preset to Hardware setting 1.
		if (isInitBoot == true) {
			//Serial.println("Initial boot, setting preset to HW 1");
			if (spark_dc->switchPreset(1)) {
				isInitBoot = false;
			}
		}
	}
	// Check if presets have been updated
	spark_dc->checkForUpdates();
	// Reading button input
	spark_bh->readButtons();
	// Update LED status
	spark_led->updateLEDs();
	// Update display
	spark_display->update();
}
