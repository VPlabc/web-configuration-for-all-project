
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
    DatalogRepondTime = CONFIG::init_time_client();
    unsigned long nows = DatalogRepondTime.epochTime;
    LOGLN("nows: " + String(nows));

    // time_t Time = time(nullptr);
    struct tm  tmstruct;
    if(!getLocalTime(&tmstruct)){LOGLN("Failed to obtain time");return;}
    String month = "";if(tmstruct.tm_mon+1 < 10){month = "0" + String(tmstruct.tm_mon+1);}
    if(tmstruct.tm_mday < 10){if(tmstruct.tm_mday < 10){RealTime = "0" + String(tmstruct.tm_mday) + "_" ;}else{RealTime = String(tmstruct.tm_mday) + "_" ;}}
    else{if(tmstruct.tm_mday-1 < 9){RealTime = "0" + String(tmstruct.tm_mday-1) + "_" ;}else{RealTime = String(tmstruct.tm_mday-1) + "_" ;}}
    RealTime +=  month + "_" + String((tmstruct.tm_year-100)+2000) ;
    String Filename = "/" + NameFile + RealTime + ".csv";
    LOGLN("File name: " + Filename + "| " + String(tmstruct.tm_hour) + ":" + String(tmstruct.tm_min) + ":" + String(tmstruct.tm_sec) + " | " + nows + "," + String(Data));

   if (!SD.begin(SDCard_CS)) {sd_card_found = false;} else {sd_card_found = true;LOGLN("SD Init ok");}
    //LOG ("saveMemoryToFile > SD Card found:" + String(SDFunc.sd_card_found) + '\n');
    //LOG ("Time:" + String(timeClient.getEpochTime()) + '\n');
    if (sd_card_found) {
        if(!SD.exists(Filename)){writeFile(SD, Filename, String(nows));LOGLN("File Created ");}
        else{appendFile(SD, Filename, String(nows));}
        appendFile(SD, Filename, ",");
        appendFile(SD, Filename, Data);
        appendFile(SD, Filename, "\n");
        //saveMemoryToFile();
    }
//  String(tmstruct.tm_hour)+':'+String(tmstruct.tm_min)
    if(DatalogRepondTime.min == 0){//day report
    unsigned long nows = DatalogRepondTime.epochTime;
    LOGLN("nows: " + String(nows));

      String Filename = "/" + NameFile + RealTime + "_day.csv";
    LOGLN("File name: " + Filename + "| " + String(DatalogRepondTime.hour) + ":" + String(DatalogRepondTime.min) + ":" + String(DatalogRepondTime.sec)  + " | " + nows + "," + String(Data));

    if (!SD.begin(SDCard_CS)) {sd_card_found = false;} else {sd_card_found = true;LOGLN("SD Init ok");}
      //LOG ("saveMemoryToFile > SD Card found:" + String(SDFunc.sd_card_found) + '\n');
      //LOG ("Time:" + String(timeClient.getEpochTime()) + '\n');
      if (sd_card_found) {
          if(!SD.exists(Filename)){writeFile(SD, Filename, String(nows));LOGLN("File Day Created");}
          else{appendFile(SD, Filename, String(nows));}
          appendFile(SD, Filename, Data);
          appendFile(SD, Filename, ",");
          //saveMemoryToFile();
      }
    }
}
long countData = 0;
long spaceData = 0;
String DLGreadFile(fs::FS &fs, String path, byte type, byte Inhour){
  // if (!SD.begin(SDCard_CS)) {sd_card_found = false;} else {sd_card_found = true;LOGLN("SD Init ok");}
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
  String PushData = "a|[{\"data\":\""+RowData + "\",\"time\":\"" + TimeIn + "\",\"Total\":" + String(countData) + "}]|";
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





// void digitalClockDisplay()
// {
//   // digital clock display of the time
// //   Serial.print(hour());
// //   printDigits(minute());
// //   printDigits(second());
//   Serial.print(" ");
//   Serial.print(day());
//   Serial.print(".");
//   Serial.print(month());
//   Serial.print(".");
//   Serial.print(year());
//   Serial.println();
// }

// String printDigits(int digits)
// {
// String digitsout = "";
//   if (digits < 10){digitsout = "0" + digits;}
//   else{digitsout = digits;}
//   return digitsout;
// }

// /*-------- NTP code ----------*/

// const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
// byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

// time_t getNtpTime()
// {
//   IPAddress ntpServerIP; // NTP server's ip address
//   while (LogntpUDP.parsePacket() > 0); // discard any previously received packets
//   Serial.println("Transmit NTP Request");
//   // get a random server from the pool
//   CONFIG::read_byte(EP_TIMEZONE ,&timez);
//   timeZone = timez;
// //   CONFIG::read_string(EP_TIME_SERVER1 , ntpServerName , MAX_DATA_LENGTH );
//   WiFi.hostByName(ntpServerName, ntpServerIP);
//   Serial.print(ntpServerName);
//   Serial.print(": ");
//   Serial.println(ntpServerIP);
//   sendNTPpacket(ntpServerIP);
//   uint32_t beginWait = millis();
//   while (millis() - beginWait < 1500) {
//     int size = LogntpUDP.parsePacket();
//     if (size >= NTP_PACKET_SIZE) {
//       Serial.println("Receive NTP Response");
//       LogntpUDP.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
//       unsigned long secsSince1900;
//       // convert four bytes starting at location 40 to a long integer
//       secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
//       secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
//       secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
//       secsSince1900 |= (unsigned long)packetBuffer[43];
//       return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
//     }
//   }
//   Serial.println("No NTP Response :-(");
//   return 0; // return 0 if unable to get the time
// }

// // send an NTP request to the time server at the given address
// void sendNTPpacket(IPAddress &address)
// {
//   // set all bytes in the buffer to 0
//   memset(packetBuffer, 0, NTP_PACKET_SIZE);
//   // Initialize values needed to form NTP request
//   // (see URL above for details on the packets)
//   packetBuffer[0] = 0b11100011;   // LI, Version, Mode
//   packetBuffer[1] = 0;     // Stratum, or type of clock
//   packetBuffer[2] = 6;     // Polling Interval
//   packetBuffer[3] = 0xEC;  // Peer Clock Precision
//   // 8 bytes of zero for Root Delay & Root Dispersion
//   packetBuffer[12] = 49;
//   packetBuffer[13] = 0x4E;
//   packetBuffer[14] = 49;
//   packetBuffer[15] = 52;
//   // all NTP fields have been given values, now
//   // you can send a packet requesting a timestamp:
//   LogntpUDP.beginPacket(address, 123); //NTP requests are to port 123
//   LogntpUDP.write(packetBuffer, NTP_PACKET_SIZE);
//   LogntpUDP.endPacket();
// }

#endif//LOG_