#ifndef SPARKPRESETCONTROL_H
#define SPARKPRESETCONTROL_H

#include "SparkDataControl.h"
#include "SparkPresetBuilder.h"
#include "SparkStatus.h"
#include "SparkTypes.h"

#define PRESET_EDIT_NONE 0
#define PRESET_EDIT_STORE 1
#define PRESET_EDIT_DELETE 2

class SparkDataControl;

class SparkPresetControl {
public:
    static SparkPresetControl &getInstance();
    SparkPresetControl(const SparkPresetControl &) = delete; // Disable copy constructor
    SparkPresetControl &operator=(const SparkPresetControl &) = delete;

    void init();

    // Return active or pending preset/bank, set/get active preset number
    const Preset &activePreset() const { return activePreset_; }
    Preset &pendingPreset() { return pendingPreset_; }
    const int &activePresetNum() const { return activePresetNum_; }
    const int &pendingPresetNum() const { return pendingPresetNum_; }

    // int& activePresetNum() {return activePresetNum_;}
    const int &activeBank() const { return activeBank_; }
    const int &pendingBank() const { return pendingBank_; }
    const int numberOfBanks() const { return presetBuilder.getNumberOfBanks(); }
    const Preset &appReceivedPreset() const { return appReceivedPreset_; }

    void updatePendingWithActive();
    void updateActiveWithPendingPreset();

    const bool allHWPresetsAvailable() const { return allHWPresetsAvailable_; }

    const int presetNumToEdit() const { return presetNumToEdit_; }
    const string responseMsg() const { return responseMsg_; }
    const int presetEditMode() const { return presetEditMode_; }

    void setBank(int i);
    void increaseBank();
    void decreaseBank();
    bool increasePresetLooper();
    bool decreasePresetLooper();
    bool switchPreset(int pre, bool isInitial);
    void updateFromSparkResponseHWPreset(int presetNum);
    void toggleFX(Pedal receivedEffect);
    // TODO: Clean up with toggleFX
    void switchFXOnOff(const string name, bool onOff);

    void updateFromSparkResponsePreset(bool isSpecial);
    void updateFromSparkResponseAmpPreset(string presetJson);
    void updateFromSparkResponseACK();

    void
    processPresetEdit(int presetNum = 0);
    void resetPresetEdit(bool resetEditMode, bool resetPreset = false);
    void resetPresetEditResponse();
    bool handleDeletePreset();
    bool processPresetSelect(int presetNum);

    // get a preset from saved presets
    Preset getPreset(int bank, int pre);

    void getMissingHWPresets();
    void resetStatus();
    void checkForUpdates(int operationMode);

private:
    SparkPresetControl();
    ~SparkPresetControl();

    // PRESET variables
    Preset activePreset_;
    Preset pendingPreset_ = activePreset_;
    int activeBank_ = 0;
    int pendingBank_ = 0;
    int activePresetNum_ = 1;
    int pendingPresetNum_ = 1;
    bool allHWPresetsAvailable_ = false;

    // AMP Mode presets
    Preset appReceivedPreset_;
    int presetNumToEdit_ = 0;
    int presetBankToEdit_ = 0;
    int presetEditMode_ = PRESET_EDIT_NONE;

    string responseMsg_ = "";

    int lastUpdateCheck = 0;
    int updateInterval = 3000;

    SparkPresetBuilder presetBuilder;
    SparkDataControl *sparkDC;
    SparkStatus &statusObject = SparkStatus::getInstance();
    static eSPIFFS fileSystem;

    string sparkPresetFileName = "/config/SparkPreset.config";

    // AMP mode preset operations
    void processStorePresetRequest(int presetNum);
    void processDeletePresetRequest();
    void setPresetDeletionFlag();
    void updatePendingBankStatus();

    // File related operations
    void readLastPresetFromFile();
    void writeLastPresetToFile();

    // TODO: maybe this method needs to be static to run as a task
    static void checkForMissingPresets(void *args);
    void updatePendingPreset(int bnk);
    void setActiveHWPreset();
};

#endif