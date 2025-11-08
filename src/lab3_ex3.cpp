#include "Arduino.h"
#include "WiFi.h"
#include <ArduinoJson.h>
#include "PubSubClient.h"
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// Pin definitions
#define LIGHT_PIN 33
#define BUTTON_PIN 25
#define RED_LED_PIN 26
#define GREEN_LED_PIN 27
#define BLUE_LED_PIN 14
#define YELLOW_LED_PIN 12

// LCD object
hd44780_I2Cexp lcd;
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

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
      mqtt_client.subscribe("ttpu/iot/abdulaziz/led/red");
      mqtt_client.subscribe("ttpu/iot/abdulaziz/led/green");
      mqtt_client.subscribe("ttpu/iot/abdulaziz/led/blue");
      mqtt_client.subscribe("ttpu/iot/abdulaziz/led/yellow");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }
  String state = doc["state"];
  unsigned long timestamp = time(NULL);

  // Display on Serial Monitor
  Serial.print("Received on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(message);

  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Topic:");
  lcd.print(String(topic).substring(20)); // Display only the LED color
  lcd.setCursor(0, 1);
  lcd.print("State:");
  lcd.print(state);

  // Control LEDs
  if (String(topic).endsWith("red")) digitalWrite(RED_LED_PIN, state == "ON" ? HIGH : LOW);
  if (String(topic).endsWith("green")) digitalWrite(GREEN_LED_PIN, state == "ON" ? HIGH : LOW);
  if (String(topic).endsWith("blue")) digitalWrite(BLUE_LED_PIN, state == "ON" ? HIGH : LOW);
  if (String(topic).endsWith("yellow")) digitalWrite(YELLOW_LED_PIN, state == "ON" ? HIGH : LOW);
}

void setup()
{
  Serial.begin(115200);
  pinMode(LIGHT_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);

  // Initialize LCD
  if (lcd.begin(LCD_COLS, LCD_ROWS)) {
    Serial.println("LCD initialization failed!");
    while (1);
  }
  lcd.print("MQTT Ready!");
  delay(2000);
  lcd.clear();

  connectWifi();
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setCallback(mqttCallback);
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
  delay(10);
}