#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // Include ArduinoJson library for parsing JSON

const char* ssid = "DON";
const char* password = "chaganoma";
const char* apiKey = "e1171a938db04949b4d195117240211";

String location = "Belagavi";  // Default location

// Define GPIO pins for LEDs
const int LED_PIN_COLD = 2;  // LED for temperature below 15°C, connected to D2
const int LED_PIN_WARM = 4;  // LED for temperature above 15°C, connected to D4

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();

  // Attempt to connect to Wi-Fi within 10 seconds
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to Wi-Fi");
  } else {
    Serial.println("\nFailed to connect to Wi-Fi. Rebooting...");
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);
  connectToWiFi();

  // Initialize LED pins
  pinMode(LED_PIN_COLD, OUTPUT);
  pinMode(LED_PIN_WARM, OUTPUT);

  // Ensure LEDs are off at start
  digitalWrite(LED_PIN_COLD, LOW);
  digitalWrite(LED_PIN_WARM, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    // Read the city name from the serial input
    location = Serial.readStringUntil('\n');
    location.trim();  // Remove any trailing newline characters

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = "http://api.weatherapi.com/v1/current.json?key=" + String(apiKey) + "&q=" + location;

      Serial.println("Requesting data from: " + url);
      http.begin(url.c_str());

      int httpResponseCode = http.GET();

      if (httpResponseCode == 200) {
        String payload = http.getString();
        Serial.println(payload);  // Print the full JSON response

        // Parse the JSON to get temperature
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
          Serial.print("JSON parsing failed: ");
          Serial.println(error.c_str());
        } else {
          // Get the temperature in Celsius from the JSON response
          float temperature = doc["current"]["temp_c"];
          Serial.print("Temperature in ");
          Serial.print(location);
          Serial.print(": ");
          Serial.print(temperature);
          Serial.println(" °C");

          // Turn on appropriate LED based on temperature
          if (temperature < 15) {
            digitalWrite(LED_PIN_COLD, HIGH);  // Turn on cold LED
            digitalWrite(LED_PIN_WARM, LOW);   // Ensure warm LED is off
          } else {
            digitalWrite(LED_PIN_COLD, LOW);   // Ensure cold LED is off
            digitalWrite(LED_PIN_WARM, HIGH);  // Turn on warm LED
          }
        }
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);

        if (httpResponseCode == -1) {
          Serial.println("Check your network connection or API URL.");
        } else if (httpResponseCode == 403) {
          Serial.println("Invalid API key or access denied.");
        } else if (httpResponseCode == 404) {
          Serial.println("Requested data not found. Check location or API endpoint.");
        }
      }

      http.end();
    } else {
      Serial.println("Wi-Fi Disconnected. Reconnecting...");
      connectToWiFi();
    }
  }
  delay(1000);  // Small delay to prevent spamming requests
}
