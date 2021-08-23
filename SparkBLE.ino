#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Regexp.h>
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

/////////////////////////////////////////////////////////
//
// INIT AND RUN
//
/////////////////////////////////////////////////////////

void setup() {
	isInitBoot = true;

	Serial.begin(115200);
	while (!Serial);

	// Initializing control classes
	Serial.println("Initializing:");
	Serial.println(" - Data Control");
	spark_dc = new SparkDataControl();
	spark_dc->init();
	Serial.println(" - Button Handler");
	spark_bh = new SparkButtonHandler(spark_dc);
	Serial.println(" - LED Control");
	spark_led = new SparkLEDControl(spark_dc);
	Serial.println(" - Display Control");
	spark_display = new SparkDisplayControl(spark_dc);
	Serial.println("Done.");

}


void loop() {

	while (!spark_dc->checkBLEConnection()){ ;}

	//After connection is established, continue.
	// On first boot, set the preset to Hardware setting 1.
	if (isInitBoot == true) {
		//Serial.println("Initial boot, setting preset to HW 1");
		spark_dc->switchPreset(1);
		isInitBoot = false;
	}

	// Reading button input
	spark_bh->readButtons();
	// Check if presets have been updated
	spark_dc->checkForUpdates();
	// Update LED status
	spark_led->updateLEDs();
	// Update display
	spark_display->update();

}
