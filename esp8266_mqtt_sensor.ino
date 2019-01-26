/**
  @file esp8266_mqtt_sensor.ino
  @author Christoph Jurczyk
  @date January 25, 2019
  @brief Main file for ESP8266 MQTT sensor node

  This sketch reads environmental data from an DHT11 sensor, sends it via MQTT
  to a MQTT broker and puts the ESP8266 to sleep mode to save power.
  For this wake up process D0 has to be connected to reset pin of the ESP8266.
  Note: That the wake up connection of D0 to reset has to be removed for programming.

  @section license License
  This library is released under the GNU General Public License v3.0.
*/


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <MQTTClient.h>
#include <DHT.h>
#include "wifi.h"

/** The follwoing lines enables an easy activation and
    deactivation of the debug infomration. */
/** Uncomment `#define DEBUG` to enable debug information */
//#define DEBUG
#ifdef DEBUG
#define DBEGIN(...)    Serial.begin(__VA_ARGS__)
#define DPRINT(...)    Serial.print(__VA_ARGS__)
#define DPRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#define DBEGIN(...)
#define DPRINT(...)
#define DPRINTLN(...)
#endif

/** IP-address of MQTT host */
const char* MQTThost = "192.168.1.38";

/** MQTT path for measurements */
const char* MQTTpathTemp = "/LivingRoom/temperature";
const char* MQTTpathHumi = "/LivingRoom/humidity";

/** Time between wake ups in s */
#define WAKEUP_TIME 120

/** Object declaration */
WiFiClient net;
MQTTClient mqtt;
/** Number of trials to connect to wifi and mqtt server */
#define WIFI_MQTT_TRIALS 10

/** DHT configuration */
#define DHTTYPE DHT11
#define DHTPIN  14   // GPIO14 (D5)
DHT dht(DHTPIN, DHTTYPE, 11);

/** Type definition for sensor values to share between function */
typedef struct {
  float temperature = 0; /* Temperature in degC */
  float humidity = 0;  /* Humidity in % */
} TypeSensorData;


void setup() {
  /** Print debug header */
  DBEGIN(115200);
  DPRINTLN();
  DPRINTLN("Booting...");

  /** Setup DHT */
  dht.begin();

  /** Setup wifi connection*/
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  /** Setup MQTT connection */
  mqtt.begin(MQTThost, net);

  /** Connect Wifi and MQTT */
  MqttWifiConnect();
  DPRINTLN("Setup completed...");
}


void loop() {
  /** Check for open MQTT connection */
  if (!mqtt.connected()) {
    MqttWifiConnect();
  }

  /** Process MQTT messages */
  mqtt.loop();

  /** Read sensor values */
  TypeSensorData sensor_data = readSensorData();

  /** Send measurements via MQTT */
  sendSensorData(sensor_data);

  /** Put ESP in sleep mode to save energy */
  espDeepSleep();
}



//----------------------------------------------------------------------------

/**
  @brief Function to connect Wifi and MQTT
*/
void MqttWifiConnect(void) {
  uint8_t trial_counter = 0;

  /** Connect to Wifi */
  while ((WiFi.waitForConnectResult() != WL_CONNECTED)) {
    trial_counter++;
    WiFi.begin(ssid, password);
    DPRINTLN("WiFi connection failed. Retry...");
    delay(1000);

    if (trial_counter > WIFI_MQTT_TRIALS)
    {
      DPRINTLN("Could not connect to WiFi!");
      espDeepSleep();
    }
  }

  DPRINT("Wifi connection successful - IP-Address: ");
  DPRINTLN(WiFi.localIP());

  /** Connect to MQTT server */
  trial_counter = 0;
  DPRINTLN("Try to connect to MQTT...");
  while (!mqtt.connect(MQTThost)) {
    trial_counter++;
    DPRINTLN("Try to connect to MQTT...");

    if (trial_counter > WIFI_MQTT_TRIALS)
    {
      DPRINTLN("Could not connect to MQTT server!");
      espDeepSleep();
    }
  }

  DPRINTLN("MQTT connected!");
}


/**
   @brief Function to send measurements via MQTT

   @param sensor_data
   Is the sensor data which should be commited.
*/
void sendSensorData(TypeSensorData sensor_data) {
  DPRINTLN("Sending... ");
  if (!isnan(sensor_data.temperature) || !isnan(sensor_data.humidity)) {
    mqtt.publish(MQTTpathTemp, String(sensor_data.temperature));
    mqtt.publish(MQTTpathHumi, String(sensor_data.humidity));

    DPRINT("Temperature: ");
    DPRINT(String(sensor_data.temperature));
    DPRINT(" Humidity: ");
    DPRINTLN(String(sensor_data.humidity));
  } else {
    DPRINTLN("Error: sensor_data.temperature or sensor_data.humidity is a nan!");
  }
}


/**
  @brief Function to read sensor data.
*/
TypeSensorData readSensorData(void) {
  TypeSensorData data;

  data.temperature = dht.readTemperature();
  data.humidity = dht.readHumidity();

  return data;
}

/**
  @brief Function to put ESP in deep sleep
*/
void espDeepSleep(void)
{
  DPRINTLN("Bye bye...");

  /** Wait for a moment... */
  delay(100);

  ESP.deepSleep(WAKEUP_TIME * 1e6, WAKE_RF_DEFAULT);
  DPRINTLN("Ups...this line should be never reached...");
}
