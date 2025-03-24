/*
 * SparkBTControl.h
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARKBLECONTROL_H_
#define SPARKBLECONTROL_H_

#include "Config_Definitions.h"
#include "SparkTypes.h"
#include <Arduino.h>
#include <BluetoothSerial.h>
#include <NimBLEDevice.h>
#include <string>
#include <vector>

#include "SparkDataControl.h"

using namespace std;

// Service and characteristics UUIDs of Spark Amp

using ByteVector = vector<byte>;
class SparkDataControl;

// Forward declaration of Callbacks classes, does nothing special, only default actions
// class ClientCallbacks: public NimBLEClientCallbacks {};

class SparkBTControl : public NimBLEAdvertisedDeviceCallbacks,
                       NimBLECharacteristicCallbacks,
                       NimBLEServerCallbacks,
                       NimBLEClientCallbacks {
public:
    SparkBTControl();
    SparkBTControl(SparkDataControl *dc);
    virtual ~SparkBTControl();

    /**
     * @brief  Checks if a Spark amp has been found to connect
     *
     * When scanning for devices in APP mode, this function can be used
     * to check if a Spark Amp has been found to connect
     *
     *
     * @return TRUE if found
     */
    const bool isConnectionFound() const {
        return isConnectionFound_;
    }
    /**
     * @brief  To check if Ignitron is connected to the Spark Amp
     *
     * This is used in a loop to check if the connection to the Spark Amp is active
     *
     *
     * @return TRUE if connected
     */
    const bool isAmpConnected() const {
        return isAmpConnected_;
    }
    /**
     * @brief  To check if the Spark App is connected to Ignitron
     *
     * This is used in a loop to check if the connection to the Spark App is active
     *
     *
     * @return TRUE if connected
     */
    const bool isAppConnected() const {
        return isAppConnectedBLE_ || isAppConnectedSerial_;
    }
    /**
     * @brief  Initiates connection to the Spark Amp
     *
     * After a connection has been found, this function can be used to
     * establish a connection to the Spark Amp
     *
     *
     * @return TRUE if connection was successful
     */
    bool connectToServer();
    /**
     * @brief  Subscribe to notifications from Spark Amp.
     *
     * This is called by SparkDataControl so it gets notified when the Spark Amp
     * returns messages to Ignitron
     *
     * @param notifyCallback Function to be called when notifications are received
     *
     * @return TRUE if successful
     */
    bool subscribeToNotifications(notify_callback notifyCallback = nullptr);
    /**
     * @brief  Send messages via BLE to Spark Amp or App
     *
     * Send messages/commands/acknowledgements to the Spark Amp and App
     * This will trigger a change in the Spark Amp setting.
     *
     * @param cmd vector of byte vectors containing the messages. Each byte vector is a chunk to be sent to Spark Amp/App
     * @param response indicate if a response is expected. Always false in this case
     *
     * @return TRUE if successful
     */
    bool writeBLE(ByteVector &cmd, bool withDelay = false, bool response = false);
    /**
     * @brief  Initializes Ignitron BLE as client to connect to the Spark Amp
     *
     * Sets up the BLE connection and initiates a scan for the Spark Amp. When
     * servers have been found, the notifyCallback function is called back.
     *
     * @param notifyCallback function called back when servers are found
     *
     */
    void initBLE(notify_callback notifyCallback = nullptr);
    /**
     * @brief  Starts a scan for servers to connect to.
     *
     * Initiates a scan for servers. To be called when not connected to a server
     *
     */
    void startScan();

    /**
     * @brief  Checks if a scan is currently running
     *
     * @return TRUE if scan is running
     */
    const bool isScanning() const {
        return NimBLEDevice::getScan()->isScanning();
    }

    /**
     * @brief  Stops a running scan.
     *
     * Stops a scan for servers. Has to be called before establishing a connection to a server
     *
     */
    void stopScan();

    /**
     * @brief  Starts Ignitron BLE as server.
     *
     * This enables the Spark App to connect to Ignitron to send presets.
     *
     */
    void startServer();
    /**
     * @brief  Notifies Spark App for any new messages
     *
     * This is used to notify Spark App mainly during the initiation of a connection
     * so that the Spark App can connect to Ignitron
     *
     */
    void notifyClients(const vector<CmdData> &msg);

    void stopBLEServer();

    void startBTSerial();
    void stopBTSerial();

    bool byteAvailable() {
        if (btSerial != NULL) {
            return btSerial->available();
        } else
            return false;
    }
    byte readByte() {
        if (btSerial != NULL) {
            return btSerial->read();
        } else
            return false;
    }

    void setMaxBleMsgSize(int size) {
        if (size > 0)
            bleMaxMsgSize_ = size;
    }

private:
    const string SPARK_BLE_SERVICE_UUID = "FFC0";
    const string SPARK_BLE_WRITE_CHAR_UUID = "FFC1";
    const string SPARK_BLE_NOTIF_CHAR_UUID = "FFC2";

    /** Create a single global instance of the callback class to be used by all clients */
    // static ClientCallbacks clientCB;
    NimBLEAdvertisedDevice *advDevice_;
    NimBLEClient *client_ = nullptr;

    BluetoothSerial *btSerial = nullptr;
    const string btNameBle = "Spark 40 BLE";      // Spark 40 BLE
    const string btNameSerial = "Spark 40 Audio"; // Spark 40 Audio

    bool isAmpConnected_ = false;
    bool isConnectionFound_ = false;
    // isClientConnected will be set when a client is connected to ESP in AMP mode
    bool isAppConnectedBLE_ = false;
    static bool isAppConnectedSerial_;
    notify_callback notifyCB_;

    const uint32_t kScanTime = 0; /** 0 = scan forever */
    const uint8_t kNotificationOn[2] = {0x1, 0x0};
    int bleMaxMsgSize_ = 0x64;

    static void scanEndedCB(NimBLEScanResults results);
    void onResult(NimBLEAdvertisedDevice *advertisedDevice);
    void setAdvertisedDevice(NimBLEAdvertisedDevice *device);
    void onDisconnect(NimBLEClient *client);
    void onConnect(NimBLEClient *client);

    // Server mode functions
    NimBLEServer *server_ = nullptr;
    NimBLEService *sparkService_ = nullptr;
    NimBLECharacteristic *sparkWriteCharacteristic_ = nullptr;
    NimBLECharacteristic *sparkNotificationCharacteristic_ = nullptr;
    NimBLEAdvertising *advertising_ = nullptr;

    SparkDataControl *spark_dc_;
    void onWrite(NimBLECharacteristic *characteristic);
    void onSubscribe(NimBLECharacteristic *characteristic,
                     ble_gap_conn_desc *desc, uint16_t subValue);
    void onConnect(NimBLEServer *server, ble_gap_conn_desc *desc);
    void onDisconnect(NimBLEServer *server);

    static void serialCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

    int notificationCount = 0;
};

#endif /* SPARKBLECONTROL_H_ */
