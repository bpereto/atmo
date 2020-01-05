# Atmo - Weather Station

"Atmo" is a homemade indoor/outdoor weather station

The basis is a small weather station based on Arduino, which is equipped with sensors.   
The sensors are read out periodically and the data is transmitted centrally via MQTT and stored in an InfluxDB. The Data can be visualized with Grafana Dashboards.

![](docs/img/atmo.png?raw=true)

There is a lot to improve in this Project and far from completed, contributions are welcome.

## Hardware

Tested Boards:
* Adafruit Huzzah32 Feather (ESP32)

Used Sensors:
* Adafruit BME680 (Temperature, Humidity, Pressure, Air Quality)
* Adafruit TSL2591 (Lux)

## Installation & Configurations

### Configure ESP

Blogpost: https://blog.sandchaschte.ch/de/posts/atmo-weather-station-part2

* Install Boards & Libraries in Arduino IDE
* `cp atmo/config.default.ino atmo/config.ino`
* Configure WiFi and MQTT Settings in `atmo/config.ino`
* Compile & Upload.

### Install MQTT, InfluxDB, Grafana

* `docker-compose up`

Default User in InfluxDB, MQTT and Bridge is `atmo` and Password is `IOTRocks!`.   

To change Passwords: 
* Create File `.secrets`   

```
INFLUXDB_USER_PASSWORD=<my_password>   
BRIDGE_INFLUXDB_PASSWORD=<my_password>   
BRIDGE_MQTT_PASSWORD=<my_password>   
```

* Update Password in `mqtt/passwd`   

```
mosquitto_passwd -c mosquitto/passwd atmo
```

### Usage
* MQTT Port 1883 (without TLS)
* InfluxDB Port 8086
* Grafana Port 3000 (http)

### Grafana
* Import Dashboard: `dashboards/atmo.json`

## Links

* https://blog.sandchaschte.ch/en/posts/atmo-weather-station/
