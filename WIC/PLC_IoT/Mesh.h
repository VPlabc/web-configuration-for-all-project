#include "Arduino.h"
// #include "MeshWifi.h"
#include "config.h"
#ifdef MeshNetwork
#include "webinterface.h"
WEBINTERFACE_CLASS PLCweb_interface;
#include <esp_now.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>
// #include "DataLog.h"
#include "PLC_IoT/PLC_Master.h"
PLC_MASTER PLC_mesh;
#include <SPI.h>
#include <SD.h>
#define LED 8
#include "MQTTcom.h"
#include "esp_wifi.h"
// #include "DataLog.h"
// Replace with your network credentials (STATION)
// const char* ssid = "Lau B.";
// const char* password = "12345678";
// const char* ssid = "iSoft";
// const char* password = "i-soft@123";
AsyncWebSocket PLCws("/ws");

// #include "RealTimeClock.h"
// repondTime RepondTime;

esp_now_peer_info_t MeshSlave;
int chan; 

enum MessageType {PAIRING, DATA,};

MessageType MSmessageType;

int counter = 0;

// Structure example to receive data
// Must match the sender structure
#ifdef MotionData
typedef struct struct_message {
    uint8_t msgType;
    uint8_t rssi; 
    uint8_t id;
    uint8_t state;
    uint16_t distanceMoving;
    uint8_t energyMoving;
    uint16_t  distanceActive;
    uint8_t energyActive;
    uint16_t  maxStation;
    uint16_t  maxMoving;
    uint16_t  inactivity;
    float  VMT;
    float  AMT;
    float  VMain;
    float  AMain;
} struct_message;
#endif//MotionData

#ifdef RFData
typedef struct RFsensor_data {
    uint8_t msgType;
    uint8_t rssi; 
    uint8_t id;
    uint8_t state;
    uint8_t NodeId;
    float  VMT;
    float  AMT;
    float  VMain;
    float  AMain;
} RFsensor_data;

typedef struct RFdataRestore {
    uint8_t msgType;
    uint8_t rssi; 
    uint8_t id;
    uint8_t state;
    uint8_t NodeId;
    float  VMT;
    float  AMT;
    float  VMain;
    float  AMain;
    unsigned long  lastime;
} RFdataRestore;
//struct_message  msgSD;
RFdataRestore  RFsensorsFame;
RFdataRestore     RFsensors[NUM_SENSORS];
int RF_sensors_saved = 0;
#endif//RFData


#ifdef VOMData
typedef struct VOMsensor_data {
    uint8_t msgType;
    uint8_t rssi; 
    uint8_t id;
    uint8_t state;
    uint8_t NodeId;
    float  VMT;
    float  AMT;
    float  VMain;
    float  AMain;
} VOMsensor_data;
//struct_message  msgSD;
VOMsensor_data     VOMsensors[NUM_SENSORS];
int VOM_sensors_saved = 0;
#endif//VOMData

typedef struct struct_pairing {       // new structure for pairing
    uint8_t msgType;
    uint8_t id;
    uint8_t macAddr[6];
    uint8_t channel;
} struct_pairing;


#ifdef RFData
RFsensor_data incomingReadings;
RFsensor_data outgoingSetpoints;
#endif//RFData


#ifdef VOMData
VOMsensor_data incomingReadings;
VOMsensor_data outgoingSetpoints;
#endif//VOMData

struct_pairing MSpairingData;

// uint8_t NodeID[100];
// uint8_t StatE[100];
// int RssI[100];
// float Vsolar[100];
// float Asolar[100];
// float Vmain[100];
// float Amain[100];
// long lastTime[100];

int MeshconnectWebSocket = 0;
byte MeshDebug = 0;
byte rssi_display = 0;
unsigned long SentlastEventTime = 0;
unsigned long SentlastEventTime1 = 0;
void SaveData(String NameFile, String Data);
void SendtoWeb();
void SaveSensors(int ID);

String DataLogStr = "";

void setConnect(int connectState){MeshconnectWebSocket = connectState;}
void readMemoryFromFileS();
void saveMemoryToFileS(int sensorssaved);
// ---------------------------- esp_ now -------------------------
void printMAC(const uint8_t * mac_addr){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
}

bool addPeer(const uint8_t *peer_addr) {      // add pairing
  memset(&MeshSlave, 0, sizeof(MeshSlave));
  const esp_now_peer_info_t *peer = &MeshSlave;
  memcpy(MeshSlave.peer_addr, peer_addr, 6);
  
  MeshSlave.channel = chan; // pick a channel
  MeshSlave.encrypt = 0; // no encryption
  // check if the peer exists
  bool exists = esp_now_is_peer_exist(MeshSlave.peer_addr);
  if (exists) {
    // MeshSlave already paired.
    Serial.println("Already Paired");
    return true;
  }
  else {
    esp_err_t addStatus = esp_now_add_peer(peer);
    if (addStatus == ESP_OK) {
      // Pair success
      Serial.println("Pair success");
      return true;
    }
    else 
    {
      Serial.println("Pair failed");
      return false;
    }
  }

} 
byte MeshResent = 0;
String message = "";



// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success to " : "Delivery Fail to ");
  printMAC(mac_addr);
  Serial.println();
  if (status == ESP_NOW_SEND_SUCCESS && (outgoingSetpoints.NodeId > 0 )){Serial.println("Success");SentlastEventTime = millis();MeshResent++;
  DataLogStr = String(incomingReadings.VMT) + "," + String(incomingReadings.AMT) + "," + String(incomingReadings.VMain) + "," + String(incomingReadings.AMain);
  SaveData(String (incomingReadings.id) + "_DataLog_" , DataLogStr);outgoingSetpoints.NodeId = 0;
  SaveSensors(incomingReadings.id);
  }
  MeshResent++;if (MeshResent>5){MeshResent = 0;Serial.println("Done");outgoingSetpoints.NodeId = 0;}
}

/// @brief Mesh recive Data
/// @param mac_addr 
/// @param incomingData 
/// @param len 
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  Serial.print(len);
  Serial.print(" bytes of data received from : ");
  printMAC(mac_addr);
  Serial.println();
  StaticJsonDocument<1000> root;
  String payload;
  uint8_t type = incomingData[0];       // first message byte is the type of message 
  switch (type) {
  case DATA :                           // the message is data type
    memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    // create a JSON document with received data and send it by event to the web page
    
    root["id"] = String(incomingReadings.id);
    root["state"] = String(incomingReadings.state);
    root["rssi"] = String(incomingReadings.rssi);
    root["vmt"] = String(incomingReadings.VMT );
    root["amt"] = String(incomingReadings.AMT );
    root["vmain"] = String(incomingReadings.VMain );
    root["amain"] = String(incomingReadings.AMain );
    CFrepondTime MeshRepondTime;
    MeshRepondTime = CONFIG::init_time_client();
    LOGLN("nows: " + String(MeshRepondTime.epochTime));
    root["times"] = String(MeshRepondTime.epochTime);
    
   
    serializeJson(root, payload);
    Serial.print("event send :");
    serializeJson(root, Serial);

    if(MeshconnectWebSocket == 1 || MeshconnectWebSocket == 2){socket_server->sendTXT(ESPCOM::current_socket_id, payload.c_str());}
    // events.send(payload.c_str(), "new_readings", millis());
    Serial.println();
    outgoingSetpoints.NodeId = incomingReadings.id;//recive done send callback to ID
    break;
  
  case PAIRING:                            // the message is a pairing request 
    memcpy(&MSpairingData, incomingData, sizeof(MSpairingData));
    // Serial.println(MSpairingData.msgType);
    // Serial.println(MSpairingData.id);
    Serial.print("Pairing request from: ");
    printMAC(mac_addr);
    Serial.println();
    // Serial.println(MSpairingData.channel);
    if (MSpairingData.id > 0) {     // do not replay to server itself
      if (MSpairingData.msgType == PAIRING) { 
        MSpairingData.id = 0;       // 0 is server
        // Server is in AP_STA mode: peers need to send data to server soft AP MAC address 
        WiFi.softAPmacAddress(MSpairingData.macAddr);   
        MSpairingData.channel = chan;
        Serial.println("send response");
        esp_now_send(mac_addr, (uint8_t *) &MSpairingData, sizeof(MSpairingData));
        addPeer(mac_addr);
      }  
    }  
    break; 
  }
}
/// @brief 
/////////////////////////////////////////////// WIFI RF Function /////////////////////////////////////////////////
typedef struct {
  unsigned frame_ctrl: 16;
  unsigned duration_id: 16;
  uint8_t addr1[6]; /* receiver address */
  uint8_t addr2[6]; /* sender address */
  uint8_t addr3[6]; /* filtering address */
  unsigned sequence_ctrl: 16;
  uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;
//La callback que hace la magia
void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
  // All espnow traffic uses action frames which are a subtype of the mgmnt frames so filter out everything else.
  if (type != WIFI_PKT_MGMT)
    return;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  int rssi = ppkt->rx_ctrl.rssi;
  rssi_display = rssi;
}
uint8_t current_protocol;
wifi_interface_t current_wifi_interface;
int check_protocol()
{
  CONFIG::read_byte (EP_EEPROM_DEBUG, &MeshDebug);
    char error_buf1[100];
  if(MeshDebug){
    LOGLN();
    LOGLN("Lookline_________________________");
    LOGLN();
     esp_err_t error_code = esp_wifi_get_protocol(current_wifi_interface, &current_protocol);
     esp_err_to_name_r(error_code,error_buf1,100);
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
}
// #define WIFI_PROTOCOL_11B         1
// #define WIFI_PROTOCOL_11G         2
// #define WIFI_PROTOCOL_11N         4
// #define WIFI_PROTOCOL_LR          8
void initESP_NOW(){
  // esp_wifi_set_protocol(current_wifi_interface, WIFI_PROTOCOL_LR);
  // if(MeshDebug)check_protocol();
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
} 

void Mesh_setup() {
  // Initialize Serial Monitor
//   Serial.begin(9600);

  Serial.println();
  Serial.print("Server MAC Address:  ");
  Serial.println(WiFi.macAddress());

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);
  // Set device as a Wi-Fi Station
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {delay(1000);
//     Serial.println("Setting as a Wi-Fi Station..");
//   }

//   Serial.print("Server SOFT AP MAC Address:  ");
//   Serial.println(WiFi.softAPmacAddress());

//   chan = WiFi.channel();
//   Serial.print("Station IP Address: ");
//   Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());


  #ifdef RFData
  readMemoryFromFileS();
  #endif//RFData
  LOGLN("DATA READ | Sensor Saved: " + String(RF_sensors_saved) + "| ")
    for(byte i = 0 ; i < RF_sensors_saved + 1; i++){
      // NodeID[i] = RFsensors[i].id;
      
    }
  initESP_NOW(); 
//   // Start Web server
//   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//     request->send_P(200, "text/html", index_html);
//   });
  

//   // Events 
//   events.onConnect([](AsyncEventSourceClient *client){
//     if(client->lastId()){
//       Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
//     }
//     // send event with message "hello!", id current millis
//     // and set reconnect delay to 1 second
//     client->send("hello!", NULL, millis(), 10000);
//   });
//   server.addHandler(&events);
  
//   // start server
//   server.begin();
}

byte Sentcount = 0;
void Mesh_loop() {
  static const unsigned long EVENT_INTERVAL_MS = 500;
  if ((millis() - SentlastEventTime) > EVENT_INTERVAL_MS) {
    if(outgoingSetpoints.NodeId > 0 ){
      outgoingSetpoints.msgType = DATA;
      Serial.println("Event interval");
    //   events.send("ping",NULL,millis());
      SentlastEventTime = millis();
    //   readDataToSend();
      esp_now_send(NULL, (uint8_t *) &outgoingSetpoints, sizeof(outgoingSetpoints));
      Serial.println("Sent "  + String(outgoingSetpoints.NodeId));
      Sentcount++;if(Sentcount>5){Sentcount = 0;outgoingSetpoints.NodeId = 0;}
    }
  }

  static const unsigned long EVENT_INTERVAL_MS1 = 1000;
  if ((millis() - SentlastEventTime1) > EVENT_INTERVAL_MS1) {
      SendtoWeb();
    //   events.send("ping",NULL,millis());
      SentlastEventTime1 = millis();
  }
}

int countEventsSave = 0;
void SendtoWeb(){
    StaticJsonDocument<1000> root;
    String payload; 
    for(byte i = 0; i < RF_sensors_saved ; i++) {
      root["id"] = String( RFsensors[i].id);
      root["state"] = String(RFsensors[i].state);
      root["rssi"] = String(RFsensors[i].rssi);
      root["vmt"] = String( RFsensors[i].VMT);
      root["amt"] = String(RFsensors[i].AMT);
      root["vmain"] = String( RFsensors[i].VMain);
      root["amain"] = String( RFsensors[i].AMain);
      root["times"] = String(RFsensors[i].lastime);


      serializeJson(root, payload);
    // Serial.print("event send :");
    // serializeJson(root, Serial);
    // Serial.println("________________________________________________________________");
    // Serial.println();
      if ((( RFsensors[i].VMT > 0 &&  RFsensors[i].VMT < 10) || (RFsensors[i].VMain > 0 && RFsensors[i].VMain < 5)) && mqttConnected && MQTTCOM::GetDataACK() == 0){PLC_mesh.PushMQTT(payload, "/isoft_sensor/update" + String(RFsensors[i].id));}
      if(MeshconnectWebSocket == 1 || MeshconnectWebSocket == 2){socket_server->broadcastTXT(payload.c_str());}
        payload =""; 
    }
    // LOGLN("mqttConnected: " + String(mqttConnected));
    // LOGLN("MeshconnectWebSocket: " + String(MeshconnectWebSocket));
}


void SaveSensors(int ID) {
static bool sd_card_found;
static bool new_sensor_found;
new_sensor_found = false;
    CFrepondTime MeshRepondTime;
    MeshRepondTime = CONFIG::init_time_client();

  for (int i = 0; i < RF_sensors_saved; i++) {
    if (RFsensors[i].id == ID){
       RFsensors[i].state = incomingReadings.state;
       RFsensors[i].rssi = incomingReadings.rssi;
       RFsensors[i].VMT = incomingReadings.VMT;
       RFsensors[i].AMT = incomingReadings.AMT;
       RFsensors[i].VMain = incomingReadings.VMain;
       RFsensors[i].AMain = incomingReadings.AMain;
       RFsensors[i].lastime = MeshRepondTime.epochTime;
      new_sensor_found = true;
    }
  }

  if ( new_sensor_found == false ) {
    RFsensors[RF_sensors_saved].id = incomingReadings.id;
    RFsensors[RF_sensors_saved].state = incomingReadings.state;
    RFsensors[RF_sensors_saved].rssi = incomingReadings.rssi;
    RFsensors[RF_sensors_saved].VMT = incomingReadings.VMT;
    RFsensors[RF_sensors_saved].AMT = incomingReadings.AMT;
    RFsensors[RF_sensors_saved].VMain = incomingReadings.VMain;
    RFsensors[RF_sensors_saved].AMain = incomingReadings.AMain;
    RFsensors[RF_sensors_saved].lastime = MeshRepondTime.epochTime;
    RF_sensors_saved++;LOGLN("\nSaved New Sensor id " + String(incomingReadings.id));
  }
    if (!SD.begin(SDCard_CS)) {sd_card_found = false;} else {sd_card_found = true; }
    //LOG ("saveMemoryToFile > SD Card found:" + String(sd_card_found) + '\n');
    //LOG ("Time:" + String(timeClient.getEpochTime()) + '\n');
  if (sd_card_found) {
    saveMemoryToFileS(RF_sensors_saved);
  }
}


void saveMemoryToFileS(int sensorssaved) {
  static bool sd_card_found;
static bool new_sensor_found;
  if (!SD.begin(SDCard_CS)) {sd_card_found = false;} else {sd_card_found = true; }
  LOG ("saveMemoryToFile: " + String(sd_card_found) +"\n");

    if (SD.exists("/memory.bin")) {SD.remove("/memory.bin");}
    File file = SD.open("/memory.bin", "w");
    if (file) {
      if (file.write(sensorssaved)) {
      for (int i = 0; i < sensorssaved; i++) file.write((uint8_t *)&RFsensors[i], sizeof(struct RFdataRestore));
      }else {
      LOGLN ("write failed");
      }
    }else {
    LOGLN ("Open failed");
    }
  file.close();
}

void readMemoryFromFileS() {
  static bool sd_card_found;
static bool new_sensor_found;
  if (!SD.begin(SDCard_CS)) {sd_card_found = false;} else {sd_card_found = true; }
  LOG ("readMemoryFromFile: " + String(sd_card_found) + "\n");
  if (sd_card_found == true) {
    if (SD.exists("/memory.bin")) {
      File file = SD.open("/memory.bin", "r");
      RF_sensors_saved = file.read();//LOG (String(sensors_saved) + " Device Exists\n");
      for (int i = 0; i <RF_sensors_saved; i++) file.read((uint8_t *)&RFsensors[i], sizeof(struct RFdataRestore));
      file.close();
    }
    else{
    LOG ("File not Exists\n");
    }
  }
}


#endif//Meshnetwaork