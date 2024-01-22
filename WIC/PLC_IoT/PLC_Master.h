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


#define LED_ON(pin)    digitalWrite(pin, HIGH)
#define LED_OFF(pin)   digitalWrite(pin, LOW)

void modbusSet(uint16_t addr, uint16_t value);
void GetIdList(int idlist[]);
void connectWeb(byte connected);
void SetLoRaValue();
void setup();
void loop();
};
extern PLC_MASTER PLC_master;
#endif//PLC_MASTER_UI
#endif//PLC_MASTER_