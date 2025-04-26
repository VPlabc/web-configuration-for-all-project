#ifndef WifiRF_
#define WifiRF_
#include <Arduino.h>
#include "config.h"

#include "esp_wifi.h"
#include <esp_now.h>
#include <WiFi.h>

#define MESHSLAVE 0
#define WIFIMODE 1
#define MESHMODE 2
class WifiRF
{
public:
    byte Debug = false;
    long counter();
    int check_protocol();
    void WifiBegin();

    void sendDataNode();
    void getDataNode();
    
};
extern WifiRF WIFIRF;
#endif//WifiRF_