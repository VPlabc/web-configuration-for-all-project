#ifndef main_iotdevice
#define main_iotdevice
#include "config.h"
#ifdef IOTDEVICE_UI
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

#define MESHWIFI 0
#define MQTT     1
#define RS485    2

class IoT_Device
{
public:
const int LEDButton = 2;

byte LEDType = 0;
//---------------------------------------------- OLED

byte RunMode = 0;
byte Debug = 0;
byte ROLE = 0;
byte COMMODE = 0;
byte current_status = 0;      // 0=offline , 1=online    ,2=OTA
void setup();
void loop();
void UpdateStatus();
void Command(String Cmd, String netID, String ID, String CAT, String SleepTime);
void WifiMode();
void colorWipe(uint32_t color, int wait);
void theaterChase(uint32_t color, int wait);
void rainbow(int wait);
void theaterChaseRainbow(int wait);
void larson_scan();
void setStrip(int r);
void ledFadeToBeat(int R,int G,int B,int bright);
void LED_Signal(int replay, int speed);
void updateDisplay();
void OLED_Display(String msg,byte line);
void OLED_Clear();
void TaskServer();
void Mesh_Init();
void getConfig();
void MeshBegin();
void NodeDataUpdate();
void NodeCategoryInit(byte category);
void NodeCategoryRead(byte category);
int check_protocol();
byte getRunMode();
byte getDebug() ;
void sendDataNode();
void sendCurrentStatus(byte status);
//----------------------------------------------------------------


private:

};
extern IoT_Device IOT_DEVICE;
#endif//#ifdef IOTDEVICE_UI
#endif//main_h