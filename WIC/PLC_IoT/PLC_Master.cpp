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

///////////////////////// Modbus Role //////////////////////////
enum {master,slave};
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
  HOLDING_REGS_SIZE // leave this one
};
#ifdef MASTER_MODBUS
byte ModbusRole = master;
#else
byte ModbusRole = slave;
#endif//MASTER_MODBUS

byte Register[4][HOLDING_REGS_SIZE];

int SlaveParameter[HOLDING_REGS_SIZE];
int HOLDING_REGS_CoilData[HOLDING_REGS_SIZE];//1-9999
int HOLDING_REGS_InPutData[HOLDING_REGS_SIZE];//10001-19999
int HOLDING_REGS_AnalogInData[HOLDING_REGS_SIZE];//30001-39999
int HOLDING_REGS_AnalogOutData[HOLDING_REGS_SIZE];//40001-49999

  byte Lora_CH = 0;
  byte BoardIDs = 0;
  uint8_t M0 = 0;
  uint8_t M1 = 0;
  byte ComMode = LoRa;

void PLC_MASTER::SetLoRaValue(){
  CONFIG::read_byte(EP_EEPROM_ID, &BoardIDs);
  #ifdef USE_LORA
  CONFIG::read_byte(EP_EEPROM_CHANELS, &Lora_CH);
  WriteLoRaConfig(Lora_CH, BoardIDs);ReadLoRaConfig();
  #endif//  #ifdef USE_LORA
}
void PLC_MASTER::setup(){
//   Serial.begin(115200);
//   Serial2.begin(9600);  

  Wire.begin();

  pinMode(LED_STATUS, OUTPUT);
  pinMode(BTN_SET,    INPUT_PULLUP);
  pinMode(BootButton,   INPUT_PULLUP);
  pinMode(SW_1,       OUTPUT);
  pinMode(SW_2,       OUTPUT);
  LOG("Setup PLC done");
    #ifdef USE_LORA
    M0 = SW_1; M1 = SW_2;
  if(ComMode == LoRa){SetPinLoRa( M0,  M1,  16,  17);}
  #endif//  #ifdef USE_LORA
  #ifdef ModbusCom
    PLCModbusCom.modbus_setup(ModbusRole);
    SetLoRaValue();
    LOG("Modbus Start!!");
  //////////////////////////////////////
#endif//ModbusCom 
}
void DataInput(){    
       if(ModbusRole == slave) {
          HOLDING_REGS_AnalogOutData[BOARDID] = 1;
          HOLDING_REGS_AnalogOutData[NETID] = 2;
          HOLDING_REGS_AnalogOutData[RUNSTOP] = random(1,0);
          HOLDING_REGS_AnalogOutData[ONOFF] = random(0,100);
          HOLDING_REGS_AnalogOutData[_PLAN] = random(0,100);
          HOLDING_REGS_AnalogOutData[PLANSET] = random(0,100);
          HOLDING_REGS_AnalogOutData[_RESULT] = random(0,100);
          HOLDING_REGS_AnalogOutData[RESULTSET] = random(0,100);
          HOLDING_REGS_AnalogOutData[MAXPLAN] = random(0,100);
          HOLDING_REGS_AnalogOutData[PCS] = random(0,100);
          HOLDING_REGS_AnalogOutData[TIMEINC] = random(0,100);
          HOLDING_REGS_AnalogOutData[DELAYCOUNTER] =  random(0,100);
          HOLDING_REGS_AnalogOutData[ROLE] =  random(0,100);
          HOLDING_REGS_AnalogOutData[_RSSI] =  random(0,100);//DataLookline.RSSI;
          HOLDING_REGS_AnalogOutData[COMMODE] = random(0,100);
          HOLDING_REGS_AnalogOutData[TYPE] =  random(0,100);
          HOLDING_REGS_AnalogOutData[ONWIFI] =  17;
          HOLDING_REGS_AnalogOutData[CMD] =  18;  
        // LOG("Read: ");for(byte i=0;i < HOLDING_REGS_SIZE; i++){LOG("|");LOG(String(HOLDING_REGS_AnalogOutData[i]));}LOGLN();
        

        #ifdef ModbusCom
          PLCModbusCom.modbus_write_update(HOLDING_REGS_AnalogOutData);//UpdateParamOnce = true;
        #endif//ModbusCom
        }
        if(ModbusRole == master){
          HOLDING_REGS_AnalogOutData[BOARDID] = 17;
          HOLDING_REGS_AnalogOutData[NETID] = 16;
          HOLDING_REGS_AnalogOutData[RUNSTOP] = 15;
          HOLDING_REGS_AnalogOutData[ONOFF] = 14;
          HOLDING_REGS_AnalogOutData[_PLAN] = 13;
          HOLDING_REGS_AnalogOutData[PLANSET] = 12;
          HOLDING_REGS_AnalogOutData[_RESULT] = 11;
          HOLDING_REGS_AnalogOutData[RESULTSET] = 10;
          HOLDING_REGS_AnalogOutData[MAXPLAN] = 9;
          HOLDING_REGS_AnalogOutData[PCS] = 8;
          HOLDING_REGS_AnalogOutData[TIMEINC] = 7;
          HOLDING_REGS_AnalogOutData[DELAYCOUNTER] =  6;
          HOLDING_REGS_AnalogOutData[ROLE] =  5;
          HOLDING_REGS_AnalogOutData[_RSSI] =  4;//DataLookline.RSSI;
          HOLDING_REGS_AnalogOutData[COMMODE] = 3;
          HOLDING_REGS_AnalogOutData[TYPE] =  2;
          HOLDING_REGS_AnalogOutData[ONWIFI] =  1;
          HOLDING_REGS_AnalogOutData[CMD] =  0; 
        #ifdef ModbusCom
          PLCModbusCom.modbus_write_update(HOLDING_REGS_AnalogOutData);//UpdateParamOnce = true;
        #endif//ModbusCom
        }

}

void PLC_MASTER::loop(){// LOG("Loop");
   PLCModbusCom.modbus_loop(ModbusRole);

static unsigned long lastEventTime = millis();
static unsigned long lastEventTimess = millis();
static const unsigned long EVENT_INTERVAL_MS = 3000;
static const unsigned long EVENT_INTERVAL_MSs = 1000;
if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
  lastEventTime = millis();
// LED_ON(LED_STATUS); 
//           WebData += String(Looklines[i].nodeID) + ",";
//Register[0][0]  ID
//Register[1][0]  Address
//Register[2][0]  Type
//Register[3][0]  Value

}
if ((millis() - lastEventTimess) > EVENT_INTERVAL_MSs) {
    lastEventTimess = millis();
 LED_OFF(LED_STATUS);
PLCModbusCom.debugs();
// DataInput();
// LOGLN("Copy register");
// PLCModbusCom.GetData(HOLDING_REGS_AnalogInData);
for(int i = 0 ; i < HOLDING_REGS_SIZE ; i++) {Register[0][i] = 1;Register[2][i] = 1;Register[1][i] =  i;Register[3][i] = HOLDING_REGS_AnalogInData[i]; }
        String Log = "Read Data\n";
        Log += "ID: " + String(HOLDING_REGS_AnalogInData[BOARDID]) + " |";
        Log += "Network: " + String(HOLDING_REGS_AnalogInData[NETID]) + " |";
        Log += "Run/Stop: " + String(HOLDING_REGS_AnalogInData[RUNSTOP]) + " |";
        Log += "On/Off: " + String(HOLDING_REGS_AnalogInData[ONOFF]) + " |";
        Log += "Plan:" + String(HOLDING_REGS_AnalogInData[_PLAN]) + " |";
        Log += "Plan set:" + String(HOLDING_REGS_AnalogInData[PLANSET]) + " |";
        Log += "Result:" + String(HOLDING_REGS_AnalogInData[_RESULT]) + " |";
        Log += "Result set:" + String(HOLDING_REGS_AnalogInData[RESULTSET]) + " |";
        Log += "Plan Limit:" + String(HOLDING_REGS_AnalogInData[MAXPLAN]) + " |";
        Log += "Pcs/h:" + String(HOLDING_REGS_AnalogInData[PCS]) + " |";
        Log += "Time:" + String(HOLDING_REGS_AnalogInData[TIMEINC]) + " |";
        Log += "Type:" + String(HOLDING_REGS_AnalogInData[TYPE]) + " |";
        Log += "Role:" + String(HOLDING_REGS_AnalogInData[ROLE]) + " |";
        Log += "OnWifi:" + String(HOLDING_REGS_AnalogInData[ONWIFI]) + " |";
        Log += "Delay Count:" + String(HOLDING_REGS_AnalogInData[DELAYCOUNTER]) + " |";
        Log += "Com mode:" + String(HOLDING_REGS_AnalogInData[COMMODE])  + "\n";
        LOGLN(Log);
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
for(int i = 1 ; i < 82 ; i++){
json += ",{\"RegID\":";
json += Register[0][i];
json += ",\"RegAddr\":";
json += Register[1][i];
json += ",\"RegType\":";
json += Register[2][i];
json += ",\"RegValue\":";
if(Register[3][i] > 100){Register[3][i] = 0;}
json += Register[3][i];  
json += "}";
}
json += "],\"RegsList\":[";//  "state":
json += "{\"regs\":";
json += Register[1][0];;
json += "}";
for(int i = 1 ; i < 82 ; i++){//18191
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
// LOG(json);
          // socket_server->broadcastTXT(json);
// PLCModbusCom.modbus_loop(ModbusRole);

}
// PLCModbusCom.modbus_read_update(HOLDING_REGS_AnalogOutData);
}


#endif//PLC_MASSTER_UI



