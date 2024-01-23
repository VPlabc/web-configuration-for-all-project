// #22012024 UPDATE NEW
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


////////////////////////////////////////////////////////////////
#ifdef SDCARD_FEATURE
#include <SPI.h>
#include <SD.h>
#include "FS.h"
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
  LOGLN("_________________________________________ LORA RF ________________________________________");

    // M0 = SW_1; M1 = SW_2;
    M0 = IO2_HEADER; M1 = IO1_HEADER;
  if(ComMode == LoRa){CONFIG::SetPinForLoRa(M0, M1, 16, 17);}
    SetLoRaValue();
  // String para[4];
  // Lora_Config_update(para);
  // Str_Lora_CH = para[0];
  // Air_Rate = para[1];
  // Baud_Rate = para[2];
  // Lora_PWR = para[3];
  
  #endif//  #ifdef USE_LORA
  #ifdef ModbusCom
  LOGLN("_________________________________________ MODBUS ________________________________________");
  if (!CONFIG::read_byte (EP_EEPROM_ROLE, &bbuf ) ) {} else {ModbusRole = bbuf;}
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
  LOGLN("_________________________________________ SD CARD ________________________________________");
  SPI.begin(SCLK, MISO, MOSI);
  SPI.setClockDivider(SPI_CLOCK_DIV32);
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
  // // Create a file on the SD card and write the data labels
  // File file = SD.open("/data.txt");
  // if(!file) {
  //   Serial.println("File doens't exist");
  //   Serial.println("Creating file...");
  //   writeFile(SD, "/data.txt", "Reading ID, Date, Hour, Temperature \r\n");
  // }
  // else {
  //   Serial.println("File already exists");  
  // }
  // file.close(); 
#endif//#ifdef SDCARD_FEATURE
  ///////////////////////////////

  LOGLN("____________________________________________________________________________________________");
  LOGLN("Setup PLC done");///////

}
bool onceInfo = true;
 byte cardNumber = 4;
 uint16_t Plan = 0;
 uint16_t Result = 0;
 uint32_t sumPlan = 0;
 uint32_t sumResult = 0;
 uint16_t averagePlan = 0;
 uint16_t averageResult = 0;

 time_t time_log;
void PLC_MASTER::loop(){// LOG("Loop");

   PLCModbusCom.modbus_loop(ModbusRole);
   
  if(PLCModbusCom.getModbusupdateState() == 1){// da co data tu web gui ve
    PLCModbusCom.setModbusupdateState(0);
    LOGLN( PLCModbusCom.getModbusupdateData());
    PLCModbusCom.Write_PLC(PLCModbusCom.getModbusupdateAddr(), PLCModbusCom.getModbusupdateData());
  }
  static unsigned long lastEventTime1 = millis();
// static unsigned long lastEventTimess = millis();
static const unsigned long EVENT_INTERVAL_MS1 = 1000;
// static const unsigned long EVENT_INTERVAL_MSs = 1000;
if ((millis() - lastEventTime1) > EVENT_INTERVAL_MS1) {
  lastEventTime1 = millis();

for(byte i = 0 ; i < cardNumber ; i++){
  Plan = random(0,9999);//PLCModbusCom.holdingRegisters[0+7*i];
  Result = random(0,9999);//PLCModbusCom.holdingRegisters[2+7*i];
 logSDCard( i, time_log,  Plan,  Result);
}
}
static unsigned long lastEventTime = millis();
// static unsigned long lastEventTimess = millis();
static const unsigned long EVENT_INTERVAL_MS = 1000;
// static const unsigned long EVENT_INTERVAL_MSs = 1000;
if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
  lastEventTime = millis();
 time_log++;
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
    if(connectWebSocket == 1){socket_server->broadcastTXT(json);sendInfo();}
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
  if ((WiFi.getMode() == WIFI_STA) || (WiFi.getMode() == WIFI_AP_STA)) {
    if(WiFi.localIP().toString() == "0.0.0.0"){info_data["ip_address"] = WiFi.softAPIP().toString();}
      else{info_data["ip_address"] = WiFi.localIP().toString();}
  } else if (WiFi.getMode() == WIFI_AP) {info_data["ip_address"] = WiFi.softAPIP().toString();}
  // info_data["ip_address"] = WiFi.localIP().toString();
  info_data["mac_address"] = WiFi.macAddress();
  info_data["version"] = FRMW_VERSION;
  int baudRate = 0;
  if (!CONFIG::read_buffer (EP_BAUD_RATE,  (byte *) &baudRate, INTEGER_LENGTH)) {LOG ("Error read baudrate\r\n") }
  info_data["baud"] = baudRate;
  char   b[150];
  serializeJson(info_data, b); 
  socket_server->broadcastTXT(b);;
}

void PLC_MASTER::connectWeb(byte connected){
  connectWebSocket = connected;
  PLCModbusCom.connectModbus(connected);
}

void PLC_MASTER::GetIdList(int idlist[]){
  for(byte i=0;i<sizeof(idlist);i++){
    IDList[i] = idlist[i];
  }
}
void deleteFile(fs::FS &fs, const char * path);
// // Function to get temperature
// void PLC_MASTER::getReadings(){
//   sensors.requestTemperatures(); 
//   temperature = sensors.getTempCByIndex(0); // Temperature in Celsius
//   //temperature = sensors.getTempFByIndex(0); // Temperature in Fahrenheit
//   Serial.print("Temperature: ");
//   Serial.println(temperature);
// }

// // Function to get date and time from NTPClient
// void PLC_MASTER::getTimeStamp() {
//   while(!timeClient.update()) {
//     timeClient.forceUpdate();
//   }
//   // The formattedDate comes with the following format:
//   // 2018-05-28T16:00:13Z
//   // We need to extract date and time
//   formattedDate = timeClient.getFormattedDate();
//   Serial.println(formattedDate);

//   // Extract date
//   int splitT = formattedDate.indexOf("T");
//   dayStamp = formattedDate.substring(0, splitT);
//   Serial.println(dayStamp);
//   // Extract time
//   timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
//   Serial.println(timeStamp);
// }
void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    LOG ("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

// Write the sensor readings on the SD card
void PLC_MASTER::logSDCard(byte card, time_t time, uint16_t Plan, uint16_t Result) {
  String dataMessage="";
  // Serial.print("Save data: ");
    dataMessage = String(time)+','+String(Plan)+','+String(Result) + ',';
    // LOGLN(dataMessage);
    // char databuff[150];dataMessage.toCharArray(databuff, sizeof(dataMessage));
    // LOGLN("size String: " + String(dataMessage.length()));
    File file = SD.open(("/data" + String(card) + ".txt").c_str(), FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.println(dataMessage)) {
    // Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
    // appendFile(SD, ("/data"+String(card)+".txt").c_str(), databuff);
}
String PLC_MASTER::loadSDCard(int card, int start, int end){
  LOGLN("Card: " + String(card));
  String DataOut="";
  String DataFilter = "";
  String DataOutFilter = "";
  String DataOutArray[3];
  uint32_t count1 = 0;
  float_t planSum = 0;
  float_t resultSum = 0;
  uint32_t planCount = 0;
  uint32_t resultCount = 0;
  uint16_t maxPlan = 0;
  uint16_t minPlan = 0;
  uint16_t maxResult = 0;
  uint16_t minResult = 0;
  byte count = 0;
  File dataFile = SD.open("/data" + String(card) + ".txt");
  if (!dataFile) {
    LOGLN("Failed to open file");
  }
  else{
     uint16_t startTime = millis();
    while(dataFile.available()){
      count1++;
      // Serial.write(dataFile.read());
      char charin = (char)dataFile.read();
      DataOut += charin;
      // if(charin == ',')LOGLN(charin)
      if(charin == ','){
        DataOutArray[count] = DataFilter;
        count++;
        DataFilter = "";
        if(count > 2){count = 0;
        if(DataOutArray[0].toInt() >= start && DataOutArray[0].toInt() <= end  ){
          DataOutFilter += "Time: " + DataOutArray[0] + " | Plan: " + DataOutArray[1] + " | Result: " + DataOutArray[2] + '\n';
          planSum += DataOutArray[1].toInt();
          planCount++;
          resultSum += DataOutArray[2].toInt();
          resultCount++;
          // if()
        }

        }
      }else{
        if(charin == '\n'){}else{DataFilter += charin;}
      }
    }
     uint16_t endTime = millis();
    // LOG(DataOut);
    dataFile.close();
    LOG(DataOutFilter);
    LOGLN("Time proccess:  " + String((endTime - startTime)) + " Ms  /  " + count1 + " records");
    LOGLN("Number of valid plans: " + String(planCount));
    LOGLN("Number of valid results: " + String(resultCount));
    LOGLN("Average Plan: " + String(planSum/planCount));
    LOGLN("Average Result: " + String(resultSum/resultCount));
  }

  return DataOut;
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    LOG ("File deleted");
  } else {
    LOG ("Delete failed");
  }
}

void PLC_MASTER::DelSDCard(int card){
deleteFile(SD, ("/data" + String(card) + ".txt").c_str());
}


// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void PLC_MASTER::writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void PLC_MASTER::appendFile(fs::FS &fs, const char * path, const char * message) {
  // Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.println(message)) {
    // Serial.println("Message appended");
  } else {
    // Serial.println("Append failed");
  }
  file.close();
}
// std::pair<float, float> calculateAverage(String DataOutArray[], int size) {
//   float sumPlan = 0;
//   float sumResult = 0;
//   float averagePlan = 0;
//   float averageResult = 0;
//   int countav = 0;

//   for (int i = 0; i < size; i++) {
//     if (DataOutArray[i] != "") {
//       sumPlan += DataOutArray[i].substring(0, DataOutArray[i].indexOf("|")).toFloat();
//       sumResult += DataOutArray[i].substring(DataOutArray[i].indexOf("|") + 2).toFloat();
//       countav++;
//     }
//   }

//   if (countav != 0) {
//     averagePlan = sumPlan / countav;
//     averageResult = sumResult / countav;
//   }

//   return std::make_pair(averagePlan, averageResult);
// }

// int size = sizeof(DataOutArray) / sizeof(DataOutArray[0]);
// std::pair<float, float> avg = calculateAverage(DataOutArray, size);
// float averagePlan = avg.first;
// float averageResult = avg.second;
// LOG("Gia tri trung binh cua Plan: " + String(averagePlan));
// LOG("Gia tri trung binh cua Result: " + String(averageResult));
#endif//PLC_MASSTER_UI





