#include "Arduino.h"
// #include "MeshWifi.h"
#include <esp_now.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>
#include "DataLog.h"

#define LED 8

// Replace with your network credentials (STATION)
// const char* ssid = "Lau B.";
// const char* password = "12345678";
// const char* ssid = "iSoft";
// const char* password = "i-soft@123";

esp_now_peer_info_t MeshSlave;
int chan; 

enum MessageType {PAIRING, DATA,};

MessageType MSmessageType;

int counter = 0;

// Structure example to receive data
// Must match the sender structure
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

typedef struct struct_pairing {       // new structure for pairing
    uint8_t msgType;
    uint8_t id;
    uint8_t macAddr[6];
    uint8_t channel;
} struct_pairing;

struct_message incomingReadings;
struct_message outgoingSetpoints;
struct_pairing MSpairingData;

unsigned long SentlastEventTime = 0;
void SaveData(String NameFile, String Data);

String DataLogStr = "";
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
// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success to " : "Delivery Fail to ");
  printMAC(mac_addr);
  Serial.println();
  if (status == ESP_NOW_SEND_SUCCESS && (outgoingSetpoints.energyMoving > 0 )){Serial.println("Success");SentlastEventTime = millis();MeshResent++;
  DataLogStr = String(incomingReadings.VMT) + "," + String(incomingReadings.AMT) + "," + String(incomingReadings.VMain) + "," + String(incomingReadings.AMain);
  SaveData("DataLog_" , DataLogStr);outgoingSetpoints.energyMoving = 0;
  }
  MeshResent++;if (MeshResent>5){MeshResent = 0;Serial.println("Done");outgoingSetpoints.energyMoving = 0;}
}


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
    root["id"] = incomingReadings.id;
    // root["distanceStat"] = incomingReadings.distanceActive;
    // root["energyStat"] = incomingReadings.energyActive;
    // root["distanceMov"] = String(incomingReadings.distanceMoving);
    // root["energyMov"] = String(incomingReadings.energyMoving);
    root["state"] = String(incomingReadings.state);
    root["rssi"] = String(incomingReadings.rssi);
    root["vmt"] = String(incomingReadings.VMT );
    root["amt"] = String(incomingReadings.AMT );
    root["vmain"] = String(incomingReadings.VMain );
    root["amain"] = String(incomingReadings.AMain );
    serializeJson(root, payload);
    Serial.print("event send :");
    serializeJson(root, Serial);
    // events.send(payload.c_str(), "new_readings", millis());
    Serial.println();
    outgoingSetpoints.energyMoving = incomingReadings.id;//recive done send callback to ID
    break;
  
  case PAIRING:                            // the message is a pairing request 
    memcpy(&MSpairingData, incomingData, sizeof(MSpairingData));
    Serial.println(MSpairingData.msgType);
    Serial.println(MSpairingData.id);
    Serial.print("Pairing request from: ");
    printMAC(mac_addr);
    Serial.println();
    Serial.println(MSpairingData.channel);
    if (MSpairingData.id > 0) {     // do not replay to server itself
      if (MSpairingData.msgType == PAIRING) { 
        MSpairingData.id = 0;       // 0 is server
        // Server is in AP_STA mode: peers need to send data to server soft AP MAC address 
        WiFi.softAPmacAddress(MSpairingData.macAddr);   
        MSpairingData.channel = chan;
        Serial.println("send response");
        esp_err_t result = esp_now_send(mac_addr, (uint8_t *) &MSpairingData, sizeof(MSpairingData));
        addPeer(mac_addr);
      }  
    }  
    break; 
  }
}

void initESP_NOW(){
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
    if(outgoingSetpoints.energyMoving > 0 ){
      outgoingSetpoints.msgType = DATA;
      Serial.println("Event interval");
    //   events.send("ping",NULL,millis());
      SentlastEventTime = millis();
    //   readDataToSend();
      esp_now_send(NULL, (uint8_t *) &outgoingSetpoints, sizeof(outgoingSetpoints));
      Serial.println("Sent "  + String(outgoingSetpoints.energyMoving));
      Sentcount++;if(Sentcount>5){Sentcount = 0;outgoingSetpoints.energyMoving = 0;}
    }
  }
}