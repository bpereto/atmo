#!/usr/bin/env python3

"""A MQTT to InfluxDB Bridge
This script receives MQTT data and saves those to InfluxDB.
"""

import re
import os
import ssl
from typing import NamedTuple

import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient

TLS_CERT_PATH = '/etc/pki/tls/cert.pem'

INFLUXDB_ADDRESS = os.getenv('BRIDGE_INFLUXDB_ADDRESS', 'localhost')
INFLUXDB_USER = os.getenv('BRIDGE_INFLUXDB_USER', 'atmo')
INFLUXDB_PASSWORD = os.getenv('BRIDGE_INFLUXDB_PASSWORD', 'IOTRocks!')
INFLUXDB_DATABASE = os.getenv('BRIDGE_INFLUXDB_DATABASE', 'atmo')

MQTT_ADDRESS = os.getenv('BRIDGE_MQTT_ADDRESS', 'localhost')
MQTT_PORT = os.getenv('BRIDGE_MQTT_PORT', 1883)
MQTT_USER = os.getenv('BRIDGE_MQTT_USER', 'atmo')
MQTT_PASSWORD = os.getenv('BRIDGE_MQTT_PASSWORD', 'IOTRocks!')
MQTT_SECURE = os.getenv('BRIDGE_MQTT_SECURE', False)
MQTT_TOPIC = '/sensors/+/+'  # /sensors/[atmo]/temperature
MQTT_REGEX = '/sensors/([^/]+)/([^/]+)'
MQTT_CLIENT_ID = 'MQTTInfluxDBBridge'

influxdb_client = InfluxDBClient(INFLUXDB_ADDRESS, 8086, INFLUXDB_USER, INFLUXDB_PASSWORD, None)


class SensorData(NamedTuple):
    device: str
    measurement: str
    value: float


def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC)


def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    print(msg.topic + ' ' + str(msg.payload))
    sensor_data = _parse_mqtt_message(msg.topic, msg.payload.decode('utf-8'))
    if sensor_data is not None:
        _send_sensor_data_to_influxdb(sensor_data)


def _parse_mqtt_message(topic, payload):
    match = re.match(MQTT_REGEX, topic)
    if match:
        device = match.group(1)
        measurement = match.group(2)
        if measurement == 'status':
            return None
        return SensorData(device, measurement, float(payload))
    else:
        return None


def _send_sensor_data_to_influxdb(sensor_data):
    json_body = [
        {
            'measurement': sensor_data.measurement,
            'tags': {
                'device': sensor_data.device
            },
            'fields': {
                'value': sensor_data.value
            }
        }
    ]
    influxdb_client.write_points(json_body)


def _init_influxdb_database():
    databases = influxdb_client.get_list_database()
    if len(list(filter(lambda x: x['name'] == INFLUXDB_DATABASE, databases))) == 0:
        influxdb_client.create_database(INFLUXDB_DATABASE)
    influxdb_client.switch_database(INFLUXDB_DATABASE)


def main():
    _init_influxdb_database()

    mqtt_client = mqtt.Client(MQTT_CLIENT_ID)
    if MQTT_SECURE:
        MQTT_PORT = 8883
        mqtt_client.tls_set(ca_certs=TLS_CERT_PATH, tls_version=ssl.PROTOCOL_TLSv1_2)
        mqtt_client.tls_insecure_set(False)
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_ADDRESS, MQTT_PORT)
    mqtt_client.loop_forever()


if __name__ == '__main__':
    print('MQTT to InfluxDB bridge')
    main()

