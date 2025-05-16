#include <DHT.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include "DHTesp.h"

#define DHT_PIN 2  // Replace with the GPIO pin connected to the DHT22 sensor
#define LED_GREEN_PIN 21  // Replace with the GPIO pin connected to the green LED bulb
#define LED_RED_PIN 22  // Replace with the GPIO pin connected to the red LED bulb

char ssid[] = "Wokwi-GUEST";
char pass[] = "";
WiFiClient client;

unsigned long myChannelNumber = 2308799;
const char *myWriteAPIKey = "Y5D386LU3W5X66Y2";
int statusCode;
DHTesp dhtSensor;

int ledGreen = LED_GREEN_PIN;
int ledRed = LED_RED_PIN;

struct Data {
  float temperature;
  float humidity;
};

Data data;  // Declare a variable to store the data

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);
}

void connectToCloud() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect");
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
      for (int i = 0; i < 5; i++) {
        Serial.print(".");
        delay(1000);
      }
    }
    Serial.println("\nConnected.");
  }
}

void computeData() {
  TempAndHumidity sensorData = dhtSensor.getTempAndHumidity();
  data.temperature = sensorData.temperature;
  data.humidity = sensorData.humidity;
  Serial.println("-----------");
  Serial.println("Humi: " + String(data.humidity));
  Serial.println("Temp: " + String(data.temperature));
  Serial.println("-----------");
}

void writeData() {
  ThingSpeak.setField(1, data.humidity);
  ThingSpeak.setField(2, data.temperature);
  statusCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (statusCode == 200)
    Serial.println("Channel update successful.");
  else
    Serial.println("Problem Writing data. HTTP error code: " + String(statusCode));
  delay(15000);  // Data to be uploaded every 15 seconds
}

void loop() {
  connectToCloud();
  computeData();
  writeData();

  // Read temperature and humidity
  float temperature = dhtSensor.getTemperature();
  float humidity = dhtSensor.getHumidity();

  // Print the results
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  // Add a condition for air quality
  if (temperature > 25.0 && humidity > 70.0) {
    Serial.println("Air quality might be poor (high temperature and humidity).");
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);
  } else if (temperature < 20.0 && humidity < 30.0) {
    Serial.println("Air quality might be poor (low temperature and humidity).");
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);
  } else {
    Serial.println("Air quality appears to be normal.");
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);
  }

  delay(10000);  // Wait for some time before the next measurement (10 seconds)
}