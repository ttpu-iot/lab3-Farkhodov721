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


const int LIGHT_PIN = 33;
const int BUTTON_PIN = 25;


unsigned long lastPublishTime = 0;
const long publishInterval = 5000;




/*************************
 * SETUP
 */
void setup()
{

    Serial.begin(1152000);
    pinMode(BUTTON_PIN, INPUT);

}


/*************************
 * LOOP
 */
void loop() 
{

}

