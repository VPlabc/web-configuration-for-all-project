
#include <ArduinoJson.h>
#include "config.h"
#include "SDFunction.h"
#include <SPI.h>
#include <SD.h>
// #include "time.h"
#include "TimeLib.h"
// #include <WiFiUdp.h>
// #include <NTPClient.h>
// #include "TimeLib.h"
#ifndef LOG_
#define LOG_
// WiFiUDP LogntpUDP_dtl;
// NTPClient timeClient_dtl(LogntpUDP_dtl);
// #include "RealTimeClock.h"
// repondTime DatalogRepondTime;

byte bbufer;
// String sbuf;
char sbuf[MAX_DATA_LENGTH + 1];
byte hoursLog,minsLog,secsLog;
String dateLog,dayLog,monthLog,yearLog;
String RealTime = "20_03_24";
enum{Day,Week,Month,Year};
String ListData[30];
void DLGreadFile(fs::FS &fs, String path, byte type);
void writeFile(fs::FS &fs, String path, String message);
void appendFile(fs::FS &fs, String path, String message);

String uptime() {
  int duration = time(nullptr);
  if (duration >= 86400) return String(duration / 86400) + " days";
  if (duration >= 3600)  return String(duration / 3600) + + " hoursLog";
  if (duration >= 60)    return String(duration / 60) + " minutes";
  return String(duration) + " seconds";
}


bool sd_card_found = false;
void SaveData(String NameFile, String Data){
    CFrepondTime DatalogRepondTime;
    unsigned long nows = 0;
    // struct tm  tmstruct;
    if(WiFi.status() == WL_CONNECTED){
    DatalogRepondTime = CONFIG::Get_Time(); nows = DatalogRepondTime.epochTime;
    }
    else{
      DatalogRepondTime =  CONFIG::Get_Time();nows = DatalogRepondTime.epochTime;}
    // LOGLN("nows: " + String(nows));

    // time_t Time = time(nullptr);
    // if(!getLocalTime(&tmstruct)){LOGLN("Failed to obtain time");return;}
    String month = "";if(DatalogRepondTime.month < 10){month = "0" + String(DatalogRepondTime.month);}
    String year = "";
    if(DatalogRepondTime.year < 1970){year = String((DatalogRepondTime.year-100)+2000);}else{year = DatalogRepondTime.year;}
    // if(DatalogRepondTime.day < 10){
      if(DatalogRepondTime.day < 10){RealTime = "0" + String(DatalogRepondTime.day-1) + "_" ;}else{RealTime = String(DatalogRepondTime.day-1) + "_" ;}
      // }
    // else{if(DatalogRepondTime.day-1 < 9){RealTime = "0" + String(DatalogRepondTime.day-1) + "_" ;}else{RealTime = String(DatalogRepondTime.day-1) + "_" ;}}
    RealTime +=  month + "_" + year ;
    String Filename = "/" + NameFile + RealTime + ".csv";
    LOGLN("File name: " + Filename + "| " + String(DatalogRepondTime.hour) + ":" + String(DatalogRepondTime.min) + ":" + String(DatalogRepondTime.sec) + " | " + nows + "," + String(Data));

   if (!SD.begin(SDCard_CS)) {sd_card_found = false;} else {sd_card_found = true;}
    //LOG ("saveMemoryToFile > SD Card found:" + String(SDFunc.sd_card_found) + '\n');
    //LOG ("Time:" + String(timeClient.getEpochTime()) + '\n');
    if (sd_card_found) {
          String content = String(nows) + "," + Data + ",\n" ;
        if(!SD.exists(Filename)){writeFile(SD, Filename, String(content));LOGLN("File Created ");}
        else{appendFile(SD, Filename, String(content));}
        //saveMemoryToFile();
    }
//  String(DatalogRepondTime.tm_hour)+':'+String(DatalogRepondTime.tm_min)
    if(DatalogRepondTime.min == 0){//day report
    unsigned long nows = DatalogRepondTime.epochTime;
    // LOGLN("nows: " + String(nows));

      String Filename = "/" + NameFile + RealTime + "_day.csv";
    LOGLN("File name: " + Filename + "| " + String(DatalogRepondTime.hour) + ":" + String(DatalogRepondTime.min) + ":" + String(DatalogRepondTime.sec)  + " | " + nows + "," + String(Data));

    if (!SD.begin(SDCard_CS)) {sd_card_found = false;} else {sd_card_found = true;}
      //LOG ("saveMemoryToFile > SD Card found:" + String(SDFunc.sd_card_found) + '\n');
      //LOG ("Time:" + String(timeClient.getEpochTime()) + '\n');
      if (sd_card_found) {
          String content = String(nows) + "," + Data + ",\n" ;
          if(!SD.exists(Filename)){writeFile(SD, Filename, String(content));LOGLN("File Day Created");}
          else{appendFile(SD, Filename, String(content));}
          //saveMemoryToFile();
      }
    }
    LOGLN("Save Datalog done");
}
long countData = 0;
long spaceData = 0;
String DLGreadFile(fs::FS &fs, String path, byte type, byte Inhour){
  if (!SD.begin(SDCard_CS)) {sd_card_found = false;} else {sd_card_found = true;}
  Serial.println("Reading file: " + path + " | houre: " + Inhour);
  File file = fs.open(path);
  if(!file){LOG ("Failed to open file for reading");return "{\"data\":\"Failed to open file for reading\"}";}
  Serial.println("Read from file: ");
  String ReadIn ="";
  String TimeIn ="";
  String RowData ="";
  char charIn = 0;
  // int Mins = 0;
  int Hours = 0;
  while(file.available()){
    charIn = (char)file.read();
    if(charIn == '\n' || charIn == ','){
      unsigned long t_unix_date1;
      if(ReadIn.length() > 8 && type == 0){t_unix_date1 = ReadIn.toInt(); 
        if(Inhour == hour(t_unix_date1)){
          TimeIn += ReadIn + ",";countData++;
          // printf("Date1: %4d-%02d-%02d %02d:%02d:%02d\n", year(t_unix_date1), month(t_unix_date1), day(t_unix_date1), hour(t_unix_date1), minute(t_unix_date1), second(t_unix_date1));
        }
        if(Inhour == 24){
          TimeIn += ReadIn + ",";countData++;
          // printf("Date1: %4d-%02d-%02d %02d:%02d:%02d\n", year(t_unix_date1), month(t_unix_date1), day(t_unix_date1), hour(t_unix_date1), minute(t_unix_date1), second(t_unix_date1));
        }
        ReadIn = "";
      }
      else if(ReadIn.length() > 8 && type == 1){TimeIn += ReadIn + ",";countData++;ReadIn = "";}
      else if(ReadIn.length() > 1 && ReadIn.length() <= 8 && type == 1){ RowData+=  ReadIn + ",";
      ReadIn = "";}
      else if(ReadIn.length() > 1 && ReadIn.length() <= 8 && type == 0){ 
        if(Inhour == hour(t_unix_date1)){
          if(spaceData > 3){spaceData = 0;Hours = hour(t_unix_date1);}RowData+=  ReadIn + ",";spaceData++;
        }
        if(Inhour == 24){
          if(spaceData > 3){spaceData = 0;Hours = hour(t_unix_date1);}RowData+=  ReadIn + ",";spaceData++;
        }
        ReadIn = "";
      }
    }
    else{ReadIn += charIn;}
  }//lay a| lam ki tu phan biet
  LOGLN("Read done");
  String PushData = "a|[{\"data\":\""+RowData + "\",\"time\":\"" + TimeIn + "\",\"Total\":" + String(countData) + "}]|";
  LOGLN("Total:" + String(countData));
  file.close();countData = 0;
  return PushData;
}

void writeFile(fs::FS &fs, String path, String message){
//   Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    LOG ("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    // LOG ("File written");
  } else {
    LOG ("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, String path, String message){
//   Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    LOG ("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
    // LOG ("data appended");
  } else {
    LOG ("Append failed");
  }
  file.close();
}

#endif//LOG_