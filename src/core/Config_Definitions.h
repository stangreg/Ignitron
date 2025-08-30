/*
 * Common.h
 *
 *  Created on: 26.11.2021
 *      Author: steffen
 */

#ifndef CONFIG_DEFINITIONS_H_
#define CONFIG_DEFINITIONS_H_

#include <Arduino.h>
#include <string>
using namespace std;

// #define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#define DEBUG_PRINTVECTOR(x) SparkHelper::printByteVector(x)
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(...)
#define DEBUG_PRINTVECTOR(x)
#endif

// Software version
const string VERSION = "2.0.0";

// Battery indicator
// Note: This battery can be for the Ignitron controller or the Spark amp.
// When BATTERY_TYPE = BATTERY_TYPE_AMP is selected,
// the battery indicator will show the AMP battery status,
// otherwise the internal battery of Ignitron (if present)
//
// As ESP32 only support ADC input less than 3.3V, will use a voltage divider built by two
// simple resistors in serial to pull down the voltage ESP32's ADC pin receives.
//
// Connection:
//   GND(Battery-) - resistor 1 - resistor 2 - Battery+
//   ESP ADC pin________________|
//
// ESP32 ADC pin support ADC input in 0V to 3.3V, choose the right combination of the resistors,
// make sure the maximum voltage ESP32 receives on voltage pin is less than 3.3V.
// For example, for a 3S Li-ion Battery pack, battery voltage ranges from 9V to 12.6V,
// choosing 5.1k ohm resistor for resistor 1, and 15k ohm resistor for resistor 2,
// so: VoltageOnADCPin = BatteryVoltage / (5.1k + 15k) * 5.1k
// When battery is full, VoltageOnADCPin = 12.6V / (5.1k + 15k) * 5.1k ~= 3.197V < 3.3V,
// so this voltage divider setting is suitable for this need.

#define ENABLE_BATTERY_STATUS_INDICATOR

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
const int BATTERY_VOLTAGE_ADC_PIN = 36;

enum BatteryLevel {
    BATTERY_LEVEL_0,           // 0-10%
    BATTERY_LEVEL_1,           // 10-50%
    BATTERY_LEVEL_2,           // 50-90%
    BATTERY_LEVEL_3,           // 90-100%
    BATTERY_LEVEL_CHARGING = 9 // when charging (only used for AMP battery)
};

// only required for Amp battery
const float BATTERY_MAX_LEVEL = 4096.0;
const float BATTERY_MIN_LEVEL = 3480.0;

#define BATTERY_TYPE_LI_ION 0
#define BATTERY_TYPE_LI_FE_PO4 1
#define BATTERY_TYPE_AMP 2

#define BATTERY_TYPE BATTERY_TYPE_AMP // Choose from BATTERY_TYPE_LI_ION or BATTERY_TYPE_LI_FE_PO4 for Ignitron internal battery or BATTERY_TYPE_AMP for Spark battery
const int BATTERY_CELLS = 3;
const float VOLTAGE_DIVIDER_R1 = (5.1 * 1000); // 5.1k ohm
const float VOLTAGE_DIVIDER_R2 = (15 * 1000);  // 15k ohm

#if BATTERY_TYPE == BATTERY_TYPE_LI_ION
const float BATTERY_CAPACITY_VOLTAGE_THRESHOLD_90 = (BATTERY_CELLS * 4.1);
const float BATTERY_CAPACITY_VOLTAGE_THRESHOLD_50 = (BATTERY_CELLS * 3.7);
const float BATTERY_CAPACITY_VOLTAGE_THRESHOLD_10 = (BATTERY_CELLS * 3.3);
#elif BATTERY_TYPE == BATTERY_TYPE_LI_FE_PO4
const float BATTERY_CAPACITY_VOLTAGE_THRESHOLD_90 = (BATTERY_CELLS * 3.35);
const float BATTERY_CAPACITY_VOLTAGE_THRESHOLD_50 = (BATTERY_CELLS * 3.26);
const float BATTERY_CAPACITY_VOLTAGE_THRESHOLD_10 = (BATTERY_CELLS * 3.0);
#elif BATTERY_TYPE == BATTERY_TYPE_AMP // Level between 3480(?) and 4095 (more sensitive to the end)
const float BATTERY_CAPACITY_VOLTAGE_THRESHOLD_90 = (BATTERY_MIN_LEVEL + (BATTERY_MAX_LEVEL - BATTERY_MIN_LEVEL) * 0.7);
const float BATTERY_CAPACITY_VOLTAGE_THRESHOLD_50 = (BATTERY_MIN_LEVEL + (BATTERY_MAX_LEVEL - BATTERY_MIN_LEVEL) * 0.4);
const float BATTERY_CAPACITY_VOLTAGE_THRESHOLD_10 = (BATTERY_MIN_LEVEL + (BATTERY_MAX_LEVEL - BATTERY_MIN_LEVEL) * 0.1);
#endif

// Define BatteryChargingStatus outside of battery type conditionals so it's always available
enum BatteryChargingStatus {
    BATTERY_CHARGING_STATUS_DISCHARGING,
    BATTERY_CHARGING_STATUS_POWERED,
    BATTERY_CHARGING_STATUS_CHARGING,
    BATTERY_CHARGING_STATUS_FULL_CHARGED
};

#endif

// OLED Driver config
// By default an SSD1306 driver is used which is most common for 0.96" 128x64 OLED displays
// 1.3" OLED 128x64 displays commonly uses an SH1106 driver.
// Choose driver below, only one can be defined!

#define OLED_DRIVER_SSD1306
// #define OLED_DRIVER_SH1106
// #define OLED_DRIVER_SH1107

// Optional setting for enabling Blink mode when in Manual/FX mode.
// Depending on the user preferences enabling blink can be a help
// to see that we have left the normal Preset mode. However on stage
// this can be irritating with a pedal flashing.
const bool ENABLE_FX_BLINK = false;

// Button GPIOs
enum ButtonGpio {
    BUTTON_PRESET1_GPIO = 25,
    BUTTON_DRIVE_GPIO = 25,

    BUTTON_PRESET2_GPIO = 26,
    BUTTON_MOD_GPIO = 26,

    BUTTON_PRESET3_GPIO = 32,
    BUTTON_DELAY_GPIO = 32,

    BUTTON_PRESET4_GPIO = 33,
    BUTTON_REVERB_GPIO = 33,

    BUTTON_BANK_DOWN_GPIO = 19,
    BUTTON_NOISEGATE_GPIO = 19,

    BUTTON_BANK_UP_GPIO = 18,
    BUTTON_COMP_GPIO = 18
};

// Button long press time
const int LONG_BUTTON_PRESS_TIME = 1000;

// LED GPIOs

// If the optional DEDICATED_PRESET_LEDS is defined below it will
// slightly alter the behaviour of Ignitron to make the FX and
// PRESET LEDs work independently. Also the bottom  line in the
// OLED display will no longer show FX symbols but instead
// show which *mode* Ignitron is in:
// (PRESET/FX MANUAL/LOOP-CONFIG/LOOP-CNTRL).
// The FX LEDS will show the actual FX pedals in use for each preset.
// The dedicated Preset LED GPIO pins LED_PRESET<n>_GPIO is defined
// separately below under the #ifdef DEDICATED_PRESET_LEDS clause.

// #define DEDICATED_PRESET_LEDS

#ifdef DEDICATED_PRESET_LEDS
enum LedGpio {
    LED_DRIVE_GPIO = 27,
    LED_MOD_GPIO = 13,
    LED_DELAY_GPIO = 16,
    LED_REVERB_GPIO = 14,
    LED_NOISEGATE_GPIO = 23,
    LED_COMP_GPIO = 17,

    LED_PRESET1_GPIO = 0,
    LED_PRESET2_GPIO = 4,
    LED_PRESET3_GPIO = 12,
    LED_PRESET4_GPIO = 15,
    LED_BANK_DOWN_GPIO = 23,
    LED_BANK_UP_GPIO = 17,

    LED_GPIO_INVALID = -1
};

#else
enum LedGpio {
    LED_DRIVE_GPIO = 27,
    LED_MOD_GPIO = 13,
    LED_DELAY_GPIO = 16,
    LED_REVERB_GPIO = 14,
    LED_NOISEGATE_GPIO = 23,
    LED_COMP_GPIO = 17,

    LED_PRESET1_GPIO = LED_DRIVE_GPIO,
    LED_PRESET2_GPIO = LED_MOD_GPIO,
    LED_PRESET3_GPIO = LED_DELAY_GPIO,
    LED_PRESET4_GPIO = LED_REVERB_GPIO,
    LED_BANK_DOWN_GPIO = LED_NOISEGATE_GPIO,
    LED_BANK_UP_GPIO = LED_COMP_GPIO,

    LED_GPIO_INVALID = -1
};

// If GPIO 0, 4, 12, 15 is physically connected in hardware but
// DEDICATED_PRESET_LEDS is undefined, we need to set those outputs
// to LOW. So we define an extra set of defines to allow them to be
// controlled
enum LedOptionalGpio {
    OPTIONAL_GPIO_1 = 0,
    OPTIONAL_GPIO_2 = 4,
    OPTIONAL_GPIO_3 = 12,
    OPTIONAL_GPIO_4 = 15
};
#endif

// LED/Button numbering
enum FxLedButtonNumber {
    DRIVE_NUM = 1,
    MOD_NUM = 2,
    DELAY_NUM = 3,
    REVERB_NUM = 4,
    NOISEGATE_NUM = 5,
    COMP_NUM = 6,
    INVALID_FX_BUTTON_NUM = -1
};

enum PresetLedButtonNum {
    PRESET1_NUM = 1,
    PRESET2_NUM = 2,
    PRESET3_NUM = 3,
    PRESET4_NUM = 4,
    BANK_DOWN_NUM = 5,
    BANK_UP_NUM = 6,
    INVALID_PRESET_BUTTON_NUM = -1
};

// Positions of FX types in Preset struct
enum FxType {
    INDEX_FX_NOISEGATE,
    INDEX_FX_COMP,
    INDEX_FX_DRIVE,
    INDEX_FX_AMP,
    INDEX_FX_MOD,
    INDEX_FX_DELAY,
    INDEX_FX_REVERB,
    INDEX_FX_INVALID = -1
};

// Spark button configs in APP mode
enum SubMode {
    SUB_MODE_FX,
    SUB_MODE_PRESET,
    SUB_MODE_LOOPER,
    SUB_MODE_SPK_LOOPER,
    SUB_MODE_LOOP_CONFIG,
    SUB_MODE_LOOP_CONTROL,
    SUB_MODE_TUNER
};

// Spark operation modes
enum OperationMode {
    SPARK_MODE_APP = 1,
    SPARK_MODE_AMP,
    SPARK_MODE_KEYBOARD
};

enum BTMode {
    BT_MODE_BLE = 1,
    BT_MODE_SERIAL
};

enum AmpType {
    AMP_TYPE_40,
    AMP_TYPE_MINI,
    AMP_TYPE_GO,
    AMP_TYPE_2
};

const string AMP_NAME_SPARK_40 = "Spark 40";
const string AMP_NAME_SPARK_MINI = "Spark MINI";
const string AMP_NAME_SPARK_GO = "Spark GO";
const string AMP_NAME_SPARK_2 = "Spark 2";
const string AMP_NAME_SPARK_NEO = "Spark NEO";

enum LooperCommand {
    SPK_LOOPER_CMD_COUNTIN = 0x02,
    SPK_LOOPER_CMD_REC = 0x04,
    SPK_LOOPER_CMD_STOP_REC = 0x05,
    SPK_LOOPER_CMD_RETRY = 0x06,
    SPK_LOOPER_CMD_REC_COMPLETE = 0x07,
    SPK_LOOPER_CMD_PLAY = 0x08,
    SPK_LOOPER_CMD_STOP = 0x09,
    SPK_LOOPER_CMD_DELETE = 0x0A,
    SPK_LOOPER_CMD_DUB = 0x0B,
    SPK_LOOPER_CMD_STOP_DUB = 0x0C,
    SPK_LOOPER_CMD_UNDO = 0x0D,
    SPK_LOOPER_CMD_REDO = 0x0E
};

enum LooperLedID {
    SPK_LOOPER_REC_DUB_LED_ID = 1,
    SPK_LOOPER_PLAY_STOP_LED_ID = 2,
    SPK_LOOPER_UNDO_REDO_LED_ID = 3,
    SPK_LOOPER_BPM_LED_ID = 5
};

#endif /* CONFIG_DEFINITIONS_H_ */
