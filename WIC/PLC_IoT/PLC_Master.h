#ifndef PLC_MASTER_
#define PLC_MASTER_
#include "config.h"
#ifdef PLC_MASTER_UI
#include <Arduino.h>
#include <ArduinoJson.h>

#ifdef DEBUG_FLAG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(x) Serial.printf(x)
#else
#define debug(x)
#define debugln(x)
#define debugf(x)
#endif
/////////////////////////////////////// Communication 
enum{LoRa,MESH,MQTT,RS485com};

class PLC_MASTER
{
public:
#define BootButton 0
#define LED_STATUS  25
#define BTN_SET     26

#define SW_1        27
#define SW_2        14 

#define IO1_HEADER  2.
#define IO2_HEADER  15

#define I2C_SDA     21
#define I2C_SCL     22

#define LED_ON(pin)    digitalWrite(pin, HIGH)
#define LED_OFF(pin)   digitalWrite(pin, LOW)

void SetLoRaValue();
void setup();
void loop();
};
extern PLC_MASTER PLC_master;
#endif//PLC_MASTER_UI
#endif//PLC_MASTER_