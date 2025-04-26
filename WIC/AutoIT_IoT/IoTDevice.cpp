// #01012023: update
// MQTT pub/sub
// node mesh master/slave
// gateway master

//Node Connector 
//CONNECTOR
//RS485 Connect
  //GND --- 1
  //A+  --- 2
  //B-  --- 3
//IO Connector


#include <Arduino.h>
#include "config.h"
#include "espcom.h"
#include "esp_oled.h"
#include "wificonf.h"
#include "FirmwareUpdate.h"
UpdateFW UDFW;
UpdateFW UPDATEFWS;
#ifdef IOTDEVICE_UI
#include "MeshWifi.h"
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
//______ Include
#include "IoTDevice.h" 
IoT_Device IOT_DEVICE_;
#include "MQTTcom.h"
MQTTCOM mqttcommu;
#include <PubSubClient.h>


//________ Sensor
#include "DHTesp.h"
DHTesp dht;
#define DHTPin 32
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire ds18x20[] = { 32, 33, 34, 35 };
const int oneWireCount = sizeof(ds18x20) / sizeof(OneWire);
DallasTemperature sensor[oneWireCount];

const byte AnalogPins[] = { 32, 33, 34, 35 ,36 ,39};

#define SWITCHPIN 32
#define RELAYPIN 33
#define  ANALOGPIN1 36
#define ANALOGPIN2 39
// #include "WifiFunc.h"
// #include "updatefirmware.h"
#define MESH_ID               6734922
#define GROUP_SWITCH          1
#define GROUP_HT              2
#define GROUP_MOTION          3
#define GROUP_RELAY           4
#define GROUP_MOISTURE        5
#define GROUP_TEMP            6
#define GROUP_VALVE           7
#define GROUP_POSITION        8
#define GROUP_SHT             9
#define battery_cutoff_volt   3.3

// Define variables to store BME280 readings to be sent
byte networkID;
byte nodeID;
byte Status;
byte state;
byte category;
float temperature;
float humidity;
float mbattery;
float battery;
byte OLED_MODE = 0;

byte ButtonUpdateFw = 0;
byte UpdateFw = 0;
byte ButtonPin = 15;
// byte ButtonPin = 13;
byte LEDPin = 2;
String NameBoard = "";
String mqtt_server = "";
String MQTTPort = "";
String TopicOut = "";
String TopicIn = "";
bool checkFW = true;
byte ModeRun1 = 0;
byte Result = 0;
String FW_version;
int ConnectRetry = 0;//Wifi connect retry

String MasterStatus = "STT";
// Define variables to store incoming readings
byte incomingnetworkID;
byte incomingnodeID;
byte incomingCatagory;
byte incomingstatus;
float incomingtemperature;
float incominghumidity;
float incomingmbattery;
float incomingbattery;
int incomingrssi_display;

int function = 0;
int function1 = 0;
long count = 0;byte Menu = 0;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    byte networkID;       //1
    byte nodeID;          //1
    byte category;        //1
    byte status;          //1
    float temperature;    //4
    float humidity;       //4
    float mbattery;       //4
    float battery;        //4
    int RSSI;             //4
} struct_message;




typedef struct struct_command_message {
    byte Command;       //1
    byte networkID;       //1
    byte nodeID;          //1
    byte category;        //1
    byte time;            //1
} struct_command_message;


struct_command_message DataCommand;
// Create a struct_message called BME280Readings to hold sensor readings
struct_message DataSenddings;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

int             sensors_saved = 0;
uint8_t         incomingData[sizeof(struct struct_message)];
uint8_t         isDataCommand[sizeof(struct struct_command_message)];
size_t          received_msg_length;
bool            new_sensor_found;

// #define OLED_SH1106
// #define OLED_SSD1306
#ifdef OLED_SH1106
#include <SH1106.h>
// Initialize the OLED display using Wire library
SH1106  display(0x3C, 21, 22);
#endif//OLED_SH1106
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#ifdef OLED_SSD1306
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#endif//OLED_SSD1306
//---------------------------------   Hardware
#define BUTTON_PIN1   13
#define BUTTON_PIN   0
#define PIXEL_PIN    2  // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 8  // Number of NeoPixels
#define Status_LED 12

#define  Debug_Ser Serial
/////////////////////////////////////////////////////////////////
#include <ClickButton.h>
ClickButton button(BUTTON_PIN, LOW, CLICKBTN_PULLUP);
ClickButton button1(BUTTON_PIN1, LOW, CLICKBTN_PULLUP);
/////////////////////////////////////////////////////////////////
//---------------------------------   Neopixel
// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 50 // Set BRIGHTNESS to about 1/5 (max = 255)
#define BRIGHTNESS_HEART 100 // Set BRIGHTNESS to about 1/5 (max = 255)
// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

  // ledFadeToBeat(255,200,0,BRIGHTNESS_HEART);                    // Routine that fades color intensity to the beat
  // ledFadeToBeat(0,255,0,BRIGHTNESS_HEART);                    // Routine that fades color intensity to the beat
  //   strip.setBrightness(BRIGHTNESS); 
  // larson_scan();
  // // theaterChase(strip.Color(127, 127, 127), 150); // White
// theaterChaseRainbow(150);
  //----------------------------------------------------------------
int     mode     = 0;    // Currently-active animation mode, 0-9
int pos = 0, dir = 1; // Position, direction of "eye"


bool STT = true;

// if ( incomingCatagory == 0 && incomingstatus == 0 ){LOG("Reciver: mesh"); ESPCOM::print("Reciver: mesh", WS_PIPE);}
// if ( incomingCatagory == 0 && incomingstatus == 1 ){ LOG("Reciver: online");  ESPCOM::print("Reciver: online", WS_PIPE);}
// if ( incomingCatagory == 1 && incomingstatus == 0 ){ LOG("Reciver: update");  ESPCOM::print("Reciver: update", WS_PIPE);}
void IoT_Device::sendCurrentStatus(byte status) {
    DataSenddings.networkID = 0;DataSenddings.nodeID = 0;
    DataSenddings.temperature = 0;DataSenddings.humidity = 0;DataSenddings.mbattery = 0;
    DataSenddings.battery = 0;DataSenddings.RSSI = 0;
  if(status == 0){LOG("Sent to master: mesh\n");DataSenddings.category = 0;DataSenddings.status = 0;}
  if(status == 1){LOG("Sent to master: online\n");DataSenddings.category = 0;DataSenddings.status = 1;}
  if(status == 2){LOG("Sent to master: update\n");DataSenddings.category = 1;DataSenddings.status = 0;}
    memcpy(&DataSenddings, incomingData, sizeof(incomingData));
    if(RunMode == MESHSLAVE){
    RF_Serial.write(incomingData, sizeof(incomingData));
    RF_Serial.write('\n'); }
}


byte IoT_Device::getRunMode() {
  return RunMode;
}
byte IoT_Device::getDebug() {
  return Debug;
}


void OnWiFi();










//----------------------------------------------------------------
//------------ Gateway Master Function ---------------------------
//----------------------------------------------------------------
//Command("0","0","0","0","0");//0_0_0_0_0 MQTT loss
//Command("0","0","0","0","1");//0_0_0_0_1 MQTT ok
//Command("0","0","0","1","0");//0_0_0_1_0 Mesh Slave Restart
//Command("0","0","0","1","1");//0_0_0_1_1 Mesh Slave Wifi Enable
//Command("0","0","0","1","1");//0_0_0_2_0 Master Restart
//Command("0","0","0","1","1");//0_0_0_2_1 Master Wifi Enable

//Command("1",netID,ID,"1","0");//Control State = 1
//Command("1",netID,ID,"0","0");//Control State = 0
void IoT_Device::Command(String Cmd, String netID, String ID, String CAT, String SleepTime){
  CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &RunMode);
  if(RunMode == WIFIMODE){//Gateway Master
    if(netID == "0" && ID == "0" && CAT == "2" && SleepTime == "0"){LOGLN("Restart");delay(2000);ESP.restart();}
    else if(netID == "0" && ID == "0" && CAT == "2" && SleepTime == "1"){LOGLN("Wifi On");delay(2000);OnWiFi();}
    else {
    DataCommand.Command = Cmd.toInt();
    DataCommand.networkID = netID.toInt();
    DataCommand.nodeID = ID.toInt();
    DataCommand.category = CAT.toInt();
    DataCommand.time = SleepTime.toInt();
    if(DataCommand.Command == 0){LOGLN("Setting cmd");}
    if(DataCommand.Command == 1){LOGLN("Control cmd");}
    memcpy(isDataCommand, &DataCommand, sizeof(DataCommand));
    RF_Serial.write(isDataCommand, sizeof(isDataCommand));
    RF_Serial.write("\n");
    }
  }
}
byte hours;
byte mins;
byte secs;
void readtime()
{
  time_t now = time(nullptr);
  //Wed Sep 1 21:59:03 2021
  hours = ((ctime(&now)[11]-48)*10)+(ctime(&now)[12]-48);
  mins = ((ctime(&now)[14]-48)*10)+(ctime(&now)[15]-48);
  secs = ((ctime(&now)[17]-48)*10)+(ctime(&now)[18]-48);
}
void IoT_Device::UpdateStatus()// MQTT
{
  DynamicJsonDocument sensor(500);
#ifdef vplab
  sensor["deviceid"] = incomingnodeID;
  sensor["networkid"] = incomingnetworkID;
  sensor["category"] = incomingCatagory;
  sensor["status"]  = incomingstatus;
  sensor["temperature"]  = incomingtemperature;
  sensor["humidity"]  = incominghumidity;
  sensor["mbattery"] = incomingmbattery;
  sensor["battery"] = incomingmbattery;
  sensor["rssi"] = incomingrssi_display;
#endif//isoft
#ifdef isoft
  // CONFIG::readtime();
  CONFIG::read_byte (EP_EEPROM_ID, &incomingnodeID);
  sensor["id"] = incomingnodeID;
  sensor["name"] = "temp";
  sensor["value"]  = temperature;
  sensor["unit"]  = "calcius";
  sensor["name"] = "humi";
  sensor["value"]  = humidity;
  sensor["unit"]  = "%";
  time_t now = time(nullptr);
  sensor["createAt"] = ctime(&now);
#endif//isoft

  String payload = "";
  size_t n = serializeJson(sensor, payload);
  LOGLN(payload);
  mqttcommu.mqttPublish(payload);
}




void IoT_Device::WifiMode()
{
    RunMode = WIFIMODE;
    if(wifi_config.GetWifiMode() == true){
      esp_wifi_set_protocol(current_wifi_interface, 7);
      check_protocol();
        WiFi.mode(WIFI_AP_STA);
        if(LEDType == 0){ledFadeToBeat(0,0,255,BRIGHTNESS_HEART);colorWipe(0x000000, 100);}
        else{LED_Signal(3, 100);}
          if (!wifi_config.Setup(0,LED_STATUS, HIGH) ) {
          OLED_DISPLAY::clear_lcd();
          OLED_DISPLAY::BigDisplay("WiFi ON", 15, 17);
          OLED_DISPLAY::setCursor(40, 48);
          }
          ESPCOM::print(WiFi.localIP().toString().c_str(), OLED_PIPE);
          checkFW = true;

      Result = CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &ModeRun1);
      if(Debug)Debug_Ser.println("STA ModeRun:" + String(ModeRun1));
    }
    
    else if(wifi_config.GetWifiMode() == false){
      Result = CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &ModeRun1);
      if(Debug)Debug_Ser.println("AP ModeRun:" + String(ModeRun1));
        // WiFi.setPhyMode (WIFI_PHY_MODE_11G);
        // WiFi.mode(WIFI_AP);
        if(LEDType == 0){ledFadeToBeat(255,0,255,BRIGHTNESS_HEART);colorWipe(0x000000, 100);}
        else{LED_Signal(3, 500);}
        // OLED_DISPLAY::clear_lcd();
        // OLED_DISPLAY::BigDisplay("WiFi ON", 15, 17);
        // OLED_DISPLAY::setCursor(40, 48);
        ESPCOM::print(WiFi.softAPIP().toString().c_str(), OLED_PIPE);
        // if (!wifi_config.Setup (true) ) {       
        //   LOG("Safe mode 2");
        // }
          // wifi_config.Safe_Setup();
      // Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, 2);//OFF WiFi AP Run Moto Mode
      // Result = CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &ModeRun1);
      // Debug_Ser.println("ModeRun:" + String(ModeRun1));
      //delay(3000);
    }
}
#ifdef ARDUINO_ARCH_ESP32

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#ifndef ARDUINO_ARCH_ESP8266
int IoT_Device::check_protocol()
{
    char error_buf1[100];
  if(Debug){
    LOGLN();
    LOGLN("___________________________________");
    LOGLN();
  }
     esp_err_t error_code = esp_wifi_get_protocol(current_wifi_interface, &current_protocol);
     esp_err_to_name_r(error_code,error_buf1,100);
  if(Debug){
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
#endif//
void MeshRecive(const uint8_t * mac, const uint8_t *incomingDatas, int len);
void IoT_Device::Mesh_Init(){
  #ifndef ARDUINO_ARCH_ESP8266
  String dataDisp = "";
  if (!CONFIG::is_locked(FLAG_BLOCK_OLED)) {
   IOT_DEVICE.OLED_Display("Mesh Init",2);}
    else{ OLED_DISPLAY::display_text("Mesh Init", 0, 0, 85);}
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
    if (!CONFIG::is_locked(FLAG_BLOCK_OLED)) {
    IOT_DEVICE.OLED_Display("Error!!",3);}
    else{dataDisp = "Error!!";OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 16, 128);}
    if(Debug)LOGLN("Error initializing ESP-NOW");
    strip.setBrightness(100);strip.setPixelColor(0, 0xff0000);strip.show(); 
    return;
  }
  else{
    if (!CONFIG::is_locked(FLAG_BLOCK_OLED)) {
    IOT_DEVICE.OLED_Display("OK!!",3);}
    else{dataDisp = "OK!!";OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 16, 128);}
    if(Debug)LOGLN("initializing ESP-NOW OK");
    strip.setBrightness(100);strip.setPixelColor(0, 0x00ff00);strip.show(); 
  }
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    if(Debug)LOGLN("Failed to add peer");
    if (!CONFIG::is_locked(FLAG_BLOCK_OLED)) {
    IOT_DEVICE.OLED_Display("Failed to add peer",4);}
    else{dataDisp = "Failed to add peer!!";OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 24, 128);}
    strip.setBrightness(100);strip.setPixelColor(1, 0xff0000);strip.show();
    // RunMode = 1;
    return;
  }
  else{
    if(Debug)LOGLN("add peer OK");
    if (!CONFIG::is_locked(FLAG_BLOCK_OLED)) {
    IOT_DEVICE.OLED_Display("add peer OK",4);}
    else{dataDisp = "add peer OK!!";OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 24, 128);}
    strip.setBrightness(100);strip.setPixelColor(1, 0x00ff00);strip.show(); 
    }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(MeshRecive);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
  #endif//
}

void IoT_Device::MeshBegin()
{
  CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &RunMode);
    if(RunMode == MESHSLAVE){if(Debug)LOGLN("RunMode: Mesh Slave");sendCurrentStatus(0);}
    if(RunMode == MESHMODE){if(Debug)LOGLN("RunMode: Mesh (Node)");}
    if(RunMode == WIFIMODE){if(Debug)LOGLN("RunMode: Wifi (Gateway)");sendCurrentStatus(1);}
#ifdef ARDUINO_ARCH_ESP8266
        WiFi.setPhyMode (WIFI_PHY_MODE_11G);
        WifiMode();
#else
   if(RunMode == MESHMODE || RunMode == MESHSLAVE){
    if(Debug)LOGLN("Mesh Init");
      WiFi.disconnect();WiFi.mode(WIFI_OFF); Mesh_Init(); 
      OLED_DISPLAY::clear_lcd();
      if(LEDType == 0){ledFadeToBeat(255,255,0,BRIGHTNESS_HEART);colorWipe(0x000000, 100);}
      else{LED_Signal(4, 100);}
    }
  if(RunMode == WIFIMODE){WifiMode();}
  #endif//
}











//----------------------------------------------------------------
//------------ Mesh Reciver Function   ---------------------------
//----------------------------------------------------------------
// Callback when data is received Mesh

void MeshRecive(const uint8_t * mac, const uint8_t *incomingDatas, int len) {
  if(IOT_DEVICE.RunMode == MESHMODE){
    if(len == sizeof(incomingReadings)) {
    memcpy(&incomingReadings, incomingDatas, sizeof(incomingReadings));
    LOG("Data  incomingReadings");
    // LOGLN(len);
    incomingnetworkID = incomingReadings.networkID;
    incomingnodeID = incomingReadings.nodeID;
    incomingstatus = incomingReadings.status;
    incomingCatagory = incomingReadings.category;
    incominghumidity = incomingReadings.humidity;
    incomingmbattery = incomingReadings.mbattery;
    incomingbattery = incomingReadings.battery;
    incomingrssi_display = incomingReadings.RSSI;
    IOT_DEVICE.updateDisplay();
    }
    if(len == sizeof(DataCommand)){// Control by Mesh
      memcpy(&DataCommand, incomingDatas, sizeof(DataCommand));
      if(DataCommand.Command == 1 && DataCommand.networkID == networkID && DataCommand.nodeID == nodeID){
        if(IOT_DEVICE.Debug){LOGLN("Data Controller");
        LOG("Network ID:" + String( DataCommand.networkID) + " | ID:" + String( DataCommand.nodeID));
        LOGLN(" | state:" + String(DataCommand.category));}
          state = DataCommand.category;LOGLN("update state");
          Status = state;delay(500);
          IOT_DEVICE.sendDataNode();
      }// Setting by Mesh
      if(DataCommand.Command == 0 && DataCommand.networkID == networkID && DataCommand.nodeID == nodeID){
        if(IOT_DEVICE.Debug){LOGLN("Data Setting");      
        LOG("Network ID:" + String( DataCommand.networkID) + " | ID:" + String( DataCommand.nodeID));
        LOGLN(" | category:" + String(DataCommand.category) + " | time:" + String(DataCommand.time) );}
          Result = CONFIG::write_byte (EP_EEPROM_NET_ID, DataCommand.networkID);  
          Result = CONFIG::write_byte (EP_EEPROM_CATEGORY, DataCommand.category);  
          Result = CONFIG::write_byte (EP_EEPROM_SLEEP_TIME, DataCommand.time);  
      }
    }
  }//RunMode == MESHMODE
  if(IOT_DEVICE.RunMode == MESHSLAVE){
    if(len == sizeof(incomingReadings)) {
    memcpy(&incomingReadings, incomingDatas, sizeof(incomingReadings));
    LOG("Data  incomingReadings");
    // LOGLN(len);
    incomingnetworkID = incomingReadings.networkID;
    incomingnodeID = incomingReadings.nodeID;
    incomingstatus = incomingReadings.status;
    incomingCatagory = incomingReadings.category;
    incominghumidity = incomingReadings.humidity;
    incomingmbattery = incomingReadings.mbattery;
    incomingbattery = incomingReadings.battery;
    incomingrssi_display = incomingReadings.RSSI;
    IOT_DEVICE.updateDisplay();
    }
    // Serial.println("DataIn");
    RF_Serial.write(incomingDatas, len);
    RF_Serial.write('\n');
    //Serial.println("update status");
  }
}








void IoT_Device::setup() {
  // Set device as a Wi-Fi Station
  // Init Serial Monitor
  Serial.begin(115200);
  // #ifndef ARDUINO_ARCH_ESP8266
  RF_Serial.begin(115200);
  // #endif//
  IOT_DEVICE.getConfig();
  if(Debug)LOGLN("\n\nButton Mullti");
    button.debounceTime   = 20;   // Debounce timer in ms
    button.multiclickTime = 250;  // Time limit for multi clicks
    button.longClickTime  = 1000; // Time until long clicks register
    button1.debounceTime   = 20;   // Debounce timer in ms
    button1.multiclickTime = 250;  // Time limit for multi clicks
    button1.longClickTime  = 1000; // Time until long clicks register

  if(LEDType == 0){
    strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
    strip.show();  // Initialize all pixels to 'off' 
  }
  else{pinMode(PIXEL_PIN,OUTPUT);}

  
        

  // Init OLED display
  #ifdef OLED_SH1106
  if(OLED_MODE == OLED_130){
    if(!display.init()) { 
    if(Debug)LOGLN(F("SH1106 allocation failed"));
    for(;;);
    }
    IOT_DEVICE.OLED_Display("VPlab Gateway",0);
  }
  #endif//OLED_SH1106
  #ifdef OLED_SSD1306
  if(OLED_MODE == OLED_096){
    if(!display1.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
      if(Debug)LOGLN(F("SSD1306 allocation failed"));
      for(;;);
    }
    IOT_DEVICE.OLED_Display("VPlab Gateway",0);

  }
  #endif//OLED_SSD1306
  //----------------------------------------------------------------
  // MQTT
  if(RunMode == WIFIMODE)mqttcommu.setup();
  if(ROLE == Node) NodeCategoryInit(category);
}







void OnWiFi(){
  Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, WIFIMODE);  
    if(IOT_DEVICE.RunMode == MESHSLAVE){IOT_DEVICE_.sendCurrentStatus(1);}
    delay(3000);ESP.restart(); 
}

byte          receiver_status = 1;    // tracks the ESPNow receiver status
bool Statement = false;
void IoT_Device::loop() {

if(checkFW && RunMode == WIFIMODE  && (WiFi.status() == WL_CONNECTED))UDFW.repeatedCall();

#ifndef ARDUINO_ARCH_ESP8266
  button.Update();
  button1.Update();
  if (button1.clicks != 0) function1 = button1.clicks;
  if (button.clicks != 0) function = button.clicks;
  //if (button1.clicks != 0)Debug_Ser.println("function:" + String(function1));
  //if (button.clicks != 0)Debug_Ser.println("function:" + String(function));

  if(( button.clicks == 1 || button1.clicks == 1) && Menu == 0){function = 0;function1 = 0;
    if(Debug)Debug_Ser.println("Single click");
    if((category == GROUP_RELAY || category == GROUP_SWITCH || category == GROUP_MOTION )&& RunMode == MESHMODE){state = !state;Status = state;LOGLN("state " + String(Status)); sendDataNode();}
    // if(RunMode == WIFIMODE){Statement = !Statement;Command("1" ,"1", String(nodeID),String(Statement));LOGLN("(Gateway)state " + String(Status));}
    // UpdateStatus();
  }
  if(( button.clicks == 2 || button1.clicks == 2)  && Menu == 0){function = 0;function1 = 0;
  if(Debug)Debug_Ser.println("Single click2");
    if(RunMode < 2){
      checkFW = !checkFW;if(Debug)Debug_Ser.println("Check FW:" + String(checkFW));
      if(LEDType == 0){ledFadeToBeat(255,0,255,BRIGHTNESS_HEART);ledFadeToBeat(255,0,0,BRIGHTNESS_HEART);colorWipe(0x000000, 100);}
      else{LED_Signal(6, 100);}
    }
    if(RunMode == WIFIMODE){Command("0" ,"1", String(3), String(4), String(0));LOGLN("(Gateway)Category " + String(4) + " | Time:" + String(0));}
    
  }
  if(( button.clicks == 3 || button1.clicks == 3) && Menu > 0){function = 0;function1 = 0;Menu = 0;OLED_DISPLAY::clear_lcd();
  if(Debug)Debug_Ser.println("Single click3 Menu 0");}
  if(( button.clicks == 3 || button1.clicks == 3) && Menu == 0){function = 0;function1 = 0;Menu = 1;OLED_DISPLAY::clear_lcd();
  if(Debug)Debug_Ser.println("Single click3 Menu 1");}
  if(( button.clicks == 1 || button1.clicks == 1)  && Menu > 0){function = 0;function1 = 0;Menu++;if(Menu > 3){Menu = 1;}Debug_Ser.println("Single click3 Menu " + String(Menu));OLED_DISPLAY::clear_lcd();}
  if(( button.clicks == 2 || button1.clicks == 2)  && Menu == 3){checkFW = true;function = 0;function1 = 0;UPDATEFWS.ShowMess("Checking...");if(Debug)Debug_Ser.println("Check FW");}

  // fade if button is held down during single-click
  if(function == -1 || function1 == -1)
  {
    function = 0;function1 = 0;
      if(Debug)Debug_Ser.println("hold button");
      if(Menu > 0){Menu = 0;OLED_DISPLAY::clear_lcd();}
      if(Menu == 0){
        if(LEDType == 0){ledFadeToBeat(255,0,0,BRIGHTNESS_HEART);colorWipe(0x000000, 100);}
        else{LED_Signal(2, 100);}
        if(RunMode > 2){Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, WIFIMODE);delay(3000);ESP.restart();}
        if(RunMode == 2){Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, WIFIMODE);delay(3000);ESP.restart();}
        if(RunMode == 1){Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, MESHMODE);delay(3000);ESP.restart();}
        if(RunMode == 0){Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, WIFIMODE);delay(3000);ESP.restart();}
      }
  } 
  if(function == -2 || function1 == -2)
  {
    function = 0;function1 = 0;
    #ifdef ARDUINO_ARCH_ESP8266
        WiFi.setPhyMode (WIFI_PHY_MODE_11G);
    #else
    if(RunMode == WIFIMODE || RunMode == MESHMODE){if(Debug)LOGLN("MESH Slave (RF gateway)");
      Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, MESHSLAVE);delay(3000);ESP.restart();
    }
    #endif//
  } 
  if(function == -3 || function1 == -3)
  {
    function = 0;function1 = 0;
    #ifdef ARDUINO_ARCH_ESP8266
        WiFi.setPhyMode (WIFI_PHY_MODE_11G);
    #else
      IOT_DEVICE.check_protocol();
      esp_wifi_set_protocol(current_wifi_interface, 7);
      IOT_DEVICE.check_protocol();
    #endif//
      if(Debug)Debug_Ser.println("hold button 3");
      if(Debug)Debug_Ser.println("Wifi on");
      RunMode = 1;
    OLED_DISPLAY::clear_lcd();
    WifiMode();
    
  } 
    // Reset function
    function = 0;function1 = 0;
  #endif//#ifndef ARDUINO_ARCH_ESP8266

  if(RunMode == MESHSLAVE){///Messh data recive from Gateway master
    if (RF_Serial.available()) {
        received_msg_length = RF_Serial.readBytesUntil('\n', incomingData, sizeof(incomingData));
        LOGLN("size incomingData" + String(sizeof(incomingData)));
        LOGLN("size DataCommand" + String(sizeof(DataCommand)));
        // if(Debug)LOGLN("Data");
        if (received_msg_length == sizeof(DataCommand)) {  // got a msg setting from a gateway
          memcpy(&DataCommand, incomingData, sizeof(DataCommand));
          if(DataCommand.Command == 0){
            if(Debug)LOGLN("received DataSetting");
            if(Debug)LOG  ("net ID:" + String(DataCommand.networkID) + " | ID:" + String(DataCommand.nodeID));
            if(Debug)LOGLN(" | category:" + String(DataCommand.category) + " | time:" + String(DataCommand.time));
            if(DataCommand.networkID == 0 && DataCommand.nodeID == 0) {
              LOG("master cmd");
              if ( DataCommand.category == 0 && DataCommand.time == 0 ) MasterStatus = "MQTT loss";
              if ( DataCommand.category == 0 && DataCommand.time == 1 ) MasterStatus = "MQTT OK";
              if ( DataCommand.category == 1 && DataCommand.time == 0 ) delay(2000);ESP.restart();
              if ( DataCommand.category == 1 && DataCommand.time == 1 ) OnWiFi();
            }else{
            #ifdef ESP32
            esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DataCommand, sizeof(DataCommand));
              if (result == ESP_OK) {
                if(Debug)LOGLN("Sent with success");
              }
              else {
                if(Debug)LOGLN("Error sending the data");
                //  IOT_DEVICE.MeshBegin();
              }
              #endif//#ifdef ESP32
            }
          }
          if(DataCommand.Command == 1){
            if(Debug)LOGLN("received DataControl");
            if(Debug)LOG  ("net ID:" + String(DataCommand.networkID) + " | ID:" + String(DataCommand.nodeID));
            if(Debug)LOGLN(" | state:" + String(DataCommand.category));
            #ifdef ESP32
            esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DataCommand, sizeof(DataCommand));
              if (result == ESP_OK) {
                if(Debug)LOGLN("Sent with success");
              }
              else {
                if(Debug)LOGLN("Error sending the data");
                //  IOT_DEVICE.MeshBegin();
              }
              #endif//ESP32
          }
        } 
    }
      
  }//RunMode == MESHSLAVE
  if(RunMode == WIFIMODE){
    mqttcommu.loop();
    NodeDataUpdate();
    if (RF_Serial.available()) {//Data recive from RF module  
      received_msg_length = RF_Serial.readBytesUntil('\n', incomingData, sizeof(incomingData));
        if (received_msg_length == sizeof(incomingData)) {  // got a msg from a sensor
          memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
          // Serial.println(incomingReadings.networkID);
          // if ( incomingReadings.networkID == networkID ) {
              incomingnetworkID = incomingReadings.networkID;
              incomingnodeID = incomingReadings.nodeID;
              incomingstatus = incomingReadings.status;
              incomingCatagory = incomingReadings.category;
              incominghumidity = incomingReadings.humidity;
              incomingtemperature = incomingReadings.temperature;
              incomingmbattery = incomingReadings.mbattery;
              incomingbattery = incomingReadings.battery;
              incomingrssi_display = incomingReadings.RSSI;
              if(incomingnetworkID == 0 && incomingnodeID == 0) {
                LOG("\n\nslave cmd\n\n");
                if ( incomingCatagory == 0 && incomingstatus == 0 ){LOG("\n\nReciver: mesh\n\n"); ESPCOM::print("Reciver: mesh", WS_PIPE);}
                if ( incomingCatagory == 0 && incomingstatus == 1 ){ LOG("\n\nReciver: online\n\n");  ESPCOM::print("Reciver: online", WS_PIPE);}
                if ( incomingCatagory == 1 && incomingstatus == 0 ){ LOG("\n\nReciver: update\n\n");  ESPCOM::print("Reciver: update", WS_PIPE);}
              }else if(incomingCatagory <= 8 && incomingstatus <= 2){
                IOT_DEVICE.updateDisplay();
                UpdateStatus();
              }
            // sensorMessageReceived();
            // saveSensorData();
          // }
          }
    }
  }//RunMode == WIFIMODE
#ifdef ARDUINO_ARCH_ESP8266

#else// ESP32
      if(RunMode == MESHMODE){
        if(ROLE == Node)NodeCategoryRead(category);
        if(ROLE == Node)NodeDataUpdate();
      }
    // count++;delay(1);
    // if (count >= 10000) {count = 0;//send node data
    // IoT_Device::getConfig();
    //   // Send message via ESP-NOW
    //   if(ROLE == Gateway){
    //     UpdateStatus();
    //   }
    // }
#endif//ARDUINO_ARCH_ESP8266
  // }

  // if(UpdateFw== true && WifiMode == STAMODE)IoT_Device::UpdateLoop();
  // if(WifiMode == STAMODE && UpdateFw == false)IoT_Device::connectWifi();
  // if(ConnectRetry > 10 && WifiMode == STAMODE ){ConnectRetry = 0;WifiMode = APMODE;WifiSetup();}
  // TaskServer();
}

















void IoT_Device::sendDataNode(){
    DataSenddings.networkID = networkID;
    DataSenddings.nodeID = nodeID;
    DataSenddings.category = category;
    DataSenddings.status = Status;
    DataSenddings.temperature = temperature;
    DataSenddings.humidity = humidity;
    DataSenddings.mbattery = mbattery;
    DataSenddings.battery = battery;
    DataSenddings.RSSI = rssi_display;
    if(Debug){
      LOGLN("DATA SENDDINGS");
      LOG("network ID: ");LOG(networkID);
      LOG(" | nodeID: ");LOG(nodeID);
      LOG(" | Category: ");LOG(category);
      LOG(" | status: ");LOG(Status);
      LOG(" | Temperature: ");LOG(temperature);LOG(" ºC");
      LOG(" | Humidity: ");LOG(humidity);LOG(" %");
      LOG(" | mbattery: ");LOG(mbattery);LOG(" V");
      LOG(" | battery: ");LOG(battery);LOG(" V");
      LOG(" | RSSI: ");LOG(rssi_display);
      LOGLN();
    }
    #ifdef ESP32
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DataSenddings, sizeof(DataSenddings));
    if (result == ESP_OK) {
      if(Debug)LOGLN("Sent with success");
    }
    else {
      if(Debug)LOGLN("Error sending the data");
      //  IOT_DEVICE.MeshBegin();
    }
    #endif//def ESP32
}
/// Node Sendata to Gateway
/// @brief Node Send To Gateway

ESPResponseStream espresponse;
void IoT_Device::NodeDataUpdate(){
  static uint32_t last_node_update= 0;
  uint32_t now_node = millis();
  if (now_node - last_node_update > 30000) {//Node Update after 5000 ms
        last_node_update = now_node;
    if(ROLE == Node)sendDataNode();
    
    String msg = "";
    // msg = "Master WS: update";
    // ESPCOM::println(msg.c_str(), WS_PIPE, &espresponse);
    ESPCOM::println("Master WEB: update", WEB_PIPE, &espresponse);
    // msg = "Master SERIAL: update";
    // ESPCOM::println(msg.c_str(), SERIAL_PIPE, &espresponse);
    // msg = "Master TCP: update";
    // ESPCOM::println(msg.c_str(), TCP_PIPE, &espresponse);
    // LOGLN("Master update");
  }//if (now_dht - last_dht_update > 5000) {
}
/// @brief Node Category
/// @param cat 
void IoT_Device::NodeCategoryRead(byte cat){
// if(RunMode == MESHMODE){
    if(cat == GROUP_HT){
      static uint32_t last_dht_update= 0;
      uint32_t now_dht = millis();
      if (now_dht - last_dht_update > dht_iot.getMinimumSamplingPeriod() *1.2) {
          last_dht_update = now_dht;
          humidity = dht_iot.getHumidity();
          temperature = dht_iot.getTemperature();

          // Serial.print();
          if(dht_iot.getStatusString() == "TIMEOUT"){
              humidity = 0;
              temperature = 0;
          }
          if(COMMODE == MQTT){ UpdateStatus();}

      }
    }
    if(cat == GROUP_SHT){
      static uint32_t last_dht_update= 0;
      uint32_t now_dht = millis();
      if (now_dht - last_dht_update > 333) {
          last_dht_update = now_dht;
          Sensor.UpdateData();
          humidity = Sensor.GetRelHumidity();
          temperature = Sensor.GetTemperature();
          if(COMMODE == MQTT){ 
            incomingtemperature = temperature;
            UpdateStatus();}
          // Serial.print();
          // if(Sensor.getStatusString() == "TIMEOUT"){
          //     humidity = 0;
          //     temperature = 0;
          // }

      }
    }
    if(cat == GROUP_TEMP){
      static uint32_t last_temp_update= 0;
      uint32_t now_temp = millis();
      if (now_temp - last_temp_update > 5000) {
        last_temp_update = now_temp;
        for (int i = 0; i < oneWireCount; i++) {
          temperature = sensor[i].getTempCByIndex(0);
          Serial.print("Temperature for the sensor ");
          Serial.print(i);
          Serial.print(" is ");
          Serial.println(temperature);
        }
          if(COMMODE == MQTT){ UpdateStatus();}
      }
    } 
    if(cat == GROUP_SWITCH){
      if(digitalRead(SWITCHPIN) == 0){
        Status = true;
      }
      else{
        Status = false;
      }
    }
    if(cat == GROUP_MOTION){
      if(digitalRead(SWITCHPIN) == 0){
        Status = true;
      }
      else{
        Status = false;
      }
    }
    if(cat == GROUP_MOISTURE){
      humidity = 0 ;temperature = 0 ;
      for(byte i = 0; i < 200; i++){humidity = humidity + analogRead(ANALOGPIN1);}
      for(byte i = 0; i < 200; i++){temperature = temperature + analogRead(ANALOGPIN1);}
      humidity = humidity / 200;
      temperature = temperature / 200;
    }
    if(cat == GROUP_RELAY){
      if(state){digitalWrite(RELAYPIN, HIGH);}
      else{digitalWrite(RELAYPIN, LOW);}
      if (state == 0) {
        digitalWrite(2, LOW);   // Turn the LED on (Note that LOW is the voltage level
        Status = 0;
      } 
      if (state == 1) { 
        digitalWrite(2, HIGH);  // Turn the LED off by making the voltage HIGH
        Status = 1;
      }
    }
    if(cat == GROUP_VALVE){
      if(state){digitalWrite(RELAYPIN, HIGH);}
      else{digitalWrite(RELAYPIN, LOW);}
      if(analogRead(ANALOGPIN1) <= 20){Status = 1;}
      if(analogRead(ANALOGPIN1) <= 20){Status = 0;}
      if(analogRead(ANALOGPIN1) >= 100 && analogRead(ANALOGPIN2) >= 100){Status = 2;}
    }
  // }
}
void IoT_Device::NodeCategoryInit(byte cat)
{
    if(cat == GROUP_SWITCH){
      pinMode(SWITCHPIN, INPUT_PULLUP);
    }
    else if (cat == GROUP_HT){
      dht.setup(DHTPin); 
    }
    else if (cat == GROUP_MOTION){
      pinMode(SWITCHPIN, INPUT_PULLUP);
    }
    else if (cat == GROUP_RELAY){
      pinMode(SWITCHPIN, INPUT_PULLUP);
      pinMode(RELAYPIN, OUTPUT);
    }
    else if (cat == GROUP_MOISTURE){
      pinMode(ANALOGPIN1, INPUT);
      pinMode(ANALOGPIN2, INPUT);
    }
    else if (cat == GROUP_TEMP){
        DeviceAddress deviceAddress;
        for (int i = 0; i < oneWireCount; i++) {
          sensor[i].setOneWire(&ds18x20[i]);
          sensor[i].begin();
          if (sensor[i].getAddress(deviceAddress, 0)) sensor[i].setResolution(deviceAddress, 12);
        }
    }
    else if (cat == GROUP_VALVE){
      pinMode(SWITCHPIN, INPUT_PULLUP);
      pinMode(RELAYPIN, OUTPUT);
      pinMode(ANALOGPIN1, INPUT);
      pinMode(ANALOGPIN2, INPUT);
    }
}


#endif////////////////////////////////
//----------------------------------------------------------------
//-------------------------- Load Config -------------------------
//----------------------------------------------------------------
void IoT_Device::getConfig(){
  if(STT)LOGLN()
  Debug = DEBUG; 
  if(CONFIG::read_byte (EP_EEPROM_DEBUG, &Debug)){
    if(Debug && STT)Serial.println("Data");
    if(STT)LOGLN("Read Debug OK, value:" + String((Debug==0)?"Not Debug":"Debug"));
  }
  networkID = 1;
  if(CONFIG::read_byte (EP_EEPROM_NET_ID, &networkID)){
    if(Debug && STT)LOGLN("Read network ID OK, value:" + String(networkID));
  }
  nodeID = 2;
  if(CONFIG::read_byte (EP_EEPROM_ID, &nodeID)){
    if(Debug && STT)LOGLN("Read ID OK, value:" + String(nodeID));
  }
  category = GROUP_RELAY;
  if(CONFIG::read_byte (EP_EEPROM_CATEGORY, &category)){
    if(Debug && STT)LOGLN("Read CATEGORY OK, value:" + String(category));
  }

  category = MESHWIFI;
  if(CONFIG::read_byte (EP_EEPROM_COM_MODE, &COMMODE)){
    if(Debug && STT)LOGLN("Read COM MODE OK, value:" + String(COMMODE));
  }

  Status = 0;
  temperature = 26.8;
  humidity = 76.5;
  mbattery = 4.1;
  battery = 0;
  battery = 0;
  //define hardware

  OLED_MODE = OLED_130;
  if(CONFIG::read_byte (EP_EEPROM_OLED_TYPE, &OLED_MODE)){
    if(Debug && STT)LOGLN("Read OLED_MODE OK, value:" + String((OLED_MODE==0)?"OLED 0.96\"":"OLED 1.3\""));
  }
  ROLE = Node; 
  if(CONFIG::read_byte (EP_EEPROM_ROLE, &ROLE)){
    if(Debug && STT)LOGLN("Read ROLE OK, value:" + String((ROLE == 0)?"Node":"Gateway"));
  }
  NameBoard = "VPlab";
  if(CONFIG::read_string (EP_EEPROM_NAME, NameBoard, MAX_NAME_LENGTH)){
    if(Debug && STT)LOGLN("Read NAME OK, value:" + String(NameBoard));
  }
  CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &RunMode);
  if(IOT_DEVICE.RunMode > 2){
    CONFIG::write_byte (EP_EEPROM_WIFI_MODE, 1); 
    CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &RunMode);
    }
    if(RunMode == MESHSLAVE){if(Debug && STT)LOGLN("RunMode: Mesh Slave");}
    if(RunMode == MESHMODE){if(Debug && STT)LOGLN("RunMode: Mesh (Node)");}
    if(RunMode == WIFIMODE){if(Debug && STT)LOGLN("RunMode: Wifi (Gateway)");}
  ButtonUpdateFw = USE;
  if(CONFIG::read_byte (EP_EEPROM_FW_BUTTON, &ButtonUpdateFw)){
    if(Debug && STT)LOGLN("Read FW BUTTON OK, value:" + String((ButtonUpdateFw==0)?"Use":"Not Use"));
  }
  if(CONFIG::read_byte (EP_EEPROM_UPDATE_FW, &UpdateFw)){
    if(Debug && STT)LOGLN("Read UPDATE FW AUTO OK, value:" + String((UpdateFw==0)?"Use":"Not Use"));
  }
  if(CONFIG::read_byte (EP_EEPROM_PIN_BUTTON, &ButtonPin)){
    if(Debug && STT)LOGLN("Read Button Pin OK, value:" + String(ButtonPin));
  }
  if(CONFIG::read_byte (EP_EEPROM_PIN_LEDFULL, &LEDPin)){
    if(Debug && STT)LOGLN("Read LED Pin OK, value:" + String(LEDPin));
  }
  if(CONFIG::read_byte (EP_EEPROM_LED_TYPE, &LEDType)){
    if(Debug && STT)LOGLN("Read LED Type OK, value:" + String(LEDType));
  }
  // PRGM_VERSION
    if(Debug && STT)LOGLN("Firmware Version:" + UDFW.FirmwareVer);
      String str_= "";
  char mqttUserName[MAX_MQTT_USER_LENGTH + 1];
  char mqttUserPassword[MAX_MQTT_PASS_LENGTH + 1];
  char mqttbroker[MAX_MQTT_BROKER_LENGTH + 1];
  byte mqttPort;
  if(CONFIG::read_string (EP_MQTT_BROKER, mqttbroker, MAX_MQTT_BROKER_LENGTH)){
    if(Debug && STT)LOGLN("mqtt broker:" + String(mqttbroker));
  }
  if(CONFIG::read_string (EP_MQTT_USER, mqttUserName, MAX_MQTT_USER_LENGTH)){
    if(String(mqttUserName) == "_"){str_.toCharArray(mqttUserName, str_.length());}
    if(Debug && STT)LOGLN("mqtt user:" + String(mqttUserName));
  }
  if(CONFIG::read_string (EP_MQTT_PASS, mqttUserPassword, MAX_MQTT_PASS_LENGTH)){
    if(String(mqttUserPassword) == "_"){str_.toCharArray(mqttUserPassword, str_.length());}
    if(Debug && STT)LOGLN("mqtt pass:" + String(mqttUserPassword));
  }   
  if(CONFIG::read_byte (EP_MQTT_PORT, &mqttPort)){
    if(Debug && STT)LOGLN("mqtt port:" + String(mqttPort));
  } 

  STT = false;
}

//----------------------------------------------------------------
//--------------------------   Display  -------------------------
//----------------------------------------------------------------
void IoT_Device::updateDisplay(){
  if(LEDType == 0){strip.setBrightness(50);strip.setPixelColor(0, 0x00ff00);strip.setPixelColor(4, 0x00ff00);strip.show(); }
  else{digitalWrite(PIXEL_PIN,HIGH);}
  // Display Readings on OLED Display
  // #ifdef OLED_SH1106
    String dataDisp = "";
    // display.clear();
    OLED_DISPLAY::display_text("INCOMING READINGS", 0, 0, 128);
    // #define GROUP_SWITCH          1
    // #define GROUP_HT              2
    // #define GROUP_MOTION          3
    // #define GROUP_RELAY           4
    // #define GROUP_MOISTURE        5
    // #define GROUP_TEMP            6
    // #define GROUP_VALVE           7
    // #define GROUP_POSITION        8
    if(incomingCatagory == GROUP_HT){
    dataDisp = "Temperature: " + String(incomingtemperature) + "ºC";
    OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 13, 128);
    dataDisp = "Humidity: " + String(incominghumidity) + "%";
    OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 24, 128);
    }
    if(incomingCatagory == GROUP_TEMP){
    dataDisp = "Temperature: " + String(incomingtemperature) + "ºC";
    OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 13, 128);
    }
    if(incomingCatagory == GROUP_SWITCH){
    dataDisp = "Switch: " + String(incomingstatus);
    OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 13, 128);
    }
    if(incomingCatagory == GROUP_MOTION){
    dataDisp = "Motion: " + String(incomingstatus);
    OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 13, 128);
    }
    if(incomingCatagory == GROUP_RELAY){
    dataDisp = "Relay: " + String(incomingstatus);
    OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 13, 128);
    }
    if(incomingCatagory == GROUP_MOISTURE){
    dataDisp = "Moisture: " + String(incominghumidity) + "%";
    OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 13, 128);
    }
    if(incomingCatagory == GROUP_VALVE){
    dataDisp = "State: " + String(incomingstatus);
    OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 13, 128);
    }
    if(incomingCatagory == GROUP_POSITION){
    dataDisp = "Lat: " + String(incomingtemperature);
    OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 13, 128);
    dataDisp = "Lon: " + String(incominghumidity);
    OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 24, 128);
    }
    dataDisp = "ID: " + String(incomingnodeID);
    OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 36, 128);
    dataDisp = "RSSI: " + String(incomingrssi_display) + "    |" + String(MasterStatus);
    OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 48, 128);
  // if(OLED_MODE == OLED_130){
    // IOT_DEVICE.OLED_Display(dataDisp,4);
    // display.setContrast(255);
    // display.drawString(0,0,"INCOMING READINGS");
    // display.drawString(0,15,"Temperature: ");
    // display.drawString(64,15,String(incomingtemperature) + "ºC");
    // display.drawString(0,25,"Humidity: ");
    // display.drawString(64,25,String(incominghumidity) + "%");
    // display.drawString(0,35,"ID: ");
    // display.drawString(30,35,String(incomingnodeID));
    // display.drawString(0,45,"RSSI: ");
    // display.drawString(30,45,String(incomingrssi_display));
    // display.display();
  // }
  // #endif//OLED_SH1106
  #ifdef OLED_SSD1306
  if(OLED_MODE == OLED_096) {
    String dataDisp = "";
    // display1.clearDisplay();
    // IOT_DEVICE.OLED_Display("INCOMING READINGS",0);
    dataDisp = "Temperature: " + String(incomingtemperature) + "ºC";
    OLED_DISPLAY::L0 = dataDisp;
    // IOT_DEVICE.OLED_Display(dataDisp,1);
    dataDisp = "Humidity: " + String(incominghumidity) + "%";
    OLED_DISPLAY::L1 = dataDisp;
    // IOT_DEVICE.OLED_Display(dataDisp,2);
    dataDisp = "ID: " + String(incomingnodeID);
    OLED_DISPLAY::L2 = dataDisp;
    // IOT_DEVICE.OLED_Display(dataDisp,3);
    dataDisp = "RSSI: " + String(incomingrssi_display);
    OLED_DISPLAY::L3 = dataDisp;
    // IOT_DEVICE.OLED_Display(dataDisp,4);
    // display1.clearDisplay();
    // display1.setTextSize(1);
    // display1.setTextColor(WHITE);
    // display1.setCursor(0, 0);
    // display1.println("INCOMING READINGS");
    // display1.setCursor(0, 15);
    // display1.print("Temperature: ");
    // display1.print(incomingtemperature);
    // display1.cp437(true);
    // display1.write(248);
    // display1.print("C");
    // display1.setCursor(0, 25);
    // display1.print("Humidity: ");
    // display1.print(incominghumidity);
    // display1.print("%");
    // display1.setCursor(0, 35);
    // display1.print("ID: ");
    // display1.print(incomingnodeID);
    // display1.setCursor(0, 45);
    // display1.print("RSSI: ");
    // display1.print(incomingrssi_display);
    // display1.display();
  }
  #endif//OLED_SSD1306
  if(Debug){
    LOGLN("INCOMING READINGS");
    LOG("network ID: ");
    LOG(incomingReadings.networkID);
    LOG(" | nodeID: ");
    LOG(incomingReadings.nodeID);
    LOG(" | Category: ");
    LOG(incomingReadings.category);
    LOG(" | status: ");
    LOG(incomingReadings.status);
    LOG(" | Temperature: ");
    LOG(incomingReadings.temperature);
    LOG(" ºC");
    LOG(" | Humidity: ");
    LOG(incomingReadings.humidity);
    LOG(" | mbattery: ");LOG(incomingReadings.mbattery);LOG(" V");
    LOG(" | battery: ");LOG(incomingReadings.battery);LOG(" V");
    LOG(" %");
    LOG(" | RSSI: ");
    LOG(incomingReadings.RSSI);
    LOGLN();
  }
  delay(50);
  if(LEDType == 0){strip.setBrightness(0);strip.setPixelColor(0, 0x00ff00);strip.setPixelColor(4, 0x00ff00);strip.show(); }
  else{digitalWrite(PIXEL_PIN,LOW);}
}


void IoT_Device::OLED_Display(String msg,byte line){
  #ifdef OLED_SH1106
    if(OLED_MODE == OLED_130){

      display.setContrast(255);
      if(line == 0){
      display.drawString(0,0,msg);}
      if(line == 1){
      display.drawString(0,15,msg);}
      if(line == 2){
      display.drawString(0,25,msg);}
      if(line == 3){
      display.drawString(0,35,msg);}
      if(line == 4){
      display.drawString(0,45,msg);}
      display.display();
    }
    #endif//OLED_SH1106
    #ifdef OLED_SSD1306
    if(OLED_MODE == OLED_096) {
      display1.setTextSize(1);
      display1.setTextColor(WHITE);
      if(line ==  0){
      display1.setCursor(0, 0);
      display1.println(msg);}
      if(line ==  1){
      display1.setCursor(0, 15);
      display1.print(msg);}
      if(line ==  2){
      display1.setCursor(0, 25);
      display1.print(msg);}
      if(line ==  3){
      display1.setCursor(0, 35);
      display1.print(msg);}
      if(line ==  4){
      display1.setCursor(0, 45);
      display1.print(msg);}
      display1.display();
    }
    #endif//OLED_SSD1306
  }
void IoT_Device::OLED_Clear(){
  #ifdef OLED_SH1106
    if(OLED_MODE == OLED_130){
      display.clear();
    }
  #endif//OLED_SH1106
  #ifdef OLED_SSD1306
    if(OLED_MODE == OLED_096) {
      display1.clearDisplay();
    }
  #endif//OLED_SSD1306
}




/// @brief LED Display Function
/// @param replay 
/// @param speed 
void IoT_Device::LED_Signal(int replay, int speed)
{
  for(int i = 0 ; i < replay; i++){digitalWrite(PIXEL_PIN,HIGH);delay(speed);digitalWrite(PIXEL_PIN,LOW);delay(speed);}
}
void IoT_Device::larson_scan()
{
    int j;

  // Draw 5 pixels centered on pos.  setPixelColor() will clip any
  // pixels off the ends of the strip, we don't need to watch for that.
  strip.setPixelColor(pos - 3, 0x101010); // Dark red
  strip.setPixelColor(pos - 2, 0x404040); // Dark red
  strip.setPixelColor(pos - 1, 0x808080); // Medium red
  strip.setPixelColor(pos    , 0xFFFFFF); // Center pixel is brightest
  strip.setPixelColor(pos + 1, 0x808080); // Medium red
  strip.setPixelColor(pos + 2, 0x404040); // Dark red
  strip.setPixelColor(pos + 3, 0x101010); // Dark red



  strip.show();
  delay(200);

  // Rather than being sneaky and erasing just the tail pixel,
  // it's easier to erase it all and draw a new one next time.
  for(j=-3; j<= 3; j++) strip.setPixelColor(pos+j, 0);

  // Bounce off ends of strip
  pos += dir;
  if(pos < 0) {
    pos = 1;
    dir = -dir;
  } else if(pos >= strip.numPixels()) {
    pos = strip.numPixels() - 3;
    dir = -dir;
  }
}

void IoT_Device::ledFadeToBeat(int R,int G,int B,int bright) {

  for(int j = 0 ; j < 8 ; j++) {strip.setPixelColor(j, R, G, B);}
      strip.show();     
   int x = 3;
   for (int ii = 1 ; ii <bright ; ii = ii = ii + x){
     strip.setBrightness(ii);
     strip.show();              
     delay(20);
    }
    
    x = 3;
   for (int ii = bright ; ii > 3 ; ii = ii - x){
     strip.setBrightness(ii);
     strip.show();              
     delay(20);
     }
    
    }
// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void IoT_Device::colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.

void IoT_Device::theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.

void IoT_Device::rainbow(int wait) {
  // Hue of first pixel runs 3 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 3*65536. Adding 256 to firstPixelHue each time
  // means we'll make 3*65536/256 = 768 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 3*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.

void IoT_Device::theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

#ifdef ARDUINO_ARCH_ESP8266

void IoT_Device::MeshBegin()
{
  CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &RunMode);
    if(RunMode == MESHSLAVE){if(Debug)LOGLN("RunMode: Mesh Slave");}
    if(RunMode == MESHMODE){if(Debug)LOGLN("RunMode: Mesh (Node)");}
    if(RunMode == WIFIMODE){if(Debug)LOGLN("RunMode: Wifi (Gateway)");}
        // WiFi.setPhyMode (WIFI_PHY_MODE_11B);
    // WifiMode();

}

void IoT_Device::setup() {
  // Set device as a Wi-Fi Station
  // Init Serial Monitor
  Serial.begin(115200);
  // #ifndef ARDUINO_ARCH_ESP8266
  RF_Serial.begin(115200);
  // #endif//
  IOT_DEVICE.getConfig();
  if(Debug)LOGLN("\n\nButton Mullti");
    button.debounceTime   = 20;   // Debounce timer in ms
    button.multiclickTime = 250;  // Time limit for multi clicks
    button.longClickTime  = 1000; // Time until long clicks register
    button1.debounceTime   = 20;   // Debounce timer in ms
    button1.multiclickTime = 250;  // Time limit for multi clicks
    button1.longClickTime  = 1000; // Time until long clicks register

  if(LEDType == 0){
    strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
    strip.show();  // Initialize all pixels to 'off' 
  }
  else{pinMode(PIXEL_PIN,OUTPUT);}
 
  
        

  // Init OLED display
  #ifdef OLED_SH1106
  if(OLED_MODE == OLED_130){
    if(!display.init()) { 
    if(Debug)LOGLN(F("SH1106 allocation failed"));
    for(;;);
    }
    IOT_DEVICE.OLED_Display("VPlab Gateway",0);
  }
  #endif//OLED_SH1106
  #ifdef OLED_SSD1306
  if(OLED_MODE == OLED_096){
    if(!display1.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
      if(Debug)LOGLN(F("SSD1306 allocation failed"));
      for(;;);
    }
    IOT_DEVICE.OLED_Display("VPlab Gateway",0);

  }
  #endif//OLED_SSD1306

  //----------------------------------------------------------------
  // MQTT
  if(RunMode == WIFIMODE)mqttcommu.setup();
}

void IoT_Device::loop() {

}

#endif////////////////////////////////
#endif//IOTDEVICE_UI