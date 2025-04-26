/*
  webinterface.cpp - ESP3D configuration class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <pgmspace.h>
#include "config.h"
#include "webinterface.h"
#include "wificonf.h"
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <StreamString.h>
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#include <FS.h>
#if defined (ASYNCWEBSERVER)
#include <ESPAsyncWebServer.h>
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#if defined (ASYNCWEBSERVER)
#include <ESPAsyncTCP.h>
#else
#include <ESP8266WebServer.h>
#endif
#else //ESP32
#include <WiFi.h>
#if defined (ASYNCWEBSERVER)
#include <AsyncTCP.h>
#else
#include <WebServer.h>
#endif
#include "SPIFFS.h"
#include "Update.h"
#endif

#include "GenLinkedList.h"
#include "command.h"
#include "espcom.h"

#ifdef SSDP_FEATURE
#ifdef ARDUINO_ARCH_ESP32
#include <ESP32SSDP.h>
#else
#include <ESP8266SSDP.h>
#endif
#endif

#if defined(ASYNCWEBSERVER)
#include "asyncwebserver.h"
#else
#include "syncwebserver.h"
#endif
#ifdef Gyro_UI
#include "GyroDatalog/Gyro_Datalog.h"
GyroDatalog Gyro;
#include <FS.h>
#include <SDFS.h>
#endif//Gyro_UI
#ifdef Valve_UI
#include "Valve/Valve.h"
Valve webValve;
#endif//Valve_UI
#ifdef LOOKLINE_UI
#include "lookline/LookLine.h"
#endif//LOOOKLINE_UI
#include "WIC.h"
WIC Wc;
#ifdef SDCARD_FEATURE
#include <SPI.h>
#include <SD.h>
#endif//SDCARD_FEATURE

#define MAX_AUTH_IP 10

long id_connection = 0;

#ifndef USE_AS_UPDATER_ONLY

uint8_t Checksum(const char * line, uint16_t lineSize)
{
    uint8_t checksum_val =0;
    for (uint16_t i=0; i < lineSize; i++) {
        checksum_val = checksum_val ^ ((uint8_t)line[i]);
    }
    return checksum_val;
}

String CheckSumLine(const char* line, uint32_t linenb)
{
    String linechecksum = "N" + String(linenb)+ " " + line;
    uint8_t crc = Checksum(linechecksum.c_str(), linechecksum.length());
    linechecksum+="*"+String(crc);
    return linechecksum;
}

bool purge_serial()
{
    uint32_t start = millis();
    uint8_t buf [51];
    ESPCOM::flush (DEFAULT_PRINTER_PIPE);
    CONFIG::wait (5);
    log_esp3d("Purge Serial");
    while (ESPCOM::available(DEFAULT_PRINTER_PIPE) > 0 ) {
        if ((millis() - start ) > 2000) {
            log_esp3d("Purge timeout");
            return false;
        }
        size_t len = ESPCOM::readBytes (DEFAULT_PRINTER_PIPE, buf, 50);
        buf[len] = '\0';
        if ( ( CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER) ) {
            String s = (const char *)buf;
            //repetier never stop sending data so no need to wait if have 'wait' or 'busy'
            if((s.indexOf ("wait") > -1) || (s.indexOf ("busy") > -1)) {
                return true;
            }
            log_esp3d("Purge interrupted");
        }
        CONFIG::wait (5);
    }
    CONFIG::wait (0);
    log_esp3d("Purge done");
    return true;
}

size_t wait_for_data(uint32_t timeout)
{
    uint32_t start = millis();
    while ((ESPCOM::available(DEFAULT_PRINTER_PIPE) < 2) && ((millis()-start) < timeout)) {
        CONFIG::wait (10);
    }
    return ESPCOM::available(DEFAULT_PRINTER_PIPE);
}

uint32_t Get_lineNumber(String & response)
{
    int32_t l = 0;
    String sresend = "Resend:";
    if ( CONFIG::GetFirmwareTarget() == SMOOTHIEWARE) {
        sresend = "rs N";
    }
    int pos = response.indexOf(sresend);
    if (pos == -1 ) {
        return -1;
    }
    pos+=sresend.length();
    int pos2 = response.indexOf("\n", pos);
    String snum = response.substring(pos, pos2);
    //remove potential unwished char
    snum.replace("\r", "");
    l = snum.toInt();
    log_esp3d("Line requested is %d", l);
    return l;
}

//function to send line to serial///////////////////////////////////////
//if newlinenb is NULL no auto correction of line number in case of resend
bool sendLine2Serial (String &  line, int32_t linenb,  int32_t * newlinenb)
{
    log_esp3d ("Send line %d");
    String line2send;
    String sok = "ok";
    String sresend = "Resend:";
    if (newlinenb) {
        *newlinenb = linenb;
    }
    if ( CONFIG::GetFirmwareTarget() == SMOOTHIEWARE) {
        sresend = "rs N";
    }
#ifdef DISABLE_SERIAL_CHECKSUM
    linenb = -1;
#endif
    if (linenb != -1) {
        if ( ( CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER) ) {
            sok+=" " + String(linenb);
        }
        line2send = CheckSumLine(line.c_str(),linenb);
    } else {
        line2send = line;
    }
    //purge serial as nothing is supposed to interfere with upload
    purge_serial();
    //send line
    ESPCOM::println (line2send, DEFAULT_PRINTER_PIPE);
    ESPCOM::flush(DEFAULT_PRINTER_PIPE);
    //check answer
    if (wait_for_data(2000) > 0 ) {
        bool done = false;
        uint32_t timeout = millis();
        uint8_t count = 0;
        //got data check content
        String response ;
        while (!done) {
            size_t len = ESPCOM::available(DEFAULT_PRINTER_PIPE);
            //get size of buffer
            if (len > 0) {
                uint8_t * sbuf = (uint8_t *)malloc(len+1);
                if(!sbuf) {
                    return false;
                }
                //read buffer
                ESPCOM::readBytes (DEFAULT_PRINTER_PIPE, sbuf, len);
                //convert buffer to zero end array
                sbuf[len] = '\0';
                //use string because easier to handle and allow to re-assemble cutted answer
                response += (const char*) sbuf;
                //in that case there is no way to know what is the right number to use and so send should be failed
                if (( ( CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER) ) && (response.indexOf ("skip") != -1)) {
                    log_esp3d ("Wrong line requested");
                    count = 5;
                }
                //it is resend ?
                int pos = response.indexOf (sresend);
                //be sure we get full line to be able to process properly
                if (( pos > -1) && (response.lastIndexOf("\n") > pos)) {
                    log_esp3d ("Resend detected");
                    uint32_t line_number = Get_lineNumber(response);
                    //this part is only if have newlinenb variable
                    if (newlinenb != nullptr) {
                        *newlinenb = line_number;
                        free(sbuf);
                        //no need newlinenb in this one in theory, but just in case...
                        return sendLine2Serial (line, line_number, newlinenb);
                    } else {
                        //the line requested is not the current one so we stop
                        if (line_number !=(uint32_t)linenb) {
                            log_esp3d ("Wrong line requested");
                            count = 5;
                        }
                    }
                    count++;
                    if (count > 5) {
                        free(sbuf);
                        log_esp3d ("Exit too many resend or wrong line");
                        return false;
                    }
                    purge_serial();
                    log_esp3d ("Resend x%d", count);
                    response="";
                    ESPCOM::println (line2send, DEFAULT_PRINTER_PIPE);
                    ESPCOM::flush (DEFAULT_PRINTER_PIPE);
                    wait_for_data(1000);
                    timeout = millis();

                } else {
                    if ( (response.indexOf (sok) > -1) ) { //we have ok so it is done
                        free(sbuf);
                        log_esp3d ("Got ok");
                        purge_serial();
                        return true;
                    }
                }
                free(sbuf);
            }
            //no answer or over buffer  exit
            if ( (millis() - timeout > 2000) ||  (response.length() >200)) {
                log_esp3d("Time out");
                done = true;
            }
            CONFIG::wait (5);
        }
    }
    log_esp3d ("Send line error");
    return false;
}

//send M29 / M30 command to close file on SD////////////////////////////
void CloseSerialUpload (bool iserror, String & filename, int32_t linenb)
{
    purge_serial();
    String command = "M29";
    purge_serial();
    if (!sendLine2Serial (command,linenb, &linenb)) {
        wait_for_data(2000);
        if (!sendLine2Serial (command,linenb, &linenb)) {
            if ((CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER) ) {
                sendLine2Serial (command,-1, NULL);
            }
        }
    }
    if (iserror) {
        String cmdfilename = "M30 " + filename;
        sendLine2Serial (cmdfilename,-1, NULL);
        ESPCOM::println (F ("SD upload failed"), PRINTER_PIPE);
        ESPCOM::flush (DEFAULT_PRINTER_PIPE);
        web_interface->_upload_status = UPLOAD_STATUS_FAILED;
    }  else {
        ESPCOM::println (F ("SD upload done"), PRINTER_PIPE);
        ESPCOM::flush (DEFAULT_PRINTER_PIPE);
        web_interface->_upload_status = UPLOAD_STATUS_SUCCESSFUL;
    }
    //lets give time to FW to proceed
    wait_for_data(2000);
    purge_serial();
    web_interface->blockserial = false;
}
#endif //USE_AS_UPDATER_ONLY

#ifdef SDCARD_FEATURE
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Download(){ // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
// link  /SDdown?file=
  if (web_interface->web_server.hasArg("file") > 0 ) { // Arguments were received
    String Filename = String(web_interface->web_server.arg("file"));
        #ifdef ESP8266
        if(CONFIG::GyroStates == false){
            if (!SD.begin(SDCard_CS, SPI_FULL_SPEED)) {Serial.print("SD Card not found");}
        }
        File download = SD.open(Filename);
        #else
        if (!SD.begin(SDCard_CS)) {Serial.print("SD Card not found");}
        File download = SD.open("/"+Filename);
        #endif//ESP8266
    if (download) {
      Serial.println("File name:" + Filename);
      web_interface->web_server.sendHeader("Content-Type", "text/text");
      web_interface->web_server.sendHeader("Content-Disposition", "attachment; filename="+Filename);
      web_interface->web_server.sendHeader("Connection", "close");
      web_interface->web_server.streamFile(download, "application/octet-stream");
      download.close();
    } 
  }
  //else SelectInput("File Download","Enter filename to download","download","download");
}
#endif//SDCARD_FEATURE
#ifdef Gyro_UI
void Gyro_State(){
if (web_interface->web_server.hasArg("State") > 0 ) { // Arguments were received
    String State = String(web_interface->web_server.arg("State"));
    Serial.println("State:" + State);
    if(State == "Run"){
        if (!SD.begin(SDCard_CS, SPI_FULL_SPEED)) {Serial.print("SD Card not found");}
        else {CONFIG::write_byte(GyroState,1);Gyro.Start();}
    }
    if(State == "Stop"){CONFIG::write_byte(GyroState,0);digitalWrite(D4,LOW);}
    web_interface->web_server.send(200,"text/html",State);
  }
}
#endif//Gyro_UI
//constructor
WEBINTERFACE_CLASS::WEBINTERFACE_CLASS (int port) : web_server (port)
#if defined(ASYNCWEBSERVER)
    , web_events("/events")
#ifdef WS_DATA_FEATURE
    , web_socket("/ws")
#endif
#endif
{
    //that handle "/" and default index.html.gz
#if defined(ASYNCWEBSERVER)
    //trick to catch command line on "/" before file being processed
    web_server.serveStatic ("/", SPIFFS, "/").setDefaultFile ("index.html").setFilter (filterOnRoot);
    web_server.serveStatic ("/", SPIFFS, "/Nowhere");
    //events functions
    web_events.onConnect(handle_onevent_connect);
    //events management
    web_server.addHandler(&web_events);
#ifdef WS_DATA_FEATURE
    //Websocket function
    web_socket.onEvent(handle_Websocket_Event);
    //Websocket management
    web_server.addHandler(&web_socket);
#endif
#else
    web_server.on("/",HTTP_ANY, handle_web_interface_root);
    web_server.on("/tool",HTTP_ANY, handle_web_interface_tool);
#endif
    //need to be there even no authentication to say to UI no authentication
    web_server.on("/login", HTTP_ANY, handle_login);
#ifdef SSDP_FEATURE
    web_server.on ("/description.xml", HTTP_GET, handle_SSDP);
#endif
#ifdef CAPTIVE_PORTAL_FEATURE
#if defined(ASYNCWEBSERVER)
    web_server.on ("/generate_204", HTTP_ANY,  [] (AsyncWebServerRequest * request) {
        request->redirect ("/");
    });
    web_server.on ("/gconnectivitycheck.gstatic.com", HTTP_ANY,  [] (AsyncWebServerRequest * request) {
        request->redirect ("/");
    });
    //do not forget the / at the end
    web_server.on ("/fwlink/", HTTP_ANY,  [] (AsyncWebServerRequest * request) {
        request->redirect ("/");
    });
#else
    web_server.on("/generate_204",HTTP_ANY, handle_web_interface_root);
    web_server.on("/gconnectivitycheck.gstatic.com",HTTP_ANY, handle_web_interface_root);
    //do not forget the / at the end
    web_server.on("/fwlink/",HTTP_ANY, handle_web_interface_root);
#endif
#endif
    //SPIFFS
    web_server.on ("/files", HTTP_ANY, handleFileList, SPIFFSFileupload);
    
#ifdef SDCARD_FEATURE
    //SD
    web_server.on ("/SDfiles", HTTP_ANY, handleSDCardFileList, SDCardFileupload);
    web_server.on("/SDdown",  File_Download);
#endif//SDCARD
#ifdef Gyro_UI
    web_server.on("/Gyro",  Gyro_State);
#endif//Gyro_UI
#ifdef WEB_UPDATE_FEATURE
    web_server.on ("/updatefw", HTTP_ANY, handleUpdate, WebUpdateUpload);
#endif
    //Page not found handler
    web_server.onNotFound ( handle_not_found);
    //web commands
    web_server.on ("/command", HTTP_ANY, handle_web_command);
    web_server.on ("/command_silent", HTTP_ANY, handle_web_command_silent);
    //Serial SD management
    web_server.on ("/upload_serial", HTTP_ANY, handle_serial_SDFileList, SDFile_serial_upload);

//   //web_interface->web_server.on("/",  handleRoot);
//   //web_interface->web_server.on("/config", handleConfig);               
#ifdef Valve_UI  
   web_server.on("/raw", handleRaw);
   web_server.on("/status", handleStatus);
   web_server.on("/mqtt", handleMqtt);
   web_server.on("/sensor", handleRawFile);
   web_server.on("/names", handleNames);
   web_server.on("/json", handleJson);
   web_server.on("/info", handleInfo);
#endif//Valve_UI                    
#ifdef LOOKLINE_UI
//    web_server.on("/raw", handleLooklineRaw);
   
#endif//LOOKLINE_UI
//   web_server.on("/delete", handleDeleteSensor);
//   web_server.on("/deleteF", handleDeleteFile);
//   web_server.on("/rota_enable", handleROTA);
//   web_server.on("/retry_sd", handleRetrySD);
//   web_server.on("/reboot", handleReboot); 
//   web_server.on("/scan", handleScan);  
//   web_server.on("/nscan", handleStopScan);
//   web_server.on("/scan1", handleScan1); 
//   //web_server.on("/bootstrap.css",  html_bootstrap );
//   //web_server.on("/main.css",  html_css );
//   //web_server.on("/main.js",   html_js );
//   //web_server.on("/favicon.ico",   handleNotFound);
//   web_server.on("ctrl",  handleControl);
//   web_server.on("/SDupload1", SDhandleRoot1);
//   web_server.on("/SDupload", SDhandleRoot);
//   web_server.onNotFound(handleNotFound);

// Sdt hai quan VN 02838650456


#ifdef Valve_UI
//   /*handling uploading file */
    web_server.on("/SDupdate", HTTP_POST, [](){
    web_interface->web_server.sendHeader("Connection", "close");
  },[](){
    File root;
    HTTPUpload& upload = web_interface->web_server.upload();
    bool SDopened = Valve::getOpened();
    if(SDopened == false){ 
      SDopened = true;
      root = SD.open((String("/") + upload.filename).c_str(), FILE_WRITE);  
      if(!root){
        debugln("- failed to open file for writing");
        return;
      }
    } 
    if(upload.status == UPLOAD_FILE_WRITE){
      if(root.write(upload.buf, upload.currentSize) != upload.currentSize){
        debugln("- failed to write");
        return;
      }
    } else if(upload.status == UPLOAD_FILE_END){
      root.close();
      debugln("UPLOAD_FILE_END");
      SDopened = false;
    }
  });
#endif//Valve_UI  
    blockserial = false;
    restartmodule = false;
    _head = NULL;
    _nb_ip = 0;
    _upload_status = UPLOAD_STATUS_NONE;
}
//Destructor
WEBINTERFACE_CLASS::~WEBINTERFACE_CLASS()
{
    while (_head) {
        auth_ip * current = _head;
        _head = _head->_next;
        delete current;
    }
    _nb_ip = 0;
}
//check authentification
level_authenticate_type  WEBINTERFACE_CLASS::is_authenticated()
{
#ifdef AUTHENTICATION_FEATURE
    if (web_server.hasHeader ("Cookie") ) {
        String cookie = web_server.header ("Cookie");
        int pos = cookie.indexOf ("ESPSESSIONID=");
        if (pos != -1) {
            int pos2 = cookie.indexOf (";", pos);
            String sessionID = cookie.substring (pos + strlen ("ESPSESSIONID="), pos2);
            IPAddress ip = web_server.client().remoteIP();
            //check if cookie can be reset and clean table in same time
            return ResetAuthIP (ip, sessionID.c_str() );
        }
    }
    return LEVEL_GUEST;
#else
    return LEVEL_ADMIN;
#endif
}

#ifdef AUTHENTICATION_FEATURE
//add the information in the linked list if possible
bool WEBINTERFACE_CLASS::AddAuthIP (auth_ip * item)
{
    if (_nb_ip > MAX_AUTH_IP) {
        return false;
    }
    item->_next = _head;
    _head = item;
    _nb_ip++;
    return true;
}

//Session ID based on IP and time using 16 char
char * WEBINTERFACE_CLASS::create_session_ID()
{
    static char  sessionID[17];
//reset SESSIONID
    for (int i = 0; i < 17; i++) {
        sessionID[i] = '\0';
    }
//get time
    uint32_t now = millis();
//get remote IP
    IPAddress remoteIP = web_server.client().remoteIP();
//generate SESSIONID
    if (0 > sprintf (sessionID, "%02X%02X%02X%02X%02X%02X%02X%02X", remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3], (uint8_t) ( (now >> 0) & 0xff), (uint8_t) ( (now >> 8) & 0xff), (uint8_t) ( (now >> 16) & 0xff), (uint8_t) ( (now >> 24) & 0xff) ) ) {
        strcpy (sessionID, "NONE");
    }
    return sessionID;
}



bool WEBINTERFACE_CLASS::ClearAuthIP (IPAddress ip, const char * sessionID)
{
    auth_ip * current = _head;
    auth_ip * previous = NULL;
    bool done = false;
    while (current) {
        if ( (ip == current->ip) && (strcmp (sessionID, current->sessionID) == 0) ) {
            //remove
            done = true;
            if (current == _head) {
                _head = current->_next;
                _nb_ip--;
                delete current;
                current = _head;
            } else {
                previous->_next = current->_next;
                _nb_ip--;
                delete current;
                current = previous->_next;
            }
        } else {
            previous = current;
            current = current->_next;
        }
    }
    return done;
}

//Get info
auth_ip * WEBINTERFACE_CLASS::GetAuth (IPAddress ip, const char * sessionID)
{
    auth_ip * current = _head;
    //auth_ip * previous = NULL;
    //get time
    //uint32_t now = millis();
    while (current) {
        if (ip == current->ip) {
            if (strcmp (sessionID, current->sessionID) == 0) {
                //found
                return current;
            }
        }
        //previous = current;
        current = current->_next;
    }
    return NULL;
}

//Review all IP to reset timers
level_authenticate_type WEBINTERFACE_CLASS::ResetAuthIP (IPAddress ip, const char * sessionID)
{
    auth_ip * current = _head;
    auth_ip * previous = NULL;
    //get time
    //uint32_t now = millis();
    while (current) {
        if ( (millis() - current->last_time) > 180000) {
            //remove
            if (current == _head) {
                _head = current->_next;
                _nb_ip--;
                delete current;
                current = _head;
            } else {
                previous->_next = current->_next;
                _nb_ip--;
                delete current;
                current = previous->_next;
            }
        } else {
            if (ip == current->ip) {
                if (strcmp (sessionID, current->sessionID) == 0) {
                    //reset time
                    current->last_time = millis();
                    return (level_authenticate_type) current->level;
                }
            }
            previous = current;
            current = current->_next;
        }
    }
    return LEVEL_GUEST;
}
#endif

//Check what is the content tye according extension file
String WEBINTERFACE_CLASS::getContentType (String filename)
{
    if (filename.endsWith (".htm") ) {
        return "text/html";
    } else if (filename.endsWith (".html") ) {
        return "text/html";
    } else if (filename.endsWith (".css") ) {
        return "text/css";
    } else if (filename.endsWith (".js") ) {
        return "application/javascript";
    } else if (filename.endsWith (".png") ) {
        return "image/png";
    } else if (filename.endsWith (".gif") ) {
        return "image/gif";
    } else if (filename.endsWith (".jpeg") ) {
        return "image/jpeg";
    } else if (filename.endsWith (".jpg") ) {
        return "image/jpeg";
    } else if (filename.endsWith (".ico") ) {
        return "image/x-icon";
    } else if (filename.endsWith (".xml") ) {
        return "text/xml";
    } else if (filename.endsWith (".pdf") ) {
        return "application/x-pdf";
    } else if (filename.endsWith (".zip") ) {
        return "application/x-zip";
    } else if (filename.endsWith (".gz") ) {
        return "application/x-gzip";
    } else if (filename.endsWith (".txt") ) {
        return "text/plain";
    }
    return "application/octet-stream";
}


WEBINTERFACE_CLASS * web_interface;
