# Sparky
An ESP32 based footswitch to communicate with the Spark Amp via Bluetooth LE.

Sparky gives you full control over your presets:
* Switch between the four **hardware presets**
* Switch between **custom saved presets** organized in banks
* **Activate / Deactivate effects** for the selected preset
* **Load new presets** via the Spark App
* **Delete** stored presets

Adding new presets to Sparky can easily be done as it can also act as a Spark Amp. So simply connect to Sparky with your Spark app and load new presets directly from ToneCloud (or your downloaded presets) to Sparky. 

One big advantage of Sparky is that it communicates with your Spark Amp using the **Bluetooth LE** protocol.
This means you can still connect your mobile with your Spark Amp as an audio speaker and play along your favorite songs while you control your effects.

The current active preset and effects are indicated via LEDs. In addition, the built-in display provides information on
* selected bank / preset number
* selected preset name
* activated pedal types in the FX chain
* Connection status to Spark Amp and Spark App


## Operation modes
The Sparky has two operation modes, APP mode and AMP mode.
In APP mode, the pedal is in standard operation mode. It behaves like the Spark App towards the Spark Amp and can then be used to switch between presets and/or switch effects on and off.
AMP mode can be used to manage the presets stored on the Sparky, presets can be added from the app or be deleted from Sparky.
Standard mode on startup is the APP mode. 

**To enter AMP mode, hold the `Preset 1` button during startup.**

### APP mode
In APP mode, the foot switches can be used to either switch between pre-saved presets (**Preset mode**) or to control all single effects in the selected preset (**FX mode**). Modes can easily be switched by long pressing the `Bank-Up` button.
When selecting Preset mode, four buttons are used to select presets. The other two buttons can be used to navigate through different preset banks. This way the user has access to a big number of saved presets. When pressing the foot switch of the current active preset, the effect configured in the DRIVE section can be enabled and disabled.
In FX mode, the user has direct access to all effects of the selected preset. 
Each switch controls a different FX pedal type:


#### Function of switches:

##### FX mode (APP)
|Switch | Press pattern |Function |
|---| -----|------ |
|`Bank down` | Short |Noise Gate |
|`Bank up` | Short |Compressor |
|`Preset 1` | Short |Drive |
|`Preset 2` | Short |Modulation |
|`Preset 3` | Short |Delay |
|`Preset 4` | Short |Reverb | 
|`Bank up` | Long | Switch to Preset mode |
|`Preset 2` | Long | Restart Sparky |

##### Preset mode (APP)
|Switch | Press pattern |Function |
|---| -----|------ |
|`Bank down` | Short |Navigate bank up |
|`Bank up` | Short |Navigate bank down |
|`Preset 1` | Short |Select preset 1 |
|`Preset 2` | Short |Select preset 2 |
|`Preset 3` | Short |Select preset 3 |
|`Preset 4` | Short |Select preset 4 | 
|`Bank up` | Long | Switch to FX mode|
|`Preset 2` | Long | Restart Sparky |

### AMP mode
In AMP mode, Sparky acts like a Spark AMP and can communicate with the Spark app running on a mobile. New presets can be stored and existing presets can be deleted.

#### Connecting the Spark app with Sparky
1. Start Sparky in AMP mode (hold Preset 1 button during startup).
2. Open the Spark app on the mobile
3. For the first connection, make sure to have your real Spark amp powered off to avoid the app conneting to it.
4. Hit the connect button in the app (or the + button in the connection overview)
5. Once the connection is established, you can give the connection a name so you can tell it better from your regular Spark amp connection.


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

|Switch | Press pattern |Function | Remark |
|---| -----|------ | ----- |
|`Bank down` | Short |Navigate bank up | - |
|`Bank up` | Short |Navigate bank down | - |
|`Preset 1` | Short |Select preset 1 | Press same preset twice to store received preset from app |
|`Preset 2` | Short |Select preset 2 | Press same preset twice to store received preset from app |
|`Preset 3` | Short |Select preset 3 | Press same preset twice to store received preset from app |
|`Preset 4` | Short |Select preset 4 | Press same preset twice to store received preset from app |
|`Bank down` | Long | Unload preset received from app| - |
|`Preset 2` | Long | Restart Sparky |

#### Deleting a preset (only possible with no preset from the app loaded)
1. Start Sparky in AMP mode (hold `Preset 1` button during startup).
2. Use the `Preset`/`Bank` buttons on Sparky to navigate to the desired preset position
3. **Long press** the `Bank Down` button to mark the selected preset for deletion
4. The LED of the selected preset should start blinking
5. In the display you should see a message that deletion is prepared
6. **Long press** the `Bank Down` button again to confirm deletion.
(Hitting any other button will cancel the deletion and return back to navigation)

|Switch | Press pattern |Function | Remark |
|---| -----|------ | ----- |
|`Bank down` | Short |Navigate bank up | - |
|`Bank up` | Short |Navigate bank down | - |
|`Preset 1` | Short |Select preset 1 | -  |
|`Preset 2` | Short |Select preset 2 | - |
|`Preset 3` | Short |Select preset 3 | - |
|`Preset 4` | Short |Select preset 4 | - |
|`Bank down` | Long | Mark saved preset for deletion| *Pressing any other button in that state will cancel the deletion* |
|`Bank down` | Long | Delete preset marked for deletion| *Only if preset was first marked for deletion* |
|`Preset 2` | Long | Restart Sparky |


## FX reference
In order to know which effects are available with paramters, see below table.
This data can be used to build own presets in JSON format `(to be described later)`

| Type       | App Name           | Technical Name    | Parameter 0             | Parameter 1             | Parameter 2         | Parameter 3    | Parameter 4  | Parameter 5 | Parameter 6         | Extra Info                    |
|------------|--------------------|-------------------|-------------------------|-------------------------|---------------------|----------------|--------------|-------------|---------------------|-------------------------------|
| Noise Gate | Noise Gate         | bias.noisegate    | Threshold               | Decay                   |                     |                |              |             |                     |                               |
| Compressor | LA Comp            | LA2AComp          | Limit/Compress (Switch) | Gain                    | Peak Reduction      |                |              |             |                     |                               |
| Compressor | Sustain Comp       | BlueComp          | Level                   | Tone                    | Attack              | Sustain        |              |             |                     |                               |
| Compressor | Red Comp           | Compressor        | Output                  | Sensitivity             |                     |                |              |             |                     |                               |
| Compressor | Bass Comp          | BassComp          | Comp                    | Gain                    |                     |                |              |             |                     |                               |
| Compressor | Optical Comp       | BBEOpticalComp    | Volume                  | Comp                    | Pad (Switch)        |                |              |             |                     |                               |
| Drive      | Booster            | Booster           | Gain                    |                         |                     |                |              |             |                     |                               |
| Drive      | Clone Drive        | KlonCentaurSilver | Output                  | Treble                  | Gain                |                |              |             |                     |                               |
| Drive      | Tube Drive         | DistortionTS9     | Overdrive               | Tone                    | Level               |                |              |             |                     |                               |
| Drive      | Over Drive         | Overdrive         | Level                   | Tone                    | Drive               |                |              |             |                     |                               |
| Drive      | Fuzz Face          | Fuzz              | Volume                  | Fuzz                    |                     |                |              |             |                     |                               |
| Drive      | Black Op           | ProCoRat          | Distortion              | Filter                  | Volume              |                |              |             |                     |                               |
| Drive      | Bass Muff          | BassBigMuff       | Volume                  | Tone                    | Sustain             |                |              |             |                     |                               |
| Drive      | Guitar Muff        | GuitarMuff        | Volume                  | Tone                    | Sustain             |                |              |             |                     |                               |
| Drive      | Bassmaster         | MaestroBassmaster | Brass Volume            | Sensitivity             | Bass Volume         |                |              |             |                     |                               |
| Drive      | SAB Driver         | SABDriver         | Volume                  | Tone                    | Drive               | LP/HP (Switch) |              |             |                     |                               |
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
| Modulation | UniVibe            | UniVibe           | Speed                   | Vibrato/Chorus (Switch) | Intensity           |                |              |             |                     |                               |
| Modulation | Cloner Chorus      | Cloner            | Rate                    | Depth (Switch)          |                     |                |              |             |                     |                               |
| Modulation | Classic Vibe       | MiniVibe          | Speed                   | Intensity               |                     |                |              |             |                     |                               |
| Modulation | Tremolator         | Tremolator        | Depth                   | Speed                   | BPM On/Off (Switch) |                |              |             |                     |                               |
| Modulation | Tremolo Square     | TremoloSquare     | Speed                   | Depth                   | Level               |                |              |             |                     |                               |
| Modulation | Guitar EQ          | GuitarEQ6         | Level                   | 100                     | 200                 | 400            | 800          | 1.6k        | 3.2k                |                               |
| Modulation | Bass EQ            | BassEQ6           | Level                   | 50                      | 120                 | 400            | 800          | 4.5k        | 10k                 |                               |
| Delay      | Digital Delay      | DelayMono         | E.Level                 | Feedback                | DelayTime           | Mode           | BPM (Switch) |             |                     | Modes: 0.3 - 0.4 - 0.6 - 0.72 |
| Delay      | Echo Filt          | DelayEchoFilt     | Delay                   | Feedback                | Level               | Tone           | BPM (Switch) |             |                     |                               |
| Delay      | Vintage Delay      | VintageDelay      | Repeat Rate             | Intensity               | Echo                | BPM (Switch)   |              |             |                     |                               |
| Delay      | Reverse Delay      | DelayReverse      | Mix                     | Decay                   | Filter              | Time           | BPM (Switch) |             |                     |                               |
| Delay      | Multi Head         | DelayMultiHead    | Repeat Rate             | Intensity               | Echo Volume         | Mode Selector  | BPM (Switch) |             |                     | Modes: 0 - 0.35 - 0.65 - 0.95 |
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
