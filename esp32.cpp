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

#define TALKBACK_ID 54804
#define TALKBACK_API_KEY "215SMDC3F16JWRF3"

DHT dht(DHTPIN, DHT22); // Initialize DHT sensor on pin 5 with DHT22 type
int pirState = LOW;
float temp = 0.0;

// Initialize WiFi client for ThingSpeak
WiFiClient client;

// WiFi credentials
const char* wifi_name = "Wokwi-GUEST";
const char* password = "";


void setup() { // Initialize the DHT sensor, WiFi, and ThingSpeak
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(wifi_name, password);// Connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connection to WiFi successfuly established");

  ThingSpeak.begin(client);// Initialize ThingSpeak

  pinMode(PIRPIN, INPUT);
  pinMode(RELAY_LIGHT, OUTPUT);
  pinMode(RELAY_COOLER, OUTPUT);
  pinMode(RELAY_HEATER, OUTPUT);

  digitalWrite(RELAY_LIGHT, LOW);
  digitalWrite(RELAY_COOLER, LOW);
  digitalWrite(RELAY_HEATER, LOW);
}

String getTalkBackCommand() { // Function to get the command from TalkBack
  String url = "/talbacks/" + String(TALKBACK_ID) + "/commands/execute";
  String response = "";

  if (client.connect("api.thingspeak.com", 80)) { // Connect to the TalkBack API
    client.print(String("GET ") + url + "?api_key=" + TALKBACK_API_KEY + " HTTP/1.1\r\n" +
                 "Host: api.thingspeak.com\r\n" +
                 "Connection: close\r\n\r\n");

    bool headersEnded = false;

    while (client.connected() || client.available()) { // Read the response
      if (client.available()) { // Check if data is available
        String line = client.readStringUntil('\n'); // Read a line from the response
        line.trim();

        if (!headersEnded) { //
          if (line.length() == 0) {
            headersEnded = true; 
          }
        } else {
          response += line; // Append the line to the response
        }
      }
    }
    client.stop();
  }
  return response; // Return the response from the TalkBack API
}

String parseCommand(String rawResponse) { // Function to parse the command from the raw response
  int start = 0;
  int end = rawResponse.length() - 1;

  while (start < rawResponse.length() && isDigit(rawResponse.charAt(start))) {
    start++;
  }

  while (end >= 0 && isDigit(rawResponse.charAt(end))) {
    end--;
  }

  if (start > end) return "";
  return rawResponse.substring(start, end + 1);
}

void loop() {
  temp = dht.readTemperature();
  pirState = digitalRead(PIRPIN);

  if (pirState == HIGH) {
    digitalWrite(RELAY_LIGHT, HIGH);
    Serial.println("Motion Detected, Light ON");
    delay(20000); // Keep light ON for 20 seconds
    digitalWrite(RELAY_LIGHT, LOW);
  }

  if (temp > 25) { // Check if temperature is above 25 degrees Celsius
    digitalWrite(RELAY_COOLER, HIGH);
    digitalWrite(RELAY_HEATER, LOW);
    Serial.println("Temperature is too high from the set point, starting Cooler");
  } else if (temp < 18) { // Check if temperature is below 18 degrees Celsius
    digitalWrite(RELAY_HEATER, HIGH);
    digitalWrite(RELAY_COOLER, LOW);
    Serial.println("Temperature is too low from the set point, starting Heater");
  } else { //
    digitalWrite(RELAY_COOLER, LOW);
    digitalWrite(RELAY_HEATER, LOW);
    Serial.println("Temperature is within the set point, Cooler and Heater OFF");
  }

  String rawResponse = getTalkBackCommand();
  String command = parseCommand(rawResponse);

  Serial.println(command);

// Process the command received from TalkBack
  if (command == "TURN_ON") {
    digitalWrite(RELAY_LIGHT, HIGH);
    Serial.println("TalkBack: Light ON");
  } else if (command == "TURN_OFF") {
    digitalWrite(RELAY_LIGHT, LOW);
    Serial.println("TalkBack: Light OFF");
  }

  // Send data to ThingSpeak
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, pirState);
  ThingSpeak.setField(3, digitalRead(RELAY_COOLER));
  ThingSpeak.setField(4, digitalRead(RELAY_HEATER));
  ThingSpeak.setField(5, digitalRead(RELAY_LIGHT));
  
  ThingSpeak.writeFields(CHANNEL_ID, THINGSPEAK_API_KEY);
  
  delay(2000);
}
