// - RED LED - `D26`
// - Green LED - `D27`
// - Blue LED - `D14`
// - Yellow LED - `D12`

// - Button (Active high) - `D25`
// - Light sensor (analog) - `D33`

// - LCD I2C - SDA: `D21`
// - LCD I2C - SCL: `D22`





#include "Arduino.h"
#include "WiFi.h"
#include <ArduinoJson.h>
#include "PubSubClient.h"


// Your code here - global declarations

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_broker = "mqtt.iotserver.uz";
const int mqtt_port = 1883;
const char* mqtt_username = "userTTPU";
const char* mqtt_password = "mqttpass";
WiFiClient espClient;
PubSubClient mqtt_client(espClient);


const int RED_LED_PIN = 26;
const int GREEN_LED_PIN = 27;
const int BLUE_LED_PIN = 14;
const int YELLOW_LED_PIN = 12;

/*************************
 * SETUP
 */
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

void connectWifi() {
    Serial.println("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
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
        Serial.println("JSON parse error!");
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
        Serial.println("Connecting to MQTT broker...");
        String client_id = "esp32-" + String(random(0xffff), HEX);
        if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Connected to MQTT broker!");
            mqtt_client.subscribe("ttpu/iot/abdulaziz/led/red");
            mqtt_client.subscribe("ttpu/iot/abdulaziz/led/green");
            mqtt_client.subscribe("ttpu/iot/abdulaziz/led/blue");
            mqtt_client.subscribe("ttpu/iot/abdulaziz/led/yellow");
        } else {
            Serial.print("MQTT connect failed, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" retrying in 2s");
            delay(2000);
        }
    }
}

/*************************
 * LOOP
 */
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