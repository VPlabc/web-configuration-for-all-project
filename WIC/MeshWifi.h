#ifndef MeshWifi
#define MeshWifi
#include "config.h"
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#include <espnow.h>
#else
#include "esp_wifi.h"
#include <esp_now.h>
#include <WiFi.h>
#endif//#ifdef ARDUINO_ARCH_ESP8266
#ifdef IOTDEVICE_UI
#include "AutoIT_IoT/IoTDevice.h"
#endif//#ifdef IOTDEVICE_UI
#ifdef LOOKLINE_UI
#include  "Lookline/Lookline.h"
#endif//LOOKLINE_UI
//---------------------------------------------- Mesh
#define MAX_CHANNEL 11  // for North America // 13 in Europe
#define LED 8
#define SAVE_CHANNEL

long interval = 10000;        // Interval at which to publish sensor readings
// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t defserverAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

int rssi_display;
#ifdef ARDUINO_ARCH_ESP8266

#else
uint8_t current_protocol;
esp_now_peer_info_t peerInfo;
esp_interface_t current_esp_interface;
wifi_interface_t current_wifi_interface;


// Estructuras para calcular los paquetes, el RSSI, etc
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


 struct struct_command {  
    uint8_t msgType;
    uint8_t id;
    uint8_t command;
} ;
struct_command structcommand;

 struct struct_setting {  
    uint8_t msgType;
    uint8_t id;
    uint8_t targetmoving[9]; //
    uint8_t targetstation[9]; //
} ;
struct_setting structsetting;

 struct struct_pairing {       // new structure for pairing
    uint8_t msgType;
    uint8_t id;
    uint8_t macAddr[6];
    uint8_t channel;
};
struct_pairing structpairing;
 struct DataTrans {
unsigned long currentMillis = millis();
unsigned long previousMillis = 0; 
// unsigned long currentMillisPing = 0;
// unsigned long previousMillisPing = 0;  
// int rssi_display;
// long interval = 10000;        // Interval at which to publish sensor readings
};

enum PairingStatus {NOT_PAIRED, PAIR_REQUEST, PAIR_REQUESTED, PAIR_PAIRED,};

enum MessageType {PAIRING, DATA, SETTING, COMMAND};
enum CommandType {ONWIFI, ONMESH, RESET, LOADCONFIG};

unsigned long currentMillisMes = millis();
unsigned long previousMillisMes = 0;   // Stores last time temperature was published

unsigned int readingIdMes = 0;   
int channelMes = 1;


PairingStatus pairingStatusMes  = NOT_PAIRED;
void SetParameter(byte Status){
if(Status == PAIR_REQUEST){pairingStatusMes  = PAIR_REQUEST;}
if(Status == PAIR_PAIRED){pairingStatusMes  = PAIR_PAIRED;}
}
void getParameters(){
// DataTrans datTrans;
// dataTrans.currentMillis = currentMillisMes; 
// dataTrans.previousMillis = previousMillisMes; 
//  datTrans.currentMillisPing = currentMillisPing;
//  datTrans.previousMillisMesPing = previousMillisMesPing;
//  datTrans.rssi_display = rssi_display;
//  interval = interval;
//  dataTrans.sensors_saved = sensors_saved;
//  for(int i = 0; i < 6 ; i++){dataTrans.defserverAddress[i] = defserverAddress[i];}
}

void promiscuous_rx_cbs(void *buf, wifi_promiscuous_pkt_type_t type) {
  // All espnow traffic uses action frames which are a subtype of the mgmnt frames so filter out everything else.
  if (type != WIFI_PKT_MGMT)
    return;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  int rssi = ppkt->rx_ctrl.rssi;
  rssi_display = rssi;
}

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
#endif//

#ifdef ARDUINO_ARCH_ESP8266
// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
Serial.println(sendStatus == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
#else

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println((status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"));
}
#endif//#ifdef ARDUINO_ARCH_ESP8266



void addPeer(const uint8_t * mac_addr, uint8_t chan){
  esp_now_peer_info_t peer;
  ESP_ERROR_CHECK(esp_wifi_set_channel(chan ,WIFI_SECOND_CHAN_NONE));
  esp_now_del_peer(mac_addr);
  memset(&peer, 0, sizeof(esp_now_peer_info_t));
  peer.channel = chan;
  peer.encrypt = false;
  memcpy(peer.peer_addr, mac_addr, sizeof(uint8_t[6]));
  if (esp_now_add_peer(&peer) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(defserverAddress, mac_addr, sizeof(uint8_t[6]));
}

void printMAC(const uint8_t * mac_addr){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
}

byte resentMes = 0;bool onceOnsent = true;
void OnDataSents(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if(status == ESP_NOW_SEND_FAIL){
    interval = 1000;
    resentMes++;if(resentMes > 5){onceOnsent = true;
    resentMes = 0;Serial.println("fail 5 times");
    SetParameter(PAIR_REQUEST);
    }
  }else{resentMes = 0;if(onceOnsent){interval = 10000;onceOnsent = false;}}
}

byte repairMes = 0;

PairingStatus autoPairing(){
  switch(pairingStatusMes ) {
    case PAIR_REQUEST:
    Serial.print("Pairing request on channelMes "  );
    Serial.println(channelMes);

    // set WiFi channelMes   
    ESP_ERROR_CHECK(esp_wifi_set_channel(channelMes,  WIFI_SECOND_CHAN_NONE));
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
    }
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cbs);
    // set callback routines
    esp_now_register_send_cb(OnDataSents);
    // esp_now_register_recv_cb(MeshMain.OnDataRecvs());
    // MeshMain.OnData();
    // set pairing data to send to the server
    byte BOARD_ID;
    CONFIG::read_byte (EP_EEPROM_ID, &BOARD_ID);
    structpairing.msgType = PAIRING;
    structpairing.id = BOARD_ID;     
    structpairing.channel = channelMes;

    // add peer and send request
    addPeer(defserverAddress, channelMes);
    esp_now_send(defserverAddress, (uint8_t *) &structpairing, sizeof(structpairing));
    previousMillisMes = millis();
    pairingStatusMes  = PAIR_REQUESTED;
    break;

    case PAIR_REQUESTED:
    // time out to allow receiving response from server
    currentMillisMes = millis();
    if(currentMillisMes - previousMillisMes > 250) {
      previousMillisMes = currentMillisMes;
      // time out expired,  try next channel
      channelMes ++;digitalWrite(LED, !(digitalRead(LED)));
      if (channelMes > MAX_CHANNEL){
         channelMes = 1;
         repairMes++;if(repairMes > 10){ESP.restart();}
      }   
      pairingStatusMes  = PAIR_REQUEST;
    }
    break;

    case PAIR_PAIRED:
      // nothing to do here 
    break;
  }
  return pairingStatusMes ;
}  

#endif//MeshWifi