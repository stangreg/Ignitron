/*
 * SparkBTControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#include "SparkBTControl.h"

bool SparkBTControl::isAppConnectedSerial_ = false;

// ClientCallbacks SparkBTControl::clientCB;

SparkBTControl::SparkBTControl() {
    // advDevCB = new AdvertisedDeviceCallbacks();
    advDevice_ = nullptr;
    spark_dc_ = nullptr;
}

SparkBTControl::SparkBTControl(SparkDataControl *dc) {
    // advDevCB = new AdvertisedDeviceCallbacks();
    advDevice_ = nullptr;
    spark_dc_ = dc;
}

SparkBTControl::~SparkBTControl() {
    Serial.println("Deleting BLE objects");
    if (advDevice_) {
        delete advDevice_;
        advDevice_ = nullptr;
    }
    if (btSerial) {
        delete btSerial;
        btSerial = nullptr;
    }
}

// Initializing BLE connection with NimBLE
void SparkBTControl::initBLE(notify_callback notifyCallback) {
    // NimBLEDevice::init("");
    advDevice_ = new NimBLEAdvertisedDevice();
    notifyCB_ = notifyCallback;

    /** Optional: set the transmit power, default is 3db */
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */

    /** create new scan */
    NimBLEScan *scan = NimBLEDevice::getScan();

    /** create a callback that gets called when advertisers are found */
    scan->setAdvertisedDeviceCallbacks(this, false);

    /** Set scan interval (how often) and window (how long) in milliseconds */
    scan->setInterval(45);
    scan->setWindow(15);

    /** Active scan will gather scan response data from advertisers
     but will use more energy from both devices
     */
    scan->setActiveScan(true);
    /** Start scanning for advertisers for the scan time specified (in seconds) 0 = forever
     Optional callback for when scanning stops.
     */
    Serial.println("Starting scan");
    scan->start(kScanTime, scanEndedCB);
}

void SparkBTControl::setAdvertisedDevice(NimBLEAdvertisedDevice *device) {
    advDevice_ = device;
}

void SparkBTControl::scanEndedCB(NimBLEScanResults results) {
    Serial.println("Scan ended.");
}

void SparkBTControl::startScan() {
    spark_dc_->resetStatus();
    NimBLEDevice::getScan()->start(kScanTime, scanEndedCB);
    Serial.println("Scan initiated");
}

bool SparkBTControl::connectToServer() {
    /** Check if we have a client we should reuse first **/
    if (NimBLEDevice::getClientListSize()) {
        /** Special case when we already know this device, we send false as the
         second argument in connect() to prevent refreshing the service database.
         This saves considerable time and power.
         */
        client_ = NimBLEDevice::getClientByPeerAddress(advDevice_->getAddress());
        if (client_) {
            if (!client_->connect(advDevice_, false)) {
                Serial.println("Reconnect failed");
                isAmpConnected_ = false;
                return false;
            }
            Serial.println("Reconnected client");
        }
        /** We don't already have a client that knows this device,
         we will check for a client that is disconnected that we can use.
         */
        else {
            client_ = NimBLEDevice::getDisconnectedClient();
        }
    }

    /** No client to reuse? Create a new one. */
    if (!client_) {
        if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
            Serial.println(
                "Max clients reached - no more connections available");
            isAmpConnected_ = false;
            return false;
        }

        client_ = NimBLEDevice::createClient();

        client_->setClientCallbacks(this, false);
        /** Set initial connection parameters: These settings are 15ms interval, 0 latency, 120ms timout.
         These settings are safe for 3 clients to connect reliably, can go faster if you have less
         connections. Timeout should be a multiple of the interval, minimum is 100ms.
         Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 51 * 10ms = 510ms timeout
         */
        // client_->setConnectionParams(12, 12, 0, 51);
        client_->setConnectionParams(18, 30, 0, 600);
        /** Set how long we are willing to wait for the connection to complete (seconds), default is 30. */
        client_->setConnectTimeout(30);
        client_->getMTU();
        if (!client_->connect(advDevice_)) {
            /** Created a client but failed to connect, don't need to keep it as it has no data */
            NimBLEDevice::deleteClient(client_);
            Serial.println("Failed to connect, deleted client");
            isAmpConnected_ = false;
            return false;
        }
    }

    if (!client_->isConnected()) {
        if (!client_->connect(advDevice_)) {
            Serial.println("Failed to connect");
            isAmpConnected_ = false;
            return false;
        }
    }

    Serial.print("Connected to: ");
    Serial.println(client_->getPeerAddress().toString().c_str());
    isAmpConnected_ = true;
    return true;
}

bool SparkBTControl::subscribeToNotifications(notify_callback notifyCallback) {

    // Subscribe to notifications from Spark
    NimBLERemoteService *service = nullptr;
    NimBLERemoteCharacteristic *characteristic = nullptr;

    if (client_) {
        service = client_->getService(SPARK_BLE_SERVICE_UUID);
        if (service) { // make sure it's not null
            characteristic = service->getCharacteristic(SPARK_BLE_NOTIF_CHAR_UUID);

            if (characteristic) { // make sure it's not null
                if (characteristic->canNotify()) {
                    Serial.printf(
                        "Subscribing to service notifications of %s\n",
                        SPARK_BLE_NOTIF_CHAR_UUID.c_str());
                    Serial.println("Notifications turned on");
                    // Descriptor 2902 needs to be activated in order to receive notifications
                    characteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue(kNotificationOn, 2, true);
                    // Subscribing to Spark characteristic
                    if (!characteristic->subscribe(true, notifyCB_)) {
                        Serial.println("Subscribe failed, disconnecting");
                        // Disconnect if subscribe failed
                        client_->disconnect();
                        return false;
                    }
                }

            } else {
                Serial.printf("%s characteristic not found.\n",
                              SPARK_BLE_NOTIF_CHAR_UUID);
                return false;
            }

            Serial.println("Done with this device.");
            return true;
        } // pSrv
        else {
            Serial.printf("Service %s not found.\n", SPARK_BLE_SERVICE_UUID);
            return false;
        }
    } // client_
    else {
        Serial.print("Client not found! Need reconnection");
        isAmpConnected_ = false;
        return false;
    }
}

// To send messages to Spark via Bluetooth LE
bool SparkBTControl::writeBLE(ByteVector &cmd, bool withDelay, bool response) {
    // DEBUG_PRINTLN("Sending message:");
    // DEBUG_PRINTVECTOR(cmd);
    // DEBUG_PRINTLN();
    if (client_ && client_->isConnected()) {
        DEBUG_PRINTLN("Connection ok");
        NimBLERemoteService *service = nullptr;
        NimBLERemoteCharacteristic *characteristic = nullptr;

        service = client_->getService(SPARK_BLE_SERVICE_UUID);
        if (service) {
            characteristic = service->getCharacteristic(SPARK_BLE_WRITE_CHAR_UUID);

            if (characteristic) {
                // if (characteristic->canWrite()) {
                // for (auto block : cmd) {

                // This it to split messages into sizes of max. max_send_size.
                // As we have chosen 173, usually no further splitting is requried.
                // SparkMessage already creates messages split into 173 byte chunks
                DEBUG_PRINT("Sending block:");
                DEBUG_PRINTVECTOR(cmd);
                DEBUG_PRINTLN();
                bool return_value = true;
                ByteVector send_cmd;
                while (cmd.size() > 0) {
                    int cmd_size = cmd.size();
                    int cut_point = min(cmd_size, bleMaxMsgSize_);
                    if (cut_point > 0) {
                        send_cmd.assign(cmd.begin(), cmd.begin() + cut_point);
                        cmd.assign(cmd.begin() + cut_point, cmd.end());
                        return_value = characteristic->writeValue(send_cmd.data(), send_cmd.size(), response);
                    }
                }
                if (return_value) {
                    // Delay seems to be required in order to not lose any packages.
                    // Seems to be more stable with a short delay
                    // also seems to be not working for Spark Mini without a delay.s
                    // TEST SPark GO
                    if (withDelay) {
                        delay(80);
                    }
                } else {
                    Serial.println("There was an error with writing!");
                    // Disconnect if write failed
                    client_->disconnect();
                    isAmpConnected_ = false;
                    return false;
                }
                //} // For each block
                //}  // if can write
            } // if characteristic
            else {
                Serial.printf("Characteristic %s not found. Disconnecting client.\n",
                              SPARK_BLE_WRITE_CHAR_UUID);
                // Disconnect if write failed
                client_->disconnect();
                isAmpConnected_ = false;
                return false;
            }
        } // if service
        else {
            Serial.printf("%s service not found. Disconnecting client.\n", SPARK_BLE_SERVICE_UUID);
            // Disconnect if write failed
            client_->disconnect();
            isAmpConnected_ = false;
            return false;
        }
        DEBUG_PRINTLN("Done with this command!");
        return true;
    } else {
        isAmpConnected_ = false;
        return false;
    }
}

void SparkBTControl::onResult(NimBLEAdvertisedDevice *advertisedDevice) {

    if (advertisedDevice->isAdvertisingService(
            NimBLEUUID(SPARK_BLE_SERVICE_UUID))) {
        Serial.println("Found Spark, connecting.");
        /** stop scan before connecting */
        // Commented as workaround, might need to get back here, is currently with DataControl;
        NimBLEDevice::getScan()->stop();
        /** Save the device reference in a global for the client to use*/
        setAdvertisedDevice(advertisedDevice);
        /** Ready to connect now */
        isConnectionFound_ = true;
        // delay(500);
    }
}

void SparkBTControl::startServer() {
    Serial.println("Starting NimBLE Server");

    /** sets device name */
    NimBLEDevice::init(btNameBle);

    /** Optional: set the transmit power, default is 3db */
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
    NimBLEDevice::setSecurityAuth(
        /*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);

    server_ = NimBLEDevice::createServer();
    server_->setCallbacks(this);

    sparkService_ = server_->createService(
        SPARK_BLE_SERVICE_UUID);
    sparkWriteCharacteristic_ =
        sparkService_->createCharacteristic(
            SPARK_BLE_WRITE_CHAR_UUID,
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);

    uint8_t initialWriteValue[] = {0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
                                   0x77, 0x77, 0x77};

    sparkWriteCharacteristic_->setValue(initialWriteValue);
    sparkWriteCharacteristic_->setCallbacks(this);

    sparkNotificationCharacteristic_ =
        sparkService_->createCharacteristic(
            SPARK_BLE_NOTIF_CHAR_UUID,
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    uint8_t initialNotificationValue[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                                          0x88, 0x88, 0x88, 0x88};
    sparkNotificationCharacteristic_->setValue(initialNotificationValue);
    sparkNotificationCharacteristic_->setCallbacks(this);

    /** Start the services when finished creating all Characteristics and Descriptors */
    sparkService_->start();
    advertising_ = NimBLEDevice::getAdvertising();

    advertising_->addServiceUUID(sparkService_->getUUID());

    /** Add the services to the advertisment data **/
    /** If your device is battery powered you may consider setting scan response
     *  to false as it will extend battery life at the expense of less data sent.
     */
    advertising_->setScanResponse(true);
    advertising_->start();

    Serial.println("Advertising Started");
}

void SparkBTControl::onWrite(NimBLECharacteristic *pCharacteristic) {

    // This method will be triggered if receiving data in AMP mode.
    DEBUG_PRINT(pCharacteristic->getUUID().toString().c_str());
    DEBUG_PRINTLN(": onWrite()");
    string rxValue = pCharacteristic->getValue();
    ByteVector byteVector;
    byteVector.clear();

    if (rxValue.length() > 0) {
        for (int i = 0; i < rxValue.length(); i++) {
            byteVector.push_back((byte)(rxValue[i]));
        }
    }
    // Add message to Queue for processing
    spark_dc_->queueMessage(byteVector);
}

void SparkBTControl::onSubscribe(NimBLECharacteristic *pCharacteristic,
                                 ble_gap_conn_desc *desc, uint16_t subValue) {
    string str = "Address: ";
    str += string(NimBLEAddress(desc->peer_ota_addr)).c_str();
    if (subValue == 0) {
        str += " Unsubscribed to ";
    } else if (subValue == 1) {
        str += " Subscribed to notifications for ";
    } else if (subValue == 2) {
        str += " Subscribed to indications for ";
    } else if (subValue == 3) {
        str += " Subscribed to notifications and indications for ";
    }
    str += string(pCharacteristic->getUUID());

    Serial.println(str.c_str());
};

void SparkBTControl::notifyClients(const vector<CmdData> &msg) {
    if (server_) {
        NimBLEService *service = server_->getServiceByUUID(SPARK_BLE_SERVICE_UUID);
        if (service) {
            NimBLECharacteristic *characteristic = service->getCharacteristic(
                SPARK_BLE_NOTIF_CHAR_UUID);
            if (characteristic) {
                for (auto block : msg) {
                    /*DEBUG_PRINTLN("Sending data:");
                    DEBUG_PRINTVECTOR(block);
                    DEBUG_PRINTLN();*/
                    characteristic->setValue(&block.data.data()[0], block.data.size());
                    characteristic->notify();
                }
                DEBUG_PRINTLN("Clients notified.");
            }
        }
    }

    if (btSerial && btSerial->hasClient()) {
        DEBUG_PRINTLN("Sending message via BT Serial:");
        for (auto chunk : msg) {
            ByteVector chunkData = chunk.data;
            for (auto by : chunkData) {
                if (by < 16) {
                    DEBUG_PRINT("0");
                }
                DEBUG_PRINT(by, HEX);
                btSerial->write(by);
            }
            DEBUG_PRINTLN();
            DEBUG_PRINTF("Free Heap size: %d\n", ESP.getFreeHeap());
        }
    }
}

// AMP Mode
void SparkBTControl::onConnect(NimBLEServer *pServer_,
                               ble_gap_conn_desc *desc) {
    isAppConnectedBLE_ = true;
    Serial.println("Multi-connect support: start advertising");
    //	pServer->updateConnParams(desc->conn_handle, 40, 80, 5, 51);
    NimBLEDevice::startAdvertising();
}

// APP mode
void SparkBTControl::onConnect(NimBLEClient *pClient_) {
    NimBLEClientCallbacks::onConnect(pClient_);
    spark_dc_->getAmpName();
}

// AMP mode when App is disconnected
void SparkBTControl::onDisconnect(NimBLEServer *pServer_) {
    Serial.println("Client disconnected");
    isAppConnectedBLE_ = false;
    notificationCount = 0;
    Serial.println("Start advertising");
    NimBLEDevice::startAdvertising();
}

// APP mode when Amp is disconnected
void SparkBTControl::onDisconnect(NimBLEClient *pClient_) {
    isAmpConnected_ = false;
    isConnectionFound_ = false;
    if (!(NimBLEDevice::getScan()->isScanning())) {
        startScan();
    }
    NimBLEClientCallbacks::onDisconnect(pClient_);
}

void SparkBTControl::stopScan() {
    if (isScanning()) {
        Serial.println("Scan stopped");
        NimBLEDevice::getScan()->stop();
    } else {
        Serial.print("Scan is not running");
    }
}

void SparkBTControl::serialCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    if (event == ESP_SPP_SRV_OPEN_EVT) {
        Serial.println("Client Connected");
        isAppConnectedSerial_ = true;
    }
    if (event == ESP_SPP_CLOSE_EVT) {
        Serial.println("Client disconnected");
        isAppConnectedSerial_ = false;
    }
}

void SparkBTControl::startBTSerial() {
    btSerial = new BluetoothSerial();
    btSerial->register_callback(serialCallback);
    // btStart();
    if (btSerial->begin(btNameSerial.c_str(), false)) {
        Serial.printf("Started BT Serial with name %s \n",
                      btNameSerial.c_str());
        btSerial->flush();
    } else {
        Serial.println("BT Serial start failed!");
    }
}

void SparkBTControl::stopBTSerial() {
    // Stop Bluetooth Serial
    btSerial->end();
    delete btSerial;
    btSerial = nullptr;
    Serial.println("BT Serial stopped");
}

void SparkBTControl::stopBLEServer() {

    if (server_) {
        Serial.println("Switching off BLE server");
        server_->stopAdvertising();
    }
}
