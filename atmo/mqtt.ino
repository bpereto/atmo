#pragma once
#include "vars.h"

//
// MQTT
//
String default_topic_path = "/sensors/";
Adafruit_MQTT_Client mqtt(&wlan_client, mqtt_config.server, mqtt_config.port, mqtt_config.user, mqtt_config.password);
  
//
// INIT
//

/**
 * Function to send sensors readings to MQTT Broker in specified topics
 */
bool MQTT_send_values(AtmoValues_t values) {

    bool ret;
    MQTT_connect();

    Adafruit_MQTT_Publish topic = Adafruit_MQTT_Publish(&mqtt, String(default_topic_path + hostname + "/dummy").c_str());

    Serial.print(F("\nSending to MQTT...\n"));

    // TEMPERATURE
    topic = Adafruit_MQTT_Publish(&mqtt, String(default_topic_path + hostname + "/temperature").c_str());
    if (! topic.publish(values.temperature)) {
      Serial.println(F("Failed to send temperature"));
    }

    // HUMIDITY
    topic = Adafruit_MQTT_Publish(&mqtt, String(default_topic_path + hostname + "/humidity").c_str());
    if (! topic.publish(values.humidity)) {
      Serial.println(F("Failed to send humidity"));
    }

    // PRESSURE
    topic = Adafruit_MQTT_Publish(&mqtt, String(default_topic_path + hostname + "/pressure").c_str());
    if (! topic.publish(values.pressure)) {
      Serial.println(F("Failed to send pressure"));
    }

    // GAS
    topic = Adafruit_MQTT_Publish(&mqtt, String(default_topic_path + hostname + "/gas").c_str());
    if (! topic.publish(values.gas)) {
      Serial.println(F("Failed to send gas"));
    }

    // IAQ
    topic = Adafruit_MQTT_Publish(&mqtt, String(default_topic_path + hostname + "/iaq").c_str());
    if (! topic.publish(values.iaq)) {
      Serial.println(F("Failed to send IAQ"));
    }

    // ALTITUDE
    topic = Adafruit_MQTT_Publish(&mqtt, String(default_topic_path + hostname + "/altitude").c_str());
    if (! topic.publish(values.altitude)) {
      Serial.println(F("Failed to send real altitude"));
    }

    // LIGHT
    topic = Adafruit_MQTT_Publish(&mqtt, String(default_topic_path + hostname + "/light").c_str());
    if (! topic.publish(values.light)) {
      Serial.println(F("Failed to send light"));
    }
}

/** 
 * Function to connect and reconnect as necessary to the MQTT server.
 * Should be called in the loop function and it will take care of connecting.
 */
bool MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return true;
  }

  Serial.print("Connecting to MQTT ");
  Serial.print(F(mqtt_config.server));
  Serial.println();

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         return false;
       }
  }
  Serial.println("MQTT Connected!");
  return true;
}
