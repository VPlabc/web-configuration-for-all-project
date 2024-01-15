/*
    This file is part of WIC Firmware for 3D printer.

    WIC Firmware for 3D printer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    WIC Firmware for 3D printer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this Firmware.  If not, see <http://www.gnu.org/licenses/>.

    This firmware is using the standard arduino IDE with module to support ESP8266/ESP32:
    https://github.com/esp8266/Arduino
    https://github.com/espressif/arduino-esp32

    Latest version of the code and documentation can be found here :
    https://github.com/luc-github/WIC

    Main author: luc lebosse

*/
// #define LOOKLINE_UI


#include <Arduino.h>
#include "WIC.h"
#include <EEPROM.h>
#ifdef ARDUINO_ARCH_ESP8266
#include <FS.h>
#endif
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#if defined(ASYNCWEBSERVER)
#include <ESPAsyncWebServer.h>
#endif
#include "config.h"
#if defined(TIMESTAMP_FEATURE)
#include <time.h>
#endif
#include "wificonf.h"
#include "espcom.h"
#include "webinterface.h"
#include "command.h"
#ifdef ARDUINO_ARCH_ESP8266
#include "ESP8266WiFi.h"
#if defined(ASYNCWEBSERVER)
#include <ESPAsyncTCP.h>
#else
#include <ESP8266WebServer.h>
#endif
#else // ESP32
#include <WiFi.h>
#if defined(ASYNCWEBSERVER)
#include <AsyncTCP.h>
#else
#include <WebServer.h>
#endif
#include <rom/rtc.h>
#include "esp_wifi.h"
#include "FS.h"
#include "SPIFFS.h"
#include "Update.h"
#endif
#include <WiFiClient.h>

#include "FirmwareUpdate.h"
UpdateFW MainUDFW;

#ifdef CAPTIVE_PORTAL_FEATURE
#include <DNSServer.h>
extern DNSServer dnsServer;
#endif


#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#include <FS.h>
#ifdef ESP_OLED_FEATURE
#include "esp_oled.h"
#endif
#ifdef DHT_FEATURE
#include <DHTesp.h>
DHTesp dht;
#endif
#define LEDstt -1

#if defined(ASYNCWEBSERVER)
#include "asyncwebserver.h"
#else
#include "syncwebserver.h"
#endif
#ifdef Valve_UI
#include "Valve/Valve.h"
Valve valves;
#endif // Valve_UI
#ifdef Switch_UI
#include "LightTimer/LightTimer.h"
LightTimer light;
#endif // Switch_UI
#ifdef CircuitTesting_UI
#include "CircuitTesting/CircuitTesting.h"
CircuitTest CIRCUITTEST;
#endif // CircuitTesting_UI
#ifdef Gyro_UI
#include "GyroDatalog/Gyro_Datalog.h"
GyroDatalog GyLog;

#include <ClickButton.h>

const int buttonPin1 = D3;
ClickButton button1(buttonPin1, LOW, CLICKBTN_PULLUP);
// Arbitrary LED function
int Buttonfunction = 0;
bool State = false;
bool GyroWifiOn = true;
#include "GyroDatalog/Gyro_Datalog.h"
GyroDatalog Gyros;
#endif // Gyro_UI

#ifdef MESHCOM_UI
#include "MeshCom/MeshCom.h"
Mesh_Com meshcom;
#endif // MESHCOM_UI

#ifdef Moto_UI
#include "Moto/Moto.h"
Moto MOTO;
byte ModeRun = 0;
#endif //

#ifdef IOTDEVICE_UI
#include "AutoIT_IoT/IoTDevice.h"
IoT_Device IOT_DEVICE;
#endif // IOTDEVICE_UI
#ifdef AUTOITGW_UI
#include "AutoIT_IoT/AutoITGW.h"
Auto_Device AutoitGW;
#endif // AUTOITGW_UI
#ifdef LOOKLINE_UI
#include "LookLine/LookLine.h"
LOOKLINE_PROG lookline_prog;
#endif // LOOKLINE_UI
#ifdef PLC_MASTER_UI
#include "PLC_IoT/PLC_Master.h"
PLC_MASTER PLC_MASTER_Prog;
#endif // PLC_MASTER_UI
// Contructor
CONFIG conf;

byte RunMode = 0;
bool looklineDebug = false;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef TIMER_INTER_FEATURES
////////////////////////////////////////// TIMER INTERUPT ///////////////////////////////////
volatile int interruptCounter;
int totalInterruptCounter; 
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  totalInterruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}
#endif// TIMER_INTER_FEATURES

TaskHandle_t Task1;
TaskHandle_t Task2;
// TaskHandle_t Task3;

#include "Tsk1.h"
#include "Tsk2.h"

WIC::WIC()
{
}

// Begin which setup everything
void WIC::begin(uint16_t startdelayms, uint16_t recoverydelayms)
{
#ifdef TIMER_INTER_FEATURES
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 10, true);
  timerAlarmEnable(timer);
#endif//TIMER_INTER

#ifdef Moto_UI
    // ModeRun = 0;CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &ModeRun);
    if (ModeRun == 1)
    {
#ifdef Moto_UI
        MOTO.setup();
#endif //
    }
    else
    {
#endif // Moto_UI
#ifdef MESHCOM_UI
        byte dataReaded = 0;
        bool stateS = CONFIG::read_byte(EP_EEPROM_WIFI_MODE, &dataReaded);
        meshcom.WiFi_on = dataReaded;
        if (meshcom.WiFi_on == 0)
        {
            meshcom.setup();
            LOG("RF Mode: Mesh\n");
        }
        else
        {
            LOG("RF Mode: Wifi\n");
#endif // MESHCOM_UI
            Auto = true;
            // init:
            WiFi.disconnect();
            WiFi.mode(WIFI_OFF);
            // check EEPROM Version
#if defined(DEBUG_WIC) && defined(DEBUG_OUTPUT_SERIAL)
            CONFIG::InitBaudrate(9600);
            delay(2000);
            LOG("\r\nDebug Serial set\r\n")
#endif
            CONFIG::adjust_EEPROM_settings();
            CONFIG::InitOutput();
#ifdef ESP_OLED_FEATURE
            OLED_DISPLAY::begin();
#endif
            bool breset_config = false;
            web_interface = NULL;
#ifdef TCP_IP_DATA_FEATURE
            data_server = NULL;
#endif

#ifdef MKS_TFT_FEATURE
            startdelayms = 1000;
#endif
#ifdef ESP_OLED_FEATURE
#ifndef Moto_UI
            OLED_DISPLAY::BigDisplay("VPlab", 15, 10);
            OLED_DISPLAY::setCursor(0, 42);
            String txt = "  Ver:" + String(MainUDFW.FirmwareVer);
            ESPCOM::print(txt.c_str(), OLED_PIPE);
            uint32_t start_display_time = millis();
            uint32_t now = millis();
            while (now - start_display_time < startdelayms)
            {
                int v = (100 * (millis() - start_display_time)) / startdelayms;
                OLED_DISPLAY::display_mini_progress(v);
                OLED_DISPLAY::update_lcd();
                delay(100);
                now = millis();
            }
#else
            delay(startdelayms);
#endif // Moto_UI
#else
    delay(startdelayms);
#endif//ESP_OLED_FEATURE
            CONFIG::InitDirectSD();
            CONFIG::InitPins();
#ifdef RECOVERY_FEATURE
            delay(recoverydelayms);
            // check if reset config is requested
            if (digitalRead(RESET_CONFIG_PIN) == 0)
            {
                breset_config = true; // if requested =>reset settings
            }
#endif
            // check if EEPROM has value
            if (!CONFIG::InitBaudrate() || !CONFIG::InitExternalPorts())
            {
                breset_config = true; // cannot access to config settings=> reset settings
                LOG("Error no EEPROM access\r\n")
            }
            // reset is requested
            if (breset_config)
            {
                // update EEPROM with default settings
                CONFIG::InitBaudrate(DEFAULT_BAUD_RATE);
#ifdef ARDUINO_ARCH_ESP8266
                Serial.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
#endif
                delay(2000);
                ESPCOM::println(F("ESP EEPROM reset"), PRINTER_PIPE);
#ifdef DEBUG_WIC
                CONFIG::print_config(DEBUG_PIPE, true);
                delay(1000);
#endif
                CONFIG::reset_config();
                delay(1000);
                // put some default value to a void some exception at first start
                WiFi.mode(WIFI_AP);
                // wifi_config.WiFi_on = true;
#ifdef ARDUINO_ARCH_ESP8266
                WiFi.setPhyMode(WIFI_PHY_MODE_11G);
#else
        esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PHY_MODE_11G);
#endif
                CONFIG::esp_restart();
            } // if (breset_config) {
#if defined(DEBUG_WIC) && defined(DEBUG_OUTPUT_SERIAL)
            LOG("\r\n");
            delay(500);
            ESPCOM::flush(DEFAULT_PRINTER_PIPE);
#endif
            // get target FW
            CONFIG::InitFirmwareTarget();
            delay(100);
            // Update is done if any so should be Ok
#ifdef ARDUINO_ARCH_ESP32
            SPIFFS.begin(true);
#else
    SPIFFS.begin();
#endif
            // basic autostart
            if (SPIFFS.exists("/autostart.g"))
            {
                FS_FILE file = SPIFFS.open("/autostart.g", SPIFFS_FILE_READ);
                if (file)
                {
                    String autoscript = file.readString();
                    if (autoscript.length() > 0)
                    {
                        // clean line
                        autoscript.replace("\n", "");
                        autoscript.replace("\r", "");
                        ESPCOM::println(autoscript.c_str(), DEFAULT_PRINTER_PIPE);
                    }
                    file.close();
                }
            }
#ifdef LOOKLINE_UI
// CONFIG::read_byte(EP_EEPROM_COM_MODE, &RunMode);
// if(RunMode != MESH){
#endif// lookline_ui
            // setup wifi according settings

// #ifndef LOOKLINE_UI
            if (!wifi_config.Setup(false, LED_STATUS, 1))
            {
#ifdef ESP3D_UI
                OLED_DISPLAY::setCursor(0, 11);
#endif // Moto
       // try again in AP mode
                ESPCOM::println(F("Safe mode 1"), PRINTER_PIPE);
                    // LOGLN("Safe mode 1");
                if (!wifi_config.Setup(true, LED_STATUS, 1))
                {
#ifdef ESP3D_UI
                    wifi_config.Safe_Setup();
#endif //
                    // LOGLN("Safe mode 2");
                    ESPCOM::println(F("Safe mode 2"), PRINTER_PIPE);
                }
            }
// #endif// lookline_ui
        #ifdef LOOKLINE_UI
        // }
        #endif// lookline_ui
            delay(100);
            // setup servers
            if (!wifi_config.Enable_servers())
            {
                ESPCOM::println(F("Error enabling servers"), PRINTER_PIPE);
            }
            /*#ifdef ARDUINO_ARCH_ESP8266
                if	(rtc_info->reason	==	REASON_WDT_RST	||

                        rtc_info->reason	==	REASON_EXCEPTION_RST	||

                        rtc_info->reason	==	REASON_SOFT_WDT_RST)	{
                        String s = "reset ";
                        s+= String(rtc_info->reason);

                    if	(rtc_info->reason	==	REASON_EXCEPTION_RST)	{
                        s+=" except ";
                        s+=String(rtc_info->exccause);

                    }
                    ESPCOM::println (s, PRINTER_PIPE);
                }
            #else
                if((( reason_0< 17) || ( reason_1< 17)) && !(((reason_0 == 1) && (reason_1 == 14)) || ((reason_0 == 16) && (reason_1 == 14))))
                {
                    String s = "reset ";
                    ESPCOM::println (s, PRINTER_PIPE);
                    s+=String(reason_0);
                    s+="/";
                    s+=String(reason_1);

                }
            #endif*/
            // #ifdef ESP_OLED_FEATURE
            //         OLED_DISPLAY::setCursor(0, 0);
            //         ESPCOM::print(WiFi.localIP().toString().c_str(), OLED_PIPE);
            // #endif//#ifdef ESP_OLED_FEATURE

#ifdef Valve_UI
            valves.valve_setup();
// time_t nows = time(nullptr);
// LOG ("Seconds:" +String(nows) + '\n');
#endif // Valve_UI
#ifdef Switch_UI
            light.Setup();
#endif // Switch_UI
#ifdef CircuitTesting_UI
            CIRCUITTEST.Setup();
#endif // CircuitTesting_UI
#ifdef Gyro_UI
            GyLog.Setup();
            button1.debounceTime = 20;    // Debounce timer in ms
            button1.multiclickTime = 250; // Time limit for multi clicks
            button1.longClickTime = 1000; // time until "held-down clicks" register
#endif                                    // Gyro_UI
#ifdef Moto_UI
            MOTO.setup();
#endif //
#ifdef ASYNCWEBSERVER
            if (WiFi.getMode() != WIFI_AP)
            {
                WiFi.scanNetworks(true);
            }
#endif
#ifdef MESHCOM_UI
        }
#endif // MESHCOM_UI
#ifdef IOTDEVICE_UI
// }
#endif // IOTDEVICE_UI
#ifdef Moto_UI
    }
#endif // if(stateS == 1){
    ESPCOM::println(F("Setup Done"), PRINTER_PIPE);
    // LOG("Setup Done\r\n");
#ifdef AUTOITGW_UI
    AutoitGW.setup();
    LOG("AutoIT Setup\r\n");
#endif // AUTOITGW_UI
#ifdef IOTDEVICE_UI
    IOT_DEVICE.setup();
    IOT_DEVICE.MeshBegin();
    LOG("IoT Device Setup\r\n");

    CONFIG::read_byte(EP_EEPROM_WIFI_MODE, &RunMode);
    if(RunMode >= 2){CONFIG::write_byte(EP_EEPROM_WIFI_MODE, WIFIMODE);}
#endif // IOTDEVICE_UI
#ifdef LOOKLINE_UI
    lookline_prog.setup();   
#endif // LOOKLINE_UI

#ifdef PLC_MASTER_UI
PLC_MASTER_Prog.setup();
#endif//PLC_MASTER_UI

//------------------------------------------------------------------------
  //Task1 :
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task*/
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core x */
  delay(500);
  //------------------------------------------------------------------------
  //Task2 :
  xTaskCreatePinnedToCore(
    Task2code,   /* Task function. */
    "Task2",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task*/
    &Task2,      /* Task handle to keep track of created task */
    1);          /* pin task to core x */
  delay(500);
  //------------------------------------------------------------------------
  
}

bool onece = true;
bool onece1 = true;
bool onece2 = true;
bool CheckFWonce = false;
byte resent = 0;
bool Init_UI = false;

#ifdef LOOKLINE_UI
// void WIC::Set_Init_UI(String auths){LOGLN("Set_Init_UI " + auths);socket_server->broadcastTXT(auths);}

void WIC::SetDebug(bool state){
    looklineDebug = state;LOGLN("looklineDebug " + String(state));
}
#endif// LOOKLINE_UI
// Process which handle all input
void WIC::process(){
#ifdef Moto_UI
    if (ModeRun == 1)
    {
        MOTO.loop();
    }
    else
    {
        MOTO.loop();
#endif //
#ifdef MESHCOM_UI
        meshcom.loop();
        if (meshcom.WiFi_on == 1)
        {
            if (onece)
            {
                onece = false;
                LOG("RF Mode: Wifi\n");
            }
        }
        if (meshcom.WiFi_on == 0)
        {
            if (onece)
            {
                onece = false;
                LOG("RF Mode: Mesh\n");
            }
        }
        if (meshcom.WiFi_on == 0)
        {
        }
        else
        {
#endif // MESHCOM_UI
#ifdef Gyro_UI
#ifdef FC_Gyro
            GyroWifiOn = true;
#endif // FC_Gyro
            if (GyroWifiOn)
            {
#endif // Gyro_UI
#ifdef AUTOITGW_UI
                AutoitGW.loop();
#endif // AUTOITGW_UI
#ifdef IOTDEVICE_UI
                IOT_DEVICE.loop();
// if(IOT_DEVICE.RunMode == WWIFIMODE){
//   if(onece){onece = false;LOG("RF Mode: Wifi\n");}
// }
// if(IOT_DEVICE.RunMode == MESHMODE){
//   if(onece){onece = false;LOG("RF Mode: Mesh\n");}
// }
// if(IOT_DEVICE.RunMode == 2){
// }
// else{
#endif // IOTDEVICE_UI
#ifdef ARDUINO_ARCH_ESP8266
#ifdef MDNS_FEATURE
                wifi_config.mdns.update();
#endif
#endif
#if !defined(ASYNCWEBSERVER)
// web requests for sync
#ifdef MESHCOM_UI
// LOG("Mode: " + String(meshcom.getMode()) + "\n");
#endif // MESHCOM_UI
#ifdef LOOKLINE_UI
                lookline_prog.loop();
                // LOGLN("RunMode:" + String(RunMode));
                //  if (RunMode != MESH){
                    if (onece1){onece1 = false;ESPCOM::println(F("Wifi Server Working..."), PRINTER_PIPE);}
#endif // LOOKLINE_UI
#ifdef IOTDEVICE_UI

                    // if(onece){onece = false;LOG("\nRun Mode:"+String(IOT_DEVICE.RunMode)+"\n");}
                    if (RunMode == WIFIMODE){
                        if (onece1){onece1 = false;LOG("\nWifi Server Working...\n");}
#endif // IOTDEVICE_UI
#ifdef AUTOITGW_UI
                        // if(onece){onece = false;LOG("\nRun Mode:"+String(IOT_DEVICE.RunMode)+"\n");}
                        if (onece1){onece1 = false;LOG("\nWifi Server Working...\n");}
                    if (onece1){onece1 = false;LOG("\nWifi Server Working...\n");}
#endif // AUTOITGW_UI
                    if (onece1){onece1 = false;ESPCOM::println(F("Wifi Server Working..."), PRINTER_PIPE);}
                        web_interface->web_server.handleClient();
                        socket_server->loop();
#ifdef IOTDEVICE_UI
                    } // if(IOT_DEVICE.RunMode == WIFIMODE)
#endif                // IOTDEVICE_UI
#ifdef LOOKLINE_UI
                //  }// if (RunMode == MESH){
#endif // LOOKLINE_UI
#ifdef MESHCOM_UI
#endif // MESHCOM_UI
#endif
                // be sure wifi is on to proceed wifi function

#ifdef MESHCOM_UI
                if (meshcom.WiFi_on == 1)
                {
#endif // MESHCOM_UI
                    if (WiFi.getMode() != WIFI_OFF){
#ifdef CAPTIVE_PORTAL_FEATURE
                        if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
                        {
                            dnsServer.processNextRequest();
#ifdef Switch_UI
                            if (onece2){onece2 = false;LOG("\nWifi Portal Working...\n");}
#endif // Switch_UI Auto_Device
#ifdef IOTDEVICE_UI
                            if (onece2){onece2 = false;LOG("\nWifi Portal Working...\n");}
#endif // IOTDEVICE_UI
#ifdef AUTOITGW_UI
                            if (onece2){onece2 = false;LOG("\nWifi Portal Working...\n");}
#endif // AUTOITGW_UI
    
                            if (onece2){onece2 = false;ESPCOM::println(F("Wifi Portal Working..."), PRINTER_PIPE);
                            }
                            static unsigned long previousMillis = 0;
                            if (millis() - previousMillis >= 300) {if(WiFi.status() != WL_CONNECTED){digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));}previousMillis = millis();}
                            
                        }//if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
#endif
                        // TODO use config
                        CONFIG::wait(0);
                    }//if (WiFi.getMode() != WIFI_OFF)

                    // read / bridge all input
                    ESPCOM::bridge();
                    // in case of restart requested
                    if (web_interface->restartmodule)
                    {
                        CONFIG::esp_restart();
                    }
#ifdef Moto_UI
                }
#endif // if(stateS == 1){
#ifdef MESHCOM_UI
            } // if(meshcom.getMode() == 1){
#endif // MESHCOM_UI
#ifdef Gyro_UI
        } // GyroWifiOn
#endif    // Gyro_UI

delay(10);

#ifdef ESP_OLED_FEATURE
        static uint32_t last_oled_update = 0;
        if (!CONFIG::is_locked(FLAG_BLOCK_OLED))
        {
            uint32_t now_oled = millis();
            if (now_oled - last_oled_update > 1000)
            {
                last_oled_update = now_oled;
// refresh signal
#ifdef IOTDEVICE_UI

                if (RunMode == WIFIMODE)
                {
                    OLED_DISPLAY::display_signal(wifi_config.getSignal(WiFi.RSSI()), 90, 0);
                    if (OLED_DISPLAY::L0_size > 85)
                    {
#ifdef IOTDEVICE_UI
                        OLED_DISPLAY::display_text(OLED_DISPLAY::L0.c_str(), 0, 0, 128);
#else
                        OLED_DISPLAY::display_text(OLED_DISPLAY::L0.c_str(), 0, 0, 85);
#endif // IOTDEVICE_UI
                    }
                }
                else
                {
                    // if line 0 is > 85 refresh
                    if (OLED_DISPLAY::L0_size > 128)
                    {
#ifdef IOTDEVICE_UI
                        OLED_DISPLAY::display_text(OLED_DISPLAY::L0.c_str(), 0, 0, 128);
#else
                        OLED_DISPLAY::display_text(OLED_DISPLAY::L0.c_str(), 0, 0, 85);
#endif // IOTDEVICE_UI
                    }
                }
#else
                if (WiFi.getMode() == WIFI_OFF)
                {
                    OLED_DISPLAY::display_signal(-1);
                }
                else
                {
                    OLED_DISPLAY::display_signal(wifi_config.getSignal(WiFi.RSSI()), 90, 0);
                }
#endif // IOTDEVICE_UI
#ifndef Moto_UI
                // if line 1 is > 128 refresh
                if (OLED_DISPLAY::L1_size > 128)
                {
                    OLED_DISPLAY::display_text(OLED_DISPLAY::L1.c_str(), 0, 13, 128);
                }
                // if line 2 is > 128 refresh
                if (OLED_DISPLAY::L2_size > 128)
                {
                    OLED_DISPLAY::display_text(OLED_DISPLAY::L2.c_str(), 0, 24, 128);
                }
                // if line 3 is > 128 refresh
                if (OLED_DISPLAY::L3_size > 128)
                {
                    OLED_DISPLAY::display_text(OLED_DISPLAY::L3.c_str(), 0, 36, 128);
                }
                // if line 4 is > 128 refresh
                if (OLED_DISPLAY::L4_size > 128)
                {
                    OLED_DISPLAY::display_text(OLED_DISPLAY::L4.c_str(), 0, 48, 128);
                }
                OLED_DISPLAY::update_lcd();
#endif // MOTO_DASH
            }
        }
#endif
//////////////////////////////////////////////////////////////// loop 5S

            /// @brief //// Loop 5 Seconds
            static uint32_t last_Loop5S_update = 0;
            uint32_t now_fw = millis();
            if (now_fw - last_Loop5S_update > (5 * 1000))
            {   last_Loop5S_update = now_fw;
                if(looklineDebug)LOGLN(".");
#ifdef Switch_UI
                light.LoadData();       
#ifdef ServerUpdateFW
                fwCheck = false;
#ifdef ARDUINO_ARCH_ESP8266
                CheckFWloop();
#else  // ESP32
                CheckFWloop();
#endif // ARDUINO_ARCH_
#endif // ServerUpdateFW
                LOG(String(light.hours) + ":" + String(light.mins) + "| Lock " + String(light.locks));
                // Debug_Ser.println(" brightness: " + String(brightness));
#endif // Switch_UI
            }//if (now_fw - last_Loop5S_update > (5 * 1000))
//////////////////////////////////////////////////////////////// loop 5S
//////////////////////////////////////////////////////////////// loop 100mS
            /// @brief //// Loop 1 Seconds
            static uint32_t last_Loop100mS_update = 0;
            uint32_t now_100ms = millis();
            if (now_fw - last_Loop100mS_update > (1 * 10))
            {   last_Loop100mS_update = now_100ms;
                // if(looklineDebug)LOGLN("Loop 100ms");
                #ifdef LOOKLINE_UI
                lookline_prog.TimerPlanInc();
                #endif// LOOKLINE_UI
            }
//////////////////////////////////////////////////////////////// loop 100mS

#ifdef DHT_FEATURE
        if (CONFIG::DHT_type != 255){
            static uint32_t last_dht_update = 0;
            uint32_t now_dht = millis();
            if (now_dht - last_dht_update > (CONFIG::DHT_interval * 1000)){
        
                last_dht_update = now_dht;
                float humidity = dht.getHumidity();
                float temperature = dht.getTemperature();
                if (strcmp(dht.getStatusString(), "OK") == 0)
                {
                    String s = String(temperature, 2);
                    String s2 = s + " " + String(humidity, 2);
#if defined(ASYNCWEBSERVER)
                    web_interface->web_events.send(s2.c_str(), "DHT", millis());
#else
                    s = "DHT:" + s2;
                    socket_server->sendTXT(ESPCOM::current_socket_id, s);
#endif
#ifdef ESP_OLED_FEATURE
                    if (!CONFIG::is_locked(FLAG_BLOCK_OLED))
                    {
                        s = String(temperature, 2);
                        s += "°C";
                        OLED_DISPLAY::display_text(s.c_str(), 84, 16);
                    }
#endif
                }

            }//if (now_dht - last_dht_update > (CONFIG::DHT_interval * 1000))
        }//if (CONFIG::DHT_type != 255)
#endif
#ifdef Valve_UI
        valves.valve_loop();
#endif // Valve_UI
#ifdef Switch_UI
        light.Loop();
#endif // Switch_UI
#ifdef CircuitTesting_UI
        CIRCUITTEST.Loop();
#endif // CircuitTesting_UI
#ifdef Gyro_UI
        GyLog.Loop();
        // Update button state
        button1.Update();

        // Save click codes in LEDfunction, as click codes are reset at next Update()
        if (button1.clicks != 0)
        {
            Buttonfunction = button1.clicks;
        }
        //  as it would toggle Run/Stop
        if (Buttonfunction == 1)
        {
            State = !State;
            GyroWifiOn = false;
            Buttonfunction = 10;
#ifdef FC_Gyro
            if (State)
            {
                GyLog.Start_PPM();
                CONFIG::write_byte(GyroState, 1);
                Serial.println("Run");
            }
            else
            {
                CONFIG::write_byte(GyroState, 0);
                digitalWrite(LEDstt, HIGH);
                Serial.println("Stop");
            }
#else
            if (State)
            {
                CONFIG::write_byte(GyroState, 1);
                Gyros.Start();
                Serial.println("Run");
            }
            else
            {
                CONFIG::write_byte(GyroState, 0);
                digitalWrite(LEDstt, HIGH);
                Serial.println("Stop");
            }
#endif // FC_Gyro
        }
        // slow blink (must hold down button. 1 second long blinks)
        if (Buttonfunction == -1)
        {
            GyroWifiOn = true;
            Serial.println("Wifi On");
            Buttonfunction = 10;
            digitalWrite(LEDstt, LOW);
        }
#endif // Gyro_UI

// todo use config
// CONFIG::wait(0);
#ifdef MESHCOM_UI
    }
#endif // MESHCOM_UI
#ifdef IOTDEVICE_UI
// }
#endif // #ifdef IOTDEVICE_UI

////////////////////////////////////////////////////////// TIMER ///////////////////
#ifdef TIMER_INTER_FEATURES
    if (interruptCounter > 0) {
        // LOG("| TIMER_INTER_FEATURES |");
    // lookline_prog.TimerPlanInc();
        // LOGLN("| ");
    // static int countTimer = 0;countTimer++;if(countTimer > 1){countTimer = 0;LOGLN("one second");}
    //for(byte node = 0 ; node < 100 ; node++){Data[3][node]++;}
    // if(ComMode == MQTT){countermqtt++;if(countermqtt > 50){countermqtt = 55;}}

    portENTER_CRITICAL_ISR(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL_ISR(&timerMux);
  }
#endif//#ifdef TIMER_INTER_FEATURES
///////////////////////////////////////////////////////////////////////////////////
}//void WIC::process()


