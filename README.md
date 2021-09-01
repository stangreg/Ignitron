# Sparky
An ESP32 module to communicate with the Spark Amp

## Operation modes
The Sparky has two operation modes, APP mode and AMP mode.
In APP mode, the pedal is in standard operation mode. It behaves like the Spark App towards the Spark Amp and can then be used to switch between presets and/or switch effects on and off.
AMP mode can be used to manage the presets stored on the Sparky, presets can be added from the app or be deleted from Sparky.
Standard mode on startup is the APP mode. To enter AMP mode, hold the **Preset 1** button during startup.

## APP mode
In APP mode, the foot switches can be used to either switch between pre-saved presets (**Preset mode**) or to control all single effects in the selected preset (**FX mode**). Modes can easily be switched by long pressing the Bank-Up button.
When selecting Preset mode, four buttons are used to select presets. The other two buttons can be used to navigate through different banks with presets. This way the user has access to a big number of saved presets. When pressing the foot switch of the current active preset, the effect configured in the DRIVE section can be enabled or disabled.
In FX mode, the user has direct access to all effects of the selected preset. 
Each switch controls a different FX pedal type:


Function of switches:

##### FX mode (APP)
|Switch | Press pattern |Function |
|---| -----|------ |
|Bank down | Short |Noise Gate |
|Bank up | Short |Compressor |
|Preset 1 | Short |Drive |
|Preset 2 | Short |Modulation |
|Preset 3 | Short |Delay |
|Preset 4 | Short |Reverb | 
|Bank up | Long | Switch to Preset mode|

##### Preset mode (APP)
|Switch | Press pattern |Function |
|---| -----|------ |
|Bank down | Short |Navigate bank up |
|Bank up | Short |Navigate bank down |
|Preset 1 | Short |Select preset 1 |
|Preset 2 | Short |Select preset 2 |
|Preset 3 | Short |Select preset 3 |
|Preset 4 | Short |Select preset 4 | 
|Bank up | Long | Switch to FX mode|


## AMP mode
In AMP mode, Sparky acts like a Spark AMP and can communicate with the Spark app running on a mobile. New presets can be stored and existing presets can be deleted.

#### Connecting the Spark app with Sparky
1. Start Sparky in AMP mode (hold Preset 1 button during startup).
2. Open the Spark app on the mobile
3. For the first connection, make sure to have your real Spark amp powered off to avoid the app conneting to it.
4. Hit the connect button in the app (or the + button in the connection overview)
5. Once the connection is established, you can give the connection a name so you can tell it better from your regular Spark amp connection.


### Storing a new preset (a preset from the app needs to be loaded)
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
|Bank down | Short |Navigate bank up | - |
|Bank up | Short |Navigate bank down | - |
|Preset 1 | Short |Select preset 1 | Press same preset twice to store received preset from app |
|Preset 2 | Short |Select preset 2 | Press same preset twice to store received preset from app |
|Preset 3 | Short |Select preset 3 | Press same preset twice to store received preset from app |
|Preset 4 | Short |Select preset 4 | Press same preset twice to store received preset from app |
|Bank down | Long | Unload preset received from app| - |


### Deleting an existing preset
1. Start Sparky in AMP mode (hold `Preset 1` button during startup).
2. Use the `Preset`/`Bank` buttons on Sparky to navigate to the desired preset position
3. **Long press** the `Bank Down` button to mark the selected preset for deletion
4. The LED of the selected preset should start blinking
5. In the display you should see a message that deletion is prepared
6. **Long press** the `Bank Down` button again to confirm deletion.
(Hitting any other button will cancel the deletion and return back to navigation)



#### Deleting a preset (only possible with no preset from the app loaded)
|Switch | Press pattern |Function | Remark |
|---| -----|------ | ----- |
|Bank down | Short |Navigate bank up | - |
|Bank up | Short |Navigate bank down | - |
|Preset 1 | Short |Select preset 1 | -  |
|Preset 2 | Short |Select preset 2 | - |
|Preset 3 | Short |Select preset 3 | - |
|Preset 4 | Short |Select preset 4 | - |
|Bank down | Long | Mark saved preset for deletion| *Pressing any other button in that state will cancel the deletion* |
|Bank down | Long | Delete preset marked for deletion| *Only if preset was first marked for deletion* |



