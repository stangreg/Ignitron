# Ignitron - Software
This document describes the software side of the **Ignitron** foot pedal.

## IDE
I am using the [Sloeber](http://sloeber.io) Eclipse plugin for development as it offers more convenient functions than the standard Arduino IDE. Of course, you can use the IDE of your choice.
Whichever IDE you are using, you will need to set it up so it can build the code for the ESP32.
I assume you already know how to setup an ESP32 project in an IDE, only specifics I have used will be described here.

You need to add the board manager URL https://dl.espressif.com/dl/package_esp32_index.json

The following board configuration has been setup for the specific board used:
- Board: ESP 32 (Node32s)
- Upload protocol : Default
- Port: *select a serial port from the dropdown*
- Partition scheme: Minimal SPIFFS (Large APPS with OTA)
- Flash frequency: 80 Mhz
- Core Debug Level: None
- Upload Speed: 921600

When configuring the Serial monitor, please make sure to select a baud rate of 115200, otherwise nothing will be visible.

### Libraries
There is a number of custom libraries used on top of the standard ones to use with ESP32. 

|Library|Version|Comment|
|---|---:|---|
| Adafruit SSD1306 | 2.4.6 | Display control|
| Adafruit_GFX_Library | 1.10.12 | Advanced display control |
| ArduinoJson | 6.8.15 | JSON document parsing |
| ButtonFever | 1.0.0 | Button library |
| Effortless-SPIFFS | 2.3.0 | File system to manage the presets |
| ESP32-BLE-Keyboard | 0.3.0 | Keyboard used for the looper app control |
| ESPmDNS | 1.0.0| mDNS used for the OTA web server |
| FS | 1.0.0 | Basic File system library|
| NimBLE-Arduino  | 1.3.1 | BLE library |
| RegExp  | 1.0.6 | Used for manipulate preset file names |
| Update | 1.0.0 | OTA Update |
| WebServer | 1.0.0 | Webserver for OTA updates |
| WiFi | 1.2.7 | WiFi to enable OTA updates |

## Stucture of the code
A (rudimentary and incomplete) doxygen documentation has been created [here](https://github.com/stangreg/Ignitron/blob/main/doxygen/html/index.html)
Class structure:

| Class/File | Function |
|---|---|
| Ignitron.ino | Basic .ino file which only provides the setup() and loop() functions. It invokes the other control classes (see below) and controls the execution loop |
| SparkBLEControl | Controls the communication via Bluetooth LE protocol. Holds the connections to the Spark Amp and the App. |
| SparkButtonHandler | Registers HW button presses and delegates execution to the control classes. |
| **SparkDataControl** | **This is the core control class. It controls data flow and status across all other control classes.** |
| SparkDisplayControl | This controls which information is shown when on the display |
| SparkHelper | Few helper functions for byte manipulation |
| SparkLEDControl | Controls the LEDs depending on current status |
| SparkMessage | Builds command messages to be sent to the Spark Amp via BLE |
| SparkOTAServer | This is a (rudimentary) server to enable OTA firmware updates |
| SparkPresetBuilder | This transforms JSON file input to presets and vice versa, also builds the preset banks. | 
| SparkStreamReader | Decoding of received data from Spark Amp or Spark App for further processing |
| SparkTypes | Container class to hold Preset and CommandData sructs |
| Credentials.h.template | This is a template file, see below before building. |

## Building the code
**Important:** Before building the code, rename the file to Credentials.h and put in your SSID name and WiFi password.
After setting up the project in the IDE, the code should build with the standard tool chain.
