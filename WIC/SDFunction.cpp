#include "config.h"
// #ifdef Valve_UI
#include "SDFunction.h"
#include "WIC.h"
// #include "Valve/Valve.h"
// Valve valve;
#include <WiFiClient.h>
#include <WiFiServer.h>
#ifdef ARDUINO_ARCH_ESP8266
#if defined(ASYNCWEBSERVER)
#include <ESPAsyncTCP.h>
#else
#include <ESP8266WebServer.h>
#endif
#else //ESP32
#if defined(ASYNCWEBSERVER)
#include <AsyncTCP.h>
#else
#include <WebServer.h>
#endif
#endif
#ifdef ARDUINO_ARCH_ESP8266
    ESP8266WebServer web_server;
#else
    WebServer web_server;
#endif// ARDUINO_ARCH_ESP  


#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <WiFiClient.h>  
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <FS.h>
#include <LITTLEFS.h>
#include <PubSubClient.h>

#ifdef DEBUG_FLAG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(x) Serial.printf(x)
#else
#define debug(x)
#define debugln(x)
#define debugf(x)
#endif

// WiFiClient            wifiClient;
// WiFiUDP ntpUDP;
// NTPClient timeClient(ntpUDP);

typedef struct sensor_data {
  int     mesh_id;
  uint8_t sensor_id;
  byte    category;
  byte    status;
  byte    RSSI;
  bool    RSSIenable;
  float   temperature;
  float   humidity;
  float   battery;
  float   battery12;
  time_t  timestamp;
  time_t  Lasttimestamp;
  time_t cycleTime;
  uint32_t Nodecounter;
  uint32_t NodeTimeOut;
  byte     NodeComfirm;
  byte     GateWayCommand;
  byte     GateWayControl;
  byte     NodeRun;//0: idle,1:begin,2:end,3:come back

} sensor_data;
//struct_message  msgSD;
sensor_data     sensors[NUM_SENSORS];
int sensors_saved = 0;
//SDFunction::user_setting={};
//int     sensors_saved = 0;
//uint8_t Valve::incomingData[sizeof(msgSD)];
// size_t  Valve::received_msgSD_length;
// bool  Valve::new_sensor_found = false;


bool SDFunction::sd_card_found = true;


String SDserverIndex = 
"<!DOCTYPE html><html lang='en'><head><meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=no'/>"
 " <title>Update file</title>"
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<meta content=\"text/html;charset=utf-8\" http-equiv=\"Content-Type\">"
  "<meta content=\"utf-8\" http-equiv=\"encoding\">"
  "<link rel=\"stylesheet\" href=\"bootstrap.css\">"
  "<link rel=\"stylesheet\" href=\"main.css\">"
  "<script src=\"main.js\"></script>"
  "</head>"
  "<body>"
  "<div id=\"top_bar\" style='position:absolute;top:0px;height:5px;width:100%;background-color:#F7C642;'></div>"
  "<div class=\"container py-3\">"
  "<header>"
    "<div class=\"d-flex flex-column flex-md-row align-items-center pb-3 mb-4 border-bottom\">"
    "<a href=\"/\" class=\"d-flex align-items-center text-dark text-decoration-none\"><span class=\"fs-4 logo\"></span></a><span class=\"fs-4\"  onclick=\"loadSensors();return false;\">Update File</span><nav class=\"d-inline-flex mt-2 mt-md-0 ms-md-auto\"><span class=\"me-3 py-2 me-4 text-danger\" id=\"rota_status\"></span><span class=\"me-3 py-2 text-dark text-decoration-none d-none\"  style=\"cursor:pointer;\" onclick=\"recheckSDCard();return false;\" id=\"sdcard_status\"><svg viewBox=\"0 0 24 24\" style=\"width:16px;height:16px;color: #df1919;\"><path fill=\"currentColor\" d=\"M8,2A2,2 0 0,0 6,4V11L4,13V20A2,2 0 0,0 6,22H18A2,2 0 0,0 20,20V4A2,2 0 0,0 18,2H8M9,4H11V8H9V4M12,4H14V8H12V4M15,4H17V8H15V4Z\"></path></svg></span>"
    "<a class='me-3 py-2 text-dark text-decoration-none' href='/'><svg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' class='bi bi-house' viewBox='0 0 16 16'><path fill-rule='evenodd' d='M2 13.5V7h1v6.5a.5.5 0 0 0 .5.5h9a.5.5 0 0 0 .5-.5V7h1v6.5a1.5 1.5 0 0 1-1.5 1.5h-9A1.5 1.5 0 0 1 2 13.5zm11-11V6l-2-2V2.5a.5.5 0 0 1 .5-.5h1a.5.5 0 0 1 .5.5z'/><path fill-rule='evenodd' d='M7.293 1.5a1 1 0 0 1 1.414 0l6.647 6.646a.5.5 0 0 1-.708.708L8 2.207 1.354 8.854a.5.5 0 1 1-.708-.708L7.293 1.5z'/></svg></a>"
   "   </nav>"
     "  </div>"
 " </header>  "
 "<body>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Upload'>"
"</form>"
"</br><div id='prg'>progress: 0%</div>"
"</body>"
"<script>"
"$('form').submit(function(e){"
    "e.preventDefault();"
      "var form = $('#upload_form')[0];"
      "var data = new FormData(form);"
      " $.ajax({"
            "url: '/SDupdate',"
            "type: 'POST',"               
            "data: data,"
            "contentType: false,"                  
            "processData:false,"  
            "xhr: function() {"
                "var xhr = new window.XMLHttpRequest();"
                "xhr.upload.addEventListener('progress', function(evt) {"
                    "if (evt.lengthComputable) {"
                        "var per = evt.loaded / evt.total;"
                        "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
                    "}"
               "}, false);"
               "return xhr;"
            "},"                                
            "success:function(d, s) {"    
                "console.log('success!')"
           "},"
            "error: function (a, b, c) {"
            "}"
          "});"
"});"
"</script>";


SDFunction::SDFunction()
{

}
uint64_t SDFunction::GetTotalSize(){
   uint64_t cardSize = SD.cardSize() / (1024 * 1024);
}
void SDFunction::Setup(){
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
}
String SDFunction::printDirectory(File dir, int numTabs) {
  String response = SDserverIndex + "</br><div class='row mb-0' style='font-size:0.8rem;'>";
  dir.rewindDirectory();
  
  while(true) {
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //LOG ("**nomorefiles**\n");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       LOG ('\t');   // we'll have a nice indentation
     }
     // Recurse for directories, otherwise print the file size
     if (entry.isDirectory()) {
       SDFunction::printDirectory(entry, numTabs+1);
     } else {
       String Xbutton = "<div class='col-10 text-left'><a style='color:black;font-size:16px;text-decoration: none;' href='/deleteF?id="+String(entry.name())+"'> <svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\" fill=\"currentColor\" class=\"bi bi-trash3-fill\" viewBox=\"0 0 16 16\"><path d=\"M11 1.5v1h3.5a.5.5 0 0 1 0 1h-.538l-.853 10.66A2 2 0 0 1 11.115 16h-6.23a2 2 0 0 1-1.994-1.84L2.038 3.5H1.5a.5.5 0 0 1 0-1H5v-1A1.5 1.5 0 0 1 6.5 0h3A1.5 1.5 0 0 1 11 1.5Zm-5 0v1h4v-1a.5.5 0 0 0-.5-.5h-3a.5.5 0 0 0-.5.5ZM4.5 5.029l.5 8.5a.5.5 0 1 0 .998-.06l-.5-8.5a.5.5 0 1 0-.998.06Zm6.53-.528a.5.5 0 0 0-.528.47l-.5 8.5a.5.5 0 0 0 .998.058l.5-8.5a.5.5 0 0 0-.47-.528ZM8 4.5a.5.5 0 0 0-.5.5v8.5a.5.5 0 0 0 1 0V5a.5.5 0 0 0-.5-.5Z\"/></svg></a></div>";
       response += String("<div class='col-2 text-left'>" + String(entry.name()) + "</div>")  ;
       response += Xbutton + "</br></html> ";
     }
     entry.close();
   }
   return  response + String("</br></br>") ;
}

bool SDFunction::loadFromSDCARD(String path){
  path.toLowerCase();
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".txt")) dataType = "text/plain";
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".js")) dataType = "text/js";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".log")) dataType = "text/log";
  else if(path.endsWith(".bin")) dataType = "text/bin";
  else if(path.endsWith(".zip")) dataType = "application/zip";  
  //LOG (dataType);
  File dataFile = SD.open(path.c_str());

  if (!dataFile)
    return false;

  if (web_server.streamFile(dataFile, dataType) != dataFile.size()) {
    LOG ("Sent less data than expected!\n");
  }

  dataFile.close();
  return true;
}

void SDFunction::handleRawFile() {

  /*  FORMAT: timestamp, category, status, temperature, humidty, battery  */
  if ( SDFunction::sd_card_found == false ) web_server.send ( 404, "text/html",  "No SD card found." );

  if ( SD.exists("/data/" + web_server.arg("id")) ) {
    File sensor = SD.open("/data/" + web_server.arg("id"));
    int fsize = sensor.size();
    web_server.sendHeader("Content-Length", (String)(fsize));
    size_t fsizeSent = web_server.streamFile(sensor, "text/plain");
    sensor.close();
  }  else  web_server.send ( 404, "text/html",  "Invalid request." );

}

void SDFunction::handleNames() {

  if ( SDFunction::sd_card_found == false ) web_server.send ( 200, "text/html",  "" );

  if ( SD.exists("/config.txt") ) {
    File sensor = SD.open("/config.txt");
    int fsize = sensor.size();
    web_server.sendHeader("Content-Length", (String)(fsize));
    size_t fsizeSent = web_server.streamFile(sensor, "text/plain");
    sensor.close();
  }  else  web_server.send ( 200, "text/html",  "" );
}

void SDFunction::handleDeleteFile() {

  if ( SD.exists("/" + web_server.arg("id")) ) {
    SD.remove("/" + web_server.arg("id"));
  }
}
void SDFunction::handleDeleteSensor() {

  if ( SD.exists("/data/" + web_server.arg("id")) ) {
    SD.remove("/data/" + web_server.arg("id"));
    web_server.send(200, "text/plain", "1");
  } else {
    web_server.send(200, "text/plain", "0");
  }
}



void SDFunction::handleRetrySD() {

  if (SDFunction::sd_card_found == true) {
    web_server.send(200, "text/plain", "1");
    return;
  } else {

    if (!SD.begin(SDCard_CS)) {
      SDFunction::sd_card_found = false;
      //valve.showInfo("SD Card", "error: not found", 5);
      web_server.send(200, "text/plain", "0");
    } else {
      //valve.showInfo("SD Card", "loading sensors", 3);
      web_server.send(200, "text/plain", "1");
      SDFunction::sd_card_found = true;
      //valve.readMemoryFromFile();
    }
  }
}

void SDFunction::saveMemoryToFile() {

//LOG ("saveMemoryToFile: " + String(SDFunction::sd_card_found) +"\n");
  if (SDFunction::sd_card_found == true) {
    if (SD.exists("/memory.bin")) {SD.remove("/memory.bin");}
    File file = SD.open("/memory.bin", FILE_WRITE);
    if (file) {
      file.write(sensors_saved);
      for (int i = 0; i < sensors_saved; i++) file.write((uint8_t *)&sensors[i], sizeof(struct sensor_data));
      file.close();
    }
  }
}

void SDFunction::readMemoryFromFile() {
//LOG ("readMemoryFromFile: " + String(SDFunction::sd_card_found) + "\n");
  if (SDFunction::sd_card_found == true) {
    if (SD.exists("/memory.bin")) {
      File file = SD.open("/memory.bin", FILE_READ);
      sensors_saved = file.read();//LOG (String(sensors_saved) + " Device Exists\n");
      for (int i = 0; i <sensors_saved; i++) file.read((uint8_t *)&sensors[i], sizeof(struct sensor_data));
      file.close();
    }
    else{
    LOG ("File not Exists\n");
    }
  }
}

String SDFunction::JsonRaw()
{
    String raw;
  for (int i = 0; i < sensors_saved; i++) {
    raw += String(sensors[i].sensor_id) + ",";
    raw += String(sensors[i].category) + ",";
    raw += String(sensors[i].status) + ",";
    raw += String(sensors[i].temperature) + ",";
    raw += String(sensors[i].humidity) + ",";
    raw += "°C,";
    raw += String(sensors[i].battery ) + ",";
    raw += String(sensors[i].battery12 ) + ",";
    raw += String(sensors[i].timestamp) + ",";
    raw += String(sensors[i].RSSI);
    if ( i < sensors_saved - 1) raw += "\n";
  }
  return raw;
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void createDir(fs::FS &fs, const char * path);
void removeDir(fs::FS &fs, const char * path);
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void deleteFile(fs::FS &fs, const char * path);
void testFileIO(fs::FS &fs, const char * path);

void SDFunction::TestSDCard()
{
  if(!SD.begin(SDCard_CS)){
    LOG ("Card Mount Failed\n");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    LOG ("No SD card attached\n");
    return;
  }

  LOG ("SD Card Type: ");
  if(cardType == CARD_MMC){
    LOG ("MMC\n");
  } else if(cardType == CARD_SD){
    LOG ("SDSC\n");
  } else if(cardType == CARD_SDHC){
    LOG ("SDHC\n");
  } else {
    LOG ("UNKNOWN\n");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  listDir(SD, "/", 0);
  createDir(SD, "/mydir");
  listDir(SD, "/", 0);
  removeDir(SD, "/mydir");
  listDir(SD, "/", 1);
  writeFile(SD, "/hello.txt", "Hello ");
  appendFile(SD, "/hello.txt", "World!\n");
  readFile(SD, "/hello.txt");
  deleteFile(SD, "/foo.txt");
  renameFile(SD, "/hello.txt", "/foo.txt");
  readFile(SD, "/foo.txt");
  testFileIO(SD, "/test.txt");
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    LOG ("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    LOG ("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      LOG ("  DIR : ");
      LOG (file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      LOG (file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    LOG ("Dir created");
  } else {
    LOG ("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path){
  Serial.printf("Removing Dir: %s\n", path);
  if(fs.rmdir(path)){
    LOG ("Dir removed");
  } else {
    LOG ("rmdir failed");
  }
}

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

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    LOG ("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    LOG ("File written");
  } else {
    LOG ("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    LOG ("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      LOG ("Message appended");
  } else {
    LOG ("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    LOG ("File renamed");
  } else {
    LOG ("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    LOG ("File deleted");
  } else {
    LOG ("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char * path){
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file){
    len = file.size();
    size_t flen = len;
    start = millis();
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    LOG ("Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if(!file){
    LOG ("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++){
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}
// #endif//Valve_UI