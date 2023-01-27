#define ESP8266 
#include <Arduino.h>
#include "config.h"
#ifdef MESHCOM_UI
// #include "WIC.h"

// #include "wificonf.h"
// #include "espcom.h"
// #include "webinterface.h"
// #include "command.h"
#include "MeshCom.h"
#include<ESP8266WiFi.h>
#include<espnow.h>

#define MY_ROLE         ESP_NOW_ROLE_COMBO              // set the role of this device: CONTROLLER, SLAVE, COMBO
#define RECEIVER_ROLE   ESP_NOW_ROLE_COMBO              // set the role of the receiver
#define WIFI_CHANNEL    12
#define MeshID 4
//#define MY_NAME         "SENSOR NODE"
//uint8_t receiverAddress[] = {0xBC, 0xDD, 0xC2, 0xBA, 0xFA, 0xB1};     // PC MAC

#define MY_NAME         "Gateway NODE"
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   // SENSOR MAC
struct __attribute__((packed)) dataPacket {
  int ID;
  int sensor1;
  float sensor2;
};

typedef struct struct_messages {
  int     mesh_id;
  uint8_t sensor_id[6];
  bool    status ;
} struct_messages;
uint8_t         MeshIncomingData[sizeof(struct struct_messages)];
size_t          MeshIreceived_msg_length;

byte     MeshCurrent_status;      // 0=offline , 1=online    
byte dataReaded = 0;
bool Result = false;

byte Mesh_Com::getMode(){
    byte Mode = 0;
  if(dataReaded == 1){MeshCurrent_status = 2;Mode = 1;}//Wifi mode
  if(dataReaded == 0){MeshCurrent_status = 0;Mode = 0;}//Mesh mode
  return Mode;
}
void sendCurrentStatus() {
  if(dataReaded == 1)MeshCurrent_status = 2;//Wifi mode
  if(dataReaded == 0)MeshCurrent_status = 0;//Mesh mode
  Serial.println(MeshCurrent_status);
}

void transmissionComplete(uint8_t *receiver_mac, uint8_t transmissionStatus) {
  if(transmissionStatus == 0) {
    Serial.println("Data sent successfully");
  } else {
    Serial.print("Error code: ");
    Serial.println(transmissionStatus);
  }
}

void dataReceived(uint8_t *senderMac, uint8_t *data, uint8_t dataLength) {
  char macStr[18];
  dataPacket packet;  

  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", senderMac[0], senderMac[1], senderMac[2], senderMac[3], senderMac[4], senderMac[5]);

  Serial.println();
  Serial.print("ID:" + String(MeshID) + " | ");
  Serial.print("Received data from: ");
  Serial.println(macStr);
  
  memcpy(&packet, data, sizeof(packet));
  
  Serial.print("ID: ");
  Serial.println(packet.ID);
  Serial.print("sensor1: ");
  Serial.println(packet.sensor1);
  Serial.print("sensor2: ");
  Serial.println(packet.sensor2);
}
 
void Mesh_Com::setup() {
  Serial.begin(115200);     // initialize serial port

  Serial.println();
  Serial.println();
  Serial.println();
  Serial.print(MY_NAME);
  Serial.println("...initializing...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();        // we do not want to connect to a WiFi network

  if(esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }
  else
  {
    Serial.println("ESPNow ESP8266 Init Success");
    esp_now_set_self_role(MY_ROLE);   
    esp_now_register_send_cb(transmissionComplete);         // this function will get called once all data is sent
    esp_now_register_recv_cb(dataReceived);               // this function will get called whenever we receive data
    esp_now_add_peer(broadcastAddress, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);
  }
  Serial.println("Initialized.");
}

void startOTA() {
  Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, 1);
  Result = CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &dataReaded);
  Serial.println("RF Mode: Wifi " );
  Serial.println(dataReaded);
  sendCurrentStatus();
  delay(3000);ESP.restart();
}

void startMesh() {
  Result = CONFIG::write_byte (EP_EEPROM_WIFI_MODE, 0);
  Result = CONFIG::read_byte (EP_EEPROM_WIFI_MODE, &dataReaded);
  Serial.println("RF Mode: Mesh" );
  delay(3000);ESP.restart();
}
void SendTest()
{
  dataPacket packet;
  packet.sensor1 = 789;
  packet.sensor2 = 4044;
  packet.ID = MeshID;
  int result = esp_now_send(broadcastAddress, (uint8_t *) &packet, sizeof(packet));   
  if (result == 0)
  {
    Serial.println("Broadcast message success");
  }
  else
  {
    Serial.println("Unknown error");
  } 
  
  //sendReading((uint8_t *) &packet, sizeof(packet));
}
/* ############################ Loop ############################################# */

bool once = false;
void Mesh_Com::loop()
{
  //Serial.println("ping");
  if (Serial.available()) {
    MeshIreceived_msg_length = Serial.readBytesUntil('\n', MeshIncomingData, sizeof(MeshIncomingData));
    if (MeshIreceived_msg_length == sizeof(MeshIncomingData)) {
      //sendReading(MeshIncomingData , sizeof(MeshIncomingData));
    } else {
      if (MeshIncomingData[0] == '0') {MeshCurrent_status = 0;sendCurrentStatus();}
      if (MeshIncomingData[0] == '1') {ESP.restart();}
      if (MeshIncomingData[0] == '2') {startOTA();}
      if (MeshIncomingData[0] == '3') {startMesh();}
      if (MeshIncomingData[0] == '4') {SendTest();}
    }
  }
  // delay(1000);
}




#endif//MESHCOM_UI