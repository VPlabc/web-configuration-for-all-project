#ifdef WIFIRF
#include "WifiRF.h"
#include "MeshWifi.h"

    byte RFRunMode;   
    byte networkID = 1;       //1
    byte nodeID = 1;          //1
    byte status = 1;            //1
    long timeout = 0;
typedef struct struct_command_message {
    byte Command;       //1
    byte networkID;       //1
    byte nodeID;          //1
    byte category;        //1
    byte time;            //1
    byte status;            //1
} struct_command_message;
struct_command_message DataCommand;

    int WifiRF::check_protocol()
    {
    char error_buf1[100];
    if(WIFIRF.Debug){
        LOGLN();
        LOGLN("___________________________________");
        LOGLN();
    }
        esp_err_t error_code = esp_wifi_get_protocol(current_wifi_interface, &current_protocol);
        esp_err_to_name_r(error_code,error_buf1,100);
    if(Debug){
        LOG("esp_wifi_get_protocol error code: ");
        LOGLN(error_buf1);
        LOGLN("Code: " + String(current_protocol));
        if ((current_protocol&WIFI_PROTOCOL_11B) == WIFI_PROTOCOL_11B)
        LOGLN("Protocol is WIFI_PROTOCOL_11B");
        if ((current_protocol&WIFI_PROTOCOL_11G) == WIFI_PROTOCOL_11G)
        LOGLN("Protocol is WIFI_PROTOCOL_11G");
        if ((current_protocol&WIFI_PROTOCOL_11N) == WIFI_PROTOCOL_11N)
        LOGLN("Protocol is WIFI_PROTOCOL_11N");
        if ((current_protocol&WIFI_PROTOCOL_LR) == WIFI_PROTOCOL_LR)
        LOGLN("Protocol is WIFI_PROTOCOL_LR");
        LOGLN("___________________________________");
        LOGLN();
        LOGLN();
    }
        return current_protocol;
    }//check_protocol()

void MeshRecive(const uint8_t * mac, const uint8_t *incomingDatas, int len);

    void Mesh_Init(){
    #ifndef ARDUINO_ARCH_ESP8266
    String dataDisp = "";
    if (!CONFIG::is_locked(FLAG_BLOCK_OLED)) {
    // IOT_DEVICE.OLED_Display("Mesh Init",2);}
    //     else{ OLED_DISPLAY::display_text("Mesh Init", 0, 0, 85);
    }
    // Init ESP-NOW
    WiFi.mode(WIFI_STA);
    #ifdef ARDUINO_ARCH_ESP8266
    #else
    if(WIFIRF.check_protocol() != 8){
    esp_wifi_set_protocol(current_wifi_interface, WIFI_PROTOCOL_LR);
    WIFIRF.check_protocol();
    }
    #endif//#ifdef ARDUINO_ARCH_ESP8266
    if (esp_now_init() != ESP_OK) {
        if (!CONFIG::is_locked(FLAG_BLOCK_OLED)) {
        // IOT_DEVICE.OLED_Display("Error!!",3);
        }
        // else{dataDisp = "Error!!";OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 16, 128);}
        if(WIFIRF.Debug)LOGLN("Error initializing ESP-NOW");
        // strip.setBrightness(100);strip.setPixelColor(0, 0xff0000);strip.show(); 
        return;
    }
    else{
        // if (!CONFIG::is_locked(FLAG_BLOCK_OLED)) {
        // IOT_DEVICE.OLED_Display("OK!!",3);}
        // else{dataDisp = "OK!!";OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 16, 128);}
        if(WIFIRF.Debug)LOGLN("initializing ESP-NOW OK");
        // strip.setBrightness(100);strip.setPixelColor(0, 0x00ff00);strip.show(); 
    }
    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        if(WIFIRF.Debug)LOGLN("Failed to add peer");
        if (!CONFIG::is_locked(FLAG_BLOCK_OLED)) {
        // IOT_DEVICE.OLED_Display("Failed to add peer",4);
        }
        // else{dataDisp = "Failed to add peer!!";OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 24, 128);}
        // strip.setBrightness(100);strip.setPixelColor(1, 0xff0000);strip.show();
        // RFRunMode = 1;
        return;
    }
    else{
        if(WIFIRF.Debug)LOGLN("add peer OK");
        if (!CONFIG::is_locked(FLAG_BLOCK_OLED)) {
        // IOT_DEVICE.OLED_Display("add peer OK",4);
        }
        // else{dataDisp = "add peer OK!!";OLED_DISPLAY::display_text(dataDisp.c_str(), 0, 24, 128);}
        // strip.setBrightness(100);strip.setPixelColor(1, 0x00ff00);strip.show(); 
        }
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(MeshRecive);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
    #endif//
    }//Mesh_Init()

    void WifiRF::WifiBegin()
    {
    CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &RFRunMode);
        if(RFRunMode == MESHSLAVE){if(WIFIRF.Debug)LOGLN("RFRunMode: Mesh Slave");}
        if(RFRunMode == MESHMODE){if(WIFIRF.Debug)LOGLN("RFRunMode: Mesh (Node)");}
        if(RFRunMode == WIFIMODE){if(WIFIRF.Debug)LOGLN("RFRunMode: Wifi (Gateway)");}
    #ifdef ARDUINO_ARCH_ESP8266
            WiFi.setPhyMode (WIFI_PHY_MODE_11G);
            WifiMode();
    #else
    if(RFRunMode == MESHMODE || RFRunMode == MESHSLAVE){
        if(WIFIRF.Debug)LOGLN("Mesh Init");
        WiFi.disconnect();WiFi.mode(WIFI_OFF); Mesh_Init(); 
        // OLED_DISPLAY::clear_lcd();
        // if(LEDType == 0){ledFadeToBeat(255,255,0,BRIGHTNESS_HEART);colorWipe(0x000000, 100);}
        // else{LED_Signal(4, 100);}
        }
    if(RFRunMode == WIFIMODE){
        //WifiMode();
        }
    #endif//
    }
    long WifiRF::counter(){    
        unsigned long previousMillis = 0;
        const long interval = 100;
        if (millis() - previousMillis >= interval) {
            previousMillis = millis();timeout++;
            // Perform timeout-related operations here
        }
        return timeout;
    }
    //----------------------------------------------------------------
    //------------ Mesh Reciver Function   ---------------------------
    //----------------------------------------------------------------
    // Callback when data is received Mesh

void MeshRecive(const uint8_t * mac, const uint8_t *incomingDatas, int len) {
    if(RFRunMode == MESHMODE){
        // if(len == sizeof(incomingReadings)) {
        // memcpy(&incomingReadings, incomingDatas, sizeof(incomingReadings));
        // LOG("Data  incomingReadings");
        // // LOGLN(len);
        // incomingnetworkID = incomingReadings.networkID;
        // incomingnodeID = incomingReadings.nodeID;
        // incomingstatus = incomingReadings.status;
        // // incomingCatagory = incomingReadings.category;
        // // incominghumidity = incomingReadings.humidity;
        // // incomingmbattery = incomingReadings.mbattery;
        // // incomingbattery = incomingReadings.battery;
        // // incomingrssi_display = incomingReadings.RSSI;
        // // IOT_DEVICE.updateDisplay();
        // }
        if(len == sizeof(DataCommand)){// Control by Mesh
        memcpy(&DataCommand, incomingDatas, sizeof(DataCommand));
        if(DataCommand.Command == 1 && DataCommand.networkID == networkID && DataCommand.nodeID == nodeID){

        status = DataCommand.status;
        if(status == 1){timeout = 0;}
            // if(IOT_DEVICE.Debug){LOGLN("Data Controller");
            // LOG("Network ID:" + String( DataCommand.networkID) + " | ID:" + String( DataCommand.nodeID));
            // LOGLN(" | state:" + String(DataCommand.category));}
            // state = DataCommand.category;LOGLN("update state");
            // Status = state;delay(500);
            // IOT_DEVICE.sendDataNode();
        }// Setting by Mesh
        
        }
    }//RFRunMode == MESHMODE
}
void WifiRF::getDataNode(){
 LOGLN("Data Controller");   
}
    //----------------------------------------------------------------
    //--------------- Mesh Send Function   ---------------------------
    //----------------------------------------------------------------



void WifiRF::sendDataNode(){
    DataCommand.networkID = networkID;
    DataCommand.nodeID = nodeID;
    DataCommand.status = status;
    // DataCommand.RSSI = rssi_display;
    if(Debug){
      LOGLN("DATA SENDDINGS");
      LOG("network ID: ");LOG(networkID);
      LOG(" | nodeID: ");LOG(nodeID);
      LOG(" | status: ");LOG(status);
      LOG(" | RSSI: ");LOG(rssi_display);
      LOGLN();
    }
    #ifdef ESP32
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DataCommand, sizeof(DataCommand));
    if (result == ESP_OK) {
      if(Debug)LOGLN("Sent with success");
    }
    else {
      if(Debug)LOGLN("Error sending the data");
      //  IOT_DEVICE.MeshBegin();
    }
    #endif//def ESP32
}
#endif//WIFIRF