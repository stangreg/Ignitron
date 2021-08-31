# Sparky
An ESP32 module to communicate with the Spark Amp

## Operation modes
The Sparky has two operation modes, APP mode and AMP mode.
In APP mode, the pedal is in standard operation mode. It behaves like the Spark App towards the Spark Amp and can then be used to switch between presets and/or switch effects on and off.
AMP mode can be used to manage the presets stored on the Sparky, presets can be added from the app or be deleted from Sparky.

## APP mode
In APP mode, the foot switches can be used to either switch between pre-saved presets (Preset mode) or to control all single effects in the selected preset (FX mode). Modes can easily be switched by long pressing the Bank-Up button.
When selecting Preset mode, four buttons are used to select presets. The other two buttons can be used to navigate through different banks with presets. This way the user has access to a big number of saved presets. When pressing the foot switch of the current active preset, the effect configured in the DRIVE section can be enabled or disabled.
In FX mode, the user has direct access to all effects of the selected preset. 
Each switch controls a different FX pedal type:
