#ifndef MQTTCOM_
#define MQTTCOM_
#include "config.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#ifdef MQTT_USE
class MQTTCOM
{
public:
    bool mqtt_connected = false;
    static void SetDataACK(byte d);
    static byte GetDataACK();
    void setup();
    void loop();
    void sensorMessageReceived(int category,int message, int status, float temperature, float humidity, float battery);
    void mqttReconnect();
    
    void mqttPublish(String payload ,String Topic ) ;
    bool connect_state();
};

extern MQTTCOM mqttcom;

#endif//MQTT_USE
#endif//MQTTCOM