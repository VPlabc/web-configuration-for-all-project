//ERROR:220723
//slave: Counter failed , Delay counter not working ->> Fixed error
//master: time inc error ->> Fixed error
////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include "config.h"
#ifdef LOOKLINE_UI



// #include "wificonf.h"

// #include "webinterface.h"
#include "command.h"
// ESPResponseStream espresponses;
#include "espcom.h"

#include "WIC.h"
WIC looklineWIC;
#include "webinterface.h"

#include "wificonf.h"
WIFI_CONFIG WifiConFig;

#include "LookLine.h"
 Command cmdLookLine;
 LOOKLINE_PROG Lookline_PROG;
#include "7SegModule.h"
#include "FirmwareUpdate.h"
UpdateFW UDFWLookLine;
#include "syncwebserver.h"
WebSocketsServer * socket_servers;
#ifdef ModbusCom
#include "Modbus_RTU.h"
Modbus_Prog LLModbusCom;
#endif//ModbusCom 
#include "webinterface.h"
#ifdef CAPTIVE_PORTAL_FEATURE
#include <DNSServer.h>
DNSServer LooklinednsServer;
const byte DNS_PORT = 53;
#endif
#ifdef USE_LORA
    #include "LoRa_E32.h"
#endif//USE_LORA
#ifdef MQTT_Mode
    #include <PubSubClient.h>
    WiFiClient client;
    PubSubClient mqtt(client);
#endif//MQTT_Mode

#define BootButton 0

byte DispMode = Main;
byte start = 0;
bool config = 0;
bool newFWdetec = false;
byte NodeRun = 1;
byte role = 0;
bool Reciver = false;
extern byte LooklineDebug = true;

int CountOT_m=0;
int CountOT_Hm=0;
int CountOT_Lm=0;
int PLAN = 0;
int PLAN_ = 0;
int RESULT = 0;
int PLanSet =    1;//boi so Plan
int ResultSet =  1;//boi so Result
int pcsInShift = 1;//số sản phẩm chạy theo Plan
int PlanLimit =  9999;
byte ModuleType=0;
int delayForCounter = 1000;
#ifdef MASTER_MODBUS
int mtBoardID = 0;
int mtNetID = 0;
int mtPLAN=0;
int mtRESULT=0;
int mtPLanSet =    1;//boi so Plan
int mtResultSet =  1;//boi so Result
int mtpcsInShift = 1;//số sản phẩm chạy theo Plan
int mtPlanLimit =  9999;
byte mtModuleType = 0;
byte mtComMode = 0;
byte mtWiFiMode = 0;
int mtTime = 0;
bool mtNodeRun = false;
byte mtRole = 0;
byte mtDispMode = 0;
int mtdelayForCounter = 1000;

int mtGetBoardID = 0;
int mtGetNetID = 0;
int mtGetPLAN=0;
int mtGetRESULT=0;
int mtGetPLanSet =    1;//boi so Plan
int mtGetResultSet =  1;//boi so Result
int mtGetpcsInShift = 1;//số sản phẩm chạy theo Plan
int mtGetPlanLimit =  9999;
byte mtGetModuleType = 0;
byte mtGetComMode = 0;
byte mtGetWiFiMode = 0;
int mtGetTime = 0;
bool mtGetNodeRun = false;
byte mtGetRole = 0;
byte mtGetDispMode = 0;
int mtGetdelayForCounter = 1000;

#endif//MASTER_MODBUS
byte AmountNode = 0;
byte CheckLoss = 0;
  bool setupEn = false;
  bool SetClear = false;
  bool SetWifi = false;
extern  int NodeID =0;
bool UpdateParamOnce = false;

  bool MeshRun = false;  
  bool ReSent = 1;

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
  byte TEST = 0;
  int DisplayMode = Main;
  byte ComMode = LoRa;
  byte WiFiMode = 0;
  int counter2 = 0;//nhap nhay
  int counter3 = 0;//connected 
  int Counter4 = 0;//GatewayTimeoutSend 
  int Counter5 = 0;//GatewayTimeoutfeedback 
  int Counterstatus = 10000;//GatewayTimeout status
  byte countSer;
bool done;
  int IDSent = 0;
char buffer[250];
bool LookLineOnce = true;
bool LookLineOnce1 = true;
  int Time = 10;

String SettingName[17] = {
    "NodeID",\
    "NetworkID",\
    "Runstate",\
    "Display mode",\
    "Plan",\
    "PlanSet", \
    "Result", \
    "ResultSet", \
    "Planlimit",\
    "Pcs/h", \
    "Time",\
    "Delayforcounter",\
    "Role",\
    "Rssi",\
    "Commode",\
    "Moduletype",\
    "Wifimode"\
    };
//////////////// registers of your slave ///////////////////
enum 
{     
  BOARDID,
  NETID,
  RUNSTOP,
  ONOFF,
  _PLAN,
  PLANSET,
  _RESULT,
  RESULTSET,
  MAXPLAN,
  PCS,
  TIMEINC,
  DELAYCOUNTER,
  ROLE,
  _RSSI,  
  COMMODE,
  TYPE,
  ONWIFI,
  CMD,   
  HOLDING_REGS_SIZE // leave this one
};
int ROM_Address[] ={ 
  EP_EEPROM_ID ,
  EP_EEPROM_NETID ,
  EP_EEPROM_RUN , 
  EP_EEPROM_ON_OFF ,
  EP_EEPROM_PLAN , 
  EP_EEPROM_PLAN_SET,
  EP_EEPROM_RESULT ,
  EP_EEPROM_RESULT_SET , 
  EP_EEPROM_PLANMAX, 
  EP_EEPROM_PCS ,
  EP_EEPROM_TIME_PLAN , 
  EP_EEPROM_COUNTER_DELAY , 
  EP_EEPROM_ROLE, 
  0 , 
  EP_EEPROM_COM_MODE ,
  EP_EEPROM_MODULE_TYPE,  
  0 , 
  0
  };

int SlaveParameter[HOLDING_REGS_SIZE];
int HOLDING_REGS_ReadData[HOLDING_REGS_SIZE];
int HOLDING_REGS_WriteData[HOLDING_REGS_SIZE];
//#include "MeshNetwork.h"
#include "MQTT.h"
#include "LoRa.h"
// #define LoRa_Seri Serial2
#define PC_Seri Serial
#include "Task_Prog.h"
TaskPin taskPins;
/////////////////////////////////////////////////////////////////
#include <ClickButton.h>
ClickButton button(BootButton, LOW, CLICKBTN_PULLUP);


struct_command_message DataCommand;
struct_Parameter_message DataLookline;

bool new_Lookline_found = false;
// struct_message  msg1;
struct_Parameter_message  Looklines[NUM_LOOKLINES];
//SDFunction::user_setting={};
int Looklines_saved = 0;
struct_Config_message DataConfig;

bool Starting = true;

void saveLooklineData(byte saveRSSI,byte saveID,byte saveNetID,byte saveState,int savePlan,int saveResult,byte Type,byte saveCom,byte saveWifi) ;


#ifdef ESP32
 #ifdef __cplusplus
  extern "C" {
 #endif
  uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
#else 
  // uint8_t temprature_sens_read();
#endif//ESP32

#include "esp_wifi.h"
uint8_t current_protocol;
esp_interface_t current_esp_interface;
wifi_interface_t current_wifi_interface;

int check_protocol()
{
    char error_buf1[100];
  if(LooklineDebug){
    LOGLN();
    LOGLN("___________________________________");
    LOGLN();
  }
     esp_err_t error_code = esp_wifi_get_protocol(current_wifi_interface, &current_protocol);
     esp_err_to_name_r(error_code,error_buf1,100);
  if(LooklineDebug){
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


#ifdef Mesh_Network
////////////////////// MESH
/////////////////////////////////////////////////////////////////
#include <WiFi.h>

#include <esp_now.h>
void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength);
void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen);
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status);
void broadcast(const String &message);
void MeshSetup(void);
void MeshLoop(void);

//char buffer[ESP_NOW_MAX_DATA_LEN + 1];


esp_now_peer_info_t peerInfo;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   // SENSOR MAC`
int rssi_display;
// Estructuras para calcular los paquetes, el RSSI, etc
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
  // const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  int rssi = ppkt->rx_ctrl.rssi;
  rssi_display = 120 + rssi;
}
  char bufferMesh[250];
  bool Meshdone;
void broadcast(const String &message);
void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
{
  snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}
void OnLookLineDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(LooklineDebug){
  // LOG("\r\nLast Packet Send Status:\t");
  // LOGLN(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  
  }
}

bool readOnce =  1;
void MeshLookLineRecive(const uint8_t *macAddr, const uint8_t *data, int dataLen)
{
  MeshRun = true;
  // only allow a maximum of 250 characters in the message + a null terminating byte
      // ESPCOM::println("Reciver Data", PRINTER_PIPE);
  
  int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
  strncpy(buffer, (const char *)data, msgLen);
  // make sure we are null terminated
  buffer[msgLen] = 0;
  // format the mac address
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  // debug log the message to the serial port
  // LOGLN("Received message from:" + String(macStr));
  // what are our instructions
    done = true;
    String ids = "";
      ids += buffer[0];
      ids += buffer[1];
      ids += buffer[2];
      ids += buffer[3];
      NodeID = ids.toInt();
      //  LOG("Mesh revice | ID:");LOGLN(NodeID);
      // Data_Proccess(buffer);
    // if(LooklineDebug)LOGLN("Mesh revice LookLine");
  if(sizeof(DataLookline) == msgLen && role == GATEWAY){
    memcpy(&DataLookline, data, sizeof(DataLookline));
    // ESPCOM::println("NetID: " +String(DataLookline.networkID), PRINTER_PIPE);
    // ESPCOM::println("ID: " +String(DataLookline.nodeID), PRINTER_PIPE);
    
    String id = "";
      id += String((DataLookline.nodeID / 1000) % 10);
      id += String((DataLookline.nodeID / 100) % 10);
      id += String((DataLookline.nodeID / 10) % 10);
      id += String((DataLookline.nodeID / 1) % 10);

      String StringPlan = "";
      StringPlan += (DataLookline.PLAN / 1000) % 10;
      StringPlan += (DataLookline.PLAN / 100) % 10;
      StringPlan += (DataLookline.PLAN / 10) % 10;
      StringPlan += (DataLookline.PLAN / 1) % 10;

      String StringResult = "";
      StringResult += (DataLookline.RESULT / 1000) % 10;
      StringResult += (DataLookline.RESULT / 100) % 10;
      StringResult += (DataLookline.RESULT / 10) % 10;
      StringResult += (DataLookline.RESULT / 1) % 10;
      String State = "";
      if(DataLookline.state){State ="1" + String(WiFiMode);}else{State = "0" + String(WiFiMode);}
      String sentData = id + "04" + "18" + StringPlan + StringResult + State;
      delay(100);
 
      // LOGLN("Mesh recived " + sentData);
      if(readOnce){ CONFIG::read_byte(EP_EEPROM_ID, &BoardIDs);}
      if(readOnce){ readOnce = false;CONFIG::read_byte(EP_EEPROM_NETID, &NetIDs);}
      if(NetIDs > 254){CONFIG::read_byte(EP_EEPROM_NETID, &NetIDs);}
      // ESPCOM::println("Gateway ID: " +String(BoardIDs), PRINTER_PIPE);

    if(DataLookline.networkID == NetIDs){
      digitalWrite(taskStatus_LED, LOW);delay(50);
      PC_Seri.println(sentData);
      String DataString = "Data Recivered: ID:"+String(DataLookline.nodeID);
      // DataString += "|Net:"+String(DataLookline.networkID);
      // DataString += "|RSSI:"+String(DataLookline.RSSI);
      // DataString += "|COM:"+String(DataLookline.Com);
      // DataString += "|Type:"+String(DataLookline.type);
      // DataString += "|Wifi:"+String(DataLookline.WiFi);
      // LOGLN(DataString);
      saveLooklineData(DataLookline.RSSI,DataLookline.nodeID,DataLookline.networkID,DataLookline.state,DataLookline.PLAN,DataLookline.RESULT,DataLookline.type,DataLookline.Com,DataLookline.WiFi);
       
        #ifdef SLAVE_MODBUS      
          HOLDING_REGS_ReadData[BOARDID] = DataLookline.nodeID;
          HOLDING_REGS_ReadData[NETID] = DataLookline.networkID;
          HOLDING_REGS_ReadData[RUNSTOP] =DataLookline.state;
          HOLDING_REGS_ReadData[ONOFF] = 1;
          HOLDING_REGS_ReadData[_PLAN] = DataLookline.PLAN;
          HOLDING_REGS_ReadData[PLANSET] = 1;
          HOLDING_REGS_ReadData[_RESULT] = DataLookline.RESULT;
          HOLDING_REGS_ReadData[RESULTSET] = 1;
          HOLDING_REGS_ReadData[MAXPLAN] = 0;
          HOLDING_REGS_ReadData[PCS] = 0;
          HOLDING_REGS_ReadData[TIMEINC] = 0;
          HOLDING_REGS_ReadData[DELAYCOUNTER] =  0;
          HOLDING_REGS_ReadData[ROLE] =  0;
          HOLDING_REGS_ReadData[_RSSI] =  rssi_display;//DataLookline.RSSI;
          HOLDING_REGS_ReadData[COMMODE] = DataLookline.Com;
          HOLDING_REGS_ReadData[TYPE] =  DataLookline.type;
          HOLDING_REGS_ReadData[ONWIFI] =  DataLookline.WiFi;
          HOLDING_REGS_ReadData[CMD] =  0;  
        // LOG("Read: ");for(byte i=0;i < HOLDING_REGS_SIZE; i++){LOG("|");LOG(String(HOLDING_REGS_ReadData[i]));}LOGLN();
        #ifdef ModbusCom
          LLModbusCom.modbus_read_update(HOLDING_REGS_ReadData);UpdateParamOnce = true;
        #endif//SLAVE_MODBUS
        #endif//SLAVE_MODBUS
      if(role == GATEWAY){     
        if(start == 2){ 
          String WebData = "GATEWAY: ";
          for (int i = 0; i < Looklines_saved; i++) {
            if(DataLookline.nodeID >= 0){
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
          socket_server->broadcastTXT(WebData);
        }
      digitalWrite(taskStatus_LED, HIGH);
      DataLookline.nodeID = DataLookline.nodeID;
      DataLookline.networkID = NetIDs;
      DataLookline.Cmd = OKcmd;

      esp_now_send(broadcastAddress, (uint8_t *) &DataLookline, sizeof(DataLookline));
      // esp_now_send(broadcastAddress, (const uint8_t *)Log.c_str(), Log.length());
      pinMode(27, OUTPUT);  
      digitalWrite(27, LOW);delay(100);digitalWrite(27, HIGH);
      }
    }  
    else{
      if(LooklineDebug) LOGLN();LOGLN("Wrong network");
      LOGLN("Network ID: " + String(DataLookline.networkID));
      LOGLN("GW Network ID: " + String(NetIDs));
    }
  }
  
  if(sizeof(DataLookline) == msgLen && (role == NODE || role == REPEARTER)){
    memcpy(&DataLookline, data, sizeof(DataLookline));
    if(LooklineDebug)LOGLN("Mesh revice LookLine:" + String(DataLookline.nodeID));
    if(DataLookline.nodeID  == BoardIDs && DataLookline.networkID == NetIDs){
      LOG("Recive Lookline ID:" + String(DataLookline.nodeID ));
      if(DataLookline.Cmd == OKcmd){LOGLN(" OK");ReSent = false;}
      if(DataLookline.Cmd == ONWIFIcmd){LOGLN(" ON Wifi");Lookline_PROG.SetConfig(0);}
      if(DataLookline.Cmd == UPDATEcmd){LOGLN(" UPDATE");
        PLAN = DataLookline.PLAN;
        RESULT = DataLookline.RESULT;
        NodeRun = DataLookline.state;
        Time = DataLookline.Nodecounter;

      }
    }
  }
}





void MeshLookLineSetup()
{
  #ifdef Mesh_Network
   // Init ESP-NOW
  WiFi.mode(WIFI_STA);
  #ifdef ARDUINO_ARCH_ESP8266
  #else
  if(check_protocol() != 8){
  esp_wifi_set_protocol(current_wifi_interface, WIFI_PROTOCOL_LR);
  check_protocol();
  }
  #endif//#ifdef ARDUINO_ARCH_ESP8266

  if (esp_now_init() != ESP_OK) {
    if(LooklineDebug)LOGLN("Error initializing ESP-NOW");
    return;
  }
  else{
    if(LooklineDebug)LOGLN("initializing ESP-NOW OK");
  }
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnLookLineDataSent);
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    if(LooklineDebug)LOGLN("Failed to add peer");
    return;
  }
  else{
    if(LooklineDebug)LOGLN("add peer OK");
  } 
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(MeshLookLineRecive);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
  #endif//
  
}
void broadcast(const String &message){
  esp_now_send(broadcastAddress, (const uint8_t *)message.c_str(), message.length());
}
#endif//  #ifdef Mesh_Network

void LoRaLooklineSetup()
{
  looklineWIC.SetWifiMode(0);
  #ifdef Mesh_Network
    check_protocol();
    esp_wifi_set_protocol(current_wifi_interface, 6);
    check_protocol();
  #endif// #ifdef Mesh_Network
    WiFi.disconnect();WiFi.mode(WIFI_OFF);
    // WiFi.mode(WIFI_AP_STA); 

    // wifi_config.Safe_Setup();
    // if(SetupPortal() == false){LOGLN("SetupPortal failed");}

if (!wifi_config.Setup())
  {
       // try again in AP mode
    ESPCOM::println(F("Safe mode 1"), PRINTER_PIPE);
          // LOGLN("Safe mode 1");
    if (!wifi_config.Setup(true))
    {
                  wifi_config.Safe_Setup();
        // LOGLN("Safe mode 2");
        ESPCOM::println(F("Safe mode 2"), PRINTER_PIPE);
    }
  }

    // CONFIG::write_byte(EP_EEPROM_COM_MODE, LoRa);

 }
 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// extern bool modbusUpdateOnce = true;
/////////////////////////////////////////////////////////////////////////////////
#ifdef MASTER_MODBUS
void LOOKLINE_PROG::lookline_modbus_update(int RBoardID,int RNetID,bool RunStop,bool OnOff,int Plan, int PlanSet, int RResult,int RResultSet,int RPlanLimit,int Pcs,int TimeInc,int RComMode,int Type,int RRole,int OnWifi,int DelayCounter) {
  mtBoardID = RBoardID;
  mtNetID = RNetID;
  mtNodeRun = RunStop;
  DisplayMode = OnOff;
  mtPLAN = Plan;
  mtPLanSet = PlanSet;
  mtRESULT = RResult;
  mtResultSet = RResultSet;
  mtPlanLimit = RPlanLimit;
  mtpcsInShift = Pcs;
  mtTime = TimeInc;
  mtComMode = RComMode;
  mtModuleType = Type;
  mtRole = RRole;
  mtWiFiMode = OnWifi;
  mtdelayForCounter = DelayCounter;
    // Serial.println();
    // Serial.println("Update from Modbus");
    // Serial.println();
    if(role == GATEWAY){
      #ifdef MASTER_MODBUS
          saveLooklineData(RPlanLimit,RBoardID,RNetID,RunStop,Plan,RResult,Type,RComMode,OnWifi);
      #endif//MASTER_MODBUS
      if(start == 2){      
            String WebData = "GATEWAY:";
            for (int i = 0; i < Looklines_saved; i++) {
              if(DataLookline.nodeID >= 0){
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
            socket_server->broadcastTXT(WebData);
            LOGLN("update to web");
          }
      }
} 
void LOOKLINE_PROG::lookline_modbus_get(int RBoardID,int RNetID,bool RunStop,bool OnOff,int Plan, int PlanSet, int RResult,int RResultSet,int RPlanLimit,int Pcs,int TimeInc,int RComMode,int Type,int RRole,int OnWifi,int DelayCounter) {
  mtGetBoardID = RBoardID;
  mtGetNetID = RNetID;
  mtGetNodeRun = RunStop;
  DisplayMode = OnOff;
  mtGetPLAN = Plan;
  mtGetPLanSet = PlanSet;
  mtGetRESULT = RResult;
  mtGetResultSet = RResultSet;
  mtGetPlanLimit = RPlanLimit;
  mtGetpcsInShift = Pcs;
  mtGetTime = TimeInc;
  mtGetComMode = RComMode;
  mtGetModuleType = Type;
  mtGetRole = RRole;
  mtGetWiFiMode = OnWifi;
  mtGetdelayForCounter = DelayCounter;
  // Serial.println("Update from Modbus");
} 
#endif//MASTER_MODBUS
bool Updateonce = false;
#ifdef ModbusCom
void LOOKLINE_PROG::slave_modbus_update(int HOLDING_REGS_Data[]){
  if(HOLDING_REGS_Data[CMD] == 1 && UpdateParamOnce == true){
  UpdateParamOnce = false;
    // LOGLN("feedback message");
    // LOG("Parameter Read: ");for(byte i=0;i < HOLDING_REGS_SIZE; i++){LOG("|");LOG(String(HOLDING_REGS_Data[i]));}LOGLN();
    LLModbusCom.modbus_write_setParameter(HOLDING_REGS_SIZE + 1, 0, 0);//Feedback
  }
}

void LOOKLINE_PROG::slave_modbus_get(int HOLDING_REGS_Data[]) {

 if(HOLDING_REGS_Data[CMD] == 4 && Updateonce){Updateonce = false;
    UpdateLookLineData();
    LLModbusCom.modbus_write_setParameter(HOLDING_REGS_SIZE + 1, 0, 0);//Feedback
 }

 if(HOLDING_REGS_Data[CMD] == 3 && Updateonce){Updateonce = false;
    int zero = 0;
    CONFIG::write_buffer (EP_EEPROM_PLAN, (byte*) &zero, INTEGER_LENGTH);
    CONFIG::write_buffer (EP_EEPROM_RESULT, (byte*) &zero, INTEGER_LENGTH);
    UpdateLookLineData();
    LLModbusCom.modbus_write_setParameter(HOLDING_REGS_SIZE + 1, 0, 0);//Feedback
 }

if(HOLDING_REGS_Data[CMD] == 2 && Updateonce == false){Updateonce = true;
  HOLDING_REGS_ReadData[CMD] = 2;LLModbusCom.modbus_read_update(HOLDING_REGS_ReadData);
}
if(HOLDING_REGS_Data[CMD] == 1 && Updateonce){//Cmd = 1 get data
  Updateonce = false;
  LOGLN("Update Slave Parameter");
  LOG("Data Read: ");for(byte i=0;i < HOLDING_REGS_SIZE; i++){LOG(String(SlaveParameter[i]));LOG("|");}LOGLN();
  LOG("Parameter Read: ");for(byte i=0;i < HOLDING_REGS_SIZE; i++){LOG("|");LOG(String(HOLDING_REGS_Data[i]));
  }LOGLN();LOGLN();

  byte RomAddress = HOLDING_REGS_Data[_RSSI];
   SlaveParameter[RomAddress] = HOLDING_REGS_Data[RomAddress];
   LOGLN("parameter: " + SettingName[RomAddress] + "| Address: " + String(ROM_Address[RomAddress]));
    if((RomAddress>= 0 && RomAddress<= ONOFF || (RomAddress>= ROLE && RomAddress<= ONWIFI))){
      CONFIG::write_byte( ROM_Address[RomAddress], HOLDING_REGS_Data[RomAddress]);
    }
    if(RomAddress> ONOFF && RomAddress< ROLE){
      CONFIG::write_buffer( ROM_Address[RomAddress], (const byte *) &HOLDING_REGS_Data[RomAddress],INTEGER_LENGTH);
    }
  for(byte i=0;i < HOLDING_REGS_SIZE; i++){SlaveParameter[i] = HOLDING_REGS_ReadData[i];}
    if(SettingName[RomAddress] == "Commode"){delay(1000); ESP.restart();}
    
  
  UpdateLookLineData();
  LLModbusCom.modbus_write_setParameter(HOLDING_REGS_SIZE + 1, 0, 0);//Feedback

  }
  
  // Serial.println("Update from Modbus");
}
#endif//MODBUS 
void LOOKLINE_PROG::caculaOT()
{
  float nums = 0;
  if (PLAN_ > RESULT && pcsInShift > 0)
  {
    //Debug_Ser.println("Step1");
    #ifdef SLAVE_MODBUS
    CountOT_m = (PLAN_ - RESULT)*100;
    #endif
    #ifdef MASTER_MODBUS
    CountOT_m = (mtPLAN - mtRESULT)*100;
    #endif//MASTER_MODBUS
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

void LOOKLINE_PROG::SetLookineValue(){
  CONFIG::read_byte(EP_EEPROM_ID, &BoardIDs);
  #ifdef USE_LORA
  CONFIG::read_byte(EP_EEPROM_CHANELS, &Lora_CH);
  WriteLoRaConfig(Lora_CH, BoardIDs);ReadLoRaConfig();
  #endif//  #ifdef USE_LORA
  caculaOT();
  #ifndef LOOKLINE_MASTER
  SerDisplay(); SetValue(PLAN_,  RESULT,  CountOT_Hm,  CountOT_Lm,NodeRun);
  #endif//LOOKLINE_MASTER
}
void LOOKLINE_PROG::UpdateLookLineData(){
  // CONFIG::read_byte(EP_EEPROM_ID, &BoardIDs);
  BoardIDs = 0;
  CONFIG::read_byte (EP_EEPROM_ROLE, &role);

#ifdef SLAVE_MODBUS

  CONFIG::read_byte(EP_EEPROM_NETID, &NetIDs);
  CONFIG::read_byte(EP_EEPROM_ID, &BoardIDs);
  CONFIG::read_buffer(EP_EEPROM_PLAN,(byte *) &PLAN_, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_RESULT,(byte *) &RESULT, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_PLAN_SET,(byte *) &PLanSet, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_RESULT_SET,(byte *) &ResultSet, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_PLANMAX,(byte *) &PlanLimit, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_PCS,(byte *) &pcsInShift, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_TIME_PLAN,(byte *) &Time, INTEGER_LENGTH);
  CONFIG::read_byte(EP_EEPROM_COM_MODE, &ComMode);
  CONFIG::read_byte(EP_EEPROM_RUN, &NodeRun);
  CONFIG::read_byte(EP_EEPROM_MODULE_TYPE, &ModuleType);
  CONFIG::read_byte (EP_WIFI_MODE, &WiFiMode);

  CONFIG::read_buffer(EP_EEPROM_COUNTER_DELAY,(byte *) &delayForCounter, INTEGER_LENGTH);
  if(role == NODE || role == REPEARTER){ 
          HOLDING_REGS_ReadData[BOARDID] = BoardIDs;
          HOLDING_REGS_ReadData[NETID] = NetIDs;
          HOLDING_REGS_ReadData[RUNSTOP] =NodeRun;
          HOLDING_REGS_ReadData[ONOFF] = DispMode;
          HOLDING_REGS_ReadData[_PLAN] = PLAN_;
          HOLDING_REGS_ReadData[PLANSET] = PLanSet;
          HOLDING_REGS_ReadData[_RESULT] = RESULT;
          HOLDING_REGS_ReadData[RESULTSET] = ResultSet;
          HOLDING_REGS_ReadData[MAXPLAN] = PlanLimit;
          HOLDING_REGS_ReadData[PCS] = pcsInShift;
          HOLDING_REGS_ReadData[TIMEINC] = Time;
          HOLDING_REGS_ReadData[DELAYCOUNTER] =  delayForCounter;
          HOLDING_REGS_ReadData[COMMODE] = ComMode;
          HOLDING_REGS_ReadData[TYPE] =  ModuleType;
          HOLDING_REGS_ReadData[_RSSI] =  rssi_display;
          HOLDING_REGS_ReadData[ROLE] =  role;
          HOLDING_REGS_ReadData[ONWIFI] =  WiFiMode;
          HOLDING_REGS_ReadData[CMD] =  1;
    #ifdef ModbusCom
          LLModbusCom.modbus_read_update(HOLDING_REGS_ReadData);UpdateParamOnce = true;
    #endif//ModbusCom
          for(byte i=0;i < HOLDING_REGS_SIZE; i++){SlaveParameter[i] = HOLDING_REGS_ReadData[i];}
    }
    if(role == GATEWAY){ 
          HOLDING_REGS_ReadData[BOARDID] = BoardIDs;
          HOLDING_REGS_ReadData[NETID] = NetIDs;
          HOLDING_REGS_ReadData[RUNSTOP] =0;
          HOLDING_REGS_ReadData[ONOFF] = 0;
          HOLDING_REGS_ReadData[_PLAN] = 0;
          HOLDING_REGS_ReadData[PLANSET] = 0;
          HOLDING_REGS_ReadData[_RESULT] = 0;
          HOLDING_REGS_ReadData[RESULTSET] = 0;
          HOLDING_REGS_ReadData[MAXPLAN] = 0;
          HOLDING_REGS_ReadData[PCS] = 0;
          HOLDING_REGS_ReadData[TIMEINC] = Time;
          HOLDING_REGS_ReadData[DELAYCOUNTER] = 0;
          HOLDING_REGS_ReadData[COMMODE] = ComMode;
          HOLDING_REGS_ReadData[TYPE] =  ModuleType;
          HOLDING_REGS_ReadData[_RSSI] =  DataLookline.RSSI;
          HOLDING_REGS_ReadData[ROLE] =  role;
          HOLDING_REGS_ReadData[ONWIFI] =  WiFiMode;
          HOLDING_REGS_ReadData[CMD] =  1;
          
    #ifdef ModbusCom
          LLModbusCom.modbus_read_update(HOLDING_REGS_ReadData);UpdateParamOnce = true;
    #endif//MODBUSCOM 
          for(byte i=0;i < HOLDING_REGS_SIZE; i++){SlaveParameter[i] = HOLDING_REGS_ReadData[i];}
      }
  #endif//LOOKLINE_MASTER
  CONFIG::read_buffer(EP_EEPROM_TIMESENT,(byte *) &TimeSent, INTEGER_LENGTH);
  CONFIG::read_byte(EP_EEPROM_AMOUNTNODE, &AmountNode);

  CONFIG::read_byte (EP_EEPROM_DEBUG, &LooklineDebug);
  CONFIG::read_byte(EP_EEPROM_TEST_MODE, &TEST);
  CONFIG::read_byte(EP_EEPROM_CHANELS, &Lora_CH);
  #ifdef USE_LORA
  if(ComMode == LoRa){WriteLoRaConfig(Lora_CH, BoardIDs);}
  #endif//  #ifdef USE_LORA
  if(LooklineDebug)LOGLN("Update Data");

} //void LOOKLINE_PROG::UpdateLookLineData(){

void LOOKLINE_PROG::DebugOut(String msg,byte output){//1 = Web / 2 = Serial
if(output == 1) ESPCOM::webprint(msg);
if(output == 2) ESPCOM::print(msg, DEBUG_PIPE);
}

// void LOOKLINE_PROG::UpdateLookLineDataFormMaster(byte readrom){
//   if(Starting == false){
//       LOGLN();
//       LOGLN("read Rom: " + String(readrom));
//     if(readrom == 2 || readrom == 3 || readrom == 4 || readrom == 6)
//     {
//       if(readrom == 2){CONFIG::read_byte(EP_EEPROM_RUN, &NodeRun);}
//       if(readrom == 3){}
//       if(readrom == 4){CONFIG::read_buffer(EP_EEPROM_PLAN,(byte *) &PLAN_, INTEGER_LENGTH);}
//       if(readrom == 6){CONFIG::read_buffer(EP_EEPROM_RESULT,(byte *) &RESULT, INTEGER_LENGTH);}
//     }
//     else{UpdateLookLineData();}
//     SerDisplay();
//   }
// }

void LOOKLINE_PROG::LookLineInitB(int pos,byte Mode){
  #ifdef MASTER_MODBUS
  start = 1;
  #endif
  CONFIG::read_byte(pos, &Mode);
  if(pos == EP_EEPROM_ROLE){if(LooklineDebug)LOGLN("Role is :" + String(Mode));LookLineOnce1 = true;//LLModbusCom.modbus_write_setParameter(LLModbusCom.ROLE,Mode);
  }
  if(pos == EP_EEPROM_COM_MODE){if(LooklineDebug)LOGLN("Com mode is :" + String(Mode));ComMode = Mode;//LLModbusCom.modbus_write_setParameter(LLModbusCom.COMMODE,Mode);
  }
  if(pos == EP_EEPROM_UPDATE_MODE){if(LooklineDebug)LOGLN("Update mode is :" + String(Mode));}
  if(pos == EP_EEPROM_AMOUNTNODE){if(LooklineDebug)LOGLN("Amount node is :" + String(Mode));AmountNode = Mode;}
  if(pos == EP_EEPROM_RUN){if(LooklineDebug)LOGLN("Run set is :" + String(Mode));NodeRun = Mode;
  // LLModbusCom.modbus_write_setParameter(LLModbusCom.RUNSTOP,Mode);
      SetLookineValue();
    }
  if(pos == EP_EEPROM_ON_OFF){if(LooklineDebug)LOGLN("On/Off set is :" + String(Mode));}
  #ifdef USE_LORA
  if(pos == EP_EEPROM_CHANELS){if(LooklineDebug)LOGLN("Chanel set is :" + String(Mode));WriteLoRaConfig(Mode, BoardIDs);Lora_CH = Mode;}
  #endif//  #ifdef USE_LORA
  if(pos == EP_EEPROM_MODULE_TYPE){if(LooklineDebug)LOGLN("Set Module type :" + String(Mode));PinMapInit();LookLineOnce1 = true;//LLModbusCom.modbus_write_setParameter(LLModbusCom.TYPE,Mode);
  }
  if(pos == EP_EEPROM_TIMESENT){if(LooklineDebug)LOGLN("Time Sent :" + String(Mode));}
  if(pos == EP_EEPROM_DEBUG){if(LooklineDebug)LOGLN("Debug Mode:" + String(Mode));}
  if(pos == EP_EEPROM_TEST_MODE){if(LooklineDebug)LOGLN("Test Mode:" + String(Mode));TEST = Mode;}
  if(pos == EP_EEPROM_ID){if(LooklineDebug)LOGLN("Board ID is :" + String(Mode));BoardIDs = Mode;
    // LLModbusCom.modbus_write_setParameter(LLModbusCom.BOARDID,Mode);
  }
  if(pos == EP_EEPROM_NETID){if(LooklineDebug)LOGLN("Net ID is :" + String(Mode));NetIDs = Mode;
    // LLModbusCom.modbus_write_setParameter(LLModbusCom.NETID,Mode);
  }
    
  // LLModbusCom.modbus_write_setParameter(int pos,int Value) ;

    
  // EP_EEPROM_TIMESENT
}


void LOOKLINE_PROG::LookLineInitI(int pos,int Mode){
  #ifdef MASTER_MODBUS
  start = 1;
  #endif
CONFIG::read_buffer(pos,(byte*) &Mode, INTEGER_LENGTH);
if(pos == EP_EEPROM_TIME_PLAN){if(LooklineDebug)LOGLN("Time for Plan is :" + String(Mode));Time = Mode;//LLModbusCom.modbus_write_setParameter(LLModbusCom.TIMEINC,Mode);
}
if(pos == EP_EEPROM_TIMESENT){if(LooklineDebug)LOGLN("TIME sent is :" + String(Mode));TimeSent = Mode;
  CONFIG::read_byte(EP_EEPROM_ID, &BoardIDs);
    #ifdef USE_LORA
  CONFIG::read_byte(EP_EEPROM_CHANELS, &Lora_CH);
  WriteLoRaConfig(Lora_CH, BoardIDs);ReadLoRaConfig();
  #endif//  #ifdef USE_LORA
  }
if(pos == EP_EEPROM_PLAN){if(LooklineDebug)LOGLN("Plan is :" + String(Mode));PLAN_ = Mode;         
  // LLModbusCom.modbus_write_setParameter(LLModbusCom.PLAN,Mode);
  SetLookineValue();
  #ifdef MASTER_MODBUS
  mtPLAN = PLAN;
  #endif//MASTER_MODBUS
  SetValue(PLAN_,  RESULT,  CountOT_Hm,  CountOT_Lm,NodeRun);
}
if(pos == EP_EEPROM_PLAN_SET){if(LooklineDebug)LOGLN("Plan set is :" + String(Mode));PLanSet = Mode;//LLModbusCom.modbus_write_setParameter(LLModbusCom.PLANSET,Mode);
  SetValue(PLAN_,  RESULT,  CountOT_Hm,  CountOT_Lm,NodeRun);}
if(pos == EP_EEPROM_RESULT){if(LooklineDebug)LOGLN("Result is :" + String(Mode));RESULT = Mode;//LLModbusCom.modbus_write_setParameter(LLModbusCom.RESULT,Mode);
  SetLookineValue();
  #ifdef MASTER_MODBUS
  mtRESULT = Mode;
  #endif//MASTER_MODBUS
  SetValue(PLAN_,  RESULT,  CountOT_Hm,  CountOT_Lm,NodeRun);

}
if(pos == EP_EEPROM_RESULT_SET){if(LooklineDebug)LOGLN("Result set is :" + String(Mode));ResultSet = Mode;//LLModbusCom.modbus_write_setParameter(LLModbusCom.RESULTSET,Mode);
  SetValue(PLAN_,  RESULT,  CountOT_Hm,  CountOT_Lm,NodeRun);}
if(pos == EP_EEPROM_PCS){if(LooklineDebug)LOGLN("Result set is :" + String(Mode));pcsInShift = Mode;//LLModbusCom.modbus_write_setParameter(LLModbusCom.PCS,Mode);
}
if(pos == EP_EEPROM_PLANMAX){if(LooklineDebug)LOGLN("Plan limit is :" + String(Mode));PlanLimit = Mode;//LLModbusCom.modbus_write_setParameter(LLModbusCom.MAXPLAN,Mode);
  SetLookineValue();
}
if(pos == EP_EEPROM_COUNTER_DELAY){if(LooklineDebug)LOGLN("Counter delay :" + String(Mode));//LLModbusCom.modbus_write_setParameter(LLModbusCom.DELAYCOUNTER,Mode);
SetValue(PLAN_,  RESULT,  CountOT_Hm,  CountOT_Lm,NodeRun);}SerDisplay();
}
void LOOKLINE_PROG::PinMapInit(){
CONFIG::read_byte(EP_EEPROM_MODULE_TYPE, &ModuleType);
if(ModuleType == ModGateway){
  #ifndef LOOKLINE_MASTER
  uint8_t MapPin1[10] = {27,14, 2,15,19,25,26,18, 5, 4};// main board gateway V14
  for(byte i = 0 ; i < 10; i++){MapPin[i] = MapPin1[i];}
  CONFIG::write_byte (EP_EEPROM_ROLE, GATEWAY) ;
  #endif
 if(LooklineDebug){if(LooklineDebug)LOGLN("Lookline Gateway V14");}//Debug

}//if(ModuleType == MoGateway){
else{//lookline main V14
#ifndef LOOKLINE_MASTER
  uint8_t MapPin0[10] = {25,26, 2,15,19,27,13,22,21, 4};// main board lookline V14
  for(byte i = 0 ; i < 10; i++){MapPin[i] = MapPin0[i];}
  #endif//#ifndef LOOKLINE_MASTER
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

  if(LooklineDebug)LOGLN("pinmap --- SHCP:" + String(SHCP) + " STCP:" + String(STCP)  + " DATA1:" + String(DATA1)   + " DATA2:" + String(DATA2) + " DATA3:" + String(DATA3) + " M0:" + String(M0) + " M1:" + String(M1) + " X0:" + String(X0)  + " X1:" + String(X1) + " X2:" + String(X2)  + " X3:" + String(X3)  + " X4:" + String(X4) + " Status_LED:" + String(Startus_LED) + " Signal_LED:" + String(Signal_LED) ) ;

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
    if(LooklineDebug)
      LOGLN("Counter4:" + String(Counter4) + 
      " |Counter5:" + String(Counter5) + 
      " |Role: " + String(role) + 
      " |Amount node: " + String(AmountNode) + 
      " | Module type: " + String(ModuleType) + 
      "| Com Mode: " + String(ComMode) + 
        #ifdef USE_LORA
      "| M0:" +  String(M0) + "(" + String(digitalRead(M0)) + 
      ")| M1:" +  String(M1) +  "(" + String(digitalRead(M1)) + 
      ") | CH:" + String(Lora_CH) + 
      #endif//  #ifdef USE_LORA
      "| TEST:" + String(TEST));
}
void LOOKLINE_PROG::displayMode(byte Mode){
  DispMode = Mode;
}

void LOOKLINE_PROG::SetPlan(int SetPlans){
  PLAN_ = SetPlans;
  caculaOT();SerDisplay(); SetValue(PLAN_,  RESULT,  CountOT_Hm,  CountOT_Lm,NodeRun);
    // LLModbusCom.modbus_write_setParameter(LLModbusCom.PLAN,Mode);
}

void LOOKLINE_PROG::SetResult(int SetResults){
  RESULT = SetResults;
  #ifdef MASTER_MODBUS
  mtRESULT = RESULT;
  #endif//MASTER_MODBUS
  caculaOT();SerDisplay(); SetValue(PLAN_,  RESULT,  CountOT_Hm,  CountOT_Lm,NodeRun);
    // LLModbusCom.modbus_write_setParameter(LLModbusCom.RESULT,Mode);
}

void LOOKLINE_PROG::SetRun(byte SetRuns){
  if(SetRuns < 2){
  NodeRun = SetRuns;if(LooklineDebug)LOGLN("Run/Stop Lookline");
  LOG("Run:"+ String(NodeRun));//start = 1;
  #ifdef MASTER_MODBUS
   LLModbusCom.modbus_write_setParameter(LLModbusCom.RUNSTOP,SetRuns);
   LLModbusCom.modbus_write_setParameter(LLModbusCom.ONOFF,0,0);
  #endif//MASTER_MODBUS
    DispMode = Main;
  }
  if(SetRuns == 2){
      // LOGLN("Function Off Lookline");
    if(role == NODE|| role == REPEARTER){
      if(LooklineDebug)LOGLN("Off Lookline");
      DispMode = SLEEP;NodeRun = 0;
      #ifdef MASTER_MODBUS
      LLModbusCom.modbus_write_setParameter(LLModbusCom.ONOFF,1,0);
      #endif//MASTER_MODBUS
    }
    if(role == GATEWAY){
      if(LooklineDebug)LOGLN("MESH Mode");
      if(ComMode != MESH){
          CONFIG::write_byte(EP_EEPROM_COM_MODE, MESH);}
      else{CONFIG::write_byte(EP_EEPROM_COM_MODE, LoRa);}
          delay(1000);
          ESP.restart();
    }
  }
  if(SetRuns == 3){
    if(LooklineDebug)LOGLN("On wifi main board");
      #ifdef MASTER_MODBUS
      LLModbusCom.modbus_write_setParameter(LLModbusCom.ONWIFI,0,0);
      #endif//MASTER_MODBUS
      #ifdef SLAVE_MODBUS
        CONFIG::write_byte(EP_EEPROM_COM_MODE, LoRa);
          delay(1000);
          ESP.restart();
      #endif//MASTER_MODBUS
  }
  if(SetRuns == 4){ 
    if(LooklineDebug)LOGLN("Off wifi main board");
      #ifdef MASTER_MODBUS
      LLModbusCom.modbus_write_setParameter(LLModbusCom.ONWIFI,2,0);
      #endif//SLAVE_MODBUS
      #ifdef SLAVE_MODBUS
        CONFIG::write_byte(EP_EEPROM_COM_MODE, MESH);
          delay(1000);
          ESP.restart();
      #endif//MASTER_MODBUS
  }
  if(SetRuns == 5){ 
    if(LooklineDebug)LOGLN("Clear Plan/Result");
    byte zero = 0;
       CONFIG::write_buffer (EP_EEPROM_PLAN, (byte*) &zero, INTEGER_LENGTH);
      CONFIG::write_buffer (EP_EEPROM_RESULT, (byte*) &zero, INTEGER_LENGTH);
      #ifdef MASTER_MODBUS
      LLModbusCom.modbus_write_setParameter(LLModbusCom.ONOFF,2,0);
      #endif//SLAVE_MODBUS
      #ifdef SLAVE_MODBUS
        // PLAN_ = 0;
        // RESULT = 0;
      #endif//MASTER_MODBUS
  }
  
}
byte LOOKLINE_PROG::GetRun(){return NodeRun;}
byte LOOKLINE_PROG::GetDebug(){return LooklineDebug;}
void LOOKLINE_PROG::SetDone(){done = true;}




















void LOOKLINE_PROG::setup() {
  Serial.begin(115200);
  // Serial2.begin(9600);
    button.debounceTime   = 20;   // Debounce timer in ms
    button.multiclickTime = 250;  // Time limit for multi clicks
    button.longClickTime  = 1000; // Time until long clicks register

  UpdateLookLineData();
  #ifndef LOOKLINE_MASTER
  PinMapInit();
        // if(LooklineDebug)LOGLN("Main Data1--" + String(DATA1));//2
        // if(LooklineDebug)LOGLN("Main Data2--" + String(DATA2));//15
        // if(LooklineDebug)LOGLN("Main Data3--" + String(DATA3));//19
        // if(LooklineDebug)LOGLN("Main STCP--" + String(STCP));//26
        // if(LooklineDebug)LOGLN("Main SHCP--" + String(SHCP));//25
  SetPin(DATA1, DATA2, DATA3, SHCP, STCP, BoardIDs, NetIDs, Lora_CH, Startus_LED, TimeSent);
  PrintSeg(Seg[0], Seg[0], Seg1[0]);latch();
  
  #ifdef USE_LORA
  if(ComMode == LoRa){SetPinLoRa( M0,  M1,  16,  17);}
  #endif//  #ifdef USE_LORA
  
  if(ModuleType != ModGateway){ SetPin7Seg(DATA1, DATA2, DATA3, SHCP, STCP);}
  // #if defined(DEBUG_WIC) && defined(DEBUG_OUTPUT_SERIAL)
  //           // CONFIG::InitBaudrate(DEFAULT_BAUD_RATE);
  //           // delay(2000);
  //           LOG("\r\nDebug Serial set\r\n")
  //           DebugOut("\r\nSet serial baudrate\r\n", OUPUT);
  // #endif
  #endif//LOOKLINE_MASTER
  DebugOut("Initialized.", OUPUT);
  SendMsg("Initialized.");
          String monitor = "";



  #ifndef LOOKLINE_MASTER
  /////// Test
#ifdef TestDisplayIntro
if(ModuleType != GATEWAY){  
  for (int i = 0; i < 10; i++)
  {
    // TestDisplay(i);
    displays(i*1111, i*1111, i*1111, i*1111, taskPin.Data1, taskPin.Data2, taskPin.Data3, taskPin.SHCP, taskPin.STCP, 0);latch();

    // #ifdef TEST_MODE
    digitalWrite(Startus_LED, digitalRead(Startus_LED) ^ 1);
    // #endif//TEST_MODE
    delay(300);
  }
caculaOT(); SetValue(PLAN_,  RESULT,  CountOT_Hm,  CountOT_Lm,NodeRun);
}
#endif//not dev
#else
  // if(check_protocol() != 8){
  esp_wifi_set_protocol(current_wifi_interface, 7);
  check_protocol();
  // }
#endif//#ifndef LOOKLINE_MASTER
SerDisplay();

    CONFIG::read_byte(EP_EEPROM_TEST_MODE, &TEST);
    if(TEST >= 2){TEST = 0;CONFIG::write_byte(EP_EEPROM_TEST_MODE, TEST);}
  #ifdef USE_LORA
if(ComMode == LoRa){
  CONFIG::read_byte(EP_EEPROM_ID, &BoardIDs);
  CONFIG::read_byte(EP_EEPROM_CHANELS, &Lora_CH);
  WriteLoRaConfig(Lora_CH, BoardIDs);ReadLoRaConfig();
}
#endif//  #ifdef USE_LORA
  // Serial2.begin(9600);
  ///////////////////////////////////////////////////////////////////////////////////
  #ifndef LOOKLINE_MASTER 
if(TEST){
    if(ComMode == LoRa){
    #ifdef USE_LORA
    digitalWrite(M0_, HIGH);
    digitalWrite(M1_, HIGH);
    // Startup all pins and UART
    e32ttl100.begin();

  //  If you have ever change configuration you must restore It
    ResponseStructContainer c;
    c = e32ttl100.getConfiguration();
    Configuration configuration = *(Configuration*) c.data;
    LOGLN(c.status.getResponseDescription());
    configuration.CHAN = 0x17;
    configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
    e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  c.close();
    LOGLN("Hi, I'm going to send message!");

    // Send message
    ResponseStatus rs = e32ttl100.sendMessage("Hello, world?");
    // Check If there is some problem of succesfully send
    LOGLN(rs.getResponseDescription());
    digitalWrite(M0_, LOW);
    digitalWrite(M1_, LOW);
    #endif////////////////////////////////
    }
  }//if(ComMode == LoRa){
      UpdateLookLineData();
            if(ComMode == MESH){ monitor += "Communica: MESH |";MeshLookLineSetup();}
            if(ComMode == MQTT){ monitor += "Communica: MQTT |";}
            if(ComMode == LoRa){ monitor += "Communica: Lora |";}
            if(ComMode == RS485com){ monitor += "Communica: RS485 |";}

            monitor += "firmware:";
            monitor += UDFWLookLine.FirmwareVer;
            monitor +=  "| X0: ";
            monitor += String(analogRead(X0));
            monitor += "  X1:";
            monitor += String(analogRead(X1));
            monitor += "  X2:";
            monitor += String(analogRead(X2));
            monitor += "  X3:";
            monitor += String(analogRead(X3));
            monitor += "  X4:";
            monitor += String(analogRead(X4));
         if(LooklineDebug)LOGLN(monitor);
#endif//LOOKLINE_MASTER

}//settup























/* ############################ Loop ############################################# */


// #define TEST_LORA


  void LoRaCommu(byte mode);
int countTimer = 0;
String strIn = "";
#ifdef MASTER_MODBUS
const long intervalMain = 10;
#else//SLAVE
const long intervalMain = 10;
#endif//MASTER_MODBUS
unsigned long previousMillisMain = 0;
bool MainTimer = true;
int TimerModbus = 0;
void LOOKLINE_PROG::loop()
{
if(((start == 0 || start == 2 & ComMode == LoRa) || ComMode == MESH)){// = 0 normal , =1 wait for update to web, = 2 run loop with socket
  // countTimer++; if(countTimer > 10000){
  //   countTimer = 0;
  //   SerDisplay();
  // }
    button.Update();
  if (button.clicks != 0) function = button.clicks;
  if(button.clicks == 1 ){function = 0;
    NodeRun = !NodeRun;
  }
  
  if(button.clicks == -1 ){function = 0;
  }

  if(button.clicks == -3 ){function = 0;
    LOGLN("boot: NODE");
      CONFIG::write_byte(EP_EEPROM_ROLE, NODE);
      CONFIG::write_byte(EP_EEPROM_MODULE_TYPE, ModuleV141);
      ESP.restart();

  }

  if(button.clicks == -2 ){function = 0;
    LOGLN("boot: Gateway");
    if(ComMode == MESH){CONFIG::write_byte(EP_EEPROM_COM_MODE, LoRa);}
    if(ComMode == LoRa){CONFIG::write_byte(EP_EEPROM_COM_MODE, MESH);}
      CONFIG::write_byte(EP_EEPROM_ROLE, GATEWAY);
      CONFIG::write_byte(EP_EEPROM_MODULE_TYPE, ModGateway);
      ESP.restart();

  }

  //  Counterstatus++;if(Counterstatus > 10000){Counterstatus = 0;LookLineOnce1 = true;
  // if(LooklineDebug)LOGLN("Status");UpdateLookLineData();
  // }
if(TimerModbus > 100){TimerModbus = 0;
  // LOG("Data Write: ");for(byte i=0;i < HOLDING_REGS_SIZE; i++){LOG(String(HOLDING_REGS_WriteData[i]));LOG("|");}LOGLN();
  // LOG("Data Read: ");for(byte i=0;i < HOLDING_REGS_SIZE; i++){LOG(String(HOLDING_REGS_ReadData[i]));LOG("|");}LOGLN();
        // LLModbusCom.debugs();

}
unsigned long currentMillisMain = millis();
  if (currentMillisMain - previousMillisMain >= intervalMain)
  {
    previousMillisMain = currentMillisMain;
    TimerPlanInc();TimerModbus++;
    if(MainTimer){MainTimer = false;LOGLN("main Timer");}
  }
  if(LookLineOnce){LookLineOnce = false; 
    if(role == GATEWAY){DebugOut("Gateway Run.\n", OUPUT);}
    if(role == NODE || role == REPEARTER){DebugOut("Lookline Run.\n", OUPUT);}
  }
#ifndef LOOKLINE_MASTER
  TaskDisplay(DispMode);
  TaskInPut();
#endif//LOOKLINE_MAS
    if(role == GATEWAY){
      MonitorMode = Main;
      if(delayForCounter == 1){

      }
    }
    // if (role == NODE || role == REPEARTER ){digitalWrite(Startus_LED,NodeRun);}
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
  if(ComMode == LoRa){if(TEST){LoRaCommu(1);}else{LoRaCommu(0);}}
  digitalWrite(M0_, LOW);digitalWrite(M1_, LOW);
  #endif//LoRa_Network
  #ifdef Mesh_Network
  if(ComMode == MESH){
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
      
      if(strIn == "Mesh"){LOGLN("On Mesh ok!!");
        // if(strIn == "onwifi"){
          check_protocol();
          CONFIG::write_byte(EP_EEPROM_COM_MODE, MESH);
          delay(1000);ESP.restart();
          strIn = "";
        // }else{
        //   strIn = "";
        // }
      }
      if(strIn == "RESET"){LOGLN("Reset ok!!");
         CONFIG::reset_config();
      strIn = "";
      }
      if(strIn == "Gateway"){LOGLN("switch to Gateway ok!!");
          CONFIG::write_byte(EP_EEPROM_ROLE, GATEWAY);
          CONFIG::write_byte(EP_EEPROM_MODULE_TYPE, GATEWAY);
      strIn = "";
      }
      if(charin == '\n'){LOG(strIn);strIn = "";}
    }
  }
  #endif//Mesh_Network

  
  /////////////////////////////////// Gateway mode ///////////////////////////
  if(LookLineOnce1){LookLineOnce1 = false; ShowParameters();}

  }//start 
}//Loop

































  byte counter6 = 0;//so lan gui lai



  int Data[5][100];
  int LastTime[100];
  int MaxTime[100];

void LOOKLINE_PROG::SetParameter(int Plan, int taskPLanSet, int Result, int ResultSet, int taskTime, int taskpcsInShift, int taskPass, int taskDotIn){
  PLAN_ = Plan;PLanSet = taskPLanSet;RESULT = Result;ResultSet = ResultSet;Time = taskTime;pcsInShift = taskpcsInShift;Pass = taskPass;DotIn = taskDotIn;
  if(LooklineDebug)LOGLN("Set parameters from gateway");
}

#define TempDisplay

bool OnceCheck = true;
int totalInterruptCounterLookline = 0; 
int TimeCacu = 0;
int timestarting = 0;
void LOOKLINE_PROG::TimerPlanInc()
{
    if(Starting == true){timestarting++;
        if(OnceCheck){OnceCheck = false;LOGLN("Starting");}
      if(timestarting > 300){timestarting = 0;
        // Updateonce = true;
        Starting = false;LOGLN(); 
        LOGLN("Started");LOGLN(); 
        UpdateLookLineData();
      }
    }
  //  LOGLN("TimerPlanInc");  
   if(role == GATEWAY){ Counter5++;
   if(Counter5 > 100){
      Counter5 = 0;for(int i = 0 ; i < NUM_LOOKLINES ; i++){Looklines[i].Nodecounter++;}  
      // LLModbusCom.debugs();
      String WebData = "GATEWAY:";
      for (int i = 0; i < Looklines_saved; i++) {
        if(DataLookline.nodeID > 0){
        WebData += String(Looklines[i].nodeID) + ",";
        WebData += String(Looklines[i].networkID) + ",";
        WebData += String(Looklines[i].state) + ",";
        WebData += String(Looklines[i].PLAN) + ",";
        WebData += String(Looklines[i].RESULT) + ",";
        WebData += String(Looklines[i].RSSI) + ",";
        WebData += String(Looklines[i].Com) + ",";
        WebData += String(Looklines[i].WiFi) + ",";
        WebData += String(Looklines[i].type) + ",";
        WebData += String(Looklines[i].Nodecounter);
        if ( i < Looklines_saved - 1) WebData += '\n';
        }
      }
      if(start == 2)socket_server->broadcastTXT(WebData);  
    }//if(Counter5 > 100){
   }//if(role == GATEWAY){

  if(role == NODE && role == REPEARTER)totalInterruptCounterLookline++;
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
        #ifndef MASTER_MODBUS
        connected(counter3++);
        if (counter3 > 3)
          counter3 = 0;
        #endif//MASTER_MODBUS
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
//////Run/Stop Plan
        #ifdef SLAVE_MODBUS
        CONFIG::read_buffer(EP_EEPROM_TIME_PLAN,(byte*) &Time, INTEGER_LENGTH);
        #else
        Time = 50;
        #endif//SLAVE_MODBUS
      // if (DotIn == 0)
      // {
      //   TimeCacu = (Time*10) * 1;
      // }
      // if (DotIn == 1)
      // {
      //   TimeCacu = (Time*10) * 10;
      // }
      // if (DotIn == 2)
      // {  
      //   TimeCacu = (Time*10) * 100;
      // }
      // if (DotIn == 3)
      // {
      //   TimeCacu = (Time*10) * 1000;
      // }
      
    if (NodeRun == true)
    {
      // LOGLN("Tik");
      digitalWrite(Startus_LED, HIGH);
      counter2++;
      if (counter2 >= Time)
      {
        counter2 = 0;
        //digitalWrite(Signal_LED, digitalRead(Signal_LED) ^ 1);
        #ifdef SLAVE_MODBUS
        PLAN_ = PLAN_ + PLanSet;//MODBUS_Write();
        #else//MASTER_MODBUS
        PLAN = mtPLAN;
        RESULT = mtRESULT;
        #endif//SLAVE_MODBUS
        caculaOT(); SetValue(PLAN_,  RESULT,  CountOT_Hm,  CountOT_Lm,NodeRun);
        SerDisplay();
        if(PLAN_ % 10 == 9){
                if (!CONFIG::write_buffer (EP_EEPROM_PLAN, (const byte *) &PLAN_, INTEGER_LENGTH) ) {
                  LOG("save Plan failed");
                }
           CONFIG::read_buffer(EP_EEPROM_PLAN, (byte *) &PLAN_, INTEGER_LENGTH);
        }
        
        if (PLAN_ > PlanLimit)
        {
          LOGLN("================================================");
          LOGLN("================ PLAN LIMIT ====================");
          LOGLN("================================================");
          PLAN_ = PlanLimit;
        }
        //
      }
    }//RUN
    else{
      digitalWrite(Startus_LED, LOW);counter2++;
      if (counter2 >= 500)
      {
        // LOGLN("Tik");
        counter2 = 0;//Monitor
        #ifdef MASTER_MODBUS
        PLAN = mtPLAN;
        #endif//SLAVE_MODBUS

      if(start == 2)socket_server->broadcastTXT("VALUE:" + String(PLAN_) + ":" + String(RESULT) + ":" + String(CountOT_Hm) + "." + String(CountOT_Lm));
      caculaOT(); SetValue(PLAN_,  RESULT,  CountOT_Hm,  CountOT_Lm,NodeRun);
      if(config == 1 || config == 2 ){
        DataLookline.nodeID = BoardIDs;
        DataLookline.networkID = NetIDs;
        DataLookline.PLAN = PLAN_;
        DataLookline.RESULT = RESULT;
        DataLookline.state =NodeRun;
        #ifdef SLAVE_MODBUS
        DataLookline.RSSI = rssi_display;
        #endif//SLAVE_MODBUS
        DataLookline.Com = ComMode;
        DataLookline.WiFi = WiFiMode;
        DataLookline.type = ModuleType - 2;
        DataLookline.Cmd = UPDATEcmd;
      }
      SerDisplay();
        // if(NodeRun){LOG("Runing");}else{LOG("Stopping");} if(LooklineDebug)LOGLN(" | Time: "+ String(Time));
      }
    }
      counter4++;
      if(counter4 > 100){counter4 = 0;counter6++;
        if(ComMode == MESH && ReSent){
          if(config == 1 || config == 2){
        #ifdef SLAVE_MODBUS

            // esp_wifi_set_protocol(current_wifi_interface, WIFI_PROTOCOL_LR);
            esp_now_send(broadcastAddress, (uint8_t *) &DataLookline, sizeof(DataLookline));if(LooklineDebug)LOGLN("Monitor Mesh send data");
          #endif//SLAVE_MODBUS
            // esp_wifi_set_protocol(current_wifi_interface, 6);
            }
          // esp_now_send(broadcastAddress, (const uint8_t *)Log.c_str(), Log.length());
        }
        if(counter6 > 5)ReSent = false;
      }
  }//if (role == NODE || role == REPEARTER )
    if(totalInterruptCounterLookline >= 100){
      
    // if(LooklineDebug)LOGLN("TimerPlanInc | Role:" + String(role));
    // SerDisplay();
    
      
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




void LoRaCommu(byte mode)
{
#ifdef USE_LORA
  if(mode == 0){
    // If something available
  digitalWrite(M0_, LOW);digitalWrite(M1_, LOW);
    if (e32ttl100.available()>1) {
        // read the String message
      ResponseContainer rc = e32ttl100.receiveMessage();
      // Is something goes wrong print error
      if (rc.status.code!=1){
        rc.status.getResponseDescription();
      }else{
        // Print the data received
        LOGLN(rc.data);
  digitalWrite(M0_, LOW);digitalWrite(M1_, LOW);
          ResponseStatus rs = e32ttl100.sendMessage(rc.data);
        // Check If there is some problem of succesfully send
        LOGLN(rs.getResponseDescription());
      }
    }
  }
  if(mode == 1){
      	// If something available
      if (e32ttl100.available()>1) {
        // read the String message
        ResponseContainer rc = e32ttl100.receiveMessage();
        // Is something goes wrong print error
        if (rc.status.code!=1){
          rc.status.getResponseDescription();
        }else{
          // Print the data received
          LOGLN(rc.data);
          String input = rc.data;
            for (int j = 0 ; j < input.length() ;j++){
              buffer[j] = input[j];
              if (j > 50)
              {break;}
            } 
        
          done = true;
          setupEn = true;
          String ids = "";
          ids += buffer[0];
          ids += buffer[1];
          ids += buffer[2];
          ids += buffer[3];
          NodeID = ids.toInt();
          Data_Proccess(buffer);
          LoRa_Seri.flush();
          countSer = 0;
    }
   }
  }//Mode 1
  #endif//  #ifdef USE_LORA
}

void Data_Proccess(char byte_buffer[])
{
if(done == true){if(LooklineDebug)LOGLN("Done");
        done = false;
    String cmds = String(byte_buffer[4]) + String(byte_buffer[5]);
    String Length = String(byte_buffer[6]) + String(byte_buffer[7]);
    // if(LooklineDebug)LOGLN("Cmd:" + String(cmds.toInt()));
    // if(LooklineDebug)LOGLN("Length:" + String(Length.toInt()));
    if (cmds.toInt() == cmdLookLine.updateStt && (role == NODE || role == REPEARTER))
    {
      if (Length.toInt() == 12)
      {
        if(NodeID == BoardIDs)
        {
          // if(LooklineDebug){
            if(LooklineDebug)LOGLN("Sent to gateway " );
            // if(ComMode == MQTT){ LOG("  Topic:");if(LooklineDebug)LOGLN(String(Lookline_PROG.TopicOut));}
            // else{if(LooklineDebug)LOGLN();}
          // }
          // if(LooklineDebug){
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
            if(ComMode == MESH && role == NODE){esp_now_send(broadcastAddress, (const uint8_t *)sentData.c_str(), sentData.length());LOGLN("Mesh Send Data");}
          #endif//Mesh_Network   
          #ifdef RS485
            if(ComMode == RS485com){RS485_Ser.println(sentData);RS485_Ser.flush();}
          #endif//RS485    
          #ifdef USE_LORA 
            if(ComMode == LoRa){
              // digitalWrite(M0, LOW);digitalWrite(M1, LOW);
              // LoRa_Seri.println(sentData);LoRa_Seri.flush();
  digitalWrite(M0_, LOW);digitalWrite(M1_, LOW);
              e32ttl100.sendMessage(sentData);
              if(LooklineDebug)LOGLN(sentData);
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
          // if(LooklineDebug)LOGLN(buffer);
          if(LooklineDebug)LOGLN("sent to gateway");
          if(LooklineDebug)LOGLN("__________________________________");
        }//if(NodeID == BoardIDs)
        //payload [IDh,IDl,01,16,PlanH,PlanL,ResultH,ResultL,TimeH,TimeL,PlanSH,PlanSL,ResultSH,ResultSL,PassH,PassL,PCSH,PCSL,DOT]
      }//if (Length.toInt() == 12)
      
    }//(cmds.toInt() == cmdLookLine.updateStt)
    if(cmds.toInt() == cmdLookLine.setup)
    {
      if (Length.toInt() == 16){
        if(NodeID == BoardIDs){
           String StringPlan = "";
          StringPlan += (byte_buffer[8] / 1000) % 10;
          StringPlan += (byte_buffer[9] / 100) % 10;
          StringPlan += (byte_buffer[10] / 10) % 10;
          StringPlan += (byte_buffer[11] / 1) % 10;
          //  PLAN = StringPlan.toInt();
          String StringResult = "";
          StringResult += (byte_buffer[12] / 1000) % 10;
          StringResult += (byte_buffer[13] / 100) % 10;
          StringResult += (byte_buffer[14] / 10) % 10;
          StringResult += (byte_buffer[15] / 1) % 10; 
            RESULT = StringResult.toInt();
          String StringTime = "";
          StringTime += (byte_buffer[16] / 1000) % 10;
          StringTime += (byte_buffer[17] / 100) % 10;
          StringTime += (byte_buffer[18] / 10) % 10;
          StringTime += (byte_buffer[19] / 1) % 10; 
            int taskTime = StringTime.toInt();
          String PlanSets = "";
          PlanSets += (byte_buffer[20] / 1000) % 10;
          PlanSets += (byte_buffer[21] / 100) % 10;
          PlanSets += (byte_buffer[22] / 10) % 10;
          PlanSets += (byte_buffer[23] / 1) % 10; 
           int  taskPLanSet = PlanSets.toInt();
          String ResultSets = "";
          ResultSets += (byte_buffer[24] / 1000) % 10;
          ResultSets += (byte_buffer[25] / 100) % 10;
          ResultSets += (byte_buffer[26] / 10) % 10;
          ResultSets += (byte_buffer[27] / 1) % 10; 
           int  ResultSet = ResultSets.toInt();
          String StringPass = "";
          StringPass += (byte_buffer[28] / 1000) % 10;
          StringPass += (byte_buffer[29] / 100) % 10;
          StringPass += (byte_buffer[30] / 10) % 10;
          StringPass += (byte_buffer[31] / 1) % 10; 
           int  taskPass = StringPass.toInt();
          String PcsShift = "";
          PcsShift += (byte_buffer[32] / 1000) % 10;
          PcsShift += (byte_buffer[33] / 100) % 10;
          PcsShift += (byte_buffer[34] / 10) % 10;
          PcsShift += (byte_buffer[35] / 1) % 10; 
           int  taskpcsInShift = PcsShift.toInt();
          String StringDot = "";
          StringDot += (byte_buffer[36] / 10) % 10;
          StringDot += (byte_buffer[37] / 1) % 10;
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
            if(ComMode == MESH){esp_now_send(broadcastAddress, (const uint8_t *)sentData.c_str(), sentData.length());}
          #endif//Mesh_Network   
          #ifdef RS485
            if(ComMode == RS485com){RS485_Ser.println(sentData);RS485_Ser.flush();}
          #endif//RS485    
          #ifdef USE_LORA 
            if(ComMode == LoRa){
            //   digitalWrite(M0, LOW);digitalWrite(M1, LOW);
            // LoRa_Seri.println(sentData);LoRa_Seri.flush();
            // if(LooklineDebug)LOGLN(sentData);
            // ResponseStatus rs = e32ttl100.sendMessage(sentData); // OK The message is received on the other device
            // // Check If there is some problem of succesfully send
            //   LOGLN(rs.getResponseDescription());
            digitalWrite(M0_, LOW);digitalWrite(M1_, LOW);
            e32ttl100.sendMessage(sentData);
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
        digitalWrite(taskStatus_LED, LOW);delay(100);digitalWrite(taskStatus_LED, HIGH);
        }//if(taskModuleType == ModGateway){
        String ID = String(byte_buffer[0]) + String(byte_buffer[1]) + String(byte_buffer[2]) + String(byte_buffer[3]); 
          if(ID.toInt() == IDSent){
            SendMsg(String(buffer));
            PC_Seri.println(String(buffer));counter5 = TimeSent;delay(300);counter5 = TimeSent - 2;
            String plan = String(byte_buffer[8]) + String(byte_buffer[9]) + String(byte_buffer[10]) + String(byte_buffer[11]);
            String result = String(byte_buffer[12]) + String(byte_buffer[13]) + String(byte_buffer[14]) + String(byte_buffer[15]);
            String Runs = String(byte_buffer[16]);
            String Modes = String(byte_buffer[17]);
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
        // UpdateLookLineData();
        #ifdef SLAVE_MODBUS
        CONFIG::read_byte(EP_EEPROM_ID, &BoardIDs);
        CONFIG::read_byte(EP_EEPROM_NETID, &NetIDs);
        
        
        // CONFIG::read_byte(EP_EEPROM_CHANELS, &Lora_CH);
        // CONFIG::read_byte(EP_EEPROM_DEBUG, &LooklineDebug);
        // CONFIG::read_byte(EP_EEPROM_AMOUNTNODE, &AmountNode);
        // CONFIG::read_byte(EP_EEPROM_TIMESENT, &TimeSent);
        // CONFIG::read_byte(EP_EEPROM_MODULE_TYPE, &ModuleType);
        // CONFIG::read_byte(EP_EEPROM_ROLE, &role);
        // CONFIG::read_buffer(EP_EEPROM_TIME_PLAN,(byte*) &Time, INTEGER_LENGTH);
        // CONFIG::read_buffer(EP_EEPROM_PLAN,(byte*) &PLAN, INTEGER_LENGTH);
        // CONFIG::read_buffer(EP_EEPROM_RESULT,(byte*) &RESULT, INTEGER_LENGTH);
        String Log = "";
        Log = "| Board ID: "+ String(BoardIDs);
        Log +=" | Network ID: "+ String(NetIDs);
        Log +=" | Chanel : "+ String(Lora_CH);
        // Log += "| M0:" +  String(M0) + "(" + String(digitalRead(M0)) + ")| M1:" +  String(M1) +  "(" + String(digitalRead(M1)) + ")";
        if(role == NODE || role == REPEARTER){
          if(NodeRun){Log +="| Runing";}else{Log +="| Stopping";} 
          Log +=" | Time: "+ String(Time) + "| ";
          if(DispMode == Test){Log +="Disp: Test ";}
          if(DispMode == Setting_){Log +="Disp: Setting ";}
          if(DispMode == ConFi){Log +="Disp: ConFi ";}
          if(DispMode == Online){Log +="Disp: Online ";}
          if(DispMode == SLEEP){Log +="Disp: SLEEP ";}
          if(DispMode == CLEAR){Log +="Disp: Clear ";}
          Log +="|Plan: ";Log +=PLAN_;
          Log +="|  Result: ";Log +=RESULT;
          Log +="|  O.T: ";Log +=CountOT_Hm;Log +="."; Log +=CountOT_Lm;   
          Log +="| Startus_LED pin:" + String(Startus_LED) + "(" + digitalRead(Startus_LED) + ")";
        } else{
          Log += "| Amount node: " + String(AmountNode);
        }
        Log +="  |Temp: ";
        #ifdef ESP32
        Log +=(temprature_sens_read() - 32) / 1.8;
        #endif//ESp32
        Log +=" C | Rssi:" + String(rssi_display);
         
        if(LooklineDebug)LOGLN(Log);
        // if(TEST)broadcast("Node: " + String(BoardID));
        DataLookline.nodeID = BoardIDs;
        DataLookline.networkID = NetIDs;
        DataLookline.PLAN = PLAN_;
        DataLookline.RESULT = RESULT;
        DataLookline.state =NodeRun;
        DataLookline.RSSI = rssi_display;
        DataLookline.Com =ComMode;
        DataLookline.WiFi =0;
        #ifdef Mesh_Network 
        if(ComMode == MESH){
        esp_now_send(broadcastAddress, (uint8_t *) &DataLookline, sizeof(DataLookline));
        // esp_now_send(broadcastAddress, (const uint8_t *)Log.c_str(), Log.length());
        }
        #endif//#ifdef Mesh_Network 
        // ShowParameters();
        if(role == NODE || role == REPEARTER){ 
          HOLDING_REGS_ReadData[BOARDID] = BoardIDs;
          HOLDING_REGS_ReadData[NETID] = NetIDs;
          HOLDING_REGS_ReadData[RUNSTOP] =NodeRun;
          HOLDING_REGS_ReadData[ONOFF] = DispMode;
          HOLDING_REGS_ReadData[_PLAN] = PLAN_;
          HOLDING_REGS_ReadData[PLANSET] = PLanSet;
          HOLDING_REGS_ReadData[_RESULT] = RESULT;
          HOLDING_REGS_ReadData[RESULTSET] = ResultSet;
          HOLDING_REGS_ReadData[MAXPLAN] = PlanLimit;
          HOLDING_REGS_ReadData[PCS] = pcsInShift;
          HOLDING_REGS_ReadData[TIMEINC] = Time;
          HOLDING_REGS_ReadData[DELAYCOUNTER] =  delayForCounter;
          HOLDING_REGS_ReadData[COMMODE] = ComMode;
          HOLDING_REGS_ReadData[TYPE] =  ModuleType;
          HOLDING_REGS_ReadData[_RSSI] =  rssi_display;
          HOLDING_REGS_ReadData[ROLE] =  role;
          HOLDING_REGS_ReadData[ONWIFI] =  WiFiMode;
          HOLDING_REGS_ReadData[CMD] =  1;
    #ifdef ModbusCom
          LLModbusCom.modbus_read_update(HOLDING_REGS_ReadData);UpdateParamOnce = true;
    #endif
          for(byte i=0;i < HOLDING_REGS_SIZE; i++){SlaveParameter[i] = HOLDING_REGS_ReadData[i];}
          
        }
        #endif//SLAVE_MODBUS
        #ifdef MASTER_MODBUS
        String Log = "";
        Log = "| Board ID: "+ String(mtBoardID);
        Log +=" | Network ID: "+ String(mtNetID);
        // Log += "| M0:" +  String(M0) + "(" + String(digitalRead(M0)) + ")| M1:" +  String(M1) +  "(" + String(digitalRead(M1)) + ")";
        if(role == NODE || role == REPEARTER){
          if(mtNodeRun){Log +="| Runing";}else{Log +="| Stopping";} 
          Log +=" | Time: "+ String(mtTime) + "| ";
          if(mtDispMode == Test){Log +="Disp: Test ";}
          if(mtDispMode == Setting_){Log +="Disp: Setting ";}
          if(mtDispMode == ConFi){Log +="Disp: ConFi ";}
          if(mtDispMode == Online){Log +="Disp: Online ";}
          if(mtDispMode == SLEEP){Log +="Disp: SLEEP ";}
          if(mtDispMode == CLEAR){Log +="Disp: Clear ";}
          Log +="|Plan: ";Log += mtPLAN;
          Log +="|  Result: ";Log += mtRESULT;
          Log +="|  O.T: ";Log += CountOT_Hm;Log +="."; Log +=CountOT_Lm;   
          Log +="| Startus_LED pin:" + String(Startus_LED) + "(" + digitalRead(Startus_LED) + ")";
        } else{
          // Log += "| Amount node: " + String(AmountNode);
        }
        Log +="  |Temp: ";
        #ifdef ESP32
        Log +=(temprature_sens_read() - 32) / 1.8;
        #endif//ESp32
        Log +=" C |";
        if(start == 2 && (role == NODE || role == REPEARTER))socket_server->broadcastTXT("VALUE:" + String(mtPLAN) + ":" + String(mtRESULT) + ":" + String(CountOT_Hm) + "." + String(CountOT_Lm));
        if(LooklineDebug)LOGLN(Log);
        // if(TEST)broadcast("Node: " + String(BoardID));
        DataLookline.nodeID = mtBoardID;
        DataLookline.networkID = mtNetID;
        DataLookline.PLAN = mtPLAN;
        DataLookline.RESULT = mtRESULT;
        DataLookline.state = mtNodeRun;
        PLAN = mtPLAN;
        RESULT = mtRESULT;
        #endif//MASTER_MODBUS
        // LLModbusCom.debugs();
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
  boolNodeRun = true;
  bool lock = true;//IR Lock mode
  */
  DynamicJsonDocument doc(1500);
  deserializeJson(doc, Input);
  JsonObject obj = doc.as<JsonObject>();
  //{"Command":"Config","Run":1,"time":10,"Pla":1,"Res":0,"PSet":1,"RSet":1,"pass":0,"DotI":0,"PlL":9999,"PCS":10,"ID":1,"TS":10,"MM":0,"CMS":0,"QN":5,"GT":1,"STAN":1,"ILCH":410,"DFC":1000}

  if(obj["Command"].as<String>() == "Config"){ 
      if(LooklineDebug)LOGLN("Config Readding");
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
#define         NUM_SENSORS 20
typedef struct sensor_data {
  int boardid;
  int networkid;
  int plan;
  int result;
  byte status;
  time_t  timestamp;
  byte RSSI;
} sensor_data;
int sensors_saved = 0;
sensor_data     sensors[NUM_SENSORS];

void handleLooklineRaw() {
  sensors_saved = 4;
   for (int i = 0; i < sensors_saved; i++) {
    sensors[i].boardid = i;
    sensors[i].networkid = 1;
    sensors[i].status = random(1);
    sensors[i].plan = random(9999);
    sensors[i].result = random(9999);
    sensors[i].timestamp = random(9999);
    sensors[i].RSSI = random(100);
  }
  LOG ("/raw" + '\n');
  String raw;
  for (int i = 0; i < sensors_saved; i++) {
    raw += String(sensors[i].boardid) + ",";
    raw += String(sensors[i].networkid) + ",";
    raw += String(sensors[i].status) + ",";
    raw += String(sensors[i].plan) + ",";
    raw += String(sensors[i].result) + ",";
    raw += String(sensors[i].timestamp) + ",";
    raw += String(sensors[i].RSSI);
    if ( i < sensors_saved - 1) raw += '\n';
  }
  web_interface->web_server.send(200, "text/plain", raw);
}

///////////////////////////////////////////////// monitor
byte LOOKLINE_PROG::GetStart(){
  return Starting;
}
byte LOOKLINE_PROG::GetStatus(){
  return start;
}
void LOOKLINE_PROG::SetStart(byte START){// = 0 normal , =1 wait for update to web, = 2 run loop with socket
  start = START;LOGLN();LOGLN("Start:" + String(start));LOGLN();

// ESPCOM::println("Start: " + String(START), PRINTER_PIPE);
//   if(start == 2){//esp_wifi_set_protocol(current_wifi_interface, 3);
//     // if(oneceWifi){oneceWifi = false;LoRaLooklineSetup();LOGLN("OneceWifi");}
//     ComMode = 4;oneceMesh = true;
//   }
//   if(start == 0){//esp_wifi_set_protocol(current_wifi_interface, WIFI_PROTOCOL_LR);
//     if(MeshRun){oneceMesh = false;ComMode = MESH;}
//     if(oneceMesh){oneceMesh = false;MeshLookLineSetup();oneceWifi = true;LOGLN("onceMesh");}
//   }
}
void LOOKLINE_PROG::SetConfig(bool CONFIG){config = CONFIG;
  if(CONFIG == 1){
    #ifdef MASTER_MODBUS
    ComMode = LoRa;
    #else
    MeshLookLineSetup();LOGLN("MeshLooklineSetup");ComMode = MESH;
    #endif//MASTER_MODBUS
    CONFIG::write_byte(EP_EEPROM_COM_MODE, MESH);
  }
  if(CONFIG == 0){LoRaLooklineSetup();LOGLN("LoRaLooklineSetup");ComMode = LoRa;
    CONFIG::write_byte(EP_EEPROM_COM_MODE, LoRa);
  }
  
}
int LOOKLINE_PROG::Get_role(){
  return role;
}
void LOOKLINE_PROG::Set_Init_UI(String auths){LOGLN("Set_Init_UI " + auths);socket_server->broadcastTXT(auths);}



//////////////////////////////////////////////////////////////// GATEWAY MONITOR FUNCTIONS////////////////////////////////

void saveLooklineData(byte saveRSSI,byte saveID,byte saveNetID,byte saveState,int savePlan,int saveResult,byte Type,byte saveCom,byte saveWifi) {//Save Web

  new_Lookline_found = false;
  String msg = "Save Lookline " + String(saveID);
  // ESPCOM::println(msg, PRINTER_PIPE);
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
      // ESPCOM::println(msg, PRINTER_PIPE);
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