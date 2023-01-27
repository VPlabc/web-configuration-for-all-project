// #01012023: update
// MQTT pub/sub
// node mesh master/slave
// gateway master


#include <Arduino.h>
#include "config.h"
#include "espcom.h"
#include "esp_oled.h"
#include "wificonf.h"
#include "FirmwareUpdate.h"
UpdateFW UDFWS;
#ifdef AUTOITGW_UI
#include "MQTTcom.h"
MQTTCOM mqttcommu;
#include "MeshWifi.h"
#include <Wire.h>
//______ Include
#include "AutoITGW.h" 
#include <PubSubClient.h>



#define SWITCHPIN 32
#define RELAYPIN 33
#define ANALOGPIN1 36
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
#define battery_cutoff_volt   3.3

// Define variables to store BME280 readings to be sent
byte networkID;
byte nodeID;
byte status;
byte state;
byte category;
float temperature;
float humidity;
float mbattery;
float battery;
byte OLED_MODE = 0;

byte ButtonUpdateFw = 0;
byte UpdateFw = 0;
byte ButtonPin = 13;
byte LEDPin = 2;
String NameBoard = "";
String mqtt_server = "";
String MQTTPort = "";
String TopicOut = "";
String TopicIn = "";
bool checkFW = false;
byte ModeRun1 = 0;
byte Result = 0;
String FW_version;
int ConnectRetry = 0;//Wifi connect retry


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
byte     current_status;      // 0=offline , 1=online    ,2=OTA

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

typedef struct struct_control_message {
    byte networkID;       //1
    byte nodeID;          //1
    byte status;          //1
} struct_control_message;


typedef struct struct_setting_message {
    byte networkID;       //1
    byte nodeID;          //1
    byte category;        //1
    byte time;            //1
} struct_setting_message;

struct_control_message DataControl;
struct_setting_message DataSetting;
// Create a struct_message called BME280Readings to hold sensor readings
struct_message DataSenddings;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

int             sensors_saved = 0;
uint8_t         incomingData[sizeof(struct struct_message)];
uint8_t         isDataControl[sizeof(struct struct_control_message)];
uint8_t         isDataSetting[sizeof(struct struct_setting_message)];
size_t          received_msg_length;
bool            new_sensor_found;


#include <SH1106.h>
// Initialize the OLED display using Wire library
SH1106  display(0x3C, 21, 22);

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
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
int     mode     = 0;    // Currently-active animation mode, 0-9
int pos = 0, dir = 1; // Position, direction of "eye"
bool STT = true;

void sendCurrentStatus() {
  Serial.println(current_status);
}
byte Auto_Device::getRunMode() {
  return RunMode;
}
byte Auto_Device::getDebug() {
  return Debug;
}
//----------------------------------------------------------------
//------------ Gateway Master Function ---------------------------
//----------------------------------------------------------------
void Auto_Device::Control(String netID, String ID, String State){
  if(RunMode == WIFIMODE){//Gateway Master
    DataControl.networkID = netID.toInt();
    DataControl.nodeID = ID.toInt();
    DataControl.status = State.toInt();
    memcpy(isDataControl, &DataControl, sizeof(DataControl));
    RF_Serial.write(isDataControl, sizeof(isDataControl));
    RF_Serial.write("\n");
    LOGLN("Control cmd");
  }
}

void Auto_Device::Setting(String netID, String ID, String CAT, String SleepTime){
  DataSetting.networkID = netID.toInt();
  DataSetting.nodeID = ID.toInt();
  DataSetting.category = CAT.toInt();
  DataSetting.time = SleepTime.toInt();
  memcpy(isDataSetting, &DataSetting, sizeof(DataSetting));
  RF_Serial.write(isDataSetting, sizeof(isDataSetting));
  RF_Serial.write("\n");
    LOGLN("Setting cmd");
}

void Auto_Device::UpdateStatus()// MQTT
{
  DynamicJsonDocument sensor(500);
  sensor["deviceid"] = incomingnodeID;
  sensor["networkid"] = incomingnetworkID;
  sensor["category"] = incomingCatagory;
  sensor["status"]  = incomingstatus;
  sensor["temperature"]  = incomingtemperature;
  sensor["humidity"]  = incominghumidity;
  sensor["mbattery"] = incomingmbattery;
  sensor["battery"] = incomingmbattery;
  sensor["rssi"] = incomingrssi_display;
  String payload = "";
  size_t n = serializeJson(sensor, payload);
  LOGLN(payload);
  mqttcommu.mqttPublish(payload);
}
//----------------------------------------------------------------
//-------------------------- Load Config -------------------------
//----------------------------------------------------------------
void Auto_Device::getConfig(){
  if(STT)LOGLN()
  Debug = DEBUG; 
  if(CONFIG::read_byte (EP_EEPROM_DEBUG, &Debug)){
    if(Debug && STT)Serial.println("Data");
    if(STT)LOGLN("Read Debug OK, value:" + String((Debug==0)?"Not Debug":"Debug"));
  }
  // networkID = 1;
  // if(CONFIG::read_byte (EP_EEPROM_NET_ID, &networkID)){
  //   if(Debug && STT)LOGLN("Read network ID OK, value:" + String(networkID));
  // }
  // nodeID = 2;
  // if(CONFIG::read_byte (EP_EEPROM_ID, &nodeID)){
  //   if(Debug && STT)LOGLN("Read ID OK, value:" + String(nodeID));
  // }
  // category = GROUP_RELAY;
  // if(CONFIG::read_byte (EP_EEPROM_CATEGORY, &category)){
  //   if(Debug && STT)LOGLN("Read CATEGORY OK, value:" + String(category));
  // }

  // status = 0;
  // temperature = 26.8;
  // humidity = 76.5;
  // mbattery = 4.1;
  // battery = 0;
  // battery = 0;
  // //define hardware

  // OLED_MODE = OLED_130;
  // if(CONFIG::read_byte (EP_EEPROM_OLED_TYPE, &OLED_MODE)){
  //   if(Debug && STT)LOGLN("Read OLED_MODE OK, value:" + String((OLED_MODE==0)?"OLED 0.96\"":"OLED 1.3\""));
  // }
  // ROLE = Node; 
  // if(CONFIG::read_byte (EP_EEPROM_ROLE, &ROLE)){
  //   if(Debug && STT)LOGLN("Read ROLE OK, value:" + String((ROLE == 0)?"Node":"Gateway"));
  // }
  // NameBoard = "VPlab";
  // if(CONFIG::read_string (EP_EEPROM_NAME, NameBoard, MAX_NAME_LENGTH)){
  //   if(Debug && STT)LOGLN("Read NAME OK, value:" + String(NameBoard));
  // }
  // CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &RunMode);
  // if(Auto_Device::RunMode > 2){
  //   CONFIG::write_byte (EP_EEPROM_WIFI_MODE, 1); 
  //   CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &RunMode);
  //   }
  //   if(RunMode == MESHSLAVE){if(Debug && STT)LOGLN("RunMode: Mesh Slave");}
  //   if(RunMode == MESHMODE){if(Debug && STT)LOGLN("RunMode: Mesh (Node)");}
  //   if(RunMode == WIFIMODE){if(Debug && STT)LOGLN("RunMode: Wifi (Gateway)");}
  // ButtonUpdateFw = USE;
  // if(CONFIG::read_byte (EP_EEPROM_FW_BUTTON, &ButtonUpdateFw)){
  //   if(Debug && STT)LOGLN("Read FW BUTTON OK, value:" + String((ButtonUpdateFw==0)?"Use":"Not Use"));
  // }
  // if(CONFIG::read_byte (EP_EEPROM_UPDATE_FW, &UpdateFw)){
  //   if(Debug && STT)LOGLN("Read UPDATE FW AUTO OK, value:" + String((UpdateFw==0)?"Use":"Not Use"));
  // }
  // if(CONFIG::read_byte (EP_EEPROM_PIN_BUTTON, &ButtonPin)){
  //   if(Debug && STT)LOGLN("Read Button Pin OK, value:" + String(ButtonPin));
  // }
  // if(CONFIG::read_byte (EP_EEPROM_PIN_LEDFULL, &LEDPin)){
  //   if(Debug && STT)LOGLN("Read LED Pin OK, value:" + String(LEDPin));
  // }
  // if(CONFIG::read_byte (EP_EEPROM_LED_TYPE, &LEDType)){
  //   if(Debug && STT)LOGLN("Read LED Type OK, value:" + String(LEDType));
  // }
  // PRGM_VERSION
    if(Debug && STT)LOGLN("Firmware Version:" + UDFWS.FirmwareVer);
      String str_= "";
  char mqttUserName[MAX_MQTT_USER_LENGTH + 1];
  char mqttUserPassword[MAX_MQTT_PASS_LENGTH + 1];
  char mqttbroker[MAX_MQTT_BROKER_LENGTH + 1];
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

  STT = false;
}

//----------------------------------------------------------------
//--------------------------   Display  -------------------------
//----------------------------------------------------------------
void Auto_Device::updateDisplay(){
  digitalWrite(PIXEL_PIN,HIGH);
  // Display Readings on OLED Display

  if(OLED_MODE == OLED_130){
    display.clear();
    display.setContrast(255);
    display.drawString(0,0,"INCOMING READINGS");
    display.drawString(0,15,"Temperature: ");
    display.drawString(64,15,String(incomingtemperature) + "ºC");
    display.drawString(0,25,"Humidity: ");
    display.drawString(64,25,String(incominghumidity) + "%");
    display.drawString(0,35,"ID: ");
    display.drawString(30,35,String(incomingnodeID));
    display.drawString(0,45,"RSSI: ");
    display.drawString(30,45,String(incomingrssi_display));
    display.display();
  }


  if(OLED_MODE == OLED_096) {
    display1.clearDisplay();
    display1.setTextSize(1);
    display1.setTextColor(WHITE);
    display1.setCursor(0, 0);
    display1.println("INCOMING READINGS");
    display1.setCursor(0, 15);
    display1.print("Temperature: ");
    display1.print(incomingtemperature);
    display1.cp437(true);
    display1.write(248);
    display1.print("C");
    display1.setCursor(0, 25);
    display1.print("Humidity: ");
    display1.print(incominghumidity);
    display1.print("%");
    display1.setCursor(0, 35);
    display1.print("ID: ");
    display1.print(incomingnodeID);
    display1.setCursor(0, 45);
    display1.print("RSSI: ");
    display1.print(incomingrssi_display);
    display1.display();
  }

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
  digitalWrite(PIXEL_PIN,LOW);
}


// void Auto_Device::OLED_Display(String msg,byte line){

//     if(OLED_MODE == OLED_130){

//       display.setContrast(255);
//       if(line == 0){
//       display.drawString(0,0,msg);}
//       if(line == 1){
//       display.drawString(0,15,msg);}
//       if(line == 2){
//       display.drawString(0,25,msg);}
//       if(line == 3){
//       display.drawString(0,35,msg);}
//       if(line == 4){
//       display.drawString(0,45,msg);}
//       display.display();
//     }
//     if(OLED_MODE == OLED_096) {
//       display1.setTextSize(1);
//       display1.setTextColor(WHITE);
//       if(line ==  0){
//       display1.setCursor(0, 0);
//       display1.println(msg);}
//       if(line ==  1){
//       display1.setCursor(0, 15);
//       display1.print(msg);}
//       if(line ==  2){
//       display1.setCursor(0, 25);
//       display1.print(msg);}
//       if(line ==  3){
//       display1.setCursor(0, 35);
//       display1.print(msg);}
//       if(line ==  4){
//       display1.setCursor(0, 45);
//       display1.print(msg);}
//       display1.display();
//     }
//   }
// void Auto_Device::OLED_Clear(){
//     if(OLED_MODE == OLED_130){
//       display.clear();
//     }
//     if(OLED_MODE == OLED_096) {
//       display1.clearDisplay();
//     }
// }




/// @brief LED Display Function
/// @param replay 
/// @param speed 
void Auto_Device::LED_Signal(int replay, int speed)
{
  for(int i = 0 ; i < replay; i++){digitalWrite(PIXEL_PIN,HIGH);delay(speed);digitalWrite(PIXEL_PIN,LOW);delay(speed);}
}


void Auto_Device::setup() {
  // Set device as a Wi-Fi Station
  // Init Serial Monitor
  Serial.begin(115200);
  // #ifndef ARDUINO_ARCH_ESP8266
  RF_Serial.begin(115200);
  // #endif//
  Auto_Device::getConfig();
  // if(Debug)LOGLN("\n\nButton Mullti");
  //   button.debounceTime   = 20;   // Debounce timer in ms
  //   button.multiclickTime = 250;  // Time limit for multi clicks
  //   button.longClickTime  = 1000; // Time until long clicks register
  //   button1.debounceTime   = 20;   // Debounce timer in ms
  //   button1.multiclickTime = 250;  // Time limit for multi clicks
  //   button1.longClickTime  = 1000; // Time until long clicks register
    
    pinMode(PIXEL_PIN,OUTPUT);
 
  // Init OLED display

  // if(OLED_MODE == OLED_130){
  //   if(!display.init()) { 
  //   if(Debug)LOGLN(F("SH1106 allocation failed"));
  //   for(;;);
  //   }
  //   Auto_Device::OLED_Display("VPlab Gateway",0);
  // }
  // if(OLED_MODE == OLED_096){
  //   if(!display1.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
  //     if(Debug)LOGLN(F("SSD1306 allocation failed"));
  //     for(;;);
  //   }
  //   Auto_Device::OLED_Display("VPlab Gateway",0);

  // }

  //----------------------------------------------------------------
  // MQTT
  mqttcommu.setup();
}

byte          receiver_status = 1;    // tracks the ESPNow receiver status
void Auto_Device::loop() {
  mqttcommu.loop();
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
              Auto_Device::updateDisplay();
              UpdateStatus();
            // sensorMessageReceived();
            // saveSensorData();
          // }
          }
        
        else {
          if (incomingData[0] == '0' && incomingData[1] == '2' && incomingData[2] == '3') receiver_status = 0;
          // if (incomingData[0] == '1') receiver_status = 1;
          // if (incomingData[0] == '2') receiver_status = 2;
          // if (incomingData[0] == 'D'&&incomingData[1] == 'a'&&incomingData[2] == 't'&&incomingData[3] == 'a') 
          //   receiver_status = 3;LOGLN("Slave Debug:");

          // Serial.write(RF_Serial.read());
      }
    }
}

#endif//AUTOITGW_UI