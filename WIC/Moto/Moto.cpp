#include <Arduino.h>
#include "config.h"
#include "espcom.h"
#include "esp_oled.h"
#include "wificonf.h"
#include "FirmwareUpdate.h"
#ifdef Moto_UI
#include "Moto/Moto.h"
#include <esp_oled.h>
#include <Wire.h>
#include <ds3231.h>
#include <time.h>
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire ds18x20[] = { SensorPin1, SensorPin2 };
const int oneWireCount = sizeof(ds18x20) / sizeof(OneWire);
DallasTemperature sensor[oneWireCount];

long previousMillis_2 = 0;
long previousMillis_DS1 = 0;
long previousMillis_DS2 = 0;
float Bat_volt;
float TemperatureEnegy;
float Temperature;
#include <ClickButton.h>
int function = 0;
int LEDfunction;
byte ModeRun1 = 0;
bool Result;
byte UPdateAffter15days = 0;
int voltage_offset = 20;// set the correction offset value
byte Menu = 0;
float Bat = 0;
bool checkFW = false;
// Instantiate ClickButton objects in an array
  ClickButton button1(Button, LOW, CLICKBTN_PULLUP);

const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

UpdateFW MOTOUPDATEFW;

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void Moto::setup(){
////////////////////////////////// DS18B20////////////////////////////////////
  // Start up the library on all defined bus-wires
  DeviceAddress deviceAddress;
  for (int i = 0; i < oneWireCount; i++) {
    sensor[i].setOneWire(&ds18x20[i]);
    sensor[i].begin();
    if (sensor[i].getAddress(deviceAddress, 0)) sensor[i].setResolution(deviceAddress, 12);
  }
////////////////////////////////////////////////////////////////////////
DS3231_init(DS3231_CONTROL_INTCN);
pinMode(GearN, INPUT_PULLUP);
pinMode(Gear1, INPUT_PULLUP);
pinMode(Gear2, INPUT_PULLUP);
pinMode(Gear3, INPUT_PULLUP);
pinMode(Gear4, INPUT_PULLUP);
pinMode(Gear5, INPUT_PULLUP);
pinMode(VoltSensorPin, INPUT);
pinMode(Light, OUTPUT);
pinMode(LEDButton, OUTPUT);
    button1.debounceTime   = 20;   // Debounce timer in ms
    button1.multiclickTime = 250;  // Time limit for multi clicks
    button1.longClickTime  = 1000; // Time until long clicks register
digitalWrite(LEDButton, HIGH);
  Result = CONFIG::read_byte (EP_EEPROM_UPDATE_MODE, &UPdateAffter15days);
#ifndef DEBUG_FLAG_  
     if(WiFi.status() == WL_CONNECTED)
     {
       updateTime();
     }
#else
if(UPdateAffter15days == false){
if(WiFi.status() == WL_CONNECTED)
{
  updateTime();
}
  Result = CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &ModeRun1);
  if(ModeRun1 == 1){Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, 0);}//ON WiFi Run WiFi Config Mode
}
else{
GetTime();
  if(Days == 15){
    if(WiFi.status() == WL_CONNECTED)
    {
      updateTime();
    }
  Result = CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &ModeRun1);
  if(ModeRun1 == 1){Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, 0);}//ON WiFi Run WiFi Config Mode
}
}
#endif//DEBUG_FLAG
  OLED_DISPLAY::clear_lcd();
}
bool OFFWIFI = false;bool SOS = false;byte count = 0;byte GearPos = 0;
void Moto::loop(){
  button1.Update();
  if (button1.clicks != 0) function = button1.clicks;
  
  // Toggle LED on single clicks
  if(button1.clicks == 1 && Menu == 0){function = 0;digitalWrite(Light, !digitalRead(Light));LOGLN("Single click");}
  if(button1.clicks == 2 && Menu == 0){function = 0;SOS = !SOS;if(SOS == false){digitalWrite(LEDButton, HIGH);}
  LOGLN("Single click2");}
  if(button1.clicks == 3 && Menu > 0){function = 0;Menu = 0;OLED_DISPLAY::clear_lcd();
  LOGLN("Single click3 Menu 0");}
  if(button1.clicks == 3 && Menu == 0){function = 0;Menu = 1;OLED_DISPLAY::clear_lcd();
  LOGLN("Single click3 Menu 1");}
  if(button1.clicks == 1 && Menu > 0){function = 0;Menu++;if(Menu > 3){Menu = 1;}OLED_DISPLAY::clear_lcd();}
  if(button1.clicks == 2 && Menu == 3){checkFW = true;function = 0;MOTOUPDATEFW.ShowMess("Checking...");
  LOGLN("Check FW");}

  // fade if button is held down during single-click
  if(function == -1 && button1.depressed == true)
  {
    LOGLN("hold button");
    if(Menu == 0){wifi_b = false;WiFi.disconnect();WiFi.mode (WIFI_OFF);OLED_DISPLAY::clear_lcd();}
    if(Menu > 0){Menu = 0;OLED_DISPLAY::clear_lcd();}
    function = 0;
  } 
  if(function == -2 && button1.depressed == true)
  {
    LOGLN("hold button 2");
      wifi_b = true;
      if(GearPos == 10){GearPos = 0;OLED_DISPLAY::clear_lcd();}
    OLED_DISPLAY::clear_lcd();
    if(wifi_config.GetWifiMode() == true){
      if(wifi_b == false){WiFi.disconnect();WiFi.mode (WIFI_OFF);}
      else{
        WiFi.mode(WIFI_STA);
          if (!wifi_config.Setup() ) {}
          OLED_DISPLAY::clear_lcd();
          OLED_DISPLAY::BigDisplay("WiFi ON", 15, 17);
          OLED_DISPLAY::setCursor(40, 48);
          ESPCOM::print(WiFi.localIP().toString().c_str(), OLED_PIPE);
      }
    }
    else if(wifi_config.GetWifiMode() == false){
      Result = CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &ModeRun1);
      LOGLN("ModeRun:" + String(ModeRun1));
      if(wifi_b == false){WiFi.disconnect();WiFi.mode (WIFI_OFF);}
      else{
        WiFi.mode(WIFI_AP);
        OLED_DISPLAY::clear_lcd();
        OLED_DISPLAY::BigDisplay("WiFi ON", 15, 17);
        OLED_DISPLAY::setCursor(40, 48);
        ESPCOM::print(WiFi.softAPIP().toString().c_str(), OLED_PIPE);
      // Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, 2);//OFF WiFi AP Run Moto Mode
      // Result = CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &ModeRun1);
      // LOGLN("ModeRun:" + String(ModeRun1));
      //delay(3000);
      }
    }
    function = 0;
  } 
  else {
    // Reset function
    function = 0;
  }
    unsigned long currentMillis = millis();
    
    if ((currentMillis - previousMillis_DS1) >= 10000 && wifi_b == false) {
      Bat = VoltUpdate();
      for (int i = 0; i < oneWireCount; i++) {
        sensor[i].requestTemperatures();
      }
      //LOGLN("update");
      WiFi.disconnect();
      WiFi.mode (WIFI_OFF);
      if(GearPos == 0){GearPos = 10;OLED_DISPLAY::clear_lcd();}
    }
    
    if ((currentMillis - previousMillis_DS1) >= 11000 && wifi_b == false) {
      previousMillis_DS1 = currentMillis; 
      TemperatureEnegy = sensor[0].getTempCByIndex(0);
      Temperature = sensor[1].getTempCByIndex(0);
    }
    if ((currentMillis - previousMillis_2) >= 100 && wifi_b == false) {
      previousMillis_2 = currentMillis;count++; 
      if(SOS && count > 3){count = 0;digitalWrite(LEDButton, !digitalRead(LEDButton));}
      else{}
    #ifdef MOTO_DEBUG
    LOGLN("DONE");
    LOGLN("Gear: " + GearUpdate());
    LOGLN("Gear value : " + String(analogRead(GearN)) +'|'+ String(analogRead(Gear1)) +'|'+  String(analogRead(Gear2)) +'|'+ String(analogRead(Gear3)) +'|'+  String(analogRead(Gear4)) +'|'+  String(analogRead(Gear5)));
    #endif//MOTO_DEBUG
    if(Menu == 0){
      String Gear_en = GearUpdate();
      
      GetTime();
      String Month_,Day_,Hour_,Min_;
          if(Months < 10){Month_ = "0" + String(Months);}else{Month_ = String(Months);}
          if(Days < 10){Day_ = "0" + String(Days);}else{Day_ = String(Days);}
          if(hours < 10){Hour_ = "0" + String(hours);}else{Hour_ = String(hours);}
          if(mins < 10){Min_ = "0" + String(mins);}else{Min_ = String(mins);}
          // OLED_DISPLAY::setCursor(0, 30-12);
          // ESPCOM::print(Day_+"/"+ Month_+"/"+String(Years), OLED_PIPE);
          OLED_DISPLAY::DisplayText(Day_+"/"+ Month_+"/"+String(Years),55, 0, 35-11, 10, false);
          //OLED_DISPLAY::setCursor(0, 35);
          //ESPCOM::print("Out Side         | Engine", OLED_PIPE);
          OLED_DISPLAY::DisplayText("Out Side         Engine",23*5, 0, 35, 10, false);
          OLED_DISPLAY::GearDisplay(Gear_en, 65 + GearPos, 0);
          OLED_DISPLAY::TempDisplay(Temperature, 64, 48);
          OLED_DISPLAY::TempDisplay(TemperatureEnegy, 0, 48);
          //OLED_DISPLAY::BatDisplay(Bat, 74, 27-11);
          OLED_DISPLAY::TimeDisplay(Hour_, Min_, 0, 15-11);
      }
    if(Menu == 1){
        OLED_DISPLAY::BigDisplay("Setting", 15, 17);
        OLED_DISPLAY::setCursor(40, 48);
        ESPCOM::print("Display", OLED_PIPE);
    }
    if(Menu == 2){
        OLED_DISPLAY::BigDisplay("Setting", 15, 17);
        OLED_DISPLAY::setCursor(40, 48);
        ESPCOM::print("More", OLED_PIPE);
    }
    if(Menu == 3){
        OLED_DISPLAY::BigDisplay("Firmware", 15, 17);
        OLED_DISPLAY::setCursor(40, 48);
        ESPCOM::print("Check and update", OLED_PIPE);
    }
    }
}
String Moto::GearUpdate()
{ 
  String GearIs = "";
  if(analogRead(GearN) < 2000){GearIs = "N";}
  else if(analogRead(Gear1) < 2200){GearIs = "1";}
  else if(analogRead(Gear2) < 2000){GearIs = "2";}
  else if(analogRead(Gear3) < 2000){GearIs = "3";}
  else if(analogRead(Gear4) < 2200){GearIs = "4";}
  else if(analogRead(Gear5) < 2200){GearIs = "5";}
  else {GearIs = "_";}
  return GearIs;
}
float Moto::VoltUpdate() { 
  //adcStart(27);adcStart(14);adcStart(12);adcStart(13);
  // LOG("| ADC27:" + String(analogRead(27)));
  // LOG("| ADC14:" + String(analogRead(14)));
  // LOG("| ADC12:" + String(analogRead(12)));
  // LOGLN("| ADC13:" + String(analogRead(13)));
  int volt = analogRead(VoltSensorPin);// read the input
  float voltage = map(volt,0,4095, 0, 2500) + voltage_offset;// map 0-1023 to 0-2500 and add correction offset
  
  voltage /=100;// divide by 100 to get the decimal values
  return voltage;
}
void Moto::SetTime(uint8_t ss,uint8_t mm, uint8_t hh, uint8_t dd,uint8_t mo,int16_t year)
{
  GetTime();
  //if(mins != mm && hours != hh && Days != dd && Months != mo && Years != year){
    struct ts t;
        t.sec = ss;
        t.min = mm;
        t.hour = hh;
        t.mday = dd;
        t.mon = mo;
        t.year = year;
        DS3231_set(t);
        LOGLN("/////////////////////////////Set Time////////////////////////////");
        LOGLN("Time:" + String(t.hour)+':'+String(t.min));
        LOGLN("Date:" + String(t.mday)+"/"+String(t.mon)+"/"+String(t.year));
  //}
  Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, 1);//OFF WiFi Run Moto Mode
  Result = CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &ModeRun1);
  LOGLN("ModeRun:" + String(ModeRun1));
}
void Moto::GetTime()
{
  struct ts t;
  DS3231_get(&t);
  secs = t.sec;
  mins = t.min;
  hours = t.hour;
  Days = t.mday;
  Months = t.mon;
  Years = t.year;
}
void Moto::updateTime()
{
      struct tm  tmstruct;
    // time_t now;
    // time (&now);
    // localtime_r (&now, &tmstruct);
  
    //init and get the time
    String s1, s2, s3;
    int8_t t1;
    byte d1;
    if (!CONFIG::read_string (EP_TIME_SERVER1, s1, MAX_DATA_LENGTH) ) {
        s1 = FPSTR (DEFAULT_TIME_SERVER1);
    }
    if (!CONFIG::read_string (EP_TIME_SERVER2, s2, MAX_DATA_LENGTH) ) {
        s2 = FPSTR (DEFAULT_TIME_SERVER2);
    }
    if (!CONFIG::read_string (EP_TIME_SERVER3, s3, MAX_DATA_LENGTH) ) {
        s3 = FPSTR (DEFAULT_TIME_SERVER3);
    }
    if (!CONFIG::read_byte (EP_TIMEZONE, (byte *) &t1 ) ) {
        t1 = DEFAULT_TIME_ZONE;
    }
    if (!CONFIG::read_byte (EP_TIME_ISDST, &d1 ) ) {
        d1 = DEFAULT_TIME_DST;
    }
    configTime (3600 * (t1), d1 * 3600, s1.c_str(), s2.c_str(), s3.c_str() );
    time_t now = time(nullptr);
    if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA) {
        int nb = 0;
        while ((now < 8 * 3600 * 2) && (nb < 6)) {
            CONFIG::wait(500);
            nb++;
            now = time(nullptr);
        }
    }
    printLocalTime();
    if(!getLocalTime(&tmstruct)){
      Serial.println("Failed to obtain time");
      return;
    }  
    LOG("Time:" + String(tmstruct.tm_hour)+':'+String(tmstruct.tm_min) + '\n');
    LOG("Date:" + String(tmstruct.tm_mday)+"/"+String(tmstruct.tm_mon+1)+"/"+String((tmstruct.tm_year-100)+2000) + '\n');
   if(tmstruct.tm_hour-1 < 0 || tmstruct.tm_hour-1 > 23 || (tmstruct.tm_year-100)+2000 < 0){}
   else{SetTime(tmstruct.tm_sec,tmstruct.tm_min,tmstruct.tm_hour,tmstruct.tm_mday,tmstruct.tm_mon+1,(tmstruct.tm_year-100)+2000);}
   
}
#endif//Moto_UI