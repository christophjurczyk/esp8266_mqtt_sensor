# ESP8266 MQTT Sensor
This sketch can provide sensor information of an DHT11 with an ESP8266 via WiFi to an MQTT server. The ESP8266 will wake up from deep sleep mode every 120 s (`WAKEUP_TIME`) and transmit to an MQTT server.
The design was tested on a NodeMCU DEVKIT 1.0.

## Wiring
|Pin | GPIO |     |
|----|------|-----|
|D0  |GPIO16|RST  |
|D5  |GPIO14|DHT data pin|

## Configuration
Before using you have to configure your Wifi SSID and password in the wifi.h file and the MQTT paramaters in the .ino file.
