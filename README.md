# ![Sparky][Sparky_logo]
An ESP32 based foot pedal to communicate with the Spark Amp via Bluetooth LE.

Sparky gives you full control over your presets:
* Switch between the four **hardware presets**
* Switch between **custom saved presets** organized in banks
* **Activate / Deactivate effects** for the selected preset
* **Load new presets** via the Spark App
* **Delete** stored presets

Adding new presets to Sparky can easily be done as it can also act as a Spark Amp. Simply connect to Sparky with your Spark app and load new presets directly from ToneCloud (or your downloaded presets) to Sparky. 

One big advantage of Sparky is that it communicates with your Spark Amp using the **Bluetooth LE** protocol.
This means you can still connect your mobile with your Spark Amp as an audio speaker and play along your favorite songs while you control your effects.

The current active preset and effects are indicated via LEDs.\
In addition, the **built-in display** provides information on
* selected bank and preset number
* selected preset name
* activated pedal types in the FX chain
* connection status to Spark Amp and Spark App


## Operation modes
The Sparky has two operation modes, **APP mode** and **AMP mode**.\
In **APP mode** (default mode on startup), Sparky connects to a Spark Amp and behaves like the Spark App towards the Spark Amp. It can then be used to switch between saved presets and/or toggle FX swtiches.
**AMP mode** can be used to manage the presets stored on the Sparky, presets can be added from the app or be deleted from Sparky.\
To enter **AMP mode**, hold the `Preset 1` button during startup.

## APP mode
In APP mode, the foot switches can be used to either switch between pre-saved presets (**Preset mode**) or to control all single effects in the selected preset (**FX mode**). Modes can easily be switched by long pressing the `Bank-Up` button.
When selecting **Preset mode**, four buttons are used to select presets the other two buttons are used to navigate through different preset banks. This way the user has access to a big number of saved presets. When pressing the foot switch of the current active preset, the effect configured in the DRIVE section can be enabled and disabled.
In **FX mode**, the user has direct access to all effects of the selected preset. 
Each switch controls a different FX pedal type:


### Button commands in APP mode:

#### Preset mode
|Button      | Press pattern | Function           |
|----------- | :-----------: | ------------------ |
|`Bank down` | Short         | Navigate bank up   |
|`Bank up`   | Short         | Navigate bank down |
|`Preset 1`  | Short         | Select preset 1    |
|`Preset 2`  | Short         | Select preset 2    |
|`Preset 3`  | Short         | Select preset 3    |
|`Preset 4`  | Short         | Select preset 4    | 
|`Bank up`   | Long          | Switch to FX mode  |
|`Preset 2`  | Long          | Restart Sparky     |


#### FX mode
|Button      | Press pattern | Function              |
|----------- | :-----------: | --------------------- |
|`Bank down` | Short         | Noise Gate            |
|`Bank up`   | Short         | Compressor            |
|`Preset 1`  | Short         | Drive                 |
|`Preset 2`  | Short         | Modulation            |
|`Preset 3`  | Short         | Delay                 |
|`Preset 4`  | Short         | Reverb                | 
|`Bank up`   | Long          | Switch to Preset mode |
|`Preset 2`  | Long          | Restart Sparky        |

-----------------------------------------------------------------

## AMP mode
In AMP mode, Sparky acts like a Spark AMP and can communicate with the Spark app running on a mobile. New presets can be stored and existing presets can be deleted.

### Connecting the Spark app with Sparky
1. Start Sparky in AMP mode (hold `Preset 1` button during startup).
2. Open the Spark app on the mobile
3. For the first connection, make sure to have your real Spark amp powered off to avoid the app conneting to it.
4. Hit the connect button in the app (or the + button in the connection overview)
5. Once the connection is established, you can give the connection a name so you can tell it better from your regular Spark amp connection.

### The easy way

#### Storing a new preset (a preset from the app needs to be loaded)
1. Start Sparky in AMP mode (hold Preset 1 button during startup).
2. Connect the Spark app with Sparky (see above)
3. Select a preset in the app (either saved in the app or from the ToneCloud).
4. The preset name should appear in the bottom line of the display
5. The `Preset`/`Bank` buttons on Sparky can be used to navigate to the  desired preset position. 
6. Press the `Preset` button to mark the position for storing
7. The LED of the selected preset position should start blinking
8. Hit the same `Preset` button a second time to confirm storage
(Hitting any other `Preset`/`Bank` button will revert the state back to navigating)
9. The preset will be entered in the selected position, other presets will be pushed back

##### Button commands (case: preset has been received from Spark app)
| Button     | Press pattern | Function                        | Remark |
| ---------- | :-----------: | ------------------------------- | ------ |
|`Bank down` | Short         | Navigate bank up                |        |
|`Bank up`   | Short         | Navigate bank down              |        |
|`Preset 1`  | Short         | Select preset 1                 | *Press twice to store received preset* |
|`Preset 2`  | Short         | Select preset 2                 | *Press twice to store received preset* |
|`Preset 3`  | Short         | Select preset 3                 | *Press twice to store received preset* |
|`Preset 4`  | Short         | Select preset 4                 | *Press twice to store received preset* |
|`Bank down` | Long          | Unload preset                   | *Removes loaded preset*                         |
|`Preset 2`  | Long          | Restart Sparky                  |        |

#### Deleting a preset (only possible with no preset from the app loaded)
1. Start Sparky in AMP mode (hold `Preset 1` button during startup).
2. Use the `Preset`/`Bank` buttons on Sparky to navigate to the desired preset position
3. **Long press** the `Bank Down` button to mark the selected preset for deletion
4. The LED of the selected preset should start blinking
5. In the display you should see a prompt if deletion should be executed
6. If you want to cancel the deletion, simply short press any other button
7. Otherwise, **long press** the `Bank Down` button again to confirm deletion.
(Hitting any other button will cancel the deletion and return back to navigation)

##### Button commands (case: no preset has been received, slot is empty)
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
|`Preset 2`  | Long          | Restart Sparky           |        |

-------------------------------------------------------

### The hard way (for techies)
Sparky stores presets in a JSON format using the SPIFFS file system.
Each preset is stored in a separate file and presets are organized in a separate text file called 'PresetList.txt'. This list simply stored the file names of the presets, the order defines the way the banks are filled.
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

#### Storing new presets
*This method is only recommended if a preset cannot be transferred via the Spark app or if the preset files have been received/generated in a different way.*
1. Create a **preset file** in JSON format as shown above
2. Make sure the **file name** does not exceed 31 characters (including the `.json` suffix)
3. Put the file name into the **data folder**
4. Insert the file name (including the `.json` suffix) to the desired prefix location in the `PresetList.txt` file
5. *Place the PresetList.txt file into the data folder (if not already there)*
6. **Upload the data folder** to Sparky via the Arduino IDE (or other tools) 

As a guidance which effect and amp names cana be used and in which order the parameters of each effect have to be given, please refer to 

## FX reference
In order to know which effects are available with paramters, see below table.
This data can be used to build own presets in JSON format (see above).
Parameters marked with `Switch` can only have values of 0 or 1.

| Type       | App Name           | Technical Name    | Parameter 0             | Parameter 1             | Parameter 2         | Parameter 3    | Parameter 4  | Parameter 5 | Parameter 6         | Extra Info                    |
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

# The code



[Sparky_logo]: https://github.com/stangreg/SparkBLE/blob/main/Sparky_logo_large.png