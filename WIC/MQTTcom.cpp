#define   DEBUG_FLAG

#include "config.h"
#ifdef MQTT_USE
#include "MQTTcom.h"
MQTTCOM mqttprg;
#include "command.h"

#include "wificonf.h"
#include "espcom.h"
#ifdef IOTDEVICE_UI
#include "AutoIT_IoT/IoTDevice.h"
IoT_Device AutoIT_MQTT;
#endif//IOTDEVICE_UI
#ifdef AUTOITGW_UI
#include "AutoIT_IoT/AutoITGW.h"
Auto_Device AutoIT_MQTT;
#endif//AUTOITGW_UI
#ifdef PLC_MASTER_UI
#include "PLC_IoT/PLC_Master.h"
PLC_MASTER mqtt_PLC;
#endif//PLC_MASTER
WiFiClient wifiClient_;
MQTTCOM mqttcom;

#include <ArduinoJson.h>
// #include "FileConfig.h"
//  NetworkFileConfig MQTTnetconfig;
//  RespondNetworkData MQTTNetworkDatas;

////////////////////////////////////////////////////////////////
#ifdef SDCARD_FEATURE
#include <SPI.h>
#include <SD.h>
#endif//SDCARD_FEATURE
////////////////////////////////////////////////////////////////

#ifdef DEBUG_FLAG
#define debug_mqtt(x) LOG(x)
#define debug_mqttln(x) LOGLN(x)
#else
#define debug_mqtt(x)
#define debug_mqttln(x)
#endif
/* ------------------------ Messages --------------------------------------- */

#define MESH_ID               6734922
#define GROUP_SWITCH          1
#define GROUP_HT              2
#define GROUP_MOTION          3
#define GROUP_RELAY           4
#define GROUP_MOISTURE        5
#define GROUP_TEMP            6
#define GROUP_VALVE           7

/* --------------------------- MQTT ---------------------------------------------- */

#define       MQTT_MSG_SIZE    200
char          mqttTopic[MQTT_MSG_SIZE];
#define       MSG_BUFFER_SIZE  (50)
PubSubClient  mqttClient(wifiClient_);
unsigned long next_mqtt_connection_attempt_timestamp1 = 0;
String        thingName;
const char*   willTopic         = "LWT";
const char*   willMessage       = "offline";
boolean       willRetain        = true;
byte          willQoS           = 0;
///  MQTT ///////////////////////////
String mqttbroker = "";
String mqttUserName="";
String mqttUserPassword="";
int32_t MQTTcount = 0;

// #include "RealTimeClock.h"

#include "time.h"
#include "TimeLib.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
WiFiUDP LogntpUDPMQTT;
NTPClient timeClientMQTT(LogntpUDPMQTT);




#ifdef vplab
String TopicUpdate = "/vplab/update";
String TopicStatus = "/vplab/status";
String TopicControl= "/vplab/control";
String TopicSetting= "/vplab/setting";
String Brand = "/stat";
#endif//vplab
#ifdef isoft
String TopicUpdate = "/isoft_sensor/update";
String TopicStatus = "/isoft_sensor/status";
String TopicControl= "/isoft_sensor/control";
String TopicSetting= "/isoft_sensor/setting";
String Brand = "/isoft";
#endif//isoft

#ifdef vule
String TopicUpdate = "/datalogger/shiratechPoE/stations/Line1/monitor";
String TopicStatus = "/isoft_sensor/status";
String TopicControl= "/isoft_sensor/control";
String TopicSetting= "/isoft_sensor/setting";
String Brand = "/vule";
#endif//isoft

const char* chararray = "";

const char* mqttserver = "";
const char* charmqttUserName = "";
const char* charmqttUserPassword = "";
byte MQTTreconnect = 0;
byte MQTTcheck = 0;bool PrintOnce = 1;

byte Debug = true;
byte DataACK = false;
String FileName = "";
byte type;
byte inhour;
String PushData = "";
void getDataFormSD();

void Show_Data_In(char* topic,byte* payload, unsigned int length)
{
    String Payload = "";
    String Topic = "";
    if(Debug){
        LOG("Message arrived [");
        LOG(topic);
        LOG("] ");}
        for (int i = 0; i < length; i++) {
            Payload += (char)payload[i];
        }
        if(Debug){
        LOGLN();
        LOGLN(Payload);}
}

/// @brief MQTT Subrise Message
/// @param topic 
/// @param payload 
/// @param length 
void callback(char* topic, byte* payload, unsigned int length) {
  String Payload = "";
  String Topic = String(topic);
    for (int i = 0; i < length; i++) {Payload += (char)payload[i];}
if(Debug){
    LOG("Topic:" + Topic);
    LOGLN(" Payload:" + Payload);
    }
    if (Payload.indexOf("data:") >= 0) {
      String dataRevice = Payload.substring(5);
      // Serial.println("json:" + dataRevice);
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, dataRevice);
        JsonObject obj = doc.as<JsonObject>();
        //{"id":1,"type":0,"week":0,"date":"2024-03-15"}
        String date = obj["date"]; type = obj[String("type")]; inhour = obj[String("houre")];int id = obj[String("id")];
        date.replace("-", "_");
        if(type == 0){FileName = "/" + String(id) + "_DataLog_" + date + ".csv";}
        if(type == 1){FileName = "/" + String(id) + "_DataLog_" + date + "_day.csv";}
        // LOGLN("file name: " + FileName);
        DataACK = true;
    }
    // Show_Data_In(topic, payload, length);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, Payload);
    JsonObject obj = doc.as<JsonObject>();
    String State = obj["state"];
    String Category = obj["category"];
    String SleepTime = obj["time"];
    // LOGLN(State);
    // LOGLN(Category);
    // LOGLN(SleepTime);
    if(State != "null"){
    String NetID = obj["netid"];
    String DeviceID = obj["id"];
    #ifdef AutoIT
    AutoIT_MQTT.Command("1",NetID ,DeviceID, State, "0");
    #endif
    // if(Debug)
    }

    if(Category != "null"){
    String NetworkID = obj["networkid"];
    String DeviceID = obj["nodeid"];
    #ifdef AutoIT
        AutoIT_MQTT.Command("0",NetworkID ,DeviceID, Category, SleepTime);
    #endif
    // if(Debug)

    }
}

byte MQTTCOM::GetDataACK(){return DataACK;}
void MQTTCOM::SetDataACK(byte d){ DataACK = d;}

bool Statup = 1;
void MQTTCOM::setup()
{
  mqttClient.disconnect();delay(1000);
  #ifdef RF
    RF_Serial.begin(115200);
  #endif
  int mqttPort;
  #ifdef FILECONFIG
    CFRespondNetworkData MQTTNetworkDatas;
    MQTTNetworkDatas = CONFIG::init_Network_config();
    mqttbroker= MQTTNetworkDatas.MQhost;
    mqttPort = MQTTNetworkDatas.MQport.toInt();
    mqttUserName = MQTTNetworkDatas.MQuser;mqttUserName.replace(" ","");
    charmqttUserName = mqttUserName.c_str();
    mqttUserPassword = MQTTNetworkDatas.MQpass;mqttUserPassword.replace(" ","");
    charmqttUserPassword = mqttUserPassword.c_str();
  #else
  if(CONFIG::read_string (EP_MQTT_BROKER, mqttbroker, MAX_MQTT_BROKER_LENGTH)){
    // LOGLN("mqtt broker:" + String(mqttbroker));
  }
  CONFIG::read_buffer (EP_MQTT_PORT,  (byte *) &mqttPort, INTEGER_LENGTH);
  #endif//#ifdef FILECONFIG

    mqttbroker.replace(" ","");
    mqttserver = mqttbroker.c_str();
    mqttClient.setServer( mqttserver, mqttPort);
    mqttClient.setCallback(callback);
  String MAC = WiFi.macAddress();
  MAC.replace(":", "");
  thingName = "i-Soft_Hub_" + MAC + random(0,9999);
    #ifdef AutoIT
        Debug = DEBUG; 
    #endif
  if(CONFIG::read_byte (EP_EEPROM_DEBUG, &Debug)){
    LOGLN("Debug mode :" + String((Debug==0)?"Not Debug":"Debug"));
  }
  // PrintOnce = 1;
}
unsigned long next_receiver_ping_timestamp1;
bool MQTTonce = true;
void MQTTCOM::loop()
{
  if( DataACK == 1){DataACK = 2;getDataFormSD();DataACK = 0;}
  if(WiFi.status() == WL_CONNECTED){mqttReconnect();mqttClient.loop();if(MQTTonce){LOG("MQTT Working...\n");MQTTonce = false;}}
}

/* ################################# MQTT ########################################### */

void MQTTCOM::mqttReconnect() {
  if ( String(mqttbroker) == "" || WiFi.status() != WL_CONNECTED){
    // #ifdef VOM
    //  MQTTcheck++;
    //   if(MQTTcheck > 5){
    //   byte RunMode = 1;
    //   CONFIG::read_byte(EP_WIFI_MODE, &RunMode);
    //   if(RunMode < 2) {RunMode = 2;CONFIG::write_byte(EP_WIFI_MODE, RunMode);}
    //   LOG("MQTT disconnect .... Restart");delay(1000);ESP.restart();}
    //   #endif//VOM
  }
  if ( mqttClient.connected() ) {
    mqtt_connected = true; return;
  }
  #ifdef FILECONFIG
  CFRespondNetworkData MQTTNetworkDatas;
  MQTTNetworkDatas = CONFIG::init_Network_config();
  if(PrintOnce){
    // LOGLN("___ MQTT ___ \n MQTT Host: " + MQTTNetworkDatas.MQhost);
    // LOGLN("MQTT Port: " + MQTTNetworkDatas.MQport);
    // LOGLN("MQTT User: " + MQTTNetworkDatas.MQuser);
    // LOGLN("MQTT Password: " + MQTTNetworkDatas.MQpass);
  }
  int mqttPort;
  mqttPort = MQTTNetworkDatas.MQport.toInt();
  mqttbroker = MQTTNetworkDatas.MQhost;mqttbroker.replace(" ","");
  mqttserver = mqttbroker.c_str();
  mqttUserName = MQTTNetworkDatas.MQuser;mqttUserName.replace(" ","");
  charmqttUserName = mqttUserName.c_str();
  mqttUserPassword = MQTTNetworkDatas.MQpass;mqttUserPassword.replace(" ","");
  charmqttUserPassword = mqttUserPassword.c_str();
  #else//EEPROM
    if(CONFIG::read_string (EP_MQTT_BROKER, mqttbroker, MAX_MQTT_BROKER_LENGTH)){
      if(PrintOnce)LOGLN("mqtt broker:" + String(mqttbroker));
    }
      mqttbroker.replace(" ","");
      mqttserver = mqttbroker.c_str();
    if(CONFIG::read_string (EP_MQTT_USER, mqttUserName, MAX_MQTT_USER_LENGTH)){
      if(String(mqttUserName) == "_"){charmqttUserName = "";}
      mqttUserName.replace("_","");
      if(PrintOnce)LOGLN("mqtt user:" + String(mqttUserName));
    }
      mqttUserName.replace(" ","");
      charmqttUserName = mqttUserName.c_str();
    if(CONFIG::read_string (EP_MQTT_PASS, mqttUserPassword, MAX_MQTT_PASS_LENGTH)){
      if(String(mqttUserPassword) == "_"){charmqttUserPassword = "";}
      mqttUserPassword.replace("_","");
      if(PrintOnce)LOGLN("mqtt pass:" + String(mqttUserPassword));
    } 

    mqttUserPassword.replace(" ","");
    charmqttUserPassword = mqttUserPassword.c_str();
  #endif//#ifdef FILECONFIG
     PrintOnce = 0; 
  mqtt_connected = false;

  MQTTcount--;
   if ( MQTTcount < 0 && !mqttClient.connected()) {MQTTcount = 50;
        
        mqttbroker.replace(" ","");
        mqttserver = mqttbroker.c_str();
        mqttClient.setServer( mqttserver, mqttPort);
         String MAC = WiFi.macAddress();
        MAC.replace(":", "");
        thingName = "i-Soft_Hub_" + MAC + random(0,9999);
        if (mqttClient.connect(thingName.c_str(),charmqttUserName,charmqttUserPassword)) {
          // if(Debug){
          // LOGLN("MQTT connected | user:" + String(charmqttUserName)+ "|pass:" + String(charmqttUserPassword) +"|");
          // LOGLN("connected");}
        String CharArray = Brand + TopicStatus;
        struct tm  tmstructMQTT;getLocalTime(&tmstructMQTT);
        if(tmstructMQTT.tm_hour > 24){tmstructMQTT.tm_hour = tmstructMQTT.tm_hour - 231;}
        if(tmstructMQTT.tm_hour > 24){tmstructMQTT.tm_hour = tmstructMQTT.tm_hour - 24;}
        int days = 0;if(tmstructMQTT.tm_mday > 10){days = tmstructMQTT.tm_mday-1;}else{days = tmstructMQTT.tm_mday;}
        // LOG("Time:" + String(tmstructMQTT.tm_hour)+':'+String(tmstructMQTT.tm_min) + '\n');
        // LOG("Date:" + String(tmstructMQTT.tm_mday-1)+"/"+String(tmstructMQTT.tm_mon+1)+"/"+String((tmstructMQTT.tm_year-100)+2000) + '\n');
        timeClientMQTT.update();byte ID = 0;CONFIG::read_byte (EP_EEPROM_ID, &ID);
        String msg = "Gateway online | " + String(timeClientMQTT.getEpochTime());
        msg =" Node "+ String(ID) + " Online | " + String(tmstructMQTT.tm_hour)+':'+String(tmstructMQTT.tm_min) + " | " + String(days)+"/"+String(tmstructMQTT.tm_mon+1)+"/"+String((tmstructMQTT.tm_year-100)+2000);
        if(Statup == 1){Statup = 0;mqttClient.publish(CharArray.c_str(), msg.c_str(), willRetain);}
        // if(Debug)LOGLN(F("Gateway online "));
        // if(Debug)LOGLN(CharArray.c_str());

        CharArray = Brand + TopicSetting;
        // if(Debug)LOGLN(CharArray.c_str());
        mqttClient.subscribe(CharArray.c_str());

        CharArray = Brand + TopicControl;
        // if(Debug)LOGLN(CharArray.c_str());
        mqttClient.subscribe(CharArray.c_str());
        // showInfo("MQTT", "connected", 3);
        if(Debug)LOGLN("MQTT connected");mqtt_connected = true;
        //Command("0","0","0","0","0");//0_0_0_0_0 MQTT loss
        //Command("0","0","0","0","1");//0_0_0_0_1 MQTT ok  
        #ifdef autoit
            AutoIT_MQTT.Command("0","0","0","0","1");//0_0_0_0_1 MQTT ok
        #endif
        ESPCOM::print("MQTT connected", WS_PIPE);
        mqtt_connected = true;
    } else {
        if(Debug)LOGLN("MQTT connect failed"); //mqttClient.disconnect();delay(1000);
        #ifdef autoit
            AutoIT_MQTT.Command("0","0","0","0","0");//0_0_0_0_0 MQTT loss
        #endif
        ESPCOM::print("MQTT connect failed", WS_PIPE);
        // showInfo("MQTT", "connection failed", 3);
        MQTTreconnect++; if(MQTTreconnect > 3)ESP.restart();
        mqtt_connected = false;
    }
  }
}
bool  MQTTCOM::connect_state() {return mqtt_connected;}
byte fail_count = 0;
void MQTTCOM::mqttPublish(String payload ,String Topic) {
    //char mqttUserPassword[10];
    String CharArray = Brand + Topic;
    strcpy (mqttTopic, CharArray.c_str());
    // strcat (mqttTopic, "/status");
    // LOGLN(mqttTopic);
    // LOGLN(payload);
    if(Debug){
      // debug_mqtt(mqttTopic);
      // debug_mqtt(' ');
      // debug_mqttln(payload);
      }
    int len = payload.length();
    if(mqttClient.publish_P(mqttTopic, (const uint8_t*)payload.c_str() ,len, willRetain)){
      // LOGLN("Push Done");
      fail_count = 0;
    }else{
      LOGLN("Push Failed");MQTTCOM::setup();
      fail_count++;if(fail_count > 10){ESPCOM::print("MQTT Failed", WS_PIPE);LOGLN("MQTT push failed Reset"); ESP.restart();}
    }
}

void getDataFormSD(){
  #ifdef DataLog
        PushData = COMMAND::get_dataLog(SD, FileName, type, inhour);
        // LOGLN("Load:\n " + PushData);
        mqttprg.mqttPublish(PushData, "/isoft_sensor/updateChart");
        PushData = "";
#endif//DataLog
}
#endif//MQTT_USE 