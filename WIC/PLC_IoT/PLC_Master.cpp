// #22012024 UPDATE NEW
// #define SHT
#define MeshNetwork
#define DataLog

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

#include "SDFunction.h"
SDFunction sdFunction;
#include "RealTimeClock.h"

#ifdef DataLog
#include "DataLog.h"
#endif//DataLog

#ifdef MeshNetwork
// #include "MeshWifi.h"
#include "Mesh.h"
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

#include "DataTransfer/data_transfer.h"
DataTransfer plcDataTrans;
//   #include <SimpleModbusSlave.h>
//   SimpleModbusSlave NodeSlave;

#define             FRMW_VERSION         "1.2236"
#define             PRGM_VERSION         "1.0"
///////////////////////// Modbus Role //////////////////////////
enum {slave,master};
//////////////// registers of your slave ///////////////////


unsigned long EVENT_INTERVAL_MS1 = 3000;
unsigned long REFRESH_INTERVAL_MS = 600000;
#ifdef MASTER_MODBUS
byte MBRole = master;
#else
byte MBRole = slave;
#endif//MASTER_MODBUS
byte CharterQuality = 40; 
byte connectWebSocket = 0;
byte IDList[255];
uint16_t Register[4][200];
byte ProductName[4][40];
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

String Webmessage = "";
void PLC_MASTER::SocketRecive(uint8_t *Payload){
    Webmessage = String((char*)Payload);
    if (Webmessage.indexOf("data:") >= 0) {
      String dataRevice = Webmessage.substring(5);
      // Serial.println("json:" + dataRevice);
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, dataRevice);
        JsonObject obj = doc.as<JsonObject>();
        //{"id":1,"type":0,"week":0,"date":"2024-03-15"}
        String date = obj["date"];byte type = obj[String("type")];int id = obj[String("id")];
        date.replace("-", "_");
        String FileName = "/" + String(id) + "_DataLog_" + date + ".csv";
        // LOGLN("file name: " + FileName);
        DLGreadFile(SD, FileName, type);
    }
}


void PLC_MASTER::SetLoRaValue(){
CONFIG::SetLoRaValue();
}
        byte bbuff = 0;
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
  if (!CONFIG::read_byte (EP_EEPROM_ROLE, &bbuff ) ) {MBRole = 1;} else {MBRole = bbuff;}
    PLCModbusCom.modbus_setup(MBRole);
    if(MBRole == master){LOG("Modbus Master ");PLCModbusCom.connectModbus(1);}
    if(MBRole == slave){LOG("Modbus Slave ");}
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
#ifdef TIMESTAMP_FEATURE
  LOGLN("Time Init ________________________________________");
// SetupTime();
updateTime();
  LOGLN("File Init ________________________________________");
  if(!SD.begin(SDCard_CS)){LOG ("Card Mount Failed\n");return; }
  String FileName = "DataLog_20_03_24.csv";
  if(!SD.exists(FileName)){writeFile(SD, FileName, "Creat file");LOGLN("File Created ");}

#endif////////////////
#ifdef MeshNetwork
Mesh_setup();
#endif//#ifdef MeshNetwork
  LOGLN("______________________________________________________");
  LOGLN("Setup PLC done");///////

}  
String PLC_MASTER::readfile(){
    Serial.println("Read file");

}


bool WebSendata = false;  
bool UpdateFirmware = false;
unsigned long lastRefresh1 = millis();
void PLC_MASTER::UpdateFW(bool UDFW){UpdateFirmware = UDFW;static bool once=true;if(once){once = false;LOGLN("disable for update fw")}}

bool onceInfo = true;
void PLC_MASTER::loop(){// LOG("Loop");
//Refresh wifi
if(UpdateFirmware==false){
  
  if(connectWebSocket == 0){
    #ifndef MeshNetwork
    static unsigned long lastRefresh1 = millis();
    if (((millis() - lastRefresh1) > REFRESH_INTERVAL_MS ) && WiFi.status() != WL_CONNECTED){lastRefresh1 = millis();
      WiFi.disconnect();WiFi.mode(WIFI_OFF);
      wifi_config.Setup(true, LED_STATUS, 1);LOGLN("Refresh Wifi");
    }
    #endif//MeshNetwork
    static unsigned long previousMillis = 0;
    if (millis() - previousMillis >= 100 ) {digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));previousMillis = millis();}
  }
  if(connectWebSocket == 1){
    static unsigned long previousMillis = 0;
    if (millis() - previousMillis >= 300 ) {digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));previousMillis = millis();}
  }
/////
#ifdef MQTT_USE
  #ifdef MQTTSSL
     if(WiFi.status() == WL_CONNECTED)webSocket.loop();
  #else
  if(connectWebSocket == 1 || connectWebSocket == 2)mqttPLC.loop();
  #endif
// webSocket.sendEVENT("hello");
#endif//MQTT_USE

#ifdef MeshNetwork
Mesh_loop();
#endif//#ifdef MeshNetwork
  //  if(connectWebSocket == 1 && WebSendata == 0){
    PLCModbusCom.modbus_loop(MBRole);//}
   
  // if(PLCModbusCom.getModbusupdateState() == 1){// da co data tu web gui ve
  //   PLCModbusCom.setModbusupdateState(0);
  //   // LOGLN( PLCModbusCom.getModbusupdateData());
  //   PLCModbusCom.Write_PLC(PLCModbusCom.getModbusupdateAddr(), PLCModbusCom.getModbusupdateData());
  // }
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

    time_t Nows = timeClient.getEpochTime();
    timeClient.update();
    time_t Times = time(nullptr);
    // LOGLN("Data Log: " + String(ctime(&Times)) + " | " + String(Nows));
  #if defined(MQTTSSL) && defined(MQTT_USE)
  if(!mqttConnected && connectWebSocket == 1 || connectWebSocket == 2){mqttPLC.mqttReconnect();}
  #endif///////////////////////////// ID Slave ////////// Type ////////////// Address ////////////// Data  //////////////////////
for(int i = 0 ; i < 120 ; i++) {Register[0][i] = 1;Register[2][i] = 2;Register[1][i] =  i;Register[3][i] = PLCModbusCom.holdingRegisters[i]; }
// for(int i = 100 ; i < 200 ; i++) {Register[0][i] = 1;Register[2][i] = 1;Register[1][i] =  i;Register[3][i] = PLCModbusCom.getInputRegs()[i-100]; }
RespondChar decod;byte charterOffset = 43;

for(int j = 0 ; j < 4 ; j++) {
  for(int i = 0 ; i < CharterQuality/2 ; i++) {
    decod = plcDataTrans.DecodeWord(PLCModbusCom.holdingRegisters[(charterOffset+i)+(j*(CharterQuality/2))]);
    if(decod.char1!=0){ProductName[j][i*2] = decod.char1;}
    if(decod.char2!=0){ProductName[j][i*2+1] = decod.char2;}
  }
  // LOGLN();
}

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
for(int i = 1 ; i < 120 ; i++){
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
for(int i = 1 ; i < 200 ; i++){//18191
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

json += "],\"ProductList\":[";//  "state":
json += "{\"id\":";
json += 0 + charterOffset;
json += ",\"Name\":\" ";
  for(int k = 0 ; k < CharterQuality ; k++){if(ProductName[0][k]!=0){json += (char)ProductName[0][k];}else{json += ' ';}}
json += "\"}";
for(int i = 1 ; i < 4 ; i++){//18191
json += ",{\"id\":";
json += (i*CharterQuality/2) + charterOffset;
json += ",\"Name\":\" ";
for(int k = 0 ; k < CharterQuality ; k++){if(ProductName[i][k]!=0){json += (char)ProductName[i][k];}else{json += ' ';}}
json += "\"}";
}

json += "]}";
// LOG(json);
    if(connectWebSocket == 1){WebSendata = 1;socket_server->broadcastTXT(json);sendInfo();WebSendata = 0;}
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
  socket_server->broadcastTXT(b);
}

void PLC_MASTER::connectWeb(byte connected){
  LOGLN("Connected: "+String(connected));
  connectWebSocket = connected;
  PLCModbusCom.connectModbus(connected-2);
  lastRefresh1 = millis();
}

void PLC_MASTER::GetIdList(int idlist[]){
  // for(byte i=0;i<sizeof(idlist);i++){
  //   IDList[i] = idlist[i];
  // }
}
#endif//PLC_MASSTER_UI



