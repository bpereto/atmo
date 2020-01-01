#pragma once
#include "vars.h"
#include <WiFi.h>

void WIFI_connect(WifiConfig_t wlan_config) {
  Serial.println(); Serial.println();
  WiFi.begin(wlan_config.ssid, wlan_config.password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.print("Connected to the WiFi network: ");
  Serial.println(wlan_config.ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
