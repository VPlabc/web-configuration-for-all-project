/*
  syncwebserver.cpp - ESP3D sync functions class

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
#include <Arduino.h>
#include "config.h"
#if !defined(ASYNCWEBSERVER)

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
#if defined ( ARDUINO_ARCH_ESP8266)
#include "ESP8266WiFi.h"
#include <ESP8266WebServer.h>
#endif
#if defined ( ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include "SPIFFS.h"
#include "Update.h"
#include <esp_ota_ops.h>
#endif

#include "GenLinkedList.h"
#include "command.h"
#include "espcom.h"
#include "WIC.h"
WIC W;
#ifdef Switch_UI
#include <EEPROM.h>
#include <time.h>
#endif//Switch_UI
#ifdef SSDP_FEATURE
#ifdef ARDUINO_ARCH_ESP32
#include <ESP32SSDP.h>
#else
#include <ESP8266SSDP.h>
#include <FS.h>
#include <SDFS.h>
#endif
#endif

#include <SPI.h>
#include <SD.h>

//embedded response file if no files on SPIFFS
#include "nofile.h"
#include "syncwebserver.h"
WebSocketsServer * socket_server;
#ifdef PLC_MASTER_UI
#include "PLC_IoT/PLC_Master.h"
PLC_MASTER webPLC;
#endif//PLC_MASTER_UI
#ifdef Switch_UI
#include "LightTimer/LightTimer.h"
LightTimer lights;
#endif//Switch_UI

#define ESP_ERROR_AUTHENTICATION 1
#define ESP_ERROR_FILE_CREATION  2
#define ESP_ERROR_FILE_WRITE 3
#define ESP_ERROR_UPLOAD 4
#define ESP_ERROR_NOT_ENOUGH_SPACE 5
#define ESP_ERROR_UPLOAD_CANCELLED 6
#define ESP_ERROR_FILE_CLOSE 7
#define ESP_ERROR_NO_SD 8
#define ESP_ERROR_MOUNT_SD 9
#define ESP_ERROR_RESET_NUMBERING 10
#define ESP_ERROR_BUFFER_OVERFLOW 11
#define ESP_ERROR_START_UPLOAD 12


void pushError(int code, const char * st, uint16_t web_error = 500, uint16_t timeout = 1000){
    if (socket_server && st) {
        String s = "ERROR:" + String(code) + ":";
        s+=st;
        socket_server->sendTXT(ESPCOM::current_socket_id, s);
        if (web_error != 0) {
            if (web_interface) {
                if (web_interface->web_server.client().available() > 0) {
                    web_interface->web_server.send (web_error, "text/xml", st);
                }
            }
        }
        uint32_t t = millis();
        while (millis() - t < timeout) {
            socket_server->loop();
            delay(10);
        }
    }
} 

//abort reception of packages
void cancelUpload(){  
    if (web_interface) {
        if (web_interface->web_server.client().available() > 0) {
            HTTPUpload& upload = (web_interface->web_server).upload();
            upload.status = UPLOAD_FILE_ABORTED;
#if defined (ARDUINO_ARCH_ESP8266)
            web_interface->web_server.client().stopAll();
#endif
#if defined (ARDUINO_ARCH_ESP32) 
            errno = ECONNABORTED;
            web_interface->web_server.client().stop();
#endif
            delay(100);
        } 
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{

    switch(type) {
    case WStype_DISCONNECTED:
        //USE_SERIAL.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED: {
        //IPAddress ip = socket_server->remoteIP(num);
        //USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        String s = "CURRENT_ID:" + String(num);
        // send message to client
        ESPCOM::current_socket_id = num;
        socket_server->sendTXT(ESPCOM::current_socket_id, s);
        s = "ACTIVE_ID:" + String(ESPCOM::current_socket_id);
        socket_server->broadcastTXT(s);
        #ifdef Valve_UI
        //W.Load();W.wsSendToWeb();
        #endif//Valve_UI
        #ifdef Switch_UI
        lights.Load();lights.wsSendToWeb();
        #endif//Valve_UI
    }
    break;
    case WStype_TEXT:
    #ifdef PLC_MASTER_UI
    webPLC.SocketRecive(payload);
    #endif//PLC_MASTER_UI
#ifdef Switch_UI    
            lights.reciverDataFromWeb(payload);
#endif//Switch_UI
        //USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

        // send message to client
        // webSocket.sendTXT(num, "message here");

        // send data to all connected clients
        // webSocket.broadcastTXT("message here");
        break;
    case WStype_BIN:
        //USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
        //hexdump(payload, length);

        // send message to client
        // webSocket.sendBIN(num, payload, length);
        break;
    default:
        break;
    }

}



extern bool  deleteRecursive(String path);
extern void CloseSerialUpload (bool iserror, String & filename, int32_t linenb);
extern bool sendLine2Serial (String &  line, int32_t linenb, int32_t* newlinenb);
extern bool purge_serial();

const uint8_t PAGE_404 [] PROGMEM = "<HTML>\n<HEAD>\n<title>Redirecting...</title> \n</HEAD>\n<BODY>\n<CENTER>Unknown page : $QUERY$- you will be redirected...\n<BR><BR>\nif not redirected, <a href='http://$WEB_ADDRESS$'>click here</a>\n<BR><BR>\n<PROGRESS name='prg' id='prg'></PROGRESS>\n\n<script>\nvar i = 0; \nvar x = document.getElementById(\"prg\"); \nx.max=5; \nvar interval=setInterval(function(){\ni=i+1; \nvar x = document.getElementById(\"prg\"); \nx.value=i; \nif (i>5) \n{\nclearInterval(interval);\nwindow.location.href='/';\n}\n},1000);\n</script>\n</CENTER>\n</BODY>\n</HTML>\n\n";
const uint8_t PAGE_CAPTIVE [] PROGMEM = "<HTML>\n<HEAD>\n<title>Captive Portal</title> \n</HEAD>\n<BODY>\n<CENTER>Captive Portal page : $QUERY$- you will be redirected...\n<BR><BR>\nif not redirected, <a href='http://$WEB_ADDRESS$'>click here</a>\n<BR><BR>\n<PROGRESS name='prg' id='prg'></PROGRESS>\n\n<script>\nvar i = 0; \nvar x = document.getElementById(\"prg\"); \nx.max=5; \nvar interval=setInterval(function(){\ni=i+1; \nvar x = document.getElementById(\"prg\"); \nx.value=i; \nif (i>5) \n{\nclearInterval(interval);\nwindow.location.href='/';\n}\n},1000);\n</script>\n</CENTER>\n</BODY>\n</HTML>\n\n";
#define  CONTENT_TYPE_HTML "text/html"

//Root of Webserver/////////////////////////////////////////////////////

void handle_web_interface_root()
{
    
    String path = "/index.html";
    String contentType =  web_interface->getContentType(path);
    String pathWithGz = path + ".gz";
    //if have a index.html or gzip version this is default root page
    if((SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) && !web_interface->web_server.hasArg("forcefallback") && web_interface->web_server.arg("forcefallback")!="yes") {
        if(SPIFFS.exists(pathWithGz)) {
            path = pathWithGz;
        }
        FS_FILE file = SPIFFS.open(path, SPIFFS_FILE_READ);
        web_interface->web_server.streamFile(file, contentType);
        file.close();
        return;
    }
    //if no lets launch the default content
    web_interface->web_server.sendHeader("Content-Encoding", "gzip");
    web_interface->web_server.send_P(200,CONTENT_TYPE_HTML,PAGE_NOFILES,PAGE_NOFILES_SIZE);
}

void handle_web_interface_tool()
{
    web_interface->web_server.sendHeader("Content-Encoding", "gzip");
    web_interface->web_server.send_P(200,CONTENT_TYPE_HTML,PAGE_NOFILES,PAGE_NOFILES_SIZE);
}

//Login check///////////////////////////////////////////////////////////
void handle_login()
{
#ifdef AUTHENTICATION_FEATURE
    String smsg;
    String sUser,sPassword;
    String auths;
    int code = 200;
    bool msg_alert_error=false;
    //disconnect can be done anytime no need to check credential
    if (web_interface->web_server.hasArg("DISCONNECT")) {
        String cookie = web_interface->web_server.header("Cookie");
        int pos = cookie.indexOf("ESPSESSIONID=");
        String sessionID;
        if (pos!= -1) {
            int pos2 = cookie.indexOf(";",pos);
            sessionID = cookie.substring(pos+strlen("ESPSESSIONID="),pos2);
        }
        web_interface->ClearAuthIP(web_interface->web_server.client().remoteIP(), sessionID.c_str());
        web_interface->web_server.sendHeader("Set-Cookie","ESPSESSIONID=0");
        web_interface->web_server.sendHeader("Cache-Control","no-cache");
        String buffer2send = "{\"status\":\"Ok\",\"authentication_lvl\":\"guest\"}";
        web_interface->web_server.send(code, "application/json", buffer2send);
        return;
    }

    level_authenticate_type auth_level= web_interface->is_authenticated();
    if (auth_level == LEVEL_GUEST) {
        auths = F("guest");
    } else if (auth_level == LEVEL_USER) {
        auths = F("user");
    } else if (auth_level == LEVEL_ADMIN) {
        auths = F("admin");
    } else {
        auths = F("???");
    }

    //check is it is a submission or a query
    if (web_interface->web_server.hasArg("SUBMIT")) {
        //is there a correct list of query?
        if ( web_interface->web_server.hasArg("PASSWORD")&& web_interface->web_server.hasArg("USER")) {
            //USER
            sUser = web_interface->web_server.arg("USER");
            if ( !((sUser==FPSTR(DEFAULT_ADMIN_LOGIN)) || (sUser==FPSTR(DEFAULT_USER_LOGIN)))) {
                msg_alert_error=true;
                smsg=F("Error : Incorrect User");
                code=401;
            }
            if (msg_alert_error == false) {
                //Password
                sPassword = web_interface->web_server.arg("PASSWORD");
                String sadminPassword;

                if (!CONFIG::read_string(EP_ADMIN_PWD, sadminPassword, MAX_LOCAL_PASSWORD_LENGTH)) {
                    sadminPassword=FPSTR(DEFAULT_ADMIN_PWD);
                }

                String suserPassword;

                if (!CONFIG::read_string(EP_USER_PWD, suserPassword, MAX_LOCAL_PASSWORD_LENGTH)) {
                    suserPassword=FPSTR(DEFAULT_USER_PWD);
                }

                if(!(((sUser==FPSTR(DEFAULT_ADMIN_LOGIN)) && (strcmp(sPassword.c_str(),sadminPassword.c_str())==0)) ||
                        ((sUser==FPSTR(DEFAULT_USER_LOGIN)) && (strcmp(sPassword.c_str(),suserPassword.c_str()) == 0)))) {
                    msg_alert_error=true;
                    smsg=F("Error: Incorrect password");
                    Serial.println(sPassword.c_str());
                    Serial.println(sadminPassword.c_str());
                    code = 401;
                }
            }
        } else {
            msg_alert_error=true;
            smsg = F("Error: Missing data");
            code = 500;
        }
        //change password
        if ( web_interface->web_server.hasArg("PASSWORD")&& web_interface->web_server.hasArg("USER") && web_interface->web_server.hasArg("NEWPASSWORD") && (msg_alert_error==false) ) {
            String newpassword =  web_interface->web_server.arg("NEWPASSWORD");
            if (CONFIG::isLocalPasswordValid(newpassword.c_str())) {
                int pos=0;
                if(sUser==FPSTR(DEFAULT_ADMIN_LOGIN)) {
                    pos = EP_ADMIN_PWD;
                } else {
                    pos = EP_USER_PWD;
                }
                if (!CONFIG::write_string(pos,newpassword.c_str())) {
                    msg_alert_error=true;
                    smsg = F("Error: Cannot apply changes");
                    code = 500;
                }
            } else {
                msg_alert_error=true;
                smsg = F("Error: Incorrect password");
                code = 500;
            }
        }
        if ((code == 200) || (code == 500)) {
            level_authenticate_type current_auth_level;
            if(sUser == FPSTR(DEFAULT_ADMIN_LOGIN)) {
                current_auth_level = LEVEL_ADMIN;
            } else  if(sUser == FPSTR(DEFAULT_USER_LOGIN)) {
                current_auth_level = LEVEL_USER;
            } else {
                current_auth_level = LEVEL_GUEST;
            }
            //create Session
            if ((current_auth_level != auth_level) || (auth_level== LEVEL_GUEST)) {
                auth_ip * current_auth = new auth_ip;
                current_auth->level = current_auth_level;
                current_auth->ip=web_interface->web_server.client().remoteIP();
                strcpy(current_auth->sessionID,web_interface->create_session_ID());
                strcpy(current_auth->userID,sUser.c_str());
                current_auth->last_time=millis();
                if (web_interface->AddAuthIP(current_auth)) {
                    String tmps ="ESPSESSIONID=";
                    tmps+=current_auth->sessionID;
                    web_interface->web_server.sendHeader("Set-Cookie",tmps);
                    web_interface->web_server.sendHeader("Cache-Control","no-cache");
                    switch(current_auth->level) {
                    case LEVEL_ADMIN:
                        auths = "admin";
                        break;
                    case LEVEL_USER:
                        auths = "user";
                        break;
                    default:
                        auths = "guest";
                        break;
                    }
                } else {
                    delete current_auth;
                    msg_alert_error=true;
                    code = 500;
                    smsg = F("Error: Too many connections");
                }
            }
        }
        if (code == 200) {
            smsg = F("Ok");
        }

        //build  JSON
        String buffer2send = "{\"status\":\"" + smsg + "\",\"authentication_lvl\":\"";
        buffer2send += auths;
        buffer2send += "\"}";
        web_interface->web_server.send(code, "application/json", buffer2send);
    } else {
        if (auth_level != LEVEL_GUEST) {
            String cookie = web_interface->web_server.header("Cookie");
            int pos = cookie.indexOf("ESPSESSIONID=");
            String sessionID;
            if (pos!= -1) {
                int pos2 = cookie.indexOf(";",pos);
                sessionID = cookie.substring(pos+strlen("ESPSESSIONID="),pos2);
                auth_ip * current_auth_info = web_interface->GetAuth(web_interface->web_server.client().remoteIP(), sessionID.c_str());
                if (current_auth_info != NULL) {
                    sUser = current_auth_info->userID;
                }
            }
        }
        String buffer2send = "{\"status\":\"200\",\"authentication_lvl\":\"";
        buffer2send += auths;
        buffer2send += "\",\"user\":\"";
        buffer2send += sUser;
        buffer2send +="\"}";
        web_interface->web_server.send(code, "application/json", buffer2send);
    }
#else
    web_interface->web_server.sendHeader("Cache-Control","no-cache");
    web_interface->web_server.send(200, "application/json", "{\"status\":\"Ok\",\"authentication_lvl\":\"admin\"}");
#endif
}

//SSDP interface////////////////////////////////////////
#ifdef SSDP_FEATURE
void handle_SSDP()
{
    SSDP.schema(web_interface->web_server.client());
}
#endif

//SPIFFS files list and file commands///////////////////////////////////
void handleFileList()
{
    level_authenticate_type auth_level = web_interface->is_authenticated();
    if (auth_level == LEVEL_GUEST) {
        web_interface->_upload_status=UPLOAD_STATUS_NONE;
        web_interface->web_server.send(401,"text/plain","Authentication failed!\n");
        return;
    }
    String path ;
    String Status = "Ok";
    if ((web_interface->_upload_status == UPLOAD_STATUS_FAILED) || (web_interface->_upload_status == UPLOAD_STATUS_FAILED)) {
        Status = "Upload failed";
        web_interface->_upload_status=UPLOAD_STATUS_NONE;
    }
    //be sure root is correct according authentication
    if (auth_level == LEVEL_ADMIN) {
        path = "/";
    } else {
        path = "/user";
    }
    //get current path
    if(web_interface->web_server.hasArg("path")) {
        path += web_interface->web_server.arg("path") ;
    }
    //to have a clean path
    path.trim();
    path.replace("//","/");
    if (path[path.length()-1] !='/') {
        path +="/";
    }
    //check if query need some action
    if(web_interface->web_server.hasArg("action")) {
        //delete a file
        if(web_interface->web_server.arg ("action") == "delete" && web_interface->web_server.hasArg ("filename")) {
            String filename;
            String shortname = web_interface->web_server.arg ("filename");
            shortname.replace ("/","");
            filename = path + web_interface->web_server.arg ("filename");
            filename.replace ("//","/");
            if (!SPIFFS.exists (filename)) {
                Status = shortname + F(" does not exists!");
            } else {
                if (SPIFFS.remove(filename)) {
                    Status = shortname + F(" deleted");
                    //what happen if no "/." and no other subfiles ?
    #if defined(ARDUINO_ARCH_ESP8266)
                        FS_DIR dir = SPIFFS.openDir(path);
                        if (!dir.next()) {
    #else
                        String ptmp = path;
                        if ( (path != "/") && (path[path.length() - 1] == '/') ) {
                            ptmp = path.substring (0, path.length() - 1);
                        }
                        FS_FILE dir = SPIFFS.open (ptmp);
                        FS_FILE dircontent = dir.openNextFile();
                        if (!dircontent) {
    #endif
                            //keep directory alive even empty
                            FS_FILE r = SPIFFS.open (path+"/.", SPIFFS_FILE_WRITE);
                            if (r) {
                                r.close();
                            }
                        }
                    } else {
                        Status = F("Cannot deleted ") ;
                        Status+=  shortname ;
                    }
                }
            }
            //delete a directory
            if(web_interface->web_server.arg("action") == "deletedir" && web_interface->web_server.hasArg("filename")) {
                String filename;
                String shortname = web_interface->web_server.arg("filename");
                shortname.replace("/","");
                filename = path + web_interface->web_server.arg("filename");
                filename += "/";
                filename.replace("//","/");
                if (filename != "/") {
                    bool delete_error = false;
    #if defined ( ARDUINO_ARCH_ESP8266)
                    FS_DIR dir = SPIFFS.openDir(path + shortname);
                    {
                        while (dir.next()) {
    #else
                    FS_FILE dir = SPIFFS.open(path + shortname);
                    {
                        FS_FILE file2deleted = dir.openNextFile();
                        while (file2deleted) {
    #endif
    #if defined ( ARDUINO_ARCH_ESP8266)
                            String fullpath = dir.fileName();
    #else
                            String fullpath = file2deleted.name();
    #endif
                            if (!SPIFFS.remove(fullpath)) {
                                delete_error = true;
                                Status = F("Cannot deleted ") ;
                                Status+=fullpath;
                            }
    #if defined(ARDUINO_ARCH_ESP32)
                            file2deleted = dir.openNextFile();
    #endif
                    }
                }
                if (!delete_error) {
                    Status = shortname ;
                    Status+= " deleted";
                }
            }
        }
        //create a directory
        if(web_interface->web_server.arg("action")=="createdir" && web_interface->web_server.hasArg("filename")) {
            String filename;
            filename = path + web_interface->web_server.arg("filename") +"/.";
            String shortname = web_interface->web_server.arg("filename");
            shortname.replace("/","");
            filename.replace("//","/");
            if(SPIFFS.exists(filename)) {
                Status = shortname + F(" already exists!");
            } else {
                FS_FILE r = SPIFFS.open(filename,SPIFFS_FILE_WRITE);
                if (!r) {
                    Status = F("Cannot create ");
                    Status+=  shortname ;
                } else {
                    r.close();
                    Status = shortname + F(" created");
                }
            }
        }
    }
        String jsonfile = "{";
    #if defined ( ARDUINO_ARCH_ESP8266 )
        FS_DIR dir = SPIFFS.openDir(path);
    #else
        String ptmp = path;
        if ( (path != "/") && (path[path.length() - 1] == '/') ) {
            ptmp = path.substring (0, path.length() - 1);
        }
        FS_FILE dir = SPIFFS.open(ptmp);
    #endif
        jsonfile+="\"files\":[";
        bool firstentry=true;
        String subdirlist="";
    #if defined ( ARDUINO_ARCH_ESP8266 )
        while (dir.next()) {
            String filename = dir.fileName();
    #else
        File fileparsed = dir.openNextFile();
        while (fileparsed) {
            String filename = fileparsed.name();
    #endif
            String size ="";
            bool addtolist=true;
            //remove path from name
            filename = filename.substring(path.length(),filename.length());
            //check if file or subfile
            if (filename.indexOf("/")>-1) {
                //Do not rely on "/." to define directory as SPIFFS upload won't create it but directly files
                //and no need to overload SPIFFS if not necessary to create "/." if no need
                //it will reduce SPIFFS available space so limit it to creation
                filename = filename.substring(0,filename.indexOf("/"));
                String tag="*";
                tag += filename + "*";
                if (subdirlist.indexOf(tag)>-1 || filename.length()==0) { //already in list
                    addtolist = false; //no need to add
                } else {
                    size = -1; //it is subfile so display only directory, size will be -1 to describe it is directory
                    if (subdirlist.length()==0) {
                        subdirlist+="*";
                    }
                    subdirlist += filename + "*"; //add to list
                }
            } else {
                //do not add "." file
                if (!((filename==".") || (filename==""))) {
    #if defined ( ARDUINO_ARCH_ESP8266)
                    size = CONFIG::formatBytes(dir.fileSize());
    #else
                    size = CONFIG::formatBytes(fileparsed.size());
    #endif

                } else {
                    addtolist = false;
                }
            }
            if(addtolist) {
                if (!firstentry) {
                    jsonfile+=",";
                } else {
                    firstentry=false;
                }
                jsonfile+="{";
                jsonfile+="\"name\":\"";
                jsonfile+=filename;
                jsonfile+="\",\"size\":\"";
                jsonfile+=size;
                jsonfile+="\"";
                jsonfile+="}";
            }
    #ifdef ARDUINO_ARCH_ESP32
            fileparsed = dir.openNextFile();
    #endif
        }
        jsonfile+="],";
        jsonfile+="\"path\":\"" + path + "\",";
        jsonfile+="\"status\":\"" + Status + "\",";
        size_t totalBytes;
        size_t usedBytes;
    #if defined ( ARDUINO_ARCH_ESP8266)
        fs::FSInfo info;
        SPIFFS.info(info);
        totalBytes = info.totalBytes;
        usedBytes = info.usedBytes;
    #else
        totalBytes = SPIFFS.totalBytes();
        usedBytes = SPIFFS.usedBytes();
    #endif
        jsonfile+="\"total\":\"" + CONFIG::formatBytes(totalBytes) + "\",";
        jsonfile+="\"used\":\"" + CONFIG::formatBytes(usedBytes) + "\",";
        jsonfile.concat(F("\"occupation\":\""));
        jsonfile+= CONFIG::intTostr(100*usedBytes/totalBytes);
        jsonfile+="\"";
        jsonfile+="}";
        path = "";
        web_interface->web_server.sendHeader("Cache-Control", "no-cache");
        web_interface->web_server.send(200, "application/json", jsonfile);
        web_interface->_upload_status=UPLOAD_STATUS_NONE;
        //Serial.println(jsonfile);
}

//SPIFFS files uploader handle
void SPIFFSFileupload()
{
    static FS_FILE fsUploadFile = (FS_FILE)0;
    static String filename;
    //get authentication status
    level_authenticate_type auth_level= web_interface->is_authenticated();
    //Guest cannot upload
    if (auth_level == LEVEL_GUEST) {
        web_interface->_upload_status=UPLOAD_STATUS_FAILED;
        ESPCOM::println (F ("Upload rejected"), PRINTER_PIPE);
        pushError(ESP_ERROR_AUTHENTICATION, "Upload rejected", 401);
    } else {
        //get current file ID
        HTTPUpload& upload = (web_interface->web_server).upload();
        if ((web_interface->_upload_status != UPLOAD_STATUS_FAILED) || (upload.status == UPLOAD_FILE_START)) {
            //Upload start
            //**************
            if(upload.status == UPLOAD_FILE_START) {
                web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
                String upload_filename = upload.filename;
                String  sizeargname  = upload_filename + "S";
                if (upload_filename[0] != '/') filename = "/" + upload_filename;
                else filename = upload.filename;
                //according User or Admin the root is different as user is isolate to /user when admin has full access
                if(auth_level != LEVEL_ADMIN) {
                    upload_filename = filename;
                    filename = "/user" + upload_filename;
                }
                
                if (SPIFFS.exists (filename) ) {
                    SPIFFS.remove (filename);
                }
                if (fsUploadFile ) {
                    fsUploadFile.close();
                }
                if ((web_interface->web_server).hasArg (sizeargname.c_str()) ) {
                    uint32_t filesize = (web_interface->web_server).arg (sizeargname.c_str()).toInt();
    #if defined ( ARDUINO_ARCH_ESP8266)
                    fs::FSInfo info;
                    SPIFFS.info(info);
                     uint32_t freespace = info.totalBytes- info.usedBytes;
    #endif
    #if defined ( ARDUINO_ARCH_ESP32)
                    uint32_t freespace = SPIFFS.totalBytes() - SPIFFS.usedBytes();
    #endif
                    
                    if (filesize > freespace) {
                        web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                        pushError(ESP_ERROR_NOT_ENOUGH_SPACE, "Upload rejected, not enough space");
                    }
                }
                if (web_interface->_upload_status != UPLOAD_STATUS_FAILED) {
                    //create file
                    fsUploadFile = SPIFFS.open(filename, SPIFFS_FILE_WRITE);
                    //check If creation succeed
                    if (fsUploadFile) {
                        //if yes upload is started
                        web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
                    } else {
                        //if no set cancel flag
                        web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                        ESPCOM::println (F ("Error ESP create"), PRINTER_PIPE);
                        pushError(ESP_ERROR_FILE_CREATION, "File creation failed");
                    }
                }
                //Upload write
                //**************
            } else if(upload.status == UPLOAD_FILE_WRITE) {
                //check if file is available and no error
                if(fsUploadFile && web_interface->_upload_status == UPLOAD_STATUS_ONGOING) {
                    //no error so write post date
                    if (upload.currentSize != fsUploadFile.write(upload.buf, upload.currentSize)){
                        web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                        ESPCOM::println (F ("Error ESP write"), PRINTER_PIPE);
                        pushError(ESP_ERROR_FILE_WRITE, "File write failed");
                        }
                } else {
                    //we have a problem set flag UPLOAD_STATUS_FAILED
                    web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                    ESPCOM::println (F ("Error ESP write"), PRINTER_PIPE);
                }
                //Upload end
                //**************
            } else if(upload.status == UPLOAD_FILE_END) {
                //check if file is still open
                if(fsUploadFile) {
                    //close it
                    fsUploadFile.close();
                    if (web_interface->_upload_status == UPLOAD_STATUS_ONGOING) {
                        web_interface->_upload_status = UPLOAD_STATUS_SUCCESSFUL;
                    }
                } else {
                    //we have a problem set flag UPLOAD_STATUS_FAILED
                    web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                    ESPCOM::println (F ("Error ESP close"), PRINTER_PIPE);
                    pushError(ESP_ERROR_FILE_CLOSE, "File close failed");
                }
                //Upload cancelled
                //**************
            } else {
                    web_interface->_upload_status = UPLOAD_STATUS_FAILED;
                    return;
                    //ESPCOM::println (F ("Error ESP upload"), PRINTER_PIPE);
                    //pushError(ESP_ERROR_UPLOAD, "File upload failed");
            }
        }
    }
    if (web_interface->_upload_status == UPLOAD_STATUS_FAILED) {
        cancelUpload();
        if (SPIFFS.exists (filename) ) {
            SPIFFS.remove (filename);
            }
    }
    CONFIG::wait(0);
}

#ifdef SDCARD_FEATURE
//SDCard files list and file commands///////////////////////////////////
void handleSDCardFileList()
{
    level_authenticate_type auth_level = web_interface->is_authenticated();
    if (auth_level == LEVEL_GUEST) {
        web_interface->_upload_status=UPLOAD_STATUS_NONE;
        web_interface->web_server.send(401,"text/plain","Authentication failed!\n");
        return;
    }
    String path ;
    String Status = "Ok";
    String filename;
    String shortname;
        #ifdef ESP8266
    if(CONFIG::GetState() == false){
        if (!SD.begin(SDCard_CS, SPI_FULL_SPEED)) {Status = "SD Card not found";}
    }
    #else
    if (!SD.begin(SDCard_CS)){Status = "SD Card not found";}
    #endif//ESP8266
    if ((web_interface->_upload_status == UPLOAD_STATUS_FAILED) || (web_interface->_upload_status == UPLOAD_STATUS_FAILED)) {
        Status = "Upload failed";
        web_interface->_upload_status=UPLOAD_STATUS_NONE;
    }
    //be sure root is correct according authentication
    if (auth_level == LEVEL_ADMIN) {
        path = "/";
    } else {
        path = "/user";
    }
    
    //get current path
    if(web_interface->web_server.hasArg("path")) {
        path += web_interface->web_server.arg("path") ;
        if(path.length() > 1){path = "//";path += web_interface->web_server.arg("path") ;}
    }
    //to have a clean path
    path.trim();
    if (path[path.length()-1] !='/') {
        path +="/";
    }
    
    path.replace("//","");
    //check if query need some action
    if(web_interface->web_server.hasArg("action")) {
        //delete a file
        if(web_interface->web_server.arg ("action") == "delete" && web_interface->web_server.hasArg ("filename")) {
            shortname = web_interface->web_server.arg ("filename");
            //shortname.replace ("/","");
            filename = path + web_interface->web_server.arg ("filename");
            filename.replace ("//","/");
            #ifdef ESP8266
            if (!SDFS.exists (filename)) {
            #else
            if (!SD.exists (filename)) {
            #endif//ESP8266
                Status = shortname + F(" does not exists!");
            } else {
            #ifdef ESP8266
            if (!SDFS.remove (filename)) {
            #else
                    LOG(filename + '\n');
            if (SD.remove(filename)) {
            #endif//ESP8266
                    Status = shortname + F(" deleted");
                    //what happen if no "/." and no other subfiles ?
    // #if defined(ARDUINO_ARCH_ESP8266)
    //                     FS_DIR dir = SDFS.openDir(path);
    //                     if (!dir.next()) {
    // #else
    //                     String ptmp = path;
    //                     if ( (path != "/") && (path[path.length() - 1] == '/') ) {
    //                         ptmp = path.substring (0, path.length() - 1);
    //                     }
    //                     #ifdef ESP8266
    //                     FS_FILE dir = SDFS.open (ptmp);
    //                     #else
    //                     File dir = SD.open (ptmp);
    //                     #endif//ESP8266
                        
    //                     File dircontent = dir.openNextFile();
    //                     if (!dircontent) {
    // #endif
    //                         //keep directory alive even empty
    //                         #ifdef ESP8266
    //                         FS_FILE r = SDFS.open (path+"/.", "w");
    //                         #else
    //                         File r = SD.open (path+"/.", FILE_WRITE);
    //                         #endif//ESP8266
                            
    //                         if (r) {
    //                             r.close();
    //                         }
    //                     }
                    } else {
                        Status = F("Cannot deleted ") ;
                        Status+=  shortname ;
                    }
                }
            }
            //delete a directory
            if(web_interface->web_server.arg("action") == "deletedir" && web_interface->web_server.hasArg("filename")) {
                shortname = web_interface->web_server.arg("filename");
                shortname.replace("/","");
                //filename = "/";
                filename = path + web_interface->web_server.arg("filename");
                filename.replace("//","");
                if (filename != "") {
                    bool delete_error = false;
    #if defined ( ARDUINO_ARCH_ESP8266)
                    FS_DIR dir = SDFS.openDir(path + shortname);
                    {
                        while (dir.next()) {
    #else
                    
                    #ifdef ESP32
                    File dir = SD.open(path + shortname, FILE_READ);
                    {
                    #endif//ESP32
                        File file2deleted = dir.openNextFile();
                        while (file2deleted) {
    #endif
    #if defined ( ARDUINO_ARCH_ESP8266)
                            String fullpath = dir.fileName();
    #else
                            String fullpath = file2deleted.name();
    #endif
    
                Serial.println("fullpath:"+fullpath);
                
                            #ifdef ESP8266
                            if (!SDFS.remove(fullpath)) {
                            #else
                            if (!SD.rmdir(fullpath)) {
                            #endif//ESP8266
                                delete_error = true;
                                Status = F("Cannot deleted ") ;
                                Status+=fullpath;
                            }
    #if defined(ARDUINO_ARCH_ESP32)
                            file2deleted = dir.openNextFile();
    #endif
                    }
                }
                if (!delete_error) {
                    Status = shortname ;
                    Status+=" deleted";
                }
            }
        }
        //create a directory
        if(web_interface->web_server.arg("action")=="createdir" && web_interface->web_server.hasArg("filename")) {
            filename = path + web_interface->web_server.arg("filename") +"/.";
            shortname = web_interface->web_server.arg("filename");
            shortname.replace("/","");
            filename.replace("//","/");
            #ifdef ESP8266
            if(SDFS.exists(filename)) {
            #else
            if(SD.exists("/" + filename)) {
            #endif//ESP8266
                Status = shortname + F(" already exists!");
            } else {
                #ifdef ESP8266
                FS_FILE r = SDFS.open(filename,"w");
                if (!r) {
                #else
                if (!SD.mkdir(filename)) {
                #endif//ESP8266
                    Status = F("Cannot create ");
                    Status+=  shortname ;
                } else {
                #ifdef ESP8266
                    r.close();
                #endif//ESP8266
                    Status = shortname + F(" created");
                }
            }
        }
    }
 
#ifdef ESP8266   
        // Serial.println("path:"+String(path));
        // Serial.println("filename:"+filename);
        // Serial.println("shortname:"+shortname);
        //path = "/";
        // Serial.println("load");
        // Serial.println("Dir:"+path);
        String jsonfile = "{";
    #if defined ( ARDUINO_ARCH_ESP8266 )
        FS_DIR dir = SDFS.openDir(path);
    #else
        String ptmp = path;
        if ( (path != "/") && (path[path.length() - 1] == '/') ) {
            ptmp = path.substring (0, path.length() - 1);
        }
        File dir = SD.open(ptmp);
    #endif
        jsonfile+="\"SDfiles\":[";
        bool firstentry=true;
        String subdirlist="";
    #if defined ( ARDUINO_ARCH_ESP8266 )
        while (dir.next()) {
            String filename = dir.fileName();
    #else
        File fileparsed = dir.openNextFile();
        while (fileparsed) {
            String filename = fileparsed.name();
    #endif
            String size = "-1";
            bool addtolist=true;
            //remove path from name
            
            //path = "/";
            filename = filename.substring(path.length()-1,filename.length());
            //check if file or subfile
            if (filename.indexOf("/")>-1) {
                //Do not rely on "/." to define directory as SD upload won't create it but directly files
                //and no need to overload SD if not necessary to create "/." if no need
                //it will reduce SD available space so limit it to creation
                filename = filename.substring(0,filename.indexOf("/"));
                Serial.println("file Name:" + filename);
                String tag="*";
                tag += filename + "*";
                if (subdirlist.indexOf(tag)>-1 || filename.length()==0) { //already in list
                    addtolist = false; //no need to add
                    Serial.println("addtolist tag:" + String(tag) + "|" + String(addtolist));
                } else {
                    size = "-1"; //it is subfile so display only directory, size will be -1 to describe it is directory
                    if (subdirlist.length()==0) {
                        subdirlist+="*";
                    }
                    subdirlist += filename + "*"; //add to list
                }
            } else {
                //do not add "." file
                if (!((filename==".") || (filename==""))) {
    #if defined ( ARDUINO_ARCH_ESP8266)
                    size = CONFIG::formatBytes(dir.fileSize());
    #else
                    size = CONFIG::formatBytes(fileparsed.size());
                    
                    
    #endif
                //if(size == "0 B")size = "-1";
                Serial.println("size:" + String(size));
                } else {
                    addtolist = false;
                    Serial.println("else addtolist:" + String(addtolist));
                }
            }
                    Serial.println("addtolist:" + String(addtolist));
            if(addtolist) {
                if (!firstentry) {
                    jsonfile+=",";
                } else {
                    firstentry=false;
                }
                //if(size == "0")size = "-1";
                jsonfile+="{";
                jsonfile+="\"name\":\"";
                jsonfile+=filename;
                jsonfile+="\",\"size\":\"";
                jsonfile+=size;
                jsonfile+="\"";
                jsonfile+="}";
            }
    #ifdef ARDUINO_ARCH_ESP32
            fileparsed = dir.openNextFile();
    #endif
        }
        jsonfile+="],";
        jsonfile+="\"path\":\"" + path + "\",";
        jsonfile+="\"status\":\"" + Status + "\",";
        size_t totalBytes;
        size_t usedBytes;
    #if defined ( ARDUINO_ARCH_ESP8266)
        fs::FSInfo info;
        SDFS.info(info);
        totalBytes = info.totalBytes;
        usedBytes = info.usedBytes;
    #else
        totalBytes = SD.totalBytes();
        usedBytes = SD.usedBytes();
    #endif
        jsonfile+="\"total\":\"" + CONFIG::formatBytes(totalBytes) + "\",";
        jsonfile+="\"used\":\"" + CONFIG::formatBytes(usedBytes) + "\",";
        jsonfile.concat(F("\"occupation\":\""));
        jsonfile+= CONFIG::intTostr(100*usedBytes/totalBytes);
        jsonfile+="\"";
        jsonfile+="}";
        path = "";
#else//ESSP32
    String ptmp = path;
    if ( (path != "/") && (path[path.length() - 1] == '/') ) {
        ptmp = path.substring (0, path.length() - 1);
    }
    //Serial.printf("Listing directory: %s\n", ptmp);

    ptmp.replace("//","/");
  File root = SD.open(ptmp);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }
  uint8_t levels = 0;
  String jsonfile = "{";
  jsonfile+="\"SDfiles\":[";
  bool firstentry=true;
  File file = root.openNextFile();
  while(file){
    if (!firstentry) {jsonfile+=",";} else {firstentry=false;}
    if(file.isDirectory()){
    //   Serial.print("  DIR : ");
    //   Serial.println(file.name());
        String FileName = file.name();
        FileName.replace("/","");
        jsonfile+="{";
        jsonfile+="\"name\":\"";
        jsonfile+=FileName;
        jsonfile+="\",\"size\":\"";
        jsonfile+="-1";
        jsonfile+="\"";
        jsonfile+="}";
      if(levels){
        //listDir(fs, file.name(), levels -1);
      }
    } else {
        String size = CONFIG::formatBytes(file.size());
    //   Serial.print("  FILE: ");
    //   Serial.print(file.name());
    //   Serial.print("  SIZE: ");
    //   Serial.println(file.size());
        String FileName = file.name();
        FileName.replace(ptmp,"");
        FileName.replace("/","");
        jsonfile+="{";
        jsonfile+="\"name\":\"";
        jsonfile+=FileName;
        jsonfile+="\",\"size\":\"";
        jsonfile+=size;
        jsonfile+="\"";
        jsonfile+="}";
    }
    file = root.openNextFile();
  }
  
    jsonfile+="],";
    jsonfile+="\"path\":\"" + path + "\",";
    jsonfile+="\"status\":\"" + Status + "\",";
    size_t totalBytes;
    size_t usedBytes;
    totalBytes = SD.totalBytes() / (1024 * 1024);
    usedBytes = SD.usedBytes() / (1024 * 1024);
    jsonfile+="\"total\":\"" + String(totalBytes) + "\",";
    jsonfile+="\"used\":\"" + String(usedBytes) + "\",";
    jsonfile.concat(F("\"occupation\":\""));
    jsonfile+= CONFIG::intTostr(100*usedBytes/totalBytes);
    jsonfile+="\"";
    jsonfile+="}";
    path = "";
#endif//ESSP8266

        web_interface->web_server.sendHeader("Cache-Control", "no-cache");
        web_interface->web_server.send(200, "application/json", jsonfile);
        web_interface->_upload_status=UPLOAD_STATUS_NONE;
        //Serial.println(jsonfile);
        //Serial.println("path:"+path);
        //Serial.println("ptmp:"+ptmp);
}
//SD files uploader handle
void SDCardFileupload()
{
    static File fsUploadFile = (File)0;
    static String filename;
    //get authentication status
    level_authenticate_type auth_level= web_interface->is_authenticated();
    //Guest cannot upload
    if (auth_level == LEVEL_GUEST) {
        web_interface->_upload_status=UPLOAD_STATUS_FAILED;
        ESPCOM::println (F ("Upload rejected"), PRINTER_PIPE);
        pushError(ESP_ERROR_AUTHENTICATION, "Upload rejected", 401);
    } else {
        //get current file ID
        HTTPUpload& upload = (web_interface->web_server).upload();
        if ((web_interface->_upload_status != UPLOAD_STATUS_FAILED) || (upload.status == UPLOAD_FILE_START)) {
            //Upload start
            //**************
            if(upload.status == UPLOAD_FILE_START) {
                web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
                String upload_filename = upload.filename;
                String  sizeargname  = upload_filename + "S";
                if (upload_filename[0] != '/') filename = "/" + upload_filename;
                else filename = upload.filename;
                //according User or Admin the root is different as user is isolate to /user when admin has full access
                if(auth_level != LEVEL_ADMIN) {
                    upload_filename = filename;
                    filename = "/user" + upload_filename;
                }
                
            #ifdef ESP8266
            if (SDFS.exists (filename) ) {
                SDFS.remove (filename);
                #else
            if (SD.exists (filename) ) {
                SD.remove (filename);
            #endif//ESP8266
                }
                if (fsUploadFile ) {
                    fsUploadFile.close();
                }
                if ((web_interface->web_server).hasArg (sizeargname.c_str()) ) {
                    uint32_t filesize = (web_interface->web_server).arg (sizeargname.c_str()).toInt();
    #if defined ( ARDUINO_ARCH_ESP8266)
                    fs::FSInfo info;
                    SDFS.info(info);
                     uint32_t freespace = info.totalBytes- info.usedBytes;
    #endif
    #if defined ( ARDUINO_ARCH_ESP32)
                    uint32_t freespace = SD.totalBytes() - SD.usedBytes();
    #endif
                    
                    if (filesize > freespace) {
                        web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                        pushError(ESP_ERROR_NOT_ENOUGH_SPACE, "Upload rejected, not enough space");
                    }
                }
                if (web_interface->_upload_status != UPLOAD_STATUS_FAILED) {
                    //create file
                    #ifdef ESP8266
                    fsUploadFile = SDFS.open(filename, "w");
                    #else
                    fsUploadFile = SD.open(filename, FILE_WRITE);
                    #endif//ESP8266
                    
                    //check If creation succeed
                    if (fsUploadFile) {
                        //if yes upload is started
                        web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
                    } else {
                        //if no set cancel flag
                        web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                        ESPCOM::println (F ("Error ESP create"), PRINTER_PIPE);
                        pushError(ESP_ERROR_FILE_CREATION, "File creation failed");
                    }
                }
                //Upload write
                //**************
            } else if(upload.status == UPLOAD_FILE_WRITE) {
                //check if file is available and no error
                if(fsUploadFile && web_interface->_upload_status == UPLOAD_STATUS_ONGOING) {
                    //no error so write post date
                    if (upload.currentSize != fsUploadFile.write(upload.buf, upload.currentSize)){
                        web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                        ESPCOM::println (F ("Error ESP write"), PRINTER_PIPE);
                        pushError(ESP_ERROR_FILE_WRITE, "File write failed");
                        }
                } else {
                    //we have a problem set flag UPLOAD_STATUS_FAILED
                    web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                    ESPCOM::println (F ("Error ESP write"), PRINTER_PIPE);
                }
                //Upload end
                //**************
            } else if(upload.status == UPLOAD_FILE_END) {
                //check if file is still open
                if(fsUploadFile) {
                    //close it
                    fsUploadFile.close();
                    if (web_interface->_upload_status == UPLOAD_STATUS_ONGOING) {
                        web_interface->_upload_status = UPLOAD_STATUS_SUCCESSFUL;
                    }
                } else {
                    //we have a problem set flag UPLOAD_STATUS_FAILED
                    web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                    ESPCOM::println (F ("Error ESP close"), PRINTER_PIPE);
                    pushError(ESP_ERROR_FILE_CLOSE, "File close failed");
                }
                //Upload cancelled
                //**************
            } else {
                    web_interface->_upload_status = UPLOAD_STATUS_FAILED;
                    return;
                    //ESPCOM::println (F ("Error ESP upload"), PRINTER_PIPE);
                    //pushError(ESP_ERROR_UPLOAD, "File upload failed");
            }
        }
    }
    if (web_interface->_upload_status == UPLOAD_STATUS_FAILED) {
        cancelUpload();
        
            #ifdef ESP8266
            if (SDFS.exists (filename) ) {
                SDFS.remove (filename);
                #else
            if (SD.exists (filename) ) {
                SD.remove (filename);
            #endif//ESP8266
            }
    }
    CONFIG::wait(0);
}
#endif//SDCARD_FEATURE


#ifdef SDCARD_FEATURE
String webpage = "";
//void SelectInput(String heading1, String heading2, String command, String arg_calling_name);
// void SD_file_download(String filename);
// //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// void File_Download(){ // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
//   if (web_interface->web_server.args() > 0 ) { // Arguments were received
//     if (web_interface->web_server.hasArg("/SDdown")) SD_file_download(web_interface->web_server.arg(0));
    
//     Serial.println("File name:" + String(web_interface->web_server.arg(0)));
//   }
//   //else SelectInput("File Download","Enter filename to download","download","download");
// }
// //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// void SD_file_download(String filename){
//   //if (SD_present) { 
//     String Filename = "/"+filename;
//     Serial.println("File name:" + Filename);
//     File download = SD.open(Filename);
//     if (download) {
//       web_interface->web_server.sendHeader("Content-Type", "text/text");
//       web_interface->web_server.sendHeader("Content-Disposition", "attachment; filename="+filename);
//       web_interface->web_server.sendHeader("Connection", "close");
//       web_interface->web_server.streamFile(download, "application/octet-stream");
//       download.close();
//     } 
// //     else ReportFileNotPresent("download"); 
// //   } else ReportSDNotPresent();
// }

// void append_page_header() {
//   webpage  = F("<!DOCTYPE html><html>");
//   webpage += F("<head>");
//   webpage += F("<title>File Server</title>"); // NOTE: 1em = 16px
//   webpage += F("<meta name='viewport' content='user-scalable=yes,initial-scale=1.0,width=device-width'>");
//   webpage += F("<style>");
//   webpage += F("body{max-width:65%;margin:0 auto;font-family:arial;font-size:105%;text-align:center;color:blue;background-color:#F7F2Fd;}");
//   webpage += F("ul{list-style-type:none;margin:0.1em;padding:0;border-radius:0.375em;overflow:hidden;background-color:#dcade6;font-size:1em;}");
//   webpage += F("li{float:left;border-radius:0.375em;border-right:0.06em solid #bbb;}last-child {border-right:none;font-size:85%}");
//   webpage += F("li a{display: block;border-radius:0.375em;padding:0.44em 0.44em;text-decoration:none;font-size:85%}");
//   webpage += F("li a:hover{background-color:#EAE3EA;border-radius:0.375em;font-size:85%}");
//   webpage += F("section {font-size:0.88em;}");
//   webpage += F("h1{color:white;border-radius:0.5em;font-size:1em;padding:0.2em 0.2em;background:#558ED5;}");
//   webpage += F("h2{color:orange;font-size:1.0em;}");
//   webpage += F("h3{font-size:0.8em;}");
//   webpage += F("table{font-family:arial,sans-serif;font-size:0.9em;border-collapse:collapse;width:85%;}"); 
//   webpage += F("th,td {border:0.06em solid #dddddd;text-align:left;padding:0.3em;border-bottom:0.06em solid #dddddd;}"); 
//   webpage += F("tr:nth-child(odd) {background-color:#eeeeee;}");
//   webpage += F(".rcorners_n {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:20%;color:white;font-size:75%;}");
//   webpage += F(".rcorners_m {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:50%;color:white;font-size:75%;}");
//   webpage += F(".rcorners_w {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:70%;color:white;font-size:75%;}");
//   webpage += F(".column{float:left;width:50%;height:45%;}");
//   webpage += F(".row:after{content:'';display:table;clear:both;}");
//   webpage += F("*{box-sizing:border-box;}");
//   webpage += F("footer{background-color:#eedfff; text-align:center;padding:0.3em 0.3em;border-radius:0.375em;font-size:60%;}");
//   webpage += F("button{border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:20%;color:white;font-size:130%;}");
//   webpage += F(".buttonsm{border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:9%; color:white;font-size:70%;}");
//   webpage += F(".buttonm {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:15%;color:white;font-size:70%;}");
//   webpage += F(".buttonw {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:40%;color:white;font-size:70%;}");
//   webpage += F("a{font-size:75%;}");
//   webpage += F("p{font-size:75%;}");
//   webpage += F("</style></head><body><h1>File Server "); webpage += String(ServerVersion) + "</h1>";
// }
// //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// void append_page_footer(){ // Saves repeating many lines of code for HTML page footers
//   webpage += F("<ul>");
//   webpage += F("<li><a href='/'>Home</a></li>"); // Lower Menu bar command entries
//   webpage += F("<li><a href='/download'>Download</a></li>"); 
//   webpage += F("</ul>");
//   webpage += "<footer>&copy;"+String(char(byte(0x40>>1)))+String(char(byte(0x88>>1)))+String(char(byte(0x5c>>1)))+String(char(byte(0x98>>1)))+String(char(byte(0x5c>>1)));
//   webpage += String(char((0x84>>1)))+String(char(byte(0xd2>>1)))+String(char(0xe4>>1))+String(char(0xc8>>1))+String(char(byte(0x40>>1)));
//   webpage += String(char(byte(0x64/2)))+String(char(byte(0x60>>1)))+String(char(byte(0x62>>1)))+String(char(0x70>>1))+"</footer>";
//   webpage += F("</body></html>");
// }
// //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// void SendHTML_Header(){
//   server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
//   server.sendHeader("Pragma", "no-cache"); 
//   server.sendHeader("Expires", "-1"); 
//   server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
//   server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves. 
//   append_page_header();
//   server.sendContent(webpage);
//   webpage = "";
// }
// //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// void SendHTML_Content(){
//   web_interface->web_server.sendContent(webpage);
//   webpage = "";
// }
// //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// void SendHTML_Stop(){
//   web_interface->web_server.sendContent("");
//   web_interface->web_server.client().stop(); // Stop is needed because no content length was sent
// }
// //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// void SelectInput(String heading1, String heading2, String command, String arg_calling_name){
//   SendHTML_Header();
//   webpage += F("<h3 class='rcorners_m'>");webpage += heading1+"</h3><br>";
//   webpage += F("<h3>"); webpage += heading2 + "</h3>"; 
//   webpage += F("<FORM action='/"); webpage += command + "' method='post'>"; // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
//   webpage += F("<input type='text' name='"); webpage += arg_calling_name; webpage += F("' value=''><br>");
//   webpage += F("<type='submit' name='"); webpage += arg_calling_name; webpage += F("' value=''><br><br>");
//   append_page_footer();
//   SendHTML_Content();
//   SendHTML_Stop();
// }
#endif//SDCARD_FEATURE
//FW update using Web interface/////////////////////////////////////////
#ifdef WEB_UPDATE_FEATURE
void WebUpdateUpload()
{
    #ifdef PLC_MASTER_UI
    webPLC.connectWeb(0);
    webPLC.UpdateFW(1);
    #endif//PLC_MASTER_UI
    static size_t last_upload_update;
    static uint32_t maxSketchSpace ;
    //only admin can update FW
    if(web_interface->is_authenticated() != LEVEL_ADMIN) {
        web_interface->_upload_status=UPLOAD_STATUS_FAILED;
        ESPCOM::println (F ("Update failed"), PRINTER_PIPE);
        log_esp3d("Web Update failed");
        pushError(ESP_ERROR_AUTHENTICATION, "Upload rejected",401);
    } else {
        //get current file ID
        HTTPUpload& upload = (web_interface->web_server).upload();
        if ((web_interface->_upload_status != UPLOAD_STATUS_FAILED) || (upload.status == UPLOAD_FILE_START)) {
            //Upload start
            //**************
            if(upload.status == UPLOAD_FILE_START) {
                ESPCOM::println (F ("Update Firmware"), PRINTER_PIPE);
                web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
                String  sizeargname  = upload.filename + "S";
                
    #if defined ( ARDUINO_ARCH_ESP8266)
                WiFiUDP::stopAll();
    #endif
                size_t flashsize = 0;
    #if defined ( ARDUINO_ARCH_ESP8266)
                maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    #else 
                if (esp_ota_get_running_partition()) {
                    const esp_partition_t* partition = esp_ota_get_next_update_partition(NULL);
                    if (partition) {
                        maxSketchSpace = partition->size;
                    }
                }  
    #endif
                if ((web_interface->web_server).hasArg (sizeargname.c_str()) ) {
                    flashsize = (web_interface->web_server).arg (sizeargname).toInt();
                } else {
                    flashsize = maxSketchSpace;
                }
                if ((flashsize > maxSketchSpace) || (flashsize == 0)) {
                    web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                    pushError(ESP_ERROR_NOT_ENOUGH_SPACE, "Upload rejected");
                }
                if (web_interface->_upload_status != UPLOAD_STATUS_FAILED) {
                    last_upload_update = 0;
                    if(!Update.begin(maxSketchSpace)) { //start with max available size
                        web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                        pushError(ESP_ERROR_NOT_ENOUGH_SPACE, "Upload rejected");
                    } else {
                        if (( CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER)) ESPCOM::println (F ("Update 0%%"), PRINTER_PIPE);
                        else ESPCOM::println (F ("Update 0%"), PRINTER_PIPE);
                    }
                }
                //Upload write
                //**************
            } else if(upload.status == UPLOAD_FILE_WRITE) {
                //check if no error
                if (web_interface->_upload_status == UPLOAD_STATUS_ONGOING) {
                    //we do not know the total file size yet but we know the available space so let's use it
                    if ( ((100 * upload.totalSize) / maxSketchSpace) !=last_upload_update) {
                        last_upload_update = (100 * upload.totalSize) / maxSketchSpace;
                        String s = "Update ";
                        s+= String(last_upload_update);
                        s+= "%";
                        if (( CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER)) s+= "%";
                        ESPCOM::println (s.c_str(), PRINTER_PIPE);
                    }
                    if(Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                        web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                        pushError(ESP_ERROR_FILE_WRITE, "File write failed");
                    }
                }
                //Upload end
                //**************
            } else if(upload.status == UPLOAD_FILE_END) {
                if(Update.end(true)) { //true to set the size to the current progress
                    //Now Reboot
                    if (( CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER)) ESPCOM::println (F("Update 100%%"), PRINTER_PIPE);
                    else ESPCOM::println (F("Update 100%"), PRINTER_PIPE);
                    web_interface->_upload_status=UPLOAD_STATUS_SUCCESSFUL;
                }
            } else if(upload.status == UPLOAD_FILE_ABORTED) {
                ESPCOM::println (F("Update Failed"), PRINTER_PIPE);
                web_interface->_upload_status=UPLOAD_STATUS_FAILED;
                //pushError(ESP_ERROR_UPLOAD_CANCELLED, "Upload cancelled");
            }
        }
    }
    
    if (web_interface->_upload_status==UPLOAD_STATUS_FAILED) {
        cancelUpload();
        Update.end();
    }
    
    CONFIG::wait(0);
}

void handleUpdate()
{
    level_authenticate_type auth_level = web_interface->is_authenticated();
    if (auth_level != LEVEL_ADMIN) {
        web_interface->_upload_status=UPLOAD_STATUS_NONE;
        web_interface->web_server.send(403,"text/plain","Not allowed, log in first!\n");
        return;
    }
    String jsonfile = "{\"status\":\"" ;
    jsonfile+=CONFIG::intTostr(web_interface->_upload_status);
    jsonfile+="\"}";
    //send status
    web_interface->web_server.sendHeader("Cache-Control", "no-cache");
    web_interface->web_server.send(200, "application/json", jsonfile);
    //if success restart
    if (web_interface->_upload_status==UPLOAD_STATUS_SUCCESSFUL) {
        CONFIG::wait(2000);
        web_interface->restartmodule=true;
    } else {
        web_interface->_upload_status=UPLOAD_STATUS_NONE;
    }
}
#endif

//Handle not registred path on SPIFFS neither SD ///////////////////////
void handle_not_found()
{
    static const char NOT_AUTH_NF [] PROGMEM = "HTTP/1.1 301 OK\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";

    if (web_interface->is_authenticated() == LEVEL_GUEST) {
        web_interface->web_server.sendContent_P(NOT_AUTH_NF);
        return;
    }
    bool page_not_found = false;
    String path = web_interface->web_server.urlDecode(web_interface->web_server.uri());
    String contentType =  web_interface->getContentType(path);
    String pathWithGz = path + ".gz";
    log_esp3d("Not found %s, type %s", path.c_str(), contentType.c_str());
    if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
        if(SPIFFS.exists(pathWithGz)) {
            path = pathWithGz;
        }
        FS_FILE file = SPIFFS.open(path, SPIFFS_FILE_READ);
        web_interface->web_server.streamFile(file, contentType);
        file.close();
        return;
    } else {
        page_not_found = true;
    }

    if (page_not_found ) {
#ifdef CAPTIVE_PORTAL_FEATURE
        if (WiFi.getMode()!=WIFI_STA ) {
            String contentType=FPSTR(PAGE_CAPTIVE);
            String stmp = WiFi.softAPIP().toString();
            //Web address = ip + port
            String KEY_IP = F("$WEB_ADDRESS$");
            String KEY_QUERY = F("$QUERY$");
            if (wifi_config.iweb_port!=80) {
                stmp+=":";
                stmp+=CONFIG::intTostr(wifi_config.iweb_port);
            }
            contentType.replace(KEY_IP,stmp);
            contentType.replace(KEY_IP,stmp);
            contentType.replace(KEY_QUERY,web_interface->web_server.uri());
            web_interface->web_server.send(200,"text/html",contentType);
            //web_interface->web_server.sendContent_P(NOT_AUTH_NF);
            return;
        }
#endif
        log_esp3d("Page not found");
        path = F("/404.htm");
        contentType =  web_interface->getContentType(path);
        pathWithGz =  path + F(".gz");
        if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
            if(SPIFFS.exists(pathWithGz)) {
                path = pathWithGz;
            }
            FS_FILE file = SPIFFS.open(path, SPIFFS_FILE_READ);
            web_interface->web_server.streamFile(file, contentType);
            file.close();

        } else {
            //if not template use default page
            contentType=FPSTR(PAGE_404);
            String stmp;
            if (WiFi.getMode()==WIFI_STA ) {
                stmp=WiFi.localIP().toString();
            } else {
                stmp=WiFi.softAPIP().toString();
            }
            //Web address = ip + port
            String KEY_IP = F("$WEB_ADDRESS$");
            String KEY_QUERY = F("$QUERY$");
            if (wifi_config.iweb_port!=80) {
                stmp+=":";
                stmp+=CONFIG::intTostr(wifi_config.iweb_port);
            }
            contentType.replace(KEY_IP,stmp);
            contentType.replace(KEY_QUERY,web_interface->web_server.uri());
            web_interface->web_server.send(200,"text/html",contentType);
        }
    }
}

//Handle web command query and send answer /////////////////////////////
void handle_web_command()
{
    level_authenticate_type auth_level= web_interface->is_authenticated();
    /*  if (auth_level == LEVEL_GUEST) {
          web_interface->web_server.send(403,"text/plain","Not allowed, log in first!\n");
          return;
      }*/
    String buffer2send = "";
    ESPResponseStream espresponse;
    String cmd = "";
    if (web_interface->web_server.hasArg("plain") || web_interface->web_server.hasArg("commandText")) {
        if (web_interface->web_server.hasArg("plain")) {
            cmd = web_interface->web_server.arg("plain");
        } else {
            cmd = web_interface->web_server.arg("commandText");
        }
        log_esp3d("WebCommand %s",cmd.c_str());
    } else {
        log_esp3d("Invalid arg");
        web_interface->web_server.send(200,"text/plain","Invalid command");
        return;
    }
    //if it is for ESP module [ESPXXX]<parameter>
    cmd.trim();
    int ESPpos = cmd.indexOf("[ESP");
    if (ESPpos==0) {
        //is there the second part?
        int ESPpos2 = cmd.indexOf("]",ESPpos);
        if (ESPpos2>-1) {
            //Split in command and parameters
            String cmd_part1=cmd.substring(ESPpos+4,ESPpos2);
            String cmd_part2="";
            //only [ESP800] is allowed login free if authentication is enabled
            if ((auth_level == LEVEL_GUEST)  && (cmd_part1.toInt()!=800)) {
                web_interface->web_server.send(401,"text/plain","Authentication failed!\n");
                return;
            }
            //is there space for parameters?
            if ((uint)ESPpos2<cmd.length()) {
                cmd_part2=cmd.substring(ESPpos2+1);
            }
            //if command is a valid number then execute command
            if(cmd_part1.toInt()!=0) {
                COMMAND::execute_command(cmd_part1.toInt(), cmd_part2, WEB_PIPE, auth_level, &espresponse);
                ESPCOM::flush(WEB_PIPE, &espresponse);
            }
            //if not is not a valid [ESPXXX] command
        }
    } else {
        if (auth_level == LEVEL_GUEST) {
            web_interface->web_server.send(401,"text/plain","Authentication failed!\n");
            return;
        }
        //send command to serial as no need to transfer ESP command
        //to avoid any pollution if Uploading file to SDCard
        if ((web_interface->blockserial) == false) {  
            //block every query
            web_interface->blockserial = true;
            log_esp3d("Block Serial");
            //empty the serial buffer and incoming data
            log_esp3d("Start PurgeSerial");
            if(ESPCOM::available(DEFAULT_PRINTER_PIPE)) {
                ESPCOM::bridge();
                CONFIG::wait(1);
            }
            log_esp3d("End PurgeSerial");
            web_interface->web_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
            web_interface->web_server.sendHeader("Content-Type","text/plain",true);
            web_interface->web_server.sendHeader("Cache-Control","no-cache");
            web_interface->web_server.send(200);
            //send command
            log_esp3d("Start PurgeSerial");
            if(ESPCOM::available(DEFAULT_PRINTER_PIPE)) {
                ESPCOM::bridge();
                CONFIG::wait(1);
            }
            log_esp3d("End PurgeSerial");
            ESPCOM::println (cmd, DEFAULT_PRINTER_PIPE);
            bool done = false;
            String current_buffer;
            String current_line;
            //int pos;
            int temp_counter = 0;
            String tmp;
            bool datasent = false;
            uint32_t timeout = millis();
            //pickup the list
            while ((millis() - timeout < 2000) && !done) {
                //give some time between each buffer
                if (ESPCOM::available(DEFAULT_PRINTER_PIPE)) {
                    log_esp3d("Got data");
                    timeout = millis();
                    size_t len = ESPCOM::available(DEFAULT_PRINTER_PIPE);
                    uint8_t sbuf[len+1];
                    //read buffer
                    ESPCOM::readBytes (DEFAULT_PRINTER_PIPE, sbuf, len);
                    //change buffer as string
                    sbuf[len]='\0';
                    //add buffer to current one if any
                    current_buffer += (char * ) sbuf;
                    while (current_buffer.indexOf("\n") !=-1) {
                        log_esp3d("Remove new line");
                        //remove the possible "\r"
                        current_buffer.replace("\r","");
                        //get line
                        current_line = current_buffer.substring(0,current_buffer.indexOf("\n"));
                        //if line is command ack - just exit so save the time out period
                        if ((current_line == "ok") || (current_line == "wait") || (current_line.startsWith("ok") && !((CONFIG::GetFirmwareTarget() == REPETIER) || (CONFIG::GetFirmwareTarget() == REPETIER4DV)))) {
                            done = true;
                            buffer2send +=current_line;
                            log_esp3d("Found ok/wait add New buffer %s", buffer2send.c_str());
                            buffer2send +="\n";
                            break;
                        }
                        //get the line and transmit it
                        //check command
                        if ((CONFIG::GetFirmwareTarget() == REPETIER) || (CONFIG::GetFirmwareTarget() == REPETIER4DV)) {
                            //save time no need to continue
                            if (current_line.indexOf("busy:") > -1) {
                                temp_counter++;
                            } 
                            else if (COMMAND::check_command(current_line, NO_PIPE, false)) {
                               temp_counter ++ ;
                            }
                        } else {
                            if (COMMAND::check_command(current_line, NO_PIPE, false)) {
                               temp_counter ++ ;
                            }
                        }
                        if (temp_counter > 5) {
                            log_esp3d("Timeout X5");
                            done = true;
                            break;
                        }
                        if ((CONFIG::GetFirmwareTarget()  == REPETIER) || (CONFIG::GetFirmwareTarget() == REPETIER4DV)) {
                            if (!current_line.startsWith( "ok ")) {
                                buffer2send +=current_line;
                                buffer2send +="\n";
                            }
                        } else {
                            buffer2send +=current_line;
                            log_esp3d("New buffer %s", buffer2send.c_str());
                            buffer2send +="\n";
                        }
                        if (buffer2send.length() > 1200) {
                            web_interface->web_server.sendContent(buffer2send);
                           log_esp3d("Sending %s", buffer2send.c_str());
                            buffer2send = "";
                            datasent = true;
                        }
                        //current remove line from buffer
                        tmp = current_buffer.substring(current_buffer.indexOf("\n")+1,current_buffer.length());
                        current_buffer = tmp;
                        CONFIG::wait (0);
                    }
                    CONFIG::wait (0);
                } else {
                    CONFIG::wait(1);
                }
                //it is sending too many temp status should be heating so let's exit the loop
                if (temp_counter > 5) {
                    done = true;
                }
            }
            log_esp3d("Finished");
            //to be sure connection close
            if (buffer2send.length() > 0) {
                web_interface->web_server.sendContent(buffer2send);
                log_esp3d("Sending %s", buffer2send.c_str());
                datasent = true;
            }
            if (!datasent) {
                web_interface->web_server.sendContent(" \r\n");
            }
            web_interface->web_server.sendContent("");
            log_esp3d("Start PurgeSerial");
            if(ESPCOM::available(DEFAULT_PRINTER_PIPE)) {
                ESPCOM::bridge();
                CONFIG::wait(1);
            }
            log_esp3d("End PurgeSerial");
            web_interface->blockserial = false;
            log_esp3d("Release PurgeSerial");
        } else {
            web_interface->web_server.send(200,"text/plain","Serial is busy, retry later!");
        }
    }
}

//Handle web command query and sent ack or fail instead of answer //////
void handle_web_command_silent()
{
    level_authenticate_type auth_level= web_interface->is_authenticated();
    if (auth_level == LEVEL_GUEST) {
        web_interface->web_server.send(401,"text/plain","Authentication failed!\n");
        return;
    }
    String buffer2send = "";
    String cmd = "";
    //int count ;
    if (web_interface->web_server.hasArg("plain") || web_interface->web_server.hasArg("commandText")) {
        if (web_interface->web_server.hasArg("plain")) {
            cmd = web_interface->web_server.arg("plain");
        } else {
            cmd = web_interface->web_server.arg("commandText");
        }
        log_esp3d("Web Command:%s", cmd.c_str());
    } else {
        log_esp3d("Invalid argument");
        web_interface->web_server.send(200,"text/plain","Invalid command");
        return;
    }
    //if it is for ESP module [ESPXXX]<parameter>
    cmd.trim();
    int ESPpos = cmd.indexOf("[ESP");
    if (ESPpos==0) {
        //is there the second part?
        int ESPpos2 = cmd.indexOf("]",ESPpos);
        if (ESPpos2>-1) {
            //Split in command and parameters
            String cmd_part1=cmd.substring(ESPpos+4,ESPpos2);
            String cmd_part2="";
            //is there space for parameters?
            if ((uint)ESPpos2<cmd.length()) {
                cmd_part2=cmd.substring(ESPpos2+1);
            }
            //if command is a valid number then execute command
            if(cmd_part1.toInt()!=0) {
                if (COMMAND::execute_command(cmd_part1.toInt(),cmd_part2,NO_PIPE, auth_level)) {
                   web_interface->web_server.send(200,"text/plain","ok");
                } else {
                   web_interface->web_server.send(500,"text/plain","error");
                }

            }
            //if not is not a valid [ESPXXX] command
        }
    } else {
        //send command to serial as no need to transfer ESP command
        //to avoid any pollution if Uploading file to SDCard
        if ((web_interface->blockserial) == false) {
            //send command
            ESPCOM::println (cmd, DEFAULT_PRINTER_PIPE);
            web_interface->web_server.send(200,"text/plain","ok");
        } else {
            web_interface->web_server.send(200,"text/plain","Serial is busy, retry later!");
        }
    }

}


//Serial SD files list//////////////////////////////////////////////////
void handle_serial_SDFileList()
{
#ifndef USE_AS_UPDATER_ONLY
    //this is only for admin an user
    if (web_interface->is_authenticated() == LEVEL_GUEST) {
        web_interface->_upload_status=UPLOAD_STATUS_NONE;
        web_interface->web_server.sendHeader("Cache-Control", "no-cache");
        web_interface->web_server.send(401, "application/json", "{\"status\":\"Authentication failed!\"}");
        return;
    }
    
    log_esp3d("Serial SD upload done");
    String Status="Ok";
    if ((web_interface->_upload_status == UPLOAD_STATUS_FAILED) || (web_interface->_upload_status == UPLOAD_STATUS_FAILED)) {
        Status = "Upload failed";
        web_interface->_upload_status = UPLOAD_STATUS_NONE;
    }
    String jsonfile = "{\"status\":\"" + Status + "\"}";
    web_interface->web_server.sendHeader("Cache-Control", "no-cache");
    web_interface->web_server.send(200, "application/json", jsonfile);
    web_interface->blockserial = false;
    web_interface->_upload_status=UPLOAD_STATUS_NONE;
#endif //USE_AS_UPDATER_ONLY
}

#define NB_RETRY 5
#define MAX_RESEND_BUFFER 228
#define SERIAL_CHECK_TIMEOUT 2000
//SD file upload by serial
void SDFile_serial_upload()
{
#ifndef USE_AS_UPDATER_ONLY
    static int32_t lineNb =-1;
    static String current_line;
    static bool is_comment = false;
    static String current_filename;
    String response;
    //Guest cannot upload - only admin and user
    if(web_interface->is_authenticated() == LEVEL_GUEST) {
        web_interface->_upload_status=UPLOAD_STATUS_FAILED;
        ESPCOM::println (F ("SD upload rejected"), PRINTER_PIPE);
        pushError(ESP_ERROR_AUTHENTICATION, "Upload rejected", 401);
        log_esp3d("SD upload rejected");
    } else {
        //retrieve current file id
        HTTPUpload& upload = (web_interface->web_server).upload();
        if((web_interface->_upload_status != UPLOAD_STATUS_FAILED) || (upload.status == UPLOAD_FILE_START)) {
            //Upload start
            //**************
            if(upload.status == UPLOAD_FILE_START) {
                web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
                log_esp3d("Upload start");
                String command = "M29";
                String resetcmd = "M110 N0";
                if (CONFIG::GetFirmwareTarget() == SMOOTHIEWARE)resetcmd = "N0 M110";
                lineNb=1;
                //close any ongoing upload and get current line number
                if(!sendLine2Serial (command,-1, &lineNb)){
                    //it can failed for repetier
                    if ( ( CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER) ) {
                        if(!sendLine2Serial (command,1, NULL)){
                            log_esp3d("Upload start failed");
                            web_interface->_upload_status= UPLOAD_STATUS_FAILED;
                            pushError(ESP_ERROR_START_UPLOAD, "Upload rejected");
                        }
                    } else {
                        log_esp3d("Upload start failed");
                        web_interface->_upload_status= UPLOAD_STATUS_FAILED;
                        pushError(ESP_ERROR_START_UPLOAD, "Upload rejected");
                    }
                } else {
                    //Mount SD card
                    command = "M21";
                    if(!sendLine2Serial (command,-1, NULL)){
                        log_esp3d("Mounting SD failed");
                        web_interface->_upload_status= UPLOAD_STATUS_FAILED;
                        pushError(ESP_ERROR_MOUNT_SD, "Mounting SD failed");
                    }
                    if (web_interface->_upload_status != UPLOAD_STATUS_FAILED) {
                        //Reset line numbering
                        if(!sendLine2Serial (resetcmd,-1, NULL)){
                            log_esp3d("Reset Numbering failed");
                            web_interface->_upload_status= UPLOAD_STATUS_FAILED;
                            pushError(ESP_ERROR_RESET_NUMBERING, "Reset Numbering failed");
                        }
                    }
                    if (web_interface->_upload_status != UPLOAD_STATUS_FAILED) {
                        lineNb=1;
                        //need to lock serial out to avoid garbage in file
                        (web_interface->blockserial) = true;
                        current_line ="";
                        current_filename = upload.filename;
                        is_comment = false;
                        String response;
                        ESPCOM::println (F ("Uploading..."), PRINTER_PIPE);
                        //Clear all serial
                        ESPCOM::flush (DEFAULT_PRINTER_PIPE);
                        purge_serial();
                        //besure nothing left again
                        purge_serial();
                        command = "M28 " + upload.filename;
                        //send start upload
                        //no correction allowed because it means reset numbering was failed
                        if (sendLine2Serial(command, lineNb, NULL)){
                            CONFIG::wait(1200);
                            //additional purge, in case it is slow to answer
                            purge_serial();
                            web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
                            log_esp3d("Creation Ok");
                            
                        } else  {
                            web_interface->_upload_status= UPLOAD_STATUS_FAILED;
                            log_esp3d("Creation failed");
                            pushError(ESP_ERROR_FILE_CREATION, "File creation failed");
                        }
                    }
                }
                //Upload write
                //**************
                //upload is on going with data coming by 2K blocks
            } else if(upload.status == UPLOAD_FILE_WRITE) { //if com error no need to send more data to serial
                for (uint pos = 0;( pos < upload.currentSize) && (web_interface->_upload_status == UPLOAD_STATUS_ONGOING); pos++) { //parse full post data
                    //feed watchdog
                    CONFIG::wait(0);
                    //it is a comment
                    if (upload.buf[pos] == ';') {
                        log_esp3d("Comment found");
                        is_comment = true;
                    }
                    //it is an end line
                    else  if ( (upload.buf[pos] == 13) || (upload.buf[pos] == 10) ) {
                        //if comment line then reset
                        is_comment = false;
                        //does line fit the buffer ?
                        if (current_line.length() < MAX_RESEND_BUFFER) {
                            //do we have something in buffer ?
                            if (current_line.length() > 0 ) {
                                lineNb++;
                                if (!sendLine2Serial (current_line, lineNb, NULL) ) {
                                    log_esp3d("Error sending line");
                                    CloseSerialUpload (true, current_filename,lineNb);
                                    web_interface->_upload_status= UPLOAD_STATUS_FAILED;
                                    pushError(ESP_ERROR_FILE_WRITE, "File write failed");
                                }
                                //reset line
                                current_line = "";

                            } else {
                                log_esp3d ("Empy line");
                            }
                        } else {
                            //error buffer overload
                            log_esp3d ("Error over buffer(1)");
                            lineNb++;
                            web_interface->_upload_status= UPLOAD_STATUS_FAILED;
                            pushError(ESP_ERROR_BUFFER_OVERFLOW, "Error buffer overflow");
                        }
                    } else if (!is_comment) {
                        if (current_line.length() < MAX_RESEND_BUFFER) {
                            current_line += char (upload.buf[pos]);  //copy current char to buffer to send/resend
                        } else {
                            log_esp3d ("Error over buffer(2)");
                            lineNb++;
                            web_interface->_upload_status= UPLOAD_STATUS_FAILED;
                            pushError(ESP_ERROR_BUFFER_OVERFLOW, "Error buffer overflow");
                        }
                    }
                }
                //Upload end
                //**************
            } else if(upload.status == UPLOAD_FILE_END && web_interface->_upload_status == UPLOAD_STATUS_ONGOING) {
                //if last part does not have '\n'
                if (current_line.length()  > 0) {
                    lineNb++;
                    if (!sendLine2Serial (current_line, lineNb, NULL) ) {
                        log_esp3d ("Error sending buffer");
                        lineNb++;
                        CloseSerialUpload (true, current_filename, lineNb);
                        web_interface->_upload_status= UPLOAD_STATUS_FAILED;
                        pushError(ESP_ERROR_FILE_WRITE, "File write failed");
                    }
                }
                log_esp3d ("Upload finished");
                lineNb++;
                CloseSerialUpload (false, current_filename, lineNb);
                //Upload cancelled
                //**************
            } else { //UPLOAD_FILE_ABORTED
                log_esp3d("Error, Something happened");
                web_interface->_upload_status= UPLOAD_STATUS_FAILED;
                //pushError(ESP_ERROR_UPLOAD_CANCELLED, "Upload cancelled");
            }
        }
    }
    
    if (web_interface->_upload_status == UPLOAD_STATUS_FAILED) {
        ESPCOM::println (F ("Upload failed"), PRINTER_PIPE);
        lineNb++;
        CloseSerialUpload (true, current_filename, lineNb);
        cancelUpload();
    }
#endif //USE_AS_UPDATER_ONLY
}

#endif

