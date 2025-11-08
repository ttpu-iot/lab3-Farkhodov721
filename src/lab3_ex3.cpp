#include "Arduino.h"
#include "WiFi.h"
#include <ArduinoJson.h>
#include "PubSubClient.h"

// Pin definitions
#define LIGHT_PIN 33
#define BUTTON_PIN 25

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_broker = "mqtt.iotserver.uz";
const int mqtt_port = 1883;
const char* mqtt_username = "userTTPU";
const char* mqtt_password = "mqttpass";

const char* topic_light = "ttpu/iot/abdulaziz/sensors/light";
const char* topic_button = "ttpu/iot/abdulaziz/events/button";

unsigned long lastPublishTime = 0;
const long publishInterval = 5000;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

void connectWifi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqtt_client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    String client_id = "esp32-" + String(random(0xffff), HEX);
    if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker!");
      // Subscribe to LED topics here if needed for full ex3
      // mqtt_client.subscribe("ttpu/iot/abdulaziz/led/red");
      // mqtt_client.subscribe("ttpu/iot/abdulaziz/led/green");
      // mqtt_client.subscribe("ttpu/iot/abdulaziz/led/blue");
      // mqtt_client.subscribe("ttpu/iot/abdulaziz/led/yellow");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(LIGHT_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  connectWifi();
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  connectMQTT();
}

void loop() 
{
  // Button event publishing (debounced)
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != lastButtonState) {
      lastButtonState = reading;
      unsigned long timestamp = time(NULL);
      String eventType = (reading == HIGH) ? "PRESSED" : "RELEASED";
      String payload = "{\"event\": \"" + eventType + "\", \"timestamp\": " + String(timestamp) + "}";
      mqtt_client.publish(topic_button, payload.c_str());
    }
  }

  // Light sensor publishing every 5 seconds
  unsigned long currentTime = millis();
  if (currentTime - lastPublishTime >= publishInterval) {
    int lightValue = analogRead(LIGHT_PIN);
    unsigned long timestamp = time(NULL);
    String payload = "{\"light\": " + String(lightValue) + ", \"timestamp\": " + String(timestamp) + "}";
    mqtt_client.publish(topic_light, payload.c_str());
    lastPublishTime = currentTime;
  }

  // Connection checks
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  if (!mqtt_client.connected()) {
    connectMQTT();
  }
  mqtt_client.loop();
  delay(50);
}