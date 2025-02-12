# Ignitron - Software
This document describes the software part of the **Ignitron** foot pedal.

## IDE
I am using the Visual Studio Code IDE with PlatfromIO plugin for development as it offers more convenient functions than the standard Arduino IDE. Of course, you can use the IDE of your choice.
Whichever IDE you are using, you will need to set it up so it can build the code for the ESP32.
I assume you already know how to setup an ESP32 project in an IDE, only specifics I have used will be described here. When you use PlatformIO, the config file of the project should already contain most settings, you might need to slightly adjust depending on the exact board you are using.

You need to add the board manager URL https://dl.espressif.com/dl/package_esp32_index.json

The following board configuration has been setup for the specific board used:
- Board: ESP 32 (Node32s)
- Upload protocol : Default
- Port: *select a serial port from the dropdown*
- Partition scheme: No OTA (Large Apps) (or similar)
- Flash frequency: 80 Mhz
- Core Debug Level: None
- Upload Speed: 921600

When configuring the Serial monitor, please make sure to select a baud rate of 115200, otherwise nothing will be visible.

### Libraries
There is a number of custom libraries used on top of the standard ones to use with ESP32.

| Library | Version | Comment | Source |
|---|---:|---|---|
| Adafruit SSD1306 | 2.5.13 | Display control | Arduino IDE Library |
| Adafruit SH110X | 2.1.11 | Display control | Arduino IDE Library |
| Adafruit BusIO | 1.14.5 | Bus IO | Arduino IDE Library |
| Adafruit_GFX_Library | 1.11.11 | Advanced display control | Arduino IDE Library |
| ArduinoJson | 7.3.0 | JSON document parsing | Arduino IDE Library |
| ButtonFever | 1.0.0 | Button library | Arduino IDE Library |
| ESP32-BLE-Keyboard | 0.3.2 | Keyboard used for the looper app control | [GitHub](https://github.com/T-vK/ESP32-BLE-Keyboard) |
| NimBLE-Arduino  | 1.4.1 | BLE library | Arduino IDE Library |

## Stucture of the code
A (rudimentary and incomplete) doxygen documentation has been created [here](https://github.com/stangreg/Ignitron/blob/main/doxygen/html/index.html).\

### Class structure
| Class/File | Function |
|---|---|
| Ignitron.ino | Basic .ino file which only provides the setup() and loop() functions. It invokes the other control classes (see below) and controls the execution loop |
| SparkBTControl | Controls the communication via Bluetooth LE and Serial protocol. Holds the connections to the Spark Amp and the App. |
| SparkBLEKeyboard | BLE Keyboard class, inherited from BleKeyboard. Adds start method to enable/disable easily |
| SparkButtonHandler | Registers HW button presses and delegates execution to the control classes. |
| **SparkDataControl** | **This is the core control class. It controls data flow and status across all other control classes.** |
| SparkDisplayControl | This controls which information is shown when on the display |
| SparkHelper | Few helper functions for byte manipulation |
| SparkLEDControl | Controls the LEDs depending on current status |
| SparkMessage | Builds command messages to be sent to the Spark Amp via BLE |
| SparkPresetBuilder | This transforms JSON file input to presets and vice versa, also builds the preset banks. |
| SparkPresetControl | Manages current status of active and pending presets and switching between presets. |
| SparkStatus | Holds the current messages received from the app (preset, number, looper status etc.). |
| SparkStreamReader | Decoding of received data from Spark Amp or Spark App for further processing |
| SparkTypes | Container class to hold Preset and CommandData structs |
| Config_Definitions | Configuration items to map LEDs and buttons to GPIOs, enable DEBUG mode, enable additional features, and other technical definitions |

## Building the code
After setting up the project in the IDE, the code should build with the standard tool chain.

To compile, add the switch **`-D USE_NIMBLE`** to the compile options.

**Note:** Ignitron will work as a bluetooth keyboard in Looper mode. This could lead to your mobile OS keyboard to disappear, which can ben annoying. In order to prevent this, in the file "BleKeyboard.cpp" (part of ESP-BLE-Keyboard library) change line\
`USAGE(1),           0x06,`\
to `USAGE(1),           0x07,`\
in the `_hidReportDescriptor` array.

This will cause Ignitron to be treated like a **keypad** instead of a **keyboard**, which keeps your phone's keyboard active.

## Installing Firmware and data files
After building and installing the firmware on the board, it is required to also transfer the data directory to the board. In order to do so, use the PlatformIO targets 'Build Filesystem Image' and 'Upload Filesystem image'. You might need to specify the correct partitioning in the platformio.ini file. Please make sure to set the board settings correctly before uploading (see above). 

In case you don't like the default presets, you can delete the presets you don't want, and also change the file *PresetList.txt* accordingly. This file contains the file names of the presets to use. Lines starting with `--` are ignored.

**Note:** Be aware that when storing or deleting a preset to Ignitron using the AMP mode, the file is automatically rewritten, so custom commented lines will be removed.

## Additional Features

### Battery Indicator

Starting from version 1.7.2, Ignitron can be powered by an internal battery pack and displays the remaining capacity on the LED screen.

Benefits of using an internal battery:

- No AC adapter, fewer cables, and no buzzing from the AC adapter.
- Can use the battery to power other pedals.

To enable this feature:

1. Follow the battery indicator section of the hardware guide in [hardware/README.md](../hardware/README.md#battery-indicator) and wire the hardware components.
2. Go to [src/Config_Definitions.h](Config_Definitions.h) and uncomment `#define ENABLE_BATTERY_STATUS_INDICATOR` by removing the `//` at the beginning of that line.
3. Select the type of battery you are using. The default settings are for Li-ion batteries. If you are using LiFePO4 batteries, change the `#define BATTERY_TYPE` line to `#define BATTERY_TYPE BATTERY_TYPE_LI_FE_PO4`.
4. Select the series type. The default is for 3S batteries. Change the `#define BATTERY_CELLS` line to `#define BATTERY_CELLS 2` if you are using a 2S battery pack.
5. If you have custom resistors for the voltage divider, change `#define VOLTAGE_DIVIDER_R1` and `#define VOLTAGE_DIVIDER_R2` accordingly.
6. Build and flash the ESP32 with the new settings.

There will be a battery indicator in APP Mode with four states for different levels: 0-10%, 10-50%, 50-90%, 90-100%.

### SH1106 1.3" OLED Display driver

The SH1106 driver is already linked in the Libraries section of the platform.ini file, but it needs to be activated in code since the two drivers uses different handles and somewhat different API. By default the SSD1306 driver is active but can be exchanged to the SH1106.

Go to [src/Config_Definitions.h](Config_Definitions.h) and enable **one of** `#define OLED_DRIVER_SSD1306` or `#define OLED_DRIVER_SH1106` by moving the `//` at the beginning of the line to the definition that should be **disabled**. 

### Dedicated Preset LEDs

Go to [src/Config_Definitions.h](Config_Definitions.h) and uncomment the line `#define DEDICATED_PRESET_LEDS`.
This will enable the use of 4 new GPIO pins (0, 4, 12, 15) and related hardware LEDs and resistors as described in the hardware Readme. This also enables a different behavior in **Ignitron** as it will light up the corresponding FX LEDs for the active pedals when a preset is selected, instead of displaying the FX symbols in the OLED display (which is redundant in this config). The bottom line of the OLED display will also show the current mode that is active (Preset mode|Manual FX|Looper mode). 