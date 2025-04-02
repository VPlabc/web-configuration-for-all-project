#define   DEBUG_FLAG

#include "config.h"
#ifdef MQTT_USE
#include "MQTTcom.h"
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
WiFiClient            wifiClient_;
MQTTCOM mqttcom;

#ifdef DEBUG_FLAG
#define debug(x) LOG(x)
#define debugln(x) LOGLN(x)
#else
#define debug(x)
#define debugln(x)
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
boolean       willRetain        = false;
byte          willQoS           = 0;
///  MQTT ///////////////////////////
String mqttbroker = "";
String mqttUserName="";
String mqttUserPassword="";
int32_t MQTTcount = 0;
String TopicUpdate = "/vplab/update";
String TopicStatus = "/vplab/status";
String TopicControl= "/vplab/control";
String TopicSetting= "/vplab/setting";

const char* chararray = "";

const char* mqttserver = "";
const char* charmqttUserName = "";
const char* charmqttUserPassword = "";


byte Debug = false;

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
    AutoIT_MQTT.Command("1",NetID ,DeviceID, State, "0");
    // if(Debug)
    }

    if(Category != "null"){
    String NetworkID = obj["networkid"];
    String DeviceID = obj["nodeid"];
    AutoIT_MQTT.Command("0",NetworkID ,DeviceID, Category, SleepTime);
    // if(Debug)

    }
}


void MQTTCOM::setup()
{
  RF_Serial.begin(115200);
  if(CONFIG::read_string (EP_MQTT_BROKER, mqttbroker, MAX_MQTT_BROKER_LENGTH)){
    // LOGLN("mqtt broker:" + String(mqttbroker));
  }
    mqttbroker.replace(" ","");
    mqttserver = mqttbroker.c_str();
    mqttClient.setServer( mqttserver, 1883);
    mqttClient.setCallback(callback);
  String MAC = WiFi.macAddress();
  MAC.replace(":", "");
  thingName = "VPLAB_Hub_" + MAC;
    Debug = DEBUG; 
  if(CONFIG::read_byte (EP_EEPROM_DEBUG, &Debug)){
    LOGLN("Debug:" + String((Debug==0)?"Not Debug":"Debug"));
  }
}
unsigned long next_receiver_ping_timestamp1;
bool MQTTonce = true;
void MQTTCOM::loop()
{
    if(WiFi.status() == WL_CONNECTED){mqttReconnect();mqttClient.loop();if(MQTTonce){LOG("MQTT Working...\n");MQTTonce = false;}}
}

/* ################################# MQTT ########################################### */
byte MQTTreconnect = 0;
void MQTTCOM::mqttReconnect() {
  if(CONFIG::read_string (EP_MQTT_BROKER, mqttbroker, MAX_MQTT_BROKER_LENGTH)){
    // LOGLN("mqtt broker:" + String(mqttbroker));
  }
    mqttbroker.replace(" ","");
    mqttserver = mqttbroker.c_str();
  if(CONFIG::read_string (EP_MQTT_USER, mqttUserName, MAX_MQTT_USER_LENGTH)){
    if(String(mqttUserName) == "_"){charmqttUserName = "";}
    // LOGLN("mqtt user:" + String(mqttUserName));
  }
    mqttUserName.replace("_","");
    charmqttUserName = mqttUserName.c_str();
  if(CONFIG::read_string (EP_MQTT_PASS, mqttUserPassword, MAX_MQTT_PASS_LENGTH)){
    if(String(mqttUserPassword) == "_"){charmqttUserPassword = "";}
    // LOGLN("mqtt pass:" + String(mqttUserPassword));
  }  
    mqttUserPassword.replace("_","");
    charmqttUserPassword = mqttUserPassword.c_str();
  mqtt_connected = false;

  if ( String(mqttbroker) == "" || WiFi.status() != WL_CONNECTED) return;
  if ( mqttClient.connected() ) {
    mqtt_connected = true; return;
  }
  MQTTcount--;
   if ( MQTTcount < 0 && !mqttClient.connected()) {MQTTcount = 500;

        if (mqttClient.connect(thingName.c_str())) {
          if(Debug){
          LOGLN("MQTT connected | user:" + String(charmqttUserName)+ "|pass:" + String(charmqttUserPassword) +"|");
          LOGLN("connected");}
        String CharArray = "/stat" + TopicStatus;
        mqttClient.publish(CharArray.c_str(), "Gateway online");
        if(Debug)LOGLN(F("Gateway online "));
        if(Debug)LOGLN(CharArray.c_str());

        CharArray = "/stat" + TopicSetting;
        if(Debug)LOGLN(CharArray.c_str());
        mqttClient.subscribe(CharArray.c_str());

        CharArray = "/stat" + TopicControl;
        if(Debug)LOGLN(CharArray.c_str());
        mqttClient.subscribe(CharArray.c_str());
        // showInfo("MQTT", "connected", 3);
        if(Debug)LOGLN("MQTT connected");
        //Command("0","0","0","0","0");//0_0_0_0_0 MQTT loss
        //Command("0","0","0","0","1");//0_0_0_0_1 MQTT ok  
        AutoIT_MQTT.Command("0","0","0","0","1");//0_0_0_0_1 MQTT ok
        ESPCOM::print("MQTT connected", WS_PIPE);
        mqtt_connected = true;
    } else {
        if(Debug)LOGLN("MQTT connect failed");
        AutoIT_MQTT.Command("0","0","0","0","0");//0_0_0_0_0 MQTT loss
        ESPCOM::print("MQTT connect failed", WS_PIPE);
        // showInfo("MQTT", "connection failed", 3);
        MQTTreconnect++;
        if(MQTTreconnect > 5)MQTTCOM::setup();
        mqtt_connected = false;
    }
  }
}

void MQTTCOM::mqttPublish(String payload ) {
    //char mqttUserPassword[10];
    String CharArray = "/stat" + TopicUpdate;
    strcpy (mqttTopic, CharArray.c_str());
    //   strcat (mqttTopic, "/status");
    if(Debug){
      // debug(mqttTopic);
      // debug(' ');
      // debugln(payload);
      }
    size_t len = payload.length();
    mqttClient.publish(mqttTopic, payload.c_str() , len);
}

#endif//MQTT_USE 