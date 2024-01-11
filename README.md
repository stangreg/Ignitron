<p align="center">
  <img width="387" height="141" src="https://github.com/stangreg/Ignitron/blob/main/resources/Ignitron_logo_large.png">
</p>

An ESP32 based foot pedal to communicate with the Spark Amp and Spark App via Bluetooth LE (iOS) and Bluetooth Serial (Android). Also works as a control device for a Looper app on the mobile.
**Ignitron works with all Spark Devices** (Spark 40, Spark Mini, Spark GO).

**Ignitron** gives you full control over your Spark Amp:
* Switch between the four **hardware presets**
* Switch between **custom saved presets** organized in banks
* **Toggle effects** for the selected preset
* **Save new presets** to Ignitron via the Spark App
* **Delete** stored presets
* Control **Looper apps** on a mobile (and other apps which support bluetooth keyboard controls)

Adding new presets to **Ignitron** can easily be done as it can also act like a Spark Amp. Simply connect to **Ignitron** with the Spark App and save new presets directly from ToneCloud (or your downloaded presets) to **Ignitron**.

One big advantage of **Ignitron** is that it communicates with the Spark Amp using the **Bluetooth LE** protocol.
This means it is possible to connect your mobile to your Spark Amp as an audio speaker and play along your favorite songs while you control your effects with Ignitron.

The current active preset and effects are indicated via LEDs.\
In addition, the **built-in display** provides information on
* selected bank and preset number
* selected preset name
* operation mode (Preset, Manual/FX, Looper)
* activated pedal types in the FX chain
* Bluetooth connection status to Spark Amp and Spark App

This work was inspired by the [Sparkpal project](https://github.com/jamesguitar3/sparkpal), and the [Python Spark Parser project](https://github.com/paulhamsh/Spark-Parser) helped me to understand the messages being sent to and from the Spark Amp.

The following sections show how to [build](https://github.com/stangreg/SparkBLE#Building-Ignitron) and [operate](https://github.com/stangreg/SparkBLE#Operating-Ignitron) Ignitron.

<p align="center">
  <img src="https://github.com/stangreg/Ignitron/blob/main/resources/Ignitron_with_Spark.JPG" width=200>
  <img src="https://github.com/stangreg/Ignitron/blob/main/resources/Ignitron_preset_mode.JPG" width=200>
  <img src="https://github.com/stangreg/Ignitron/blob/main/resources/Ingitron_FX_Mode2.jpg" width=200>
  <img src="https://github.com/stangreg/Ignitron/blob/main/resources/Ignitron_full_wiring.JPG" width=200>
</p>

# Building Ignitron
Software: [click here](https://github.com/stangreg/Ignitron/blob/main/src/)

Hardware: [click here](https://github.com/stangreg/Ignitron/blob/main/hardware/)

# Operating Ignitron

## Table of contents
* [Operation modes](https://github.com/stangreg/SparkBLE#operation-modes)
* [APP mode](https://github.com/stangreg/SparkBLE#app-mode)
* [Keyboard mode](https://github.com/stangreg/SparkBLE#keyboard-mode)
* [AMP mode](https://github.com/stangreg/SparkBLE#amp-mode)
  * [Storing a new preset](https://github.com/stangreg/SparkBLE#storing-a-new-preset-a-preset-from-the-app-needs-to-be-loaded)
  * [Deleting an existing preset](https://github.com/stangreg/SparkBLE#deleting-a-preset-only-possible-with-no-preset-from-the-app-loaded)
  * [Manually creating presets](https://github.com/stangreg/SparkBLE#manually-creating-presets)
    * [Preset format](https://github.com/stangreg/SparkBLE#preset-format)
    * [FX parameter reference](https://github.com/stangreg/SparkBLE#fx-parameter-reference)
* [OTA Update](https://github.com/stangreg/SparkBLE#OTA-Update)


## Operation modes
**Ignitron** has two operation modes, **APP mode** and **AMP mode**.\
In **APP mode** (default mode on startup), **Ignitron** connects to a Spark Amp and behaves like the Spark App towards the Spark Amp. It can then be used to switch between saved presets, toggle FX pedals, and even connect it to a Looper app on a mobile (providing 6 buttons to control a (looper or tabs) app).

**AMP mode** can be used to manage the presets stored on **Ignitron**, presets can be saved via the app or be deleted from **Ignitron**. For Android, the Bluetooth mode has to be set to Serial (SRL), for iOS it has to be set to BLE.\

**Keyboard mode** is useful when Ignitron should act as a bluetooth keyboard control without controlling a Spark Amp. This provides 2x6=12 buttons to control any supporting app (e.g., a looper or a tabs app).

To enter **AMP mode**, hold the `Preset 1` button during startup.
To enter **Keyboard mode**, hold the `Preset 3` button during startup.

Amp mode and Keyboard mode can best be started by holding the respective button pressed while pressing and holding `Preset 2` button to reset.

A graphical overview of all modes with button mapping can be found [here](https://github.com/stangreg/Ignitron/blob/main/resources/ignitron-cheatsheet.pdf). (Thanks to [@itarozzi](https://www.github.com/itarozzi) for creating this.)

## APP mode
In APP mode, the foot switches can be used to either switch between pre-saved presets (**Preset mode**), control all single effects in the selected preset (**Manual/FX mode**), or switch between presets while controlling an app on your mobile, e.g. a Looper app (**Looper mode**). You can easily toggle between **Preset mode** and **Manual/FX mode** by long pressing the `Bank-Up` button. To toggle between **Preset mode** and **Looper mode**, simply long-press the `Bank down` button.

When selecting **Preset mode**, four buttons are used to select presets, the other two buttons are used to navigate through the preset banks. This way the user has access to a huge number of saved presets. When pressing the foot switch of the current active preset, the effect configured in the DRIVE section can be toggled.\
In **Manual/FX mode**, the user has direct access to all effects of the selected preset. Pressing a switch will toggle the respective FX type.\
When **Looper mode** is activated, Ignitron acts partially like a bluetooth keyboard. You can use the buttons to control a looper app, e.g. Loopy HD, and also switch between stored presets by long pressing the `Bank down`/`Bank up` buttons. **Note:** When switching presets in Looper mode, it will do this across banks.  


### Button commands in APP mode:

#### Preset mode
|Button      | Press pattern | Function           |
|----------- | :-----------: | ------------------ |
|`Bank down` | Short         | Navigate bank up   |
|`Bank up`   | Short         | Navigate bank down |
|`Preset 1`  | Short         | Select preset 1 / Toggle Drive |
|`Preset 2`  | Short         | Select preset 2 / Toggle Drive |
|`Preset 3`  | Short         | Select preset 3 / Toggle Drive |
|`Preset 4`  | Short         | Select preset 4 / Toggle Drive |
|`Preset 4` | Long           | Switch to Looper mode |
|`Bank up`   | Long          | Switch to FX mode  |
|`Preset 2`  | Long          | Restart **Ignitron**  |

#### Manual/FX mode
|Button      | Press pattern | Function              |
|----------- | :-----------: | --------------------- |
|`Bank down` | Short         | Toggle Noise Gate     |
|`Bank up`   | Short         | Toggle Compressor/Wah |
|`Preset 1`  | Short         | Toggle Drive          |
|`Preset 2`  | Short         | Toggle Modulation     |
|`Preset 3`  | Short         | Toggle Delay          |
|`Preset 4`  | Short         | Toggle Reverb         |
|`Bank up`   | Long          | Switch to Preset mode |
|`Preset 2`  | Long          | Restart **Ignitron**  |

#### Looper mode (combined with Preset functionality)
|Button      | Press pattern | Function           |
|----------- | :-----------: | ------------------ |
|`Preset 1`  | Short         | Sends character "A" (can be freely configured in app) |
|`Preset 2`  | Short         | Sends character "B" (can be freely configured in app) |
|`Preset 3`  | Short         | Sends character "C" (can be freely configured in app) |
|`Preset 4`  | Short         | Sends character "D" (can be freely configured in app) |
|`Bank down` | Short         | Sends character "E" (can be freely configured in app) |
|`Bank up`   | Short         | Sends character "F" (can be freely configured in app) |
|`Bank down` | Long          | Switch to previous preset (across banks)  |
|`Bank up` | Long          | Switch to next preset (across banks)  |
|`Preset 4` | Long          | Switch to Preset mode |
|`Preset 2`  | Long          | Restart **Ignitron**     |

***Note:*** *In Looper mode, Ignitron is connected to your mobile as a bluetooth keyboard. If supported by the respective app on the mobile, the buttons can be freely configured to any function offered by the respective App. You may need to manually connect to the Ignitron BLE keyboard in your Bluetooth settings after switching to Looper mode.*

Example Button setup (available commands depend on availability in the respective app):
|Button      | Function           |
|----------- | ------------------ |
|`Preset 1`  | Record/overdub current track |
|`Preset 2`  | Play/pause session |
|`Preset 3`  | Delete current track (press twice) |
|`Preset 4`  | Mute current track |
|`Bank down` | Switch to previous track |
|`Bank up`   | Switch to next track |

-----------------------------------------------

## Keyboard mode
This mode will turn the Ignitron into a Bluetooth LE keyboard. All buttons will act like keyboard presses, there is no functionality regarding Preset change or other Spark Amp related functions. Use this mode if you want to only use Ignitron as a looper control device (or any other supporting app like a tabs app) . If you want to use looper control in combination with Spark Amp control, use the Looper mode as part of the APP mode (see above).

In keyboard mode, the following keys will be sent to the connected device:

|Button      | Press pattern | Function           |
|----------- | :-----------: | ------------------ |
|`Preset 1`  | Short         | Sends character "1" (function can be freely configured in app) |
|`Preset 2`  | Short         | Sends character "2" (function can be freely configured in app) |
|`Preset 3`  | Short         | Sends character "3" (function can be freely configured in app) |
|`Preset 4`  | Short         | Sends character "4" (function can be freely configured in app) |
|`Bank down` | Short         | Sends character "5" (function can be freely configured in app) |
|`Bank up`   | Short         | Sends character "6" (function can be freely configured in app) |
|`Preset 1`  | Long          | Sends character "A" (function can be freely configured in app) |
|`Preset 2`  | Long          | Sends character "B" (function can be freely configured in app) |
|`Preset 3`  | Long          | Sends character "C" (function can be freely configured in app) |
|`Preset 4`  | Long          | Sends character "D" (function can be freely configured in app) |
|`Bank down` | Long          | Sends character "E" (function can be freely configured in app) |
|`Bank up`   | Long          | Sends character "F" (function can be freely configured in app) |


-----------------------------------------------

## AMP mode
In AMP mode, **Ignitron** acts like a Spark AMP and can communicate with the Spark app running on a mobile. New presets can be stored on **Ignitron** and existing presets can be deleted.

### Connecting the Spark app with **Ignitron**

***Note:*** By default, Ignitron is configured to use BLE which only works with iOS. In order to connect an Android device, you need to switch to Serial Bluetooth. To do so, press and hold the `Bank Up` button when in AMP mode (after step 1. below). This will toggle the Bluetooth mode and restart **Ignitron**. After toggling the Bluetooth mode, start with step 1. below again. Ignitron persists the Bluetooth mode over restarts.  

1. Start **Ignitron** in AMP mode (hold `Preset 1` button during startup).
2. Open the Spark App on the mobile
3. For the first connection, make sure to have your real Spark Amp powered off to avoid the app connecting to it.
4. Hit the connect button in the app (or the + button in the connection overview)
5. Once the connection is established, you can give the connection a name in the app so you can distinguish from your regular Spark Amp connection.

### Storing a new preset (a preset from the app needs to be loaded)
1. Start **Ignitron** in AMP mode (hold Preset 1 button during startup).
2. Connect the Spark App with **Ignitron** (see above)
3. Select a preset in the app (either saved in the app or from ToneCloud).
4. The preset name should appear in the bottom line of the display
5. The `Preset`/`Bank` buttons on **Ignitron** can be used to navigate to the desired preset position.
6. Press the `Preset` button to mark the position for storing *(the LED of the selected preset position should start blinking)*
7. Hit the same `Preset` button a second time to confirm storage
(Hitting any other `Preset`/`Bank` button will revert the state back to navigating)
8. The preset will be saved to the selected position, other presets will be pushed one position back

#### Button commands (case: preset has been received from Spark app)
| Button     | Press pattern | Function                        | Remark |
| ---------- | :-----------: | ------------------------------- | ------ |
|`Bank down` | Short         | Navigate bank up                |        |
|`Bank up`   | Short         | Navigate bank down              |        |
|`Preset 1`  | Short         | Select preset 1                 | *Press twice to store received preset* |
|`Preset 2`  | Short         | Select preset 2                 | *Press twice to store received preset* |
|`Preset 3`  | Short         | Select preset 3                 | *Press twice to store received preset* |
|`Preset 4`  | Short         | Select preset 4                 | *Press twice to store received preset* |
|`Bank down` | Long          | Unload preset                   | *Removes loaded preset if present*     |
|`Bank up`   | Long          | Toggle Bluetooth mode                   | *Toggle between BLE and BT Serial mode*  |
|`Preset 2`  | Long          | Restart **Ignitron**                  |        |


### Deleting a preset (only possible when no preset from the app is loaded)
1. Start **Ignitron** in AMP mode (hold `Preset 1` button during startup).
2. Use the `Preset`/`Bank` buttons on **Ignitron** to navigate to the desired preset position
3. **Long press** the `Bank Down` button to mark the selected preset for deletion
4. The LED of the selected preset should start blinking
5. In the display you should see a prompt if deletion should be executed
6. If you want to cancel the deletion, simply short press any other button
7. Otherwise, **long press** the `Bank Down` button again to confirm deletion.
(Hitting any other button will cancel the deletion and return back to navigation)

#### Button commands (case: no preset has been received, slot is empty)
| Button     | Press pattern | Function                 | Remark |
| ---------- | :-----------: | ------------------------ | ------ |
|`Bank down` | Short         | Navigate bank up         |        |
|`Bank up`   | Short         | Navigate bank down       |        |
|`Preset 1`  | Short         | Select preset 1          |        |
|`Preset 2`  | Short         | Select preset 2          |        |
|`Preset 3`  | Short         | Select preset 3          |        |
|`Preset 4`  | Short         | Select preset 4          |        |
|`Bank down` | Long          | Mark preset for deletion | *Pressing any other button in that state will cancel the deletion* |
|`Bank down` | Long          | Delete marked preset     | *Only if preset was first marked for deletion* |
|`Bank up` | Long          | Toggle Bluetooth mode                   | *Toggle between BLE and BT Serial mode*  |
|`Preset 2`  | Long          | Restart **Ignitron**           |        |

-------------------------------------------------------

### Manually creating presets
*This method is only recommended if a preset cannot be transferred via the Spark app or if the preset files have been received/generated in a different way.*
1. Create a **preset file** in JSON format as shown below
2. Make sure the **file name** does not exceed 31 characters (including the `.json` suffix)
3. Put the file name into the **data folder**
4. Insert the file name (including the `.json` suffix) to the desired prefix location in the `PresetList.txt` file
5. *Place the PresetList.txt file into the data folder (if not already there)*
6. **Upload the data folder** to **Ignitron** via the Arduino IDE (or other tools)

#### Preset format
**Ignitron** stores presets in a JSON format using the SPIFFS file system.
Each preset is stored in a separate file and presets are organized in a separate text file called 'PresetList.txt'. This list simply stores the file names of the presets, the order defines the way the banks are filled.
An example preset file would look like this:
```
{"PresetNumber": 127, "UUID":"DEFBB271-B3EE-4C7E-A623-2E5CA53B6DDA",
"Name":"Studio Session" , "Version":"0.7", "Description":"Description for Acoustic Preset 1", "Icon":"icon.png", "BPM": 120.0000,
"Pedals": [
  { "Name":"bias.noisegate",  "IsOn": false,  "Parameters":[0.5000,0.3467] },
  { "Name":"BBEOpticalComp",  "IsOn": true,   "Parameters":[0.7583,0.2585,0.0000] },
  { "Name":"DistortionTS9",   "IsOn": false,  "Parameters":[0.1396,0.4073,0.6898] },
  { "Name":"Acoustic",  "IsOn": true,   "Parameters":[0.6398,0.3851,0.3834,0.5994,0.5195] },
  { "Name":"ChorusAnalog",  "IsOn": true,   "Parameters":[0.8417,0.2275,0.9359,0.3513] },
  { "Name":"DelayMono",   "IsOn": false,  "Parameters":[0.2240,0.2112,0.4909,0.6000,1.0000] },
  { "Name":"bias.reverb",   "IsOn": true,   "Parameters":[0.7228,0.3262,0.2758,0.3607,0.3439,0.4860,0.4000] } ],
"Filler":"23"
}
```

As a guidance which effect and amp names can be used and in which order the parameters of each effect have to be given, please see below.

#### FX parameter reference
In order to know which effects are available with paramters, see below table.
This data can be used to build own presets in JSON format (see above).
Use the Technical Name information in the JSON files.
Parameters marked with `Switch` can only have values of 0 or 1, others can have any value between 0 and 1.

##### Standard Tones

| Type       | App&nbsp;Name           | Technical&nbsp;Name&nbsp;(JSON)    | Parameter&nbsp;0             | Parameter&nbsp;1             | Parameter&nbsp;2         | Parameter&nbsp;3    | Parameter&nbsp;4  | Parameter&nbsp;5 | Parameter&nbsp;6         | Extra&nbsp;Info                    |
|------------|--------------------|-------------------|-------------------------|-------------------------|---------------------|----------------|--------------|-------------|---------------------|-------------------------------|
| Noise Gate | Noise Gate         | bias.noisegate    | Threshold               | Decay                   |                     |                |              |             |                     |                               |
| Compressor | LA Comp            | LA2AComp          | Limit/Compress `Switch` | Gain                    | Peak Reduction      |                |              |             |                     |                               |
| Compressor | Sustain Comp       | BlueComp          | Level                   | Tone                    | Attack              | Sustain        |              |             |                     |                               |
| Compressor | Red Comp           | Compressor        | Output                  | Sensitivity             |                     |                |              |             |                     |                               |
| Compressor | Bass Comp          | BassComp          | Comp                    | Gain                    |                     |                |              |             |                     |                               |
| Compressor | Optical Comp       | BBEOpticalComp    | Volume                  | Comp                    | Pad `Switch`        |                |              |             |                     |                               |
| Drive      | Booster            | Booster           | Gain                    |                         |                     |                |              |             |                     |                               |
| Drive      | Clone Drive        | KlonCentaurSilver | Output                  | Treble                  | Gain                |                |              |             |                     |                               |
| Drive      | Tube Drive         | DistortionTS9     | Overdrive               | Tone                    | Level               |                |              |             |                     |                               |
| Drive      | Over Drive         | Overdrive         | Level                   | Tone                    | Drive               |                |              |             |                     |                               |
| Drive      | Fuzz Face          | Fuzz              | Volume                  | Fuzz                    |                     |                |              |             |                     |                               |
| Drive      | Black Op           | ProCoRat          | Distortion              | Filter                  | Volume              |                |              |             |                     |                               |
| Drive      | Bass Muff          | BassBigMuff       | Volume                  | Tone                    | Sustain             |                |              |             |                     |                               |
| Drive      | Guitar Muff        | GuitarMuff        | Volume                  | Tone                    | Sustain             |                |              |             |                     |                               |
| Drive      | Bassmaster         | MaestroBassmaster | Brass Volume            | Sensitivity             | Bass Volume         |                |              |             |                     |                               |
| Drive      | SAB Driver         | SABDriver         | Volume                  | Tone                    | Drive               | LP/HP `Switch` |              |             |                     |                               |
| Amp        | Silver 120         | RolandJC120       | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Black Duo          | Twin              | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | AD Clean           | ADClean           | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Match DC           | 94MatchDCV2       | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | ODS 50             | ODS50CN           | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Blues Boy          | BluesJrTweed      | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Tweed Bass         | Bassman           | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | AC Boost           | AC Boost          | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Checkmate          | Checkmate         | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Two Stone SP50     | TwoStoneSP50      | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | American Deluxe    | Deluxe65          | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Plexiglas          | Plexi             | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | JM45               | OverDrivenJM45    | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Lux Verb           | OverDrivenLuxVerb | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | RB 101             | Bogner            | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | British 30         | OrangeAD30        | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | American High Gain | AmericanHighGain  | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | SLO 100            | SLO100            | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | YJM100             | YJM100            | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Treadplate         | Rectifier         | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Insane             | EVH               | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Insane 6508        | 6505Plus          | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | SwitchAxe          | SwitchAxeLead     | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Rocker V           | Invader           | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | BE 101             | BE101             | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Pure Acoustic      | Acoustic          | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Fishboy            | AcousticAmpV2     | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Jumbo              | FatAcousticV2     | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Flat Acoustic      | FlatAcoustic      | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | RB-800             | GK800             | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Sunny 3000         | Sunny3000         | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | W600               | W600              | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Amp        | Hammer 500         | Hammer500         | Gain                    | Treble                  | Middle              | Bass           | Volume       |             |                     |                               |
| Modulation | Tremolo            | Tremolo           | Speed                   | Depth                   | Level               |                |              |             |                     |                               |
| Modulation | Chorus             | ChorusAnalog      | E.Level                 | Rate                    | Depth               | Tone           |              |             |                     |                               |
| Modulation | Flanger            | Flanger           | Rate                    | Mix                     | Depth               |                |              |             |                     |                               |
| Modulation | Phaser             | Phaser            | Speed                   | Intensity               |                     |                |              |             |                     |                               |
| Modulation | Vibrato            | Vibrato01         | Speed                   | Depth                   |                     |                |              |             |                     |                               |
| Modulation | UniVibe            | UniVibe           | Speed                   | Vibrato/Chorus `Switch` | Intensity           |                |              |             |                     |                               |
| Modulation | Cloner Chorus      | Cloner            | Rate                    | Depth `Switch`          |                     |                |              |             |                     |                               |
| Modulation | Classic Vibe       | MiniVibe          | Speed                   | Intensity               |                     |                |              |             |                     |                               |
| Modulation | Tremolator         | Tremolator        | Depth                   | Speed                   | BPM On/Off `Switch` |                |              |             |                     |                               |
| Modulation | Tremolo Square     | TremoloSquare     | Speed                   | Depth                   | Level               |                |              |             |                     |                               |
| Modulation | Guitar EQ          | GuitarEQ6         | Level                   | 100                     | 200                 | 400            | 800          | 1.6k        | 3.2k                |                               |
| Modulation | Bass EQ            | BassEQ6           | Level                   | 50                      | 120                 | 400            | 800          | 4.5k        | 10k                 |                               |
| Delay      | Digital Delay      | DelayMono         | E.Level                 | Feedback                | DelayTime           | Mode           | BPM `Switch` |             |                     | Modes: 0.3 (50ms) - 0.4 (200ms) - 0.6 (500ms) - 0.72 (1s) |
| Delay      | Echo Filt          | DelayEchoFilt     | Delay                   | Feedback                | Level               | Tone           | BPM `Switch` |             |                     |                               |
| Delay      | Vintage Delay      | VintageDelay      | Repeat Rate             | Intensity               | Echo                | BPM `Switch`   |              |             |                     |                               |
| Delay      | Reverse Delay      | DelayReverse      | Mix                     | Decay                   | Filter              | Time           | BPM `Switch` |             |                     |                               |
| Delay      | Multi Head         | DelayMultiHead    | Repeat Rate             | Intensity               | Echo Volume         | Mode Selector  | BPM `Switch` |             |                     | Modes: 0 (Head 1) - 0.35 (Head 2) - 0.65 (Head 3) - 0.95 (Head 4) |
| Delay      | Echo Tape          | DelayRe201        | Sustain                 | Volume                  | Tone                | Short/Long     |              |             |                     |                               |
| Reverb     | Room Studio A      | bias.reverb       | Level                   | Damping                 | Dwell               | Time           | Low Cut      | High Cut    | Selects Reverb Type | 0.0                           |
| Reverb     | Room Studio B      | bias.reverb       | Level                   | Damping                 | Dwell               | Time           | Low Cut      | High Cut    | Selects Reverb Type | 0.1                           |
| Reverb     | Chamber            | bias.reverb       | Level                   | Damping                 | Dwell               | Time           | Low Cut      | High Cut    | Selects Reverb Type | 0.2                           |
| Reverb     | Hall Natural       | bias.reverb       | Level                   | Damping                 | Dwell               | Time           | Low Cut      | High Cut    | Selects Reverb Type | 0.3                           |
| Reverb     | Hall Medium        | bias.reverb       | Level                   | Damping                 | Dwell               | Time           | Low Cut      | High Cut    | Selects Reverb Type | 0.4                           |
| Reverb     | Hall Ambient       | bias.reverb       | Level                   | Damping                 | Dwell               | Time           | Low Cut      | High Cut    | Selects Reverb Type | 0.5                           |
| Reverb     | Plate Short        | bias.reverb       | Level                   | Damping                 | Dwell               | Time           | Low Cut      | High Cut    | Selects Reverb Type | 0.6                           |
| Reverb     | Plate Rich         | bias.reverb       | Level                   | Damping                 | Dwell               | Time           | Low Cut      | High Cut    | Selects Reverb Type | 0.7                           |
| Reverb     | Plate Long         | bias.reverb       | Level                   | Damping                 | Dwell               | Time           | Low Cut      | High Cut    | Selects Reverb Type | 0.8                           |

##### Hendrix Tones
**Note:** You can simply store tones using Hendrix gear on Ignitron. As the effects are licensed and purchased In-App, you need to connect your mobile Spark App to the Spark Amp after switching on the Spark Amp. When you then disconnect the Spark App and connect Ignitron afterwards you should be able to use presets using below gear.

| Type       | App&nbsp;Name           | Technical&nbsp;Name    | Parameter&nbsp;0             | Parameter&nbsp;1             | Parameter&nbsp;2         | Parameter&nbsp;3    | Parameter&nbsp;4  | Parameter&nbsp;5 | Parameter&nbsp;6         | Extra&nbsp;Info                    |
|------------|--------------------|-------------------|-------------------------|-------------------------|---------------------|----------------|--------------|-------------|---------------------|-------------------------------|
| Compressor/Wah | J.H. Legendary Wah         | JH.Vox846    | Auto Wah Mode               | BPM Mode `Switch`                  |  ms (BPM Off)                   | Bar (BPM On)               | Sensitivity             |             |                     | Auto Wah Mode: 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 - Bar: 0.0 (1/8), 0.25 (1/4), 0.5 (1/2), 0.75 (1/1)                              |
| Drive | J.H. Axle Fuzz           | JH.AxisFuzz          | Volume | Drive                    |             |                |              |             |                     |                               |
| Drive | J.H. Super Fuzz      | JH.SupaFuzz          | Volume                   | Filter                    |               |         |              |             |                     |                               |
| Drive | J.H. Octave Fuzz           | JH.Octavia        | Level                  | Fuzz             |                     |                |              |             |                     |                               |
| Drive | J.H. Fuzz Zone          | JH.FuzzTone          | Volume                    | Attack                    |                     |                |              |             |                     |                               |
| Modulation/EQ | J.H. Legendary Vibe      | JH.VoodooVibeJr    | Speed                  | Sweep                    | Intensity        | Mix (Vibrato/Chorus)               |              |             |                     |                               |
