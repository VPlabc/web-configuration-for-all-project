#ifndef Main
#define Main
#ifdef Valve_UI
/*
lib_deps =
  # RECOMMENDED
  # Accept new functionality in a backwards compatible manner and patches
  knolleary/PubSubClient @ ^2.8
  adafruit/Adafruit GFX Library @ ^1.10.12
  adafruit/Adafruit SSD1306 @ ^2.5.0
  bblanchon/ArduinoJson @ ^6.18.5    
  adafruit/Adafruit BusIO @ ^1.9.9  
*/
#include "Arduino.h"
#define   DEBUG_FLAG
#define   TIMER_INTER
#define ENABLE_RSSI
//#define USE_OLED
#ifdef DEBUG_FLAG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(x) Serial.printf(x)
#else
#define debug(x)
#define debugln(x)
#define debugf(x)
#endif

#define   PRGM_VERSION         "2.0.3"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "iconset.h"
//#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
//#include <ESP8266HTTPUpdate.h>
#endif//ESP8266
 #ifdef ESP32
// #include <WiFi.h>
// #include <WebServer.h>
// #include <EEPROM.h>
// #include <PubSubClient.h>
// #include <WiFiUdp.h>
// #include <NTPClient.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
//#include "ValveGateway/iconset.h"
//#include <ArduinoOTA.h>
// #include <ESPmDNS.h>
// #include <ArduinoJson.h>
// #include <WiFiClient.h>
// #include <Update.h>
// #include <HTTPUpdate.h>
//#include <HTTPClient.h>
//#include <AsyncTCP.h>
//#include <ESPAsyncWebServer.h>
// #include <FS.h>
//#include <LITTLEFS.h>
#endif//ESP32

//#include <ESP8266UPDATE>
//#include "ValveGateway/cert.h"


// 
//bool startup = true;
// long timeMQTT = 0;
// int32_t timeLora = 0;
// int32_t timeLora1 = 0;
//bool synchroni
extern void TimeSetup();

typedef struct {
    byte byte1;
    byte byte2;
}RespondNodeData;

  #ifdef ESP8266
  #include <SoftwareSerial.h>
  
  SoftwareSerial LoraSer(D4, D7,false);
      #define LoRa_Ser LoraSer   
  #endif//ESP8266
  #ifdef ESP32    
      #define LoRa_Ser Serial2 
  #endif//ESP32   
#define  Debug_Ser Serial
 
/* ------------------------ SENSORS DATA --------------------------------------- */

#define MESH_ID               6734922
#define GROUP_SWITCH          1
#define GROUP_HT              2
#define GROUP_MOTION          3
#define battery_cutoff_volt   2.9
////////////////// RF Type
#define LoRa      0
#define MQTT      1
#define MESH      2
#define RS485com  3
///////////////////////////
////////////////// Board Type
#define ModeGateway   0
#define ModeNode      1
#define LooklineV130  2
#define LooklineV140  3
#define LooklineV141  4
////////////////// Command
#define FeedbackCmd 0 //request feedback
#define SleepCmd    1 //request Sleep
#define OpenCmd     1 //Open
#define CloseCmd    0 //Close

#define         NUM_SENSORS 20

typedef struct sensor_data {
  int     mesh_id;
  uint8_t sensor_id;
  byte    category;
  byte    status;
  byte    RSSI;
  bool    RSSIenable;
  float   temperature;
  float   humidity;
  float   battery;
  float   battery12;
  time_t  timestamp;
  time_t  Lasttimestamp;
  time_t cycleTime;
  uint32_t Nodecounter;
  uint32_t NodeTimeOut;
  byte     NodeComfirm;
  byte     GateWayCommand;
  byte     GateWayControl;
  byte     NodeRun;//0: idle,1:begin,2:end,3:come back

} sensor_data;
typedef struct struct_message {
  int     mesh_id;
  uint8_t sensor_id[6];
  byte    category;
  byte    status ;
  byte    RSSI;
  float   temperature;
  float   humidity;
  float   battery;
  float   battery12;
} struct_message;

  extern void handleRoot();
  extern void handleRaw();
  extern void handleMqtt();
  extern void handleRawFile();
  extern void handleNames();
  extern void handleJson();
  extern void handleInfo();
  extern void handleDeleteSensor();
  extern void handleDeleteFile();
  extern void handleROTA();
  extern void handleRetrySD();
  extern void handleReboot();
  extern void handleControl();
  extern void SDhandleRoot1();
  extern void SDhandleRoot();
  extern void handleNotFound();
  extern void handleStatus();
  extern void handleScan1();
  extern void handleScan();
  extern void handleStopScan();
  extern void showIdle(); 
  
  extern void mqttReconnect();
  extern byte NodeCycleRequest[NUM_SENSORS];
  extern int             sensors_saved;
  extern uint8_t         incomingData[];
  extern size_t          received_msg_length;
  extern bool            new_sensor_found;
  extern uint8_t TimeSent;
  extern bool Gateway;
  extern bool mqtt_connected;
  extern unsigned long     last_activity_timestamp;
  extern unsigned long     last_Sent_timestamp;
  extern unsigned long     last_Sent_timestamp1;
  extern struct_message  msg;
  extern sensor_data     sensors[NUM_SENSORS];
class Valve
{
public:
#define alert_pin  25
  ////// LED
  static uint8_t Signal_LED;
  static uint8_t Startus_LED;
  static uint8_t M0;
  static uint8_t M1;
  static uint8_t AUX;
  static uint8_t LoRaPower;
  ///////////////////////////// Map Pin ///////////////////////// 
  static uint8_t MapPin[10];
  static bool  ap_mode;
  static bool UploadProcess;
  static bool sending;
  static unsigned long next_receiver_ping_timestamp;
  static byte          receiver_status;   
  static bool update;







///////////////////////////


/* --------------------------- Sensors ---------------------------------------------- */






  
/* --------------------------- OLED ---------------------------------------------- */

#define           SCREEN_WIDTH      128
#define           SCREEN_HEIGHT     64
#define           OLED_RESET        -1
#define           SCREEN_ADDRESS    0x3C
static bool              oled_failed ;



//////////////////////////////////////////////////////////
//  #ifdef __cplusplus
//   extern "C" {
//  #endif
//   uint8_t temprature_sens_read();

// #ifdef __cplusplus
// }
// #endif


   
// //ADC_MODE(ADC_VCC); //vcc read
// uint8_t temprature_sens_read();
  static bool getOpened();
  static unsigned int EncodeRespond(byte bytel,byte byteh);
  static byte EncodeRespondByte(boolean a, boolean b, boolean c, boolean d, boolean e, boolean f, boolean g, boolean h);
  static void showInfo(String title, String msg, int idle_timeout);
  // static void showMsg(String title, String mac, String status, String battery, String battery1,byte RSSI, int idle_timeout);
  // static void alert();
  // static void showLogo();
  //static void showIdle();
  
  static void showFirmwareProgress(int progress);
  static void pingReceiver();

  static void HandleNode(int i);
 

  static void sensorMessageReceived(byte RSSI,byte IDh,byte IDl,byte State,byte BatH,byte BatL,byte Bat12H,byte Bat12L);
  static void saveSensorData(byte RSSI,byte IDh,byte IDl,byte State,byte BatH,byte BatL,byte Bat12H,byte Bat12L);
  static float getUserUnitTemperature(float t);
  static void mqttPublish(char macAdress[], String payload,  size_t len );
  static void checkTimer();
  //static void WebFunction();
  String uptime();
  
  static void ReadLoRData();
  static bool loadFromSDCARD(String path);
  static void SDhandleRoot();
  static void Learning(byte ID);
  static void SendRequest(long DELAY);
  static void MDelay(long t);
  static void webFunction();
  ///////////////////////////
  static void valve_setup();
  static void valve_loop();
  Valve();
  /////////////////////////////////////////////////////////////////////////////////////////////////////////
private:



};
#endif//Valve_UI
#endif//Main