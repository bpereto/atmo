#pragma once
#include "Adafruit_MQTT.h"
#include "vars.h"

struct MQTTConfig_t {
  char* server;
  int port;
  char* user;
  char* password;
};

bool MQTT_connect();
bool MQTT_send_values(AtmoValues_t values);
