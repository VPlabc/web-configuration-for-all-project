
#include "config.h"
#ifdef Switch_UI
#include "LightTimer.h"

#include "WIC.h"
WIC Wic;
#include <EEPROM.h>

// extern "C" {
//     #include "user_interface.h"  // Required for wifi_station_connect() to work
// }

// #include <ESP8266WiFi.h>

#define FPM_SLEEP_MAX_TIME 0xFFFFFFF


#ifdef HomeKit
#include <arduino_homekit_server.h>
#endif//HomeKit
#ifdef Switch_UI
#include "../DataTransfer/data_transfer.h"
DataTransfer SaveDATATRANS;
#define Debug_Ser Serial
int Timezone = 7;
boolean SetBrit = false;
byte brightness = 0;
#ifdef ARDUINO_ARCH_ESP8266
#define button 0//14 | Sinlink 12
#define Relay 12//Sinlink 4
#define LED_Relay 5//4 | Sinlink 13

#define LED_STT 2//
#else //ESP32
#define button 0//14 | Sinlink 12
#define Relay 23//Sinlink 4
#define LED_Relay 25//4 | Sinlink 13

#define LED_STT 27//
// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

#endif//ARDUINO_ARCH_ESP

#if defined (ASYNCWEBSERVER)
#include "asyncwebserver.h"
#else
#include "syncwebserver.h"
#endif
#endif//Switch_UI

int TimeOutBeginAutos = 0;
int TimeOutBeginTime = 0;
int32_t seconds = 0;
int32_t seconds1 = 0;
int32_t secondsminOn = 0;
int32_t secondsminOff = 0;
int32_t secondsOn[20];
int32_t secondsOf[20];
byte BrightNess[20];
int CountBrightnes = 0;
int countDown = 0;
int32_t timesleep = 0;
bool update = false;
bool update1 = false;
 byte beforeStep = 0;
 bool beforeState = false;

void WiFiOff();

#ifdef HomeKit
  #define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);
//==============================
// HomeKit setup and loop
//==============================

extern "C" homekit_server_config_t config;


  static uint32_t next_heap_millis = 0;

extern "C" homekit_characteristic_t cha_switch_on;

#define PIN_BUTTON 0 // Use the Flash-Button of NodeMCU

#define HOMEKIT_PROGRAMMABLE_SWITCH_EVENT_SINGLE_PRESS   0
#define HOMEKIT_PROGRAMMABLE_SWITCH_EVENT_DOUBLE_PRESS   1
#define HOMEKIT_PROGRAMMABLE_SWITCH_EVENT_LONG_PRESS     2
////////////////////////////////////////////////////////////////////

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_characteristic_t cha_switch_on;

#define PIN_SWITCH 12

  //Called when the switch value is changed by iOS Home APP
void LightTimer::cha_switch_on_setter(const homekit_value_t value) {
	bool on = value.bool_value;
	cha_switch_on.value.bool_value = on;	//sync the value
	LOG_D("Switch: %s", on ? "ON" : "OFF");
  //if(on == true){SendRFOn();}
  //if(on == false){SendRFOff();}
	digitalWrite(PIN_SWITCH, on ? LOW : HIGH);
}

  void LightTimer::Homekit_Setup() {
    pinMode(Relay, OUTPUT);
    digitalWrite(Relay, HIGH);

    //Add the .setter function to get the switch-event sent from iOS Home APP.
    //The .setter should be added before arduino_homekit_setup.
    //HomeKit sever uses the .setter_ex internally, see homekit_accessories_init function.
    //Maybe this is a legacy design issue in the original esp-homekit library,
    //and I have no reason to modify this "feature".
    cha_switch_on.setter = cha_switch_on_setter;
    arduino_homekit_setup(&config);

    //report the switch value to HomeKit if it is changed (e.g. by a physical button)
    //bool switch_is_on = true/false;
    //cha_switch_on.value.bool_value = switch_is_on;
    //homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
  }

  void LightTimer::Homekit_Loop() {
    //arduino_Homekit_Loop();
    const uint32_t t = millis();
    if (t > next_heap_millis) {
      // show heap info every 5 seconds
      next_heap_millis = t + 5 * 1000;
      LOG_D("Free heap: %d, HomeKit clients: %d",
          ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

    }
  }

#endif//HomeKit

LightTimer::LightTimer()
{

}

void LightTimer::Setup()
{
    #ifdef Switch_UI    
#ifdef Time
      configTime(Timezone * 3600, 0, "pool.ntp.org", "time.nist.gov"); 
      Debug_Ser.println("\nWaiting for time");
      while (!time(nullptr)) {
      Debug_Ser.print(".");
        delay(1000);
      }
#endif//Time
pinMode(Relay, OUTPUT);
pinMode(button, INPUT);
Load();
#ifdef ARDUINO_ARCH_ESP8266
#else//ESP32
  ledcSetup(ledChannel, freq, resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(Relay, ledChannel);
#endif// ARDUINO_ARCH_  
#endif//Switch_UI
#ifdef HomeKit
LightTimer::Homekit_Setup();
#endif//HomeKit
delay(1000);
    readtime();
    Load();
    LoadBool();
    if(device_one_state){update1 = true;update = false;setStatus(true);state = true;last_brightness = brightness;
    }
    else{update = true;update1 = false;setStatus(false);state = false;}
}


void LightTimer::Loop()
{
#ifdef Switch_UI
  static uint32_t last_Timer_update= 0;
  uint32_t now_Timer = millis();
  if (now_Timer - last_Timer_update > 1000) {
    last_Timer_update = now_Timer;
    readtime();
    Load();
    if(SetBrit == false) {if(locks == true){wsSendToWeb();}//SetBrit = true;
      byte brightness_Recvice = map(brightness,0 ,100, 0, 255);
      SetBrightLight(brightness_Recvice);
      //Debug_Ser.println("state brightness: " + String(SetBrit));
      //Debug_Ser.println("last_brightness: " + String(last_brightness));
    }
        // if(TimeOutBeginTime == 5){TimeOutBeginTime = 6;
        //   update = true;
        // }
    for(byte i = 0 ; i < 20 ; i++){
    //TimerOn[0][i] = ;
      if(Timer[0][i] < 24 && Timer[1][i] < 60){
        if((hours ==  Timer[0][i]  && mins == Timer[1][i]  && (secs > 0 && secs <= 10)) && Timer[2][i] == OnEnb){
          if(LightTimer::locks == false){LightTimer::locks = true;if(Timer[3][i] > 100){Timer[3][i] = 0;}last_brightness = brightness = Timer[3][i];wsSendToWeb();setStatus( true);state = true;WriteValue(Switch, "Auto");
          Debug_Ser.println(String(hours) + ":" + String(mins) + " | Bright read: "+String(Timer[3][i]));
          update1 = true;TimeOutBeginTime = 0;TimeOutBeginAutos=0;
          }//if(LightTimer::locks == false)
        }
        if((hours ==  Timer[0][i] && mins == Timer[1][i] && (secs > 0 && secs <= 10)) && Timer[2][i] == OfEnb){
          if(LightTimer::locks == false){LightTimer::locks = true;if(Timer[3][i] > 100){Timer[3][i] = 0;}last_brightness = brightness = Timer[3][i];wsSendToWeb();setStatus(false);state = false;WriteValue(Switch, "Auto");
          Debug_Ser.println(String(hours) + ":" + String(mins) + " | Bright read: "+String(Timer[3][i]));
          update = true;TimeOutBeginTime = 0;TimeOutBeginAutos=0;
          }//if(LightTimer::locks == false)
        }
                if(secs >= 11 ){LightTimer::locks = false;}

      
      }
    }   
        if(device_one_state == false){
          #ifdef DeepSleepMode
            if(Wic.Auto == true){ 
              Debug_Ser.println("Sleep");Debug_Ser.println("countDown:" + String(countDown));delay(500);
                   if(countDown < 3600 && countDown > 80){timesleep = countDown - 70;} else{timesleep = 60;}
                   if(countDown > 610){timesleep = 600;}
                   Debug_Ser.println("timesleep:" + String(timesleep));
              ESP.deepSleep(timesleep*1000000);}
          #endif//DeepSleepMode
        }
      
    countDown--;//if(countDown >= 0)Debug_Ser.println("countDown:" + String(countDown));
      if(countDown == 0) {countDown = 0;bool locker = false;
      if(state == true && locker == false){Wic.Manual = false;Debug_Ser.println("Auto Mode");locker = true;
      wsSendToWeb();setStatus(false);state = false;WriteValue(Switch, "Auto");}
      if(state == false && locker == false){Wic.Manual = false;Debug_Ser.println("Auto Mode");locker = true;
      wsSendToWeb();setStatus(true);state = true;WriteValue(Switch, "Auto");
      last_brightness = brightness = CountBrightnes;}
      }
      
      if(countDown < 0) {countDown = -1;} 
      if(update == true){CaculaTime();update = false;update1 = false;if(secondsminOn>0)countDown = secondsminOn;
      secondsminOn = secondsOn[0];
      for(byte j = 0 ; j < 20 ; j++){
        if(secondsminOn > secondsOn[j] && secondsOn[j] > 0){secondsminOn = secondsOn[j];CountBrightnes = BrightNess[j];}
        countDown = secondsminOn;
      }
      if(secondsminOn>0)countDown = secondsminOn;
        Debug_Ser.println("Seconds on = " + String(secondsminOn));
      }

      if(update1 == true){CaculaTime();update1 = false;update = false;if(secondsminOff>0)countDown = secondsminOff;
      secondsminOff = secondsOf[0];
      for(byte j = 0 ; j < 20 ; j++){
        if(secondsminOff > secondsOf[j] && secondsOf[j] > 0){secondsminOff = secondsOf[j];}
        countDown = secondsminOff;
      }
        Debug_Ser.println("Seconds off = " + String(secondsminOff));
      }


        
          
    #ifdef DeepSleepMode
     TimeOutBeginAutos++;TimeOutBeginTime++;
     if(TimeOutBeginTime > 1 && TimeOutBeginTime < 6){
      //Debug_Ser.println(String(hours) + ":" + String(mins)+ ":" + String(secs));     
      }
    if(device_one_state == true && TimeOutBeginAutos > 60 && Wic.Manual == false)
    {
      TimeOutBeginAutos = 0;
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
      WiFiOff();
      if(countDown >= 0)Debug_Ser.println("countDown:" + String(countDown));
      //ESP.deepSleep(60);
    }
    if(device_one_state == false && TimeOutBeginAutos > 60 && countDown > 60 && Wic.Manual == false)
    {
      TimeOutBeginAutos = 0;
      Wic.Auto = true;
      Debug_Ser.println("Auto Mode");
    }   
    #endif//DeepSleepMode  
  }
  #endif//Switch_UI
  
}

void LightTimer::LoadData(){
#ifdef Switch_UI           
            Load();          
#endif//Switch_UI
}

#ifdef HomeKit
LightTimer::Homekit_Loop();
#endif//HomeKit

#ifdef Switch_UI
void LightTimer::SetTime(String Data,byte ID)
{
  byte hour = Data.toInt()/3600;
  byte min = (Data.toInt()/60)-(60 * (Data.toInt()/3600));
  if(Data.toInt() > 0){//analogWrite(relay, brightness);
    if(states[ID-1] == 1){Save(ID, hour,min,OnEnb,brightness);}else{Save(ID, hour,min,OfEnb,brightness);}
  }else{
    if(states[ID-1] == 0){Save(ID, hour,min,OnDis,brightness);}else{Save(ID, hour,min,OfDis,brightness);} 
  }
  Load();
  Debug_Ser.print("Timer On"+ String(ID) + " ");Debug_Ser.print(Timer[0][ID]);
        Debug_Ser.print(":");Debug_Ser.print(Timer[1][ID]);
        if(Timer[2][ID] == OnDis){ Debug_Ser.println(" On - Dis");}
        if(Timer[2][ID] == OnEnb){ Debug_Ser.println(" On - Enb");}
        if(Timer[2][ID] == OfDis){ Debug_Ser.println(" Off - Dis");}
        if(Timer[2][ID] == OfEnb){ Debug_Ser.println(" Off - Enb");} 
        if(Timer[3][0] > 100){Timer[3][0] = 0;}   
        Debug_Ser.print(" Bright "+ String(Timer[3][ID]) + "%");
        brightness = Timer[3][ID];
}
void LightTimer::setStatus(boolean st)
{
  String State = "none";
  if (st)
  {
    //if(last_brightness == 0)last_brightness = 10;
    device_one_state = true;
    Debug_Ser.print("Change state:");Debug_Ser.println(st);
    //Debug_Ser.println("last_brightness: " + String(last_brightness));
    //brightness = last_brightness;
    byte brightness_Recvice = map(brightness,0 ,100, 0, 255);
    SetBrightLight(brightness_Recvice);
    digitalWrite(LED_Relay,LOW);
    digitalWrite(LED_STT,HIGH);
    SaveBool();
    State = "ON";update1 = true;TimeOutBeginTime = 0;
    
#ifdef Blynks
    Blynk.virtualWrite(V2, 1);
    Blynk.virtualWrite(V1, 1);
#endif//Blynks
  }
  else
  {
    device_one_state = false;
    Debug_Ser.print("Change state:");Debug_Ser.println(st);
    //Debug_Ser.println("last_brightness: " + String(last_brightness));
    //brightness = 0;
    SetBrightLight(0);
    digitalWrite(Relay, LOW); // Xuất trạng thái  ra chân GPIO16
    digitalWrite(LED_Relay,HIGH);
    digitalWrite(LED_STT,LOW);
    SaveBool();
    State = "OFF";update = true;TimeOutBeginTime = 0;
#ifdef Blynks
    Blynk.virtualWrite(V2, 0);
    Blynk.virtualWrite(V1, 0);
#endif//Blynks
  }
CaculaTime();
#ifdef Volt_FEATURE
  voltRead();
#endif//Volt_FEATURE
#ifdef Blynks
        Blynk.virtualWrite(V6, humidity);
        Blynk.virtualWrite(V5, temperature);
        //Blynk.virtualWrite(V2, digitalRead(Relay));
#endif// Blynks        
        
//        String Messenger = "Grow Light: " + State + "| Temperature: " + String(temperature) + "|";
//        Messenger += "Humidity: " + String(humidity)+ "Bright: " + String(brightness);
        String Messenger = String(state) + "| Light: " + State + "| vin: " + String(vin) + "|";
        Messenger += "brightness: " + String(brightness);
        
#ifdef Blynks
          Blynk.notify(Messenger);delay(2000);
#endif //Blynks          
        Debug_Ser.println(Messenger);
}
void LightTimer::SetBrightLight(byte Bright) {
#ifdef ARDUINO_ARCH_ESP8266
    analogWrite(Relay, Bright); // Xuất trạng thái  ra chân GPIO16
#else//ESP32
    ledcWrite(ledChannel, Bright);
#endif// ARDUINO_ARCH_ 
}
void LightTimer::changestate() {
  state = !state;
  setStatus(state);
  WriteValue(Switch, "Manual");
}
void LightTimer::reciverDataFromWeb(uint8_t *payload)
{
      if (payload[0] == 'N') {
        wsSendToWeb();setStatus( true);state = true;
        WriteValue(Switch, "Web");Debug_Ser.println("Bright before:" + String(last_brightness));
      }
      else if (payload[0] == 'F') {
        last_brightness = brightness = 0;wsSendToWeb();setStatus(false);state = false;
        WriteValue(Switch, "Web");
      }
      else if (payload[0] == 'L' && payload[1] == 'A') {
        //LoadLog();
      //ReadValue(Switch,"1234");
      wsSendToWeb();
      }
      else if (payload[0] == 'D') {
        //DeleteLog();LogID = LoadLog();
        //String path;
        //SPIFFS.remove(path);
        //path = String();
        wsSendToWeb();
      }
      
      else if (payload[0] == 'L' && payload[1] == 'H') {
         //int hourBegin = 0;//(payload[2] - 48);
          //LoadSensorLog(0);
          //wsSendDatalogHourToWeb(hourBegin);
      }
      else if(payload[0] == '!'){//set brightness
        SetBrit = true;
        uint16_t brightness_Web = (uint16_t) strtol((const char *) &payload[1], NULL, 10);
        byte brightness_Recvice = map(brightness_Web,0 ,100, 0, 255);
        SetBrightLight(brightness_Recvice);
        //Debug_Ser.println("Set brightness: " + String(brightness_Web));
      }
      else if(payload[0] == '#'){
        uint16_t brightness_Web = (uint16_t) strtol((const char *) &payload[1], NULL, 10);
        byte brightness_Recvice = map(brightness_Web,0 ,100, 0, 255);
        SetBrightLight(brightness_Recvice);
        last_brightness = brightness = brightness_Web;
         //Debug_Ser.println("brightness: " + String(brightness));
        // Debug_Ser.print("brightness= ");
        // Debug_Ser.println(brightness_Web);
        // Debug_Ser.println("last_brightness: " + String(last_brightness));
        delay(100);wsSendToWeb();
      }
      //////////////////ID 0 _ HOUR 1_2 _ MIN 3_4 _ STATE 5
      //////////////////1 0 0304
      else if (payload[0] - 48 >= 0) {
      byte ID = ((payload[0] - 48)*10)+(payload[1] - 48);
        Save(ID, (((payload[2] - 48)*10)+(payload[3] - 48)),(((payload[4] - 48)*10)+(payload[5] - 48)),(payload[6] - 48),(((payload[7] - 48)*10)+(payload[8] - 48)));
        Load();SetBrit = false;LightTimer::locks = true;
        // Debug_Ser.print("Set Timer On "+ String(ID) + " ");Debug_Ser.print(Timer[0][ID]);
        // Debug_Ser.print(":");Debug_Ser.print(Timer[1][ID]);
        // if(Timer[2][ID] == OnDis){ Debug_Ser.print(" On - Dis");}
        // if(Timer[2][ID] == OnEnb){ Debug_Ser.print(" On - Enb");}
        // if(Timer[2][ID] == OfDis){ Debug_Ser.print(" Off - Dis");}
        // if(Timer[2][ID] == OfEnb){ Debug_Ser.print(" Off - Enb");}
        // Debug_Ser.println(" Bright "+ String(Timer[3][ID]) + "%");
        delay(100);wsSendToWeb();
      }
}
//////////////////////////////////// Sendata //
void LightTimer::wsSendToWeb()
{
  byte battery = map(vin, 0.1, 25.50,0 ,100);
  String json = "{";
  //-------------------------
  json += "\"Data\":0,";
  json += "\"state\":";//  "state":
  if (device_one_state == true) json += "\"on\"";
  else json += "\"off\"";
  json += ",\"volt\":";//  "volt":
  json += "\"" + String(vin) + "V\"";
  json += ",\"battery\":";//  "battery":
  json += "\"" + String(battery) + "%\"";
  json += ",\"bright\":";//  "bright":
  json += "\"" + String(brightness) + "%\"";
  json += ",\"temp\":";//  "state":
  json += "\"" + String(temperature) + "C\"";
  json += ",\"humi\":";//  "state":
  json += "\"" + String(humidity) + "%\"";
  json += ",\"Timer\":[";//  "state":
  json += "{\"Giờ\":";
  json += Timer[0][0];
  json += ",\"Phút\":";
  json += Timer[1][0];
  json += ",\"Trạng Thái\":";
  json += Timer[2][0];
  json += ",\"độ sáng\":";
  if(Timer[3][0] > 100){Timer[3][0] = 0;}
  json += Timer[3][0];
  json += "}";
  for(byte i = 1 ; i < 20 ; i++){
  json += ",{\"Giờ\":";
  json += Timer[0][i];
  json += ",\"Phút\":";
  json += Timer[1][i];
  json += ",\"Trạng Thái\":";
  json += Timer[2][i];
  json += ",\"độ sáng\":";
  if(Timer[3][i] > 100){Timer[3][i] = 0;}
  json += Timer[3][i];
  json += "}";
  }
  json += "],\"State\":[";//  "state":
  json += "{\"Giờ\":";
  json += State[0][0];
  json += ",\"Phút\":";
  json += State[1][0];
  json += ",\"Giây\":";
  json += State[2][0];
  json += ",\"Trạng thái\":";
  json += State[3][0];
  json += ",\"độ sáng\":";
  json += State[4][0];
  json += "}";
  for(byte i = 1 ; i < LogID + 1 ; i++){
  json += ",{\"Giờ\":";
  json += State[0][i];
  json += ",\"Phút\":";
  json += State[1][i];
  json += ",\"Giây\":";
  json += State[2][i];
  json += ",\"Trạng thái\":";
  json += State[3][i];
  json += ",\"độ sáng\":";
  json += State[4][i];
  json += "}";
  }
  json += "]}";
//   int numcl = 0;
//   for (numcl = 0; numcl < max_ws_client; numcl++)
//   {
//     if (wsClientNumber[numcl] != -1)
//       socket_server->sendTXT(wsClientNumber[numcl], json);
//   }
  //sentDataWS(json);
  socket_server->broadcastTXT(json);
  //Debug_Ser.println(json);
  json = String();     
}
/////////////////////////////////// save data
void LightTimer::Save(byte ID, byte hourSet,byte minSet,byte state,byte bright)
{
  EEPROM.begin (EEPROM_SIZE);
  EEPROM.write(ADDRESS + (ID*4), hourSet);
  EEPROM.write(ADDRESS + (ID*4) + 1, minSet);
  EEPROM.write(ADDRESS + (ID*4) + 2, state);
  EEPROM.write(ADDRESS + (ID*4) + 3, bright);
  EEPROM.commit();
  EEPROM.end();
  if(state == OnEnb)update = true;
  if(state == OfEnb)update1 = true;
  Wic.Manual = true;Debug_Ser.println("Manual Mode");
}
void LightTimer::Save(byte ID, byte state)
{
  EEPROM.begin (EEPROM_SIZE);
  EEPROM.write(ADDRESS + (ID*3) + 2, state);
  EEPROM.commit();
  EEPROM.end();
}
void LightTimer::Load()
{
  EEPROM.begin (EEPROM_SIZE);
  for(byte i = 0 ; i < 20 ; i++){
  Timer[0][i] = EEPROM.read(ADDRESS + i*4);
  Timer[1][i] = EEPROM.read(ADDRESS + i*4+1);
  Timer[2][i] = EEPROM.read(ADDRESS + i*4+2);
  Timer[3][i] = EEPROM.read(ADDRESS + i*4+3);
  
  }
  
  EEPROM.end();
}
void LightTimer::WriteValue(byte types, String usr)
{
  readtime();//LoadLog();
  String filename = "";
  if(types == Sensors){filename = "/sensor.txt";}
  if(types == Switch){filename = "/switch.txt";
  byte statelog = 0;
  if(device_one_state == true && usr == "Auto"){statelog = 0;}
  if(device_one_state == true && usr == "Manual"){statelog = 1;}
  if(device_one_state == true && usr == "Web"){statelog = 2;}
  if(device_one_state == false && usr == "Auto"){statelog = 3;}
  if(device_one_state == false && usr == "Manual"){statelog = 4;}
  if(device_one_state == false && usr == "Web"){statelog = 5;}
  LogID++;if(LogID > 100){LogID = 0;}
  //SaveLog(LogID,hours,mins,secs,statelog);
  //LoadLog();
  }
}
void LightTimer::SaveBool()
{
  EEPROM.begin (EEPROM_SIZE);
  EEPROM.write(ADDRESSLOGBit,SaveDATATRANS.EncodeBitRespond(0,0,device_one_state,0,0,0,0,0));
  EEPROM.write(ADDRESSBRIGHT,brightness);
  EEPROM.commit();
  EEPROM.end();
  LoadBool();
}
void LightTimer::LoadBool()
{
  EEPROM.begin (EEPROM_SIZE);
  device_one_state = SaveDATATRANS.DecodeBitRespond(EEPROM.read(ADDRESSLOGBit)).Bit2;
  brightness = EEPROM.read(ADDRESSBRIGHT);
  EEPROM.end();
  Debug_Ser.print("State:"); Debug_Ser.println(device_one_state);
  state = device_one_state;
}
extern "C" {
  //#ifdef 
 #ifdef ARDUINO_ARCH_ESP8266
  uint16_t readvdd33(void);
 #else //ESP32
 #endif//
}
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void LightTimer::voltRead()
{
  #ifdef ARDUINO_ARCH_ESP8266
  vdd = readvdd33() / 1000.0;
  #else //ESP32
  vdd = 3.3;//ESP.getVcc() / 1000.0;
  #endif//
    for(int i = 0 ; i < 100 ; i++){
     value = value + analogRead(A0);
    }
    value = value / 100;
   vout = (value * vdd) / 1024.0; // see text
   vin = ((vout / (R2/(R1+R2)) * 4) - 1); 
   if (vin<0.09) {
   vin=0.0;//statement to quash undesired reading !
  //vin = mapfloat(vin, 2.8, 4.2, 0, 100); //2.8V as Battery Cut off Voltage & 4.2V as Maximum Voltage

  }
}
void LightTimer::CaculaTime()
{

      Debug_Ser.println("caculation time");
      Debug_Ser.println(String(hours) + ":" + String(mins)+ ":" + String(secs));
      byte ji = 0;
    for(byte i = 0 ; i < 20 ; i++){
      if((hours <= Timer[0][i])  && Timer[2][i] == OnEnb && device_one_state == false){
      Debug_Ser.println(String(Timer[0][i]) + ":" + String(Timer[1][i]) + "|" + String(Timer[2][i]) + "| Bright:" + String(Timer[3][i]));
        seconds = 0;
        seconds = (hours * 3600) + (mins * 60) + secs ;
        seconds = (((Timer[0][i] * 3600) + (Timer[1][i] * 60) + 5)) - seconds;
        // if(hours < Timer[0][i]){seconds = (Timer[0][i] - hours)*3600;}
        // if(mins <  Timer[1][i]){seconds = seconds + ((Timer[1][i] - mins)*60);
        // seconds = seconds + (60 - secs);}
        // if(mins >  Timer[1][i]){seconds = seconds + ((60 - mins)*60);
        // seconds = seconds + (60 - secs);}
        if(seconds > 0){
          //Debug_Ser.println(String(Timer[0][i]) + ":" + String(Timer[1][i]) + "|" + String(Timer[2][i]));
          //Debug_Ser.println(String(seconds));
          BrightNess[ji] = Timer[3][i];
          secondsOn[ji] = seconds;ji++;}
      }
      

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////     
      if((hours <= Timer[0][i]) && Timer[2][i] == OfEnb && device_one_state == true ){
        //Debug_Ser.println(String(Timer[0][i]) + ":" + String(Timer[1][i]) + "|" + String(Timer[2][i]));
        seconds = 0;
        seconds = (hours * 3600) + (mins * 60) + secs ;
        seconds = (((Timer[0][i] * 3600) + (Timer[1][i] * 60) + 5)) - seconds;
        // if(hours < Timer[0][i]){seconds = (Timer[0][i] - hours)*3600;}
        // if(mins <  Timer[1][i]){seconds = seconds + ((Timer[1][i] - mins)*60);
        // seconds = seconds + (60 - secs);}
        // if(mins >  Timer[1][i]){seconds = seconds + ((60 - mins)*60);
        // seconds = seconds + (60 - secs);}
        if(seconds > 0){
          //Debug_Ser.println(String(Timer[0][i]) + ":" + String(Timer[1][i]) + "|" + String(Timer[2][i]));
          //Debug_Ser.println(String(seconds));
          secondsOf[ji] = seconds;ji++;}
      }

      //if(update == true){Debug_Ser.println(String(Timer[0][i]) + ":" + String(Timer[1][i]) + "|" + String(Timer[2][i]));}
    }
}

void WiFiOff() {

    //Serial.println("diconnecting client and wifi");
    //client.disconnect();
    wifi_station_disconnect();
    wifi_set_opmode(NULL_MODE);
    wifi_set_sleep_type(MODEM_SLEEP_T);
    wifi_fpm_open();
    wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);

}

#ifdef Time 
void LightTimer::readtime()
{
  time_t now = time(nullptr);
  //Wed Sep 1 21:59:03 2021
  
  hours = ((ctime(&now)[11]-48)*10)+(ctime(&now)[12]-48);
  mins = ((ctime(&now)[14]-48)*10)+(ctime(&now)[15]-48);
  secs = ((ctime(&now)[17]-48)*10)+(ctime(&now)[18]-48);
}
#endif//Time
#endif//Switch_UI

#endif//Switch_UI