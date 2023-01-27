#ifndef main_autoit
#define main_autoit
#include "config.h"
#ifdef AUTOITGW_UI
#include <Adafruit_NeoPixel.h>
//#include <EEPROM.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <Wire.h>

#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include <DNSServer.h>
#include <espnow.h>
#define RF_Serial Serial
#define Master
#else
#include "esp_wifi.h"
#include <esp_now.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <DNSServer.h>
//---------------------------------------------- Update fw
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#ifdef ARDUINO_ARCH_ESP8266
#else
#define RF_Serial Serial2
#endif//
#endif//#ifdef ARDUINO_ARCH_ESP8266
// #include "cert.h"


#define DW(byte,bool) {digitalWrite(byte, bool)}
#define DR(byte) {digitalRead(byte)}
#define RA(byte) {analogRead(byte)}


// OLED 1.3' OLED_MODE = 1 , OLED 0.96' OLED_MODE = 0
#define OLED_096 0
#define OLED_130 1
//Gateway ROLE = 1  ,Node ROLE = 0 
#define Node 0
#define Gateway 1
#define NodeGateway 2
//Debug
#define NDEBUG 0
#define DEBUG 1
//Wifi Mode
#define MESHSLAVE 0
#define WIFIMODE 1
#define MESHMODE 2
//ButtonUpdateFw
#define USE 1

class Auto_Device
{
public:
const int LEDButton = 2;

byte LEDType = 0;
//---------------------------------------------- OLED

byte RunMode = 0;
byte Debug = 0;
byte ROLE = 0;
void setup();
void loop();
void UpdateStatus();
void Control(String netID,String ID,String State);
void Setting(String netID, String ID, String CAT, String SleepTime);
void getConfig();
void LED_Signal(int replay, int speed);
void updateDisplay();
void OLED_Display(String msg,byte line);
void OLED_Clear();
byte getRunMode();
byte getDebug() ;
//----------------------------------------------------------------


private:

};
extern Auto_Device AUTOIT_DEVICE;
#endif//#ifdef AUTOITGW_UI
#endif//main_h