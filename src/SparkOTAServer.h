/*
 * SparkOTAServer.h
 *
 *  Created on: 10.10.2021
 *      Author: steffen
 */

#ifndef SPARKOTASERVER_H_
#define SPARKOTASERVER_H_

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <Arduino.h>
#include "Credentials.h"


class SparkOTAServer {
public:
	SparkOTAServer();
	virtual ~SparkOTAServer();

	bool init();
	void handleClient();

private:
	const WifiCredentials wifiCred;
	const char *host = "ignitron";
	const char *ssid = wifiCred.ssid;
	const char *password = wifiCred.password;
	const int connectTimeout = 4000;

	static WebServer server;

	//Login page
	static const char *loginIndex;

	// Server Index Page
	static const char *serverIndex;
};

#endif /* SPARKOTASERVER_H_ */
