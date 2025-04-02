#include <Arduino.h>
#include "config.h"
#include "Modbus_RTU.h"
Modbus_Prog ModbuS;

#ifdef LOOKLINE_UI
#include "LookLine/LookLine.h"
LOOKLINE_PROG Lookline_modbus; 
#endif//LOOKLINE_UI

// #define ESP32_C3

#if defined(ESP32_C3)
#define Modbus_Serial Serial1
#elif defined(ESP32)
#define ModbusSerial Serial2
#else

#include <SoftwareSerial.h>
 SoftwareSerial serial_ESP1(5,4);
#define Modbus_Serial serial_ESP1
#endif//ESP32

// #define DEBUG
const long interval_ = 3000;
unsigned long previousMillis_ = 0;
bool Done = false;

word regs_Master[ModbuS.HOLDING_REGS_SIZE*2];

void debugs();
#define dataSize 30

const int LED_COIL = 0;
const int SWITCH_ISTS = 100;
const int ReQuen = 200;//Register Write
const int RegRead = 300;//Register Read

bool state0 = 0;
bool state1 = 1;
bool state2 = 0;
int count_mb = 0;
const long intervalupdate = 10000;
unsigned long previousMillisupdate = 0;
byte once = true;
const long interval_update = 3000;
unsigned long previousMillis_update = 0;
#define baudrate 9600//19200
// #define Enthernet
// #define WifiConnect
// #define Modbus_Serial
// #define Master
// #define SerialPort
#define TEST

#define  ledPin  2 // onboard led 
#ifdef Modbus_Serial

#define  buttonPin  0 // push button

unsigned int holdingRegs[ModbuS.HOLDING_REGS_SIZE*2]; // function 3 and 16 register array
////////////////////////////////////////////////////////////
#define baudrate 9600//19200
#define SLAVE_ID 1

// #ifdef Master
#include <SimpleModbusMaster.h>
// SimpleModbusMaster NodeMaster;

#define connection_error_led 2

//////////////////// Port information ///////////////////
#define timeout 3000
#define polling 200 // the scan rate

// If the packets internal retry register matches
// the set retry count then communication is stopped
// on that packet. To re-enable the packet you must
// set the "connection" variable to true.
#define retry_count 10 

// used to toggle the receive/transmit pin on the driver
#define TxEnablePin 2 
// enum
// {
//   PACKET1,
//   PACKET2,
//   // leave this last entry
//   TOTAL_NO_OF_PACKETS
// };

// Create an array of Packets for modbus_update()
Packet packets[TOTAL_NO_OF_PACKETS];

// Create a packetPointer to access each packet
// individually. This is not required you can access
// the array explicitly. E.g. packets[PACKET1].id = 2;
// This does become tedious though...
packetPointer packet1 = &packets[PACKET1];
packetPointer packet2 = &packets[PACKET2];

// #else//slave



// #endif//Master


#endif//Modbus_Serial

#ifdef WifiConnect
#include <Modbus.h>
#include <ModbusIP_ESP32.h>
//ModbusIP object
ModbusIP mb;
#endif//WifiConnect
#ifdef Enthernet
#include <Modbus.h>
#include <ModbusIP_ESP32.h>
#define DEBUG_ETHERNET_WEBSERVER_PORT       Serial

// Debug Level from 0 to 4
#define _ETHERNET_WEBSERVER_LOGLEVEL_       3

#include <WebServer_WT32_ETH01.h>


// Select the IP address according to your local network
IPAddress myIP(192, 168, 1, 232);
IPAddress myGW(192, 168, 1, 1);
IPAddress mySN(255, 255, 255, 0);

// Google DNS Server IP
IPAddress myDNS(8, 8, 8, 8);

//ModbusIP object
ModbusIP mb;
#endif//Enthernet
#if defined(Ethernet) || defined(WifiConnect) 
//Modbus Registers Offsets (0-9999)
const int LED_COIL = 100;
const int SWITCH_ISTS = 200;
const int ReQuen = 300;
//Used Pins
const int ledPin = 2; //GPIO0
#endif//Ethernet




void Modbus_Prog::modbus_write_update(int HOLDING_REGS_Data[]) 
{
  for(byte i=0;i < HOLDING_REGS_SIZE; i++){ModbuS.regs_WRITE[i] = HOLDING_REGS_Data[i];}
}
void Modbus_Prog::modbus_read_update(int HOLDING_REGS_Data[]) 
{
for(byte i=0;i < HOLDING_REGS_SIZE; i++){ModbuS.regs_READ[i] = HOLDING_REGS_Data[i];}
  // if(ModbuS.regs_WRITE[ONOFF] < 2){ModbuS.regs_READ[ONOFF] = HOLDING_REGS_Data[ONOFF];}
} 


void Modbus_Prog::modbus_write_setParameter(int pos,int Value, int cmd) {
  // if(pos == HOLDING_REGS_SIZE+1 && Value == 0 && cmd == 0){ModbuS.regs_WRITE[CMD] = 2;}
  // else{ ModbuS.regs_WRITE[pos] = Value;ModbuS.regs_WRITE[cmd] = cmd;debugs();}
  
} 


// #ifdef MASTER_MODBUS

enum{Slave, Master};


#define SLAVE_ID 1

#define RXD2 16
#define TXD2 17

/// @brief /// Register address
// bool role = Master;

uint8_t coils[100];
uint8_t discreteInputs[100];
uint16_t holdingRegisters[100];
uint16_t inputRegisters[100];


void debugs();



void Modbus_Prog::modbus_setup(bool role) { 

  Serial.begin(115200);
  
#ifdef SerialPort
Serial2.begin(baudrate, SERIAL_8N1, RXD2, TXD2,false);
  if(role == Master){
  Modbus_Master.setTimeoutTimeMs(100);
  Modbus_Master.begin(&Serial2);
  }
  if(role ==Slave){
    mb.begin(&Serial2);
    mb.slave(SLAVE_ID);
    for(int i = 0 ; i < dataSize ; i++){
      mb.addCoil(LED_COIL + i);
      mb.addCoil(SWITCH_ISTS + i);
      mb.addHreg(ReQuen, holdingRegisters[i]);
      mb.addHreg(RegRead,inputRegisters[i]);
    }
  }
#endif//SerialPort
#ifdef WifiConnect
  mb.config("Hoang Vuong", "91919191");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif//Wificonnect
#ifdef Enthernet
// To be called before ETH.begin()
  WT32_ETH01_onEvent();

  //bool begin(uint8_t phy_addr=ETH_PHY_ADDR, int power=ETH_PHY_POWER, int mdc=ETH_PHY_MDC, int mdio=ETH_PHY_MDIO,
  //           eth_phy_type_t type=ETH_PHY_TYPE, eth_clock_mode_t clk_mode=ETH_CLK_MODE);
  //ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);

  // Static IP, leave without this line to get IP via DHCP
  //bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = 0, IPAddress dns2 = 0);
  // ETH.config(myIP, myGW, mySN, myDNS);

  WT32_ETH01_waitForConnect();

  Serial.println("");
  Serial.println("enthernet connected");
  Serial.println(ETH.localIP());
#endif//Eth
// if(role == 0){
    // read 5 registers starting at address 0
//Modbus Setup

}





bool MB_Update_Once = true;






void Modbus_Prog::modbus_loop(bool role) {
  // #ifdef MASTER_MODBUS
    unsigned int connection_status = modbus_update(packets);

    for (int i = 0; i < HOLDING_REGS_SIZE; i++){ ModbuS.regs_READ[i] = regs_Data[i];}
    for (int i = HOLDING_REGS_SIZE; i < HOLDING_REGS_SIZE*2; i++){regs_Data[i] = ModbuS.regs_WRITE[i-(HOLDING_REGS_SIZE)];}
  #ifdef LOOKLINE_UI
  if(Lookline_modbus.Get_role() == NODE || Lookline_modbus.Get_role() == REPEARTER){  
    if(ModbuS.regs_READ[3] == 2){ModbuS.regs_WRITE[3] = 0;}
  }
  #endif// LOOKLINE_UI
  #ifdef DEBUG
  unsigned long currentMillis_ = millis();
  if (currentMillis_ - previousMillis_ >= interval_)
  {
    previousMillis_ = currentMillis_;
    Serial.println("DATA");
        
        for (int i = 0; i < HOLDING_REGS_SIZE*2; i++)
          {
            Serial.print("regs_D " + String(i) + ": ");
            Serial.println(regs_Data[i]);
          }
   }
    #endif//
     unsigned long currentMillis_update = millis();
      if (currentMillis_update - previousMillis_update >= interval_update)
      {
        previousMillis_update = currentMillis_update;   
        // modbusUpdateOnce = true;
        //////////////////////////////////////////////////////////////// 

        #ifdef MASTER_MODBUS
        for(int j = 0 ; j < HOLDING_REGS_SIZE ; j++){
          if(ModbuS.regs_last_READ[j] != ModbuS.regs_READ[j]){
            MB_Update_Once = true;
          }
        }
          if(MB_Update_Once){
            MB_Update_Once = false;
            #ifdef LOOKLINE_UI
            Lookline_modbus.lookline_modbus_update(ModbuS.regs_READ);
            #endif//#ifdef LOOKLINE_UI
           for(int j = 0 ; j < HOLDING_REGS_SIZE ; j++){
              ModbuS.regs_last_READ[j] = ModbuS.regs_READ[j];
            }
          }

            #ifdef LOOKLINE_UI
            Lookline_modbus.lookline_modbus_get(ModbuS.regs_WRITE);
        if(Lookline_modbus.Get_role() == GATEWAY){
            Lookline_modbus.lookline_modbus_get(ModbuS.regs_WRITE);
        }
      #endif// LOOKLINE_UI
      #endif//MASTER_MODBUS
      }
  #ifdef LED_STT
  if (connection_status != TOTAL_NO_OF_PACKETS){digitalWrite(connection_error_led, HIGH);  }
  else { digitalWrite(connection_error_led, LOW);}
  #endif//LED_STT
    // debugs();
  // #endif//MASTER
  #ifdef SerialPort
  // if (Modbus_Serial.available()) {
  //   int inByte = Modbus_Serial.read();
  //   Serial.write(inByte);
  // }

  // // read from port 0, send to port 1:
  // if (Serial.available()) {
  //   int inByte = Serial.read();
  //   Modbus_Serial.write(inByte);
  // }
  // discreteInputs[0] = digitalRead(buttonPins[0]);
  if(role == Master){
    Modbus_Master.readCoilsRegister(SLAVE_ID  , SWITCH_ISTS, dataSize ,coils,dataSize);//Read Coil
    Modbus_Master.readHoldingRegister(SLAVE_ID  , RegRead ,holdingRegisters ,dataSize);//Read holdingRegisters
    Modbus_Master.writeCoilsRegister(SLAVE_ID , LED_COIL , dataSize, discreteInputs, dataSize);//Write Coil 
    Modbus_Master.writeHoldingRegister(SLAVE_ID  , ReQuen , inputRegisters, dataSize);//Write REG
  }
  if(role == Slave){
    mb.task();
    yield();
    coils[0] = mb.Coil(LED_COIL);//Read Coil
    holdingRegisters[0] = mb.Hreg(ReQuen);
    mb.Coil(SWITCH_ISTS, discreteInputs[0]);//Write Coil
    mb.Hreg(RegRead,inputRegisters[0]);//Write REG
  }
   //Attach ledPin to LED_COIL register
  //  digitalWrite(ledPins, coils[0]);

  static unsigned long previousMillis = 0;
  const long interval = 1000;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    inputRegisters[0] = random(200,5000);
    if(role == Slave){Serial.println("slave");}
    if(role == Master){Serial.println("master");}
      Serial.print("holdingRegisters: ");
      // Your code here to print to serial every 2 seconds
      Serial.print(holdingRegisters[0]);
      Serial.print(" ");
      Serial.println(holdingRegisters[1]);
      // Inserted code
      Serial.print("coils: ");
      for (int i = 0; i < 2; i++) {
        Serial.print(coils[i]);
        if (i < 1) {
          Serial.print(" ");
        }
      }
      Serial.println();
      Serial.print("inputRegisters: ");
      Serial.print(inputRegisters[0]);
      Serial.print(" ");
      Serial.println(inputRegisters[1]);
      Serial.print("discreteInputs: ");
      Serial.print(discreteInputs[0]);
      Serial.print(" ");
      Serial.println(discreteInputs[1]);
      Serial.println("================================================");
  }
#endif//SerialPort
#if defined(Enthernet) || defined(WifiConnect) 
   //Call once inside loop() - all magic here
   mb.task();
  //  count_mb++;if(count_mb > 20000){count_mb = 0;
  //   // Serial.println(state);
  //   state2 = !state2;
  //   state1 = state;
  //  }
    state0 = mb.Coil(LED_COIL);
   
    mb.Ists(SWITCH_ISTS, state);
    mb.Hreg(ReQuen,random(200,5000));
   //Attach ledPin to LED_COIL register
   digitalWrite(ledPin, state);
#endif//Modbus_Serial
#ifdef Modbus_Serial
#ifdef Master
// packets[TOTAL_ERRORS] = modbus_update(packets);

  unsigned int connection_status = modbus_update(packets);
  if (connection_status != TOTAL_NO_OF_PACKETS)
  {digitalWrite(connection_error_led, HIGH);}
  else{digitalWrite(connection_error_led, LOW); }

  debugs();
#else//Slave

byte AddressRegs = HOLDING_REGS_SIZE;
holdingRegs[ModbuS.HOLDING_REGS_SIZE*2] = NodeSlave.modbus_update(holdingRegs);
    for (int i = 0; i < AddressRegs; i++){holdingRegs[i] = ModbuS.regs_READ[i];}
    for (int i = AddressRegs; i < HOLDING_REGS_SIZE*2; i++){ModbuS.regs_WRITE[i-AddressRegs] = holdingRegs[i];}
    
  #ifdef LOOKLINE_UI
        Lookline_modbus.slave_modbus_get(ModbuS.regs_WRITE) ;
        Lookline_modbus.slave_modbus_update(ModbuS.regs_READ);
  #endif//

    // (int BoardID,int NetID,bool RunStop,bool OnOff,int Plan,
    // int PlanSet, int Result,int ResultSet,int PlanLimit,
    // int Pcs,int TimeInc,int ComMode,int Type,int Role,
    // int OnWifi,int DelayCounter);
// #ifdef DEBUG
//   unsigned long currentMillis_1 = millis();
//   if (currentMillis_1 - previousMillis_1 >= interval_1)
//   {
//     previousMillis_1 = currentMillis_1;
//     Serial.println("DATA");
        
//     for (int i = 0; i < HOLDING_REGS_SIZE*2; i++)
//       {
//         Serial.print("regs_D " + String(i) + ": ");
//         Serial.println(holdingRegs[i]);
//       }
//    }
//     #endif//
        // Serial.println("DATA");
        // for (int i = 0; i < HOLDING_REGS_SIZE*2; i++)
        //   {
        //     Serial.print("regs_D " + String(i) + ": ");
        //     Serial.println(holdingRegs[i]);
        //   }
        
  // debugs();
#endif//Master
#endif//Modbus_Serial
}
// #else//SLAVE_MODBUS
       












/*
################################################################################################
            S L A V E  _  M O D B U S
################################################################################################
*/







// #endif//MASTER_MODBUS

int sent = 0;
unsigned int connection_status = 0;
const long interval = 5000;
unsigned long previousMillis = 0;
unsigned long TimeProcess;
bool MB_Once = true;
void Modbus_Prog::debugs()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;   
    // modbusUpdateOnce = true;
    //////////////////////////////////////////////////////////////// 

    /////////////////////////////////////////////////////////////////////////////////////
    #ifdef DEBUG
    #endif//DEBUG
    if(MB_Once){MB_Once = false;
      Serial.println("CONFIG INFO");
      Serial.println("Baud rate: " + String(baudrate));
      Serial.println("Time out: " + String(timeouts));
      Serial.println("Retry time: " + String(retry_count));
      Serial.println("Polling: " + String(polling));
      Serial.println("----------------------------------------------------------------");
    }

        // String Log = "Read Data\n";
        // Log += "ID: " + String(ModbuS.regs_READ[ModbuS.BOARDID]) + " |";
        // Log += "Network: " + String(ModbuS.regs_READ[ModbuS.NETID]) + " |";
        // Log += "Run/Stop: " + String(ModbuS.regs_READ[ModbuS.RUNSTOP]) + " |";
        // Log += "On/Off: " + String(ModbuS.regs_READ[ModbuS.ONOFF]) + " |";
        // Log += "Plan:" + String(ModbuS.regs_READ[ModbuS.PLAN]) + " |";
        // Log += "Plan set:" + String(ModbuS.regs_READ[ModbuS.PLANSET]) + " |";
        // Log += "Result:" + String(ModbuS.regs_READ[ModbuS.RESULT]) + " |";
        // Log += "Result set:" + String(ModbuS.regs_READ[ModbuS.RESULTSET]) + " |";
        // Log += "Plan Limit:" + String(ModbuS.regs_READ[ModbuS.MAXPLAN]) + " |";
        // Log += "Pcs/h:" + String(ModbuS.regs_READ[ModbuS.PCS]) + " |";
        // Log += "Time:" + String(ModbuS.regs_READ[ModbuS.TIMEINC]) + " |";
        // Log += "Type:" + String(ModbuS.regs_READ[ModbuS.TYPE]) + " |";
        // Log += "Role:" + String(ModbuS.regs_READ[ModbuS.ROLE]) + " |";
        // Log += "OnWifi:" + String(ModbuS.regs_READ[ModbuS.ONWIFI]) + " |";
        // Log += "Delay Count:" + String(ModbuS.regs_READ[ModbuS.DELAYCOUNTER]) + " |";
        // Log += "Com mode:" + String(ModbuS.regs_READ[ModbuS.COMMODE])  + "\n";
        // Log += "Write Data\n";
        // Log += "ID: " + String(ModbuS.regs_WRITE[ModbuS.BOARDID]) + " |";
        // Log += "Network: " + String(ModbuS.regs_WRITE[ModbuS.NETID]) + " |";
        // Log += "Run/Stop: " + String(ModbuS.regs_WRITE[ModbuS.RUNSTOP]) + " |";
        // Log += "On/Off: " + String(ModbuS.regs_WRITE[ModbuS.ONOFF]) + " |";
        // Log += "Plan:" + String(ModbuS.regs_WRITE[ModbuS.PLAN]) + " |";
        // Log += "Plan set:" + String(ModbuS.regs_WRITE[ModbuS.PLANSET]) + " |";
        // Log += "Result:" + String(ModbuS.regs_WRITE[ModbuS.RESULT]) + " |";
        // Log += "Result set:" + String(ModbuS.regs_WRITE[ModbuS.RESULTSET]) + " |";
        // Log += "Plan Limit:" + String(ModbuS.regs_WRITE[ModbuS.MAXPLAN]) + " |";
        // Log += "Pcs/h:" + String(ModbuS.regs_WRITE[ModbuS.PCS]) + " |";
        // Log += "Time:" + String(ModbuS.regs_WRITE[ModbuS.TIMEINC]) + " |";
        // Log += "Type:" + String(ModbuS.regs_WRITE[ModbuS.TYPE]) + " |";
        // Log += "Role:" + String(ModbuS.regs_WRITE[ModbuS.ROLE]) + " |";
        // Log += "OnWifi:" + String(ModbuS.regs_WRITE[ModbuS.ONWIFI]) + " |";
        // Log += "Delay Count:" + String(ModbuS.regs_WRITE[ModbuS.DELAYCOUNTER]) + " |";
        // Log += "Com mode:" + String(ModbuS.regs_WRITE[ModbuS.COMMODE]);
        // LOGLN(Log);
        
    // Serial.println("WRITE DATA");
    // for (int ei = 0; ei < 10; ei++)
    //   {
    //     Serial.print("regs_WRITE " + String(ei) + ": ");
    //     Serial.println(ModbuS.regs_WRITE[ei]);
    //   }
    // Serial.println("READ DATA");

    // for (int ei = 0; ei < 10; ei++)
    // {
    //   Serial.print("regs_READ " + String(ei) + ": ");
    //   Serial.println(ModbuS.regs_READ[ei]);
    // }
///*
    // Serial.println();
    // Serial.println();
    /////////////////////////////////////////////////////////////////////////////



    // // PackeTimeProcessREAD
    // float per_success = 0;
    // per_success = (float(ModbuS.regs_Status[4]) / float(ModbuS.regs_Status[3])) * 100;
    // Serial.println("******************************************************");
    // Serial.println("Packet1: READ  " + String(per_success) + "  %");
    // Serial.print("ModbuS.regs_Status[3] = packet1->requests;  ");
    // Serial.println(ModbuS.regs_Status[3]);

    // Serial.print("ModbuS.regs_Status[4] = packet1->successful_requests;  ");
    // Serial.println(ModbuS.regs_Status[4]);

    // Serial.print("ModbuS.regs_Status[5] = packet1->total_errors;  ");
    // Serial.println(ModbuS.regs_Status[5]);
    // Serial.println();

    // // Packet2 WRITE
    // per_success = (float(ModbuS.regs_Status[7]) / float(ModbuS.regs_Status[6])) * 100;
    // Serial.println("Packet1: WRITE " + String(per_success) + "  %");
    // Serial.print("ModbuS.regs_Status[7] = packet1->requests;  ");
    // Serial.println(ModbuS.regs_Status[6]);

    // Serial.print("ModbuS.regs_Status[8] = packet1->successful_requests;  ");
    // Serial.println(ModbuS.regs_Status[7]);

    // Serial.print("ModbuS.regs_Status[9] = packet1->total_errors;  ");
    // Serial.println(ModbuS.regs_Status[8]);
    // ///////////////////////////////////////
    // Serial.println("******************************************************");
    // //*/
    // Serial.print("ModbuS.regs_Status[9] = packet1->total_errors;  ");
    // Serial.println(ModbuS.regs_Status[8]);

    // Serial.println(sent);

  }
}

// void start_count_time_process()
// {
//   TimeProcess= millis();
// }

// void get_count_time_process()
// {
//   TimeProcess= millis() - T1;
//   if (TimeProcess!= 0)
//     Serial.println("Process time: " + String(TimeProcess));
//   TimeProcess= millis();
// }
///////////////////////////////////////////////////////////////

