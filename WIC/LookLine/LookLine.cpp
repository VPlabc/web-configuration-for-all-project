
#include <Arduino.h>
#include "config.h"
#ifdef LOOKLINE_UI

// #include "wificonf.h"
#include "espcom.h"
// #include "webinterface.h"
// #include "command.h"
#include "WIC.h"
WIC looklineWIC;
#include "LookLine.h"
 Command cmdLookLine;
 LOOKLINE_PROG Lookline_PROG;
#include "7SegModule.h"
ESPResponseStream espresponse;
#include "FirmwareUpdate.h"
UpdateFW UDFWLookLine;
#include "syncwebserver.h"
WebSocketsServer * socket_servers;

#ifdef USE_LORA
    #include "LoRa_E32.h"
#endif//USE_LORA
#ifdef MQTT_Mode
    #include <PubSubClient.h>
    WiFiClient client;
    PubSubClient mqtt(client);
#endif//MQTT_Mode

byte DispMode = Main;
byte role = 0;
#include "MeshNetwork.h"
#include "MQTT.h"
#include "LoRa.h"
#define LoRa_Seri Serial2
#define PC_Seri Serial
#include "Task_Prog.h"
TaskPin taskPins;

bool newFWdetec = false;
byte Debug = true;
  byte start = false;//Start = 1 đag kết nối Wifi với đt | Start = 0 điện thoại đã ngắt kết nối
  byte config = 0; // config = 1 đag kết nối với điện thoại 

int CountOT_m=0;
int CountOT_Hm=0;
int CountOT_Lm=0;
int PLAN=0;
int RESULT=0;
byte ModuleType=0;
byte AmountNode = 0;
byte CheckLoss = 0;
  bool setupEn = false;
  byte NodeRun = 1;

  int NodeID =0;

  byte BoardIDs = 01;
  byte NetIDs = 01;

  uint8_t SHCP = 0;
  uint8_t STCP = 0;
  //////////////////
  uint8_t DATA1 = 0;
  //////////////////
  uint8_t DATA2 = 0;
  /////////////////
  uint8_t DATA3 = 0;
  ////// LED
  uint8_t Signal_LED = 0;
  uint8_t Startus_LED = 0;
  uint8_t M0 = 0;
  uint8_t M1 = 0;

  byte TimeSent = 10;
  byte MonitorMode = 0;
  byte ComMasterSlave = 1;
  byte Lora_CH = 0;
  int DisplayMode = Main;
  byte ComMode = LoRa;
  byte WiFiMode = 0;
  int counter2 = 0;//nhap nhay
  int counter3 = 0;//connected 
  int Counter4 = 0;//GatewayTimeoutSend 
  int Counter5 = 0;//GatewayTimeoutfeedback 
  int Counterstatus = 10000;//GatewayTimeout status
  byte countSer;
  // bool setupEn;
  bool done;
  // byte NodeID;
  int IDSent = 0;
  char buffer[250];
bool LookLineOnce = true;
bool LookLineOnce1 = true;
  int Time = 10;

 #ifdef __cplusplus
  extern "C" {
 #endif
  uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif

struct_Parameter_message DataLookline;
struct_Parameter_message  Looklines[NUM_LOOKLINES];
bool new_Lookline_found = false;
//SDFunction::user_setting={};
int Looklines_saved = 0;
static unsigned long OnWifiAutoMillis = 0;//sau 60s se tu dong chuyen sang mesh mode neu ko ket noi wifi

byte LooklineDebug = true;
uint8_t current_protocol;
int rssi_display = 0;
#include "esp_wifi.h"
#include <WiFi.h>

esp_interface_t current_esp_interface;
wifi_interface_t current_wifi_interface;

typedef struct {
  unsigned frame_ctrl: 16;
  unsigned duration_id: 16;
  uint8_t addr1[6]; /* receiver address */
  uint8_t addr2[6]; /* sender address */
  uint8_t addr3[6]; /* filtering address */
  unsigned sequence_ctrl: 16;
  uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;
//La callback que hace la magia
void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
  // All espnow traffic uses action frames which are a subtype of the mgmnt frames so filter out everything else.
  if (type != WIFI_PKT_MGMT)
    return;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  int rssi = ppkt->rx_ctrl.rssi;
  rssi_display = rssi;
}
int check_protocol()
{
  CONFIG::read_byte (EP_EEPROM_DEBUG, &LooklineDebug);
    char error_buf1[100];
  if(LooklineDebug){
    LOGLN();
    LOGLN("Lookline_________________________");
    LOGLN();
     esp_err_t error_code = esp_wifi_get_protocol(current_wifi_interface, &current_protocol);
     esp_err_to_name_r(error_code,error_buf1,100);
     LOG("esp_wifi_get_protocol error code: ");
     LOGLN(error_buf1);
    LOGLN("Code: " + String(current_protocol));
    if ((current_protocol&WIFI_PROTOCOL_11B) == WIFI_PROTOCOL_11B)
      LOGLN("Protocol is WIFI_PROTOCOL_11B");
    if ((current_protocol&WIFI_PROTOCOL_11G) == WIFI_PROTOCOL_11G)
      LOGLN("Protocol is WIFI_PROTOCOL_11G");
    if ((current_protocol&WIFI_PROTOCOL_11N) == WIFI_PROTOCOL_11N)
      LOGLN("Protocol is WIFI_PROTOCOL_11N");
    if ((current_protocol&WIFI_PROTOCOL_LR) == WIFI_PROTOCOL_LR)
      LOGLN("Protocol is WIFI_PROTOCOL_LR");
    LOGLN("___________________________________");
    LOGLN();
    LOGLN();
  }
    return current_protocol;
}

void LOOKLINE_PROG::caculaOT()
{
  float nums = 0;
  if (PLAN > RESULT && pcsInShift > 0)
  {
    //Debug_Ser.println("Step1");
    CountOT_m = (PLAN - RESULT)*100;
    //Debug_Ser.println("Step2");
    nums = CountOT_m / pcsInShift;
    //Debug_Ser.println("Step3");
  //Debug_Ser.println(nums);
    CountOT_Hm = nums / 100;
    //Debug_Ser.println("Step4");
    CountOT_Lm = nums - (CountOT_Hm * 100);
    //Debug_Ser.println("Step5");
  }
  else
  {
    CountOT_m = 0;
    CountOT_Hm = 0;
    CountOT_Lm = 0;
  //ActionOT = CountOT;
  }
}

void LOOKLINE_PROG::UpdateLookLineData(){
  CONFIG::read_byte(EP_EEPROM_ID, &BoardIDs);
  CONFIG::read_byte(EP_EEPROM_NETID, &NetIDs);
  CONFIG::read_byte(EP_EEPROM_CHANELS, &Lora_CH);
  CONFIG::read_buffer(EP_EEPROM_PLAN,(byte *) &PLAN, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_PLAN_SET,(byte *) &PLanSet, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_RESULT,(byte *) &RESULT, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_RESULT_SET,(byte *) &ResultSet, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_PLANMAX,(byte *) &PlanLimit, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_PCS,(byte *) &pcsInShift, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_TIME_PLAN,(byte *) &Time, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_TIMESENT,(byte *) &TimeSent, INTEGER_LENGTH);
  CONFIG::read_byte(EP_EEPROM_AMOUNTNODE, &AmountNode);
  CONFIG::read_byte(EP_EEPROM_RUN, &NodeRun);
  CONFIG::read_byte (EP_EEPROM_ROLE, &role);
  CONFIG::read_byte (EP_WIFI_MODE, &WiFiMode);
  CONFIG::read_byte (EP_EEPROM_DEBUG, &Debug);
  caculaOT();SerDisplay(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);
  #ifdef USE_LORA
  SetChanel(Lora_CH);
  #endif//USE_LORA
  OnWifiAutoMillis = millis();
  if(Debug)LOGLN("Update Data");
  if(start){socket_server->broadcastTXT("VALUE:" + String(PLAN) + ":" + String(RESULT) + ":" + String(CountOT_Hm) + "." + String(CountOT_Lm));//LOG("update socketData");
  }
}
void LOOKLINE_PROG::DebugOut(String msg,byte output){//1 = Web / 2 = Serial
if(output == 1) ESPCOM::webprint(msg);
if(output == 2) ESPCOM::print(msg, DEBUG_PIPE, &espresponse);
}

void LOOKLINE_PROG::LookLineInitB(int pos,byte Mode){
  OnWifiAutoMillis = millis();
CONFIG::read_byte(pos, &Mode);
if(pos == EP_EEPROM_ROLE){if(Debug)LOGLN("Role is :" + String(Mode));LookLineOnce1 = true;}
if(pos == EP_EEPROM_COM_MODE){if(Debug)LOGLN("Com mode is :" + String(Mode));ComMode = Mode;}
if(pos == EP_EEPROM_UPDATE_MODE){if(Debug)LOGLN("Update mode is :" + String(Mode));}
if(pos == EP_EEPROM_AMOUNTNODE){if(Debug)LOGLN("Amount node is :" + String(Mode));AmountNode = Mode;}
if(pos == EP_EEPROM_RUN){if(Debug)LOGLN("Run set is :" + String(Mode));NodeRun = Mode;
    caculaOT();SerDisplay(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);
  }
if(pos == EP_EEPROM_ON_OFF){if(Debug)LOGLN("On/Off set is :" + String(Mode));}
if(pos == EP_EEPROM_CHANELS){if(Debug)LOGLN("Chanel set is :" + String(Mode));
  #ifdef USE_LORA
  SetChanel(Mode);
  #endif//USE_LORA
  Lora_CH = Mode;}
if(pos == EP_EEPROM_MODULE_TYPE){if(Debug)LOGLN("Set Module type :" + String(Mode));PinMapInit();LookLineOnce1 = true;}
if(pos == EP_EEPROM_TIMESENT){if(Debug)LOGLN("Time Sent :" + String(Mode));}
if(pos == EP_EEPROM_DEBUG){if(Debug)LOGLN("Debuf Mode:" + String(Mode));}
// EP_EEPROM_TIMESENT
}


void LOOKLINE_PROG::LookLineInitI(int pos,int Mode){
  OnWifiAutoMillis = millis();
CONFIG::read_buffer(pos,(byte*) &Mode, INTEGER_LENGTH);
if(pos == EP_EEPROM_TIME_PLAN){if(Debug)LOGLN("Time for Plan is :" + String(Mode));Time = Mode;}
if(pos == EP_EEPROM_TIMESENT){if(Debug)LOGLN("TIME sent is :" + String(Mode));TimeSent = Mode;}
if(pos == EP_EEPROM_PLAN){if(Debug)LOGLN("Plan is :" + String(Mode));PLAN = Mode;
    caculaOT();SerDisplay(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);
}
if(pos == EP_EEPROM_PLAN_SET){if(Debug)LOGLN("Plan set is :" + String(Mode));PLanSet = Mode;}
if(pos == EP_EEPROM_RESULT){if(Debug)LOGLN("Result is :" + String(Mode));RESULT = Mode;
      caculaOT();SerDisplay(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);

}
if(pos == EP_EEPROM_RESULT_SET){if(Debug)LOGLN("Result set is :" + String(Mode));ResultSet = Mode;}
if(pos == EP_EEPROM_PCS){if(Debug)LOGLN("Result set is :" + String(Mode));pcsInShift = Mode;}
if(pos == EP_EEPROM_COUNTER_DELAY){if(Debug)LOGLN("Counter delay :" + String(Mode));SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);}
SerDisplay();
}
void LOOKLINE_PROG::PinMapInit(){
  
CONFIG::read_byte(EP_EEPROM_MODULE_TYPE, &ModuleType);
if(ModuleType == ModGateway){
  uint8_t MapPin1[10] = {27,14, 2,15,19,25,26,18, 5, 4};// main board gateway V14
  for(byte i = 0 ; i < 10; i++){MapPin[i] = MapPin1[i];}
  CONFIG::write_byte (EP_EEPROM_ROLE, GATEWAY) ;
 if(Debug){if(Debug)LOGLN("Lookline Gateway V14");}//Debug
}//if(ModuleType == MoGateway){
else{//lookline main V14
  uint8_t MapPin0[10] = {25,26, 2,15,19,27,13,22,21, 4};// main board lookline V14
  for(byte i = 0 ; i < 10; i++){MapPin[i] = MapPin0[i];}
  CONFIG::write_byte (EP_EEPROM_ROLE, NODE) ;
}
  SHCP = MapPin[0];
  STCP = MapPin[1];
  DATA1 = MapPin[2];
  DATA2 = MapPin[3];
  DATA3 = MapPin[4];
  Signal_LED = MapPin[5];
  Startus_LED = MapPin[6];
  M0 = MapPin[7];
  M1 = MapPin[8];
  PWR = MapPin[9];

  if(Debug)LOGLN("pinmap --- SHCP:" + String(SHCP) + " STCP:" + String(STCP)  + " DATA1:" + String(DATA1)   + " DATA2:" + String(DATA2) + " DATA3:" + String(DATA3) + " M0:" + String(M0) + " M1:" + String(M1) + " X0:" + String(X0)  + " X1:" + String(X1) + " X2:" + String(X2)  + " X3:" + String(X3)  + " X4:" + String(X4) + " Status_LED:" + String(Startus_LED) + " Signal_LED:" + String(Signal_LED) ) ;

  pinMode(PWR, OUTPUT);
  digitalWrite(PWR, HIGH);

  pinMode(SHCP, OUTPUT);
  pinMode(STCP, OUTPUT);
  pinMode(DATA1, OUTPUT);
  pinMode(DATA2, OUTPUT);
  pinMode(DATA3, OUTPUT);
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(0, INPUT_PULLUP);
  pinMode(X0, INPUT_PULLUP);
  pinMode(X1, INPUT_PULLUP);
  pinMode(X2, INPUT_PULLUP);
  pinMode(X3, INPUT_PULLUP);
  pinMode(X4, INPUT_PULLUP);
  
  //SW_Mode

  pinMode(Signal_LED, OUTPUT);
  pinMode(Startus_LED,OUTPUT);
}

void ShowParameters()
{
    if(Debug)LOGLN("Counter4:" + String(Counter4) + " |Counter5:" + String(Counter5) + " |Role: " + String(role) + " |Amount node: " + String(AmountNode) + " | Module type: " + String(ModuleType) + "| Com Mode: " + String(ComMode) + "| M0:" + String(digitalRead(M0)) + "| M1:" + String(digitalRead(M1) + "| CH:" + String(Lora_CH)));
}
void LOOKLINE_PROG::displayMode(byte Mode){
  DispMode = Mode;
}

void LOOKLINE_PROG::SetPlan(int SetPlans){
  PLAN = SetPlans;
  caculaOT();SerDisplay(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);TaskDisplay(DispMode);
}

void LOOKLINE_PROG::SetResult(int SetResults){
  RESULT = SetResults;
  caculaOT();SerDisplay(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);TaskDisplay(DispMode);
}
void LoRaLooklineSetup()
{
    // CONFIG::write_byte(EP_EEPROM_COM_MODE, LoRa);ComMode = LoRa;
    // check_protocol();
    // esp_wifi_set_protocol(current_wifi_interface, 7);
    // check_protocol();
    // if(SetupPortal() == false){LOGLN("SetupPortal failed");}
    
    byte wifiMode = 0;
    CONFIG::read_byte(EP_WIFI_MODE, &wifiMode);//2 station mode // 1 AP mode
    if (wifiMode == 1 && config == 0)
    {

      WiFi.disconnect();
      WiFi.disconnect(true); // Ngắt kết nối và xóa cấu hình trước đó
      WiFi.mode(WIFI_OFF);delay(100);
      WiFi.mode(WIFI_AP);
      esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PHY_MODE_11G);
      if (!wifi_config.Setup())
      {
        if(LooklineDebug)ESPCOM::println(F("Safe mode 1"), PRINTER_PIPE);
          if (!wifi_config.Setup(true))
          {
            if(LooklineDebug)ESPCOM::println(F("Safe mode 2"), PRINTER_PIPE);
              wifi_config.Safe_Setup();
          }
      }
    }
}
byte LOOKLINE_PROG::GetRun(){return NodeRun;}
byte LOOKLINE_PROG::GetDebug(){return LooklineDebug;}
bool LOOKLINE_PROG::GetFW(){return newFWdetec;}
void LOOKLINE_PROG::SetDone(){done = true;}
void LOOKLINE_PROG::SetRun(byte SetRuns){
  if(SetRuns < 2){
  NodeRun = SetRuns;if(Debug)LOGLN("Run/Stop Lookline");
  LOGLN("Run:"+ String(NodeRun));
  CONFIG::write_byte (EP_EEPROM_RUN, NodeRun);
  }
  if(SetRuns == 2){
    if(role == NODE|| role == REPEARTER){
      if(LooklineDebug)LOGLN("Off Lookline");
      if(DispMode == Main){DispMode = SLEEP;}else{DispMode = Main;}
    }
    if(role == GATEWAY){
      // if(ComMode != MESH){
          // CONFIG::write_byte(EP_EEPROM_COM_MODE, MESH);
          // MeshLookLineSetup();ComMode = MESH;if(LooklineDebug)ESPCOM::println(F("Mesh Lookline Setup"), PRINTER_PIPE);
          // }
          // else{CONFIG::write_byte(EP_EEPROM_COM_MODE, LoRa);
          // delay(1000);
          // ESP.restart();
          // }
    }
  }
  if(SetRuns == 5){ PLAN = RESULT = 0;
    CONFIG::write_buffer (EP_EEPROM_PLAN, (byte *) &PLAN, INTEGER_LENGTH);
    CONFIG::write_buffer (EP_EEPROM_RESULT, (byte *) &RESULT, INTEGER_LENGTH);
    CONFIG::read_buffer(EP_EEPROM_PLAN, (byte *) &PLAN, INTEGER_LENGTH);
    CONFIG::read_buffer(EP_EEPROM_RESULT, (byte *) &RESULT, INTEGER_LENGTH);
    Lookline_PROG.SetResult(0);Lookline_PROG.SetPlan(0);TaskDisplay(DispMode);
  }
}
void saveLooklineData(byte saveRSSI,byte saveID,byte saveNetID,byte saveState,int savePlan,int saveResult,byte Type,byte saveCom,byte saveWifi) ;

bool LOOKLINE_PROG::GetConfigState(){return config;}
void LOOKLINE_PROG::Set_Init_UI(String auths){if(LooklineDebug)LOGLN("Set_Init_UI " + auths);socket_server->broadcastTXT(auths);}
bool onceConfigMesh = true;
bool onceConfigWifi = true;
void LOOKLINE_PROG::SetStart(byte START){start = START;
  OnWifiAutoMillis = millis();
  LOGLN("Start: " + String(START));
  // if(config == 3){onceConfigMesh = true;if(onceConfigWifi){LOGLN("Wifi Mode");LoRaLooklineSetup();LOGLN("LoRaLooklineSetup");ComMode = LoRa;onceConfigWifi = false;}}
  }
  void LOOKLINE_PROG::SetConfig(bool CONFIG){config = CONFIG;
    LOGLN("Config: " + String(config));
    // if(CONFIG == 1){onceConfigWifi = true;if(onceConfigMesh){
    //   #ifdef Mesh_Network
    //   MeshLookLineSetup();
    //   #endif//
    //   onceConfigMesh = false;}}
    // if(CONFIG == 3){onceConfigMesh = true;if(onceConfigWifi){LOGLN("Wifi Mode Sconfig");LoRaLooklineSetup();LOGLN("LoRaLooklineSetup");ComMode = LoRa;onceConfigWifi = false;}}
    // if(CONFIG == 0){onceConfigMesh = true;if(onceConfigWifi){LOGLN("Wifi Mode");LoRaLooklineSetup();LOGLN("LoRaLooklineSetup");ComMode = LoRa;onceConfigWifi = false;}}
  
  }

void LOOKLINE_PROG::setup() {
  Serial.begin(112500);
  // Serial2.begin(9600);
  UpdateLookLineData();PinMapInit();
        // if(Debug)LOGLN("Main Data1--" + String(DATA1));//2
        // if(Debug)LOGLN("Main Data2--" + String(DATA2));//15
        // if(Debug)LOGLN("Main Data3--" + String(DATA3));//19
        // if(Debug)LOGLN("Main STCP--" + String(STCP));//26
        // if(Debug)LOGLN("Main SHCP--" + String(SHCP));//25
  SetPin(DATA1, DATA2, DATA3, SHCP, STCP, BoardIDs, NetIDs, Lora_CH, Startus_LED, TimeSent);
  PrintSeg(Seg[0], Seg[0], Seg1[0]);latch();
  #ifdef USE_LORA
  SetPinLoRa( M0,  M1,  16,  17);
  ReadLoRaConfig();
  #endif//USE_LORA
  if(ModuleType != ModGateway){ SetPin7Seg(DATA1, DATA2, DATA3, SHCP, STCP);}
  // #if defined(DEBUG_WIC) && defined(DEBUG_OUTPUT_SERIAL)
  //           // CONFIG::InitBaudrate(DEFAULT_BAUD_RATE);
  //           // delay(2000);
  //           LOG("\r\nDebug Serial set\r\n")
  //           DebugOut("\r\nSet serial baudrate\r\n", OUPUT);
  // #endif
  DebugOut("Initialized.", OUPUT);
          String monitor = "";
            if(ComMode == MESH){ monitor += "Communica: MESH |";}
            if(ComMode == MQTT){ monitor += "Communica: MQTT |";}
            if(ComMode == LoRa){ monitor += "Communica: Lora |";}
            if(ComMode == RS485com){ monitor += "Communica: RS485 |";}

            monitor += "firmware:";
            monitor += UDFWLookLine.FirmwareVer;
            monitor += "\nX0: ";
            monitor += String(analogRead(X0));
            monitor += "  X1:";
            monitor += String(analogRead(X1));
            monitor += "  X2:";
            monitor += String(analogRead(X2));
            monitor += "  X3:";
            monitor += String(analogRead(X3));
            monitor += "  X4:";
            monitor += String(analogRead(X4));
         if(Debug)LOGLN(monitor);
  // String URL_FW = "http://";
  // CONFIG::write_string (EP_EEPROM_URL_FW, URL_FW.c_str() ) ;
  // CONFIG::write_string (EP_EEPROM_URL_VER, URL_FW.c_str() ) ;

  /////// Test
#ifdef TestDisplayIntro
if(ModuleType != ModGateway){  
  for (int i = 0; i < 10; i++)
  {
    // TestDisplay(i);
    displays(i*1111, i*1111, i*1111, i*1111, taskPin.Data1, taskPin.Data2, taskPin.Data3, taskPin.SHCP, taskPin.STCP, 0);latch();

    // #ifdef TEST_MODE
    digitalWrite(Startus_LED, digitalRead(Startus_LED) ^ 1);
    // #endif//TEST_MODE
    delay(300);
  }
caculaOT(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);
}
#endif//not dev
SerDisplay();

  // Serial2.begin(9600);
}
String ConvBinUnits(int bytes, int resolution) {
  if      (bytes < 1024)                 {
    return String(bytes) + " B";
  }
  else if (bytes < 1024 * 1024)          {
    return String((bytes / 1024.0), resolution) + " KB";
  }
  else if (bytes < (1024 * 1024 * 1024)) {
    return String((bytes / 1024.0 / 1024.0), resolution) + " MB";
  }
  else return "";
}
/* ############################ Loop ############################################# */


// #define TEST_LORA


  void LoRaCommu();

  String strIn = "";

void LOOKLINE_PROG::loop()
{
  //  Counterstatus++;if(Counterstatus > 10000){Counterstatus = 0;LookLineOnce1 = true;
  // if(Debug)LOGLN("Status");UpdateLookLineData();
  // }
  if(LookLineOnce){LookLineOnce = false; DebugOut("Lookline Run.", OUPUT);}
  TaskDisplay(DispMode);
  TaskInPut();
    // if (role == NODE || role == REPEARTER ){digitalWrite(Startus_LED, NodeRun);}
  #ifdef ModbusSlave
  modbus_loop();
  #endif//#ifdef ModbusSlave

////////////////////////////////// COMUNICATION //////////////////////////////////
  #ifdef RS485
  if(ComMode == RS485com && Mode > 0){if(ComMasterSlave == false){RS485COM();}}//
  #endif//RS485
  #ifdef MQTT_Mode
  if(ComMode == MQTT){MQTT_loop();}
  #endif//MQTT_Mode
  #ifdef USE_LORA
  if(ComMode == LoRa){LoRaCommu();}
  #endif//LoRa_Network
  #ifdef Mesh_Network
  if(ComMode == MESH){}
  #endif//Mesh_Network

  // digitalWrite(M0, LOW);digitalWrite(M1, LOW);
  if (millis() - OnWifiAutoMillis >= (60000*60)*5 && start == 0 ) { OnWifiAutoMillis = millis();
    
    // LoRaLooklineSetup();
    if(LooklineDebug){LOGLN("Lookline Reset");Serial.flush();}
    // ESP.restart();
  }
  static unsigned long OnWifiAutoMillisReset = 0;
  if (millis() - OnWifiAutoMillisReset > 240000) {OnWifiAutoMillisReset = millis();
    start = 0;
  }

  if (Serial.available()>1) {
    char charin = (char)Serial.read();
    strIn += charin;
    if(strIn == "Wifi"){LOGLN("On Wifi ok!!");
        check_protocol();
        esp_wifi_set_protocol(current_wifi_interface, 3);
        check_protocol();
        CONFIG::write_byte(EP_EEPROM_COM_MODE, LoRa);
        delay(1000);ESP.restart();
        strIn = "";
    }
    
    if(strIn == "RESET"){LOGLN("Reset ok!!");
       CONFIG::reset_config();
       CONFIG::write_buffer (EP_EEPROM_PLAN, (byte *) &PLAN, INTEGER_LENGTH);
       CONFIG::write_buffer (EP_EEPROM_RESULT, (byte *) &RESULT, INTEGER_LENGTH);
       CONFIG::read_buffer(EP_EEPROM_PLAN, (byte *) &PLAN, INTEGER_LENGTH);
       CONFIG::read_buffer(EP_EEPROM_RESULT, (byte *) &RESULT, INTEGER_LENGTH);
       Lookline_PROG.SetResult(0);Lookline_PROG.SetPlan(0);TaskDisplay(DispMode);
       LOGLN("Restarting... Free heap: "+ String(ESP.getFreeHeap()));
       delay(3000);ESP.restart();
    strIn = "";
    }
    if(charin == '\n'){LOG(strIn);strIn = "";}
  }
  /////////////////////////////////// Gateway mode ///////////////////////////
  if(LookLineOnce1){LookLineOnce1 = false; ShowParameters();}
  if(role == GATEWAY && AmountNode > 0 && ModuleType == ModGateway){
    if(Counter4 == 0){
      Counter4++;NodeRun = false;
       if(Debug)LOGLN("Send to node " + String(IDSent));
      if(ModuleType == ModGateway){
      digitalWrite(Startus_LED, HIGH);delay(100);digitalWrite(Startus_LED, LOW);
      }//if(ModuleType == ModGateway){
    #ifdef TEST_LORA
    LoRa_Seri.println("Send to node " + String(IDSent));
    #else
      done = true;
      buffer[4] = '0';buffer[5] = '4';
      buffer[6] = '0';buffer[7] = '0';
      Data_Proccess();
    #endif//  #ifdef USE_LORA
    }
    if(Counter4 >= 6){
      Counter4 = 0;
    }
    if(Counter5 == TimeSent){
      CheckLoss++;if(CheckLoss > 5)CheckLoss = 0;
      IDSent++;if(IDSent > AmountNode){IDSent = 0;}
      Counter4++;
      Counter5 = 0;
    }
    if(Counter5 == (TimeSent/3)){
      IDSent++;if(IDSent > AmountNode){IDSent = 0;}
      Counter4++;
    }
    if(Counter5 == (TimeSent/3) + (TimeSent/3)){
      IDSent++;if(IDSent > AmountNode){IDSent = 0;}
      Counter4++;
    }
  }
///////////////////////////////////////////////////////////////////////////////////
}









  int Data[5][100];
  int LastTime[100];
  int MaxTime[100];

void LOOKLINE_PROG::SetParameter(int Plan, int taskPLanSet, int Result, int ResultSet, int taskTime, int taskpcsInShift, int taskPass, int taskDotIn){
  PLAN = Plan;PLanSet = taskPLanSet;RESULT = Result;ResultSet = ResultSet;Time = taskTime;pcsInShift = taskpcsInShift;Pass = taskPass;DotIn = taskDotIn;
  if(Debug)LOGLN("Set parameters from gateway")
}

#define TempDisplay
struct DataPacket {
  int ID;
  int netId;
  uint8_t data[200];
};
void LOOKLINE_PROG::sendDataLookline() {
  // Tạo struct DataPacket
  DataPacket packet;
  packet.ID = BoardIDs;
  packet.netId = NetIDs;

  // Gán dữ liệu từ DataLookline vào mảng data
  packet.data[0] = (PLAN >> 8) & 0xFF;
  packet.data[1] = PLAN & 0xFF;
  packet.data[2] = (RESULT >> 8) & 0xFF;
  packet.data[3] = RESULT & 0xFF;
  packet.data[4] = NodeRun & 0xFF;
  packet.data[5] = rssi_display;//RSSI & 0xFF;
  packet.data[6] = ComMode & 0xFF;
  packet.data[7] = WiFiMode & 0xFF;
  packet.data[8] = ModuleType & 0xFF;
  packet.data[9] = 0;//Cmd & 0xFF;

  // Điền các giá trị còn lại trong mảng data bằng 0
  for (int i = 10; i < 200; i++) {
      packet.data[i] = 0;
  }

  // Gửi struct qua Serial2
  Serial2.begin(115200, SERIAL_8N1, 17, 16);
  Serial2.write((uint8_t*)&packet, sizeof(packet)); // Gửi toàn bộ struct
  Serial2.flush();

  if (LooklineDebug) {
      LOGLN("DataPacket sent via Mesh");
      // LOG("ID: " + String(packet.ID));
      // LOGLN(" | NetID: " + String(packet.netId));
      // LOGLN("Data: ");
      // for (int i = 0; i < 200; i++) {
      //   LOG(String(packet.data[i]));
      //   LOG(" | ");
      // }
      LOGLN();
  }
}
bool OnceCheck = true;
int totalInterruptCounterLookline = 0; 
void LOOKLINE_PROG::TimerPlanInc()
{
  if(WiFi.status() == WL_CONNECTED && OnceCheck){OnceCheck = false;
    // socket_servers->loop();
    if(UDFWLookLine.FirmwareVersionCheck() == 1){
      String s = "STATUS: New version";
      if(Debug)LOGLN(s);
      looklineWIC.checkFW();
      // socket_servers->broadcastTXT(s);
    }
  }
  // && (ComMode == LoRa || ComMode == MESH || ComMode == MQTT || ComMode == RS485com)
  if(role == GATEWAY){ Counter5++;if(Counter5 > 10){Counter5 = 0;for(int i = 0 ; i < NUM_LOOKLINES ; i++){Looklines[i].Nodecounter++;}  
    String WebData = "GATEWAY: ";
    for (int i = 0; i < Looklines_saved; i++) {
      if(DataLookline.nodeID > 0){
      WebData += String(Looklines[i].nodeID) + ",";
      WebData += String(Looklines[i].networkID) + ",";
      WebData += String(Looklines[i].state) + ",";
      WebData += String(Looklines[i].PLAN) + ",";
      WebData += String(Looklines[i].RESULT) + ",";
      WebData += String(Looklines[i].type) + ",";
      WebData += String(Looklines[i].RSSI) + ",";
      WebData += String(Looklines[i].Com) + ",";
      WebData += String(Looklines[i].WiFi) + ",";
      WebData += String(Looklines[i].Nodecounter);
      if ( i < Looklines_saved - 1) WebData += '\n';
      }
    }
    if(start){socket_server->broadcastTXT(WebData);  //LOGLN("update socketData");
    }
    // ESPCOM::println (WebData, WEB_PIPE);
  }//if(Counter5 > 100){
  }//if(role == GATEWAY){
totalInterruptCounterLookline++;
  ///*
  if (role == NODE || role == REPEARTER )
  { ////LED for Wifi Config
    countLED++;
    if (role == NODE)
    {
      if (countLED > 1 && countLED < 10)
      {
        // digitalWrite(Startus_LED, LOW);
        // digitalWrite(Signal_LED, LOW);
      }
      if (countLED > 10 && countLED < 100)
      {
        // digitalWrite(Startus_LED, HIGH);
        // digitalWrite(Signal_LED, HIGH);
      }
      if (countLED > 100)
      {
        countLED = 0;
      }
      if (countLED % 10 == 9)
      {
        connected(counter3++);
        if (counter3 > 3)
          counter3 = 0;
      }
    }
    if (role == REPEARTER)
    {
      if (countLED > 1000)
      {
        countLED = 0;
        // digitalWrite(Startus_LED, digitalRead(Startus_LED) ^ 1);
        // digitalWrite(Signal_LED, digitalRead(Signal_LED) ^ 1);
      }
    }
  }
  
  if (role == NODE || role == REPEARTER )
  { //////Run/Stop Plan
    if (NodeRun == true)
    {
      digitalWrite(Startus_LED, HIGH);
      counter2++;
      int TimeCacu = 0;
      if (DotIn == 0)
      {
        TimeCacu = (Time*10) * 1;
      }
      if (DotIn == 1)
      {
        TimeCacu = (Time*10) * 10;
      }
      if (DotIn == 2)
      {
        TimeCacu = (Time*10) * 100;
      }
      if (DotIn == 3)
      {
        TimeCacu = (Time*10) * 1000;
      }
      
      if (counter2 >= Time)
      {
        counter2 = 0;
        //digitalWrite(Signal_LED, digitalRead(Signal_LED) ^ 1);
        PLAN = PLAN + PLanSet;//MODBUS_Write();
        caculaOT(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);//SerDisplay();
        if(PLAN % 10 == 9){
                if (!CONFIG::write_buffer (EP_EEPROM_PLAN, (const byte *) &PLAN, INTEGER_LENGTH) ) {
                  LOG("save Plan failed");
                }
           CONFIG::read_buffer(EP_EEPROM_PLAN, (byte *) &PLAN, INTEGER_LENGTH);
        }
        
        if (PLAN > PlanLimit)
        {

          PLAN = PlanLimit;
        }
        //
      }
    }
    else{
      digitalWrite(Startus_LED, LOW);
    }
    countLED++;
    if (countLED > 100)
      {//Monitor
        String MGS = "VALUE:" + String(PLAN) + ":" + String(RESULT) + ":" + String(CountOT_Hm) + "." + String(CountOT_Lm);
        if(start>0){socket_server->broadcastTXT(MGS); //LOG("update socketData");
        }
        if (NodeRun == false){caculaOT(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);SerDisplay();}
        // if(NodeRun){LOG("Runing");}else{LOG("Stopping");} if(Debug)LOGLN(" | Time: "+ String(Time));
        countLED = 0;
      }
  }
    if(totalInterruptCounterLookline >= 100){
      
    // if(Debug)LOGLN("TimerPlanInc | Role:" + String(role));
    SerDisplay();
    if (role == NODE || role == REPEARTER ){sendDataLookline();}
    if(MonitorMode == Main){
    #ifdef TempDisplay
      // SerDisplay();
    #endif// TempDisplay 
    } 
    if(MonitorMode == 1){
     #ifdef MQTT_Mode
      if(ComMode == MQTT && Mode != 3 && countermqtt > 50){  
        #ifdef TempDisplay
        SerDisplay();    
        #endif// TempDisplay 
        }
      #endif//MQTT_Mode  
    }
    totalInterruptCounterLookline = 0;
    }
  //*/
}





void LoRaCommu()
{
   digitalWrite(M0, LOW);digitalWrite(M1, LOW);
  #ifdef USE_LORA
  while (LoRa_Seri.available())
  {    
    char inChar = (char)LoRa_Seri.read();
    LOG(inChar);

        //if (inChar == '\n') {
        if (setupEn == false)
        {
          buffer[countSer] = (char)inChar;
          //LoRaInput += (char)inChar;
          countSer++;
          if (countSer > 50)
          {
            setupEn = true;
            inChar = 0;
            countSer = 0;
          }
        }
    if (inChar == '\n')
      {   
      if(Debug)LOGLN("Done");
      #ifdef TEST_LORA
        // LOG(LoRa_Seri.read()); 
      if (role == NODE || role == REPEARTER )
      { 
        LoRa_Seri.write(LoRa_Seri.read());
      }
      #else//TEST_LORA
        done = true;
        setupEn = true;
        String ids = "";
        ids += buffer[0];
        ids += buffer[1];
        ids += buffer[2];
        ids += buffer[3];
        NodeID = ids.toInt();
        Data_Proccess();
        LoRa_Seri.flush();
        inChar = 0;
        countSer = 0;
        setupEn = false;
    #endif//TEST_LORA
      }
  }
  #endif//LoRa_Network
}

void Data_Proccess()
{
if(done == true){
        done = false;
    String cmds = String(buffer[4]) + String(buffer[5]);
    String Length = String(buffer[6]) + String(buffer[7]);
    // if(Debug)LOGLN("Cmd:" + String(cmds.toInt()));
    // if(Debug)LOGLN("Length:" + String(Length.toInt()));
    if (cmds.toInt() == cmdLookLine.updateStt && (role == NODE || role == REPEARTER))
    {
      if (Length.toInt() == 12)
      {
        if(NodeID == BoardIDs)
        {
          // if(Debug){
            if(Debug)LOGLN("Sent to gateway " );
            // if(ComMode == MQTT){ LOG("  Topic:");if(Debug)LOGLN(String(Lookline_PROG.TopicOut));}
            // else{if(Debug)LOGLN();}
          // }
          // if(Debug){
            #ifdef MQTT_Mode
              if(ComMode == MQTT && Mode != 3){  
                if(countermqtt > 50){
                String sentData = "Sent to gateway  Topic:" + String( TopicOut);
                for(int c = 0 ; c < sentData.length()+2;sentData.toCharArray(Buffer, c++));
                mqtt.publish("/TopicOut",buffer);
                //client.publish(TopicOut,buffer);
                delay(100);
                }
              }
              #endif//MQTT_Mode  
          // }
          String id = "";
          id += String((BoardIDs / 1000) % 10);
          id += String((BoardIDs / 100) % 10);
          id += String((BoardIDs / 10) % 10);
          id += String((BoardIDs / 1) % 10);

          String StringPlan = "";
          StringPlan += (PLAN / 1000) % 10;
          StringPlan += (PLAN / 100) % 10;
          StringPlan += (PLAN / 10) % 10;
          StringPlan += (PLAN / 1) % 10;

          String StringResult = "";
          StringResult += (RESULT / 1000) % 10;
          StringResult += (RESULT / 100) % 10;
          StringResult += (RESULT / 10) % 10;
          StringResult += (RESULT / 1) % 10;
          String State = "";
          if(NodeRun){State ="1" + String(WiFiMode);}else{State = "0" + String(WiFiMode);}
          String sentData = id + "04" + "18" + StringPlan + StringResult + State;
          #ifdef Mesh_Network 
            if(ComMode == MESH && Mode == 0){broadcast(sentData);}
          #endif//Mesh_Network   
          #ifdef RS485
            if(ComMode == RS485com){RS485_Ser.println(sentData);RS485_Ser.flush();}
          #endif//RS485    
          #ifdef USE_LORA 
            if(ComMode == LoRa){
              digitalWrite(M0, LOW);digitalWrite(M1, LOW);
              LoRa_Seri.println(sentData);LoRa_Seri.flush();
              if(Debug)LOGLN(sentData);
            }
          #endif//LoRa_Network  
          #ifdef MQTT_Mode
            if(ComMode == MQTT && countermqtt > 50){  
            for(int c = 0 ; c < sentData.length()+2;sentData.toCharArray(Buffer, c++));
            mqtt.publish(TopicIn,buffer);
            //client.publish(TopicOut,buffer);
            }
          #endif//MQTT_Mode
          // LOG("data Sent:");
          // if(Debug)LOGLN(buffer);
          if(Debug)LOGLN("sent to gateway");
          if(Debug)LOGLN("__________________________________");
        }//if(NodeID == BoardIDs)
        //payload [IDh,IDl,01,16,PlanH,PlanL,ResultH,ResultL,TimeH,TimeL,PlanSH,PlanSL,ResultSH,ResultSL,PassH,PassL,PCSH,PCSL,DOT]
      }//if (Length.toInt() == 12)
      
    }//(cmds.toInt() == cmdLookLine.updateStt)
    if(cmds.toInt() == cmdLookLine.setup)
    {
      if (Length.toInt() == 16){
        if(NodeID == BoardIDs){
           String StringPlan = "";
          StringPlan += (buffer[8] / 1000) % 10;
          StringPlan += (buffer[9] / 100) % 10;
          StringPlan += (buffer[10] / 10) % 10;
          StringPlan += (buffer[11] / 1) % 10;
           PLAN = StringPlan.toInt();
          String StringResult = "";
          StringResult += (buffer[12] / 1000) % 10;
          StringResult += (buffer[13] / 100) % 10;
          StringResult += (buffer[14] / 10) % 10;
          StringResult += (buffer[15] / 1) % 10; 
            RESULT = StringResult.toInt();
          String StringTime = "";
          StringTime += (buffer[16] / 1000) % 10;
          StringTime += (buffer[17] / 100) % 10;
          StringTime += (buffer[18] / 10) % 10;
          StringTime += (buffer[19] / 1) % 10; 
            int taskTime = StringTime.toInt();
          String PlanSets = "";
          PlanSets += (buffer[20] / 1000) % 10;
          PlanSets += (buffer[21] / 100) % 10;
          PlanSets += (buffer[22] / 10) % 10;
          PlanSets += (buffer[23] / 1) % 10; 
           int  taskPLanSet = PlanSets.toInt();
          String ResultSets = "";
          ResultSets += (buffer[24] / 1000) % 10;
          ResultSets += (buffer[25] / 100) % 10;
          ResultSets += (buffer[26] / 10) % 10;
          ResultSets += (buffer[27] / 1) % 10; 
           int  ResultSet = ResultSets.toInt();
          String StringPass = "";
          StringPass += (buffer[28] / 1000) % 10;
          StringPass += (buffer[29] / 100) % 10;
          StringPass += (buffer[30] / 10) % 10;
          StringPass += (buffer[31] / 1) % 10; 
           int  taskPass = StringPass.toInt();
          String PcsShift = "";
          PcsShift += (buffer[32] / 1000) % 10;
          PcsShift += (buffer[33] / 100) % 10;
          PcsShift += (buffer[34] / 10) % 10;
          PcsShift += (buffer[35] / 1) % 10; 
           int  taskpcsInShift = PcsShift.toInt();
          String StringDot = "";
          StringDot += (buffer[36] / 10) % 10;
          StringDot += (buffer[37] / 1) % 10;
           int  taskDotIn = StringDot.toInt();
          Lookline_PROG.SetParameter(PLAN, taskPLanSet, RESULT, ResultSet, taskTime, taskpcsInShift, taskPass, taskDotIn);
            // WriteAll();WriteRebootValue();
        }//if(NodeID == BoardIDs)
      }//if (Length.toInt() == 38)
    }//if(cmds.toInt() == cmdLookLine.setup)
    if (cmds.toInt() == cmdLookLine.updateFw && role == NODE && WiFiMode == true)//For gateway mode
    {
      // Mode = 3;
    }
    if (cmds.toInt() == cmdLookLine.request && role == GATEWAY)//For gateway mode
    {
      if (Length.toInt() == 0)
        {
          String id = "";
          id += String((IDSent / 1000) % 10);
          id += String((IDSent / 100) % 10);
          id += String((IDSent / 10) % 10);
          id += String((IDSent / 1) % 10);
          String sentData = id + "00" + "120000";
          #ifdef Mesh_Network 
            if(ComMode == MESH && Mode == 0) broadcast(sentData);
          #endif//Mesh_Network   
          #ifdef RS485
            if(ComMode == RS485com){RS485_Ser.println(sentData);RS485_Ser.flush();}
          #endif//RS485    
          #ifdef USE_LORA 
            if(ComMode == LoRa){digitalWrite(M0, LOW);digitalWrite(M1, LOW);
            LoRa_Seri.println(sentData);LoRa_Seri.flush();
            // if(Debug)LOGLN(sentData);
            setupEn = false;
            }
          #endif//LoRa_Network  
          #ifdef MQTT_Mode
            if(ComMode == MQTT && countermqtt > 50){  
            for(byte c = 0 ; c < sentData.length()+2;sentData.toCharArray(Buffer, c++));
            mqtt.publish(TopicOut,buffer);
            }
          #endif//MQTT_Mode
        } //if (Length.toInt() == 0)
      if (Length.toInt() >= 16)
      {
        CONFIG::read_byte(EP_EEPROM_MODULE_TYPE, &ModuleType);
        if(ModuleType == ModGateway){
        digitalWrite(taskStatus_LED, HIGH);delay(100);digitalWrite(taskStatus_LED, LOW);
        }//if(taskModuleType == ModGateway){
        String ID = String(buffer[0]) + String(buffer[1]) + String(buffer[2]) + String(buffer[3]); 
          if(ID.toInt() == IDSent){
            PC_Seri.println(String(buffer));counter5 = TimeSent;delay(300);counter5 = TimeSent - 2;
            String plan = String(buffer[8]) + String(buffer[9]) + String(buffer[10]) + String(buffer[11]);
            String result = String(buffer[12]) + String(buffer[13]) + String(buffer[14]) + String(buffer[15]);
            String Runs = String(buffer[16]);
            String Modes = String(buffer[17]);
            Data[0][IDSent] = plan.toInt();
            Data[1][IDSent] = result.toInt();
            Data[3][IDSent] = Runs.toInt();
            Data[4][IDSent] = Modes.toInt();
            //Data[3][IDSent] = 0;//taskLastTime[IDSent];
            taskLastTime[IDSent] = 0;
          }
      }//if (Length.toInt() == 16)      
    }//if (cmds.toInt() == cmdLookLine.request)
  }//Done = true
  
}

void LOOKLINE_PROG::SerDisplay()
{
      if(MonitorMode == Main){
        CONFIG::read_byte(EP_EEPROM_ID, &BoardIDs);
        CONFIG::read_byte(EP_EEPROM_NETID, &NetIDs);
        CONFIG::read_byte(EP_EEPROM_CHANELS, &Lora_CH);
        // CONFIG::read_byte(EP_EEPROM_RUN, &NodeRun);
        CONFIG::read_byte(EP_EEPROM_DEBUG, &Debug);
        CONFIG::read_byte(EP_EEPROM_AMOUNTNODE, &AmountNode);
        CONFIG::read_byte(EP_EEPROM_TIMESENT, &TimeSent);
        CONFIG::read_byte(EP_EEPROM_MODULE_TYPE, &ModuleType);
        CONFIG::read_byte(EP_EEPROM_ROLE, &role);
        CONFIG::read_buffer(EP_EEPROM_TIME_PLAN,(byte*) &Time, INTEGER_LENGTH);
        String Log = "";
        Log = "| Board ID: "+ String(BoardIDs);
        Log +=" | Network ID: "+ String(NetIDs);
        Log +=" | Chanel : "+ String(Lora_CH);
        Log += "| M0:" +  String(M0) + "(" + String(digitalRead(M0)) + ")| M1:" +  String(M1) +  "(" + String(digitalRead(M1)) + ")";
        if(role == NODE || role == REPEARTER){
          if(NodeRun){Log +="| Runing";}else{Log +="| Stopping";} 
          Log +=" | Time: "+ String(Time) + "| ";
          if(DispMode == Test){Log +="Disp: Test ";}
          if(DispMode == Setting_){Log +="Disp: Setting ";}
          if(DispMode == ConFi){Log +="Disp: ConFi ";}
          if(DispMode == Online){Log +="Disp: Online ";}
          if(DispMode == SLEEP){Log +="Disp: SLEEP ";}
          if(DispMode == CLEAR){Log +="Disp: Clear ";}
          Log +="\nPlan: ";Log +=PLAN;
          Log +="|  Result: ";Log +=RESULT;
          Log +="|  O.T: ";Log +=CountOT_Hm;Log +="."; Log +=CountOT_Lm;   
          Log +="\n Startus_LED pin:" + String(Startus_LED) + "(" + digitalRead(Startus_LED) + ")";
        } else{
          Log += "| Amount node: " + String(AmountNode);
        }
        Log +="\nTemp: ";
        Log +=(temprature_sens_read() - 32) / 1.8;
        Log +=" C |";
        LOG("WiFi Status: " + String(WiFi.status()));
        LOG(" | RSSI: " + String(WiFi.RSSI()));
        LOGLN(" | Free heap: " + String(ESP.getFreeHeap()));
        unsigned long runtimeMillis = millis();
        unsigned long seconds = (runtimeMillis / 1000) % 60;
        unsigned long minutes = (runtimeMillis / (1000 * 60)) % 60;
        unsigned long hours = (runtimeMillis / (1000 * 60 * 60)) % 24;
        unsigned long days = runtimeMillis / (1000 * 60 * 60 * 24);

        Log += "\nRuntime: ";
        Log += String(days) + "d ";
        Log += String(hours) + "h ";
        Log += String(minutes) + "m ";
        Log += String(seconds) + "s |";
        Log += " Start:" + String(start);
        Log += " | TimeReset:" + String(millis() - OnWifiAutoMillis );
        if(Debug)LOGLN(Log);
        // ShowParameters();
      }
      // if(MonitorMode == 1){
      //   String strmoni = "";
      //   if(DisplayMode == Main){strmoni = "Disp:Main";}
      //   if(DisplayMode == Test){strmoni = "Disp: Test |";}
      //   if(DisplayMode == Setting){strmoni = "Disp: Setting |";}
      //   if(DisplayMode == ConFi){strmoni = "Disp: ConFi |";}
      //   if(DisplayMode == Online){strmoni = "Disp: Online |";}
      //   if(DisplayMode == SLEEP){strmoni = "Disp: SLEEP |";}
      //   if(DisplayMode == CLEAR){strmoni = "Disp: Clear |";}
      //   for(int c = 0 ; c < strmoni.length();strmoni.toCharArray(Buffer, c++));
      //   mqtt.publish("/TopicOut", Buffer );delay(100);
      //   strmoni =  "Plan:  " + String(Plan);
      //   strmoni += " |Result:  " + String(Result);
      //   strmoni += " |O.T:  " + String((CountOT_H *100)+ CountOT_L);
      //   strmoni += " |Temp: " + String((temprature_sens_read() - 32) / 1.8) + "*C"; 
      //   for(int c = 0 ; c < strmoni.length();strmoni.toCharArray(Buffer, c++));
      //   mqtt.publish("/TopicOut", Buffer );
  
      // }
      String MSGs = "VALUE:" + String(PLAN) + ":" + String(RESULT) + ":" + String(CountOT_Hm) + "." + String(CountOT_Lm);
      if(start){socket_server->broadcastTXT(MSGs); //LOG("update socketData"); 
      }
}

void LOOKLINE_PROG::ConfigJsonProcess(String Input)
{ 
  /*
   int SetupForBegin = 0;
  long PlanLimit =  9999;
  long Plan =       0;
  long Result =     0;
  long ActionOT =   0;
  //long ResultCon = 0;
  long PLanSet =    1;//boi so Plan
  long ResultSet =  1;//boi so Result
  long pcsInShift = 1;//số sản phẩm chạy theo Plan
  long Time = 10;
  int DotIn = 0;
  bool NodeRun = true;
  bool lock = true;//IR Lock mode
  */
  DynamicJsonDocument doc(1500);
  deserializeJson(doc, Input);
  JsonObject obj = doc.as<JsonObject>();
  //{"Command":"Config","Run":1,"time":10,"Pla":1,"Res":0,"PSet":1,"RSet":1,"pass":0,"DotI":0,"PlL":9999,"PCS":10,"ID":1,"TS":10,"MM":0,"CMS":0,"QN":5,"GT":1,"STAN":1,"ILCH":410,"DFC":1000}

  if(obj["Command"].as<String>() == "Config"){ 
      if(Debug)LOGLN("Config Readding");
      Time = obj["time"].as<String>().toInt();
      PLanSet = obj["PSet"].as<String>().toInt();
      ResultSet = obj["RSet"].as<String>().toInt();
      Pass1 =  obj["pass"].as<String>().toInt();
      DotIn = obj["DotI"].as<String>().toInt();
      PlanLimit = obj["PlL"].as<String>().toInt();
      pcsInShift = obj["PCS"].as<String>().toInt(); 
      //BoardIDs = obj["ID"].as<String>().toInt();
      TimeSent = obj["TS"].as<String>().toInt();
      MonitorMode = obj["MM"].as<String>().toInt();
      ComMasterSlave = obj["CMS"].as<String>().toInt();
      AmountNode = obj["QN"].as<String>().toInt();
      GatewayTerminal = obj["GT"].as<String>().toInt();
      STAModeNormal = obj["STAN"].as<String>().toInt();
      Lora_CH = obj["ILCH"].as<String>().toInt();
      delayForCounter = obj["DFC"].as<String>().toInt();
      Pass3 =  obj["pass1"].as<String>().toInt();
  }// if(obj["Command"].as<String>() == "Config"){


}//void ConfigJsonProcess(String Input)

//////////////////////////////////////////////////////////////// GATEWAY MONITOR FUNCTIONS////////////////////////////////

void saveLooklineData(byte saveRSSI,byte saveID,byte saveNetID,byte saveState,int savePlan,int saveResult,byte Type,byte saveCom,byte saveWifi) {//Save Web

  new_Lookline_found = false;
  String msg = "Save Lookline " + String(saveID);
   if(LooklineDebug)ESPCOM::println(msg, PRINTER_PIPE);
  for (int i = 0; i < Looklines_saved; i++) {
    if (Looklines[i].nodeID == saveID){//LOGLN(saveID);
      Looklines[i].Nodecounter = 0;
      Looklines[i].nodeID = saveID;
      Looklines[i].networkID = saveNetID;
      Looklines[i].state = saveState;
      Looklines[i].PLAN = savePlan;
      Looklines[i].RESULT = saveResult;
      Looklines[i].Com = saveCom;
      Looklines[i].WiFi = saveWifi;
      Looklines[i].RSSI = saveRSSI;
      Looklines[i].type = Type;
      Looklines[i].Cmd = saveRSSI;
      //Looklines[i].timestamp = time(nullptr);
      // if(WiFi.getMode() == WIFI_STA){Looklines[i].time = timeClient.getEpochTime();}else{Looklines[Looklines_saved].time = 0;}
      new_Lookline_found = true;
    }
  }

  if ( new_Lookline_found == false ) {
      msg = "New Lookline " + String(saveID);
       if(LooklineDebug)ESPCOM::println(msg, PRINTER_PIPE);
      Looklines[Looklines_saved].Nodecounter = 0;
      Looklines[Looklines_saved].nodeID = saveID;
      Looklines[Looklines_saved].networkID = saveNetID;
      Looklines[Looklines_saved].state = saveState;
      Looklines[Looklines_saved].PLAN = savePlan;
      Looklines[Looklines_saved].RESULT = saveResult;
      Looklines[Looklines_saved].Com = saveCom;
      Looklines[Looklines_saved].WiFi = saveWifi;
      Looklines[Looklines_saved].RSSI = saveRSSI;
      Looklines[Looklines_saved].type = Type;
      Looklines[Looklines_saved].Cmd = saveRSSI;
      //Looklines[Looklines_saved].timestamp = time(nullptr);
      // if(WiFi.getMode() == WIFI_STA){Looklines[Looklines_saved].time = timeClient.getEpochTime();}else{Looklines[Looklines_saved].time = 0;}
    Looklines_saved++;
  }

#ifdef SDCARD_FEATURE
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
  #endif //SDCARD_FEATURE
  // Looklines[ID].GateWayCommand = SleepCmd;
  // for(byte l = 0 ; l < 20 ; l++){debug("ID:" + String(l) + "|" + String(Looklines[l].GateWayCommand) + "| ");}
}
#endif//LOOKLINE_UI