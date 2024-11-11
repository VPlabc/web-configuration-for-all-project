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
// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
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
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
#else

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
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
#endif//#ifdef ARDUINO_ARCH_ESP8266


// }

#endif//MeshWifi