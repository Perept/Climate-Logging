#include <InfluxDb.h>
#include <ESP8266WiFi.h>  
#define SensorPin A0 

// Sensorwerte
#define LOCATION "Pflanze1"
#define DEVICE "Sensor1"
#define TIME 60000

// WIFI
#define WIFI_SSID "Deichert_IOT"
#define WIFI_PASSWORD "PW"

// InfluxDB
#define INFLUXDB_URL "http://grafana.deichert.de:8086"
#define INFLUXDB_DB_NAME "klima"

void setup() {

  //Status der COM-Ports überprüfen
  Serial.begin(9600);
  while (!Serial) {
    delay(10); // hang out until serial port opens
  }  

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
  
}
  
void loop() { 
  printValues();
  delay(TIME);
}

void printValues() {
  
  // Messwerte definieren
  float Bodenfeuchte = analogRead(SensorPin); 

  // Ausgabe Messwerte
  Serial.print("Bodenfeuchte = ");
  Serial.println(Bodenfeuchte);
  
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
  pointDevice.addField("bodenfeuchte", Bodenfeuchte);
  client.writePoint(pointDevice);
}
