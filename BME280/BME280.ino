#include "Adafruit_Sensor.h"
#include <ESP8266WiFi.h>  
#include <Adafruit_BME280.h>
#include <InfluxDb.h>
Adafruit_BME280 bme; // I2C

// Sensorwerte
#define LOCATION "Garage"
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

void setup() {

  //Status der COM-Ports überprüfen
  Serial.begin(9600);
  while (!Serial) {
    delay(10); // hang out until serial port opens
  }  
  pinMode(D5, OUTPUT);

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

  //Verbindung zum Sensor sicherstellen
  Serial.println(F("BME280 test"));
  bool status;
  status = bme.begin(0x76);

  Serial.println(status);
   
  if (!status) {
     Serial.println("Could not find a valid BME280 sensor, check wiring!");
      while (1);
  }
  
}
  
void loop() { 
  printValues();
  delay(TIME);
}

void printValues() {
  
  // Messwerte definieren 
  float temperature = bme.readTemperature();
  float pressure = bme.readPressure();
  float humidity = bme.readHumidity();

  // Ausgabe Messwerte
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" *C");
  
  Serial.print("Pressure = ");
  Serial.print(pressure / 100.0F);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.println(" %");

//////////////////////////////////////////////////////////////////////////////

  // IF/Else Logik zur aktiven Steuerung
  if (temperature >= TEMP_Max){
    Serial.print("Temperatur zu hoch - Abluft EIN\n");
    digitalWrite(D5, HIGH);}    
  else if (humidity >= HUM_Max){
    Serial.print("Luftfeuchtigkeit zu hoch - Abluft EIN\n");
    digitalWrite(D5, HIGH);}  
  else{
    Serial.print("Werte im Normbereich\n");
    digitalWrite(D5, LOW);}

//////////////////////////////////////////////////////////////////////////////
    
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
    
  //Send Data to InfluxDB
  Point pointDevice(INFLUXDB_DB_NAME);
  pointDevice.addTag("location", LOCATION);
  pointDevice.addTag("device", DEVICE);
  pointDevice.addField("temperature", temperature);
  pointDevice.addField("humidity", humidity);
  pointDevice.addField("pressure", pressure);
  client.writePoint(pointDevice);
}
