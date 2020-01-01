/**
 *  ATMO - Home-made Weather Station
 *
 *  @author Benjamin Pereto
 *  @licence GPLv3
 */
 
#include <Wire.h>
#include <SPI.h>

#include "vars.h"

#define BME_CS 16
#define SEALEVELPRESSURE_HPA (1013.25)

//
// INIT
//
Adafruit_BME680 bme(BME_CS); // hardware SPI
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // hardware I2C

bool read_bme = false;
bool read_tsl = false;

int LOOP = 0;

float gas_baseline = 2500;
int   gas_baseline_count = 0;

//
// CONFIG
//

void setup() {

  Serial.begin(9600);
  while (!Serial);

  Serial.println(F("-=# Atmo Weather Station #=-"));
  Serial.println("I'm awake.");

  // DETECT & CONFIGURE BME
  if (bme.begin()) {
    Serial.println(F("BME680 detected!"));

    read_bme = true;

    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms

    ATMO_gas_baseline(); // burn in gas sensor
  } else {
    Serial.println("Ops! BME680 could not be found!");
  }

  //
  // DETECT & CONFIGURE TSL2591
  //
  if (tsl.begin())
  {
    Serial.println(F("TSL2591 detected!"));
    read_tsl = true;
    //tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
    //tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS)
    displayTSLSensorDetails();
  } else {
    Serial.println(F("Ops! TSL2591 could not be found!"));
  }

  //
  // connect to wifi
  //
  WIFI_connect(wlan_config);
}

void loop() {

  AtmoValues_t values = {
    .temperature = -1,
    .humidity = -1,
    .pressure = -1,
    .gas = -1,
    .iaq = -1,
    .altitude = -1,
    .light = -1
  };

  //
  // READ BME
  //
  if (read_bme) {
    if (bme.performReading()) {
      
      // cycle gas baseline
      if ((gas_baseline_count++) % 5 == 0) ATMO_gas_baseline();
      
      values.temperature = bme.temperature + atmo_temperature_offset;
      values.pressure = bme.pressure / 100.0F;
      values.humidity = bme.humidity;
      values.gas = bme.gas_resistance / 1000.0;
      values.altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    } else {
      Serial.println("Failed to perform reading on BME680 :(");
    }
  }

  //
  // READ TSL
  //
  if ( read_tsl ) {
    sensors_event_t event;
    tsl.getEvent(&event);

    if ((event.light == 0) |
        (event.light > 4294966000.0) |
        (event.light < -4294966000.0))
    {
      /* If event.light = 0 lux the sensor is probably saturated */
      /* and no reliable data could be generated! */
      /* if event.light is +/- 4294967040 there was a float over/underflow */
      Serial.println("Invalid data (adjust gain or timing)");
    } else {
      values.light = event.light;
    }
  }

  // IAQ
  values.iaq = ATMO_calc_iaq(values);

  Serial.println("\nSensor Readings:");
  Serial.println("Temperature = " + String(values.temperature, 2) + "°C");
  Serial.println("Pressure    = " + String(values.pressure)       + " hPa");
  Serial.println("Humidity    = " + String(values.humidity, 1)    + "%");
  Serial.println("Gas         = " + String(values.gas)            + " kOhms");
  Serial.println("Air Quality Index = " + String(values.iaq)      + "\n");
  
  //
  // SEND VALUES TO MQTT BROKER
  //
  if (LOOP >= 12 && bool_mqtt_publish) {
    LOOP = 0;
    MQTT_send_values(values);
  }

  //
  // SLEEP
  // @TODO: replace with deep sleep when used in battery mode
  //        keep in mind that setup() is executed after waking up from deep sleep...
  //
  Serial.print("LOOP = ");
  Serial.println(LOOP);
  LOOP++;
  Serial.println("waiting.");
  delay(5000);
  // ESP.deepSleep(loop_delay);
}


/*
 * Create Baseline of Gas Sensor
 * it needs stabilization
 */
float ATMO_gas_baseline() {
  Serial.println("\n=> Getting a new gas baseline");
  int readings = 10;
  for (int i = 1; i <= readings; i++) {
    gas_baseline += bme.readGas();
  }
  gas_baseline = gas_baseline / readings;
  Serial.println("Gas Baseline = " + String(gas_baseline, 3));
  return gas_baseline;
}


/*
 * Calculate Indoor Air Quality Index
 */
int ATMO_calc_iaq(AtmoValues_t values) {

  int humidity_score = ATMO_get_humidity_score(values.humidity);
  int gas_score = ATMO_get_gas_score();
  
  float air_quality_score = humidity_score + gas_score;
  int air_quality_index = (100 - air_quality_score) * 5;

  //Serial.print("Air Quality Score = ");
  //Serial.println(air_quality_score);
  //Serial.print("Air Quality Index = ");
  //Serial.println(air_quality_index);
  //Serial.println("Air Quality Score composited of " + String(humidity_score) + "% Humidity and " + String(gas_score) + "% Gas");

  return air_quality_index;
}


/*
 * Calculate Humidity Score for IAQ index 
 */
int ATMO_get_humidity_score(float current_humidity) {
  
  float hum_baseline = 40;
  int humidity_score;

  if (current_humidity >= 38 && current_humidity <= 42) {
    // Humidity +/-5% around optimum
    humidity_score = 0.25 * 100;
  } else {
    // Humidity is sub-optimal
    if (current_humidity < 38) {
      humidity_score = 0.25 / hum_baseline * current_humidity * 100;
    } else {
      humidity_score = ((-0.25 / (100 - hum_baseline) * current_humidity) + 0.416666) * 100;
    }
  }
  return humidity_score;
}

/*
 * Calculate Gas Score for IAQ index 
 */
int ATMO_get_gas_score() {

  int   gas_lower_limit = 10000;  // Bad air quality limit
  int   gas_upper_limit = 300000; // Good air quality limit
  
  int gas_score = (0.75 / (gas_upper_limit - gas_lower_limit) * gas_baseline - (gas_lower_limit * (0.75 / (gas_upper_limit - gas_lower_limit)))) * 100.00;
  
  // Sometimes gas readings can go outside of expected scale maximum
  if (gas_score > 75) gas_score = 75;
  if (gas_score <  0) gas_score = 0;
  
  return gas_score;
}


/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
void displayTSLSensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println(F(" lux"));
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println(F(" lux"));
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution, 4); Serial.println(F(" lux"));
  Serial.print  ("Gain:         ");
  tsl2591Gain_t gain = tsl.getGain();
  switch (gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println("1x (Low)");
      break;
    case TSL2591_GAIN_MED:
      Serial.println("25x (Medium)");
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println("428x (High)");
      break;
    case TSL2591_GAIN_MAX:
      Serial.println("9876x (Max)");
      break;
  }
  Serial.print  ("Timing:       ");
  Serial.print((tsl.getTiming() + 1) * 100, DEC);
  Serial.println(" ms");
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
}
