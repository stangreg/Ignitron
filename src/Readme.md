# Ignitron - Software
This document describes the software part of the **Ignitron** foot pedal.

## IDE
I am using the [Sloeber](https://eclipse.baeyens.it) Eclipse plugin for development as it offers more convenient functions than the standard Arduino IDE. Of course, you can use the IDE of your choice.
Whichever IDE you are using, you will need to set it up so it can build the code for the ESP32.
I assume you already know how to setup an ESP32 project in an IDE, only specifics I have used will be described here.

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

|Library|Version|Comment| Source
|---|---:|---|---|
| Adafruit SSD1306 | 2.5.7 | Display control | Arduino IDE Library |
| Adafruit BusIO | 1.14.5 | Bus IO | Arduino IDE Library |
| Adafruit_GFX_Library | 1.11.7 | Advanced display control | Arduino IDE Library |
| ArduinoJson | 6.21.3 | JSON document parsing | Arduino IDE Library |
| ButtonFever | 1.0.0 | Button library | Arduino IDE Library |
| Effortless-SPIFFS | 2.3.0 | File system to manage the presets | Arduino IDE Library |
| ESP32-BLE-Keyboard | 0.3.0 | Keyboard used for the looper app control | [GitHub](https://github.com/T-vK/ESP32-BLE-Keyboard) |
| FS | 1.0.0 | Basic File system library| Arduino IDE Library |
| NimBLE-Arduino  | 1.4.1 | BLE library | Arduino IDE Library |

## Stucture of the code
A (rudimentary and incomplete) doxygen documentation has been created [here](https://github.com/stangreg/Ignitron/blob/main/doxygen/html/index.html).\

### Class structure
| Class/File | Function |
|---|---|
| Ignitron.ino | Basic .ino file which only provides the setup() and loop() functions. It invokes the other control classes (see below) and controls the execution loop |
| SparkBLEControl | Controls the communication via Bluetooth LE protocol. Holds the connections to the Spark Amp and the App. |
| SparkBLEKeyboard | BLE Keyboard class, inherited from BleKeyboard. Adds start method to enable/disable easily |
| SparkButtonHandler | Registers HW button presses and delegates execution to the control classes. |
| **SparkDataControl** | **This is the core control class. It controls data flow and status across all other control classes.** |
| SparkDisplayControl | This controls which information is shown when on the display |
| SparkHelper | Few helper functions for byte manipulation |
| SparkLEDControl | Controls the LEDs depending on current status |
| SparkMessage | Builds command messages to be sent to the Spark Amp via BLE |
| SparkPresetBuilder | This transforms JSON file input to presets and vice versa, also builds the preset banks. |
| SparkStreamReader | Decoding of received data from Spark Amp or Spark App for further processing |
| SparkTypes | Container class to hold Preset and CommandData structs |
| Config_Definitions | Configuration items to map LEDs and buttons to GPIOs, enable DEBUG mode, and other technical definitions |

## Building the code
After setting up the project in the IDE, the code should build with the standard tool chain.

To compile, add the switch **`-D USE_NIMBLE`** to the compile options.

**Note:** Ignitron will work as a bluetooth keyboard in Looper mode. This could lead to your mobile OS keyboard to disappear, which can ben annoying. In order to prevent this, in the file "BleKeyboard.cpp" (part of ESP-BLE-Keyboard library) change line\
`USAGE(1),           0x06,`\
to `USAGE(1),           0x07,`\
in the `_hidReportDescriptor` array.

This will cause Ignitron to be treated like a **keypad** instead of a **keyboard**, which keeps your phone's keyboard active.

## Installing Firmware and data files
**Note:** With Arduino IDE version 2.x, plugins are not supported. This means you need to install Arduino IDE 1.8.x to follow the steps in this paragraph. Both versions 1.8.x and 2.x can be installed in parallel, so version 2.x can be used for development and version 1.8.x to use the plugins.

After building and installing the firmware on the board, it is required to also transfer the data directory to the board. In order to do so, you need to install the add-in `ESP32 Sketch Data Upload` to the **Arduino IDE**. Open the Ignitron sketch in the Arduino IDE and select `Data Upload` from the Tools menu. Please make sure to set the board settings correctly before uploading (see above). Unfortunately, there is no possibility to upload the data folder using Sloeber.

In case you don't like the default presets, you can delete the presets you don't want, and also change the file *PresetList.txt* accordingly. This file contains the file names of the presets to use. Lines starting with `--` are ignored.

**Note:** Be aware that when storing or deleting a preset to Ignitron using the AMP mode, the file is automatically rewritten, so custom commented lines will be removed.
