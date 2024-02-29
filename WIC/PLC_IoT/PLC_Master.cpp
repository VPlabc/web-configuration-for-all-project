// #22012024 UPDATE NEW
// #define SHT

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
#ifdef MQTT_USE
#include "MQTTcom.h"
MQTTCOM mqttPLC;
#ifdef MQTTSSL
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>
#else
#include <PubSubClient.h>
#endif
bool mqttWsConnected = false;
bool mqttConnected = false;
#endif//

#ifdef SHT
#include <SHT3x.h>
SHT3x Sensor;
#endif//#ifdef SHT
////////////////////////////////////////////////////////////////
#ifdef SDCARD_FEATURE
#include <SPI.h>
#include <SD.h>
#endif//SDCARD_FEATURE
////////////////////////////////////////////////////////////////

#include "WifiRF.h"
WifiRF wifirf;
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


unsigned long EVENT_INTERVAL_MS1 = 3000;
#ifdef MASTER_MODBUS
byte ModbusRole = master;
#else
byte ModbusRole = slave;
#endif//MASTER_MODBUS
byte connectWebSocket = 0;
byte IDList[255];
int16_t Register[4][60];

// int16_t SlaveParameter[HOLDING_REGS_SIZE];
// int16_t HOLDING_REGS_CoilData[HOLDING_REGS_SIZE];//1-9999
// int16_t HOLDING_REGS_InPutData[HOLDING_REGS_SIZE];//10001-19999
// int16_t HOLDING_REGS_AnalogInData[HOLDING_REGS_SIZE];//30001-39999
// int16_t HOLDING_REGS_AnalogOutData[HOLDING_REGS_SIZE];//40001-49999

  byte Lora_CH = 0;
  byte BoardIDs = 0;
  uint8_t M0 = 0;
  uint8_t M1 = 0;
  byte ComMode = LoRa;

bool WriteUpdate = 0;//bao cho ct biet la web co write xuong
uint16_t WriteUpAddr = 0;//dia chi khi web write


void PLC_MASTER::SetLoRaValue(){
CONFIG::SetLoRaValue();
}
        byte bbuf = 0;
void sendInfo();

void PLC_MASTER::setup(){
//   Serial.begin(115200);
//   Serial2.begin(9600);  

  Wire.begin();

  pinMode(LED_STATUS, OUTPUT);
  #ifndef MCP_USE
  pinMode(BTN_SET,    INPUT_PULLUP);
  #endif//MCP_USE
  pinMode(BootButton, INPUT_PULLUP);
  pinMode(SW_1,       OUTPUT);
  pinMode(SW_2,       OUTPUT);
  pinMode(IO1_HEADER, OUTPUT);
  pinMode(IO2_HEADER, OUTPUT);


    #ifdef USE_LORA
  LOGLN("LORA RF ________________________________________");

    M0 = SW_1; M1 = SW_2;
    // M0 = IO2_HEADER; M1 = IO1_HEADER;
  if(ComMode == LoRa){CONFIG::SetPinForLoRa(M0, M1, 16, 17);}
    SetLoRaValue();
  
  #endif//  #ifdef USE_LORA
  #ifdef ModbusCom
  LOGLN("MODBUS ________________________________________");
  if (!CONFIG::read_byte (EP_EEPROM_ROLE, &bbuf ) ) {ModbusRole = 1;} else {ModbusRole = bbuf;}
    PLCModbusCom.modbus_setup(ModbusRole);
    if(ModbusRole == master){LOG("Modbus Master ");PLCModbusCom.connectModbus(1);}
    if(ModbusRole == slave){LOG("Modbus Slave ");}
    LOGLN("Start!!");
#endif//ModbusCom 
#ifdef SDCARD_FEATURE
// byte csName[] ={4 ,5 ,12 ,14  ,33 ,32};
// for(byte j = 0; j < sizeof(csName); j++){
//     if (!SD.begin(csName[j])){LOGLN("SD Card not found |pin:" + String(csName[j]));}else{LOGLN("SD Card found |pin:" + String(csName[j]));}
//     delay(1000);
// }
  LOGLN("SD CARD ________________________________________");
  SPI.begin(SCLK, MISO, MOSI);
  if (!SD.begin(SDCard_CS)){
    LOGLN("Card Mount Failed");
  }else{
    LOGLN("Card Mount OK");
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
      return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
      Serial.println("MMC");
    } else if(cardType == CARD_SD){
      Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
  }



#endif//#ifdef SDCARD_FEATURE
  ///////////////////////////////

#ifdef SHT
  LOGLN("SHT Init ________________________________________");
  Sensor.Begin();
#endif//#ifdef SHT
#ifdef MQTT_USE
mqttPLC.setup();
#endif//MQTT_USE
  LOGLN("______________________________________________________");
  LOGLN("Setup PLC done");///////

}  
void PLC_MASTER::readfile(){
    Serial.println("Read file");
  }



bool UpdateFirmware = false;
void PLC_MASTER::UpdateFW(bool UDFW){UpdateFirmware = UDFW;LOGLN("disable for update fw")}

bool onceInfo = true;
void PLC_MASTER::loop(){// LOG("Loop");
if(UpdateFirmware==false){
#ifdef MQTT_USE
  #ifdef MQTTSSL
     if(WiFi.status() == WL_CONNECTED)webSocket.loop();
  #else
  if(connectWebSocket == 1 || connectWebSocket == 2)mqttPLC.loop();
  #endif
// webSocket.sendEVENT("hello");
#endif//MQTT_USE
   if(connectWebSocket == 1){PLCModbusCom.modbus_loop(ModbusRole);}
   
  if(PLCModbusCom.getModbusupdateState() == 1){// da co data tu web gui ve
    PLCModbusCom.setModbusupdateState(0);
    LOGLN( PLCModbusCom.getModbusupdateData());
    PLCModbusCom.Write_PLC(PLCModbusCom.getModbusupdateAddr(), PLCModbusCom.getModbusupdateData());
  }
#ifdef SHT
static unsigned long lastEventTime = millis();
// static unsigned long lastEventTimess = millis();
static const unsigned long EVENT_INTERVAL_MS = 1000;
// static const unsigned long EVENT_INTERVAL_MSs = 1000;
if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
  lastEventTime = millis();
#ifdef MQTT_USE
mqttConnected = mqttPLC.connect_state();
if((mqttConnected || mqttWsConnected)&&( connectWebSocket == 1 || connectWebSocket == 2)){
  Sensor.UpdateData();
  Serial.print("Temperature: ");
  Serial.print(Sensor.GetTemperature());
  Serial.write("\xC2\xB0"); //The Degree symbol
  Serial.println("C");
  Serial.print("Humidity: ");
  Serial.print(Sensor.GetRelHumidity());
  Serial.println("%");
  PLCModbusCom.holdingRegisters[0] = temperature;
  PLCModbusCom.holdingRegisters[2] = humidity;
  temperature = Sensor.GetTemperature();
  humidity = Sensor.GetRelHumidity();
  byte incomingnodeID;
   String sensor = "";
  // CONFIG::readtime();
  CONFIG::read_byte (EP_EEPROM_ID, &incomingnodeID);
  sensor = "{\"data\":[{\"id\":"+ String(incomingnodeID);
  sensor += ",\"name\": \"temperature\"";
  sensor += ",\"value\":" + String(temperature);
  sensor += ",\"unit\":\"calcius\"}";
  sensor += ",{\"id\":"+ String(incomingnodeID + 1);
  sensor += ",\"name\": \"humidity\"";
  sensor += ",\"value\":" + String(humidity);
  sensor += ",\"unit\":\"%\"}";
  sensor += "]";
  time_t now = time(nullptr);
  sensor += ",\"createAt\":\"" + String(ctime(&now));
  sensor += "\"}";
  LOGLN(sensor);
  #ifdef MQTTSSL
    if(!mqttWsConnected && WiFi.status() == WL_CONNECTED){webSocket.sendTXT(sensor);}
    #else
    if(mqttConnected && WiFi.status() == WL_CONNECTED){mqttPLC.mqttPublish(sensor);}
  #endif

  }
  #endif//MQTT_USE
}
#endif//#ifdef SHT

static unsigned long lastEventTime1 = millis();
if (((millis() - lastEventTime1) > EVENT_INTERVAL_MS1 )) {lastEventTime1 = millis();
  #if defined(MQTTSSL) && defined(MQTT_USE)
  if(!mqttConnected && connectWebSocket == 1 || connectWebSocket == 2){mqttPLC.mqttReconnect();}
  #endif
for(int i = 0 ; i < 30 ; i++) {Register[0][i] = 1;Register[2][i] = 1;Register[1][i] =  i;Register[3][i] = PLCModbusCom.holdingRegisters[i]; }
for(int i = 30 ; i < 60 ; i++) {Register[0][i] = 1;Register[2][i] = 1;Register[1][i] =  i;Register[3][i] = PLCModbusCom.getInputRegs()[i-30]; }
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
    if(connectWebSocket == 1){socket_server->broadcastTXT(json);sendInfo();}
}
// PLCModbusCom.modbus_read_update(HOLDING_REGS_AnalogOutData);

 }//UpdateFirmware
}

// void PLC_MASTER::modbusSet(uint16_t addr, uint16_t value){PLCModbusCom.inputRegisters[addr] = value;LOGLN("Modbus addr:"+String(addr)+" value:"+String(value));}
void sendInfo() {

  byte mac_address[6];
  WiFi.macAddress(mac_address);
  StaticJsonDocument<150> info_data;
  info_data["type"] = "info";
  info_data["version"] = PRGM_VERSION;
  info_data["wifi"] = String(WiFi.RSSI());
  if ((WiFi.getMode() == WIFI_STA) || (WiFi.getMode() == WIFI_AP_STA)) {
    if(WiFi.localIP().toString() == "0.0.0.0"){info_data["ip_address"] = WiFi.softAPIP().toString();}
      else{info_data["ip_address"] = WiFi.localIP().toString();}
  } else if (WiFi.getMode() == WIFI_AP) {info_data["ip_address"] = WiFi.softAPIP().toString();}
  // info_data["ip_address"] = WiFi.localIP().toString();
  info_data["mac_address"] = WiFi.macAddress();
  char sbuf[MAX_SSID_LENGTH + 1];
  CONFIG::read_string (EP_AP_SSID, sbuf, MAX_SSID_LENGTH);
  String rc(sbuf);byte ID;
  CONFIG::read_buffer (EP_EEPROM_ID,(byte *) &ID, INTEGER_LENGTH);
  info_data["WifiName"] = rc + " [" + String(ID) + "]";
  info_data["version"] = FRMW_VERSION;
  int baudRate = 0;
  if (!CONFIG::read_buffer (EP_BAUD_RATE,  (byte *) &baudRate, INTEGER_LENGTH)) {LOG ("Error read baudrate\r\n") }
  info_data["baud"] = baudRate;
  char   b[150];
  serializeJson(info_data, b); 
  socket_server->broadcastTXT(b);;
}

void PLC_MASTER::connectWeb(byte connected){
  LOGLN("Connected: "+String(connected));
  connectWebSocket = connected;
  PLCModbusCom.connectModbus(connected);
}

void PLC_MASTER::GetIdList(int idlist[]){
  for(byte i=0;i<sizeof(idlist);i++){
    IDList[i] = idlist[i];
  }
}
#endif//PLC_MASSTER_UI



