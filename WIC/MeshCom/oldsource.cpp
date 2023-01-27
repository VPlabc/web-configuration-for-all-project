/*
// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t  fixedAddress[]      = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}; // locally managed macAddress

typedef struct struct_messages {
  int     mesh_id;
  uint8_t sensor_id[6];
  bool    status ;
} struct_messages;
uint8_t         MeshIncomingData[sizeof(struct struct_messages)];
size_t          MeshIreceived_msg_length;
// Define variables to store incoming readings
float incomingTemp;
float incomingHum;
int incomingID;

// Updates DHT readings every 10 seconds
const long interval = 10000; 
unsigned long previousMillis = 0;    // will store last time DHT was updated 

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    int ID;
    float temp;
    float hum;
} struct_message;

// Create a struct_message called DHTReadings to hold sensor readings
struct_message DHTReadings;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

byte     MeshCurrent_status;      // 0=offline , 1=online    ,2=OTA

byte Mesh_Com::getMode(){
    byte Mode = 0;
  if (MeshCurrent_status == 2) {    
    Mode = 1;
  }
  return Mode;
}

void sendCurrentStatus() {
  Serial.println(MeshCurrent_status);
}

void startOTA() {
WiFi.disconnect();
   WiFi.mode(WIFI_AP);
   WiFi.softAP("VPlab-Receiver", "vplab.vn");
  MeshCurrent_status = 2;
  sendCurrentStatus();
}
void sendReading(uint8_t *MeshIncomingData, uint8_t len) {
  esp_now_send(fixedAddress, (uint8_t *) &MeshIncomingData, len);
}

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

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingTemp = incomingReadings.temp;
  incomingHum = incomingReadings.hum;
  incomingID = incomingReadings.ID;
}

// void getReadings(){
//   // Read Temperature
//   temperature = dht.readTemperature();
//   // Read temperature as Fahrenheit (isFahrenheit = true)
//   //float t = dht.readTemperature(true);
//   if (isnan(temperature)){
//     Serial.println("Failed to read from DHT");
//     temperature = 0.0;
//   }
//   humidity = dht.readHumidity();
//   if (isnan(humidity)){
//     Serial.println("Failed to read from DHT");
//     humidity = 0.0;
//   }
// }

void printIncomingReadings(){
  // Display Readings in Serial Monitor
  Serial.println("INCOMING READINGS");
  Serial.print("ID: ");
  Serial.println(incomingID);
  Serial.print("Temperature: ");
  Serial.print(incomingTemp);
  Serial.println(" ºC");
  Serial.print("Humidity: ");
  Serial.print(incomingHum);
  Serial.println(" %");
}
 
void Mesh_Com::setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Init DHT sensor
  //dht.begin();
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
/* ############################ Loop ############################################# */
/*
bool once = false;
    String Sreciver = "";
void Mesh_Com::loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval && MeshCurrent_status < 2) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;

    //Get DHT readings
    //getReadings();

    //Set values to send
    DHTReadings.temp = 789;
    DHTReadings.hum = 987;
    DHTReadings.ID = 2;
    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &DHTReadings, sizeof(DHTReadings));

    // Print incoming readings
    printIncomingReadings();
  }
  if (Serial.available()) {
    MeshIreceived_msg_length = Serial.readBytesUntil('\n', MeshIncomingData, sizeof(MeshIncomingData));
    if (MeshIreceived_msg_length == sizeof(MeshIncomingData)) {
      sendReading(MeshIncomingData , sizeof(MeshIncomingData));
    } else {
      if (MeshIncomingData[0] == '0') {sendCurrentStatus();}
      if (MeshIncomingData[0] == '1') {ESP.restart();}
      if (MeshIncomingData[0] == '2') {startOTA();}
    }
  }
  if (MeshCurrent_status == 2 && once == false) { once = true;
    if (!wifi_config.Setup() ) {
        WiFi.mode (WIFI_AP);
        wifi_config.WiFi_on = true;
        ESPCOM::println (F ("Safe mode 1"), PRINTER_PIPE);
        //try again in AP mode
        if (!wifi_config.Setup (true) ) {
            ESPCOM::println (F ("Safe mode 2"), PRINTER_PIPE);
            wifi_config.Safe_Setup();
        }
    }
    delay (100);
    //setup servers
    if (!wifi_config.Enable_servers() ) {
        ESPCOM::println (F ("Error enabling servers"), PRINTER_PIPE);
    }
  }

}
//*/