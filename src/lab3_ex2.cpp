#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Pin definitions
#define RED_LED_PIN     26
#define GREEN_LED_PIN   27
#define BLUE_LED_PIN    14
#define YELLOW_LED_PIN  12

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_broker = "mqtt.iotserver.uz";
const int mqtt_port = 1883;
const char* mqtt_username = "userTTPU";
const char* mqtt_password = "mqttpass";

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

void connectWifi() {
    Serial.println("I'm about to connect to Wifi so,  wait ....");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n  Here we are! Connected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.print("Received on topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    Serial.println(message);

    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return;
    }
    String state = doc["state"];
    if (String(topic).endsWith("red")) {
        digitalWrite(RED_LED_PIN, state == "ON" ? HIGH : LOW);
        Serial.print("Red LED -> "); Serial.println(state);
    } else if (String(topic).endsWith("green")) {
        digitalWrite(GREEN_LED_PIN, state == "ON" ? HIGH : LOW);
        Serial.print("Green LED -> "); Serial.println(state);
    } else if (String(topic).endsWith("blue")) {
        digitalWrite(BLUE_LED_PIN, state == "ON" ? HIGH : LOW);
        Serial.print("Blue LED -> "); Serial.println(state);
    } else if (String(topic).endsWith("yellow")) {
        digitalWrite(YELLOW_LED_PIN, state == "ON" ? HIGH : LOW);
        Serial.print("Yellow LED -> "); Serial.println(state);
    }
}

void connectMQTT() {
    while (!mqtt_client.connected()) {
        Serial.println("CI'm about to connect to MQTT so, wait ...");
        String client_id = "esp32-" + String(random(0xffff), HEX);
        if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Here we are! Connected to MQTT Broker!");
            mqtt_client.subscribe("ttpu/iot/abdulaziz/led/red");
            mqtt_client.subscribe("ttpu/iot/abdulaziz/led/green");
            mqtt_client.subscribe("ttpu/iot/abdulaziz/led/blue");
            mqtt_client.subscribe("ttpu/iot/abdulaziz/led/yellow");
        } else {
            Serial.print("LOL, MQTT Broker disconnected. Let me fix it ..., rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 2 seconds");
            delay(2000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    connectWifi();
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setCallback(mqttCallback);
    connectMQTT();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        connectWifi();
    }
    if (!mqtt_client.connected()) {
        connectMQTT();
    }
    mqtt_client.loop();
    delay(50);
}