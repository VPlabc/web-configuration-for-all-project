/*
  config.h - ESP3D configuration class

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
//#define HomeKit
//Device Type
//#define ESP3D_UI 
//#define Valve_UI
//#define CircuitTesting_UI
//#define Gyro_UI
//#define RFHub_UI
//#define MESHCOM_UI
// #define IOTDEVICE_UI
// #define AUTOITGW_UI
// #define Moto_UI
#define LOOKLINE_UI
// #define LOOKLINE_MASTER
// #define Switch_UI
// #define PLC_MASTER_UI
// #define Basic_UI

    // #define ModbusCom

#ifdef LOOKLINE_MASTER
#define USE_SERIAL_0

    // #define LOOKLINE_UI
    // #define TIMER_INTER_FEATURES
    #define ModbusCom
    // #define TIMER_INTER_FEATURES
    #ifndef LOOKLINE_UI
        #define EP_EEPROM_DEBUG         1087// 4 bytes
        #define EP_EEPROM_ROLE          1103// 4 bytes Node|gateway|repearter
        #define EP_EEPROM_VERSION 1073// 6 bytes = ESP3D<V on one byte>
        #define LAST_EEPROM_ADDRESS 1083
    #endif//LOOKLINE_UI

    #define MASTER_MODBUS
#else//
    #define SLAVE_MODBUS
#endif//LOOKLINE_MASTER

#ifndef  LOOKLINE_MASTER
#define USE_SERIAL_0

// #define Mesh_Network
// #define USE_LORA
// #define TIMER_INTER_FEATURES
#define TIMESTAMP_FEATURE
// #define ModbusCom
#endif//LOOKLINE_MASTER

#ifdef ARDUINO_ARCH_ESP8266
// #ifndef IOTDEVICE_UI
// #define Switch_UI
// #endif//
#endif//#ifdef ARDUINO_ARCH_ESP8266

// #define Master
//version and sources location
#define FW_VERSION "14.9.9.4"
#define REPOSITORY ""
//#define ARDUINO_ARCH_ESP8266
//Customize ESP3D ////////////////////////////////////////////////////////////////////////
#define ESP8266_MODEL_NAME "ESP8266"
#define ESP8266_MODEL_URL "http://espressif.com/en/products/esp8266/"
#define ESP32_MODEL_NAME "ESP32"
#define ESP32_MODEL_URL "https://www.espressif.com/en/products/hardware/esp-wroom-32/overview"
#define ESP_MODEL_NUMBER "WIC 2.1"
#define ESP_MANUFACTURER_NAME "Espressif Systems"
#define ESP_MANUFACTURER_URL "http://espressif.com"
//default name if no mac address is valid
#define ESP_DEFAULT_NAME "VPlab"
//if commented name will follow mac address 3 last digits
//like ESP_XXXXXX (eg:ESP_028E41) to avoid overlap if several WIC
#define ESP_HOST_NAME ESP_DEFAULT_NAME


//To allow webupdate using small updater
//#define USE_AS_UPDATER_ONLY

//FEATURES - comment to disable //////////////////////////////////////////////////////////

//TIMER_INTER_FEATURES: allow to internal timer use
// #define TIMER_INTER_FEATURES

//WEB_UPDATE_FEATURE: allow to flash fw using web UI
#define WEB_UPDATE_FEATURE

#ifndef USE_AS_UPDATER_ONLY
//Do we use async webserver or not (currntly deprecated do not enable it yet)
//#define ASYNCWEBSERVER

//SERIAL_COMMAND_FEATURE: allow to send command by serial
//#define SERIAL_COMMAND_FEATURE

//TCP_IP_DATA_FEATURE: allow to connect serial from TCP/IP
// #define TCP_IP_DATA_FEATURE

//NOTIFICATION_FEATURE : allow to push notifications
//#define NOTIFICATION_FEATURE

//MKS TFT WIFI support see Wiki for wiring
//#define MKS_TFT_FEATURE

//MDNS_FEATURE: this feature allow  type the name defined
//in web browser by default: http:\\esp8266.local and connect
#define MDNS_FEATURE

//SSDD_FEATURE: this feature is a discovery protocol, supported on Windows out of the box
#define SSDP_FEATURE

//NETBIOS_FEATURE: this feature is a discovery protocol, supported on Windows out of the box
//#define NETBIOS_FEATURE

//CAPTIVE_PORTAL_FEATURE: In SoftAP redirect all unknow call to main page
#define CAPTIVE_PORTAL_FEATURE

//RECOVERY_FEATURE: allow to use GPIO2 pin as hardware reset for EEPROM, add 8s to boot time to let user to jump GPIO2 to GND
//#define RECOVERY_FEATURE

//DIRECT_PIN_FEATURE: allow to access pin using ESP201 command
#define DIRECT_PIN_FEATURE

//ESP_OLED_FEATURE: allow oled screen output
//#define ESP_OLED_FEATURE

//DHT_FEATURE: send update of temperature / humidity based on DHT 11/22
// #define DHT_FEATURE

//AUTHENTICATION_FEATURE: protect pages by login password
// #define AUTHENTICATION_FEATURE

//WS_DATA_FEATURE: allow to connect serial from Websocket
#define WS_DATA_FEATURE

//TIMESTAMP_FEATURE: Time stamp feature on direct SD  files
//#define TIMESTAMP_FEATURE
#endif //USE_AS_UPDATER_ONLY
//Extra features /////////////////////////////////////////////////////////////////////////

//Workaround for Marlin 2.X coldstart
#define DISABLE_CONNECTING_MSG

//Serial rx buffer size is 256 but can be extended
#define SERIAL_RX_BUFFER_SIZE 512

//USE_LORA: use lora module
// #define USE_LORA

#ifdef AUTOITGW_UI
#define MQTT_USE
//#define ESP_OLED_FEATURE//ON khi chay voi ESP se loi
#endif//AUTOITGW_UI
#ifdef CircuitTesting_UI
#define ESP_OLED_FEATURE

#endif//OLED_FEATURE

#ifdef Valve_UI
#ifndef ARDUINO_ARCH_ESP8266
#define USE_LoRa
#define SDCARD_FEATURE
#define TIMESTAMP_FEATURE
#endif//not (ARDUINO_ARCH_ESP8266)
#endif//Valve_UI

#ifdef Switch_UI
#define Volt_FEATURE
#define Time

#define DeepSleepMode
//#define ServerUpdateFW

//#define ARDUINO_ARCH_ESP8266
#define ADDRESSBRIGHT 1083//1 byte
#define ADDRESSLOGBit 1084//1 byte
//#define ADDRESSLOGBit 1084//1 byte
//47 
#define ADDRESSLOGSensorCounter 1085//2 byte
//49
#define ADDRESSLOGSwitch 1087//1 byte
//50
#define ADDRESS 1088//20 * 4 byte
//110
#define ADDRESSLOG 1184//100 * 4 byte
//510
//#define ADDRESSLOGSensor 510//8640 * 4 byte 
//34,560
//#define ADDRESSRest 8640//8646 * 4 byte 

#define OnDis 0
#define OnEnb 1
#define OfDis 2
#define OfEnb 3
#define Sensors 0
#define Switch 1
#endif//#ifdef Switch_UI

#ifdef Gyro_UI
#ifndef ARDUINO_ARCH_ESP8266
#define SDCARD_FEATURE
#endif//not (ARDUINO_ARCH_ESP8266)
#endif//Gyro_UI
#ifdef MESHCOM_UI
#ifdef ESP_OLED_FEATURE
#undef ESP_OLED_FEATURE
#endif//ESP_OLED_FEATURE
#ifdef TCP_IP_DATA_FEATURE
#undef TCP_IP_DATA_FEATURE
#endif//TCP_IP_DATA_FEATURE
#endif//MESHCOM_UI

#ifdef IOTDEVICE_UI
#define USE_SERIAL_0
#define ESP_OLED_FEATURE
// #ifdef ESP_OLED_FEATURE
// #undef ESP_OLED_FEATURE
// #endif//ESP_OLED_FEATURE
// #ifdef TCP_IP_DATA_FEATURE
// #undef TCP_IP_DATA_FEATURE
// #endif//TCP_IP_DATA_FEATURE
#define MQTT_USE
#endif//IOTDEVICE_UI

#ifdef ESP_OLED_FEATURE
#ifdef ARDUINO_ARCH_ESP8266
#define OLED_PIN_A 4
#define OLED_PIN_B 5
#else//ESP32
#define OLED_PIN_A 21
#define OLED_PIN_B 22
#endif//ARDUINO_ARCH_ESP8266
#define OLED_ADDRs 0x3c//0x78
#define HELTEC_EMBEDDED_PIN 0 //0 to disable
#define OLED_FLIP_VERTICALY 1 //0 to disable
#endif

//Serial Parameters
#define ESP_SERIAL_PARAM SERIAL_8N1

//which serial ESP use to communicate to printer (ESP32 has 3 serials available, ESP8266 only one)
//Uncomment one only
#ifdef MESHCOM_UI
#define USE_SERIAL_0
#endif//MESHCOM_UI
#ifdef Moto_UI
#define ESP_OLED_FEATURE
#define Time
#define USE_SERIAL_0
#endif//Moto_UI
//For ESP32 Only
//#define USE_SERIAL_1
//#define USE_SERIAL_2

//Pins Definition ////////////////////////////////////////////////////////////////////////
//-1 means use default pins of your board what ever the serial you choose
#define ESP_RX_PIN -1
#define ESP_TX_PIN -1

#ifdef RECOVERY_FEATURE
//pin used to reset setting
#define RESET_CONFIG_PIN 2
#endif

#ifdef DHT_FEATURE
#define ESP_DHT_PIN 2
#endif

//Pins where the screen is connected
#ifdef ESP_OLED_FEATURE
// #define OLED_DISPLAY_SSD1306  // OLED Display Type: SSD1306(OLED_DISPLAY_SSD1306) / SH1106(OLED_DISPLAY_SH1106), comment this line out to disable oled
// #define OLED_PIN_SDA  4  //5 //SDA;  // i2c SDA Pin
// #define OLED_PIN_SCL  5  //4 //SCL;  // i2c SCL Pin
// #define OLED_ADDR   0x3c
// #define HELTEC_EMBEDDED_PIN 16 //0 to disable
 #define OLED_FLIP_VERTICALY 1 //0 to disable
#endif


//Supported FW /////////////////////////////////////////////////////////////
#define UNKNOWN_FW   0
#define REPETIER4DV  1
#define MARLIN       2
#define MARLINKIMBRA 3
#define SMOOTHIEWARE 4
#define REPETIER     5
#define GRBL         6
#define LOOKLINE     7
#define LOOKLINEGW   8
#define VALVEGW      9
#define LIGHT_TM     10
#define TESTING      11
#define GYRODATALOG  12
#define RF_HUB       13
#define MESH_HUB     14
#define IOT_DEVICES   15
#define MOTO_DASH    16
#define MAX_FW_ID    16

//LoRa Air Rate  /////////////////////////////////////////////////////////////
#define Air_Rate_03   0
#define Air_Rate_12   1
#define Air_Rate_24   2
#define Air_Rate_48   3
#define Air_Rate_96   4
#define Air_Rate_192  5
#define MAX_Air_Rate  5

//LoRa Protocol  /////////////////////////////////////////////////////////////
#define Transparent          0
#define FIXED_Node           1
#define FIXED_Gateway        2
#define WOR_Node             3
#define WOR_Gateway          4
#define BROADCAST_MESSAGE1   5
#define BROADCAST_MESSAGE2   6
#define BROADCAST_MESSAGE3   7
#define MAX_Protocol      7
//For FW which has issue with checksum or not handling M110 properly///////
//#define DISABLE_SERIAL_CHECKSUM

//Do not Edit after this line //////////////////////////////////////////////

//DEBUG Flag do not do this when connected to printer !!!
//be noted all upload may failed if enabled
//#define DEBUG_ESP3D
//#define DEBUG_OUTPUT_SPIFFS
#define DEBUG_OUTPUT_SERIAL
// #define DEBUG_OUTPUT_TCP
#define DEBUG_OUTPUT_SOCKET
#define DEBUG_WIC
//Sanity check
#ifdef SDCARD_FEATURE
#ifdef TIMESTAMP_FEATURE
#undef TIMESTAMP_FEATURE
#endif
#endif

#if defined(ASYNCWEBSERVER)
#define ESP_USE_ASYNC true
#error it no more supported
#else
#define ESP_USE_ASYNC false
#endif

//number of clients allowed to use data port at once
#define MAX_SRV_CLIENTS 1

#define WIFI_NONE_SLEEP WIFI_PS_NONE
#define WIFI_LIGHT_SLEEP WIFI_PS_MIN_MODEM
#define WIFI_MODEM_SLEEP WIFI_PS_MAX_MODEM
#define AUTH_OPEN WIFI_AUTH_OPEN
#define AUTH_WEP WIFI_AUTH_WEP
#define AUTH_WPA_PSK WIFI_AUTH_WPA_PSK
#define AUTH_WPA2_PSK WIFI_AUTH_WPA2_PSK
#define AUTH_WPA_WPA2_PSK WIFI_AUTH_WPA_WPA2_PSK
#define WIFI_EVENT_STAMODE_CONNECTED SYSTEM_EVENT_STA_CONNECTED
#define WIFI_EVENT_STAMODE_DISCONNECTED SYSTEM_EVENT_STA_DISCONNECTED
#ifdef ESP32
#define WIFI_EVENT_APMODE_DISCONNECTED SYSTEM_EVENT_AP_STADISCONNECTED
#else
#define WIFI_EVENT_APMODE_DISCONNECTED WIFI_EVENT_SOFTAPMODE_STADISCONNECTED
#endif
#define WIFI_EVENT_STAMODE_GOT_IP SYSTEM_EVENT_STA_GOT_IP
#define WIFI_EVENT_SOFTAPMODE_STACONNECTED SYSTEM_EVENT_AP_STACONNECTED


#ifdef ARDUINO_ARCH_ESP32
#include "FS.h"
#include "SPIFFS.h"
using fs::File;
#define ENC_TYPE_NONE AUTH_OPEN
#define FS_FILE File
#define FS_DIR File
#define SD_FILE_READ FILE_READ
#define SPIFFS_FILE_READ FILE_READ
#define SD_FILE_WRITE FILE_WRITE
#define SPIFFS_FILE_WRITE FILE_WRITE
#define WIFI_PHY_MODE_11B WIFI_PROTOCOL_11B
#define WIFI_PHY_MODE_11G WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G
#define WIFI_PHY_MODE_11N WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N
#else
#define FS_DIR fs::Dir
#define FS_FILE fs::File
#define SD_FILE_READ FILE_READ
#define SPIFFS_FILE_READ "r"
#define SD_FILE_WRITE FILE_WRITE
#define SPIFFS_FILE_WRITE "w"

#define WIFI_PHY_MODE_11N WIFI_PROTOCOL_11N
#endif



//WEBHOST_SDCARD_FEATURE : to use SDCard to host webpages
//NOT YET IMPLEMENTED!!! Keep it as TODO
//#define WEBHOST_SDCARD_FEATURE

#ifdef DEBUG_OUTPUT_SOCKET
extern void log_socket(const char *format, ...);
extern const char * pathToFileName(const char * path);
#define log_esp3d(format, ...) log_socket("\n[VPLAB][%s:%u] %s(): " format "\n", pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)

// #include "espcom.h"
// #define LOG(string) {ESPCOM::webprint(string);}    
// #define LOGLN(string) {ESPCOM::webprintln(string);}    

#else
#define log_esp3d(format, ...)
#endif

#ifndef DEBUG_ESP3D
#ifdef DEBUG_OUTPUT_SPIFFS
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#include <FS.h>
#define DEBUG_PIPE NO_PIPE
#define LOG(string) { FS_FILE logfile = SPIFFS.open("/log.txt", "a+");logfile.print(string);logfile.close();}
#endif

#ifdef DEBUG_OUTPUT_SERIAL
#define DEBUG_PIPE SERIAL_PIPE
#define LOG(string) {Serial.print(string);}
#define LOGLN(string) {Serial.println(string);}
#else
#define LOG(string) {}
#define LOGLN(string) {}
#endif 
#ifdef DEBUG_OUTPUT_TCP
#include "espcom.h"
#define LOG(string) {ESPCOM::send2TCP(string, false);}
#define DEBUG_PIPE TCP_PIPE
#endif
#else
#define LOG(string) {}
#define DEBUG_PIPE NO_PIPE
#endif



#define NOLOG(string) {}

#ifndef CONFIG_h
#define CONFIG_h

#include <Arduino.h>
//#define ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ARCH_ESP8266
//extern "C" {
#include "user_interface.h"
//}
#else

#endif

#include "wificonf.h"

typedef enum {
    UPLOAD_STATUS_NONE = 0,
    UPLOAD_STATUS_FAILED = 1,
    UPLOAD_STATUS_CANCELLED = 2,
    UPLOAD_STATUS_SUCCESSFUL = 3,
    UPLOAD_STATUS_ONGOING  = 4
} upload_status_type;

typedef enum {
    NO_PIPE = 0,
    SERIAL_PIPE = 2,
    SERIAL1_PIPE = 3,
    SERIAL2_PIPE = 4,
#ifdef TCP_IP_DATA_FEATURE
    TCP_PIPE = 5,
#endif
#ifdef WS_DATA_FEATURE
    WS_PIPE = 6,
#endif
#ifdef ESP_OLED_FEATURE
    OLED_PIPE = 7,
#endif
    WEB_PIPE = 8,
    PRINTER_PIPE = 9
} tpipe;
#ifdef MESHCOM_UI
#define DEFAULT_PRINTER_PIPE SERIAL1_PIPE
#else
#define DEFAULT_PRINTER_PIPE SERIAL_PIPE
// #define DEFAULT_PRINTER_PIPE SERIAL1_PIPE
#endif//MESHCOM_UI
typedef enum {
    LEVEL_GUEST = 0,
    LEVEL_USER = 1,
    LEVEL_ADMIN = 2,
    LEVEL_STAFF = 3
} level_authenticate_type;


#define    NO_SD 0
#define    SD_DIRECTORY 1
#define    EXT_DIRECTORY 2


//flags
#define AP_MODE         1
#define CLIENT_MODE     2
#define DHCP_MODE       1
#define STATIC_IP_MODE      2

//position in EEPROM
//AP mode = 1; Station client mode = 2
#define EP_WIFI_MODE            0    //1 byte = flag
#define EP_STA_SSID             1    //33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define EP_STA_PASSWORD         34   //65 bytes 64 +1 = string ;warning does not support multibyte char like chinese
#define EP_STA_IP_MODE          99   //1 byte = flag
#define EP_STA_IP_VALUE         100  //4  bytes xxx.xxx.xxx.xxx
#define EP_STA_MASK_VALUE       104  //4  bytes xxx.xxx.xxx.xxx
#define EP_STA_GATEWAY_VALUE    108  //4  bytes xxx.xxx.xxx.xxx
#define EP_BAUD_RATE            112  //4  bytes = int
#define EP_STA_PHY_MODE         116  //1 byte = flag
#define EP_SLEEP_MODE           117  //1 byte = flag
#define EP_CHANNEL              118 //1 byte = flag
#define EP_AUTH_TYPE            119 //1 byte = flag
#define EP_SSID_VISIBLE         120 //1 byte = flag
#define EP_WEB_PORT             121 //4  bytes = int
#define EP_DATA_PORT            125 //4  bytes = int
#define EP_OUTPUT_FLAG          129 //1  bytes = flag
#define EP_HOSTNAME             130//33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define EP_DHT_INTERVAL         164//4  bytes = int
#define ESP_NOTIFICATION_TYPE   168//1 byte = flag
#define ESP_AUTO_NOTIFICATION   170//1  bytes = flag
#define EP_FREE_BYTE1           171//1  bytes = flag
#define EP_FREE_INT3            172//4  bytes = int
#define EP_ADMIN_PWD            176//21  bytes 20+1 = string  ; warning does not support multibyte char like chinese
#define EP_USER_PWD             197//21  bytes 20+1 = string  ; warning does not support multibyte char like chinese
#define EP_AP_SSID              218//33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define EP_AP_PASSWORD          251//65 bytes 64 +1 = string ;warning does not support multibyte char like chinese
#define EP_AP_IP_VALUE          316  //4  bytes xxx.xxx.xxx.xxx
#define EP_AP_MASK_VALUE        320  //4  bytes xxx.xxx.xxx.xxx
#define EP_AP_GATEWAY_VALUE     324  //4  bytes xxx.xxx.xxx.xxx
#define EP_AP_IP_MODE           329  //1 byte = flag
#define EP_AP_PHY_MODE          330  //1 byte = flag
#define EP_SD_SPEED_DIV         331  //1 byte = flag
#define ESP_NOTIFICATION_TOKEN1 332  //64 bytes 63+1 = string  ; warning does not support multibyte char like chinese
#define ESP_NOTIFICATION_TOKEN2 396  //64 bytes 63+1 = string  ; warning does not support multibyte char like chinese
#define EP_DHT_TYPE             460 //1  bytes = flag
#define EP_TARGET_FW            461 //1  bytes = flag
#define EP_TIMEZONE             462//1  bytes = flag
#define EP_TIME_ISDST           463//1  bytes = flag
#define EP_TIME_SERVER1         464//128 bytes 127+1 = string  ; warning does not support multibyte char like chinese  
#define EP_TIME_SERVER2         593 //128 bytes 127+1 = string  ; warning does not support multibyte char like chinese
#define EP_TIME_SERVER3         722 //128 bytes 127+1 = string  ; warning does not support multibyte char like chinese
#define EP_IS_DIRECT_SD         850//1  bytes = flag
#define EP_PRIMARY_SD           851//1  bytes = flag
#define EP_SECONDARY_SD         852//1  bytes = flag
#define EP_DIRECT_SD_CHECK      853//1  bytes = flag
#define EP_SD_CHECK_UPDATE_AT_BOOT 854//1  bytes = flag
#define ESP_NOTIFICATION_SETTINGS 855//128 bytes 127+1 = string  ; warning does not support multibyte char like chinese


#define EP_MQTT_BROKER   983//30  bytes = String
#define EP_MQTT_USER     1013//30  bytes = string
#define EP_MQTT_PASS     1043//30  bytes = string
#define EP_EEPROM_VERSION 1073// 6 bytes = ESP3D<V on one byte>
// #define EP_EEPROM_DEBUG 1083 // 1 bytes flag
// #define LAST_EEPROM_ADDRESS 1084
// #define ESP32_C3

#ifdef Basic_UI
#define EP_EEPROM_DEBUG 1083 // 1 bytes flag
#define LAST_EEPROM_ADDRESS 1083
#endif//Basic_UI
#ifdef AUTOITGW_UI
#define EP_EEPROM_DEBUG 1083 // 1 bytes flag
#define LAST_EEPROM_ADDRESS 1083
#endif//AUTOITGW_UI

#ifdef ESP3D_UI
#define LAST_EEPROM_ADDRESS 1079
#endif//ESP3D_UI

// #ifdef USE_LoRa
// #define EP_EEPROM_VERSION 1073// 6 bytes = ESP3D<V on one byte>
// #define LAST_EEPROM_ADDRESS 1079
// #endif//USE_LoRa

#ifdef Switch_UI
#define EP_EEPROM_DEBUG     1079// 4 bytes
#define LAST_EEPROM_ADDRESS 1083
#endif//Switch_UI
#ifdef Valve_UI
#define EP_LORA_RSSI     1074//1  bytes = byte
#define EP_LORA_CHANEL   1075//4  bytes = byte
#define EP_LORA_AIRRATE  1079//1  bytes = byte
#define EP_LORA_PROTOCOL 1080//1  bytes = byte
#define EP_LORA_T_SCAN   1081//1  bytes = byte
#define EP_LORA_T_REQUEST 1082//1  bytes = byte
#define EP_EEPROM_VERSION 1083// 6 bytes = ESP3D<V on one byte>
///////////////////////////////////////// WIC
#define LAST_EEPROM_ADDRESS 1089
#endif//Valve_UI
#ifdef CircuitTesting_UI
#define QuaPin     1073//1  bytes = byte
#define EP_Pin_1   1074//1  bytes = byte
#define EP_Pin_2   1075//1  bytes = byte
#define EP_Pin_3   1076//1  bytes = byte
#define EP_Pin_4   1077//1  bytes = byte
#define EP_Pin_5   1078//1  bytes = byte
#define EP_Pin_6   1079//1  bytes = byte
#define EP_Pin_7   1080//1  bytes = byte
#define EP_Pin_8   1081//1  bytes = byte
#define EP_Pin_9   1082//1  bytes = byte
#define EP_Pin_10  1083//1  bytes = byte
#define EP_Pin_11  1084//1  bytes = byte
#define EP_Pin_12  1085//1  bytes = byte
#define EP_Pin_13  1086//1  bytes = byte
#define EP_PinMode 1087//1  bytes = byte
#define EP_EEPROM_VERSION 1088// 6 bytes = ESP3D<V on one byte>
///////////////////////////////////////// WIC
#define LAST_EEPROM_ADDRESS 1094
#endif//CircuitTesting_UI
#ifdef Gyro_UI
#define Address_File 1079// 1 bytes 
#define GyroState 1080// 1 bytes 
#define LAST_EEPROM_ADDRESS 1081
#define DEFAULT_GYRO_STATE 0 
#endif//Gyro_UI
#ifdef RFHub_UI


#define LAST_EEPROM_ADDRESS 1081
#endif//RFHub_UI

#ifdef MESHCOM_UI
#define EP_EEPROM_WIFI_MODE 1079// 1 bytes
#define LAST_EEPROM_ADDRESS 1081
#endif//MESHCOM_UI
#ifdef Moto_UI
#define EP_EEPROM_WIFI_MODE 1079// 1 bytes
#define EP_EEPROM_UPDATE_MODE 1080// 1 bytes
#define EP_EEPROM_DISPLAY 1081// 1 bytes
#define EP_EEPROM_DEBUG 1082 // 1 bytes flag
#define EP_EEPROM_WIFISTARTUP 1083 // 1 bytes flag
#define LAST_EEPROM_ADDRESS 1084
#define TIMESTAMP_FEATURE
#endif//MESHCOM_UI
#ifdef IOTDEVICE_UI
//   //define hardware
//   networkID = 1;
//   nodeID = 2;
//   NameBoard = "VPlab";
//   OLED_MODE = OLED_130;  //Bit0
//   ROLE = Node;           //Bit1
//   Debug = DEBUG;         //Bit2
//   WifiMode = MESHMODE;   //Bit3
//   ButtonUpdateFw = USE;  //Bit4
//   UpdateFw = 1;          //Bit5
#define EP_EEPROM_WIFI_MODE 1079// 1 bytes
// #define EP_EEPROM_UPDATE_MODE 1080// 1 bytes
// #define EP_EEPROM_DISPLAY 1081// 1 bytes
#define EP_EEPROM_ID 1080 // 1 bytes
#define EP_EEPROM_NET_ID 1081 // 1 bytes
#define EP_EEPROM_OLED_TYPE 1082 // 1 bytes flag
#define EP_EEPROM_ROLE 1083 // 1 bytes flag
#define EP_EEPROM_DEBUG 1084 // 1 bytes flag
#define EP_EEPROM_FW_BUTTON 1085 // 1 bytes
#define EP_EEPROM_UPDATE_FW 1086 // 1 bytes
#define EP_EEPROM_PIN_LEDFULL 1087 // 1 bytes
#define EP_EEPROM_LED_TYPE 1088 // 1 bytes
#define EP_EEPROM_PIN_BUTTON 1089 // 1 bytes
#define EP_EEPROM_NAME     1090//30  bytes = string
#define EP_EEPROM_CATEGORY  1121//1  bytes
#define EP_EEPROM_SLEEP_TIME 1122
#define LAST_EEPROM_ADDRESS 1122
// #define TIMESTAMP_FEATURE
#endif//IOTDEVICE_UI
#ifdef LOOKLINE_UI
#define TestDisplayIntro
// #define TEST_MODE
#define DEBUG_
#define EP_EEPROM_VERSION       1073// 6 bytes = ESP3D<V on one byte>
#define EP_EEPROM_TEST_MODE     1079// 4 bytes
#define EP_EEPROM_UPDATE_MODE   1083// 4 bytes
#define EP_EEPROM_DEBUG         1087// 4 bytes
#define EP_EEPROM_ID            1091// 4 bytes
#define EP_EEPROM_NETID         1095// 4 bytes
#define EP_EEPROM_CHANELS       1099// 4 bytes
#define EP_EEPROM_ROLE          1103// 4 bytes Node|gateway|repearter
#define EP_EEPROM_RUN           1107// 4 bytes Run/Stop/Sleep
#define EP_EEPROM_AMOUNTNODE    1111// 4 bytes
#define EP_EEPROM_COM_MODE      1115// 4 bytes
#define EP_EEPROM_MODULE_TYPE   1119// 4 bytes
#define EP_EEPROM_PLAN          1123// 4 bytes
#define EP_EEPROM_RESULT        1127// 4 bytes
#define EP_EEPROM_PLANMAX       1131// 4 bytes
#define EP_EEPROM_PCS           1135// 4 bytes
#define EP_EEPROM_TIME_PLAN     1139// 4 bytes
#define EP_EEPROM_TIMESENT      1143// 4 bytes
#define EP_EEPROM_URL_VER       1147// 100 bytes
#define EP_EEPROM_URL_FW        1247// 100 bytes
#define EP_EEPROM_PLAN_SET      1347// 4 bytes
#define EP_EEPROM_RESULT_SET    1351// 4 bytes
#define EP_EEPROM_ON_OFF        1355// 1 bytes
#define EP_EEPROM_COUNTER_DELAY 1356// 4 bytes

#define LAST_EEPROM_ADDRESS 1360
// #define MQTT_Mode
#endif//LOOKLINE_UI
#ifdef PLC_MASTER_UI
#define USE_LORA
#define TestDisplayIntro
// #define TEST_MODE
#define DEBUG_
#define EP_EEPROM_VERSION       1073// 6 bytes = ESP3D<V on one byte>
#define EP_EEPROM_TEST_MODE     1079// 4 bytes
#define EP_EEPROM_UPDATE_MODE   1083// 4 bytes
#define EP_EEPROM_DEBUG         1087// 4 bytes
#define EP_EEPROM_ID            1091// 4 bytes
#define EP_EEPROM_NETID         1095// 4 bytes
#define EP_EEPROM_CHANELS       1099// 4 bytes
#define EP_EEPROM_ROLE          1103// 4 bytes Node|gateway|repearter
#define EP_EEPROM_RUN           1107// 4 bytes Run/Stop/Sleep
#define EP_EEPROM_AMOUNTNODE    1111// 4 bytes
#define EP_EEPROM_COM_MODE      1115// 4 bytes
#define EP_EEPROM_MODULE_TYPE   1119// 4 bytes
#define EP_EEPROM_PLAN          1123// 4 bytes
#define EP_EEPROM_RESULT        1127// 4 bytes
#define EP_EEPROM_PLANMAX       1131// 4 bytes
#define EP_EEPROM_PCS           1135// 4 bytes
#define EP_EEPROM_TIME_PLAN     1139// 4 bytes
#define EP_EEPROM_TIMESENT      1143// 4 bytes
#define EP_EEPROM_URL_VER       1147// 100 bytes
#define EP_EEPROM_URL_FW        1247// 100 bytes
#define EP_EEPROM_PLAN_SET      1347// 4 bytes
#define EP_EEPROM_RESULT_SET    1351// 4 bytes
#define EP_EEPROM_ON_OFF        1355// 1 bytes
#define EP_EEPROM_COUNTER_DELAY 1356// 4 bytes
#define EP_LORA_CHANEL          1360//4  bytes = byte
#define EP_LORA_AIRRATE         1364//1  bytes = byte
#define EP_LORA_PROTOCOL        1365//1  bytes = byte
#define LAST_EEPROM_ADDRESS 1366
// #define MQTT_Mode
#endif//PLC_MASTER_UI
//default values
#define DEFAULT_WIFI_MODE           AP_MODE
#ifdef MESHCOM_UI
const char DEFAULT_AP_SSID []  PROGMEM =        "MeshVPlab";
#endif//MESHCOM_UI
#ifdef Gyro_UI
const char DEFAULT_AP_SSID []  PROGMEM =        "VPlab Gyro";
#endif//Gyro_UI
#ifdef Switch_UI
const char DEFAULT_AP_SSID []  PROGMEM =        "VPlab Smart Light";
#endif//Switch_UI
#ifdef ESP3D_UI
const char DEFAULT_AP_SSID []  PROGMEM =        "VPlab 3D";
#endif//ESP3D_UI
#ifdef Moto_UI
const char DEFAULT_AP_SSID []  PROGMEM =        "VPlab Moto";
#endif//ESP3D_UI
#ifdef CircuitTesting_UI
const char DEFAULT_AP_SSID []  PROGMEM =        "CircuitTesting";
#endif//CircuitTesting_UI
#ifdef IOTDEVICE_UI
const char DEFAULT_AP_SSID []  PROGMEM =        "IoT Device";
#endif//IOTDEVICE_UI
#ifdef LOOKLINE_UI
const char DEFAULT_AP_SSID []  PROGMEM =        "Lookline";
#endif//LOOKLINE_UI
#ifdef AutoIT_UI
const char DEFAULT_AP_SSID []  PROGMEM =        "AutoIT";
#endif//AutoIT_UI
#ifdef LOOKLINE_MASTER
#ifndef LOOKLINE_UI 
    // const char DEFAULT_AP_SSID []  PROGMEM =        "Lookline";
#endif//Lookline_UI
#endif//Lookline_MASTER
#ifdef PLC_MASTER_UI
const char DEFAULT_AP_SSID []  PROGMEM =        "PLC_Master";
#endif//PLC_MASTER_UI

#ifdef Basic_UI
const char DEFAULT_AP_SSID []  PROGMEM =        "VPlab";
#endif//PLC_MASTER_UI


const char DEFAULT_AP_PASSWORD [] PROGMEM = "12345678";
#ifdef LOOKLINE_UI
const char DEFAULT_STA_SSID []  PROGMEM =        "HUAWEI-8D22";
const char DEFAULT_STA_PASSWORD [] PROGMEM =    "77091836";
#else
const char DEFAULT_STA_SSID []  PROGMEM =        "VPLab";
const char DEFAULT_STA_PASSWORD [] PROGMEM =    "12345678";
#endif// LOOKLINE_UI
const byte DEFAULT_STA_IP_MODE  =               DHCP_MODE;
const byte DEFAULT_AP_IP_MODE =                 STATIC_IP_MODE;
const byte DEFAULT_IP_VALUE[]   =           {192, 168, 0, 1};
const byte DEFAULT_MASK_VALUE[]  =          {255, 255, 255, 0};
#define DEFAULT_GATEWAY_VALUE               DEFAULT_IP_VALUE
const long DEFAULT_BAUD_RATE =          115200;
#define DEFAULT_PHY_MODE            WIFI_PHY_MODE_11G
#define DEFAULT_SLEEP_MODE          WIFI_MODEM_SLEEP
#define DEFAULT_CHANNEL             11
#define DEFAULT_AUTH_TYPE           AUTH_WPA_PSK
#define DEFAULT_SSID_VISIBLE            1
#define DEFAULT_MAX_CONNECTIONS         4
#define DEFAULT_BEACON_INTERVAL         100
const int DEFAULT_WEB_PORT =            80;
const int DEFAULT_DATA_PORT =           8888;
const char DEFAULT_ADMIN_PWD []  PROGMEM =  "admin";
const char DEFAULT_USER_PWD []  PROGMEM =   "user";
const char DEFAULT_STAFF_PWD []  PROGMEM =   "staff";
const char DEFAULT_ADMIN_LOGIN []  PROGMEM =    "admin";
const char DEFAULT_USER_LOGIN []  PROGMEM = "user";
const char DEFAULT_STAFF_LOGIN []  PROGMEM = "staff";
const char DEFAULT_TIME_SERVER1 []  PROGMEM =   "1.pool.ntp.org";
const char DEFAULT_TIME_SERVER2 []  PROGMEM =   "2.pool.ntp.org";
const char DEFAULT_TIME_SERVER3 []  PROGMEM =   "0.pool.ntp.org";
const char DEFAULT_FW_VERSION_HOST []  PROGMEM =   "0.pool.ntp.org";
const char DEFAULT_FIRMWARE_HOST []  PROGMEM =   "0.pool.ntp.org";
const char DEFAULT_BOARD_NAME []  PROGMEM =   "name";
#define DEFAULT_TIME_ZONE           7
#define DEFAULT_TIME_DST            0
#define DEFAULT_PRIMARY_SD  2
#define DEFAULT_SECONDARY_SD 1
#define DEFAULT_DIRECT_SD_CHECK 0
#define DEFAULT_SD_CHECK_UPDATE_AT_BOOT 1
#define DEFAULT_OUTPUT_FLAG 0
#define DEFAULT_DHT_TYPE 255
const int DEFAULT_DHT_INTERVAL = 30;


#define MIN_NOTIFICATION_TOKEN_LENGTH 0
#define MIN_NOTIFICATION_SETTINGS_LENGTH 0
#define MAX_NOTIFICATION_TOKEN_LENGTH 63
#define MAX_NOTIFICATION_SETTINGS_LENGTH 127

#define DEFAULT_NOTIFICATION_TYPE 0
#define DEFAULT_NOTIFICATION_TOKEN1 ""
#define DEFAULT_NOTIFICATION_TOKEN2 ""
#define DEFAULT_NOTIFICATION_SETTINGS ""
#define DEFAULT_AUTO_NOTIFICATION_STATE 1
#define NOTIFICATION_ESP_ONLINE "Hi, %ESP_NAME% is now online at %ESP_IP%"

//Notifications
#define ESP_PUSHOVER_NOTIFICATION   1
#define ESP_EMAIL_NOTIFICATION      2
#define ESP_LINE_NOTIFICATION       3
#define ESP_IFTTT_NOTIFICATION      4

#ifdef LOOKLINE_UI
#define DEFAULT_PLAN           0
#define DEFAULT_PLANSET        1
#define DEFAULT_RESULT         0
#define DEFAULT_RESULTSET      1
#define DEFAULT_TIMEPLAN       100
#define DEFAULT_PLANLIMIT      9999
#define DEFAULT_PCS            100

#define DEFAULT_TIMESENT       15
#define DEFAULT_AMOUNTNODE     5
#define DEFAULT_BOARDID        1
#define DEFAULT_NETID          1
#define DEFAULT_CHANEL         0
#define DEFAULT_ROLE           0
#define DEFAULT_COMMODE        0
#define DEFAULT_MODULETYPE     0

#endif//LOOKLINE_UI

#ifdef SDCARD_FEATURE
#define DEFAULT_IS_DIRECT_SD 1
#ifdef Gyro_UI
#define SDCard_CS D0
#endif//Gyro_ui
#ifdef Valve_UI
#define SDCard_CS 5
#define SDSpeed 25
#endif//Valve_UI
#else
#define DEFAULT_IS_DIRECT_SD 0
#endif

//SD Card reader speed
//possible values are :SPI_FULL_SPEED, SPI_DIV3_SPEED,
//SPI_HALF_SPEED, SPI_DIV6_SPEED, SPI_QUARTER_SPEED,
//SPI_EIGHTH_SPEED, SPI_SIXTEENTH_SPEED
//Decrease if reader give error
#ifdef ARDUINO_ARCH_ESP8266
#define DEFAULT_SDREADER_SPEED 2
#else
#define DEFAULT_SDREADER_SPEED 4
#endif

const uint16_t Setting[][2] = {
    {EP_WIFI_MODE, LEVEL_ADMIN},//0
    {EP_STA_SSID, LEVEL_ADMIN},//1
    {EP_STA_PASSWORD, LEVEL_ADMIN},//2
    {EP_STA_IP_MODE, LEVEL_ADMIN},//3
    {EP_STA_IP_VALUE, LEVEL_ADMIN},//4
    {EP_STA_MASK_VALUE, LEVEL_ADMIN},//5
    {EP_STA_GATEWAY_VALUE, LEVEL_ADMIN},//6
    {EP_BAUD_RATE, LEVEL_ADMIN},//7
    {EP_STA_PHY_MODE, LEVEL_ADMIN},//8
    {EP_SLEEP_MODE, LEVEL_ADMIN},//9
    {EP_CHANNEL, LEVEL_ADMIN},//10
    {EP_AUTH_TYPE, LEVEL_ADMIN},//11
    {EP_SSID_VISIBLE, LEVEL_ADMIN},//12
    {EP_WEB_PORT, LEVEL_ADMIN},//13
    {EP_DATA_PORT, LEVEL_ADMIN},//14
    {EP_HOSTNAME, LEVEL_ADMIN},//15
    {EP_ADMIN_PWD, LEVEL_ADMIN},//16
    {EP_USER_PWD, LEVEL_USER},//17
    {EP_AP_SSID, LEVEL_ADMIN},//18
    {EP_AP_PASSWORD, LEVEL_ADMIN},//19
    {EP_AP_IP_VALUE, LEVEL_ADMIN},//20
    {EP_AP_MASK_VALUE, LEVEL_ADMIN},//21
    {EP_AP_GATEWAY_VALUE, LEVEL_ADMIN},//22
    {EP_AP_IP_MODE, LEVEL_ADMIN},//23
    {EP_AP_PHY_MODE, LEVEL_ADMIN},//24
    {EP_TARGET_FW, LEVEL_USER},//25
    {EP_TIMEZONE, LEVEL_USER},//26
    {EP_TIME_ISDST, LEVEL_USER},//27
    {EP_TIME_SERVER1, LEVEL_USER},//28
    {EP_TIME_SERVER2, LEVEL_USER},//29
    {EP_TIME_SERVER3, LEVEL_USER},//30
    {EP_IS_DIRECT_SD, LEVEL_USER},//31
    {EP_PRIMARY_SD, LEVEL_USER},//32
    {EP_SECONDARY_SD, LEVEL_USER},//33
    {EP_DIRECT_SD_CHECK, LEVEL_USER}, //34
    {EP_SD_CHECK_UPDATE_AT_BOOT, LEVEL_USER},//35
    {EP_OUTPUT_FLAG, LEVEL_USER},//36
    {EP_DHT_INTERVAL, LEVEL_USER},//37
    {EP_DHT_TYPE, LEVEL_USER},//38
    {EP_SD_SPEED_DIV, LEVEL_USER}//39
};
#define AUTH_ENTRY_NB 40

#define FLAG_BLOCK_M117 0x01
#define FLAG_BLOCK_OLED 0x02
#define FLAG_BLOCK_SERIAL 0x04
#define FLAG_BLOCK_WSOCKET 0x08
#define FLAG_BLOCK_TCP 0x010

//values
#define DEFAULT_MAX_WEB_PORT            65001
#define DEFAULT_MIN_WEB_PORT            1
#define DEFAULT_MAX_DATA_PORT           65001
#define DEFAULT_MIN_DATA_PORT           1

#define MAX_TRY 2000


//sizes
#define EEPROM_SIZE                     2024 //max is 1024
#define MAX_SSID_LENGTH                 32
#define MIN_SSID_LENGTH                 1
#define MAX_PASSWORD_LENGTH             64
//min size of password is 0 or upper than 8 char
//so let set min is 0
#define MIN_PASSWORD_LENGTH              0
#define MAX_LOCAL_PASSWORD_LENGTH        16
#define MIN_LOCAL_PASSWORD_LENGTH        1
#define MAX_DATA_LENGTH                  127
#define MIN_DATA_LENGTH                  0
#define IP_LENGTH                        4
#define INTEGER_LENGTH                   4
#define MAX_HOSTNAME_LENGTH              32
#define MIN_HOSTNAME_LENGTH              1
#define WL_MAC_ADDR_LENGTH               6

#define MAX_MQTT_USER_LENGTH             30
#define MIN_MQTT_USER_LENGTH             0
#define MAX_MQTT_PASS_LENGTH             30
#define MIN_MQTT_PASS_LENGTH             0
#define MAX_MQTT_BROKER_LENGTH           60
#define MIN_MQTT_BROKER_LENGTH           0


#define MAX_LORA_CH_LENGTH             441//930
#define MIN_LORA_CH_LENGTH             410//862


#define MAX_ID_LENGTH             999
#define MIN_ID_LENGTH             1

#define MAX_NET_ID_LENGTH             999
#define MIN_NET_ID_LENGTH             1

#define MAX_NAME_LENGTH             30
#define MIN_NAME_LENGTH             1

#define MAX_LORA_TIME_SCAN_LENGTH            59
#define MIN_LORA_TIME_SCAN_LENGTH            1

#define MAX_LORA_TIME_REQUEST_LENGTH            59
#define MIN_LORA_TIME_REQUEST_LENGTH            1

#define DEFAULT_MAX_PLAN            9999
#define DEFAULT_MIN_PLAN            0

#define DEFAULT_MAX_RESULT            9999
#define DEFAULT_MIN_RESULT            0

#define DEFAULT_MAX_PLAN_SET            9999
#define DEFAULT_MIN_PLAN_SET            0

#define DEFAULT_MAX_RESULT_SET            9999
#define DEFAULT_MIN_RESULT_SET            0

#define DEFAULT_MAX_PLANMAX            9999
#define DEFAULT_MIN_PLANMAX            0

#define DEFAULT_MAX_PCS            9999
#define DEFAULT_MIN_PCS            0

#define DEFAULT_MAX_TIME_PLAN            9999
#define DEFAULT_MIN_TIME_PLAN            0

#define DEFAULT_MAX_TIMESENT            9999
#define DEFAULT_MIN_TIMESENT            0

#define DEFAULT_MAX_AMOUNTNODE            255
#define DEFAULT_MIN_AMOUNTNODE            1

#define DEFAULT_MAX_ID            200
#define DEFAULT_MIN_ID            1

#define DEFAULT_MAX_NETID            200
#define DEFAULT_MIN_NETID            1

#define DEFAULT_MAX_CHANEL            31
#define DEFAULT_MIN_CHANEL            0

#define MAX_URL_VER_LENGTH                 100
#define MIN_URL_VER_LENGTH                 1

#define MAX_URL_FW_LENGTH                 100
#define MIN_URL_FW_LENGTH                 1

//EEPROM Version
#define EEPROM_V0 0
#define EEPROM_V1 1
#define EEPROM_V2 2

#define EEPROM_CURRENT_VERSION EEPROM_V2


#if defined(ASYNCWEBSERVER)
class AsyncResponseStream;
typedef  AsyncResponseStream ESPResponseStream;
#else
class ESPResponseStream
{
public:
    bool header_sent;
    String buffer_web;
    ESPResponseStream()
    {
        header_sent=false;
    };
};
#endif

class CONFIG
{
public:

    static void wait (uint32_t milliseconds);
    static void wdtFeed();
    static byte output_flag;
    static byte GyroStates; 
#ifdef DHT_FEATURE
    static byte DHT_type;
    static int DHT_interval;
    static void InitDHT(bool refresh = false);
#endif
    static bool is_com_enabled;
    static bool is_locked(byte flag);
    static bool is_direct_sd;
    static bool read_string (int pos, char byte_buffer[], int size_max);
    static bool read_string (int pos, String & sbuffer, int size_max);
    static bool read_buffer (int pos, byte byte_buffer[], int size_buffer);
    static bool read_byte (int pos, byte * value);
    static bool write_string (int pos, const char * byte_buffer);
    static bool write_string (int pos, const __FlashStringHelper *str);
    static bool write_buffer (int pos, const byte * byte_buffer, int size_buffer);
    static bool write_byte (int pos, const byte value);
    static bool reset_config();
    static void print_config (tpipe output, bool plaintext, ESPResponseStream  *espresponse = NULL);
    static bool SetFirmwareTarget (uint8_t fw);
    static void InitFirmwareTarget();
    static void InitOutput();
    static void InitDirectSD();
    static void InitPins();
    static bool InitBaudrate(long value = 0);
    static bool DisableSerial();
    static bool InitExternalPorts();
    static uint8_t GetFirmwareTarget();
    static const char* GetFirmwareTargetName();
    static const char* GetFirmwareTargetShortName();
    static uint8_t get_EEPROM_version();
    static bool set_EEPROM_version(uint8_t v);
    static bool adjust_EEPROM_settings();
    static bool isHostnameValid (const char * hostname);
    static bool isSSIDValid (const char * ssid);
    static bool isPasswordValid (const char * password);
    static bool isLocalPasswordValid (const char * password);
    static bool isIPValid (const char * IP);
    static char * intTostr (int value);
    static String formatBytes (uint64_t bytes);
    static char * mac2str (uint8_t mac [WL_MAC_ADDR_LENGTH]);
    static byte split_ip (const char * ptr, byte * part);
    static void esp_restart (bool async = false);
    
#ifdef Gyro_UI
    static bool GetState();
#endif//Gyro_UI    
#if defined(TIMESTAMP_FEATURE)
    static void init_time_client();
#endif
private:
    static uint8_t FirmwareTarget;
};



#endif
