/*
 * Common.h
 *
 *  Created on: 26.11.2021
 *      Author: steffen
 */

#ifndef CONFIG_DEFINITIONS_H_
#define CONFIG_DEFINITIONS_H_

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
const string VERSION = "1.7.6";

// Battery indicator
// Note: This battery is for the Ignitron controller, not the Spark amp.
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

// #define ENABLE_BATTERY_STATUS_INDICATOR

#ifdef ENABLE_BATTERY_STATUS_INDICATOR
#define BATTERY_VOLTAGE_ADC_PIN 36
#define BATTERY_LEVEL_0 0 // 0-10%
#define BATTERY_LEVEL_1 1 // 10-50%
#define BATTERY_LEVEL_2 2 // 50-90%
#define BATTERY_LEVEL_3 3 // 90-100%

#define BATTERY_TYPE_LI_ION 0
#define BATTERY_TYPE_LI_FE_PO4 1

#define BATTERY_TYPE BATTERY_TYPE_LI_ION // Choose from BATTERY_TYPE_LI_ION or BATTERY_TYPE_LI_FE_PO4
#define BATTERY_CELLS 3
#define VOLTAGE_DIVIDER_R1 (5.1 * 1000) // 5.1k ohm
#define VOLTAGE_DIVIDER_R2 (15 * 1000)  // 15k ohm

#if BATTERY_TYPE == BATTERY_TYPE_LI_ION
#define BATTERY_CAPACITY_VOLTAGE_THRESHOLD_90 (BATTERY_CELLS * 4.1)
#define BATTERY_CAPACITY_VOLTAGE_THRESHOLD_50 (BATTERY_CELLS * 3.7)
#define BATTERY_CAPACITY_VOLTAGE_THRESHOLD_10 (BATTERY_CELLS * 3.3)
#elif BATTERY_TYPE == BATTERY_TYPE_LI_FE_PO4
#define BATTERY_CAPACITY_VOLTAGE_THRESHOLD_90 (BATTERY_CELLS * 3.35)
#define BATTERY_CAPACITY_VOLTAGE_THRESHOLD_50 (BATTERY_CELLS * 3.26)
#define BATTERY_CAPACITY_VOLTAGE_THRESHOLD_10 (BATTERY_CELLS * 3.0)
#endif

#endif

// Button GPIOs
#define BUTTON_PRESET1_GPIO 25
#define BUTTON_DRIVE_GPIO 25

#define BUTTON_PRESET2_GPIO 26
#define BUTTON_MOD_GPIO 26

#define BUTTON_PRESET3_GPIO 32
#define BUTTON_DELAY_GPIO 32

#define BUTTON_PRESET4_GPIO 33
#define BUTTON_REVERB_GPIO 33

#define BUTTON_BANK_DOWN_GPIO 19
#define BUTTON_NOISEGATE_GPIO 19

#define BUTTON_BANK_UP_GPIO 18
#define BUTTON_COMP_GPIO 18

// Button long press time
#define LONG_BUTTON_PRESS_TIME 1000

// LED GPIOs
#define LED_PRESET1_GPIO 27
#define LED_DRIVE_GPIO 27

#define LED_PRESET2_GPIO 13
#define LED_MOD_GPIO 13

#define LED_PRESET3_GPIO 16
#define LED_DELAY_GPIO 16

#define LED_PRESET4_GPIO 14
#define LED_REVERB_GPIO 14

#define LED_BANK_DOWN_GPIO 23
#define LED_NOISEGATE_GPIO 23

#define LED_BANK_UP_GPIO 17
#define LED_COMP_GPIO 17

// LED/Button numbering
#define DRIVE_NUM 1
#define PRESET1_NUM 1

#define MOD_NUM 2
#define PRESET2_NUM 2

#define DELAY_NUM 3
#define PRESET3_NUM 3

#define REVERB_NUM 4
#define PRESET4_NUM 4

#define NOISEGATE_NUM 5
#define BANK_DOWN_NUM 5

#define COMP_NUM 6
#define BANK_UP_NUM 6

// Positions of FX types in Preset struct
#define INDEX_FX_NOISEGATE 0
#define INDEX_FX_COMP 1
#define INDEX_FX_DRIVE 2
#define INDEX_FX_AMP 3
#define INDEX_FX_MOD 4
#define INDEX_FX_DELAY 5
#define INDEX_FX_REVERB 6

// Spark button configs in APP mode
#define BUTTON_MODE_FX 1
#define BUTTON_MODE_PRESET 2
#define BUTTON_MODE_LOOP_CONFIG 3
#define BUTTON_MODE_LOOP_CONTROL 4

// Spark operation modes
#define SPARK_MODE_APP 1
#define SPARK_MODE_AMP 2
#define SPARK_MODE_LOOPER 3
#define SPARK_MODE_KEYBOARD 4
#define SPARK_MODE_SPK_LOOPER 5
#define SPARK_MODE_TUNER 6

#define BT_MODE_BLE 1
#define BT_MODE_SERIAL 2

#define AMP_TYPE_40 1
#define AMP_TYPE_MINI 2
#define AMP_TYPE_GO 3
#define AMP_TYPE_2 4

#define AMP_NAME_SPARK_40 "Spark 40"
#define AMP_NAME_SPARK_MINI "Spark MINI"
#define AMP_NAME_SPARK_GO "Spark GO"
#define AMP_NAME_SPARK_2 "Spark 2"

#define SPK_LOOPER_CMD_COUNTIN 0x02
#define SPK_LOOPER_CMD_REC 0x04
#define SPK_LOOPER_CMD_STOP_REC 0x05
#define SPK_LOOPER_CMD_RETRY 0x06
#define SPK_LOOPER_CMD_REC_COMPLETE 0x07
#define SPK_LOOPER_CMD_PLAY 0x08
#define SPK_LOOPER_CMD_STOP 0x09
#define SPK_LOOPER_CMD_DELETE 0x0A
#define SPK_LOOPER_CMD_DUB 0x0B
#define SPK_LOOPER_CMD_STOP_DUB 0x0C
#define SPK_LOOPER_CMD_UNDO 0x0D
#define SPK_LOOPER_CMD_REDO 0x0E

#define SPK_LOOPER_BPM_LED_ID 5
#define SPK_LOOPER_REC_DUB_LED_ID 1
#define SPK_LOOPER_PLAY_STOP_LED_ID 2
#define SPK_LOOPER_UNDO_REDO_LED_ID 3

#endif /* CONFIG_DEFINITIONS_H_ */
