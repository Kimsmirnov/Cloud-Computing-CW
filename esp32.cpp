#include <DHT.h>
#include <WiFi.h>
#include <ThingSpeak.h>

#define DHTPIN 5
#define PIRPIN 14
#define RELAY_LIGHT 12
#define RELAY_COOLER 13
#define RELAY_HEATER 27

#define CHANNEL_ID 2966869
#define THINGSPEAK_API_KEY "VLAH5SBSXCPJP8C8"

DHT dht(DHTPIN, DHT22);
int pirState = LOW;
float temp = 0.0;

WiFiClient client;

const char* wifi_name = "Wokwi-GUEST";
const char* password = "";


void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(wifi_name, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connection to WiFi successfuly established");

  ThingSpeak.begin(client);

  pinMode(PIRPIN, INPUT);
  pinMode(RELAY_LIGHT, OUTPUT);
  pinMode(RELAY_COOLER, OUTPUT);
  pinMode(RELAY_HEATER, OUTPUT);

  digitalWrite(RELAY_LIGHT, LOW);
  digitalWrite(RELAY_COOLER, LOW);
  digitalWrite(RELAY_HEATER, LOW);
}

void loop() {
  temp = dht.readTemperature();
  
  pirState = digitalRead(PIRPIN);

  if (pirState == HIGH) {
    digitalWrite(RELAY_LIGHT, HIGH);
    Serial.println("Motion Detected, Light ON");
  } else {
    digitalWrite(RELAY_LIGHT, LOW);
    Serial.println("No Motion Detected, Light OFF");
  }

  if (temp > 25) {
    digitalWrite(RELAY_COOLER, HIGH);
    digitalWrite(RELAY_HEATER, LOW);
    Serial.println("Temperature is too high from the set point, starting Cooler");
  } else if (temp < 18) {
    digitalWrite(RELAY_HEATER, HIGH);
    digitalWrite(RELAY_COOLER, LOW);
    Serial.println("Temperature is too low from the set point, starting Heater");
  } else {
    digitalWrite(RELAY_COOLER, LOW);
    digitalWrite(RELAY_HEATER, LOW);
    Serial.println("Temperature is within the set point, Cooler and Heater OFF");
  }

  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, pirState);
  ThingSpeak.setField(3, digitalRead(RELAY_COOLER));
  ThingSpeak.setField(4, digitalRead(RELAY_HEATER));
  ThingSpeak.setField(5, digitalRead(RELAY_LIGHT));
  
  ThingSpeak.writeFields(CHANNEL_ID, THINGSPEAK_API_KEY);
  
  delay(2000);
}
