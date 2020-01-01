#pragma once
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "Adafruit_TSL2591.h"

//
// Global
//
extern String hostname;
extern int loop_delay;

struct AtmoValues_t {
  float temperature;
  float humidity;
  float pressure;
  float gas;
  int iaq;
  float altitude;
  int light;
};

extern float atmo_temperature_offset;

//
// MQTT
//
#include "Adafruit_MQTT_Client.h"
#include "mqtt.h"

extern Adafruit_MQTT_Client mqtt;
extern MQTTConfig_t mqtt_config;
extern bool bool_mqtt_publish;

//
// WIFI
//
#include <WiFiClient.h>
//#include <WiFiClientSecure.h>
#include "wifi.h"

WiFiClient wlan_client;

extern WifiConfig_t wlan_config;
