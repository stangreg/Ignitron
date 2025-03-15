#include "SparkPresetControl.h"

SparkPresetControl::SparkPresetControl() {
}

SparkPresetControl::~SparkPresetControl() {
}

SparkPresetControl &SparkPresetControl::getInstance() {
    static SparkPresetControl INSTANCE;
    return INSTANCE;
}

void SparkPresetControl::init() {

    int operationMode = sparkDC->operationMode();
    presetBuilder.init();
    if (operationMode == SPARK_MODE_APP) {
        xTaskCreatePinnedToCore(
            checkForMissingPresets, // Function to implement the task
            "HWpresets",            // Name of the task
            10000,                  // Stack size in words
            this,                   // Task input parameter
            0,                      // Priority of the task
            NULL,                   // Task handle.
            1                       // Core where the task should run
        );
    }
    if (operationMode == SPARK_MODE_AMP) {
        pendingBank_ = 1;
        activeBank_ = 1;
        activePresetNum_ = 1;
        pendingPresetNum_ = 1;

        activePreset_ = presetBuilder.getPreset(activePresetNum_, activeBank_);
        pendingPreset_ = presetBuilder.getPreset(activePresetNum_,
                                                 pendingBank_);
    }
}

/////////////////////////////////////////////////////////
// PRESET RELATED
/////////////////////////////////////////////////////////

Preset SparkPresetControl::getPreset(int bank, int pre) {
    return presetBuilder.getPreset(bank, pre);
}

void SparkPresetControl::getMissingHWPresets() {
    int currentTime = millis();

    // Check for HW presets
    if (currentTime - lastUpdateCheck > updateInterval) {
        lastUpdateCheck = currentTime;
        if (sparkDC->processAction()) {
            // Only check for HW presets if current preset is known
            if (activePreset_.isEmpty) {
                sparkDC->getCurrentPresetFromSpark();
                return;
            }
            // DEBUG_PRINTLN("Checking missing HW presets");
            bool isAnyMissing = false;
            int numberOfPresets = presetBuilder.numberOfHWPresets();
            for (int num = 1; num <= numberOfPresets; num++) {
                // DEBUG_PRINTF("Checking missing HW preset %d.\n", num);
                bool isCurrentMissing = presetBuilder.isHWPresetMissing(num);
                if (isCurrentMissing) {
                    // if (presetBuilder.isHWPresetMissing(num)) {
                    DEBUG_PRINTF("%d is missing.\n", num);
                    sparkDC->readHWPreset(num);
                    delay(500);
                }
                isAnyMissing = isAnyMissing || isCurrentMissing;
            }
            allHWPresetsAvailable_ = !(isAnyMissing);
        }
    }
}

void SparkPresetControl::resetStatus() {

    presetBuilder.initHWPresets();
    presetBuilder.numberOfHWBanks() = 1;
    activePresetNum_ = pendingPresetNum_ = 1;
    activeHWBank_ = activeBank_ = pendingHWBank_ = pendingBank_ = 0;
}

void SparkPresetControl::checkForUpdates(int operationMode) {

    if (statusObject.isPresetNumberUpdated() &&
        (operationMode == SPARK_MODE_APP)) {
        if (pendingBank_ == 0) {
            DEBUG_PRINTLN("Preset number has changed, updating active preset");
            setActiveHWPreset();
        }
        statusObject.resetPresetNumberUpdateFlag();
    }

    // Check if active preset has been updated
    // If so, update the preset variables

    if (statusObject.isPresetUpdated() &&
        (operationMode == SPARK_MODE_APP)) {

        DEBUG_PRINTLN("Preset has changed, updating active with current preset");
        activePreset_ = statusObject.currentPreset();
        updatePendingWithActive();
        /*
        pendingPreset_ = statusObject.currentPreset();
        updateActiveWithPendingPreset();
        */
        statusObject.resetPresetUpdateFlag();
    }
}

void SparkPresetControl::updateActiveWithPendingPreset() {
    activePreset_ = pendingPreset_;
    activeBank_ = pendingBank_;
    activePresetNum_ = pendingPresetNum_;
    activeHWBank_ = pendingHWBank_;
}

void SparkPresetControl::setAmpParameters(string ampName) {
    if (ampName == AMP_NAME_SPARK_2) {
        presetBuilder.numberOfHWBanks() = 2;
    } else {
        presetBuilder.numberOfHWBanks() = 1;
    }
    presetBuilder.initHWPresets();
}

void SparkPresetControl::setBank(int i) {
    if (i > presetBuilder.getNumberOfBanks() || i < 0)
        return;
    activeBank_ = pendingBank_ = i;
}

void SparkPresetControl::increaseBank() {

    int operationMode = sparkDC->operationMode();
    if (!sparkDC->processAction()) {
        return;
    }

    // Cycle around if at the end
    if (pendingBank_ == numberOfBanks()) {
        // Roll over to 0 when going beyond the last bank
        pendingBank_ = 0;
        pendingHWBank_ = 0;
    }
    // Check if to increase bank or if to switch between HW banks, cycle through HW banks first
    else if (pendingHWBank_ < presetBuilder.numberOfHWBanks() - 1) {
        pendingHWBank_++;
    } else {
        pendingBank_++;
    }
    if (operationMode == SPARK_MODE_AMP && pendingBank_ == 0) {
        pendingBank_ = min(1, numberOfBanks());
    }
    updatePendingBankStatus();
}

void SparkPresetControl::decreaseBank() {

    int operationMode = sparkDC->operationMode();

    if (!sparkDC->processAction()) {
        return;
    }

    // Roll over to last bank if going beyond the first bank
    if (pendingBank_ == 0 && pendingHWBank_ == 0) {
        pendingBank_ = numberOfBanks();
        pendingHWBank_ = presetBuilder.numberOfHWBanks() - 1;
    } else if (pendingBank_ == 0) {
        pendingHWBank_--;
    } else {
        pendingBank_--;
    }
    // Don't go to bank 0 in AMP mode
    if (operationMode == SPARK_MODE_AMP && pendingBank_ == 0) {
        pendingBank_ = numberOfBanks();
    }
    updatePendingBankStatus();
}

void SparkPresetControl::updatePendingBankStatus() {
    // Mark selected bank as the pending Bank to mark in display.
    // The device is configured so that it does not immediately
    // switch presets when a bank is changed, it does so only when
    // the preset button is pushed afterwards

    int operationMode = sparkDC->operationMode();

    // if (pendingBank_ != 0) {
    updatePendingPreset(pendingBank_);
    //}
    if (operationMode == SPARK_MODE_AMP) {
        activeBank_ = pendingBank_;
        updateActiveWithPendingPreset();
        if (presetEditMode_ == PRESET_EDIT_DELETE) {
            resetPresetEdit(true, true);
        } else if (presetEditMode_ == PRESET_EDIT_STORE) {
            resetPresetEdit(false, false);
        }
    }
}

void SparkPresetControl::checkForMissingPresets(void *args) {
    SparkPresetControl *presetControl = (SparkPresetControl *)args;
    // delay(3000);
    while (true) {
        presetControl->getMissingHWPresets();
        delay(1000);
    }
}

void SparkPresetControl::validateChecksums(vector<byte> checksums) {
    presetBuilder.validateChecksums(checksums);
}

void SparkPresetControl::updatePendingPreset(int bnk) {
    int presetNum = activePresetNum_ == 0 ? 1 : activePresetNum_;
    pendingPreset_ = getPreset(bnk, presetNum);
    pendingPresetNum_ = presetNum;
}

void SparkPresetControl::updatePendingWithActive() {
    pendingBank_ = activeBank_;
    pendingPreset_ = activePreset_;
    pendingHWBank_ = activeHWBank_;
}

void SparkPresetControl::setActiveHWPreset() {

    activePreset_ = presetBuilder.getPreset(pendingBank_, pendingPresetNum_);
    activePresetNum_ = pendingPresetNum_;
    activeHWBank_ = pendingHWBank_;
    if (activePreset_.isEmpty) {
        DEBUG_PRINTLN("Cache not filled, getting preset from Spark");
        sparkDC->getCurrentPresetFromSpark();
    }
}

bool SparkPresetControl::switchPreset(int pre, bool isInitial) {

    int operationMode = sparkDC->operationMode();
    DEBUG_PRINTLN("Entering switchPreset()");
    bool retValue = false;

    int bnk = pendingBank_;
    pendingPresetNum_ = pre;

    if (operationMode == SPARK_MODE_APP) {
        // Check if selected preset is equal to active preset.
        // In this case toggle the drive pedal only
        if (pre == activePresetNum_ &&
            activeBank_ == pendingBank_ &&
            pendingHWBank_ == activeHWBank_ &&
            !(activePreset_.isEmpty) &&
            !isInitial) {

            retValue = sparkDC->toggleEffect(INDEX_FX_DRIVE);

        }
        // Preset is actually changing
        else {
            // Switch HW preset
            if (bnk == 0) {
                int presetNum = pre + pendingHWBank_ * PRESETS_PER_BANK;
                Serial.printf("Changing to HW preset %d...", presetNum);
                pendingPreset_ = presetBuilder.getPreset(bnk, presetNum);
                if (pendingPreset_.isEmpty) {
                    DEBUG_PRINTLN("Pending preset empty");
                }
                retValue = sparkDC->changeHWPreset(presetNum);
            }
            // Switch to custom preset
            else {
                Serial.printf("Changing to preset %02d-%d...", bnk, pre);
                pendingPreset_ = presetBuilder.getPreset(bnk, pre);
                if (activePreset_.isEmpty) {
                    Serial.println("Active preset empty, initializing.");
                    activePreset_ = pendingPreset_;
                }
                if (pendingPreset_.isEmpty) {
                    Serial.println("Empty preset, skipping further processing");
                    return false;
                }

                retValue = sparkDC->changePreset(pendingPreset_);

            } // Else (custom preset)
        } // else (preset changing)
    } // if APP / LOOPER mode
    /*if (retValue == true) {
        activeBank_ = bnk;
        activePresetNum_ = pre;
    }*/

    return retValue;
}

void SparkPresetControl::updateFromSparkResponseHWPreset(int presetNum) {

    // activePreset_ = statusObject.currentPreset();
    Preset newPreset = presetBuilder.getPreset(activeBank_, presetNum);
    if (newPreset.isEmpty) {
        Serial.println("Preset number changed, preset not cached, getting current preset from Spark");
        sparkDC->getCurrentPresetFromSpark();
    }
    // In case there are more than (PRESETS_PER_BANK) HW Presets, we have to calculate modulo (for internal display);
    activePresetNum_ = ((presetNum - 1) % PRESETS_PER_BANK) + 1;
    activeBank_ = pendingBank_ = 0;
    // calculate current HW bank
    activeHWBank_ = (presetNum - 1) / PRESETS_PER_BANK;
    activePreset_ = newPreset;

    pendingPresetNum_ = activePresetNum_;
    pendingHWBank_ = activeHWBank_;
    /* TODO: IMPORTANT: When preset is changed at AMP (0338), we need to read the current preset and update the bank/preset.
                If preset number was requested by us (0310), we need to compare if the current preset is the same as the number of the current HW preset number.
                In case it is the same, we need to update the bank/preset number, otherwise stay as is.
    */
}

void SparkPresetControl::toggleFX(Pedal receivedEffect) {
    DEBUG_PRINTF("Received FX: %s, Status: %s\n", receivedEffect.name, receivedEffect.isOn ? "on" : "off");
    for (Pedal &pdl : activePreset_.pedals) {
        if (pdl.name == receivedEffect.name) {
            DEBUG_PRINTF("activePreset before: %s, Status: %s\n", pdl.name, pdl.isOn ? "on" : "off");
        }
    }
    for (Pedal &pdl : activePreset_.pedals) {
        if (pdl.name == receivedEffect.name) {
            pdl.isOn = receivedEffect.isOn;
        }
    }
    for (Pedal &pdl : activePreset_.pedals) {
        if (pdl.name == receivedEffect.name) {
            DEBUG_PRINTF("activePreset after: %s, Status: %s\n", pdl.name, pdl.isOn ? "on" : "off");
        }
    }
    updatePendingWithActive();
}

void SparkPresetControl::switchFXOnOff(const string fxName, bool onOff) {
    Serial.printf("Switching %s effect %s...", onOff ? "On" : "Off",
                  fxName.c_str());
    for (Pedal &pdl : pendingPreset_.pedals) {
        //    for (int i = 0; i < pendingPreset.pedals.size(); i++) {
        if (pdl.name == fxName) {
            pdl.isOn = onOff;
            break;
        }
    }
}

void SparkPresetControl::updateFromSparkResponsePreset(bool isSpecial) {

    Preset receivedPreset = statusObject.currentPreset();
    int presetNumber = receivedPreset.presetNumber;

    // in case the preset is a HW preset and the current selected one,
    // (or not coming from the background process to retrieve missing presets)
    // if ((activePresetNum_ == presetNumber && pendingBank_ == 0) || !isSpecial) {
    if (!isSpecial) {
        DEBUG_PRINTLN("Updating activePreset...");
        activePreset_ = statusObject.currentPreset();
    }
    if (isSpecial) {
        DEBUG_PRINTF("Storing preset %d into cache.\n", presetNumber + 1);
        presetBuilder.insertHWPreset(presetNumber, receivedPreset);
        statusObject.resetPresetUpdateFlag();
    }
    string uuid = activePreset_.uuid;
    pair<int, int> bankPreset = presetBuilder.getBankPresetNumFromUUID(uuid);
    int checkPresetNum = std::get<1>(bankPreset);
    if (checkPresetNum != 0) {
        activeBank_ = std::get<0>(bankPreset);
        activePresetNum_ = ((checkPresetNum - 1) % PRESETS_PER_BANK) + 1;
        if (activeBank_ == 0) {
            activeHWBank_ = (checkPresetNum - 1) / PRESETS_PER_BANK;
        }
    } else {
        Serial.println("Preset not found, not changing.");
    }
    DEBUG_PRINTF("New active bank: %d, active preset: %d\n", activeBank_, activePresetNum_);
    updatePendingWithActive();
}

void SparkPresetControl::updateFromSparkResponseAmpPreset(char *presetJson) {
    presetEditMode_ = PRESET_EDIT_STORE;
    appReceivedPreset_ = presetBuilder.getPresetFromJson(presetJson);
    DEBUG_PRINTLN("received from app:");
    DEBUG_PRINTLN(appReceivedPreset_.json.c_str());
    presetNumToEdit_ = 0;
}

void SparkPresetControl::updateFromSparkResponseACK() {
    if (pendingBank_ == 0) {
        setActiveHWPreset();
        // updatePendingWithActive();
    } // else {
    updateActiveWithPendingPreset();
    //}
}

bool SparkPresetControl::increasePresetLooper() {

    int subMode = sparkDC->subMode();
    if (!sparkDC->processAction() ||
        (subMode != SUB_MODE_LOOPER && subMode != SUB_MODE_LOOP_CONTROL)) {
        Serial.println("Looper preset change: Spark Amp not connected or not in Looper mode, doing nothing");
        return false;
    }

    int selectedPresetNum;
    if (activePresetNum_ == 4) {
        selectedPresetNum = 1;
        if (pendingBank_ == numberOfBanks()) {
            pendingBank_ = 0;
            activeHWBank_ = 0;
        }
        // Check if to increase bank or if to switch between HW banks, cycle through HW banks first
        else if (activeHWBank_ < presetBuilder.numberOfHWBanks() - 1) {
            activeHWBank_++;
        } else {
            pendingBank_++;
        }
    } else {
        selectedPresetNum = activePresetNum_ + 1;
    }
    return switchPreset(selectedPresetNum, false);
}

bool SparkPresetControl::decreasePresetLooper() {

    int subMode = sparkDC->subMode();
    if (!sparkDC->processAction() ||
        (subMode != SUB_MODE_LOOPER && subMode != SUB_MODE_LOOP_CONTROL)) {
        Serial.println("Looper preset change: Spark Amp not connected or not in Looper mode, doing nothing");
        return false;
    }

    int selectedPresetNum;
    if (activePresetNum_ == 1) {
        selectedPresetNum = 4;
        if (pendingBank_ == 0 && activeHWBank_ == 0) {
            pendingBank_ = numberOfBanks();
            activeHWBank_ = presetBuilder.numberOfHWBanks() - 1;
        } else if (pendingBank_ == 0) {
            activeHWBank_--;
        } else {
            pendingBank_--;
        }
    } else {
        selectedPresetNum = activePresetNum_ - 1;
    }
    return switchPreset(selectedPresetNum, false);
}

/////////////////////////////////////////////////////////
// AMP PRESET MANAGER
/////////////////////////////////////////////////////////

void SparkPresetControl::processPresetEdit(int presetNum) {
    if (presetNum == 0) {
        processDeletePresetRequest();
    } else if (presetEditMode_ == PRESET_EDIT_STORE) {
        processStorePresetRequest(presetNum);
    } else {
        resetPresetEdit(true, true);
        activePresetNum_ = presetNum;
        DEBUG_PRINTF("Reading presetEdit %d - %02d\n", activeBank_, activePresetNum_);
        activePreset_ = presetBuilder.getPreset(activeBank_, activePresetNum_);
        pendingPreset_ = activePreset_;
    }
}

void SparkPresetControl::processStorePresetRequest(int presetNum) {

    responseMsg_ = "";
    if (presetEditMode_ == PRESET_EDIT_STORE) {
        if (presetNumToEdit_ == presetNum && presetBankToEdit_ == pendingBank_) {
            int responseCode;
            responseCode = presetBuilder.storePreset(appReceivedPreset_,
                                                     pendingBank_, presetNum);
            if (responseCode == STORE_PRESET_OK) {
                Serial.println("Successfully stored preset");
                resetPresetEdit(true, true);
                appReceivedPreset_ = {};
                activePresetNum_ = presetNum;
                activePreset_ = presetBuilder.getPreset(activeBank_,
                                                        activePresetNum_);
                pendingPreset_ = activePreset_;
                responseMsg_ = "SAVE OK";
            }
            if (responseCode == STORE_PRESET_FILE_EXISTS) {
                responseMsg_ = "PRST EXIST";
            }
            if (responseCode == STORE_PRESET_ERROR_OPEN || responseCode == STORE_PRESET_UNKNOWN_ERROR) {
                responseMsg_ = "SAVE ERROR";
            }
        } else {
            activePresetNum_ = presetNum;
            activePreset_ = presetBuilder.getPreset(activeBank_,
                                                    activePresetNum_);
            pendingPreset_ = activePreset_;
            presetNumToEdit_ = presetNum;
            presetBankToEdit_ = pendingBank_;
        }
    }
}

void SparkPresetControl::resetPresetEdit(bool resetEditMode, bool resetPreset) {
    presetNumToEdit_ = 0;
    presetBankToEdit_ = 0;

    if (resetPreset) {
        appReceivedPreset_ = {};
    }
    if (resetEditMode) {
        presetEditMode_ = PRESET_EDIT_NONE;
    }
}

void SparkPresetControl::resetPresetEditResponse() {
    responseMsg_ = "";
}

void SparkPresetControl::processDeletePresetRequest() {
    responseMsg_ = "";
    if (presetEditMode_ == PRESET_EDIT_DELETE && activeBank_ > 0) {
        int responseCode;
        responseCode = presetBuilder.deletePreset(activeBank_,
                                                  activePresetNum_);
        if (responseCode == DELETE_PRESET_OK || responseCode == DELETE_PRESET_FILE_NOT_EXIST) {
            Serial.printf("Successfully deleted preset %d-%d\n", pendingBank_,
                          activePresetNum_);
            presetNumToEdit_ = 0;
            presetBankToEdit_ = 0;
            activePreset_ = presetBuilder.getPreset(pendingBank_,
                                                    activePresetNum_);
            pendingPreset_ = activePreset_;
            if (responseCode == DELETE_PRESET_OK) {
                responseMsg_ = "DELETE OK";
            } else {
                responseMsg_ = "FILE NOT EXITS";
            }
        }
        if (responseCode == DELETE_PRESET_ERROR_OPEN || responseCode == STORE_PRESET_UNKNOWN_ERROR) {
            responseMsg_ = "DELETE ERROR";
        }
        resetPresetEdit(true, true);
    } else {
        setPresetDeletionFlag();
        presetNumToEdit_ = activePresetNum_;
        presetBankToEdit_ = activeBank_;
    }
}

void SparkPresetControl::setPresetDeletionFlag() {
    presetEditMode_ = PRESET_EDIT_DELETE;
}

bool SparkPresetControl::handleDeletePreset() {

    int operationMode = sparkDC->operationMode();

    if (operationMode != SPARK_MODE_AMP) {
        Serial.println("Delete Preset: Not in AMP mode, doing nothing.");
        return false;
    }

    if (presetEditMode() == PRESET_EDIT_STORE) {
        resetPresetEdit(true, true);
    } else {
        processPresetEdit();
    }
    return true;
}

bool SparkPresetControl::processPresetSelect(int presetNum) {

    int operationMode = sparkDC->operationMode();
    if (!sparkDC->processAction()) {
        Serial.println("Action not meeting requirements, ignoring.");
        return false;
    }

    if (operationMode == SPARK_MODE_APP) {

        sparkDC->switchPreset(presetNum, false);

    } else if (operationMode == SPARK_MODE_AMP) { // AMP mode

        processPresetEdit(presetNum);
    }
    return true;
}
