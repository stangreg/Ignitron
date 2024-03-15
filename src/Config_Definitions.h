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


#define DEBUG
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
const string VERSION = "1.6.1";

// Button GPIOs
#define BUTTON_PRESET1_GPIO 	25
#define BUTTON_DRIVE_GPIO 		25

#define BUTTON_PRESET2_GPIO 	26
#define BUTTON_MOD_GPIO 		26

#define BUTTON_PRESET3_GPIO 	32
#define BUTTON_DELAY_GPIO 		32

#define BUTTON_PRESET4_GPIO 	33
#define BUTTON_REVERB_GPIO 		33

#define BUTTON_BANK_DOWN_GPIO 	19
#define BUTTON_NOISEGATE_GPIO 	19

#define BUTTON_BANK_UP_GPIO 	18
#define BUTTON_COMP_GPIO 		18

// Button long press time
#define LONG_BUTTON_PRESS_TIME 1000


// LED GPIOs
#define LED_PRESET1_GPIO 	27
#define LED_DRIVE_GPIO 		27

#define LED_PRESET2_GPIO 	13
#define LED_MOD_GPIO 		13

#define LED_PRESET3_GPIO 	16
#define LED_DELAY_GPIO 		16

#define LED_PRESET4_GPIO 	14
#define LED_REVERB_GPIO 	14

#define LED_BANK_DOWN_GPIO 	23
#define LED_NOISEGATE_GPIO 	23

#define LED_BANK_UP_GPIO 	17
#define LED_COMP_GPIO 		17

// LED/Button numbering
#define DRIVE_NUM 			1
#define PRESET1_NUM 		1

#define MOD_NUM 			2
#define PRESET2_NUM 		2

#define DELAY_NUM 			3
#define PRESET3_NUM			3

#define REVERB_NUM 			4
#define PRESET4_NUM 		4

#define NOISEGATE_NUM 		5
#define BANK_DOWN_NUM 		5

#define COMP_NUM 			6
#define BANK_UP_NUM 		6

// Positions of FX types in Preset struct
#define INDEX_FX_NOISEGATE	0
#define INDEX_FX_COMP		1
#define INDEX_FX_DRIVE		2
#define INDEX_FX_AMP		3
#define INDEX_FX_MOD		4
#define INDEX_FX_DELAY		5
#define INDEX_FX_REVERB		6


// Spark button configs in APP mode
#define SWITCH_MODE_FX 		1
#define SWITCH_MODE_PRESET 	2

// Spark operation modes
#define SPARK_MODE_APP 		1
#define SPARK_MODE_AMP 		2
#define SPARK_MODE_LOOPER 	3
#define SPARK_MODE_KEYBOARD 4

#define BT_MODE_BLE 		1
#define BT_MODE_SERIAL 		2

#define AMP_TYPE_40 		1
#define AMP_TYPE_MINI 		2
#define AMP_TYPE_GO 		3

#define AMP_NAME_SPARK_40	"Spark 40"
#define AMP_NAME_SPARK_MINI	"Spark MINI"
#define AMP_NAME_SPARK_GO	"Spark GO"

#endif /* CONFIG_DEFINITIONS_H_ */
