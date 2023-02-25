#ifndef MESHCOM_
#define MESHCOM_
#ifdef MESHCOM_UI
#include <Arduino.h>
//#include <ArduinoOTA.h>


#ifdef DEBUG_FLAG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(x) Serial.printf(x)
#else
#define debug(x)
#define debugln(x)
#define debugf(x)
#endif

class Mesh_Com
{
public:
bool WiFi_on;
void setup();
void loop();
byte getMode();
private:

};
extern Mesh_Com Mesh_config;
#endif//MESHCOM_UI
#endif//MESHCOM_