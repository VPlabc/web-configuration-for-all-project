#include "config.h"
//#define USE_MQTT
/*  ============================================================================

     THE GATEWAY

  ============================================================================= */
#ifdef Valve_UI
#include "WIC.h"
#include "Valve/Valve.h"
#include "SDFunction.h"
#ifdef USE_LoRa
#include "LoRaFunc.h"
#endif//USE_LoRa
#include "webinterface.h"
LoraFunc LoraFunct;
WiFiClient            wifiClient;
#include <PubSubClient.h>
/* --------------------------- MQTT ---------------------------------------------- */
PubSubClient  mqttClient(wifiClient);
/* --------------------------- MQTT ---------------------------------------------- */
#define       MQTT_MSG_SIZE    200
char          mqttTopic[MQTT_MSG_SIZE];
#define       MSG_BUFFER_SIZE  (50)
unsigned long next_mqtt_connection_attempt_timestamp = 0;
String        thingName;
const char*   willTopic         = "LWT";
const char*   willMessage       = "offline";
boolean       willRetain        = false;
byte          willQoS           = 0;
bool          mqtt_connected = false;
#ifdef USE_LoRa
#include "LoRa_E22.h"
#endif//USE_LoRa
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

#ifdef USE_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "iconset.h"
Adafruit_SSD1306  display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif//USE_OLED
#include "esp_oled.h"
#include "iconset.h"

#include "Valve/LoRaFunc.h"
#include "SDFunction.h"
//#include "Function/WifiFunction.h"

SDFunction SDFunc;




void handleRoot();
void handleControl();
void handleStatus();
void handleRawFile();
void handleNames();
void handleDeleteFile();
void handleDeleteSensor();
void handleRetrySD();
void handleReboot();
void handleScan1();
void handleScan();
void handleROTA();    

void handleInfo();
//void Valve::showInfo(String title, String msg, int idle_timeout);
void showMsg(String title, String mac, String status, String battery, String battery1,String RSSI, int idle_timeout);
void showFirmwareProgress(int progress);

void showLogo();
void mqttPublish(char macAdress[], String payload,  size_t len );
void SDhandleRoot1();
void SDhandleRoot();
bool loadFromSDCARD(String path);
//String printDirectory(File dir, int numTabs);
void handleMqtt();
void handleStopScan();
void WebFunction();
const char* web_serverIndex = 
"<!DOCTYPE html><html lang='en'><head><meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=no'/>"
 " <title>Update firmware</title>"
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<meta content=\"text/html;charset=utf-8\" http-equiv=\"Content-Type\">"
  "<meta content=\"utf-8\" http-equiv=\"encoding\">"
  "<link rel=\"stylesheet\" href=\"bootstrap.css\">"
  "<link rel=\"stylesheet\" href=\"main.css\">"
  "</head>"
  "<body>"
  "<div id=\"top_bar\" style='position:absolute;top:0px;height:5px;width:100%;background-color:#F7C642;'></div>"
  "<div class=\"container py-3\">"
  "<header>"
    "<div class=\"d-flex flex-column flex-md-row align-items-center pb-3 mb-4 border-bottom\">"
    "<a href=\"/\" class=\"d-flex align-items-center text-dark text-decoration-none\"><span class=\"fs-4 logo\"></span></a><span class=\"fs-4\"  onclick=\"loadSensors();return false;\">Update Firmware</span><nav class=\"d-inline-flex mt-2 mt-md-0 ms-md-auto\"><span class=\"me-3 py-2 me-4 text-danger\" id=\"rota_status\"></span><span class=\"me-3 py-2 text-dark text-decoration-none d-none\"  style=\"cursor:pointer;\" onclick=\"recheckSDCard();return false;\" id=\"sdcard_status\"><svg viewBox=\"0 0 24 24\" style=\"width:16px;height:16px;color: #df1919;\"><path fill=\"currentColor\" d=\"M8,2A2,2 0 0,0 6,4V11L4,13V20A2,2 0 0,0 6,22H18A2,2 0 0,0 20,20V4A2,2 0 0,0 18,2H8M9,4H11V8H9V4M12,4H14V8H12V4M15,4H17V8H15V4Z\"></path></svg></span>"
    "<a class='me-3 py-2 text-dark text-decoration-none' href='/'><svg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' class='bi bi-house' viewBox='0 0 16 16'><path fill-rule='evenodd' d='M2 13.5V7h1v6.5a.5.5 0 0 0 .5.5h9a.5.5 0 0 0 .5-.5V7h1v6.5a1.5 1.5 0 0 1-1.5 1.5h-9A1.5 1.5 0 0 1 2 13.5zm11-11V6l-2-2V2.5a.5.5 0 0 1 .5-.5h1a.5.5 0 0 1 .5.5z'/><path fill-rule='evenodd' d='M7.293 1.5a1 1 0 0 1 1.414 0l6.647 6.646a.5.5 0 0 1-.708.708L8 2.207 1.354 8.854a.5.5 0 1 1-.708-.708L7.293 1.5z'/></svg></a>"
   "   </nav>"
  "  </div>"
 " </header>  "
"<body>  "
  "<main>      "
      "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
      "  <input type='file' name='update'>"
     "         <input type='submit' value='Update'>"
    "      </form>"
   "   <div id='prg'>progress: 0%</div>"
  "</main>"
"</body>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";


#ifdef DEBUG_FLAG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(x) Serial.printf(x)
#else
#define debug(x)
#define debugln(x)
#define debugf(x)
#endif
struct settings {
  char pversion[8];
  char ssid[30];
  char password[30];
  char mqttweb_server[60];
  char mqttUserName[32];
  char mqttUserPassword[32];
  char ntpweb_server[64];
  String URL_fw_Version = "";
  String URL_fw_Bin = "";
  bool unit;
  bool WifiM;
  bool RFMode;
  bool Debug;
  byte Type;
  int LoraCH;
  byte airRate;
  byte Protocol;
  byte COMtype;
  byte TimeSendData;
  bool RSSIenable;
  byte Scantime;
} user_setting = {};


File root;
bool opened = false;
bool Valve::getOpened() { return opened; }

unsigned long last_activity_timestamp = 0;
unsigned long last_Sent_timestamp = 0;
unsigned long last_Sent_timestamp1 = 0;
#ifdef USE_LoRa
LoraFunc::requestMessage RequestMessage;
LoraFunc::Message message;
#endif//USE_LoRa
bool Valve::oled_failed = false;
bool new_sensor_found = false;
struct_message  msg1;
//sensor_data     sensors[NUM_SENSORS];
//SDFunction::user_setting={};
int sensors_saved = 0;
uint8_t  incomingData[sizeof(msg1)];
size_t   received_msg_length;
uint8_t TimeSent = 0;
byte Valve::receiver_status = 1;   
bool Valve::update = false;
bool Valve::UploadProcess = false;
bool Valve::sending = false;
bool Valve::ap_mode = false;
uint8_t Valve::Signal_LED = 0;
uint8_t Valve::Startus_LED = 0;
uint8_t Valve::M0 = 0;
uint8_t Valve::M1 = 0;
uint8_t Valve::AUX = 0;
uint8_t Valve::LoRaPower = 0;
uint8_t Valve::MapPin[10] = {25,26, 2,15,19,22,21,27,13, 4};
const unsigned char logo [] PROGMEM = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0xff,0xf0,0x03,0x00,0x00,0x00
,0x00,0x00,0xff,0xf0,0x07,0xc0,0x00,0x00
,0x00,0x00,0xff,0xf0,0x1f,0xe0,0x00,0x00
,0x00,0x00,0xff,0xf0,0x1f,0xc0,0x00,0x00
,0x00,0x00,0xff,0xf0,0x0f,0xc0,0x00,0x00
,0x00,0x00,0xff,0xf0,0x01,0xe0,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x72,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x3f,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x1f,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x1f,0x80,0x00
,0x00,0x00,0xff,0xf0,0x00,0x0f,0x80,0x00
,0x00,0x00,0xff,0xf0,0x00,0x0f,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x0e,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x0e,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x0e,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x0e,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x0f,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x0f,0x80,0x00
,0x00,0x00,0xff,0xf0,0x00,0x1f,0x80,0x00
,0x00,0x00,0xff,0xf0,0x00,0x1f,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x3f,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x72,0x00,0x00
,0x00,0x00,0xff,0xf0,0x01,0xe0,0x00,0x00
,0x00,0x00,0xff,0xf0,0x0f,0xc0,0x00,0x00
,0x00,0x00,0xff,0xf0,0x1f,0xc0,0x00,0x00
,0x00,0x00,0xff,0xf0,0x1f,0xe0,0x00,0x00
,0x00,0x00,0xff,0xf0,0x07,0xc0,0x00,0x00
,0x00,0x00,0xff,0xf0,0x03,0x00,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x00,0x00,0x00
,0x00,0x00,0xff,0xf0,0x00,0x00,0x00,0x00
,0x00,0x00,0x9f,0xf0,0x00,0x00,0x00,0x00
,0x00,0x00,0x9f,0xf0,0x00,0x00,0x00,0x00
,0x00,0x00,0x9f,0xf0,0x60,0x1f,0xe0,0x00
,0x00,0x00,0x9f,0xf0,0xf0,0x3f,0xf0,0x00
,0x00,0x00,0x9f,0xf0,0xf8,0x3f,0xf0,0x00
,0x00,0x00,0x9f,0xf1,0xd8,0x38,0x70,0x00
,0x00,0x00,0x9f,0xf1,0x9c,0x3f,0xe0,0x00
,0x00,0x00,0x9f,0xf3,0xfc,0x3f,0xe0,0x00
,0x00,0x00,0x9f,0xf3,0xfe,0x38,0x70,0x00
,0x00,0x00,0x80,0x17,0xfe,0x3f,0xf0,0x00
,0x00,0x00,0x80,0x16,0x07,0x3f,0xf0,0x00
,0x00,0x00,0x80,0x36,0x03,0x1f,0xe0,0x00
,0x00,0x00,0xff,0xf0,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
    byte bbufer;char sbuf[MAX_DATA_LENGTH + 1];char sbuf1[MAX_DATA_LENGTH + 1];
void loadSetting();
//Contructor
Valve::Valve()
{

}
/* ############################ Setup ############################################ */

void Valve::valve_setup() {
                      //__0__1__2__3__4__5__6__7__8__9
  uint8_t MapPin1[10] = {27,14, 2,15,19,25,26,15,14,13};// main board gateway V14
  for(byte i = 0 ; i < 10; i++){MapPin[i] = MapPin1[i];}

  #ifdef DEBUG_ 
  Debug_Ser.println("Lookline Gateway V14");
  #endif//DEBUG_

  Signal_LED = MapPin[5];
  Startus_LED = MapPin[6];
  M0 = MapPin[7];
  M1 = MapPin[8];
  AUX = MapPin[9];
#ifdef Module7Seg
  pinMode(PWR, OUTPUT);
  digitalWrite(PWR, HIGH);

  pinMode(SHCP, OUTPUT);
  pinMode(STCP, OUTPUT);
  pinMode(DATA1, OUTPUT);
  pinMode(DATA2, OUTPUT);
  pinMode(DATA3, OUTPUT);
#endif//#ifdef Module7Seg  
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(AUX, OUTPUT);
  pinMode(Signal_LED, OUTPUT);
  pinMode(Startus_LED, OUTPUT);
  pinMode(LoRaPower, OUTPUT);

  pinMode(alert_pin, OUTPUT);
#ifdef USE_OLED
  showLogo();
#endif//#ifdef USE_OLED
  //Serial.begin(115200);
  LoRa_Ser.begin(9600);



CONFIG::InitDirectSD();
#ifdef USE_OLED
  Valve::showInfo("Wifi", "connecting to \n\n" + String(user_setting.ssid), 10);
 #endif// USE_OLED
  //char* SSIDReal = "HOANG DUC PHUONG";
    //strncpy(user_setting.ssid, SSIDReal , sizeof(SSIDReal) );
    //debugln("SSID: " + String(user_setting.ssid));
    //debugln("Pass: " + String(user_setting.password));



  if (WiFi.status() == WL_CONNECTED) {
    LOG("IP address:\t");
    LOG(WiFi.localIP());
#ifdef USE_OLED   
    Valve::showInfo("Wifi", WiFi.localIP().toString(), 10);
 #endif// USE_OLED 
 
 if(CONFIG::read_string(EP_TIME_SERVER1 ,sbuf , MAX_DATA_LENGTH )){LOG("\nTime web_server done:"+String(sbuf) + '\n');}   
    if (String(sbuf) != "" ) timeClient.setPoolServerName(sbuf);
    timeClient.begin();

    if(CONFIG::read_byte(EP_TIMEZONE ,&bbufer )){LOG("\nTime Zone: " + String(bbufer)+'\n');}
    timeClient.setTimeOffset(bbufer);
  }


if(CONFIG::read_string(EP_MQTT_BROKER ,sbuf ,MAX_MQTT_BROKER_LENGTH)){
  //LOG("\nMQTT broker: " + String(sbuf)+'\n');
}

  mqttClient.setServer( sbuf, 1883);
   String MAC = WiFi.macAddress();
   MAC.replace(":", "");
  thingName = "VPLab_Hub";
  new_sensor_found = false;

  //debugln("Setup MQTT Done");



    //Valve::showInfo("Lora", "read config", 3);
#ifdef USE_LoRa    
  ReadLoRaConfig();
#endif//USE_LoRa  
    //Valve::showInfo("Lora", "read config done", 3);
  //debugln("Setup Read Config Done");

//if(user_setting.RFMode == Master){debugln("Master");}else{debugln("slave");}

  last_Sent_timestamp = millis();
  Valve::sending = false;
for(int j = 0 ; j < NUM_SENSORS ; j++){sensors[j].GateWayCommand =  FeedbackCmd;}
for(int j = 0 ; j < NUM_SENSORS ; j++){sensors[j].GateWayControl =  CloseCmd;}
loadSetting();
//debug("demo" + String(EncodeRespondByte(1,1,0,0,0,0,0,0)));
  //debugln("Setup Done");
 // SDFunction::TestSDCard();
 loraSetup();
}//Setup




void loadSetting(){
  /* ############################
    char pversion[8];
  char ssid[30];
  char password[30];
  char mqttweb_server[60];
  char mqttUserName[32];
  char mqttUserPassword[32];
  char ntpweb_server[64];
  String URL_fw_Version = "";
  String URL_fw_Bin = "";
  bool unit;
  bool WifiM;
  bool RFMode;
  bool Debug;
  byte Type;
  int LoraCH;
  byte airRate;
  byte Protocol;
  byte COMtype;
  int TimeSendData;
  bool RSSIenable;
  byte Scantime;
  */
  byte buff;
  if (!CONFIG::read_string (EP_MQTT_BROKER, user_setting.mqttweb_server, MAX_MQTT_BROKER_LENGTH) ) {
                    //ESPCOM::print ("???", output, espresponse);
  } 
  if (!CONFIG::read_string (EP_MQTT_USER, user_setting.mqttUserName, MAX_MQTT_BROKER_LENGTH) ) {
                    //ESPCOM::print ("???", output, espresponse);
  } 
  if (!CONFIG::read_string (EP_MQTT_PASS, user_setting.mqttUserPassword, MAX_MQTT_BROKER_LENGTH) ) {
                    //ESPCOM::print ("???", output, espresponse);
  }   
   //////////////////////////////////////////// Time Scan
    if(CONFIG::read_byte(EP_LORA_T_SCAN, &user_setting.Scantime)){
    }
    
    if(CONFIG::read_byte(EP_LORA_T_REQUEST, &user_setting.TimeSendData)){
    }
  // if (!CONFIG::read_byte (EP_LORA_RSSI, *buff) ) {
  //                   //ESPCOM::print ("???", output, espresponse);
  // } 
  // if(buff){user_setting.RSSIenable = true;}
  // else{user_setting.RSSIenable = false;}
  // if (!CONFIG::read_byte (EP_LORA_AIRRATE, user_setting.airRate) ) {
  //                   //ESPCOM::print ("???", output, espresponse);
  // }
  // if (!CONFIG::read_byte (EP_LORA_PROTOCOL, user_setting.Protocol) ) {
  //                   //ESPCOM::print ("???", output, espresponse);
  // }
  // if (!CONFIG::read_byte (EP_LORA_MASTER, user_setting.COMtype) ) {
  //                   //ESPCOM::print ("???", output, espresponse);
  // }
  //LoraFunc::loraSetup();
}

void Valve::valve_loop() {
      //web_interface->web_server.handleClient();
      checkTimer();
    if(UploadProcess == false){
      timeClient.update();
      mqttReconnect();
      mqttClient.loop();
    //timeMQTT++;if(timeMQTT > 1000){timeMQTT = 0;}
      if ( millis() > last_activity_timestamp ) showIdle();
    
      //debugln("Run core 1");
      //pingReceiver();
      //ArduinoOTA.handle();
    }//UploadProcess

    if(UploadProcess == false){
      // timeLora++;if(timeLora > 100000){timeLora = 0;
      //   timeLora1++;if(timeLora1 > 50){timeLora1 = 0;showIdle();
      //   }
      // }
      if ( millis() > last_activity_timestamp  ){ //showIdle();
      }
      #ifdef USE_LoRa
      LoraFunct.LoraRead();
      #endif//USE_LoRa
      //for(byte i = 0 ; i < 20 ; i ++){EncodeRespondByte(ControlBit[])}
      Valve::SendRequest(1000);
      #ifdef USE_LoRa
      LoraFunct.LoraRead();
      #endif//USE_LoRa
      Valve::SendRequest(1000);

      //  if ( millis() > last_Sent_timestamp )
      //    {
      //      //showIdle();
      //      for(int j = 0 ; j < NUM_SENSORS ; j++){debug("ID:" + String(j) + "|" + String(sensors.GateWayCommand) + "| ");
      //        RequestMessage.code[j] = sensors.GateWayCommand;
      //      }
      //      debugln();
      //        last_Sent_timestamp = millis() + user_setting.Scantime * 60 * 1000;
      //    }

        if ( millis() > last_Sent_timestamp1 )
        {
          if(Valve::sending == false){for(int j = 0 ; j < NUM_SENSORS ; j++){sensors[j].GateWayCommand =  FeedbackCmd;}}
          if(Valve::sending == true){for(int j = 0 ; j < NUM_SENSORS ; j++){ sensors[j].GateWayCommand =  SleepCmd;}}
          Valve::sending = !Valve::sending;
          last_Sent_timestamp = millis();
          last_Sent_timestamp1 = millis() + 30 * 60 * 1000;
        }


/* ############################*/

      //debugln("ID:" + String(idcount) + "|Time:" + String(RequestTimeoutlist));
  #ifdef Mode0    
      for(int i = 0 ; i < NUM_SENSORS ; i++){
          HandleNode(i);
      }
  #endif//Mode0
    }
  
}//loop
 


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TimeSetup(){
   //////////////////////////////////////////// Time Scan
    if(CONFIG::read_byte(EP_LORA_T_SCAN, &user_setting.Scantime)){
    }
    
    if(CONFIG::read_byte(EP_LORA_T_REQUEST, &user_setting.TimeSendData)){
    }
    LOG ("TimeScan:"+ user_setting.Scantime + '\n');
    LOG ("TimeRequest:"+ user_setting.TimeSendData + '\n');
}
void Valve::MDelay(long t){
  for(int l = 0 ; l < t; l++){checkTimer(); 
  web_interface->web_server.handleClient();
#ifdef USE_LoRa
  LoraFunct.LoraRead();
#endif//USE_LoRa

  }
}

void Valve::SendRequest(long DELAY){
#ifdef USE_LoRa
    RequestMessage.code[0] = EncodeRespondByte(sensors[7].GateWayCommand,sensors[6].GateWayCommand,sensors[5].GateWayCommand,sensors[4].GateWayCommand,sensors[3].GateWayCommand,sensors[2].GateWayCommand,sensors[1].GateWayCommand,sensors[0].GateWayCommand);
    RequestMessage.code[1] = EncodeRespondByte(sensors[15].GateWayCommand,sensors[14].GateWayCommand,sensors[13].GateWayCommand,sensors[12].GateWayCommand,sensors[11].GateWayCommand,sensors[10].GateWayCommand,sensors[9].GateWayCommand,sensors[8].GateWayCommand);
    RequestMessage.code[2] = EncodeRespondByte(sensors[23].GateWayCommand,sensors[22].GateWayCommand,sensors[21].GateWayCommand,sensors[20].GateWayCommand,sensors[19].GateWayCommand,sensors[18].GateWayCommand,sensors[17].GateWayCommand,sensors[16].GateWayCommand);
    
    RequestMessage.code[3] = EncodeRespondByte(sensors[7].GateWayControl,sensors[6].GateWayControl,sensors[5].GateWayControl,sensors[4].GateWayControl,sensors[3].GateWayControl,sensors[2].GateWayControl,sensors[1].GateWayControl,sensors[0].GateWayControl);
    RequestMessage.code[4] = EncodeRespondByte(sensors[15].GateWayControl,sensors[14].GateWayControl,sensors[13].GateWayControl,sensors[12].GateWayControl,sensors[11].GateWayControl,sensors[10].GateWayControl,sensors[9].GateWayControl,sensors[8].GateWayControl);
    RequestMessage.code[5] = EncodeRespondByte(sensors[23].GateWayControl,sensors[22].GateWayControl,sensors[21].GateWayControl,sensors[20].GateWayControl,sensors[19].GateWayControl,sensors[18].GateWayControl,sensors[17].GateWayControl,sensors[16].GateWayControl);
    //sendMessage(const void *message, const uint8_t size)
    ResponseStatus ls = LoraFunct.loraSend(&RequestMessage, sizeof(RequestMessage));
    Valve::MDelay(DELAY);
#endif//USE_LoRa    
}
    
    int bat = 0;//EncodeRespond(BatL,BatH);
    int bat12 = 0;//EncodeRespond(BatL12,BatH12);
    int ID = 0;
void Valve::sensorMessageReceived(byte RSSI,byte IDh,byte IDl,byte State,byte BatH,byte BatL,byte Bat12H,byte Bat12L) {//push MQTT
     bat = EncodeRespond(BatL,BatH);
     bat12 = EncodeRespond(Bat12L,Bat12H);
     ID = EncodeRespond(IDl,IDh);
		//debugln(IDh);
		//debugln(IDl);
    //debugln(message.category);
		//debugln(State);
    debug("ID:"+ String(ID));
    debug(" | RSSI:"+ String(RSSI));
    debug(" | State:" + String(State));
		debug(" | bat:" + String(bat*0.0001));
		debugln(" | bat:" + String(bat12*0.0001));
   DynamicJsonDocument sensor(256);
   sensor["data"]["category"] = "Valve";
    sensor["data"]["status"]  = (byte)State;

  sensor["data"]["battery"] = float(bat*0.0001);
  sensor["data"]["battery12"] = float(bat12*0.0001);
  sensor["data"]["RSSI"] = (byte)RSSI;

  char payload[100];
  size_t n = serializeJson(sensor, payload);
  int ID = EncodeRespond(IDl,IDh);
  String address = String(ID);
  char id[2];
  strcpy(id, address.c_str());
  mqttPublish(id , payload, n );
  showMsg("Valve", address, String(State), String(bat*0.0001), String(bat12*0.0001),String(RSSI), 10);

}


void Valve::saveSensorData(byte RSSI,byte IDh,byte IDl,byte State,byte BatH,byte BatL,byte Bat12H,byte Bat12L) {//Save Web
     bat = EncodeRespond(BatL,BatH);
     bat12 = EncodeRespond(Bat12L,Bat12H);
     int ID = EncodeRespond(IDl,IDh);
  new_sensor_found = false;
  
  for (int i = 0; i < sensors_saved; i++) {
    if (sensors[i].sensor_id == ID){
       sensors[i].category = 1;//msg1.category;
      sensors[i].status = State;
      sensors[i].temperature = msg1.temperature;
      sensors[i].humidity = msg1.humidity;
      sensors[i].battery = bat*0.0001;
      sensors[i].battery12 = bat12*0.0001;
      sensors[i].RSSI = RSSI;
      //sensors[i].timestamp = time(nullptr);
      sensors[i].timestamp = timeClient.getEpochTime();
      new_sensor_found = true;
    }
  }

  if ( new_sensor_found == false ) {
    sensors[sensors_saved].category = 1;//msg1.category;
    sensors[sensors_saved].status = State;
    sensors[sensors_saved].temperature = msg1.temperature;
    sensors[sensors_saved].humidity = msg1.humidity;
    sensors[sensors_saved].battery = bat*0.0001;
    sensors[sensors_saved].battery12 = bat12*0.0001;
    sensors[sensors_saved].RSSI = RSSI;
    //sensors[sensors_saved].timestamp = time(nullptr);
    sensors[sensors_saved].sensor_id = ID;
    sensors[sensors_saved].timestamp = timeClient.getEpochTime();
    sensors_saved++;
  }
    if (!SD.begin(SDCard_CS)) {SDFunc.sd_card_found = false;} else {SDFunc.sd_card_found = true; }
    //LOG ("saveMemoryToFile > SD Card found:" + String(SDFunc.sd_card_found) + '\n');
    //LOG ("Time:" + String(timeClient.getEpochTime()) + '\n');
  if (SDFunc.sd_card_found) {
    String NameFile = "/data/" + String(ID) + ".log";
    File log_file = SD.open( NameFile, "a");
    time_t nows = time(nullptr);
    log_file.print(nows);
    log_file.print(",");
    log_file.print(1);
    log_file.print(",");
    log_file.print(State);
    log_file.print(",");
    log_file.print(msg1.temperature);
    log_file.print(",");
    log_file.print(msg1.humidity);
    log_file.print(",");
    log_file.print(bat*0.0001);
    log_file.print(",");
    log_file.print(bat12*0.0001);
    log_file.print(",");
    log_file.println(RSSI);
    log_file.flush();
    log_file.close();

    SDFunction::saveMemoryToFile();
  }
  
  sensors[ID].GateWayCommand = SleepCmd;
  for(byte l = 0 ; l < 20 ; l++){debug("ID:" + String(l) + "|" + String(sensors[l].GateWayCommand) + "| ");}
}



/* ################################# Tools ########################################### */

void alert() {

  digitalWrite(alert_pin, HIGH);
  delay(100);
  digitalWrite(alert_pin, LOW);
}

float Valve::getUserUnitTemperature(float t) {

  //if (false) return t * 1.8 + 32;
  return t;
}

String uptime() {

  int duration = time(nullptr);
  if (duration >= 86400) return String(duration / 86400) + " days";
  if (duration >= 3600)  return String(duration / 3600) + + " hours";
  if (duration >= 60)    return String(duration / 60) + " minutes";
  return String(duration) + " seconds";
}


void pingReceiver() {

  if ( millis() - Valve::next_receiver_ping_timestamp > 0 ) {
    //Serial.write('0');
    Valve::next_receiver_ping_timestamp =  millis() + 5 * 60 * 1000;  // every 5 minutes
    Valve::receiver_status = 0;
  }
}


void showLogo() {
  #ifdef USE_OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    delay(500); alert();
    delay(500); alert();
    delay(500); alert();
    Valve::oled_failed = true;
  }
  if (!Valve::oled_failed) {
    display.clearDisplay();
    display.dim(true);
  }

  if (Valve::oled_failed) return;
  display.drawBitmap(0, 0, logo, 64, 64, WHITE);//128/32
  display.display();
  alert();
  //delay(3000);
  #endif//USE_OLED
}


/* ################################# MQTT ########################################### */

void mqttReconnect() {
if(CONFIG::read_string(EP_MQTT_BROKER ,sbuf ,MAX_MQTT_BROKER_LENGTH)){
  mqtt_connected = false;
     //LOG("MQTT Broker:" + String(sbuf) + '\n');
}
  if ( String(sbuf) == "" || WiFi.status() != WL_CONNECTED) return;
  if ( mqttClient.connected() ) {
     //LOG("MQTT Broker connected:" + String(sbuf) + '\n');
    mqtt_connected = true; return;
  }
  if ( millis() - next_mqtt_connection_attempt_timestamp < 0 ){
  //LOG("Reconnect MQTT Broker:" + String(user_setting.mqttweb_server) + '\n');
  next_mqtt_connection_attempt_timestamp = millis() + 5 * 60 * 1000;  // retry every 5 minutes
#ifdef MQTT_User_Pass
  if(CONFIG::read_string(EP_MQTT_USER ,sbuf ,MAX_MQTT_USER_LENGTH)){;}
  if(CONFIG::read_string(EP_MQTT_PASS ,sbuf1 ,MAX_MQTT_PASS_LENGTH)){;}
  //if (mqttClient.connect( thingName.c_str(), sbuf, sbuf1, "stat/VPLab_sensors/LWT", willQoS, willRetain, willMessage)) {
#else
  if (mqttClient.connect( thingName.c_str(), "", "", "stat/VPLab_sensors/LWT", willQoS, willRetain, willMessage)) {
#endif// MQTT_User_Pass
    mqttClient.publish("stat/VPLab_sensors/status", "online");
    mqttClient.publish("stat/VPLab_sensors/LWT", "online");
    debugln(F("stat/VPLab_sensors/status ... online "));
    Valve::showInfo("MQTT", "connected", 3);
    mqtt_connected = true;
  } else {
    Valve::showInfo("MQTT", "connection failed", 3);
    mqtt_connected = false;
  }
  }
}

void Valve::mqttPublish(char macAdress[], String payload,  size_t len ) {

  strcpy (mqttTopic, "stat/VPLab_sensor/");
  strcat (mqttTopic, macAdress);
  debug(mqttTopic);
  debug(' ');
  debugln(payload);
  mqttClient.publish(mqttTopic, payload.c_str() , len);
}


unsigned int Valve::EncodeRespond(byte bytel,byte byteh)
{
    int ret = 0;
    byte byte_one = 0;
    byte byte_two = 0;
    byte_two = bytel;
    byte_one = byteh;
    ret = byte_one;
    ret = ret << 8;
    ret = ret | byte_two;
    return ret;
}
byte Valve::EncodeRespondByte(boolean a, boolean b, boolean c, boolean d, boolean e, boolean f, boolean g, boolean h)
    {
        byte byte_1 = 0;
        byte_1 = a;
        byte_1 = byte_1 << 1;
        byte_1 = byte_1| b;
        byte_1 = byte_1 << 1;
        byte_1 = byte_1 | c;
        byte_1 = byte_1 << 1;
        byte_1 = byte_1 | d;
        byte_1 = byte_1 << 1;
        byte_1 = byte_1 | e;
        byte_1 = byte_1 << 1;
        byte_1 = byte_1 | f;
        byte_1 = byte_1 << 1;
        byte_1 = byte_1 | g;
        byte_1 = byte_1 << 1;
        byte_1 = byte_1 | h;
        return byte_1;
    }
#ifdef TIMER_INTER
////////////////////////////////////////// TIMER INTERUPT ///////////////////////////////////
volatile int interruptCounter;
uint32_t totalInterruptCounter; 
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  totalInterruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}
#endif//TIMER_INER
void Valve::checkTimer()
{
  ///////////////////////////////////////////////////////////////////////////////////

  #ifdef TIMER_INTER
    if (interruptCounter > 0) {
      /////// Node Timer 
      for(int i = 0 ; i < NUM_SENSORS ; i++){
         sensors[i].Nodecounter++;
      }

      //////////////////
      TimeSent++;
    // if(TimeSent >= User_Setting.TimeSendData){TimeSent = 0;
    //   //SendCommand();
    //   //debugln("ping");
    // }
    if(totalInterruptCounter >= 10*60*60*24){
      //startup = false;
    totalInterruptCounter = 0;
    }
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
  }
  #endif//#ifdef TIMER_INTER
}

/* ############################ Sensors ############################################# */



// void Learning(byte ID)
// {
//   #ifdef Mode0
//   HandleNode(ID);
//   #else
//   sensors[ID].NodeRun = 2;   
//   #endif//Mode0 
//   if(sensors[ID].Nodecounter < sensors[ID].NodeTimeOut){
//     if(sensors[ID].NodeRun == 2){sensors[ID].NodeRun = 0;
//     debugln(String(sensors[ID].Nodecounter) +"|"+String(sensors[ID].NodeTimeOut)+" | Back to Learning");
//     if(sensors[ID].GateWayCommand < 2){SendRequest(Learn, ID, 1500);}
//     }
//   }
//   if(sensors[ID].NodeRun == 1){
//   sensors[ID].NodeRun = 2;// End Learing
//   sensors[ID].NodeTimeOut = sensors[ID].Nodecounter;
//   sensors[ID].Nodecounter = 0;
//   debugln("End Learn, Time Wakeup: " + String(sensors[ID].NodeTimeOut));
//   //SaveMemoryToFile();
//   }
//   // else if(sensors[ID].NodeRun == 1 && sensors[ID].NodeTimeOut > 5){
//   // if(ID == IDSync){synchronized = false;startup = false;}
//   // sensors[ID].NodeTimeOut = 1;
//   // sensors[ID].Nodecounter = 0;
//   // sensors[ID].NodeRun = 1;// Begin Learning
//   // debugln("Begin Learning");
//   // }
//   else if(sensors[ID].NodeRun == 0){
//   if(ID == IDSync){synchronized = false;startup = false;}
//   synchronized = startup = false;
//   sensors[ID].NodeTimeOut = 0;
//   sensors[ID].Nodecounter = 0;
//   sensors[ID].NodeRun = 1;// Begin Learning
//   debugln("Begin Learning");
//   }
//   else if(sensors[ID].NodeRun == 2){
//     debugln(String(sensors[ID].Nodecounter) +"|"+String(sensors[ID].NodeTimeOut)+" comfirm recive data"); 
//     //sensors[ID].NodeTimeOut = sensors[ID].Nodecounter;
//     //debugln("update time:" + String(sensors[ID].Nodecounter) +"|"+String(sensors[ID].NodeTimeOut)); 
//     NodeCycleRequest[ID] = 0;//comfirm recive data
//     sensors[ID].Nodecounter = 0;
//     sensors[ID].NodeComfirm = 0;
    
//   }  
// }

// int g = 0;

// void HandleNode(int i){
// //if(sensors[i].NodeTimeOut > 0){
//   if(sensors[i].Nodecounter >= sensors[i].NodeTimeOut  && sensors[i].NodeRun >= 2){
//     if(NodeCycleRequest[i] >= user_setting.TimeSendData){
//         if(sensors[i].GateWayCommand < 2){sensors[i].GateWayCommand = FeedbackCmd;}
//         SendRequest(sensors[i].GateWayCommand, i, 2000);
//         debugln("feedback cmd to ID:"+String(i)+", time:" + String(sensors[i].Nodecounter));
//         if(sensors[i].Nodecounter >= sensors[i].NodeTimeOut){sensors[i].NodeComfirm++;debugln("Timeout");}
//         if(sensors[i].NodeComfirm > 10){
//           debugln("Timeout");
//           synchronized = startup = true;IDSync = i;
//           sensors[i].NodeComfirm = 0;sensors[i].NodeTimeOut = 0;sensors[i].NodeRun = 0;
//         }
//         NodeCycleRequest[i]= 0;
//        //}
//     }else{
//       //if(sensors[i].NodeComfirm == 0){sensors[i].NodeComfirm = 1;
//        //for(int j = 0; j < 2 ; j++){
//         if(sensors[i].GateWayCommand < 2){sensors[i].GateWayCommand = SleepCmd;}
//         else{SendRequest(sensors[i].GateWayCommand, i, 2000);}
        
//        if(sensors[i].Nodecounter >= sensors[i].NodeTimeOut){sensors[i].NodeComfirm++;}
//         if(sensors[i].NodeComfirm > 10){
//           debugln("Timeout");
//           sensors[i].NodeComfirm = 0;sensors[i].NodeTimeOut = 0;sensors[i].NodeRun = 0;
//           synchronized = startup = true;IDSync = i;
//         }
//        //}
//         debugln("sleep cmd to ID:"+String(i)+", time:" + String(sensors[i].Nodecounter));sensors[i].Nodecounter = 0;
//        NodeCycleRequest[i]++;
//       //}
//       //if(wait >= 5){wait = 0;NodeCycleRequest[i]++;sensors[i].Nodecounter = 0;}
//     }
//   }
// //}
//   if(startup == true){  
//     if(synchronized){
//       SendRequest(Learn, IDSync, 2000);
//       debugln("Learn cmd to ID:"+String(IDSync));
//     }  
//     else{
//       for(g = 0 ; g < NUM_SENSORS; g++){
//         if(startup == false)break;
//         //if(sensors[g].NodeRun < 2){
//           SendRequest(Learn, g, 1500);
//           debugln("Learn cmd to ID:"+String(g));
//         //}
//       }
//     }
//   }
// }


void handleNotFound() {

  web_interface->web_server.sendHeader("Cache-Control", "public, max-age=604800, immutable");
  web_interface->web_server.send(404, "text/html", "404 - Page Not Found");
}

// void html_css() {

//   web_interface->web_server.sendHeader("Cache-Control", "public, max-age=604800, immutable");
//   //web_interface->web_server.send_P(200, "text/css", css);
//   if(loadFromSDCARD("/main.css")){
//   }
//   else{
//     SDhandleRoot();
//   }
// }

// void html_bootstrap() {

//   web_interface->web_server.sendHeader("Cache-Control", "public, max-age=604800, immutable");
//   // web_interface->web_server.send_P(200, "text/css", bootstrap);
//   if(loadFromSDCARD("/bootstrap.css")){
//   }
//   else{
//     SDhandleRoot();
//   }
// }

// void html_js() {

//   web_interface->web_server.sendHeader("Cache-Control", "public, max-age=604800, immutable");
//   //web_interface->web_server.send_P(200, "text/javascript", js);
//   if(loadFromSDCARD("/main.js")){
//   }
//   else{
//     SDhandleRoot();
//   }
// }

// String printDirectory(File dir, int numTabs) {
//   String response = SDFunction::SDweb_serverIndex + "</br><div class='row mb-0' style='font-size:0.8rem;'>";
//   dir.rewindDirectory();
  
//   while(true) {
//      File entry =  dir.openNextFile();
//      if (! entry) {
//        // no more files
//        //debugln("**nomorefiles**");
//        break;
//      }
//      for (uint8_t i=0; i<numTabs; i++) {
//        debug('\t');   // we'll have a nice indentation
//      }
//      // Recurse for directories, otherwise print the file size
//      if (entry.isDirectory()) {
//        printDirectory(entry, numTabs+1);
//      } else {
//        String Xbutton = "<div class='col-10 text-left'><a style='color:black;font-size:16px;text-decoration: none;' href='/deleteF?id="+String(entry.name())+"'> <svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\" fill=\"currentColor\" class=\"bi bi-trash3-fill\" viewBox=\"0 0 16 16\"><path d=\"M11 1.5v1h3.5a.5.5 0 0 1 0 1h-.538l-.853 10.66A2 2 0 0 1 11.115 16h-6.23a2 2 0 0 1-1.994-1.84L2.038 3.5H1.5a.5.5 0 0 1 0-1H5v-1A1.5 1.5 0 0 1 6.5 0h3A1.5 1.5 0 0 1 11 1.5Zm-5 0v1h4v-1a.5.5 0 0 0-.5-.5h-3a.5.5 0 0 0-.5.5ZM4.5 5.029l.5 8.5a.5.5 0 1 0 .998-.06l-.5-8.5a.5.5 0 1 0-.998.06Zm6.53-.528a.5.5 0 0 0-.528.47l-.5 8.5a.5.5 0 0 0 .998.058l.5-8.5a.5.5 0 0 0-.47-.528ZM8 4.5a.5.5 0 0 0-.5.5v8.5a.5.5 0 0 0 1 0V5a.5.5 0 0 0-.5-.5Z\"/></svg></a></div>";
//        response += String("<div class='col-2 text-left'>" + String(entry.name()) + "</div>")  ;
//        response += Xbutton + "</br></html> ";
//      }
//      entry.close();
//    }
//    return  response + String("</br></br>") ;
// }


bool loadFromSDCARD(String path){
  path.toLowerCase();
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".txt")) dataType = "text/plain";
  else if(path.endsWith(".csv")) dataType = "text/plain";
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".js")) dataType = "text/js";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".log")) dataType = "text/log";
  else if(path.endsWith(".bin")) dataType = "text/bin";
  else if(path.endsWith(".zip")) dataType = "application/zip";  
  //debugln(dataType);
  File dataFile = SD.open(path.c_str());

  if (!dataFile)
    return false;

  if (web_interface->web_server.streamFile(dataFile, dataType) != dataFile.size()) {
    debugln("Sent less data than expected!");
  }

  dataFile.close();
  return true;
}

void SDhandleRoot() {
  root = SD.open("/");
  String res = SDFunction::printDirectory(root, 0);
  web_interface->web_server.send(200, "text/html", res);
}

void SDhandleRoot1() {
  root = SD.open("/data");
  String res = SDFunction::printDirectory(root, 0);
  web_interface->web_server.send(200, "text/html", res);
}

void WebFunction()
{
  

  web_interface->web_server.begin();
}
/* ############################ Portal ############################################# */

void handleControl(){
  //format /Ctrl?id=x,
  int ID = web_interface->web_server.arg("id").toInt();
  byte StateControl = web_interface->web_server.arg("s").toInt();
  debugln("Cmd Control | ID:"+ String(ID) +" | State:"+String(StateControl));
  sensors[ID].GateWayCommand = StateControl;// 0: Feedback,1: sleep, 2:Open,3:Close
}
void handleRoot() {
  //web_interface->web_server.send_P(200, "text/html", html_main);
  if(loadFromSDCARD("/main.html")){
  }
  else{
    SDhandleRoot();
  }
}

void handleRaw() {
  //LOG ("/raw" + '\n');
  String raw;
  for (int i = 0; i < sensors_saved; i++) {
    raw += String(sensors[i].sensor_id) + ",";
    raw += String(sensors[i].category) + ",";
    raw += String(sensors[i].status) + ",";
    raw += String(Valve::getUserUnitTemperature(sensors[i].temperature)) + ",";
    raw += String(sensors[i].humidity) + ",";
    if ( user_setting.unit == 1  ) raw += "°F,";
    else raw += "°C,";
    raw += String(sensors[i].battery ) + ",";
    raw += String(sensors[i].battery12 ) + ",";
    raw += String(sensors[i].timestamp) + ",";
    raw += String(sensors[i].RSSI);
    if ( i < sensors_saved - 1) raw += '\n';
  }
  web_interface->web_server.send(200, "text/plain", raw);
}

void handleStatus() {

  //LOG ("/status" + '\n');
  // sd card, mqtt, receiver ota, ntp
  bool mqtt_is_good = true;
  loadSetting();
  if ( String(user_setting.mqttweb_server) != "" && mqtt_connected != true ) mqtt_is_good = false;

  web_interface->web_server.send(200, "text/plain", String(SDFunc.sd_card_found) + "," + String( mqtt_is_good ) + "," + String(Valve::Valve::receiver_status) + "," + String( timeClient.getEpochTime() > 1635652800 ));
}



// void handleConfig() {

//   if (web_interface->web_server.method() == HTTP_POST) {
//     strncpy(user_setting.ssid,              web_interface->web_server.arg("ssid").c_str(),             sizeof(user_setting.ssid) );
//     strncpy(user_setting.password,          web_interface->web_server.arg("password").c_str(),         sizeof(user_setting.password) );
//     strncpy(user_setting.mqttweb_server,        web_interface->web_server.arg("mqttweb_server").c_str(),       sizeof(user_setting.mqttweb_server) );
//     strncpy(user_setting.mqttUserName,      web_interface->web_server.arg("mqttUserName").c_str(),     sizeof(user_setting.mqttUserName) );
//     strncpy(user_setting.mqttUserPassword,  web_interface->web_server.arg("mqttUserPassword").c_str(), sizeof(user_setting.mqttUserPassword) );
//     strncpy(user_setting.ntpweb_server,         web_interface->web_server.arg("ntpweb_server").c_str(),        sizeof(user_setting.ntpweb_server) );
//     if ( web_interface->web_server.arg("unit") == "1")  user_setting.unit = 1;
//     else user_setting.unit = 0;
//     if ( web_interface->web_server.arg("wifiM") == "1") user_setting.WifiM = 1;
//     else user_setting.WifiM = 0;
//     if ( web_interface->web_server.arg("rfM") == "1")   user_setting.RFMode = 1;
//     else user_setting.RFMode = 0;
//     if ( web_interface->web_server.arg("debuG") == "1") user_setting.Debug = 1;
//     else user_setting.Debug = 0;
//     user_setting.LoraCH = web_interface->web_server.arg("Chnl").toInt();
//     user_setting.TimeSendData = web_interface->web_server.arg("TimeSendData").toInt();
//     user_setting.Scantime = web_interface->web_server.arg("ScanTime").toInt();
//     if (web_interface->web_server.arg("ComType") == "0") user_setting.COMtype = LoRa;
//     else if (web_interface->web_server.arg("ComType") == "1") user_setting.COMtype = MESH;
//     else if (web_interface->web_server.arg("ComType") == "2") user_setting.COMtype = MQTT;
//     else if (web_interface->web_server.arg("ComType") == "3") user_setting.COMtype = RS485com;

//     if      (web_interface->web_server.arg("AirRate") == "0") user_setting.airRate = 0;
//     else if (web_interface->web_server.arg("AirRate") == "1") user_setting.airRate = 1;
//     else if (web_interface->web_server.arg("AirRate") == "2") user_setting.airRate = 2;
//     else if (web_interface->web_server.arg("AirRate") == "3") user_setting.airRate = 3;
//     else if (web_interface->web_server.arg("AirRate") == "4") user_setting.airRate = 4;
//     else if (web_interface->web_server.arg("AirRate") == "5") user_setting.airRate = 5;

//     if      (web_interface->web_server.arg("protocol") == "0") user_setting.Protocol = 0;
//     else if (web_interface->web_server.arg("protocol") == "1") user_setting.Protocol = 1;
//     else if (web_interface->web_server.arg("protocol") == "2") user_setting.Protocol = 2;
//     else if (web_interface->web_server.arg("protocol") == "3") user_setting.Protocol = 3;
//     else if (web_interface->web_server.arg("protocol") == "4") user_setting.Protocol = 4;
//     else if (web_interface->web_server.arg("protocol") == "5") user_setting.Protocol = 5;
//     else if (web_interface->web_server.arg("protocol") == "5") user_setting.Protocol = 6;
//     else if (web_interface->web_server.arg("protocol") == "5") user_setting.Protocol = 7;

//     if      (web_interface->web_server.arg("rssi") == "0") user_setting.RSSIenable = 0;
//     else if (web_interface->web_server.arg("rssi") == "1") user_setting.RSSIenable = 1;

//     user_setting.ssid[web_interface->web_server.arg("ssid").length()]
//       = user_setting.password[web_interface->web_server.arg("password").length()]
//         = user_setting.mqttweb_server[web_interface->web_server.arg("mqttweb_server").length()]
//           = user_setting.mqttUserName[web_interface->web_server.arg("mqttUserName").length()]
//             = user_setting.mqttUserPassword[web_interface->web_server.arg("mqttUserPassword").length()]
//               = user_setting.ntpweb_server[web_interface->web_server.arg("ntpweb_server").length()]
//                 = 0;  // string terminate
//     strncpy(user_setting.pversion, PRGM_VERSION , sizeof(PRGM_VERSION) );
    
//     loraSetup();
//     EEPROM.put(0, user_setting);
//     EEPROM.commit();
//     EEPROM.get( 0, user_setting );
//     //SetLoRa();
//     debugln("Configuration has been saved to EEPROM");
    
//     debugln("TimeSendata: " + String(user_setting.TimeSendData));
//     debugln("ScanTime: " + String(user_setting.Scantime));
//     debugln("SSID: " + String(user_setting.ssid));
//     debugln("Pass: " + String(user_setting.password));
//     if(loadFromSDCARD("/save.html")){
//     }
//     else{
//       SDhandleRoot();
//     }
//     next_mqtt_connection_attempt_timestamp = millis();
//     e22ttl.setMode(MODE_0_NORMAL);
//   } else {
//     ReadLoRaConfig();
//     EEPROM.get( 0, user_setting );
//     uint32_t realSize = ESP.getSketchSize();//ESP.magicFlashChipSize();
//     uint32_t ideSize = ESP.getSketchSize();
//     String c = "", f = "";
//     String ap = "", sta = "";
//     String slave = "", master = "";
//     String enable = "", disable = "";
//     String str_lora ="", str_mqtt="", str_mesh="", str_rs485="";
//     if ( user_setting.unit == 1  ) f = " checked";
//     else c = " checked";
//     if ( user_setting.WifiM == 1  ) sta = " checked";
//     else ap = " checked";
//     if ( user_setting.RFMode == 1  ) master = " checked";
//     else slave = " checked";
//     if ( user_setting.Debug == 1  ) enable = " checked";
//     else disable = " checked";

//     String s = "<!DOCTYPE html><html lang='en'><head><meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=no'/>";
//     s += "<meta content='text/html;charset=utf-8' http-equiv='Content-Type'>";
//     s += "<title>The Hub by VPLab</title>";
//     s += "  <link rel='icon' type='image/png' sizes='16x16' href='data:image/x-icon;base64,AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAQAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIdUOc6IVTvpiFU654hVOuaIVTvkiVU74IdUOsqFUDaMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACIVDvli1c9/4pWPf+KVj3/ilY9/4pXPf+LVz3/jVk+/4lWPPaBSS5pAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgk064oVRPf+EUDz/hFA8/4RQPP98Rz3/fUk8/4VRPP+KVzz/jVk+/4ROMXwAAAAA/8s1///LNf//yzX//8s1///KM///yjL//8oy///KMv//yjL//8ky//C4Nf+wfjr/f0o8/4pWPP+LVz3/AAAAAP/FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xjP//8wy/76LOf+DTzz/jFg9/4ZTOLX/xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xzP/f0s8/4pWPP+IVTvj/8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8oy/6JwO/+HUzz/ilY88//FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///KMv+fbTv/h1Q8/4pWPPP/xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///GM///xTP/fUg8/4pWPP+JVjri/8Yz///GM///xjP//8Yz///GM///xjP//8Yz///GM///xjP//8Yz///HM///yzL/sX46/4RRPP+MWD3/h1I4sf/GMvT/xjL0/8Yy9P/FMvP4wDP99740//e+NP/3vjT/9740//O6NP/dpzf/m2k7/4JOPP+KVjz/iVY8/wAAAAAAAAAAAAAAAAAAAAAAAAAAd0A74X5IPf98Rzz/fEc8/3xHPP99SDz/gEw8/4dTPP+LVz3/jFg9/4FKL2sAAAAAAAAAAAAAAAAAAAAAAAAAAIhVOuaMWD7/i1c9/4tXPf+LVz3/i1c9/4xYPf+OWT//iFQ75XxDJ0gAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACHUji2iFQ6zohTOc2HVDnMh1M5yodUOsaGUjerf0YqVAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA//8AAPAPAADwAwAA8AEAAAABAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAA8AEAAPADAADwDwAA//8AAA=='/>";
//     s += "<meta content='utf-8' http-equiv='encoding'>";
//     s += "<link rel='stylesheet' href='bootstrap.css'>";
//     s += "<link rel='stylesheet' href='main.css'>";
//     s += "<meta content='utf-8' http-equiv='encoding'>";
//     s += "<link rel='stylesheet' href='bootstrap.css'>";
//     s += "<link rel='stylesheet' href='main.css'>";
//     //s += "<script src='main.js'></script><script>getROTAStatus();setInterval( getROTAStatus, 30000);</script>";
//     s += "</head><body>";
//     s += "<div id='top_bar' style='position:absolute;top:0px;height:5px;width:100%;background-color:#ff9900;'></div>";
//     s += "<div class='container py-3'>";
//     s += "<header>";
//     s += "  <div class='d-flex flex-column flex-md-row align-items-center pb-3 mb-4 border-bottom'>";
//     s += "    <a href='/' class='d-flex align-items-center text-dark text-decoration-none'><span class='fs-4 logol'></span></a><span class='fs-4'>Configuration</span>";
//     s += "    <nav class='d-inline-flex mt-2 mt-md-0 ms-md-auto'>";
//     s += "    <a class=\"me-3 py-2 text-dark text-decoration-none\" href=\"/\"><svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\" fill=\"currentColor\" class=\"bi bi-house\" viewBox=\"0 0 16 16\"><path fill-rule=\"evenodd\" d=\"M2 13.5V7h1v6.5a.5.5 0 0 0 .5.5h9a.5.5 0 0 0 .5-.5V7h1v6.5a1.5 1.5 0 0 1-1.5 1.5h-9A1.5 1.5 0 0 1 2 13.5zm11-11V6l-2-2V2.5a.5.5 0 0 1 .5-.5h1a.5.5 0 0 1 .5.5z\"/><path fill-rule=\"evenodd\" d=\"M7.293 1.5a1 1 0 0 1 1.414 0l6.647 6.646a.5.5 0 0 1-.708.708L8 2.207 1.354 8.854a.5.5 0 1 1-.708-.708L7.293 1.5z\"/></svg></a>";
//     s += "    <a class=\"me-3 py-2 text-dark text-decoration-none\" href=\"/info\"><svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\" fill=\"currentColor\" class=\"bi bi-info-circle\" viewBox=\"0 0 16 16\"><path d=\"M8 15A7 7 0 1 1 8 1a7 7 0 0 1 0 14zm0 1A8 8 0 1 0 8 0a8 8 0 0 0 0 16z\"/><path d=\"m8.93 6.588-2.29.287-.082.38.45.083c.294.07.352.176.288.469l-.738 3.468c-.194.897.105 1.319.808 1.319.545 0 1.178-.252 1.465-.598l.088-.416c-.2.176-.492.246-.686.246-.275 0-.375-.193-.304-.533L8.93 6.588zM9 4.5a1 1 0 1 1-2 0 1 1 0 0 1 2 0z\"/></svg></a>";
//     s += "    <a class=\"me-3 py-2 text-dark text-decoration-none\" href=\"/Upload\"><svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\"  fill=\"currentColor\" class=\"bi bi-download\" viewBox=\"0 0 16 16\"><path d=\"M.5 9.9a.5.5 0 0 1 .5.5v2.5a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1v-2.5a.5.5 0 0 1 1 0v2.5a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2v-2.5a.5.5 0 0 1 .5-.5z\"/><path d=\"M7.646 11.854a.5.5 0 0 0 .708 0l3-3a.5.5 0 0 0-.708-.708L8.5 10.293V1.5a.5.5 0 0 0-1 0v8.793L5.354 8.146a.5.5 0 1 0-.708.708l3 3z\"/></svg></a>";
//     s += "    <a class='me-3 py-2 text-dark text-decoration-none' href='/SDupload'><svg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' class='bi bi-upload' viewBox='0 0 16 16'><path d='M.5 9.9a.5.5 0 0 1 .5.5v2.5a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1v-2.5a.5.5 0 0 1 1 0v2.5a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2v-2.5a.5.5 0 0 1 .5-.5z'/><path d='M7.646 1.146a.5.5 0 0 1 .708 0l3 3a.5.5 0 0 1-.708.708L8.5 2.707V11.5a.5.5 0 0 1-1 0V2.707L5.354 4.854a.5.5 0 1 1-.708-.708l3-3z'/></svg></a>";
//     s += "    <a class='me-3 py-2 text-dark text-decoration-none' href='/' onclick='return reboot();'><svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\" fill=\"currentColor\" class=\"bi bi-bootstrap-reboot\" viewBox=\"0 0 16 16\"><path d=\"M1.161 8a6.84 6.84 0 1 0 6.842-6.84.58.58 0 1 1 0-1.16 8 8 0 1 1-6.556 3.412l-.663-.577a.58.58 0 0 1 .227-.997l2.52-.69a.58.58 0 0 1 .728.633l-.332 2.592a.58.58 0 0 1-.956.364l-.643-.56A6.812 6.812 0 0 0 1.16 8z\"/><path d=\"M6.641 11.671V8.843h1.57l1.498 2.828h1.314L9.377 8.665c.897-.3 1.427-1.106 1.427-2.1 0-1.37-.943-2.246-2.456-2.246H5.5v7.352h1.141zm0-3.75V5.277h1.57c.881 0 1.416.499 1.416 1.32 0 .84-.504 1.324-1.386 1.324h-1.6z\"/><svg></a>";
//     s += "    </nav>";
//     s += "  </div>";
//     s += "</header>";
//     s += "<body><main>";
//     s += "<div class='row justify-content-between'>";
//     if (ideSize != realSize) s += "<div class='alert alert-danger' role='alert'>Your flash size (" + String(ideSize) + ") is configured incorrectly. It should be " + String(realSize) + ".</div>";
//     s += "<div class='col-md-6 col-lg-7'>";
//     s += "<form action='/config' method='post'>";
//     s += "    <div class='col-12 mt-3'><label class='form-label'>Wifi Name</label><input type='text' name='ssid' class='form-control' value='" + String(user_setting.ssid) + "'></div>";
//     s += "    <div class='col-12 mt-3'><label class='form-label'>Password</label><input type='password' name='password' class='form-control' value='" + String(user_setting.password) + "'></div>";
//     s += "    <div class='col-12 mt-3'><label class='form-label mt-4'>MQTT web_server</label><input type='text' name='mqttweb_server' class='form-control' value='" + String(user_setting.mqttweb_server) + "'></div>";
//     s += "    <div class='col-12 mt-3'><label class='form-label'>MQTT Username</label><input type='text' name='mqttUserName' class='form-control' value='" + String(user_setting.mqttUserName) + "'></div>";
//     s += "    <div class='col-12 mt-3'><label class='form-label'>MQTT Password</label><input type='password' name='mqttUserPassword' class='form-control' value='" + String(user_setting.mqttUserPassword) + "'></div>";
//     s += "    <div class='col-12 mt-3'><label class='form-label mt-4'>NTP web_server</label><input type='text' name='ntpweb_server' class='form-control' value='" + String(user_setting.ntpweb_server) + "'></div>";
//     s += "    <div class='col-12 mt-3'><label class='form-label mt-4'>Sender Timeout</label><input type='number' name='TimeSendData' class='form-control' value='" + String(user_setting.TimeSendData) + "'></div>";
//     s += "    <div class='col-12 mt-3'><label class='form-label mt-4'>Sender Timeout</label><input type='number' name='ScanTime' class='form-control' value='" + String(user_setting.Scantime) + "'></div>";
//     s += "    <div class='col-12 mt-3'>";
//     s += "          <label class='form-label me-5 mt-4'>Unit</label>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='unit' value='0'" + c + "><label class='form-check-label'>Celsius</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='unit' value='1'" + f + "><label class='form-check-label'>Fahrenheit</label></div>";
//     s += "    </div>";
//     s += "    <div class='col-12 mt-3'>";
//     s += "          <label class='form-label me-5 mt-4'>Wifi Mode </label>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='wifiM' value='0'" + ap + "><label class='form-check-label'>AP(Offline)</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='wifiM' value='1'" + sta + "><label class='form-check-label'>STA(Online)</label></div>";
//     s += "    </div>";
//     s += "    <div class='col-12 mt-3'>";
//     s += "          <label class='form-label me-5 mt-4'>RF Mode </label>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='rfM' value='0'" + slave + "><label class='form-check-label'>RF Slave</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='rfM' value='1'" + master + "><label class='form-check-label'>RF Master</label></div>";
//     s += "    </div>";   
//     s += "    <div class='col-12 mt-3'>";
//     s += "          <label class='form-label me-5 mt-4'>Debug </label>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='debuG' value='0'" + enable + "><label class='form-check-label'>Debug enable</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='debuG' value='1'" + disable + "><label class='form-check-label'>Debug disable</label></div>";
//     s += "    </div>"; 
//     s += "    <div class='col-12 mt-3'><label class='form-label'>RFChanel</label><input type='number' name='Chnl' class='form-control' value='" + String(user_setting.LoraCH) + "'></div>"; 
//     s += "    <div class='col-12 mt-3'><label class='form-label me-5 mt-4'>Communication type </label>";
 
//         if(user_setting.COMtype == LoRa){str_lora = " checked";}
//         else if(user_setting.COMtype == MQTT){str_mqtt = " checked";}
//         else if(user_setting.COMtype == MESH){str_mesh = " checked";}
//         else if(user_setting.COMtype == RS485com){str_rs485 = " checked";}
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='ComType' value='0'" ; if(user_setting.COMtype <= 0){s += " checked";} s += "><label class='form-check-label'>LoRa network</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='ComType' value='1'" ; if(user_setting.COMtype == 1){s += " checked";} s += "><label class='form-check-label'>MQTT network</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='ComType' value='2'" ; if(user_setting.COMtype == 2){s += " checked";} s += "><label class='form-check-label'>MESH network</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='ComType' value='3'" ; if(user_setting.COMtype == 3){s += " checked";} s += "><label class='form-check-label'>RS485 network</label></div>";
//     s += "    </div>"; 
//     if(user_setting.COMtype == LoRa){
//     s += "    <div class='col-12 mt-3'><label class='form-label me-5 mt-4'>Speed AirRate </label>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='AirRate' value='0'"; if(user_setting.airRate <= 0){s += " checked";} s += "><label class='form-check-label'>0.3 kbps</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='AirRate' value='1'"; if(user_setting.airRate == 1){s += " checked";} s += "><label class='form-check-label'>1.2 kbps</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='AirRate' value='2'"; if(user_setting.airRate == 2){s += " checked";} s += "><label class='form-check-label'>2.4 kbps</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='AirRate' value='3'"; if(user_setting.airRate == 3){s += " checked";} s += "><label class='form-check-label'>4.8 kbps</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='AirRate' value='4'"; if(user_setting.airRate == 4){s += " checked";} s += "><label class='form-check-label'>9.6 kbps</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='AirRate' value='5'"; if(user_setting.airRate == 5){s += " checked";} s += "><label class='form-check-label'>19.2 kbps</label></div>";
//     s += "    </div>";   
//     s += "    <div class='col-12 mt-3'><label class='form-label me-5 mt-4'>Protocol</label>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='protocol' value='0'"; if(user_setting.Protocol <= 0){s += " checked";} s += "><label class='form-check-label'>TRANSPARENT</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='protocol' value='1'"; if(user_setting.Protocol == 1){s += " checked";} s += "><label class='form-check-label'>FIXED Node</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='protocol' value='2'"; if(user_setting.Protocol == 2){s += " checked";} s += "><label class='form-check-label'>FIXED Gateway</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='protocol' value='3'"; if(user_setting.Protocol == 3){s += " checked";} s += "><label class='form-check-label'>WOR Node</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='protocol' value='4'"; if(user_setting.Protocol == 4){s += " checked";} s += "><label class='form-check-label'>WOR Gateway</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='protocol' value='5'"; if(user_setting.Protocol == 5){s += " checked";} s += "><label class='form-check-label'>BROADCAST MESSAGE 1</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='protocol' value='6'"; if(user_setting.Protocol == 6){s += " checked";} s += "><label class='form-check-label'>BROADCAST MESSAGE 2</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='protocol' value='7'"; if(user_setting.Protocol == 7){s += " checked";} s += "><label class='form-check-label'>BROADCAST MESSAGE 3</label></div>";
//     s += "    </div>"; 
//     s += "    <div class='col-12 mt-3'><label class='form-label me-5 mt-4'>rssi</label>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='rssi' value='0'"; if(user_setting.RSSIenable <= 0){s += " checked";} s += "><label class='form-check-label'>RSSI disable</label></div>";
//     s += "          <div class='form-check form-check-inline'><input class='form-check-input' type='radio' name='rssi' value='1'"; if(user_setting.RSSIenable == 1){s += " checked";} s += "><label class='form-check-label'>RSSI Enable</label></div>";
//     s += "    </div>"; 
//     } 

//     s += "    <div class='form-floating'><br/><button class='btn btn-primary btn-lg' type='submit'>Save</button><br /><br /><br /></div>";
//     s += " </form>";
//     s += "</div>";
//     s += "<div class='col-md-5 col-lg-4 order-md-last bg-light p-4 rounded-3 border border-1 text-dark'>";
//     s += "<p class='fs-4 border-bottom'>MQTT Integration</p>";
//     s += "<p class='text-break'><strong>Topic<br /></strong>stat/VPLab_sensor_SENSORID/status</p>";
//     s += "<p class='text-break'><strong>Switch type load</strong><br />{\"data\": {\"category\":\"switch\",\"status\":x,\"battery\":xx.x }}</p>";
//     s += "<p class='text-break'><strong>Climate type load</strong><br />{\"data\": {\"category\":\"climate\",\"temperature\":xx.x,\"humidity\":xx.x,\"battery\":xx.x }}</p>";
//     s += "<p class='fs-4 border-bottom mt-5'>HTTP Integration</p>";
//     s += "<p class='text-break'><strong>URL<br /></strong>/status?id=SENSORID</p>";
//     s += "<p class='text-break'><strong>Load</strong><br />simular to the MQTT loads above</p>";
//     s += "<p class='fs-4 border-bottom mt-5'>Hardware Pin Map</p>";
//     s += "<p class='text-break'>Lora Serial: Serial 2 <br/> Lora M0: "+ String(M0) +" <br/>  Lora M1: "+ String(M1) +" <br/> Lora AUX: "+ String(AUX) +"</p>";
//     s += "<p class='text-break'><strong>LoRa config<br/></strong>CH: " + String(Lora_CH) + " </br> Air rate: " + String(Air_Rate) + " <br/> Baud rate: " + String(Baud_Rate) + " <br/> Power: " + String(Lora_PWR) + " <br/> RSSI:"+ String(Lora_RSSI) +"</p>";   
//     s += "<p class='text-break'><strong>Info<br/></strong>RF Mode: ";
//     if(user_setting.RFMode == 0)s += "Slave";
//     if(user_setting.RFMode == 1)s += "Master";
//     s += " <br/> Wifi Mode: ";
//     if(user_setting.WifiM == 0)s += "Offline";
//     if(user_setting.WifiM == 1)s += "Online";    
//     s += "</p>";   
//     s += "</div>";
//     s += "</div></main></body><script src='main.js'></script><script></script></html>";

//     web_interface->web_server.send(200, "text/html", s );
//   }
// }


void handleMqtt() {
  LOG ("/Mqtt" + '\n');

  loadSetting();                
  if ( String(user_setting.mqttweb_server) == "") web_interface->web_server.send(200, "text/plain", "unconfigured");
  if ( mqtt_connected == true ) web_interface->web_server.send(200, "text/plain", "online");
  else web_interface->web_server.send(200, "text/plain", "offline");
}



void handleRawFile() {

  /*  FORMAT: timestamp, category, status, temperature, humidty, battery  */
  if ( SDFunction::sd_card_found == false ) web_interface->web_server.send ( 404, "text/html",  "No SD card found." );

  if ( SD.exists("/data/" + web_interface->web_server.arg("id")) ) {
    File sensor = SD.open("/data/" + web_interface->web_server.arg("id"));
    int fsize = sensor.size();
    web_interface->web_server.sendHeader("Content-Length", (String)(fsize));
    size_t fsizeSent = web_interface->web_server.streamFile(sensor, "text/plain");
    sensor.close();
  }  else  web_interface->web_server.send ( 404, "text/html",  "Invalid request." );

}

void handleNames() {

  if ( SDFunc.sd_card_found == false ) web_interface->web_server.send ( 200, "text/html",  "" );

  if ( SD.exists("/config.txt") ) {
    File sensor = SD.open("/config.txt");
    int fsize = sensor.size();
    web_interface->web_server.sendHeader("Content-Length", (String)(fsize));
    size_t fsizeSent = web_interface->web_server.streamFile(sensor, "text/plain");
    sensor.close();
  }  else  web_interface->web_server.send ( 200, "text/html",  "" );
}

void handleDeleteFile() {

  if ( SD.exists("/" + web_interface->web_server.arg("id")) ) {
    SD.remove("/" + web_interface->web_server.arg("id"));
  }
}

void handleDeleteSensor() {

  if ( SD.exists("/data/" + web_interface->web_server.arg("id")) ) {
    SD.remove("/data/" + web_interface->web_server.arg("id"));
    web_interface->web_server.send(200, "text/plain", "1");
  } else {
    web_interface->web_server.send(200, "text/plain", "0");
  }
}

void handleRetrySD() {

  if (SDFunc.sd_card_found == true) {
    web_interface->web_server.send(200, "text/plain", "1");
    return;
  } else {

    if (!SD.begin(SDCard_CS)) {
      SDFunc.sd_card_found = false;
      Valve::showInfo("SDCard", "error: not found", 5);
      web_interface->web_server.send(200, "text/plain", "0");
    } else {
      Valve::showInfo("SD Card", "loading sensors", 3);
      web_interface->web_server.send(200, "text/plain", "1");
      SDFunc.sd_card_found = true;
      SDFunction::readMemoryFromFile();
    }
  }
}


void handleReboot() {

  web_interface->web_server.send(200, "text/plain", "1");
  Serial.write('1');
  delay(10);
  ESP.restart();
}

void handleScan1() {
  String id = web_interface->web_server.arg("id");
  last_Sent_timestamp = millis();
  Valve::sending = false;
  handleRoot();
}
//sensors_saved
void handleScan() {
  last_Sent_timestamp = millis();
  Valve::sending = false;
  for(int j = 0 ; j < NUM_SENSORS ; j++){sensors[j].GateWayCommand =  FeedbackCmd;}
  handleRoot();
}

void handleStopScan() {
  Valve::sending = true;
  last_Sent_timestamp = millis();
  handleRoot();
}

void handleROTA() {

  Serial.write('2');    // send '2' to enable the OTA for the receiver ( experimental)
  web_interface->web_server.send(200, "text/plain", "1");
}
    
void handleJson() {
    LOG ("/json\n");
    int bat = Valve::EncodeRespond(message.batteryl,message.batteryh);
    int bat12 = Valve::EncodeRespond(message.batteryl12,message.batteryh12);
  String sid = web_interface->web_server.arg("id");
  if (!sid) {
    web_interface->web_server.send(200, "application/json; charset=UTF-8", "0");
    return;
  }

  for (int i = 0; i < sensors_saved; i++) {
    char macAddr[18];
    //sprintf(macAddr, "%02X%02X%02X%02X%02X%02X", sensors[i].sensor_id[0], sensors[i].sensor_id[1], sensors[i].sensor_id[2], sensors[i].sensor_id[3], sensors[i].sensor_id[4], sensors[i].sensor_id[5]);
    if (  String(macAddr) == sid) {
      DynamicJsonDocument sensor(256);
    sensor["data"]["category"] = "Valve";
    sensor["data"]["status"]  = (byte)message.state ;
    sensor["data"]["battery"] = float(bat*0.0001);
    sensor["data"]["battery12"] = float(bat12*0.0001);

  char payload[100];
      serializeJson(sensor, payload);
      web_interface->web_server.send(200, "application/json; charset=UTF-8", payload);
      break;
    }
  }

}

void handleInfo() {

  uint32_t realSize = ESP.getFlashChipSize();
  uint32_t ideSize = ESP.getSketchSize();
  uint32_t  SDtotal = SD.totalBytes()/1000000;
  uint32_t SDfree = (SD.totalBytes()-SD.usedBytes())/1000000;
  FlashMode_t ideMode = ESP.getFlashChipMode();

  String payload;
  payload += "<!DOCTYPE html><html lang='en'><head><title>The Hub by MrDIY</title><style>*{font-family:system-ui,-apple-system,'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans',sans-serif,'Apple Color Emoji','Segoe UI Emoji','Segoe UI Symbol','Noto Color Emoji'} h4{margin-bottom:5px}</style></head><body><table border=0>";
  payload += "<a class=\"me-3 py-2 text-dark text-decoration-none\" href=\"/\"><svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\" fill=\"currentColor\" class=\"bi bi-house\" viewBox=\"0 0 16 16\"><path fill-rule=\"evenodd\" d=\"M2 13.5V7h1v6.5a.5.5 0 0 0 .5.5h9a.5.5 0 0 0 .5-.5V7h1v6.5a1.5 1.5 0 0 1-1.5 1.5h-9A1.5 1.5 0 0 1 2 13.5zm11-11V6l-2-2V2.5a.5.5 0 0 1 .5-.5h1a.5.5 0 0 1 .5.5z\"/><path fill-rule=\"evenodd\" d=\"M7.293 1.5a1 1 0 0 1 1.414 0l6.647 6.646a.5.5 0 0 1-.708.708L8 2.207 1.354 8.854a.5.5 0 1 1-.708-.708L7.293 1.5z\"/></svg></a>";
  payload += "<tr><td width=150><h4>Wifi</h4></td></tr>";
  payload += "<tr><td>mac address</td><td>" + WiFi.macAddress() + "</td></tr>";
  payload += "<tr><td>ssid</td><td>" + String(user_setting.ssid) + "</td></tr>";
  payload += "<tr><td>password</td><td>" + String(user_setting.password[0]) + "*********</td></tr>";
  payload += "<tr><td><h4>MQTT</h4></td></tr>";
  payload += "<tr><td>web_server</td><td>" + String(user_setting.mqttweb_server) + "</td></tr>";
  payload += "<tr><td>username</td><td>" + String(user_setting.mqttUserName) + "</td></tr>";
  payload += "<tr><td>password</td><td>" + String(user_setting.mqttUserPassword) + "</td></tr>";
  payload += "<tr><td><h4>NTP</h4></td></tr>";
  payload += "<tr><td>web_server</td><td>" + String(user_setting.ntpweb_server) + "</td></tr>";
  payload += "<tr><td><h4>Flash</h4></td></tr>";
  payload += "<tr><td>Chip Version</td><td>" + String(ESP.getSdkVersion()) + "</td></tr>";
  payload += "<tr><td>size (device)</td><td>" + String(realSize) + "</td></tr>";
  payload += "<tr><td>size (ide)</td><td>" + String(ideSize) ;
  if (ideSize != realSize) payload += " (Flash size is configured incorrectly)";
  payload += "</td></tr>";
  payload += "<tr><td>Mode</td><td>";
  if ( ideMode == FM_QIO )  payload += "QIO";
  if ( ideMode == FM_QOUT ) payload += "QOUT";
  if ( ideMode == FM_DIO )  payload += "DIO";
  if ( ideMode == FM_DOUT ) payload += "QOUT";
  payload += "</td></tr>";
  payload += "<tr><td><h4>Firmware</h4></td></tr>";
  payload += "<tr><td>version</td><td>" + String(user_setting.pversion) + "</td></tr>";
  payload += "<tr><td><h4>Time</h4></td></tr>";
  payload += "<tr><td>current</td><td>" + String(timeClient.getEpochTime()) + "</td></tr>";
  payload += "<tr><td>uptime</td><td>" + uptime() + "</td></tr>";
  payload += "<tr><td><h4>SD card</h4></td></tr>";
  payload += "<tr><td>Total Space</td><td>" + String(SDtotal) + "Mb" + "</td></tr>";
  payload += "<tr><td>Free Space</td><td>" + String(SDfree) + "Mb";  
  payload += "</table></body></html>";

  web_interface->web_server.send(200, "text/html; charset=UTF-8", payload);

}

/* ################################# OLED ########################################### */

void Valve::showInfo(String title, String msg, int idle_timeout) {
#ifdef USE_OLED
  if (Valve::oled_failed) return;
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(title);
  display.setTextSize(1);
  display.setCursor(0, 24);
  display.println(msg);
  if(SDFunc.sd_card_found){
    display.setCursor(115, 55);
    display.println("SD");
  }
  display.display();
#endif//USE_OLED  
  if (Valve::oled_failed) return;
  OLED_DISPLAY::clear_lcd();
  //OLED_DISPLAY::setTextSize(2);
  //OLED_DISPLAY::setTextColor(WHITE);
  OLED_DISPLAY::setCursor(0, 0);
  OLED_DISPLAY::print(title);
  
  OLED_DISPLAY::setCursor(0, 24);
  OLED_DISPLAY::print(msg);
  if(SDFunc.sd_card_found){
    OLED_DISPLAY::setCursor(115, 54);
    OLED_DISPLAY::print("SD");
  }
  OLED_DISPLAY::update_lcd();
  if ( idle_timeout > 0 ) last_activity_timestamp = millis() + idle_timeout * 1000;
}

void showMsg(String title, String mac, String status, String battery, String battery1,String RSSI, int idle_timeout) {
#ifdef USE_OLED
  if (Valve::oled_failed) return;
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(title);
  display.print(mac);
  display.setCursor(0, 22);
  if (status == "0"){display.print("Close");}
  if (status == "1"){display.print("Open");}
  
  display.setTextSize(1);
  display.setCursor(98, 24);
  display.print(battery);
  display.print("v");
  display.display();
  display.setCursor(98, 35);
  display.print(battery1);//
  display.setCursor(30, 46);
  display.print("RSSI:");
  display.print(RSSI);
  display.display();
#endif//#ifdef USE_OLED  
  if (Valve::oled_failed) return;
  OLED_DISPLAY::clear_lcd();
  //OLED_DISPLAY::setTextSize(2);
  //OLED_DISPLAY::setTextColor(WHITE);
  OLED_DISPLAY::setCursor(0, 0);
  OLED_DISPLAY::print(title);
  OLED_DISPLAY::print(mac);
  OLED_DISPLAY::setCursor(0, 22);
  if (status == "0"){OLED_DISPLAY::print("Close");}
  if (status == "1"){OLED_DISPLAY::print("Open");}
  
  
  OLED_DISPLAY::setCursor(98, 24);
  OLED_DISPLAY::print(String(battery) + "v");
  //OLED_DISPLAY::update_lcd();
  OLED_DISPLAY::setCursor(98, 35);
  OLED_DISPLAY::print(String(battery1) + "v");//
  OLED_DISPLAY::setCursor(30, 46);
  OLED_DISPLAY::print("RSSI:" + String(RSSI));
  OLED_DISPLAY::update_lcd();
  if ( idle_timeout > 0 ) last_activity_timestamp = millis() + idle_timeout * 1000;
}

void showFirmwareProgress(int progress) {
#ifdef USE_OLED
  if (Valve::oled_failed) return;
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Firmware");
  progress = progress > 100 ? 100 : progress;
  progress = progress < 0 ? 0 : progress;
  float bar = ((float)(128 - 2) / 100) * progress;
  display.drawRect(0, 24, 128, 7, WHITE);
  display.fillRect(2, 24 + 2, bar - 2, 6 - 3, WHITE);
  display.display();
  last_activity_timestamp = millis() + 10 * 1000;
#endif//#ifdef USE_OLED  
}

void showIdle() {

  if (Valve::oled_failed) return;
  if ( Valve::ap_mode == true) {
    #ifdef USE_OLED
    Valve::showInfo("Setup", "http://192.168.4.1", 30000);
    display.drawBitmap(110, 0, wifi1_icon16x16, 16, 16, WHITE);
    display.display();
    #endif//#ifdef USE_OLED
    
    Valve::showInfo("Setup", "http://192.168.4.1", 30000);
    //OLED_DISPLAY::drawXbm(110, 0, 16, 16, wifi1_icon16x16);
    OLED_DISPLAY::update_lcd();
    return;
  }
  String msg = "";
  int count = 0;
  float lowest_battery = 4.3;
  for (int i = 0; i < sensors_saved; i++) {
    if ( sensors[i].battery < battery_cutoff_volt && sensors[i].battery > 2) count++;
    if ( sensors[i].battery < lowest_battery && sensors[i].battery > 2) lowest_battery = sensors[i].battery;
  }

  if (count == 0) {
    if (WiFi.status() == WL_CONNECTED) msg =  WiFi.localIP().toString();
    else msg = "Wifi not connected";
  } else if ( count >= 1 ) {
    msg = "low battery alert";
    alert();
  }
  OLED_DISPLAY::clear_lcd();
  // ArialMT_Plain_10, ArialMT_Plain_16, ArialMT_Plain_24
  //OLED_DISPLAY::setFont(ArialMT_Plain_16);
  OLED_DISPLAY::setCursor(0, 0);
  String numSensor = String(sensors_saved);
  OLED_DISPLAY::print(numSensor);
  //OLED_DISPLAY::print(String(sensors_saved), OLED_PIPE);
  OLED_DISPLAY::setCursor(24, 0);
  if ( sensors_saved <= 1) OLED_DISPLAY::print("Device");
  else OLED_DISPLAY::print("Devices");
  
  OLED_DISPLAY::setCursor(0, 24);
  OLED_DISPLAY::print(msg);
  //if ( SDFunc.sd_card_found == false) {esp_display::drawXbm(110, 16, 16, 16, warning_icon16x16);OLED_DISPLAY::update_lcd();}
  if(Valve::sending == false){
    
  OLED_DISPLAY::setCursor(0, 42);OLED_DISPLAY::print("Scan Mode "+String(user_setting.Scantime)+" min");}
  else{OLED_DISPLAY::setCursor(0, 42);OLED_DISPLAY::print("request:"+String(user_setting.TimeSendData)+" min");}

#ifdef USE_OLED
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(sensors_saved);
  display.setCursor(24, 0);
  if ( sensors_saved <= 1) display.print("Device");
  else display.print("Devices");
  display.setTextSize(1);
  display.setCursor(0, 24);
  display.print(msg);
  if ( SDFunc.sd_card_found == false) display.drawBitmap(110, 16, warning_icon16x16, 16, 16, WHITE);
  if(Valve::sending == false){
  display.setCursor(0, 42);display.setTextSize(1);display.print("Scan Mode "+String(user_setting.Scantime)+" min");}
  else{display.setCursor(0, 42);display.setTextSize(1);display.print("request:"+String(user_setting.TimeSendData)+" min");}

  display.display();
 #endif// USE_OLED 
  last_activity_timestamp = millis() + 5 * 60 * 1000;
}



#endif//Valve_UI
