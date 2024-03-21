
#include <ArduinoJson.h>
#include "config.h"
#include "SDFunction.h"
#include <SPI.h>
#include <SD.h>
#include "time.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
// #include "TimeLib.h"
#ifndef LOG_
#define LOG_
WiFiUDP LogntpUDP;
NTPClient timeClient(LogntpUDP);
byte bbufer;
// String sbuf;
char sbuf[MAX_DATA_LENGTH + 1];
byte hoursLog,minsLog,secsLog;
String dateLog,dayLog,monthLog,yearLog;
String RealTime = "20_03_24";

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
    time_t nows = timeClient.getEpochTime();
    timeClient.update();
    time_t Time = time(nullptr);
    struct tm  tmstruct;
    if(!getLocalTime(&tmstruct)){LOGLN("Failed to obtain time");return;}
    RealTime = String(tmstruct.tm_mday-1) + "_" + String(tmstruct.tm_mon+1) + "_" + String((tmstruct.tm_year-100)+2000);
    String Filename = "/" + NameFile + RealTime + ".csv";
    LOGLN("File name: " + Filename + "| " + RealTime + " | " + nows + "," + String(Data));

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
    //   LOG ("Message appended");
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