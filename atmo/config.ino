#pragma once
#include "vars.h"

bool bool_mqtt_publish = true;
int loop_delay = 60000; // 60 seconds

String hostname = "atmo";

WifiConfig_t wlan_config = {
  .ssid = "",
  .password = ""
};

MQTTConfig_t mqtt_config = {
  .server = "",
  .port = 1883,
  .user = "atmo",
  .password = "IOTRocks!"
};

//
// ATMO Settings
//
int atmo_send_intervall = 60; // submit values every 60 seconds
float atmo_temperature_offset = -2.0;
