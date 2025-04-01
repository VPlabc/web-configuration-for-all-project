#ifndef RTC_
#define RTC_
#include <Arduino.h>
#include "config.h"
#include "WiFi.h"
#ifdef TIMESTAMP_FEATURE
#include <time.h>
#ifdef RTC_DS3231
#include <Wire.h>
#include <ds3231.h>
#endif//RTC_DS3231
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;


byte countTimeErrors = 0;
typedef struct{
  int hour;
  int min;
  int sec;
  int day;
  int month;
  int year;
  unsigned long epochTime;
}repondTime;
typedef struct{
  int hour;
  int min;
  int sec;
  int day;
  int month;
  int year;
  unsigned long epochTime;
}repondRTC;
repondRTC GetTime();
void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void RTC_Setup(){
#ifdef RTC_DS3231
    DS3231_init(DS3231_CONTROL_INTCN);
#endif//#ifdef RTC_DS3231
}
void SetTime(uint8_t ss,uint8_t mm, uint8_t hh, uint8_t dd,uint8_t mo,int16_t year,long epochT)
{

#ifdef RTC_DS3231
  // RTC_Setup();
  //if(mins != mm && hours != hh && Days != dd && Months != mo && Years != year){
    struct ts time_RTC;
        time_RTC.sec = ss;
        time_RTC.min = mm;
        time_RTC.hour = hh;
        time_RTC.mday = dd;
        time_RTC.mon = mo;
        time_RTC.year = year;
        time_RTC.unixtime = epochT;
        DS3231_set(time_RTC);
    // DS3231_set(ss,mm,hh,dd,mo,year);
        GetTime();
        LOGLN("/////////////////////////////Set Time////////////////////////////");
        LOGLN("Time:" + String(time_RTC.hour)+':'+String(time_RTC.min));
        LOGLN("Date:" + String(time_RTC.mday+1)+"/"+String(time_RTC.mon)+"/"+String(time_RTC.year));
  //}

#endif//#ifdef RTC_DS3231
}
repondRTC GetTime()
{
  // RTC_Setup();
  repondRTC RTCTime;
#ifdef RTC_DS3231
  struct ts Gettime_RTC;
  DS3231_get(&Gettime_RTC);
  RTCTime.sec = Gettime_RTC.sec;
  RTCTime.min = Gettime_RTC.min;
  RTCTime.hour = Gettime_RTC.hour;
  RTCTime.day = Gettime_RTC.mday;
  RTCTime.month = Gettime_RTC.mon;
  RTCTime.year = Gettime_RTC.year;
  RTCTime.epochTime = Gettime_RTC.unixtime;
#endif//#ifdef RTC_DS3231
return RTCTime;
}

repondTime updateTime()
{
    repondTime retTime;
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
    // LOGLN("Time zone: " + String(t1));
    configTime (3600 * (t1), d1 * 3600, s1.c_str(), s2.c_str(), s3.c_str() );
    time_t now = time(nullptr);
    if ((WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA) && WiFi.status() == WL_CONNECTED) {
        int nb = 0;
        while ((now < 8 * 3600 * 2) && (nb < 6)) {
            CONFIG::wait(500);
            nb++;
            now = time(nullptr);
        }
    }
    // printLocalTime();
    if(!getLocalTime(&tmstruct)){
      Serial.println("Failed to obtain time");countTimeErrors++;
      // return;
    }else {
        time_t thisnow;
        int day = tmstruct.tm_mday;
        if(countTimeErrors > 10){ESP.restart();} 
        if(tmstruct.tm_hour > 24){tmstruct.tm_hour = tmstruct.tm_hour - 231;}
        if(tmstruct.tm_hour > 24){tmstruct.tm_hour = tmstruct.tm_hour - 24;}
        if(day < 10){day = tmstruct.tm_mday;}else{day = tmstruct.tm_mday;}
        LOG("Time:" + String(tmstruct.tm_hour)+':'+String(tmstruct.tm_min) + '\n');
        LOG("Date:" + String(day)+"/"+String(tmstruct.tm_mon+1)+"/"+String((tmstruct.tm_year-100)+2000) + '\n');
        repondRTC RTCTime;RTC_Setup();delay(1000);RTCTime=GetTime();
        LOGLN("RTC:" + String(RTCTime.hour) + ":" + String(RTCTime.min));
        if(RTCTime.hour > 24 || RTCTime.hour != tmstruct.tm_hour){LOGLN("error RTC => " + String(RTCTime.hour));
        time(&thisnow);retTime.epochTime = thisnow;
        if(tmstruct.tm_hour-1 < 0 || tmstruct.tm_hour-1 > 23 || (tmstruct.tm_year-100)+2000 < 0){}
        else{SetTime(tmstruct.tm_sec,tmstruct.tm_min,tmstruct.tm_hour,day,tmstruct.tm_mon+1,(tmstruct.tm_year-100)+2000,thisnow);

        }
     
    // LOGLN("now: " + String(now));
    // LOGLN("thisnow: " + String(thisnow));
    #ifdef Moto_UI
        Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, 1);//OFF WiFi Run Moto Mode
        Result = CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &ModeRun1);
        LOGLN("ModeRun:" + String(ModeRun1));
    #endif//#ifdef Moto_UI
    }        
    retTime.day = day+1;retTime.month = tmstruct.tm_mon+1;retTime.year = (tmstruct.tm_year-100)+2000;
    retTime.hour = tmstruct.tm_hour;retTime.min = tmstruct.tm_min;retTime.sec = tmstruct.tm_sec;
   } 
   return retTime;
}

#endif//RTC
#endif//RTC_