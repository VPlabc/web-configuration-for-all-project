#ifndef MeshNet_
#define MeshNet_
#include "config.h"

#ifdef Mesh_Network
#include "LookLine.h"
Command COMMAND;
#include "esp_wifi.h"
#include <WiFi.h>
#include <esp_now.h>
void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength);
void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen);
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status);
void broadcast(const String &message);
void MeshSetup(void);
void MeshLoop(void);

//char buffer[ESP_NOW_MAX_DATA_LEN + 1];

uint8_t current_protocol;
esp_now_peer_info_t peerInfo;
esp_interface_t current_esp_interface;
wifi_interface_t current_wifi_interface;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   // SENSOR MAC`
int rssi_display;

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
  char bufferMesh[250];
  bool Meshdone;
  bool Meshdebug = false;
void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
{
  snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void MeshRecive(const uint8_t *macAddr, const uint8_t *data, int dataLen)
{
  // only allow a maximum of 250 characters in the message + a null terminating byte
  int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
  strncpy(bufferMesh, (const char *)data, msgLen);
  // make sure we are null terminated
  bufferMesh[msgLen] = 0;
  // format the mac address
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  // debug log the message to the serial port
  //LOGf("Received message from: %s \n %s\n", macStr, buffer);
  // what are our instructions
    Meshdone = true;
    Lookline_PROG.SetDone();
    String ids = "";
      ids += bufferMesh[0];
      ids += bufferMesh[1];
      ids += bufferMesh[2];
      ids += bufferMesh[3];
      int NodeID = ids.toInt();
      LOG("Mesh revice | ID:");LOGLN(NodeID);
      Data_Proccess(bufferMesh);
}

// callback when data is sent
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status)
{
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  //LOG("Last Packet Sent to: ");
  //LOGLN(macStr);
  //LOG("Last Packet Send Status: ");
  //LOGLN(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}



int check_protocol()
{
    char error_buf1[100];
  if(Meshdebug){
    LOGLN();
    LOGLN("Mesh___________________________________");
    LOGLN();
  }
     esp_err_t error_code = esp_wifi_get_protocol(current_wifi_interface, &current_protocol);
     esp_err_to_name_r(error_code,error_buf1,100);
  if(Meshdebug){
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
void OnMeshDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
#ifdef IOTDEVICE_UI  
  if(IOT_DEVICE.Debug){
#endif//IOTDEVICE_UI
#ifdef LOOKLINE_UI  
  if(Lookline_PROG.GetDebug()){
#endif//LOOKLINE_UI
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  }
//   if (status ==0){
//     success = "Delivery Success :)";
//   }
//   else{
//     success = "Delivery Fail :(";
//   }
}

void MeshSetup()
{
String dataDisp = "";
 bool Meshdebug = Lookline_PROG.GetDebug();
 LOGLN("Mesh: " + String(Meshdebug));
  // Init ESP-NOW
  WiFi.mode(WIFI_STA);
  #ifdef ARDUINO_ARCH_ESP8266
  #else
  if(Meshdebug){
  if(check_protocol() != 8){
  esp_wifi_set_protocol(current_wifi_interface, WIFI_PROTOCOL_LR);
  check_protocol();
  }
  #endif//#ifdef ARDUINO_ARCH_ESP8266
  if (esp_now_init() != ESP_OK) {
    LOGLN("Error initializing ESP-NOW");
  }
  }
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnMeshDataSent);
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    if(Meshdebug)LOGLN("Failed to add peer");
  
    // RunMode = 1;
    return;
  }
  else{
    if(Meshdebug)LOGLN("add peer OK");
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(MeshRecive);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
}

void MeshLoop()
{
    
}
#endif//def MeshNet
#endif//MeshNet_