#include "config.h"
#ifdef PLC_MASTER_UI
#define ModbusCom
#include "webinterface.h"
#include "command.h"
// ESPResponseStream espresponses;
#include "espcom.h"

#include "WIC.h"
WIC looklineWIC;
#include "webinterface.h"

#include "wificonf.h"
WIFI_CONFIG WifiConFig;

#include "PLC_Master.h"
//  Command cmdLookLine;
 PLC_MASTER PLC_MASTER_PROG;
// #include "7SegModule.h"
#include "FirmwareUpdate.h"
UpdateFW UDFWLookLine;
#include "syncwebserver.h"
WebSocketsServer * socket_servers;
#ifdef ModbusCom
#include "Modbus_RTU.h"
Modbus_Prog PLCModbusCom;
#endif//ModbusCom 
#include "webinterface.h"
#ifdef CAPTIVE_PORTAL_FEATURE
#include <DNSServer.h>
DNSServer LooklinednsServer;
const byte DNS_PORT = 53;
#endif
#include "LoRa.h"

////////////////////////////////////////////////////////////////
#include <Wire.h>
/////////////////////////////////////////////////////////////////
#include <ClickButton.h>
ClickButton button(BootButton, LOW, CLICKBTN_PULLUP);

//   #include <SimpleModbusSlave.h>
//   SimpleModbusSlave NodeSlave;

#define             FRMW_VERSION         "1.2236"
#define             PRGM_VERSION         "1.0"
///////////////////////// Modbus Role //////////////////////////
enum {slave,master};
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
  Plan1,   
  Plan2,
  Plan3,
  Plan4,
  Plan5,
  Plan6,
  Plan7,
  Plan8,
  Plan9,
  Plan10,
  Plan11,
  Plan12,
  HOLDING_REGS_SIZE = 60// leave this one
};


#ifdef MASTER_MODBUS
byte ModbusRole = master;
#else
byte ModbusRole = slave;
#endif//MASTER_MODBUS
byte connectWebSocket = 0;
byte IDList[255];
int16_t Register[4][HOLDING_REGS_SIZE];

int16_t SlaveParameter[HOLDING_REGS_SIZE];
int16_t HOLDING_REGS_CoilData[HOLDING_REGS_SIZE];//1-9999
int16_t HOLDING_REGS_InPutData[HOLDING_REGS_SIZE];//10001-19999
int16_t HOLDING_REGS_AnalogInData[HOLDING_REGS_SIZE];//30001-39999
int16_t HOLDING_REGS_AnalogOutData[HOLDING_REGS_SIZE];//40001-49999

  byte Lora_CH = 0;
  byte BoardIDs = 0;
  uint8_t M0 = 0;
  uint8_t M1 = 0;
  byte ComMode = LoRa;

bool WriteUpdate = 0;//bao cho ct biet la web co write xuong
uint16_t WriteUpAddr = 0;//dia chi khi web write


void PLC_MASTER::SetLoRaValue(){
  CONFIG::read_byte(EP_EEPROM_ID, &BoardIDs);
  #ifdef USE_LORA
  CONFIG::read_byte(EP_EEPROM_CHANELS, &Lora_CH);
  WriteLoRaConfig(Lora_CH, BoardIDs);ReadLoRaConfig();
  #endif//  #ifdef USE_LORA
}
        byte bbuf = 0;
void sendInfo();
        
void PLC_MASTER::setup(){
//   Serial.begin(115200);
//   Serial2.begin(9600);  

  Wire.begin();

  pinMode(LED_STATUS, OUTPUT);
  pinMode(BTN_SET,    INPUT_PULLUP);
  pinMode(BootButton, INPUT_PULLUP);
  pinMode(SW_1,       OUTPUT);
  pinMode(SW_2,       OUTPUT);
  pinMode(IO1_HEADER, OUTPUT);
  pinMode(IO2_HEADER, OUTPUT);
  LOG("Setup PLC done");
    #ifdef USE_LORA
    // M0 = SW_1; M1 = SW_2;
    M0 = IO2_HEADER; M1 = IO1_HEADER;
  if(ComMode == LoRa){SetPinLoRa( M0,  M1,  16,  17);}
  // String para[4];
  // Lora_Config_update(para);
  // Str_Lora_CH = para[0];
  // Air_Rate = para[1];
  // Baud_Rate = para[2];
  // Lora_PWR = para[3];

  #endif//  #ifdef USE_LORA
  #ifdef ModbusCom
  if (!CONFIG::read_byte (EP_EEPROM_ROLE, &bbuf ) ) {} else {ModbusRole = bbuf;}
    PLCModbusCom.modbus_setup(ModbusRole);
    SetLoRaValue();
    if(ModbusRole == master){LOG("Modbus Master ");PLCModbusCom.connectModbus(1);}
    if(ModbusRole == slave){LOG("Modbus Slave ");}
    LOGLN("Start!!");
    
  //////////////////////////////////////

#endif//ModbusCom 
}
bool onceInfo = true;
void PLC_MASTER::loop(){// LOG("Loop");
looklineWIC.Auto = true;
  PLCModbusCom.modbus_loop(ModbusRole);
  if(PLCModbusCom.getModbusupdateState() == 1){// da co data tu web gui ve
    PLCModbusCom.setModbusupdateState(0);
    LOGLN( PLCModbusCom.getModbusupdateData());
    PLCModbusCom.Write_PLC(PLCModbusCom.getModbusupdateAddr(), PLCModbusCom.getModbusupdateData());
  }
static unsigned long lastEventTime = millis();
// static unsigned long lastEventTimess = millis();
static const unsigned long EVENT_INTERVAL_MS = 1000;
// static const unsigned long EVENT_INTERVAL_MSs = 1000;
if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
  lastEventTime = millis();


for(int i = 0 ; i < 30 ; i++) {Register[0][i] = 1;Register[2][i] = 1;Register[1][i] =  i;Register[3][i] = PLCModbusCom.holdingRegisters[i]; }
for(int i = 30 ; i < 60 ; i++) {Register[0][i] = 1;Register[2][i] = 1;Register[1][i] =  i;Register[3][i] = PLCModbusCom.getInputRegs()[i-30]; }
//         String Log = "Read Data\n";
//         Log += "ID: " + String(HOLDING_REGS_AnalogInData[BOARDID]) + " |";
//         Log += "Network: " + String(HOLDING_REGS_AnalogInData[NETID]) + " |";
//         Log += "Run/Stop: " + String(HOLDING_REGS_AnalogInData[RUNSTOP]) + " |";
//         Log += "On/Off: " + String(HOLDING_REGS_AnalogInData[ONOFF]) + " |";
//         Log += "Plan:" + String(HOLDING_REGS_AnalogInData[_PLAN]) + " |";
//         Log += "Plan set:" + String(HOLDING_REGS_AnalogInData[PLANSET]) + " |";
//         Log += "Result:" + String(HOLDING_REGS_AnalogInData[_RESULT]) + " |";
//         Log += "Result set:" + String(HOLDING_REGS_AnalogInData[RESULTSET]) + " |";
//         Log += "Plan Limit:" + String(HOLDING_REGS_AnalogInData[MAXPLAN]) + " |";
//         Log += "Pcs/h:" + String(HOLDING_REGS_AnalogInData[PCS]) + " |";
//         Log += "Time:" + String(HOLDING_REGS_AnalogInData[TIMEINC]) + " |";
//         Log += "Type:" + String(HOLDING_REGS_AnalogInData[TYPE]) + " |";
//         Log += "Role:" + String(HOLDING_REGS_AnalogInData[ROLE]) + " |";
//         Log += "OnWifi:" + String(HOLDING_REGS_AnalogInData[ONWIFI]) + " |";
//         Log += "Delay Count:" + String(HOLDING_REGS_AnalogInData[DELAYCOUNTER]) + " |";
//         Log += "Com mode:" + String(HOLDING_REGS_AnalogInData[COMMODE])  + "\n";
//         LOGLN(Log);
// LOGLN("Copy Done");
String json = "{";
//-------------------------
json += "\"Data\":0";
json += ",\"Regs\":[";//  "state":
json += "{\"RegID\":";
json += Register[0][0];
json += ",\"RegAddr\":";
json += Register[1][0];
json += ",\"RegType\":";
json += Register[2][0];
json += ",\"RegValue\":";
json += Register[3][0];
json += "}";
for(int i = 1 ; i < 60 ; i++){
json += ",{\"RegID\":";
json += Register[0][i];
json += ",\"RegAddr\":";
json += Register[1][i];
json += ",\"RegType\":";
json += Register[2][i];
json += ",\"RegValue\":";
json += Register[3][i];  
json += "}";
}
json += "],\"RegsList\":[";//  "state":
json += "{\"regs\":";
json += Register[1][0];;
json += "}";
for(int i = 1 ; i < 60 ; i++){//18191
json += ",{\"regs\":";
json += Register[1][i];
json += "}";
}
json += "],\"IDList\":[";//  "state":
json += "{\"id\":";
json += 0;
json += "}";
for(int i = 1 ; i < 10 ; i++){//18191
json += ",{\"id\":";
json += i;
json += "}";
}
json += "],\"NETList\":[";//  "state":
json += "{\"id\":";
json += 0;
json += "}";
for(int i = 1 ; i < 10 ; i++){//18191
json += ",{\"id\":";
json += i;
json += "}";
}
json += "]}";
// LOG(json);LOG(json);
    if(connectWebSocket == 1){socket_server->broadcastTXT(json);if(onceInfo){onceInfo = false;sendInfo();}}
// PLCModbusCom.modbus_loop(ModbusRole);
      // for (int i = 0; i < 30; i++){LOG(PLCModbusCom.inputRegisters[i]);LOGLN(" ");}


}
// PLCModbusCom.modbus_read_update(HOLDING_REGS_AnalogOutData);
}

// void PLC_MASTER::modbusSet(uint16_t addr, uint16_t value){PLCModbusCom.inputRegisters[addr] = value;LOGLN("Modbus addr:"+String(addr)+" value:"+String(value));}
void sendInfo() {

  byte mac_address[6];
  WiFi.macAddress(mac_address);
  StaticJsonDocument<150> info_data;
  info_data["type"] = "info";
  info_data["version"] = PRGM_VERSION;
  info_data["wifi"] = String(WiFi.RSSI());
  info_data["ip_address"] = WiFi.localIP().toString();
  info_data["mac_address"] = WiFi.macAddress();
  info_data["version"] = FRMW_VERSION;
  int baudRate = 0;
  if (!CONFIG::read_buffer (EP_BAUD_RATE,  (byte *) &baudRate, INTEGER_LENGTH)) {LOG ("Error read baudrate\r\n") }
  info_data["baud"] = baudRate;
  char   b[150];
  size_t len = serializeJson(info_data, b); 
  socket_server->broadcastTXT(b);;
}

void PLC_MASTER::connectWeb(byte connected){
  connectWebSocket = connected;
  PLCModbusCom.connectModbus(connected);
}

void PLC_MASTER::GetIdList(byte idlist[]){
  for(byte i=0;i<sizeof(idlist);i++){
    IDList[i] = idlist[i];
  }
}
#endif//PLC_MASSTER_UI



