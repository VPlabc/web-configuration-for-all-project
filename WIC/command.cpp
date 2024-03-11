/*
  command.cpp - ESP3D configuration class

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
#include "config.h"
#include "command.h"
#include "wificonf.h"
#include "webinterface.h"
#include "FirmwareUpdate.h"
UpdateFW UDFWCmd;
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#include <FS.h>
#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#define MAX_GPIO 37
int ChannelAttached2Pin[16]= {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
#else
#define MAX_GPIO 16
#endif
#ifdef TIMESTAMP_FEATURE
#include <time.h>
#endif
#ifdef ESP_OLED_FEATURE
#include "esp_oled.h"
#endif

#ifdef DHT_FEATURE
#include "DHTesp.h"
extern DHTesp dhts;
#endif

#ifdef NOTIFICATION_FEATURE
#include "notifications_service.h"
#endif

#ifdef MQTT_USE
#include "MQTTcom.h"
MQTTCOM commandMQTT;
#endif//MQTT_USER
#ifdef PLC_MASTER_UI
#include "PLC_IoT/PLC_Master.h"
PLC_MASTER PLC_cmd;
#include "Modbus_RTU.h"
Modbus_Prog cmd_modbus;
#ifdef USE_LORA
#ifndef Lora_rf
#define Lora_rf
// #include "PLC_IoT/LoRa.h"
#endif//Lora_rf
#endif//USE_LORA
#endif//PLC_MASTER_UI
#ifdef Valve_UI
#include "Valve/LoRaFunc.h"
#include "Valve/Valve.h"
#endif//Valve_UI
#ifdef LOOKLINE_UI
#include "WIC.h"
WIC cmdWic;
#include "LookLine/LookLine.h"
LOOKLINE_PROG LooklineCMD;
#endif//ValveLOOKLINE_UI_UI
String COMMAND::buffer_serial;
String COMMAND::buffer_tcp;
byte auth_val = 0;

byte LOCK = LEVEL_GUEST;
#define ERROR_CMD_MSG (output == WEB_PIPE)?F("Error: Wrong Command"):F("Cmd Error")
#define INCORRECT_CMD_MSG (output == WEB_PIPE)?F("Error: Incorrect Command"):F("Incorrect Cmd")
#define OK_CMD_MSG (output == WEB_PIPE)?F("ok"):F("Cmd Ok")

extern uint8_t Checksum(const char * line, uint16_t lineSize);
extern bool sendLine2Serial (String &  line, int32_t linenb, int32_t* newlinenb);


void setAuth(byte Auth){
    LOCK = Auth;
}
// ESPResponseStream  *espresponse;
void SendMsg(String msg){
    ESPCOM::println (msg, WEB_PIPE);
    // LOGLN("COMMAND:" + msg);
}
const char * encodeString(const char * s){
    static String tmp;
    tmp = s;
    while(tmp.indexOf("'")!=-1)tmp.replace("'", "&#39;");
    while(tmp.indexOf("\"")!=-1)tmp.replace("\"", "&#34;");
    if (tmp =="") tmp=" ";
    return tmp.c_str();
}

bool isValidNumber(String str)
{
   if(!(str.charAt(0) == '+' || str.charAt(0) == '-' || isDigit(str.charAt(0)))) return false;

   for(byte i=1;i<str.length();i++)
   {
       if(!(isDigit(str.charAt(i)) || str.charAt(i) == '.')) return false;
   }
   return true;
}

String COMMAND::get_param (String & cmd_params, const char * id, bool withspace)
{
    static String parameter;
    String sid = id;
    int start;
    int end = -1;
    parameter = "";
    //if no id it means it is first part of cmd
    if (strlen (id) == 0) {
        start = 0;
    }
    //else find id position
    else {
        start = cmd_params.indexOf (id);
    }
    //if no id found and not first part leave
    if (start == -1 ) {
        return parameter;
    }
    if (start >0){
        if (cmd_params[start-1]!=' ') return parameter;
    }
    //password and SSID can have space so handle it
    //if no space expected use space as delimiter
    if (!withspace) {
        end = cmd_params.indexOf (" ", start);
    }
#ifdef AUTHENTICATION_FEATURE
    //if space expected only one parameter but additional password may be present
    else if (sid != " pwd=") {
        end = cmd_params.indexOf (" pwd=", start);
    }
#endif
    //if no end found - take all
    if (end == -1) {
        end = cmd_params.length();
    }
    //extract parameter
    parameter = cmd_params.substring (start + strlen (id), end);
    //be sure no extra space
    parameter.trim();
    return parameter;
}
#ifdef AUTHENTICATION_FEATURE
//check admin password
bool COMMAND::isadmin (String & cmd_params)
{
    String adminpassword;
    String sadminPassword;
    if (!CONFIG::read_string (EP_ADMIN_PWD, sadminPassword, MAX_LOCAL_PASSWORD_LENGTH) ) {
        LOG ("ERROR getting admin\r\n")
        sadminPassword = FPSTR (DEFAULT_ADMIN_PWD);
    }
    adminpassword = get_param (cmd_params, "pwd=", true);
    if (!sadminPassword.equals (adminpassword) ) {
        LOG("Not identified from command line\r\n")
        return false;
    } else {
        return true;
    }
}
//check user password - admin password is also valid
bool COMMAND::isuser (String & cmd_params)
{
    String userpassword;
    String suserPassword;
    if (!CONFIG::read_string (EP_USER_PWD, suserPassword, MAX_LOCAL_PASSWORD_LENGTH) ) {
        LOG ("ERROR getting user\r\n")
        suserPassword = FPSTR (DEFAULT_USER_PWD);
    }
    userpassword = get_param (cmd_params, "pwd=", true);
    //it is not user password
    if (!suserPassword.equals (userpassword) ) {
        //check admin password
        return COMMAND::isadmin (cmd_params);
    } else {
        return true;
    }
}
#endif
bool COMMAND::execute_command (int cmd, String cmd_params, tpipe output, level_authenticate_type auth_level, ESPResponseStream  *espresponse)
{
    bool response = true;
    level_authenticate_type auth_type = auth_level;
    (void)auth_type;  //avoid warning if updater only
#ifdef AUTHENTICATION_FEATURE

    if (isadmin(cmd_params)) {
        auth_type = LEVEL_ADMIN;
        LOG("you are Admin\r\n");
    }
    if (isuser (cmd_params) && (auth_type != LEVEL_ADMIN) ) {
        auth_type = LEVEL_USER;
        LOG("you are User\r\n");
    }
#ifdef DEBUG_ESP3D
    if ( auth_type == LEVEL_ADMIN) {
        LOG("admin identified\r\n");
    } else  {
        if( auth_type == LEVEL_USER) {
            LOG("user identified\r\n");
        } else {
            LOG("guest identified\r\n");
        }
    }
#endif
#endif
    //manage parameters
    byte mode = 254;
    String parameter;
    //LOG ("\nExecute Command\r\n")
    switch (cmd) {

    //STA SSID
    //[ESP100]<SSID>[pwd=<admin password>]
    case 100:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isSSIDValid (parameter.c_str() ) ) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
        } else
#endif
            if (!CONFIG::write_string (EP_STA_SSID, parameter.c_str() ) ) {
                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                response = false;
            } else {
                ESPCOM::println (OK_CMD_MSG, output, espresponse);
            }
        break;
    //STA Password
    //[ESP101]<Password>[pwd=<admin password>]
    case 101:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isPasswordValid (parameter.c_str() ) ) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
            if (!CONFIG::write_string (EP_STA_PASSWORD, parameter.c_str() ) ) {
                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                response = false;
            } else {
                ESPCOM::println (OK_CMD_MSG, output, espresponse);
            }
        break;
    //Hostname
    //[ESP102]<hostname>[pwd=<admin password>]
    case 102:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isHostnameValid (parameter.c_str() ) ) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
            if (!CONFIG::write_string (EP_HOSTNAME, parameter.c_str() ) ) {
                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                response = false;
            } else {
                ESPCOM::println (OK_CMD_MSG, output, espresponse);
            }
        break;
    //Wifi mode (STA/AP)
    //[ESP103]<mode>[pwd=<admin password>]
    case 103:
        parameter = get_param (cmd_params, "", true);
        if (parameter == "STA") {
            mode = CLIENT_MODE;
        } else if (parameter == "AP") {
            mode = AP_MODE;
        } else {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
        if ( (mode == CLIENT_MODE) || (mode == AP_MODE) ) {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            } else if (auth_type != LEVEL_USER) {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            } else
#endif
                if (!CONFIG::write_byte (EP_WIFI_MODE, mode) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    response = false;
                } else {
                    ESPCOM::println (OK_CMD_MSG, output, espresponse);
                }
        }
        break;
    //STA IP mode (DHCP/STATIC)
    //[ESP104]<mode>[pwd=<admin password>]
    case 104:
        parameter = get_param (cmd_params, "", true);
        if (parameter == "STATIC") {
            mode = STATIC_IP_MODE;
        } else if (parameter == "DHCP") {
            mode = DHCP_MODE;
        } else {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
        if ( (mode == STATIC_IP_MODE) || (mode == DHCP_MODE) ) {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            } else
#endif
                if (!CONFIG::write_byte (EP_STA_IP_MODE, mode) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    response = false;
                } else {
                    ESPCOM::println (OK_CMD_MSG, output, espresponse);
                }
        }
        break;
#ifndef USE_AS_UPDATER_ONLY
    //AP SSID
    //[ESP105]<SSID>[pwd=<admin password>]
    case 105:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isSSIDValid (parameter.c_str() ) ) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
            if (!CONFIG::write_string (EP_AP_SSID, parameter.c_str() ) ) {
                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                response = false;
            } else {
                ESPCOM::println (OK_CMD_MSG, output, espresponse);
            }
        break;
    //AP Password
    //[ESP106]<Password>[pwd=<admin password>]
    case 106:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isPasswordValid (parameter.c_str() ) ) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
            if (!CONFIG::write_string (EP_AP_PASSWORD, parameter.c_str() ) ) {
                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                response = false;
            } else {
                ESPCOM::println (OK_CMD_MSG, output, espresponse);
            }
        break;
    //AP IP mode (DHCP/STATIC)
    //[ESP107]<mode>[pwd=<admin password>]
    case 107:
        parameter = get_param (cmd_params, "", true);
        if (parameter == "STATIC") {
            mode = STATIC_IP_MODE;
        } else if (parameter == "DHCP") {
            mode = DHCP_MODE;
        } else {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
        if ( (mode == STATIC_IP_MODE) || (mode == DHCP_MODE) ) {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            } else
#endif
                if (!CONFIG::write_byte (EP_AP_IP_MODE, mode) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    response = false;
                } else {
                    ESPCOM::println (OK_CMD_MSG, output, espresponse);
                }
        }
        break;
    // Set wifi on/off
    //[ESP110]<state>[pwd=<admin password>]
    case 110:
        parameter = get_param (cmd_params, "", true);
        if (parameter == "ON") {
            mode = 1;
        } else if (parameter == "OFF") {
            mode = 0;
        } else if (parameter == "RESTART") {
            mode = 2;
        } else {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
        if (response) {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            } else
#endif
                if (mode == 0) {
                    if (WiFi.getMode() != WIFI_OFF) {
                        //disable wifi
                        ESPCOM::println ("Disabling Wifi", output, espresponse);
#ifdef ESP_OLED_FEATURE
                        OLED_DISPLAY::display_signal(-1);
                        OLED_DISPLAY::setCursor(0, 0);
                        ESPCOM::print("", OLED_PIPE);
                        OLED_DISPLAY::setCursor(0, 16);
                        ESPCOM::print("", OLED_PIPE);
                        OLED_DISPLAY::setCursor(0, 48);
                        ESPCOM::print("Wifi disabled", OLED_PIPE);
#endif
                        WiFi.disconnect(true);
                        WiFi.enableSTA (false);
                        WiFi.enableAP (false);
                        WiFi.mode (WIFI_OFF);
                        //wifi_config.WiFi_on = false;
                        wifi_config.Disable_servers();
                        return response;
                    } else {
                        ESPCOM::println ("Wifi already off", output, espresponse);
                    }
                } else if (mode == 1) { //restart device is the best way to start everything clean
                   
                    if (WiFi.getMode() == WIFI_OFF) {
                        ESPCOM::println ("Enabling Wifi", output, espresponse);
                        web_interface->restartmodule = true;
                    } else {
                        ESPCOM::println ("Wifi already on", output, espresponse);
                    }
                } else  { //restart wifi and restart is the best way to start everything clean
                    ESPCOM::println ("Enabling Wifi", output, espresponse);
                    web_interface->restartmodule = true;
                }
        }
        break;
    //Get current IP
    //[ESP111]<header answer>
    case 111: {
        String currentIP ;
        if ((WiFi.getMode() == WIFI_STA) || (WiFi.getMode() == WIFI_AP_STA)) {
            currentIP = WiFi.localIP().toString();
        } else {
            currentIP = WiFi.softAPIP().toString();
        }
        ESPCOM::print (cmd_params, output, espresponse);
        ESPCOM::println (currentIP, output, espresponse);
        LOG (cmd_params)
        LOG (currentIP)
        LOG ("\r\n")
    }
    break;
    //Get hostname
    //[ESP112]<header answer>
    case 112: {
        String shost ;
        if (!CONFIG::read_string (EP_HOSTNAME, shost, MAX_HOSTNAME_LENGTH) ) {
            shost = wifi_config.get_default_hostname();
        }
        ESPCOM::print (cmd_params, output, espresponse);
        ESPCOM::println (shost, output, espresponse);
        LOG (cmd_params)
        LOG (shost)
        LOG ("\r\n")
    }
    break;
#if defined(TIMESTAMP_FEATURE)
    //restart time client
    case 114: {
        CONFIG::init_time_client();
        ESPCOM::println (OK_CMD_MSG, output, espresponse);
        LOG ("restart time client\r\n")
    }
    break;
    //get time client
    case 115: {
        struct tm  tmstruct;
        time_t now;
        String stmp = "";
        time(&now);
        localtime_r(&now, &tmstruct);
        stmp = String((tmstruct.tm_year)+1900) + "-";
        if (((tmstruct.tm_mon)+1) < 10) {
            stmp +="0";
        }
        stmp += String(( tmstruct.tm_mon)+1) + "-";
        if (tmstruct.tm_mday < 10) {
            stmp +="0";
        }
        stmp += String(tmstruct.tm_mday) + " ";
        if (tmstruct.tm_hour < 10) {
            stmp +="0";
        }
        stmp += String(tmstruct.tm_hour) + ":";
        if (tmstruct.tm_min < 10) {
            stmp +="0";
        }
        stmp += String(tmstruct.tm_min) + ":";
        if (tmstruct.tm_sec < 10) {
            stmp +="0";
        }
        stmp += String(tmstruct.tm_sec);
        ESPCOM::println(stmp.c_str(), output, espresponse);
    }
    break;
#endif

#ifdef DIRECT_PIN_FEATURE
    //Get/Set pin value
    //[ESP201]P<pin> V<value> [PULLUP=YES RAW=YES ANALOG=NO ANALOG_RANGE=255 CLEARCHANNELS=NO]pwd=<admin password>
    case 201:
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type == LEVEL_GUEST) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
        {
            //check if have pin
            parameter = get_param (cmd_params, "P", false);
            LOG ("Pin:")
            LOG (parameter)
            LOG (" | ")
            if (parameter == "") {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse) ;
                response = false;
            } else {
                int pin = parameter.toInt();
                //check pin is valid
                if ((pin >= 0) && (pin <= MAX_GPIO) && isValidNumber(parameter)) {
                    //check if analog or digital
                    bool isdigital = true;

                    parameter = get_param (cmd_params, "ANALOG=", false);
                    if (parameter == "YES") {
                        LOG ("Set as analog\r\n")
                        isdigital=false;
#ifdef ARDUINO_ARCH_ESP32
                        parameter = get_param (cmd_params, "CLEARCHANNELS=", false);
                        if (parameter == "YES") {
                            for (uint8_t p = 0; p < 16; p++) {
                                if(ChannelAttached2Pin[p] != -1) {
                                    ledcDetachPin(ChannelAttached2Pin[p]);
                                    ChannelAttached2Pin[p] = -1;
                                }
                            }
                        }
#endif
                    }
                    //check if is set or get
                    parameter = get_param (cmd_params, "V", false);
                    //it is a get
                    if (parameter == "") {
                        int value = 0;
                        if(isdigital) {
                            //this is to not set pin mode
                            parameter = get_param (cmd_params, "RAW=", false);
                            if (parameter == "NO") {
                                parameter = get_param (cmd_params, "PULLUP=", false);
                                if (parameter == "NO") {
                                    LOG ("Set as input\r\n")
                                    pinMode (pin, INPUT);
                                } else {
                                    //GPIO16 is different than others
                                    if (pin < MAX_GPIO) {
                                        LOG ("Set as input pull up\r\n")
                                        pinMode (pin, INPUT_PULLUP);
                                    }
#ifdef ARDUINO_ARCH_ESP8266
                                    else {
                                        LOG ("Set as input pull down 16\r\n")
                                        pinMode (pin, INPUT_PULLDOWN_16);
                                    }
#endif
                                }
                            }
                            value = digitalRead (pin);
                        } else {
#ifdef ARDUINO_ARCH_ESP8266 //only one ADC on ESP8266 A0
                            value = analogRead (A0);
#else
                            value = analogRead (pin);
#endif
                        }
                        LOG ("Read:");
                        LOG (String (value).c_str() )
                        LOG ("\n");
                        ESPCOM::println (String (value).c_str(), output, espresponse);
                    } else {
                        //it is a set
                        int value = parameter.toInt();
                        if (!isValidNumber(parameter)){
                            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                            response = false;
                            break;
                        }
                        if (isdigital) {
                            //verify it is a 0 or a 1
                            if ( (value == 0) || (value == 1) ) {
                                pinMode (pin, OUTPUT);
                                LOG ("Set:")
                                LOG (String ( (value == 0) ? LOW : HIGH) )
                                LOG ("\r\n")
                                digitalWrite (pin, (value == 0) ? LOW : HIGH);
                                ESPCOM::println (OK_CMD_MSG, output, espresponse);
                            } else {
                                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                                response = false;
                            }
                        } else {
                            int analog_range= 255;
                            parameter = get_param (cmd_params, "ANALOG_RANGE=", false);
                            if (parameter.length() > 0) {
                                analog_range = parameter.toInt();
                            }
                            LOG ("Range ")
                            LOG(String (analog_range).c_str() )
                            LOG ("\r\n")
                            if ( (value >= 0) || (value <= analog_range+1) ) {
                                LOG ("Set:")
                                LOG (String ( value) )
                                LOG ("\r\n")
#ifdef ARDUINO_ARCH_ESP8266

                                analogWriteRange(analog_range);
                                pinMode(pin, OUTPUT);
                                analogWrite(pin, value);
#else
                                int channel  = -1;
                                for (uint8_t p = 0; p < 16; p++) {
                                    if(ChannelAttached2Pin[p] == pin) {
                                        channel = p;
                                    }
                                }
                                if (channel==-1) {
                                    for (uint8_t p = 0; p < 16; p++) {
                                        if(ChannelAttached2Pin[p] == -1) {
                                            channel = p;
                                            ChannelAttached2Pin[p] = pin;
                                            p  = 16;
                                        }
                                    }
                                }
                                uint8_t resolution = 0;
                                analog_range++;
                                switch(analog_range) {
                                case 8191:
                                    resolution=13;
                                    break;
                                case 1024:
                                    resolution=10;
                                    break;
                                case 2047:
                                    resolution=11;
                                    break;
                                case 4095:
                                    resolution=12;
                                    break;
                                default:
                                    resolution=8;
                                    analog_range = 255;
                                    break;
                                }
                                if ((channel==-1) || (value > (analog_range-1))) {
                                    ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                                    return false;
                                }
                                ledcSetup(channel, 1000, resolution);
                                ledcAttachPin(pin, channel);
                                ledcWrite(channel, value);
#endif
                            } else {
                                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                                response = false;
                            }
                        }
                    }
                } else {
                    ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                    response = false;
                }
            }
        }
        break;
#endif
#ifdef MCP_USE
#define MAX_DAC_PIN 4
    //Set pin DAC value
    //[ESP202]P<pin> V<value>
    case 202:
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type == LEVEL_GUEST) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
        {
        //check if have pin
        parameter = get_param (cmd_params, "P", false);
        if (parameter == "") {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else {
            int pin = parameter.toInt();
            //check pin is valid
            if ((pin >= 0) && (pin < MAX_DAC_PIN)) {
                parameter = get_param (cmd_params, "V", false);
                //it is a get
                if (parameter == "") {                        
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
                }
                else{
                    int value = parameter.toInt();
                    LOG ("Pin:"+ String(pin) + " | Value:"+ String(value) + "\r\n");
                    CONFIG::MCP_Set(pin, value);
                }
            }
        }
        }
#endif//MCP_USE
#ifdef ESP_OLED_FEATURE
    //Output to oled
    //[ESP210]<Text>
    case 210: {
        parameter = get_param (cmd_params, "C=", false);
        int c = parameter.toInt();
        parameter = get_param (cmd_params, "L=", false);
        int l = parameter.toInt();
        parameter = get_param (cmd_params, "T=", true);
        OLED_DISPLAY::setCursor(c, l);
        ESPCOM::print(parameter.c_str(), OLED_PIPE);
        ESPCOM::println (OK_CMD_MSG, output, espresponse);
    }
    break;
    //Output to oled line 1
    //[ESP211]<Text>
    case 211: {
        parameter = get_param (cmd_params, "", true);
        OLED_DISPLAY::setCursor(0, 0);
        ESPCOM::print(parameter.c_str(), OLED_PIPE);
        ESPCOM::println (OK_CMD_MSG, output, espresponse);
    }
    break;
    //Output to oled line 2
    //[ESP212]<Text>
    case 212: {
        parameter = get_param (cmd_params, "", true);
        OLED_DISPLAY::setCursor(0, 16);
        ESPCOM::print(parameter.c_str(), OLED_PIPE);
        ESPCOM::println (OK_CMD_MSG, output, espresponse);
    }
    break;
    //Output to oled line 3
    //[ESP213]<Text>
    case 213: {
        parameter = get_param (cmd_params, "", true);
        OLED_DISPLAY::setCursor(0, 32);
        ESPCOM::print(parameter.c_str(), OLED_PIPE);
        ESPCOM::println (OK_CMD_MSG, output, espresponse);
    }
    break;
    //Output to oled line 4
    //[ESP214]<Text>
    case 214: {
        parameter = get_param (cmd_params, "", true);
        OLED_DISPLAY::setCursor(0, 48);
        ESPCOM::print(parameter.c_str(), OLED_PIPE);
        ESPCOM::println (OK_CMD_MSG, output, espresponse);
    }
    break;
#endif
    //Command delay
    case 290: {
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type == LEVEL_GUEST) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
        {
            if (parameter.length() != 0) {
                ESPCOM::println ("Pause", output, espresponse);
                CONFIG::wait(parameter.toInt());
                }
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
        }
    }
    break;
    //display ESP3D EEPROM version detected
    case 300: {
        uint8_t v = CONFIG::get_EEPROM_version();
        ESPCOM::println (String(v).c_str(), output, espresponse);
    }
    break;
    
    //Get full EEPROM settings content
    //[ESP400]
    case 400: {
        char sbuf[MAX_DATA_LENGTH + 1];
        uint8_t ipbuf[4];
        byte bbuf = 0;
        int ibuf = 0;
        parameter = get_param (cmd_params, "", true);
        //Start JSON
        ESPCOM::println (F ("{\"EEPROM\":["), output, espresponse);
        if (cmd_params == "network" || cmd_params == "") {
#ifdef Moto_UI
            //1- Baud Rate
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_BAUD_RATE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_BAUD_RATE,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Baud Rate\",\"O\":[{\"9600\":\"9600\"},{\"19200\":\"19200\"},{\"38400\":\"38400\"},{\"57600\":\"57600\"},{\"115200\":\"115200\"},{\"230400\":\"230400\"},{\"250000\":\"250000\"},{\"500000\":\"500000\"},{\"921600\":\"921600\"}]}"), output, espresponse);
#else//Moto_UI
//1- Baud Rate
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_BAUD_RATE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_BAUD_RATE,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Baud Rate\",\"O\":[{\"9600\":\"9600\"},{\"19200\":\"19200\"},{\"38400\":\"38400\"},{\"57600\":\"57600\"},{\"115200\":\"115200\"},{\"230400\":\"230400\"},{\"250000\":\"250000\"},{\"500000\":\"500000\"},{\"921600\":\"921600\"}]}"), output, espresponse);

            ESPCOM::println (F (","), output, espresponse);

            //2-Sleep Mode
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_SLEEP_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_SLEEP_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Sleep Mode\",\"O\":[{\"None\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_NONE_SLEEP), output, espresponse);
            ESPCOM::print (F ("\"},{\"Light\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_LIGHT_SLEEP), output, espresponse);
            ESPCOM::print (F ("\"},{\"Modem\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_MODEM_SLEEP), output, espresponse);
            ESPCOM::print (F ("\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

#ifdef AUTHENTICATION_FEATURE
            //5-Admin password
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_ADMIN_PWD), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_ADMIN_PWD, sbuf, MAX_LOCAL_PASSWORD_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_LOCAL_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Admin Password\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_LOCAL_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //6-User password
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_USER_PWD), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_USER_PWD, sbuf, MAX_LOCAL_PASSWORD_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_LOCAL_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"User Password\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_LOCAL_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
#endif

            //7-Hostname
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_HOSTNAME), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_HOSTNAME, sbuf, MAX_HOSTNAME_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Hostname\" ,\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_HOSTNAME_LENGTH), output, espresponse);
            ESPCOM::print (F ("\", \"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_HOSTNAME_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
        #ifdef LOOKLINE_UI
            ESPCOM::println (F (","), output, espresponse);
            //8-wifi mode
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_WIFI_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_WIFI_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Wifi mode\",\"O\":[{\"AP\":\"1\"},{\"STA\":\"2\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //9-STA SSID
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_SSID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_STA_SSID, sbuf, MAX_SSID_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Station SSID\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //10-STA password
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_PASSWORD), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_STA_PASSWORD, sbuf, MAX_PASSWORD_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Station Password\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            #endif//LOOKLINE_UI
#if defined(TIMESTAMP_FEATURE)
            ESPCOM::println (F (","), output, espresponse);
            
            //26-Time zone
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIMEZONE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_TIMEZONE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr ( (int8_t) bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Time Zone\",\"O\":["), output, espresponse);
            for (int i = -12; i <= 12 ; i++) {
                ESPCOM::print (F ("{\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (i), output, espresponse);
                ESPCOM::print (F ("\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (i), output, espresponse);
                ESPCOM::print (F ("\"}"), output, espresponse);
                if (i < 12) {
                    ESPCOM::print (F (","), output, espresponse);
                }
            }
            ESPCOM::print (F ("]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //27- DST
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIME_ISDST), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_TIME_ISDST, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Day Saving Time\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //28- Time Server1
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIME_SERVER1), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_TIME_SERVER1, sbuf, MAX_DATA_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Time Server 1\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //29- Time Server2
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIME_SERVER2), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_TIME_SERVER2, sbuf, MAX_DATA_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Time Server 2\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //30- Time Server3
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIME_SERVER3), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_TIME_SERVER3, sbuf, MAX_DATA_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Time Server 3\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
#endif

#ifdef NOTIFICATION_FEATURE
            ESPCOM::println (F (","), output, espresponse);
            //Notification type
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_NOTIFICATION_TYPE), output, espresponse);
            ESPCOM::print (F("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (ESP_NOTIFICATION_TYPE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F("\",\"H\":\"Notification\",\"O\":[{\"None\":\"0\"},{\"Pushover\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_PUSHOVER_NOTIFICATION), output, espresponse);
            ESPCOM::print (F("\"},{\"Email\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_EMAIL_NOTIFICATION), output, espresponse);
            ESPCOM::print (F("\"},{\"Line\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_LINE_NOTIFICATION), output, espresponse);
            ESPCOM::print (F("\"},{\"IFTTT\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_IFTTT_NOTIFICATION), output, espresponse);
            ESPCOM::print (F("\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //Token 1
            ESPCOM::print (F("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_NOTIFICATION_TOKEN1), output, espresponse);
            ESPCOM::print ( F("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (ESP_NOTIFICATION_TOKEN1, sbuf, MAX_NOTIFICATION_TOKEN_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print ( F("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_NOTIFICATION_TOKEN_LENGTH), output, espresponse);
            ESPCOM::print ( F ("\",\"H\":\"Token 1\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_NOTIFICATION_TOKEN_LENGTH), output, espresponse);
            ESPCOM::print ( F("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //Token 2
            ESPCOM::print (F("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_NOTIFICATION_TOKEN2), output, espresponse);
            ESPCOM::print ( F("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (ESP_NOTIFICATION_TOKEN2, sbuf, MAX_NOTIFICATION_TOKEN_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print ( F("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_NOTIFICATION_TOKEN_LENGTH), output, espresponse);
            ESPCOM::print ( F ("\",\"H\":\"Token 2\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_NOTIFICATION_TOKEN_LENGTH), output, espresponse);
            ESPCOM::print ( F("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //Notifications Settings
            ESPCOM::print (F("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_NOTIFICATION_SETTINGS), output, espresponse);
            ESPCOM::print ( F("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (ESP_NOTIFICATION_SETTINGS, sbuf, MAX_NOTIFICATION_TOKEN_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print ( F("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_NOTIFICATION_SETTINGS_LENGTH), output, espresponse);
            ESPCOM::print ( F ("\",\"H\":\"Notifications Settings\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_NOTIFICATION_SETTINGS_LENGTH), output, espresponse);
            ESPCOM::print ( F("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //Auto Notification
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_AUTO_NOTIFICATION), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (ESP_AUTO_NOTIFICATION, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Auto notification\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}"), output, espresponse);
            
            
#endif //NOTIFICATION_FEATURE
    }
        if (cmd_params == "printer" || cmd_params == "") {
            if (cmd_params == "") {
                ESPCOM::println (F (","), output, espresponse);
            }
            //Target FW
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TARGET_FW), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_TARGET_FW, &bbuf ) ) {
                ESPCOM::print ("Unknown", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Target FW\",\"O\":[{\"lookline\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (LOOKLINE), output, espresponse);
        #ifdef LOOKLINE_UI            
            ESPCOM::print (F ("\"},{\"lookline Gateway\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (LOOKLINEGW), output, espresponse);
        #endif//LOOKLINE_UI
            
            ESPCOM::print (F ("\"},{\"Circuit Testing\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (TESTING), output, espresponse);
            ESPCOM::print (F ("\"},{\"Gyro Datalog\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (GYRODATALOG), output, espresponse);
            ESPCOM::print (F ("\"},{\"RF Hub\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (RF_HUB), output, espresponse);
            ESPCOM::print (F ("\"},{\"Mesh Hub\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MESH_HUB), output, espresponse);
            ESPCOM::print (F ("\"},{\"Moto DashBoard\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MOTO_DASH), output, espresponse);
            
        #ifdef IOTDEVICE_UI  
            ESPCOM::print (F ("\"},{\"valve Gateway\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (VALVEGW), output, espresponse);
            ESPCOM::print (F ("\"},{\"Light Timer\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (LIGHT_TM), output, espresponse);
            ESPCOM::print (F ("\"},{\"IoT Device\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (IOT_DEVICES), output, espresponse);
        #endif//IOTDEVICE_UI
#ifdef ESP3D_UI            
            ESPCOM::print (F ("\"},{\"Repetier\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (REPETIER), output, espresponse);
            ESPCOM::print (F ("\"},{\"Repetier for Davinci\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (REPETIER4DV), output, espresponse);
            ESPCOM::print (F ("\"},{\"Marlin\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MARLIN), output, espresponse);
            ESPCOM::print (F ("\"},{\"Marlin Kimbra\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MARLINKIMBRA), output, espresponse);
            ESPCOM::print (F ("\"},{\"Smoothieware\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (SMOOTHIEWARE), output, espresponse);
            ESPCOM::print (F ("\"},{\"Grbl\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (GRBL), output, espresponse);
#endif//ESP3D_UI            
            ESPCOM::print (F ("\"},{\"Unknown\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (UNKNOWN_FW), output, espresponse);            
            ESPCOM::print (F ("\"}]}"), output, espresponse);
            //if(CONFIG::read_byte (EP_TARGET_FW, &bbuf ) == LOOKLINE || 
            //CONFIG::read_byte (EP_TARGET_FW, &bbuf ) == LOOKLINEGW|| 
            //CONFIG::read_byte (EP_TARGET_FW, &bbuf ) == VALVEGW){
#ifdef LOOKLINE_UI             
             ESPCOM::println (F (","), output, espresponse);
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_ID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_ID, &bbuf) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Board ID\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_ID), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_ID), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
#endif//LOOKLINE_UI
#ifdef Valve_UI  

                ESPCOM::println (F (","), output, espresponse);
                //MQTT Broker
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MQTT_BROKER), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_string (EP_MQTT_BROKER, sbuf, MAX_MQTT_BROKER_LENGTH) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print (encodeString(sbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_MQTT_BROKER_LENGTH), output, espresponse);
                ESPCOM::print (F ("\",\"H\":\"MQTT Server\",\"M\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_MQTT_BROKER_LENGTH), output, espresponse);
                ESPCOM::println (F ("\"},"), output, espresponse);
                //MQTT User
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MQTT_USER), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_string (EP_MQTT_USER, sbuf, MAX_MQTT_USER_LENGTH) ) {
                    ESPCOM::print ("", output, espresponse);
                } else {
                    ESPCOM::print (encodeString(sbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_MQTT_USER_LENGTH), output, espresponse);
                ESPCOM::print (F ("\",\"H\":\"MQTT User\",\"M\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_MQTT_USER_LENGTH), output, espresponse);
                ESPCOM::println (F ("\"},"), output, espresponse);
                //MQTT Password
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MQTT_PASS), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_string (EP_MQTT_PASS, sbuf, MAX_MQTT_PASS_LENGTH) ) {
                    ESPCOM::print ("", output, espresponse);
                } else {
                    ESPCOM::print (encodeString(sbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_MQTT_PASS_LENGTH), output, espresponse);
                ESPCOM::print (F ("\",\"H\":\"MQTT Password\",\"M\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_MQTT_PASS_LENGTH), output, espresponse);
                ESPCOM::println (F ("\"}"), output, espresponse);
                // Lora Chanels
                ESPCOM::print (F (",{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_LORA_CHANEL), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_buffer (EP_LORA_CHANEL,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                    ESPCOM::print ("915", output, espresponse);
                } else {
                    //LOG ("EP_LORA_CHANEL:" + String(ibuf));
                    ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"LoRa Chanels\",\"S\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_LORA_CH_LENGTH), output, espresponse);
                ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_LORA_CH_LENGTH), output, espresponse);
                ESPCOM::print (F ("\"}"), output, espresponse);
                //LoRa RSSI
                ESPCOM::print (F (",{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_LORA_RSSI), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_LORA_RSSI, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"LoRa RSSI\",\"O\":[{\"Disable\":\"0\"},{\"Enable\":\"1\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //LoRa Master
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_LORA_MASTER), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_LORA_MASTER, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"LoRa Role\",\"O\":[{\"Slave\":\"0\"},{\"Master\":\"1\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //LoRa Air Rate
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_LORA_AIRRATE), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_LORA_AIRRATE, &bbuf ) ) {
                    ESPCOM::print ("Unknown", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"LoRa Air Rate\",\"O\":[{\"0.3Kbps\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Air_Rate_03), output, espresponse);
                ESPCOM::print (F ("\"},{\"1.2Kbps\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Air_Rate_12), output, espresponse);
                ESPCOM::print (F ("\"},{\"2.4Kbps\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Air_Rate_24), output, espresponse);
                ESPCOM::print (F ("\"},{\"4.8Kbps\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Air_Rate_48), output, espresponse);
                ESPCOM::print (F ("\"},{\"9.6Kbps\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Air_Rate_96), output, espresponse);
                ESPCOM::print (F ("\"},{\"19.2Kbps\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Air_Rate_192), output, espresponse);
                ESPCOM::print (F ("\"}]},"), output, espresponse);
                //LoRa Protocol
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_LORA_PROTOCOL), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_LORA_PROTOCOL, &bbuf ) ) {
                    ESPCOM::print ("Unknown", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"LoRa Protocol\",\"O\":[{\"Transparent\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Transparent), output, espresponse);
                ESPCOM::print (F ("\"},{\"FIXED Node\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (FIXED_Node), output, espresponse);
                ESPCOM::print (F ("\"},{\"FIXED Gateway\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (FIXED_Gateway), output, espresponse);
                ESPCOM::print (F ("\"},{\"WOR Node\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (WOR_Node), output, espresponse);
                ESPCOM::print (F ("\"},{\"WOR Gateway\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (WOR_Gateway), output, espresponse);
                ESPCOM::print (F ("\"},{\"Broadcast 1\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (BROADCAST_MESSAGE1), output, espresponse);
                ESPCOM::print (F ("\"},{\"Broadcast 2\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (BROADCAST_MESSAGE2), output, espresponse);
                ESPCOM::print (F ("\"},{\"Broadcast 3\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (BROADCAST_MESSAGE3), output, espresponse);
                ESPCOM::print (F ("\"}]}"), output, espresponse);
                // Lora Scan Time
                ESPCOM::print (F (",{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_LORA_T_SCAN), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_LORA_T_SCAN,  &bbuf) ) {
                    ESPCOM::print ("5", output, espresponse);
                } else {
                    //LOG ("EP_LORA_T_SCAN:" + String(bbuf));
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Scan Time\",\"S\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_LORA_TIME_SCAN_LENGTH), output, espresponse);
                ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_LORA_TIME_SCAN_LENGTH), output, espresponse);
                ESPCOM::print (F ("\"}"), output, espresponse);
                // Lora Request Time
                ESPCOM::print (F (",{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_LORA_T_REQUEST), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_LORA_T_REQUEST,  &bbuf) ) {
                    ESPCOM::print ("5", output, espresponse);
                } else {
                    //LOG ("EP_LORA_T_REQUEST:" + String(bbuf));
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Request Time\",\"S\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_LORA_TIME_REQUEST_LENGTH), output, espresponse);
                ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_LORA_TIME_REQUEST_LENGTH), output, espresponse);
                ESPCOM::print (F ("\"}"), output, espresponse);
#endif//Valve_UI    
#ifdef CircuitTesting_UI 
                //Num Pin
                ESPCOM::print (F (",{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (QuaPin), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                CONFIG::read_byte (QuaPin, &bbuf);
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                ESPCOM::print (F ("\",\"H\":\"Quantity pin\",\"S\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (20), output, espresponse);
                ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (0), output, espresponse);
                ESPCOM::print (F ("\"}"), output, espresponse);               
                //Input Pin 1
                ESPCOM::print (F (",{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_Pin_1), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_Pin_1, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Input Pin 1\",\"O\":[{\"39\":\"39\"},{\"36\":\"36\"},{\"35\":\"35\"},{\"34\":\"34\"},{\"33\":\"33\"},{\"32\":\"32\"},{\"16\":\"16\"},{\"17\":\"17\"},{\"18\":\"18\"},{\"19\":\"19\"},{\"4\":\"4\"},{\"5\":\"5\"},{\"23\":\"23\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //Input Pin 2
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_Pin_2), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_Pin_2, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Input Pin 2\",\"O\":[{\"39\":\"39\"},{\"36\":\"36\"},{\"35\":\"35\"},{\"34\":\"34\"},{\"33\":\"33\"},{\"32\":\"32\"},{\"16\":\"16\"},{\"17\":\"17\"},{\"18\":\"18\"},{\"19\":\"19\"},{\"4\":\"4\"},{\"5\":\"5\"},{\"23\":\"23\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //Input Pin 3
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_Pin_3), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_Pin_3, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Input Pin 3\",\"O\":[{\"39\":\"39\"},{\"36\":\"36\"},{\"35\":\"35\"},{\"34\":\"34\"},{\"33\":\"33\"},{\"32\":\"32\"},{\"16\":\"16\"},{\"17\":\"17\"},{\"18\":\"18\"},{\"19\":\"19\"},{\"4\":\"4\"},{\"5\":\"5\"},{\"23\":\"23\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //Input Pin 4
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_Pin_4), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_Pin_4, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Input Pin 4\",\"O\":[{\"39\":\"39\"},{\"36\":\"36\"},{\"35\":\"35\"},{\"34\":\"34\"},{\"33\":\"33\"},{\"32\":\"32\"},{\"16\":\"16\"},{\"17\":\"17\"},{\"18\":\"18\"},{\"19\":\"19\"},{\"4\":\"4\"},{\"5\":\"5\"},{\"23\":\"23\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //Input Pin 5
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_Pin_5), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_Pin_5, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Input Pin 5\",\"O\":[{\"39\":\"39\"},{\"36\":\"36\"},{\"35\":\"35\"},{\"34\":\"34\"},{\"33\":\"33\"},{\"32\":\"32\"},{\"16\":\"16\"},{\"17\":\"17\"},{\"18\":\"18\"},{\"19\":\"19\"},{\"4\":\"4\"},{\"5\":\"5\"},{\"23\":\"23\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //Input Pin 6
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_Pin_6), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_Pin_6, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Input Pin 6\",\"O\":[{\"39\":\"39\"},{\"36\":\"36\"},{\"35\":\"35\"},{\"34\":\"34\"},{\"33\":\"33\"},{\"32\":\"32\"},{\"16\":\"16\"},{\"17\":\"17\"},{\"18\":\"18\"},{\"19\":\"19\"},{\"4\":\"4\"},{\"5\":\"5\"},{\"23\":\"23\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //Input Pin 7
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_Pin_7), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_Pin_7, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Input Pin 7\",\"O\":[{\"39\":\"39\"},{\"36\":\"36\"},{\"35\":\"35\"},{\"34\":\"34\"},{\"33\":\"33\"},{\"32\":\"32\"},{\"16\":\"16\"},{\"17\":\"17\"},{\"18\":\"18\"},{\"19\":\"19\"},{\"4\":\"4\"},{\"5\":\"5\"},{\"23\":\"23\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //Input Pin 8
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_Pin_8), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_Pin_8, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Input Pin 8\",\"O\":[{\"39\":\"39\"},{\"36\":\"36\"},{\"35\":\"35\"},{\"34\":\"34\"},{\"33\":\"33\"},{\"32\":\"32\"},{\"16\":\"16\"},{\"17\":\"17\"},{\"18\":\"18\"},{\"19\":\"19\"},{\"4\":\"4\"},{\"5\":\"5\"},{\"23\":\"23\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //Input Pin 9
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_Pin_9), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_Pin_9, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Input Pin 9\",\"O\":[{\"39\":\"39\"},{\"36\":\"36\"},{\"35\":\"35\"},{\"34\":\"34\"},{\"33\":\"33\"},{\"32\":\"32\"},{\"16\":\"16\"},{\"17\":\"17\"},{\"18\":\"18\"},{\"19\":\"19\"},{\"4\":\"4\"},{\"5\":\"5\"},{\"23\":\"23\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //Input Pin 10
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_Pin_10), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_Pin_10, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Input Pin 10\",\"O\":[{\"39\":\"39\"},{\"36\":\"36\"},{\"35\":\"35\"},{\"34\":\"34\"},{\"33\":\"33\"},{\"32\":\"32\"},{\"16\":\"16\"},{\"17\":\"17\"},{\"18\":\"18\"},{\"19\":\"19\"},{\"4\":\"4\"},{\"5\":\"5\"},{\"23\":\"23\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //Input Pin 11
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_Pin_11), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_Pin_11, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Input Pin 11\",\"O\":[{\"39\":\"39\"},{\"36\":\"36\"},{\"35\":\"35\"},{\"34\":\"34\"},{\"33\":\"33\"},{\"32\":\"32\"},{\"16\":\"16\"},{\"17\":\"17\"},{\"18\":\"18\"},{\"19\":\"19\"},{\"4\":\"4\"},{\"5\":\"5\"},{\"23\":\"23\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //Input Pin 12
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_Pin_12), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_Pin_12, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Input Pin 12\",\"O\":[{\"39\":\"39\"},{\"36\":\"36\"},{\"35\":\"35\"},{\"34\":\"34\"},{\"33\":\"33\"},{\"32\":\"32\"},{\"16\":\"16\"},{\"17\":\"17\"},{\"18\":\"18\"},{\"19\":\"19\"},{\"4\":\"4\"},{\"5\":\"5\"},{\"23\":\"23\"}]}"), output, espresponse);
                ESPCOM::println (F (","), output, espresponse);
                //Input Pin 13
                ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_Pin_13), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_Pin_13, &bbuf ) ) {
                    ESPCOM::print ("???", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"Input Pin 13\",\"O\":[{\"39\":\"39\"},{\"36\":\"36\"},{\"35\":\"35\"},{\"34\":\"34\"},{\"33\":\"33\"},{\"32\":\"32\"},{\"16\":\"16\"},{\"17\":\"17\"},{\"18\":\"18\"},{\"19\":\"19\"},{\"4\":\"4\"},{\"5\":\"5\"},{\"23\":\"23\"}]}"), output, espresponse);

                
#endif//CircuitTesting_UI              
            //end JSON
            //ESPCOM::println (F ("\n]}"), output, espresponse);
            //}
            //else{
            //Output flag
#ifdef ESP3D_UI  
            ESPCOM::println (F (","), output, espresponse);          
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_OUTPUT_FLAG), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"F\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_OUTPUT_FLAG, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            String s = "\",\"H\":\"Output msg\",\"O\":[{\"M117\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_M117);
            s+= "\"}";
          
#ifdef ESP_OLED_FEATURE
            s+=",{\"Oled\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_OLED);
            s+="\"}";
#endif
            s+=",{\"Serial\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_SERIAL);
            s+="\"}";
#ifdef WS_DATA_FEATURE
            s+=",{\"Web Socket\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_WSOCKET);
            s+="\"}";
#endif
#ifdef TCP_IP_DATA_FEATURE
            s+=",{\"TCP\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_TCP);
            s+="\"}";
#endif
            s+= "]}";
            ESPCOM::print (s, output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //}//if(not Lookline/lookline gateway/valve gateway)
#endif//ESP3D_UI              
#ifdef DHT_FEATURE
            ESPCOM::println (F (","), output, espresponse);
            //DHT type
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_DHT_TYPE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (CONFIG::DHT_type), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"DHT Type\",\"O\":[{\"None\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (255), output, espresponse);
            ESPCOM::print (F ("\"},{\"DHT11\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DHTesp::DHT11), output, espresponse);
            ESPCOM::print (F ("\"},{\"DHT22\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DHTesp::DHT22), output, espresponse);
            ESPCOM::print (F ("\"},{\"AM2302\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DHTesp::RHT03), output, espresponse);
            ESPCOM::print (F ("\"},{\"RHT03\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DHTesp::AM2302), output, espresponse);
            ESPCOM::print (F ("\"}]}"), output, espresponse);

            //DHT interval
            ESPCOM::println (F (","), output, espresponse);
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_DHT_INTERVAL), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (CONFIG::DHT_interval), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"DHT check (seconds)\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_WEB_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (0), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
#endif
#ifdef AUTOITGW_UI
            ESPCOM::println (F (","), output, espresponse);
            
            //////////////////////////////////////// MQTT /////////////////////////////////////////////////////
            //MQTT Broker
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MQTT_BROKER), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_MQTT_BROKER, sbuf, MAX_MQTT_BROKER_LENGTH) ) {
                ESPCOM::print ("broker.emqx.io", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_MQTT_BROKER_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"MQTT Server\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_MQTT_BROKER_LENGTH), output, espresponse);
            ESPCOM::println (F ("\"},"), output, espresponse);
            //MQTT User
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MQTT_USER), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_MQTT_USER, sbuf, MAX_MQTT_USER_LENGTH) ) {
                ESPCOM::print ("", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_MQTT_USER_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"MQTT User\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_MQTT_USER_LENGTH), output, espresponse);
            ESPCOM::println (F ("\"},"), output, espresponse);
            //MQTT Password
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MQTT_PASS), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_MQTT_PASS, sbuf, MAX_MQTT_PASS_LENGTH) ) {
                ESPCOM::print ("", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_MQTT_PASS_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"MQTT Password\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_MQTT_PASS_LENGTH), output, espresponse);
            ESPCOM::println (F ("\"}"), output, espresponse);
            //DEBUG
            ESPCOM::print (F (",{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_DEBUG), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_DEBUG, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Device debug\",\"O\":[{\"Debug\":\"1\"},{\"NoDebug\":\"0\"}]}"), output, espresponse);
            // ESPCOM::println (F (","), output, espresponse);
#endif//AUTOITGW_UI
#ifdef IOTDEVICE_UI
            ESPCOM::println (F (","), output, espresponse);
            //////////////////////////////////////// Category ////////////////////////////////////////////////
            //Category
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_CATEGORY), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_CATEGORY, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Category\",\"O\":[\
             {\"Gateway\":\"0\"}\
            ,{\"Switch\":\"1\"}\
            ,{\"DHT Sensor\":\"2\"}\
            ,{\"Motion Sensor\":\"3\"}\
            ,{\"Relay Device\":\"4\"}\
            ,{\"Moisture Sensor\":\"5\"}\
            ,{\"Temperature\":\"6\"}\
            ,{\"Valve\":\"7\"}\
            ,{\"SHT Sensor\":\"9\"}\
            ]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //////////////////////////////////////// MQTT /////////////////////////////////////////////////////
            //MQTT Broker
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MQTT_BROKER), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_MQTT_BROKER, sbuf, MAX_MQTT_BROKER_LENGTH) ) {
                ESPCOM::print ("broker.emqx.io", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_MQTT_BROKER_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"MQTT Server\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_MQTT_BROKER_LENGTH), output, espresponse);
            ESPCOM::println (F ("\"},"), output, espresponse);
            //MQTT Port
            ESPCOM::print (F ("{\"F\":\"tcp\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MQTT_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_MQTT_PORT,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"MQTT Port\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_DATA_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_DATA_PORT), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //MQTT User
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MQTT_USER), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_MQTT_USER, sbuf, MAX_MQTT_USER_LENGTH) ) {
                ESPCOM::print ("", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_MQTT_USER_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"MQTT User\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_MQTT_USER_LENGTH), output, espresponse);
            ESPCOM::println (F ("\"},"), output, espresponse);
            //MQTT Password
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MQTT_PASS), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_MQTT_PASS, sbuf, MAX_MQTT_PASS_LENGTH) ) {
                ESPCOM::print ("", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_MQTT_PASS_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"MQTT Password\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_MQTT_PASS_LENGTH), output, espresponse);
            ESPCOM::println (F ("\"}"), output, espresponse);
            
            /////////////////////////////////////// Hardware //////////////////////////////////////////////
            //netwwork ID
            ESPCOM::print (F (",{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_NET_ID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_NET_ID, &bbuf) ) {
                ESPCOM::print ("1", output, espresponse);
            } else {
                //LOG ("EP_LORA_ID:" + String(bbuf));
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Network ID\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_NET_ID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_NET_ID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);

            //ID
            ESPCOM::print (F (",{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_ID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_ID, &bbuf) ) {
                ESPCOM::print ("915", output, espresponse);
            } else {
                //LOG ("EP_LORA_ID:" + String(bbuf));
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Device ID\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_ID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_ID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"},"), output, espresponse);

            //Name board
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_NAME), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_EEPROM_NAME, sbuf, MAX_NAME_LENGTH) ) {
                ESPCOM::print ("VPlab", output, espresponse);
            } else {
                // ESPCOM::print (encodeString(sbuf), output, espresponse);
                ESPCOM::print ("VPlab", output, espresponse);

            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_NAME_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Device Name\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_NAME_LENGTH), output, espresponse);
            ESPCOM::println (F ("\"},"), output, espresponse);

            //OLED Type
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_OLED_TYPE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_OLED_TYPE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"OLED type\",\"O\":[{\"0.96inch\":\"0\"},{\"1.3inch\":\"1\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //ROLE
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_ROLE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_ROLE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Device Role\",\"O\":[{\"Node\":\"0\"},{\"Gateway\":\"1\"},{\"Node Gateway\":\"2\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //DEBUG
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_DEBUG), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_DEBUG, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Device debug\",\"O\":[{\"Debug\":\"1\"},{\"NoDebug\":\"0\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //COM Mode
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_COM_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_COM_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Com mode\",\"O\":[{\"Mesh Wifi\":\"0\"},{\"MQTT\":\"1\"},{\"RS485\":\"2\"}]}"), output, espresponse);
           
            ESPCOM::println (F (","), output, espresponse);
            //BUTTON
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_FW_BUTTON), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_FW_BUTTON, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Update FW Button\",\"O\":[{\"Use\":\"0\"},{\"Not Use\":\"1\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //UPDATE FW
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_UPDATE_FW), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_UPDATE_FW, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Update FW Auto\",\"O\":[{\"Use\":\"0\"},{\"Not Use\":\"1\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //BUTTON PIN
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_PIN_BUTTON), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_PIN_BUTTON, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Button Pin\",\"O\":[{\"Pin 12\":\"12\"},{\"Pin 13\":\"13\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //LED PIN
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_PIN_LEDFULL), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_PIN_LEDFULL, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"LED Pin\",\"O\":[{\"Pin 2\":\"2\"},{\"Pin 12\":\"12\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            
            //LED TYPE
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_LED_TYPE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_LED_TYPE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"LED Type\",\"O\":[{\"Full color\":\"0\"},{\"Singel\":\"1\"}]}"), output, espresponse);

           ESPCOM::println (F (","), output, espresponse);          
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_OUTPUT_FLAG), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"F\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_OUTPUT_FLAG, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            String s = "\",\"H\":\"Output msg\",\"O\":[{\"M117\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_M117);
            s+= "\"}";
          
#ifdef ESP_OLED_FEATURE
            s+=",{\"Oled\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_OLED);
            s+="\"}";
#endif
            s+=",{\"Serial\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_SERIAL);
            s+="\"}";
#ifdef WS_DATA_FEATURE
            s+=",{\"Web Socket\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_WSOCKET);
            s+="\"}";
#endif
#ifdef TCP_IP_DATA_FEATURE
            s+=",{\"TCP\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_TCP);
            s+="\"}";
#endif
            s+= "]}";
            ESPCOM::print (s, output, espresponse);

#endif//IOT_DEVICES
#ifdef LOOKLINE_UI
            ESPCOM::println (F (","), output, espresponse);
            //Delay for Counter
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_COUNTER_DELAY), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_EEPROM_COUNTER_DELAY,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Delay for counter(ms)\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_RESULT), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_RESULT), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //ROLE
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_ROLE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_ROLE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Device Role\",\"O\":[{\"Node\":\"0\"},{\"Gateway\":\"1\"},{\"Repeater\":\"2\"}]}"), output, espresponse);
           
            ESPCOM::println (F (","), output, espresponse);
            //COM Mode
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_COM_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_COM_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Com mode\",\"O\":[{\"LoRa\":\"0\"},{\"Mesh Wifi\":\"1\"},{\"MQTT\":\"2\"},{\"RS485\":\"3\"}]}"), output, espresponse);
           
            ESPCOM::println (F (","), output, espresponse);
            //Module type
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_MODULE_TYPE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_MODULE_TYPE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Module type\",\"O\":[{\"Auto Detect\":\"0\"},{\"Lookline Gateway V14\":\"1\"},{\"LED7 seg Board V13.0\":\"4\"},{\"LED7 seg Board V14.0\":\"2\"},{\"LED7 seg Board V14.1\":\"3\"}]}"), output, espresponse);
           
            ESPCOM::println (F (","), output, espresponse);
            //DEBUG
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_DEBUG), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_DEBUG, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Device debug\",\"O\":[{\"Debug\":\"1\"},{\"NoDebug\":\"0\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);    

            //TEST MODE
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_TEST_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_TEST_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Test Mode\",\"O\":[{\"TEST\":\"1\"},{\"No TEST\":\"0\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse); 

            // URL Version
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_URL_VER), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_EEPROM_URL_VER, sbuf, MAX_URL_VER_LENGTH) ) {
                ESPCOM::print ("VPlab", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_URL_VER_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"URL Version\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_URL_VER_LENGTH), output, espresponse);
            ESPCOM::println (F ("\"},"), output, espresponse);

            //URL firmware
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_URL_FW), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_EEPROM_URL_FW, sbuf, MAX_URL_FW_LENGTH) ) {
                ESPCOM::print ("VPlab", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_URL_FW_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"URL Firmware\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_URL_FW_LENGTH), output, espresponse);
            ESPCOM::println (F ("\"},"), output, espresponse);
            

            //UPDATE MODE
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_UPDATE_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_UPDATE_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Update Mode\",\"O\":[{\"Check FW\":\"1\"},{\"None\":\"0\"}]}"), output, espresponse);
            //FLAG Hardware
            ESPCOM::println (F (","), output, espresponse);          
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_OUTPUT_FLAG), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"F\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_OUTPUT_FLAG, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            String s = "\",\"H\":\"Output msg\",\"O\":[{\"M117\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_M117);
            s+= "\"}";
          
#ifdef ESP_OLED_FEATURE
            s+=",{\"Oled\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_OLED);
            s+="\"}";
#endif
            s+=",{\"Serial\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_SERIAL);
            s+="\"}";
#ifdef WS_DATA_FEATURE
            s+=",{\"Web Socket\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_WSOCKET);
            s+="\"}";
#endif
#ifdef TCP_IP_DATA_FEATURE
            s+=",{\"TCP\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_TCP);
            s+="\"}";
#endif
            s+= "]}";
            ESPCOM::print (s, output, espresponse);

#endif//LOOKLINE_UI
#endif//Moto_UI
        }
        //end JSON
        ESPCOM::println (F ("\n]}"), output, espresponse);
        // cmdWic.Set_Init_UI();
        #ifdef LOOKLINE_UI
        Lookline_PROG.SetStart(2);
        #endif//LOOKLINE_UI

        #ifdef PLC_MASTER_UI
        PLC_cmd.connectWeb(1);
        #endif//PLC_MASTER_UI
    }
    break;

    //Get full EEPROM Looline settings content
    //[ESP402]
    case 402: {
        char sbuf[MAX_DATA_LENGTH + 1];
        uint8_t ipbuf[4];
        byte bbuf = 0;
        int ibuf = 0;
        parameter = get_param (cmd_params, "", true);
        if      (parameter == "1524") {auth_val = 2;}
        else if (parameter == "2709") {auth_val = 1;}
        else if (parameter == "1111") {auth_val = 0; }
        #ifdef Moto_UI
        //Start JSON
        ESPCOM::println (F ("{\"EEPROMLL\":["), output, espresponse);
        if (cmd_params == "network" || cmd_params == "") { 
            //8-wifi mode
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_WIFI_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_WIFI_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Wifi mode\",\"O\":[{\"AP\":\"1\"},{\"STA\":\"2\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

             //Target FW
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TARGET_FW), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_TARGET_FW, &bbuf ) ) {
                ESPCOM::print ("Unknown", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Target FW\",\"O\":[{\"lookline\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (LOOKLINE), output, espresponse);
        #ifdef LOOKLINE_UI            
            ESPCOM::print (F ("\"},{\"lookline Gateway\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (LOOKLINEGW), output, espresponse);
        #endif//LOOKLINE_UI
            
            ESPCOM::print (F ("\"},{\"Circuit Testing\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (TESTING), output, espresponse);
            ESPCOM::print (F ("\"},{\"Gyro Datalog\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (GYRODATALOG), output, espresponse);
            ESPCOM::print (F ("\"},{\"RF Hub\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (RF_HUB), output, espresponse);
            ESPCOM::print (F ("\"},{\"Mesh Hub\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MESH_HUB), output, espresponse);
            ESPCOM::print (F ("\"},{\"Moto DashBoard\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MOTO_DASH), output, espresponse);
            ESPCOM::print (F ("\"},{\"Unknown\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (UNKNOWN_FW), output, espresponse);            
            ESPCOM::print (F ("\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            
            //9-STA SSID
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_SSID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_STA_SSID, sbuf, MAX_SSID_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Station SSID\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //10-STA password
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_PASSWORD), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_STA_PASSWORD, sbuf, MAX_PASSWORD_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Station Password\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);

            ESPCOM::println (F (","), output, espresponse);

            //26-Time zone
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIMEZONE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_TIMEZONE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr ( (int8_t) bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Time Zone\",\"O\":["), output, espresponse);
            for (int i = -12; i <= 12 ; i++) {
                ESPCOM::print (F ("{\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (i), output, espresponse);
                ESPCOM::print (F ("\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (i), output, espresponse);
                ESPCOM::print (F ("\"}"), output, espresponse);
                if (i < 12) {
                    ESPCOM::print (F (","), output, espresponse);
                }
            }
            ESPCOM::print (F ("]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //27- DST
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIME_ISDST), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_TIME_ISDST, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Day Saving Time\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //28- Time Server1
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIME_SERVER1), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_TIME_SERVER1, sbuf, MAX_DATA_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Time Server 1\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //29- Time Server2
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIME_SERVER2), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_TIME_SERVER2, sbuf, MAX_DATA_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Time Server 2\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //30- Time Server3
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIME_SERVER3), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_TIME_SERVER3, sbuf, MAX_DATA_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Time Server 3\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //8-wifi mode startup
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_WIFISTARTUP), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_WIFISTARTUP, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Wifi mode startup\",\"O\":[{\"ON\":\"1\"},{\"OFF\":\"0\"}]}"), output, espresponse);
            
        }

           

            //end JSON
        ESPCOM::println (F ("\n]}"), output, espresponse);

        #endif//moto_UI

// #if defined( PLC_MASTER_UI) | defined(Basic_UI)
        else if (parameter == "hardware"){
        #ifdef LOOKLINE_UI
            LOGLN("X1:" + String(analogRead(X1)) + "|X2:" + String(analogRead(X2)) + "|X3:" + String(analogRead(X3)));
            String Hardware = "{\"HARDWARE\":[";
            Hardware += "\"X0\":" + '\"' + String(analogRead(X0)) + "\",";
            Hardware += "\"X1\":" + '\"' + String(analogRead(X1)) + "\",";
            Hardware += "\"X2\":" + '\"' + String(analogRead(X2)) + "\",";
            Hardware += "\"X3\":" + '\"' + String(analogRead(X3)) + "\",";
            Hardware += "\"X4\":" + '\"' + String(analogRead(X4)) + '\"';
            Hardware += "],";
            ESPCOM::println (Hardware, output, espresponse);
        
        #endif//LOOKLINE_UI
            // cmdWic.Set_Init_UI("AUTH: 0");
        }
        
        //Start JSON
        ESPCOM::println (F ("{\"EEPROMLL\":["), output, espresponse);
        if (cmd_params == "network" || cmd_params == "") {
            CONFIG::read_byte (EP_EEPROM_ROLE, &bbuf );
        #ifdef LOOKLINE_UI     
        if(bbuf == NODE || bbuf == REPEARTER){   
            //1- Set Plan
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_PLAN), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_EEPROM_PLAN,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Plan\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_PLAN), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_PLAN), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //1- Set Plan set
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_PLAN_SET), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_EEPROM_PLAN_SET,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Plan set\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_PLAN_SET), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_PLAN_SET), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //2- Set Result
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_RESULT), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_EEPROM_RESULT,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Result\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_RESULT), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_RESULT), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //2- Set Result set
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_RESULT_SET), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_EEPROM_RESULT_SET,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Result set\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_RESULT_SET), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_RESULT_SET), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //2- Set Plan Max
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_PLANMAX), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_EEPROM_PLANMAX,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Plan max value\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_PLANMAX), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_PLANMAX), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            
            //2- Set PCS
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_PCS), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_EEPROM_PCS,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"PCS/h\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_PCS), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_PCS), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //2- Set Time plan
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_TIME_PLAN), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_EEPROM_TIME_PLAN,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Time increase of plan\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_TIME_PLAN), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_TIME_PLAN), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            }//if(bbuf == NODE || bbuf == REPEARTER){
            CONFIG::read_byte (EP_EEPROM_ROLE, &bbuf);
            if(bbuf == GATEWAY){
            //2- Set Time sent (gateway)
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_TIMESENT), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_EEPROM_TIMESENT,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Cycle time sent\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_TIMESENT), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_TIMESENT), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //2- Set Quality Node (gateway)
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_AMOUNTNODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_AMOUNTNODE, &bbuf) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Amount node\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_AMOUNTNODE), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_AMOUNTNODE), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            }
        #endif//LOOKLINE_UI
            //8-wifi mode
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_WIFI_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_WIFI_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Wifi mode\",\"O\":[{\"AP\":\"1\"},{\"STA\":\"2\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);


            //9-STA SSID
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_SSID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_STA_SSID, sbuf, MAX_SSID_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Station SSID\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //10-STA password
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_PASSWORD), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_STA_PASSWORD, sbuf, MAX_PASSWORD_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Station Password\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //16-AP SSID
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_SSID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_AP_SSID, sbuf, MAX_SSID_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Device name\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
             //20-AP Channel
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_CHANNEL), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_CHANNEL, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Wifi Channel\",\"O\":["), output, espresponse);
            for (int i = 1; i < 12 ; i++) {
                ESPCOM::print (F ("{\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (i), output, espresponse);
                ESPCOM::print (F ("\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (i), output, espresponse);
                ESPCOM::print (F ("\"}"), output, espresponse);
                if (i < 11) {
                    ESPCOM::print (F (","), output, espresponse);
                }
            }
            ESPCOM::print (F ("]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //20-Netwwork ID
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_NETID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_NETID, &bbuf) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Network ID\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_NETID), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_NETID), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
#ifndef Basic_UI
            //2- Set network ID
            ESPCOM::print (F ("{\"F\":\"rf\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_NETID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_NETID, &bbuf) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Network ID\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_NETID), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_NETID), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
#endif//Basic_UI
            //3-Web Port
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_WEB_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_WEB_PORT,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Web Port\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_WEB_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_WEB_PORT), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //4-Data Port
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_DATA_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_DATA_PORT,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Data Port\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_DATA_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_DATA_PORT), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //8-wifi mode
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_WIFI_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_WIFI_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Wifi mode\",\"O\":[{\"AP\":\"1\"},{\"STA\":\"2\"}]}"), output, espresponse);


            #ifdef Moto_UI
            ESPCOM::println (F (","), output, espresponse);
            //9-STA SSID
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_SSID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_STA_SSID, sbuf, MAX_SSID_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Station SSID\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //10-STA password
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_PASSWORD), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_STA_PASSWORD, sbuf, MAX_PASSWORD_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Station Password\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            #else//Moto_UI

            ESPCOM::println (F (","), output, espresponse);
            //11-Station Network Mode
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_PHY_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_STA_PHY_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Station Network Mode\",\"O\":[{\"11b\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11B), output, espresponse);
            ESPCOM::print (F ("\"},{\"11g\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11G), output, espresponse);
            ESPCOM::print (F ("\"},{\"11n\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11N), output, espresponse);
            ESPCOM::print (F ("\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //12-STA IP mode
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_IP_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_STA_IP_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Station IP Mode\",\"O\":[{\"DHCP\":\"1\"},{\"Static\":\"2\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //13-STA static IP
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_IP_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_STA_IP_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Station Static IP\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //14-STA static Mask
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_MASK_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_STA_MASK_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Station Static Mask\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //15-STA static Gateway
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_GATEWAY_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_STA_GATEWAY_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Station Static Gateway\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //16-AP SSID
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_SSID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_AP_SSID, sbuf, MAX_SSID_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"AP SSID\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //17-AP password
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_PASSWORD), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_AP_PASSWORD, sbuf, MAX_PASSWORD_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"AP Password\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //18 - AP Network Mode
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_PHY_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_AP_PHY_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"AP Network Mode\",\"O\":[{\"11b\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11B), output, espresponse);
            ESPCOM::print (F ("\"},{\"11g\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11G), output, espresponse);
            ESPCOM::print (F ("\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //19-AP SSID visibility
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_SSID_VISIBLE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_SSID_VISIBLE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"SSID Visible\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            
            //21-AP Authentication
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AUTH_TYPE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_AUTH_TYPE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Authentication\",\"O\":[{\"Open\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (AUTH_OPEN), output, espresponse);
            ESPCOM::print (F ("\"},{\"WPA\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (AUTH_WPA_PSK), output, espresponse);
            ESPCOM::print (F ("\"},{\"WPA2\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (AUTH_WPA2_PSK), output, espresponse);
            ESPCOM::print (F ("\"},{\"WPA/WPA2\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (AUTH_WPA_WPA2_PSK), output, espresponse);
            ESPCOM::print (F ("\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //22-AP IP mode
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_IP_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_AP_IP_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"AP IP Mode\",\"O\":[{\"DHCP\":\"1\"},{\"Static\":\"2\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //23-AP static IP
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_IP_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_AP_IP_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"AP Static IP\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //24-AP static Mask
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_MASK_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_AP_MASK_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"AP Static Mask\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //25-AP static Gateway
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_GATEWAY_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_AP_GATEWAY_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"AP Static Gateway\"}"), output, espresponse);
            #endif//Moto_UI
            ESPCOM::println (F (","), output, espresponse);
            //20-AP Channel
            ESPCOM::print (F ("{\"F\":\"wifi\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_CHANNEL), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_CHANNEL, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Wifi Channel\",\"O\":["), output, espresponse);
            for (int i = 1; i < 12 ; i++) {
                ESPCOM::print (F ("{\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (i), output, espresponse);
                ESPCOM::print (F ("\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (i), output, espresponse);
                ESPCOM::print (F ("\"}"), output, espresponse);
                if (i < 11) {
                    ESPCOM::print (F (","), output, espresponse);
                }
            }
            ESPCOM::print (F ("]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            // URL Version
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_URL_VER), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_EEPROM_URL_VER, sbuf, MAX_URL_VER_LENGTH) ) {
                ESPCOM::print ("VPlab", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_URL_VER_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"URL Version\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_URL_VER_LENGTH), output, espresponse);
            ESPCOM::println (F ("\"},"), output, espresponse);

            //URL firmware
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_URL_FW), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_EEPROM_URL_FW, sbuf, MAX_URL_FW_LENGTH) ) {
                ESPCOM::print ("VPlab", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_URL_FW_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"URL Firmware\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_URL_FW_LENGTH), output, espresponse);
            ESPCOM::println (F ("\"},"), output, espresponse);
            

            //UPDATE MODE
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_UPDATE_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_UPDATE_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Update Mode\",\"O\":[{\"Check FW\":\"1\"},{\"None\":\"0\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_ID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_ID, &bbuf) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Board ID\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_ID), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_ID), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);    
#ifdef USE_LORA
            //2- Set CHANEL
            ESPCOM::print (F ("{\"F\":\"rf\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_CHANELS), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_CHANELS, &bbuf) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            String chanel = CONFIG::getLoRaChanel();
            ESPCOM::print ("\",\"H\":\"Chanel("+ chanel +")\",\"S\":\"", output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_CHANEL), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_CHANEL), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
                //LoRa Air Rate
                ESPCOM::print (F ("{\"F\":\"rf\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_LORA_AIRRATE), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_LORA_AIRRATE, &bbuf ) ) {
                    ESPCOM::print ("Unknown", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                String airate = CONFIG::getLoRaAirate();
                ESPCOM::print ("\",\"H\":\"LoRa Air Rate("+ airate +")\",\"O\":[{\"0.3Kbps\":\"", output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Air_Rate_03), output, espresponse);
                ESPCOM::print (F ("\"},{\"1.2Kbps\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Air_Rate_12), output, espresponse);
                ESPCOM::print (F ("\"},{\"2.4Kbps\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Air_Rate_24), output, espresponse);
                ESPCOM::print (F ("\"},{\"4.8Kbps\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Air_Rate_48), output, espresponse);
                ESPCOM::print (F ("\"},{\"9.6Kbps\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Air_Rate_96), output, espresponse);
                ESPCOM::print (F ("\"},{\"19.2Kbps\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Air_Rate_192), output, espresponse);
                ESPCOM::print (F ("\"}]},"), output, espresponse);
                //LoRa Power
                ESPCOM::print (F ("{\"F\":\"rf\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_LORA_POWER), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_LORA_POWER, &bbuf ) ) {
                    ESPCOM::print ("Unknown", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                String lrpower = CONFIG::getLoRaPower();
                ESPCOM::print ("\",\"H\":\"LoRa Power("+ lrpower +")\",\"O\":[{\"30dBm\":\"", output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (0), output, espresponse);
                ESPCOM::print (F ("\"},{\"27dBm\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (1), output, espresponse);
                ESPCOM::print (F ("\"},{\"24dBm\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (2), output, espresponse);
                ESPCOM::print (F ("\"},{\"21dBm\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (3), output, espresponse);
                ESPCOM::print (F ("\"}]},"), output, espresponse);
                //LoRa Protocol
                ESPCOM::print (F ("{\"F\":\"rf\",\"P\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (EP_LORA_PROTOCOL), output, espresponse);
                ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
                if (!CONFIG::read_byte (EP_LORA_PROTOCOL, &bbuf ) ) {
                    ESPCOM::print ("Unknown", output, espresponse);
                } else {
                    ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
                }
                ESPCOM::print (F ("\",\"H\":\"LoRa Protocol\",\"O\":[{\"Transparent\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (Transparent), output, espresponse);
                ESPCOM::print (F ("\"},{\"FIXED Node\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (FIXED_Node), output, espresponse);
                ESPCOM::print (F ("\"},{\"FIXED Gateway\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (FIXED_Gateway), output, espresponse);
                ESPCOM::print (F ("\"},{\"WOR Node\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (WOR_Node), output, espresponse);
                ESPCOM::print (F ("\"},{\"WOR Gateway\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (WOR_Gateway), output, espresponse);
                ESPCOM::print (F ("\"},{\"Broadcast 1\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (BROADCAST_MESSAGE1), output, espresponse);
                ESPCOM::print (F ("\"},{\"Broadcast 2\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (BROADCAST_MESSAGE2), output, espresponse);
                ESPCOM::print (F ("\"},{\"Broadcast 3\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (BROADCAST_MESSAGE3), output, espresponse);
                ESPCOM::print (F ("\"}]}"), output, espresponse);
            //ROLE
            ESPCOM::print (F (",{\"F\":\"rf\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_ROLE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_ROLE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Device Role\",\"O\":[{\"Node\":\"0\"},{\"Gateway\":\"1\"},{\"Repeater\":\"2\"}]}"), output, espresponse);
           
            ESPCOM::println (F (","), output, espresponse);
            //COM Mode
            ESPCOM::print (F ("{\"F\":\"rf\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_COM_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_COM_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Com mode\",\"O\":[{\"LoRa\":\"0\"},{\"Mesh Wifi\":\"1\"},{\"MQTT\":\"2\"},{\"RS485\":\"3\"}]}"), output, espresponse);
           
            ESPCOM::println (F (","), output, espresponse);
            //Module type
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_MODULE_TYPE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_MODULE_TYPE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Module type\",\"O\":[{\"Auto Detect\":\"0\"},{\"Lookline Gateway V14\":\"1\"},{\"LED7 seg Board V13.0\":\"4\"},{\"LED7 seg Board V14.0\":\"2\"},{\"LED7 seg Board V14.1\":\"3\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
           
           
            //DEBUG
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_DEBUG), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_DEBUG, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Device debug\",\"O\":[{\"Debug\":\"1\"},{\"NoDebug\":\"0\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);    
            //UPDATE MODE
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_EEPROM_UPDATE_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_EEPROM_UPDATE_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Update Mode\",\"O\":[{\"Check FW\":\"1\"},{\"None\":\"0\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);    
#endif//USE_LORA
            //1- Baud Rate
            ESPCOM::print (F ("{\"F\":\"rf\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_BAUD_RATE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_BAUD_RATE,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Baud Rate\",\"O\":[{\"9600\":\"9600\"},{\"19200\":\"19200\"},{\"38400\":\"38400\"},{\"57600\":\"57600\"},{\"115200\":\"115200\"},{\"230400\":\"230400\"},{\"250000\":\"250000\"},{\"500000\":\"500000\"},{\"921600\":\"921600\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);  
            #ifdef MODBUS_TCP  
            //23-Modbus IP
            ESPCOM::print (F ("{\"F\":\"tcp\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MODBUS_IP_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_MODBUS_IP_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Modbus IP\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //24-Modbus static Mask
            ESPCOM::print (F ("{\"F\":\"tcp\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MODBUS_MASK_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_MODBUS_MASK_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Modbus Mask\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //25-Modbus static Gateway
            ESPCOM::print (F ("{\"F\":\"tcp\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MODBUS_GATEWAY_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_MODBUS_GATEWAY_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Modbus Gateway\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //4-Modbus Port
            ESPCOM::print (F ("{\"F\":\"tcp\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_MODBUS_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_MODBUS_PORT,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Modbus Port\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_DATA_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_DATA_PORT), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            #endif//MODBUS_TCP
            
           //FLAG Hardware         
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_OUTPUT_FLAG), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"F\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_OUTPUT_FLAG, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            String s = "\",\"H\":\"Output msg\",\"O\":[{\"M117\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_M117);
            s+= "\"}";
          
#ifdef ESP_OLED_FEATURE
            s+=",{\"Oled\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_OLED);
            s+="\"}";
#endif
            s+=",{\"Serial\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_SERIAL);
            s+="\"}";
#ifdef WS_DATA_FEATURE
            s+=",{\"Web Socket\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_WSOCKET);
            s+="\"}";
#endif
#ifdef TCP_IP_DATA_FEATURE
            s+=",{\"TCP\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_TCP);
            s+="\"}";
#endif
            s+= "]}";
            ESPCOM::print (s, output, espresponse);

        }

        ESPCOM::println (F ("\n],"), output, espresponse);
        byte role_ = 0;byte run_ = 0;
        #ifndef Basic_UI
        CONFIG::read_byte(EP_EEPROM_ROLE, &role_);
        CONFIG::read_byte(EP_EEPROM_RUN, &run_);
        if(run_ > 1) {CONFIG::write_byte(EP_EEPROM_RUN, 1);run_ = 1;}
        #endif//Basic_UI
        if(role_ == 1){ESPCOM::println (F ("\"ROLE\":\"1\""), output, espresponse);}
            else{ESPCOM::println (F ("\"ROLE\":\"0\""), output, espresponse);}
        if(auth_val == 2){ESPCOM::println (F (",\"AUTH\": \"2\""), output, espresponse);}
        else if(auth_val == 1){ESPCOM::println (F (",\"AUTH\": \"1\""), output, espresponse);}
        else             {ESPCOM::println (F (",\"AUTH\": \"0\""), output, espresponse);}
        // if(Lookline_PROG.GetFW() == 1){ESPCOM::println (F (",\"FW\":\"1\""), output, espresponse);}
        //     else{ESPCOM::println (F (",\"FW\":\"0\""), output, espresponse);}
        //end JSON
        ESPCOM::println (F ("}"), output, espresponse);

        #ifdef PLC_MASTER_UI
        PLC_cmd.connectWeb(1);
        #endif//PLC_MASTER_UI
        // Lookline_PROG.SetStart(2);
// #endif //PLC

    }

    break;

    //[ESP401]P=<position> T=<type> V=<value> pwd=<user/admin password>
    case 401: {
        //check validity of parameters
        String spos = get_param (cmd_params, "P=", false);
        String styp = get_param (cmd_params, "T=", false);
        String sval = get_param (cmd_params, "V=", true);
        sval.trim();
        int pos = spos.toInt();
        if ( (pos == 0 && spos != "0") || (pos > LAST_EEPROM_ADDRESS || pos < 0) ) {
            response = false;
        }
        if (! (styp == "B" || styp == "S" || styp == "A" || styp == "I" || styp == "F") ) {
            response = false;
        }
        if ((sval.length() == 0) && !((pos==EP_AP_PASSWORD) || (pos==EP_STA_PASSWORD) || (pos==EP_MQTT_BROKER) || (pos==EP_MQTT_USER) || (pos==EP_MQTT_PASS))) {
            response = false;
        }


#ifdef AUTHENTICATION_FEATURE
        if (response) {
            //check authentication
            level_authenticate_type auth_need = LEVEL_ADMIN;
            for (int i = 0; i < AUTH_ENTRY_NB; i++) {
                if (Setting[i][0] == pos ) {
                    auth_need = (level_authenticate_type) (Setting[i][1]);
                    i = AUTH_ENTRY_NB;
                }
            }
            if ( (auth_need == LEVEL_ADMIN && auth_type == LEVEL_USER) || (auth_type == LEVEL_GUEST) ) {
                response = false;
            }
        }
#endif
        if (response) {
            if ((styp == "B")  ||  (styp == "F")) {
                byte bbuf = sval.toInt();
                if (!CONFIG::write_byte (pos, bbuf) ) {
                    response = false;
                } else {
                    //dynamique refresh is better than restart the board
                    if (pos == EP_OUTPUT_FLAG) {
                        CONFIG::output_flag = bbuf;
                    }
                    else if (pos == EP_TARGET_FW) {
                        CONFIG::InitFirmwareTarget();
                    }
#ifdef LOOKLINE_UI
                    else if(pos == EP_EEPROM_MODULE_TYPE){
                        LooklineCMD.PinMapInit();
                    }
                    else{
                    LooklineCMD.LookLineInitB(pos, bbuf);
                    }
#endif//LOOKLINE_UI
#ifdef DHT_FEATURE
                    if (pos == EP_DHT_TYPE) {
                        CONFIG::DHT_type = bbuf;
                        CONFIG::InitDHT(true);
                    }
#endif
#ifdef NOTIFICATION_FEATURE
                    if (pos == ESP_AUTO_NOTIFICATION) {
                        notificationsservice.setAutonotification ((bbuf == 0)? false: true);
                    }
#endif
#if defined(TIMESTAMP_FEATURE)
                    if ( (pos == EP_TIMEZONE) || (pos == EP_TIME_ISDST) || (pos == EP_TIME_SERVER1) || (pos == EP_TIME_SERVER2) || (pos == EP_TIME_SERVER3) ) {
                        CONFIG::init_time_client();
                    }
#endif
#ifdef Valve_UI
                    if(pos == EP_LORA_AIRRATE || pos == EP_LORA_RSSI || pos == EP_LORA_MASTER || pos == EP_LORA_PROTOCOL){
                        loraSetup();
                    }
                    if(pos == EP_LORA_T_SCAN || pos == EP_LORA_T_REQUEST){
                        TimeSetup();
                    }
#endif//Valve_UI                    
                }
            }
            if (styp == "I") {
                int ibuf = sval.toInt();
                if (!CONFIG::write_buffer (pos, (const byte *) &ibuf, INTEGER_LENGTH) ) {
                    response = false;
                } else {
#ifdef DHT_FEATURE
                    if (pos == EP_DHT_INTERVAL) {
                        CONFIG::DHT_interval = ibuf;
                    }
#endif
//LooklineCMD.PinMapInit()
#ifdef Valve_UI
                if(pos == EP_LORA_CHANEL){
                    loraSetup();//LOG ("EP_LORA_CHANEL:" + String(ibuf));
                }
#endif//Valve_UI
#ifdef LOOKLINE_UI
                
                LooklineCMD.LookLineInitI(pos,ibuf);
                
#endif//LOOKLINE_UI
              }
            }
            if (styp == "S") {
                if (!CONFIG::write_string (pos, sval.c_str() ) ) {
                    response = false;
                }
                #ifdef MQTT_USE
                if(pos == EP_MQTT_BROKER || pos == EP_MQTT_USER || pos == EP_MQTT_PASS ){
                    commandMQTT.update();
                }
                #endif//MQTT_USE
            }
            if (styp == "A") {
                byte ipbuf[4];
                if (CONFIG::split_ip (sval.c_str(), ipbuf) < 4) {
                    response = false;
                } else if (!CONFIG::write_buffer (pos, ipbuf, IP_LENGTH) ) {
                    response = false;
                }
            }
        }
        if (!response) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
        } else {
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
        }

    }
    break;
    //update new firmware form host
    //[ESP403]cmd=readfile 
    case 403: {
        parameter = get_param (cmd_params, "cmd=", true);
        if (parameter == "update") {
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
            ESPCOM::println (F ("DHT:update fw"), output, espresponse);//Serial.println
            LOGLN();LOGLN("Update Firmware");UDFWCmd.FirmwareUpdate();
        }
        #ifdef LOOKLINE_UI
        if (parameter == "off") {
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
            Lookline_PROG.SetRun(2);
            ESPCOM::println (F ("DHT:{\"status\":\"off Lookline/On mesh Gateway\"}"), output, espresponse);
        }
        if (parameter == "run") {
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
            // ESPCOM::println (F ("Run"), output, espresponse);
            Lookline_PROG.SetRun(1);
            // Lookline_PROG.SetConfig(2);
            // Lookline_PROG.SetStart(0);
            ESPCOM::println ("DHT:{\"RUN\":1}", output, espresponse);
        }
        if (parameter == "stop") {
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
            // ESPCOM::println (F ("stop"), output, espresponse);
            Lookline_PROG.SetRun(0);
            // Lookline_PROG.SetConfig(0);
            ESPCOM::println ("DHT:{\"RUN\":0}", output, espresponse);
        }
        if (parameter == "WifiMain") {
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
            ESPCOM::println (F ("DHT:{\"WifiMain\":1}"), output, espresponse);
            Lookline_PROG.SetRun(3);
            // Lookline_PROG.SetConfig(0);
        }
        if (parameter == "WifiMainOff") {
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
            ESPCOM::println (F ("DHT:{\"WifiMainOff\":1}"), output, espresponse);
            Lookline_PROG.SetRun(4);
            // Lookline_PROG.SetConfig(0);
        }
        if (parameter == "clear") {
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
            ESPCOM::println (F ("DHT:{\"status\":\"clear\"}"), output, espresponse);
            Lookline_PROG.SetRun(5);
            // Lookline_PROG.SetConfig(0);
        }
        #endif//LOOKLINE_UI
        #ifdef PLC_MASTER_UI
        parameter = get_param (cmd_params, "cmd=", false);
        String IDparameter = "";
        if (parameter == "readfile") {
            ESPCOM::println (F ("read file"), output, espresponse);//Serial.println
            #ifdef PLC_MASTER_UI
            PLC_cmd.readfile();
            #endif//PLC_MASTER_UI
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
        }
        // LOGLN("recive [ESP403]" + parameter);
        if (parameter == "run") {IDparameter = get_param (cmd_params, "id=", true);cmd_modbus.modbusSet((uint16_t)IDparameter.toInt(), 1);//LOGLN("Run|ID:"+String(IDparameter));
            ESPCOM::println (OK_CMD_MSG, output, espresponse);}
        if (parameter == "stop") {IDparameter = get_param (cmd_params, "id=", true);cmd_modbus.modbusSet((uint16_t)IDparameter.toInt(), 2);//LOGLN("Stop|ID:"+String(IDparameter));
            ESPCOM::println (OK_CMD_MSG, output, espresponse);}
        if (parameter == "write") {IDparameter = get_param (cmd_params, "id=", false);String Valueparameter = get_param (cmd_params, "value=", false);
            cmd_modbus.modbusSet((uint16_t)IDparameter.toInt(), (uint16_t)Valueparameter.toInt());
            ESPCOM::println (OK_CMD_MSG, output, espresponse);}
        #endif//PLC_MASSTER_UI
    }
    break;
    

    //[ESP407]//Unlock Administrator
    case 407: {
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
        if (parameter == "Admin") {

            // ESPCOM::println (F ("Đăng nhập Admin"), output, espresponse);
        #ifdef LOOKLINE_UI
            Lookline_PROG.Set_Init_UI("{\"AUTH\": \"2\" }");
        #endif//LOOKLINE_UI
            String Cmd = "{\"AUTH\": \"2\" }";
            ESPCOM::println (Cmd, output, espresponse);
            // ESPCOM::println ("{\"RUN\":1}", output, espresponse);
        }
        else if (parameter == "2709") {

            // ESPCOM::println (F ("Đăng nhập bảo trì"), output, espresponse);
        #ifdef LOOKLINE_UI
            Lookline_PROG.Set_Init_UI("{\"AUTH\": \"1\" }");
        #endif//LOOKLINE_UI
            String Cmd = "{\"AUTH\": \"1\" }";
            ESPCOM::println (Cmd, output, espresponse);
        }
        else if (parameter == "hardware"){
        #ifdef LOOKLINE_UI
            LOGLN("X1:" + String(analogRead(X1)) + "|X2:" + String(analogRead(X2)) + "|X3:" + String(analogRead(X3)));    
            String Hardware = "{\"HARDWARE\":[";
            Hardware += "\"X0\":" + '\"' + String(analogRead(X0)) + "\",";
            Hardware += "\"X1\":" + '\"' + String(analogRead(X1)) + "\",";
            Hardware += "\"X2\":" + '\"' + String(analogRead(X2)) + "\",";
            Hardware += "\"X3\":" + '\"' + String(analogRead(X3)) + "\",";
            Hardware += "\"X4\":" + '\"' + String(analogRead(X4)) + '\"';
            Hardware += "]}";
            ESPCOM::println (Hardware, output, espresponse);
        #endif//LOOKLINE_UI
            // cmdWic.Set_Init_UI("AUTH: 0");
        }
    }
    break;
    


    //[ESP409]//Reset URL
    case 409: {
        #ifdef LOOKLINE_UI
        String URL_FW = "http://";
        CONFIG::write_string (EP_EEPROM_URL_FW, URL_FW.c_str() ) ;
        CONFIG::write_string (EP_EEPROM_URL_VER, URL_FW.c_str() ) ;
        #endif//LOOKLINE_UI
        String response = "{\"Statup\":\"1\"}";
        ESPCOM::println (response, output, espresponse);
        ESPCOM::println (OK_CMD_MSG, output, espresponse);

    }
    break;
    //Get available AP list (limited to 30)
    //output is JSON or plain text according parameter
    //[ESP410]<plain>
    case 410: {
        parameter = get_param (cmd_params, "", true);
        bool plain = (parameter == "plain");

#if defined(ASYNCWEBSERVER)
        if (!plain) {
            ESPCOM::print (F ("{\"AP_LIST\":["), output, espresponse);
        }
        int n = WiFi.scanComplete();
        if (n == -2) {
            WiFi.scanNetworks (ESP_USE_ASYNC);
        } else if (n) {
#else
        int n =  WiFi.scanNetworks ();
        if (!plain) {
            ESPCOM::print (F ("{\"AP_LIST\":["), output, espresponse);
        }
#endif

            for (int i = 0; i < n; ++i) {
                if (i > 0) {
                    if (!plain) {
                        ESPCOM::print (F (","), output, espresponse);
                    } else {
                        ESPCOM::print (F ("\n"), output, espresponse);
                    }
                }
                if (!plain) {
                    ESPCOM::print (F ("{\"SSID\":\""), output, espresponse);
                    ESPCOM::print (encodeString(WiFi.SSID (i).c_str()), output, espresponse);
                } else ESPCOM::print (WiFi.SSID (i).c_str(), output, espresponse);
                if (!plain) {
                    ESPCOM::print (F ("\",\"SIGNAL\":\""), output, espresponse);
                } else {
                    ESPCOM::print (F ("\t"), output, espresponse);
                }
                ESPCOM::print (CONFIG::intTostr (wifi_config.getSignal (WiFi.RSSI (i) ) ), output, espresponse);;
                //ESPCOM::print(F("%"), output, espresponse);
                if (!plain) {
                    ESPCOM::print (F ("\",\"IS_PROTECTED\":\""), output, espresponse);
                }
                if (WiFi.encryptionType (i) == ENC_TYPE_NONE) {
                    if (!plain) {
                        ESPCOM::print (F ("0"), output, espresponse);
                    } else {
                        ESPCOM::print (F ("\tOpen"), output, espresponse);
                    }
                } else {
                    if (!plain) {
                        ESPCOM::print (F ("1"), output, espresponse);
                    } else {
                        ESPCOM::print (F ("\tSecure"), output, espresponse);
                    }
                }
                if (!plain) {
                    ESPCOM::print (F ("\"}"), output, espresponse);
                }
            }
            WiFi.scanDelete();
#if defined(ASYNCWEBSERVER)
            if (WiFi.scanComplete() == -2) {
                WiFi.scanNetworks (ESP_USE_ASYNC);
            }
        }
#endif
        if (!plain) {
            ESPCOM::print (F ("]}"), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }
    }
    break;
#endif //USE_AS_UPDATER_ONLY
    //Get ESP current status in plain or JSON
    //[ESP420]<plain>
    case 420: {
        parameter = get_param (cmd_params, "", true);
        CONFIG::print_config (output, (parameter == "plain"), espresponse);
    }
    break;
    //Set ESP mode
    //cmd is RESET, SAFEMODE, RESTART
    //[ESP444]<cmd>pwd=<admin password>
    case 444:
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
        {
            if (parameter == "RESET") {
                CONFIG::reset_config();
                ESPCOM::println (F ("Reset done - restart needed"), output, espresponse);
            } else if (parameter == "SAFEMODE") {
                wifi_config.Safe_Setup();
                ESPCOM::println (F ("Set Safe Mode  - restart needed"), output, espresponse);
            } else  if (parameter == "RESTART") {
                ESPCOM::println (F ("Restart started"), output, espresponse);
                web_interface->restartmodule = true;
            } else {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            }
        }
        break;
#ifndef USE_AS_UPDATER_ONLY
    //[ESP500]<gcode>
    case 500: { //send GCode with check sum caching right line numbering
        //be sure serial is locked
        if ( (web_interface->blockserial) ) {
            break;
        }
        int32_t linenb = 1;
        cmd_params.trim() ;
        if (sendLine2Serial (cmd_params, linenb,  &linenb)) {
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
        } else { //it may failed because of skip if repetier so let's reset numbering first
            if ( ( CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER) ) {
                //reset numbering
                String cmd = "M110 N0";
                if (sendLine2Serial (cmd, -1,  NULL)) {
                    linenb = 1;
                    //if success let's try again to send the command
                    if (sendLine2Serial (cmd_params, linenb,  &linenb)) {
                        ESPCOM::println (OK_CMD_MSG, output, espresponse);
                    } else {
                        ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                        response = false;
                    }
                } else {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    response = false;
                }
            } else {

                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                response = false;
            }
        }
    }
    break;
    //[ESP501]<line>
    case 501: { //send line checksum
        cmd_params.trim();
        int8_t chk = Checksum(cmd_params.c_str(),cmd_params.length());
        String schecksum = "Checksum: " + String(chk);
        ESPCOM::println (schecksum, output, espresponse);
    }
#ifdef AUTHENTICATION_FEATURE
    //Change / Reset user password
    //[ESP555]<password>pwd=<admin password>
    case 555: {
        if (auth_type == LEVEL_ADMIN) {
            parameter = get_param (cmd_params, "", true);
            if (parameter.length() == 0) {
                if (CONFIG::write_string (EP_USER_PWD, FPSTR (DEFAULT_USER_PWD) ) ) {
                    ESPCOM::println (OK_CMD_MSG, output, espresponse);
                } else {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    response = false;
                }
            } else {
                if (CONFIG::isLocalPasswordValid (parameter.c_str() ) ) {
                    if (CONFIG::write_string (EP_USER_PWD, parameter.c_str() ) ) {
                        ESPCOM::println (OK_CMD_MSG, output, espresponse);
                    } else {
                        ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                        response = false;
                    }
                } else {
                    ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                    response = false;
                }
            }
        } else {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
        break;
    }
#endif
#ifdef NOTIFICATION_FEATURE
    //Send Notification
    //[ESP600]msg [pwd=<admin password>]
    case 600:
#ifdef AUTHENTICATION_FEATURE
        if (auth_type == LEVEL_GUEST) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            return false;
        }
#endif
        parameter = get_param (cmd_params, "", true);
        if (parameter.length() == 0) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            return false;
        }
        if (notificationsservice.sendMSG("ESP3D Notification", parameter.c_str())) {
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
        } else {
            ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
            response = false;
        }
        break;
    //Set/Get Notification settings
    //[ESP610]type=<NONE/PUSHOVER/EMAIL/LINE/IFTTT> T1=<token1> T2=<token2> TS=<Settings> [pwd=<admin password>]
    //Get will give type and settings only not the protected T1/T2
    case 610:
#ifdef AUTHENTICATION_FEATURE
        if (auth_type == LEVEL_GUEST) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            return false;
        }
#endif
        parameter = get_param (cmd_params, "", false);
        //get
        if (parameter.length() == 0) {
            uint8_t Ntype =  0;
            if (!CONFIG::read_byte (ESP_NOTIFICATION_TYPE, &Ntype ) ) {
                Ntype =0;
            }
            char sbuf[MAX_DATA_LENGTH + 1];
            static String tmp;
            tmp = (Ntype == ESP_PUSHOVER_NOTIFICATION)?"PUSHOVER":(Ntype == ESP_EMAIL_NOTIFICATION)?"EMAIL":(Ntype == ESP_LINE_NOTIFICATION)?"LINE":(Ntype == ESP_IFTTT_NOTIFICATION)?"IFTTT":"NONE";
            if (CONFIG::read_string (ESP_NOTIFICATION_SETTINGS, sbuf, MAX_NOTIFICATION_SETTINGS_LENGTH) ) {
                tmp+= " ";
                tmp += sbuf;
            }
            ESPCOM::println (tmp.c_str(), output, espresponse);
        } else {
            response = false;
            //type
            parameter = get_param (cmd_params, "type=");
            if (parameter.length() > 0) {
                uint8_t Ntype;
                parameter.toUpperCase();
                if (parameter == "NONE") {
                    Ntype = 0;
                } else if (parameter == "PUSHOVER") {
                    Ntype = ESP_PUSHOVER_NOTIFICATION;
                } else if (parameter == "EMAIL") {
                    Ntype = ESP_EMAIL_NOTIFICATION;
                } else if (parameter == "LINE") {
                    Ntype = ESP_LINE_NOTIFICATION;
                } else if (parameter == "IFTTT") {
                    Ntype = ESP_IFTTT_NOTIFICATION;    
                } else {
                    ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                    return false;
                }
                if (!CONFIG::write_byte (ESP_NOTIFICATION_TYPE, Ntype) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    return false;
                } else {
                    response = true;
                }
            }
            //Settings
            parameter = get_param (cmd_params, "TS=");
            if (parameter.length() > 0) {
                if (!CONFIG::write_string (ESP_NOTIFICATION_SETTINGS, parameter.c_str() ) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    return false;
                } else {
                    response = true;
                }
            }
            //Token1
            parameter = get_param (cmd_params, "T1=");
            if (parameter.length() > 0) {
                if (!CONFIG::write_string (ESP_NOTIFICATION_TOKEN1, parameter.c_str() ) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    return false;
                } else {
                    response = true;
                }
            }
            //Token2
            parameter = get_param (cmd_params, "T2=");
            if (parameter.length() > 0) {
                if (!CONFIG::write_string (ESP_NOTIFICATION_TOKEN2, parameter.c_str() ) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    return false;
                } else {
                    response = true;
                }
            }
            if (response) {
                //Restart service
                notificationsservice.begin();
                ESPCOM::println (OK_CMD_MSG, output, espresponse);
            } else {
                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
            }
        }
        break;
#endif
    //[ESP700]<filename>
    case 700: { //read local file
        //be sure serial is locked
        if ( (web_interface->blockserial) ) {
            break;
        }
        cmd_params.trim() ;
        if ( (cmd_params.length() > 0) && (cmd_params[0] != '/') ) {
            cmd_params = "/" + cmd_params;
        }
        FS_FILE currentfile = SPIFFS.open (cmd_params, SPIFFS_FILE_READ);
        if (currentfile) {//if file open success
            //flush to be sure send buffer is empty
            ESPCOM::flush (DEFAULT_PRINTER_PIPE);
            //until no line in file
            while (currentfile.available()) {
                String currentline = currentfile.readStringUntil('\n');
                currentline.replace("\n","");
                currentline.replace("\r","");
                currentline.trim();
                if (currentline.length() > 0) {
                    int ESPpos = currentline.indexOf ("[ESP");
                    if (ESPpos ==0) {
                        //is there the second part?
                        int ESPpos2 = currentline.indexOf ("]", ESPpos);
                        if (ESPpos2 > -1) {
                            //Split in command and parameters
                            String cmd_part1 = currentline.substring (ESPpos + 4, ESPpos2);
                            String cmd_part2 = "";
                            //is there space for parameters?
                            if ((uint)ESPpos2 < currentline.length() ) {
                                cmd_part2 = currentline.substring (ESPpos2 + 1);
                            }
                            //if command is a valid number then execute command
                            if(cmd_part1.toInt()!=0) {
                                execute_command(cmd_part1.toInt(),cmd_part2,NO_PIPE, auth_type, espresponse);
                            }
                            //if not is not a valid [ESPXXX] command ignore it
                        }
                    } else {
                        //send line to serial
                        ESPCOM::println (currentline, DEFAULT_PRINTER_PIPE);
                        CONFIG::wait (1);
                        //flush to be sure send buffer is empty
                        ESPCOM::flush (DEFAULT_PRINTER_PIPE);
                    }
                    CONFIG::wait (1);
                }
            }
            currentfile.close();
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
        } else {
            ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
            response = false;
        }

        break;
    }
    //Format SPIFFS
    //[ESP710]FORMAT pwd=<admin password>
    case 710:
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
        {
            if (parameter == "FORMAT") {
                ESPCOM::print (F ("Formating"), output, espresponse);
                SPIFFS.format();
                ESPCOM::println (F ("...Done"), output, espresponse);
            } else {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            }
        }
        break;
    //SPIFFS total size and used size
    //[ESP720]<header answer>
    case 720:
        ESPCOM::print (cmd_params, output, espresponse);
#ifdef ARDUINO_ARCH_ESP8266
        fs::FSInfo info;
        SPIFFS.info (info);
        ESPCOM::print ("SPIFFS Total:", output, espresponse);
        ESPCOM::print (CONFIG::formatBytes (info.totalBytes).c_str(), output, espresponse);
        ESPCOM::print (" Used:", output, espresponse);
        ESPCOM::println (CONFIG::formatBytes (info.usedBytes).c_str(), output, espresponse);
#else
        ESPCOM::print ("SPIFFS  Total:", output, espresponse);
        ESPCOM::print (CONFIG::formatBytes (SPIFFS.totalBytes() ).c_str(), output, espresponse);
        ESPCOM::print (" Used:", output, espresponse);
        ESPCOM::println (CONFIG::formatBytes (SPIFFS.usedBytes() ).c_str(), output, espresponse);
#endif
        break;
#endif //USE_AS_UPDATER_ONLY
    //get fw version firmare target and fw version
    //[ESP800]<header answer>
    case 800: {
        // Serial.begin(115200);
#ifdef PLC_MASTER_UI
        PLC_cmd.connectWeb(0);
        #endif//PLC_MASTER_UI
        byte sd_dir = 0;
        String shost ;
        if (!CONFIG::read_string (EP_HOSTNAME, shost, MAX_HOSTNAME_LENGTH) ) {
            shost = wifi_config.get_default_hostname();
        }
        ESPCOM::print (cmd_params, output, espresponse);
        ESPCOM::print (F ("FW version:"), output, espresponse);
        ESPCOM::print (FW_VERSION, output, espresponse);
        ESPCOM::print (F (" # FW target:"), output, espresponse);
        ESPCOM::print (CONFIG::GetFirmwareTargetShortName(), output, espresponse);
        ESPCOM::print (F (" # FW HW:"), output, espresponse);
        if (CONFIG::is_direct_sd) {
            ESPCOM::print (F ("Direct SD"), output, espresponse);
        } else {
            ESPCOM::print (F ("Serial SD"), output, espresponse);
        }
        ESPCOM::print (F (" # primary sd:"), output, espresponse);
        if (!CONFIG::read_byte (EP_PRIMARY_SD, &sd_dir ) ) {
            sd_dir = DEFAULT_PRIMARY_SD;
        }
        if (sd_dir == SD_DIRECTORY) {
            ESPCOM::print (F ("/sd/"), output, espresponse);
        } else if (sd_dir == EXT_DIRECTORY) {
            ESPCOM::print (F ("/ext/"), output, espresponse);
        } else {
            ESPCOM::print (F ("none"), output, espresponse);
        }
        ESPCOM::print (F (" # secondary sd:"), output, espresponse);
        if (!CONFIG::read_byte (EP_SECONDARY_SD, &sd_dir ) ) {
            sd_dir = DEFAULT_SECONDARY_SD;
        }
        if (sd_dir == SD_DIRECTORY) {
            ESPCOM::print (F ("/sd/"), output, espresponse);
        } else if (sd_dir == EXT_DIRECTORY) {
            ESPCOM::print (F ("/ext/"), output, espresponse);
        } else {
            ESPCOM::print (F ("none"), output, espresponse);
        }
        
        // #ifdef Gyro_UI
        // if (!CONFIG::read_byte (GyroState, &CONFIG::GetState() ) ) {
        //     CONFIG::GetState() = DEFAULT_GYRO_STATE;
        // }
        // #endif//Gyro_UI
        ESPCOM::print (F (" # authentication:"), output, espresponse);
#ifdef AUTHENTICATION_FEATURE
        ESPCOM::print (F ("yes"), output, espresponse);
#else
        ESPCOM::print (F ("no"), output, espresponse);
#endif
        ESPCOM::print (F (" # webcommunication:"), output, espresponse);
#if defined (ASYNCWEBSERVER)
        ESPCOM::print (F ("Async"), output, espresponse);
#else
        ESPCOM::print (F ("Sync:"), output, espresponse);
        String sp = String(wifi_config.iweb_port+1);
        sp += ":";
        if ((WiFi.getMode() == WIFI_STA) || (WiFi.getMode() == WIFI_AP_STA)) {
            // LOG("\r\n sta mode\r\n")
            if(WiFi.localIP().toString() == "0.0.0.0"){sp += WiFi.softAPIP().toString();}
             else{sp += WiFi.localIP().toString();}
        } else if (WiFi.getMode() == WIFI_AP) {
            // LOG("\r\nap mode\r\n")
            // LOGLN(WiFi.softAPIP().toString());
             sp += WiFi.softAPIP().toString();
        } else {
            LOG("\rdon't know mode\r\n")
             sp += "192.168.0.1";
        }
        // LOGLN(sp);
        ESPCOM::print (sp.c_str(), output, espresponse);
#endif

        ESPCOM::print (F (" # hostname:"), output, espresponse);
        ESPCOM::print (shost, output, espresponse);
        if (WiFi.getMode() == WIFI_AP) {
            ESPCOM::print (F("(AP mode)"), output, espresponse);
        }

        ESPCOM::println ("", output, espresponse);
    }
    break;
#ifndef USE_AS_UPDATER_ONLY
    //get fw target
    //[ESP801]<header answer>
    case 801:
        ESPCOM::print (cmd_params, output, espresponse);
        ESPCOM::println (CONFIG::GetFirmwareTargetShortName(), output, espresponse);
        break;
    case 810:
        web_interface->blockserial = false;
        break;
    case 900:
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type == LEVEL_GUEST) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
#endif
        if (parameter.length() == 0) {
            if (CONFIG::is_com_enabled) {
                ESPCOM::print (F ("ENABLED"), output, espresponse);
            } else {
                ESPCOM::print (F ("DISABLED"), output, espresponse);
            }
        } else {
            if (parameter == "ENABLE") {
                CONFIG::DisableSerial();
                 if (!CONFIG::InitBaudrate()){
                     ESPCOM::print (F ("Cannot enable serial communication"), output, espresponse);
                 } else {
                     ESPCOM::print (F ("Enable serial communication"), output, espresponse);
                 }
            } else if (parameter == "DISABLE") {
                ESPCOM::print (F ("Disable serial communication"), output, espresponse);
                CONFIG::DisableSerial();
            } else {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            }
        }
        break;
#endif //USE_AS_UPDATER_ONLY
    default:
        ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
        response = false;
        break;
    }
    return response;
}

bool COMMAND::check_command (String buffer, tpipe output, bool handlelockserial, bool executecmd)
{
    String buffer2;
    LOG ("Check Command:")
    LOG (buffer)
    LOG ("\r\n")
    bool is_temp = false;
    if ( (buffer.indexOf ("T:") > -1 ) || (buffer.indexOf ("B:") > -1 ) ) {
        is_temp = true;
    }
    if ( ( CONFIG::GetFirmwareTarget()  == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER) ) {
        //save time no need to continue
        if ( (buffer.indexOf ("busy:") > -1) || (buffer.startsWith ("wait") ) ) {
            return false;
        }
        if (buffer.startsWith ("ok") ) {
            return false;
        }
    } else  if (buffer.startsWith ("ok") && buffer.length() < 4) {
        return false;
    }else  if (buffer.startsWith ("Wifi")) {
        #ifdef LOOKLINE_UI
        Lookline_PROG.SetConfig(0);
        #endif//LOOOKLINE_UI
        return true;
    }else  if (buffer.startsWith ("Mesh")) {
        #ifdef LOOKLINE_UI
        Lookline_PROG.SetConfig(1);
        #endif//LOOOKLINE_UI
        return true;
    }else  if (buffer.startsWith ("Reset")) {
        CONFIG::reset_config();
        return true;
    }else  if (buffer.startsWith ("Restart")) {
        web_interface->restartmodule = true;
    }//


#ifdef SERIAL_COMMAND_FEATURE
    if (executecmd) {
        String ESP_Command;
        int ESPpos = -1;
#ifdef MKS_TFT_FEATURE
        if (buffer.startsWith("at+")) {
            //echo
            ESPCOM::print (buffer, output);
            ESPCOM::print ("\r\r\n", output);
            if (buffer.startsWith("at+net_wanip=?")) {
                String ipstr;
                if ((WiFi.getMode() == WIFI_STA) || (WiFi.getMode() == WIFI_AP_STA)) {
                    ipstr = WiFi.localIP().toString() + "," + WiFi.subnetMask().toString()+ "," + WiFi.gatewayIP().toString()+"\r\n";
                } else {
                    ipstr = WiFi.softAPIP().toString() + ",255.255.255.0," + WiFi.softAPIP().toString()+"\r\n";
                }
                ESPCOM::print (ipstr, output);
            } else if (buffer.startsWith("at+wifi_ConState=?")) {
                ESPCOM::print ("Connected\r\n", output);
            } else {
                ESPCOM::print ("ok\r\n", output);
            }
            return false;
        }
#endif
        ESPpos = buffer.indexOf ("[ESP");
        if (ESPpos == -1 && (CONFIG::GetFirmwareTarget() == SMOOTHIEWARE)) {
            ESPpos = buffer.indexOf ("[esp");
        }
        if ((ESPpos > -1) && (ESPpos< (int)strlen("echo: " ))){
            //is there the second part?
            int ESPpos2 = buffer.indexOf ("]", ESPpos);
            if (ESPpos2 > -1) {
                //Split in command and parameters
                String cmd_part1 = buffer.substring (ESPpos + 4, ESPpos2);
                String cmd_part2 = "";
                //is there space for parameters?
                if ((uint)ESPpos2 < buffer.length() ) {
                    cmd_part2 = buffer.substring (ESPpos2 + 1);
                }
                //if command is a valid number then execute command
                if (cmd_part1.toInt() != 0) {
                    execute_command (cmd_part1.toInt(), cmd_part2, output);
                }
                //if not is not a valid [ESPXXX] command
            }
        }
    }
#endif

    return is_temp;
}

//read a buffer in an array
void COMMAND::read_buffer_serial (uint8_t *b, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        read_buffer_serial (b[i]);
        //*b++;
    }
}

#ifdef TCP_IP_DATA_FEATURE
//read buffer as char
void COMMAND::read_buffer_tcp (uint8_t b)
{
    static bool previous_was_char = false;
    static bool iscomment = false;
//to ensure it is continuous string, no char separated by binaries
    if (!previous_was_char) {
        buffer_tcp = "";
        iscomment = false;
    }
//is comment ?
    if (char (b) == ';') {
        iscomment = true;
    }
//it is a char so add it to buffer
    if (isPrintable (b) ) {
        previous_was_char = true;
        //add char if not a comment
        if (!iscomment) {
            buffer_tcp += char (b);
        }
    } else {
        previous_was_char = false; //next call will reset the buffer
    }
//this is not printable but end of command check if need to handle it
    if (b == 13 || b == 10) {
        //reset comment flag
        iscomment = false;
        //Minimum is something like M10 so 3 char
        if (buffer_tcp.length() > 3) {
            check_command (buffer_tcp, TCP_PIPE);
        }
    }
}
#endif
//read buffer as char
void COMMAND::read_buffer_serial (uint8_t b)
{
    static bool previous_was_char = false;
    static bool iscomment = false;
//to ensure it is continuous string, no char separated by binaries
    if (!previous_was_char) {
        buffer_serial = "";
        iscomment = false;
    }
//is comment ?
    if (char (b) == ';') {
        iscomment = true;
    }
//it is a char so add it to buffer
    if (isPrintable (b) ) {
        previous_was_char = true;
        if (!iscomment) {
            buffer_serial += char (b);
        }
    } else {
        previous_was_char = false; //next call will reset the buffer
    }
//this is not printable but end of command check if need to handle it
    if (b == 13 || b == 10) {
        //reset comment flag
        iscomment = false;
        //Minimum is something like M10 so 3 char
        if (buffer_serial.length() > 3) {
            check_command (buffer_serial, DEFAULT_PRINTER_PIPE);
        }
    }
}
