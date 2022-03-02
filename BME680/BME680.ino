#include <ESP8266WiFi.h>
#include <InfluxDb.h>
#include "bsec.h"

// Sensorwerte
#define LOCATION "Baumhaus"
#define DEVICE "Sensor1"
#define TEMP_Max 27
#define TEMP_Min 14
#define HUM_Max 70
#define HUM_Min 50
#define TIME 60000

// WIFI
#define WIFI_SSID "Deichert_IOT"
#define WIFI_PASSWORD "23plag773imp9x"

// InfluxDB
#define INFLUXDB_URL "http://grafana.deichert.de:8086"
#define INFLUXDB_DB_NAME "klima"

// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);

// Create an object of the class Bsec
Bsec iaqSensor;

String output;

// Entry point for the example
void setup(void)
{
  Serial.begin(115200);
  Wire.begin();

  // WLAN Verbindung herstellen
  Serial.println("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.hostname(LOCATION);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("No Wifi");
    Serial.println('\n');
    delay(1000);
  }
  Serial.println();

  iaqSensor.begin(BME680_I2C_ADDR_PRIMARY, Wire);
  output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus();

  bsec_virtual_sensor_t sensorList[10] = {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };

  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();

  // Print the header
  output = "Timestamp [ms], raw temperature [°C], pressure [hPa], raw relative humidity [%], gas [Ohm], IAQ, IAQ accuracy, temperature [°C], relative humidity [%], Static IAQ, CO2 equivalent, breath VOC equivalent";
  Serial.println(output);
}

// Function that is looped forever
void loop(void)
{
  unsigned long time_trigger = millis();
  
  // Datenbank Verbindung herstellen
  InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB_NAME);

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  if (iaqSensor.run()) { // If new data is available
    output = String(time_trigger);
    output = "Sensor1, Time: " + String(time_trigger);
    output += " HPA: " + String(iaqSensor.pressure);
    output += " IAQ: " + String(iaqSensor.iaq);
    output += " IAQ ACC: " + String(iaqSensor.iaqAccuracy);
    output += " Static IAQ: " + String(iaqSensor.staticIaq);
    output += " CO2 Equi: " + String(iaqSensor.co2Equivalent);
    output += " Voc Equi: " + String(iaqSensor.breathVocEquivalent);
    output += " TEMP: " + String(iaqSensor.temperature);
    output += " HUM%: " + String(iaqSensor.humidity);
    Serial.println(output);
    Point pointDevice("Klima");
    // Set tags
    pointDevice.addTag("device", "ESP 0");
    pointDevice.addTag("sensor", "BME680 1");
    pointDevice.addTag("location", "Baumhaus");
    // Add data
    pointDevice.addField("HPA", iaqSensor.pressure);
    pointDevice.addField("IAQ", iaqSensor.iaq);
    pointDevice.addField("IAQ Accuracy", iaqSensor.staticIaq);
    pointDevice.addField("CO2 Equivalent", iaqSensor.co2Equivalent);
    pointDevice.addField("Voc Equivalent", iaqSensor.breathVocEquivalent);
    pointDevice.addField("Temperatur", iaqSensor.temperature);
    pointDevice.addField("Humidity", iaqSensor.humidity);
    // Write data
    client.writePoint(pointDevice);
  
  
  
  } else {
    checkIaqSensorStatus();
  }
}

// Helper function definitions
void checkIaqSensorStatus(void)
{
  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      output = "BSEC error code : " + String(iaqSensor.status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BSEC warning code : " + String(iaqSensor.status);
      Serial.println(output);
    }
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      output = "BME680 error code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BME680 warning code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
    }
  }
}

void errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}
