#ifndef MeshNet_
#define MeshNet_
#include "config.h"

#ifdef Mesh_Network
#include "LookLine.h"
Command COMMAND;
#include <WiFi.h>
#include <esp_now.h>
void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength);
void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen);
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status);
void broadcast(const String &message);
void MeshSetup(void);
void MeshLoop(void);

//char buffer[ESP_NOW_MAX_DATA_LEN + 1];



  char buffer[250];

void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
{
  snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
{
  // only allow a maximum of 250 characters in the message + a null terminating byte
  int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
  strncpy(buffer, (const char *)data, msgLen);
  // make sure we are null terminated
  buffer[msgLen] = 0;
  // format the mac address
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  // debug log the message to the serial port
  //LOGf("Received message from: %s \n %s\n", macStr, buffer);
  // what are our instructions
    Lookline_PROG.done = true;
    String ids = "";
      ids += buffer[0];
      ids += buffer[1];
      ids += buffer[2];
      ids += buffer[3];
      Lookline_PROG.Id = ids.toInt();
      //LOG("ID:");LOGLN(Id);
      MeshLoop();
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

void broadcast(const String &message)
{
  // this will broadcast a message to everyone in range
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    esp_now_add_peer(&peerInfo);
  }
  esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t *)message.c_str(), message.length());
  // and this will send a message to a specific device
  /*uint8_t peerAddress[] = {0x3C, 0x71, 0xBF, 0x47, 0xA5, 0xC0};
  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, peerAddress, 6);
  if (!esp_now_is_peer_exist(peerAddress))
  {
    esp_now_add_peer(&peerInfo);
  }
  esp_err_t result = esp_now_send(peerAddress, (const uint8_t *)message.c_str(), message.length());*/
  if (result == ESP_OK)
  {
    //LOGLN("Broadcast message success");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_INIT)
  {
    LOGLN("Mesh not Init.");
  }
  else if (result == ESP_ERR_ESPNOW_ARG)
  {
    LOGLN("Invalid Argument");
  }
  else if (result == ESP_ERR_ESPNOW_INTERNAL)
  {
    LOGLN("Internal Error");
  }
  else if (result == ESP_ERR_ESPNOW_NO_MEM)
  {
    LOGLN("ESP_ERR_ESPNOW_NO_MEM");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
  {
    LOGLN("Peer not found.");
  }
  else
  {
    LOGLN("Unknown error");
  }
}

void MeshSetup()
{
  #ifdef Mesh_Network
  //Set device in STA mode to begin with
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.mode(WIFI_STA);
  LOGLN("Mesh Init");
  // Output my MAC address - useful for later
  LOG("My MAC Address is: ");
  LOGLN(WiFi.macAddress());
  // shut down wifi
  WiFi.disconnect();
  // startup ESP Now
  if (esp_now_init() == ESP_OK)
  {
    LOGLN("Mesh Init Success");
    esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
  }
  else
  {
    LOGLN("Mesh Init Failed");
    delay(3000);
    
  }
  #else
  {
    WiFi.mode(WIFI_OFF);
    btStop();
  }
  #endif//Mesh_Network
  //
  
}

void MeshLoop()
{
    Data_Proccess();
}
#endif//def MeshNet
#endif//MeshNet_