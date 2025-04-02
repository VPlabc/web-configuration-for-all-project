#include <Arduino.h>
#include "config.h"
#ifdef Moto_UI
#include "Moto/Moto.h"
#endif//Moto_UI
#ifdef IOTDEVICE_UI
#include "AutoIT_IoT/IoTDevice.h"
IoT_Device IOT_DEVICE_FW;
// IOT_DEVICE IoTdevice
#endif//IOTDEVICE_UI
#ifdef LOOKLINE_UI
#include "LookLine/LookLine.h"
LOOKLINE_PROG LookLine_prog;
#endif // LOOKLINE_UI


#include "FirmwareUpdate.h"
UpdateFW FWUPD;
#include "esp_oled.h"
#include "espcom.h"
#ifdef ServerUpdateFW
#define Debug_Ser Serial
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
UpdateFW FWUPD_;
#else //ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#endif
//#include "FS.h"
//#include <LITTLEFS.h>
//#include <ArduinoJson.h>
#define FORMAT_SPIFFS_IF_FAILED true
WiFiClient            wifiClient;
#include <EEPROM.h>

#define ButtonBoot 0
#define LEDStatus D4
  // #ifdef Moto_UI
  // int LEDStatus = MOTO.LEDButton
  // #endif//Moto_UI
  // #ifdef IOTDEVICE_UI
  // int LEDStatus = IOT_DEVICE.LEDButton
  // #endif//Moto_UI

#define   PRGM_VERSION         "2.0.3"
/* this info will be read by the python script */
String hosts = "https://raw.githubusercontent.com/";
String URL_fw_Bin = "VPlabc/AutoIT/main/firmware.bin";
String URL_fw_Version = "VPlabc/AutoIT/main/version.txt";
const char* host = "raw.githubusercontent.com";
const int httpsPort = 443;
/* end of script data */

int fwVersion = 0;
bool fwCheck = false;
String fwUrl = "", fwName = "";
#endif//ServerUpdateFW

#ifdef ServerUpdateFW


#ifdef ARDUINO_ARCH_ESP8266
// DigiCert High Assurance EV Root CA
const char trustRoot[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";
X509List cert(trustRoot);
#else //ESP32
const char * rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
"-----END CERTIFICATE-----\n";
#endif//ARDUINO_ARCH_ESP8266

extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

unsigned long FW_previousMillis_2 = 0;
unsigned long FW_previousMillis = 0;        // will store last time LED was updated
const long interval = 10000;
const long mini_interval = 1000;
void FirmwareUpdate();
bool UDFOnce = true;
bool UDFOnce1 = true;
byte UFWDebug = 0;
byte counters = 0;
void setClock() {
  if(UDFOnce){UDFOnce = false;
    if(CONFIG::read_byte (EP_EEPROM_DEBUG, &UFWDebug)){
    LOGLN("UDF Debug:" + String((UFWDebug==0)?"Not Debug":"Debug"));
    }
  }
   // Set time via NTP, as required for x.509 validation
  configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  LOG("Waiting for NTP time sync: ");LOG(String(now) + "\n");
  while (now < 8 * 3600 * 2 && UDFOnce1 == 1) {
    delay(500);
    LOG(".");counters++;if(counters > 20){UDFOnce1 = false;}
    now = time(nullptr);
  }
}
bool FWonce = true; //
 void UpdateFW::repeatedCall(){
  
  if(FWonce){LOG("Check FW Working...\n");FWonce = false;}
     unsigned long currentMillis = millis();
    if ((currentMillis - FW_previousMillis) >= interval) 
     {
       // save the last time you blinked the LED
        FW_previousMillis = currentMillis;
          setClock();
          #ifdef ARDUINO_ARCH_ESP8266
          FirmwareUpdate();
          #else
          if(FirmwareVersionCheck()){
            FirmwareUpdate();
          }
          #endif//
     }
  //   if ((currentMillis - FW_previousMillis_2) >= mini_interval) {
  //     static int idle_counter=0;
  //     FW_previousMillis_2 = currentMillis;    
  //   //   debug(" Active fw version:");
  //   //   LOGLN(FirmwareVer);
  //   //   debug("Idle Loop....");
  //   //   LOGLN(idle_counter++);
  //   //  if(idle_counter%2==0)
  //   //   digitalWrite(LED_BUILTIN, HIGH);
  //   //  else 
  //   //   digitalWrite(LED_BUILTIN, LOW);
  //    //if(WiFi.status() == !WL_CONNECTED) 
  //         //connect_wifi();
  //  }
 }


  /*
  LOGLN("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  debug("Current time: ");
  debug(asctime(&timeinfo));
  */

  
void UpdateFW::FirmwareUpdate()
{ 
  #ifdef LOOKLINE_UI
    LookLine_prog.DebugOut("Update firmware....\n", OUPUT);
  #endif// LOOKLINE_UI
  #ifdef IOTDEVICE_UI
    if(UFWDebug){LOGLN("Update firmware....");
      
    }
    FWUPD.ShowMess("Update firmware....");
    #ifdef IOTDEVICE_UI
    {if(IOT_DEVICE_FW.getRunMode() == MESHSLAVE)IOT_DEVICE_FW.sendCurrentStatus(2);}
    #endif//IOTDEVICE_UI
    if(IOT_DEVICE_FW.LEDType == 0){IOT_DEVICE_FW.ledFadeToBeat(255,0,255,100);IOT_DEVICE_FW.colorWipe(0x000000, 100);}
    else{IOT_DEVICE_FW.LED_Signal(6, 100);}
  #endif//IOTDEVICE_UI
#ifdef ARDUINO_ARCH_ESP8266
  WiFiClientSecure client;
  client.setTrustAnchors(&cert);
  if (!client.connect(host, httpsPort)) {
    LOGLN("Connection failed");
  #ifdef LOOKLINE_UI
    LookLine_prog.DebugOut("Connection failed\n", OUPUT);
  #endif// LOOKLINE_UI
    
    return;
  }
  client.print(String("GET ") + URL_fw_Version + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      //LOGLN("Headers received");
      break;
    }
  }
  String payload = client.readStringUntil('\n');

  payload.trim();
  if(payload.equals(FWUPD_.FirmwareVer) )
  {   
     LOGLN("Device already on latest firmware version"); 
  }
  else
  {
    LOGLN("New firmware detected");
  #ifdef LOOKLINE_UI
    LookLine_prog.DebugOut("New firmware detected\n", OUPUT);
  #endif// LOOKLINE_UI
  #ifdef IOTDEVICE_UI
  ESPhttpUpdate.setLedPin(IOT_DEVICE_FW.LEDButton, LOW);
  #endif//Moto_UI
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, URL_fw_Bin);
        
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        LOGLN("HTTP_UPDATE_FAILD Error ("+ String(ESPhttpUpdate.getLastError())  + "): " + ESPhttpUpdate.getLastErrorString().c_str() + "\n");
        break;

      case HTTP_UPDATE_NO_UPDATES:
        LOGLN("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        LOGLN("HTTP_UPDATE_OK");
        break;
    } 
  }
  #else
    WiFiClientSecure client;
  client.setCACert(rootCACertificate);
  #ifdef Moto_UI
  httpUpdate.setLedPin(MOTO.LEDButton, LOW);
  #endif//Moto_UI
  #ifdef IOTDEVICE_UI
  httpUpdate.setLedPin(IOT_DEVICE_FW.LEDButton, LOW);
  #endif//Moto_UI
  t_httpUpdate_return ret = httpUpdate.update(client, hosts+URL_fw_Bin);

  switch (ret) {
  case HTTP_UPDATE_FAILED:
    #ifdef IOTDEVICE_UI
      if(UFWDebug)Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    #endif//#ifdef IOTDEVICE_UI
    
    #ifdef LOOKLINE_UI
      LookLine_prog.DebugOut("Update Faild!!\n", OUPUT);
    #endif// LOOKLINE_UI
      #ifdef ESP_OLED_FEATURE
      FWUPD.ShowMess("Update Faild!!");
      #endif//ESP_OLED_FEATURE
    break;

  case HTTP_UPDATE_NO_UPDATES:
    #ifdef IOTDEVICE_UI
      if(UFWDebug)Serial.println("HTTP_UPDATE_NO_UPDATES");
    #endif//#ifdef IOTDEVICE_UI
    #ifdef LOOKLINE_UI
      LookLine_prog.DebugOut("No Update!!\n", OUPUT);
    #endif// LOOKLINE_UI
      #ifdef ESP_OLED_FEATURE
      FWUPD.ShowMess("No Update!!");
      #endif//ESP_OLED_FEATURE
    break;

  case HTTP_UPDATE_OK:
    #ifdef IOTDEVICE_UI
    if(UFWDebug)Serial.println("HTTP_UPDATE_OK");
    #endif//#ifdef IOTDEVICE_UI
    #ifdef LOOKLINE_UI
      LookLine_prog.DebugOut("Update OK!!\n", OUPUT);
    #endif// LOOKLINE_UI
      #ifdef ESP_OLED_FEATURE
      FWUPD.ShowMess("Update OK!!");
      #endif//ESP_OLED_FEATURE
      #ifdef IOTDEVICE_UI
        if(IOT_DEVICE_FW.ROLE == Node){
          CONFIG::write_byte (EP_EEPROM_WIFI_MODE, MESHMODE);delay(3000);ESP.restart();
        }
        if(IOT_DEVICE_FW.ROLE == NodeGateway){
          CONFIG::write_byte (EP_EEPROM_WIFI_MODE, MESHSLAVE);delay(3000);ESP.restart();
        }
      #endif//IOTDEVICE_UI
    break;
  }
  #endif//ARDUINO_ARCH_ESP8266
 }
#ifndef ARDUINO_ARCH_ESP8266
 byte UpdateFW::FirmwareVersionCheck(void) {
  String payload;
  int HttpCode;
  String fwurl = "";
  fwurl += hosts+URL_fw_Version;
  fwurl += "?";
  fwurl += String(rand());
  #ifdef IOTDEVICE_UI
  if(UFWDebug)Serial.println(fwurl);
  #endif//#ifdef IOTDEVICE_UI
  WiFiClientSecure * client = new WiFiClientSecure;

  if (client) 
  {
    client -> setCACert(rootCACertificate);

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
    HTTPClient https;

    if (https.begin( * client, fwurl)) 
    { // HTTPS   
    #ifdef IOTDEVICE_UI   
      if(UFWDebug)Serial.print("[HTTPS] GET...\n");
      #endif//#ifdef IOTDEVICE_UI
      // start connection and send HTTP header
      delay(100);
      HttpCode = https.GET();
      delay(100);
      if (HttpCode == HTTP_CODE_OK) // if version received
      {
        payload = https.getString(); // save received version
      } else {
        #ifdef IOTDEVICE_UI
        if(UFWDebug)Serial.print("error in downloading version file:");
        if(UFWDebug)Serial.println(HttpCode);
        #endif//#ifdef IOTDEVICE_UI
    #ifdef LOOKLINE_UI
      LookLine_prog.DebugOut("error in downloading version file:" + String(HttpCode) + "\n", OUPUT);
    #endif// LOOKLINE_UI
      }
      https.end();
    }
    delete client;
  }
      
  if (HttpCode == HTTP_CODE_OK) // if version received
  {
    payload.trim();
    if (payload.equals(FirmwareVer)) {
      #ifdef LOOKLINE_UI
      LOGLN("Firmware detected ver:" + String(payload));
      #endif//#ifdef LOOKLINE_UI    
      #ifdef IOTDEVICE_UI
      if(UFWDebug)Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer);
     #endif//#ifdef IOTDEVICE_UI
     #ifdef ESP_OLED_FEATURE
      FWUPD.ShowMess("FW ver:" + String(FirmwareVer));
      #endif//ESP_OLED_FEATURE
      return 0;
    } 
    else 
    {
      
      #ifdef LOOKLINE_UI
      LOGLN("New firmware detected ver:" + String(payload));
      #endif//#ifdef LOOKLINE_UI      
      #ifdef IOTDEVICE_UI
      if(UFWDebug)Serial.println("New firmware detected ver:" + String(payload));
      #endif//#ifdef IOTDEVICE_UI
      #ifdef ESP_OLED_FEATURE
      FWUPD.ShowMess("New firmware detected");
      #endif//ESP_OLED_FEATURE
      
    #ifdef LOOKLINE_UI
      LookLine_prog.DebugOut("New firmware detected\n", OUPUT);
    #endif// LOOKLINE_UI
      return 1;
    }
  } 
  return 0;  
}  

#endif//ARDUINO_ARCH_ESP8266

void UpdateFW::ShowMess(String txt)
{
#ifdef ESP_OLED_FEATURE
        OLED_DISPLAY::BigDisplay("Firmware", 15, 17);
        OLED_DISPLAY::setCursor(0, 48);
        ESPCOM::print(txt.c_str(), OLED_PIPE);
#endif//ESP_OLED_FEATURE
}
#endif//ServerUpdateFW