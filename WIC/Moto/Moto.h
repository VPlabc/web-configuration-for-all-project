#ifndef Moto_
#define Moto_
#include "config.h"
#ifdef Moto_UI
#include <Arduino.h>

//#define MOTO_DEBUG

// #ifdef DEBUG_FLAG_
// #define debug(x) Serial.print(x)
// #define debugln(x) Serial.println(x)
// #define debugf(x) Serial.printf(x)
// #else
// #define debug(x)
// #define debugln(x)
// #define debugf(x)
// #endif

#define GearN 33
#define Gear1 32
#define Gear2 35
#define Gear3 34
#define Gear4 39
#define Gear5 36
#define Light 4
const int Button = 15;
#define SensorPin1 25
#define SensorPin2 26
#define VoltSensorPin 14

class Moto
{
public:
const int LEDButton = 2;
byte hours;
byte mins;
byte secs;
byte Days;
byte Months;
int16_t Years;
bool wifi_b = false;

bool WiFi_on;
void setup();
void loop();
byte getMode();
void readtime();
String GearUpdate();
void SetTime(uint8_t ss,uint8_t mm, uint8_t hh, uint8_t dd,uint8_t mo,int16_t year);
void GetTime();
void updateTime();
float VoltUpdate();
private:

};
extern Moto MOTO;
#endif//Moto_UI
#endif//Moto_