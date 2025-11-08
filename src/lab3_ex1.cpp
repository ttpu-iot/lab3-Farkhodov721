// - RED LED - `D26`
// - Green LED - `D27`
// - Blue LED - `D14`
// - Yellow LED - `D12`

// - Button (Active high) - `D25`
// - Light sensor (analog) - `D33`

// - LCD I2C - SDA: `D21`
// - LCD I2C - SCL: `D22`

/**************************************
 * LAB 3 - EXERCISE 1: 
 * 
 * I want to publish message to mqtt every 5 second
 **************************************/

#include "Arduino.h"
#include "WiFi.h"
#include "PubSubClient.h"


// Your code here - global declarations


const char* ssid = "Wokwi-GUEST";
const char* password = "";


const char* mqtt_broker  = "mqtt.oitserver.uz";
const int mqtt_port = 1883;
const char* mqtt_username = "userTTPU";
const char* mqtt_password = "mqttpass";


const char* topic_light = "ttpu/iot/abdulaziz/sensors/light";
const char* topic_button = "ttpu/iot/abdulaziz/events/button";


const int LIGHT_PIN = 33;
const int BUTTON_PIN = 25;


unsigned long lastPublishTime = 0;
const long publishInterval = 5000;

int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

WiFiClient espClient;  
PubSubClient mqtt_client(espClient);

void connectWifi(){
    Serial.println("\nI'm about to connect to Wifi so,  wait ...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);  
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");  
        attempts++;
    }


    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nHere we are! Connected to WiFi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nLOL, WiFi disconnected. Let me try again ...");
    }
}


void connectMQTT(){
    while(!mqtt_client.connected()){
        Serial.println("CI'm about to connect to MQTT so, wait ...");

        String client_id ="esp32-" + String(random(0xffff), HEX);

        if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password )) {
            Serial.println("Here we are! Connected to MQTT Broker!");
        } else {
            Serial.print("LOL, MQTT Broker disconnected. Let me fix it ..., rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 2 seconds");
            delay(2000);
        }
    }
}

/*************************
 * SETUP
 */
void setup()
{
    Serial.begin(115200);
    delay(1000);
    pinMode(BUTTON_PIN, INPUT);

    Serial.println("I'm about to connect to Wifi so,  wait ....");
    connectWifi();

    Serial.println("I'm setting up MQTT so, wait ....");
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    connectMQTT();

}


/*************************
 * LOOP
 */
void loop() 
{
    int reading = digitalRead(BUTTON_PIN);
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }


    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != lastButtonState) {
            lastButtonState = reading;
            unsigned long timestamp = time(NULL);

            String eventType = (reading == HIGH) ? "pressed" : "released";
            String payload = "{ \"event\": \"" + eventType + "\", \"timestamp\": " + String(timestamp) + " }";

            mqtt_client.publish(topic_button, payload.c_str());
        }
    }


    unsigned long currentTime = millis();
    if (currentTime - lastPublishTime >= publishInterval) {
        int lightValue = analogRead(LIGHT_PIN);
        unsigned long timestamp = time(NULL);

        String payload = "{ \"light\": " + String(lightValue) + ", \"timestamp\": " + String(timestamp) + " }";
        mqtt_client.publish(topic_light, payload.c_str());

        lastPublishTime = currentTime;
   }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("LOL, WiFi disconnected. Let me try again ...");
        connectWifi();
    }
    delay(50);  


    if (!mqtt_client.connected()) {
        Serial.println("LOL, MQTT disconnected. Let me fix it ...");
        connectMQTT();
    }
    mqtt_client.loop();

    delay(50);

}

