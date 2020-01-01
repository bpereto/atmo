#pragma once

struct WifiConfig_t {
  char* ssid;
  char* password;
};

void WIFI_connect(WifiConfig_t wlan_config);
