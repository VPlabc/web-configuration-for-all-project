#include <Arduino.h>
#include "config.h"

#include "Modbus_RTU_old.h"
Modbus_Prog ModbuS;

#ifdef LOOKLINE_UI
#include "LookLine/LookLine.h"
LOOKLINE_PROG Lookline_modbus; 
#endif//LOOKLINE_UI

// #define ESP32_C3

#if defined(ESP32_C3)
#define ModbusSerial Serial1
#elif defined(ESP32)
#define ModbusSerial Serial2
#else

#include <SoftwareSerial.h>
 SoftwareSerial serial_ESP1(5,4);
#define ModbusSerial serial_ESP1
#endif//ESP32

// #define DEBUG
const long interval_ = 3000;
unsigned long previousMillis_ = 0;
bool Done = false;

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
  if(pos == HOLDING_REGS_SIZE+1 && Value == 0 && cmd == 0){ModbuS.regs_WRITE[CMD] = 2;}
  else{ ModbuS.regs_WRITE[pos] = Value;ModbuS.regs_WRITE[cmd] = cmd;debugs();}
  
} 
void Modbus_Prog::Clear_Lookline_Value() {
  int zero = 0;
  #ifdef SLAVE_MODBUS
  #ifdef LOOKLINE_UI
  CONFIG::write_buffer (EP_EEPROM_PLAN, (byte*) &zero, INTEGER_LENGTH);
  CONFIG::write_buffer (EP_EEPROM_RESULT, (byte*) &zero, INTEGER_LENGTH);
  // Lookline_modbus.UpdateLookLineDataFormMaster(16);
  #endif//LOOKLINE_UI
  #endif//SLAVE_MODBUS
  Done = true;
} 

void Modbus_Prog::GetData(int regs_READ[]){
  for(int i = 0; i < HOLDING_REGS_SIZE; i++){
      regs_READ[i] = regs_Data[i];
  }
}
#ifdef MASTER_MODBUS

#ifdef MASTER_MODBUS
#define LED_STT

#else
  #include <SimpleModbusSlave.h>
  SimpleModbusSlave NodeSlave;
 #endif//MASTER

#define baudrate 9600//19200
#ifdef ESP32
#define connection_error_led 2
#else//ESP826
#define connection_error_led D4
#endif//ESP32

bool ModbusTEST = false;
int ID = 1;
// uint8_t id = 1;
uint16_t ADDRESS = 0x1000;
//uint32_t feedbackTimeout = 100000;//15s
uint32_t feedbackTimeout = 5;//5s

void debugs();



void Modbus_Prog::modbus_setup(bool role) { 
#ifdef SLAVE_MODBUS
  /* Valid modbus byte formats are:
     SERIAL_8N2: 1 start bit, 8 data bits, 2 stop bits
     SERIAL_8E1: 1 start bit, 8 data bits, 1 Even parity bit, 1 stop bit
     SERIAL_8O1: 1 start bit, 8 data bits, 1 Odd parity bit, 1 stop bit
  */
  NodeSlave.modbus_configure(baudrate, ID, -1, HOLDING_REGS_SIZE*2, 0);
  ModbusSerial.begin(baudrate);
  // modbus_update_comms(baud, byteFormat, id) is not needed but allows for easy update of the
  // port variables and slave id dynamically in any function.
  // NodeSlave.modbus_update_slave_comms(baudrate, ID); 
#endif//SLAVES_MODBUS
#ifdef MASTER_MODBUS

    // read 5 registers starting at address 0
  packet1->id = ID;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = 0;
  packet1->no_of_registers = HOLDING_REGS_SIZE;
  packet1->register_array = regs_Data;
  // write the 5 registers to the PLC starting at address 5
  packet2->id = ID;
  packet2->function = PRESET_MULTIPLE_REGISTERS;
  packet2->address = 0;
  packet2->no_of_registers = HOLDING_REGS_SIZE*2;
  packet2->register_array = regs_Data;
  
  ModbusSerial.begin(baudrate);
  modbus_configure(baudrate, timeouts, polling, retry_count, TxEnablePin, packets, TOTAL_NO_OF_PACKETS);


#endif//MASTER_MODBUS

  pinMode(connection_error_led, OUTPUT);
}


bool MB_Update_Once = true;
const long interval_update = 3000;
unsigned long previousMillis_update = 0;
void Modbus_Prog::modbus_loop(bool role) {
  #ifdef MASTER_MODBUS
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
  #endif//MASTER
  
}
#else//SLAVE_MODBUS
       












/*
################################################################################################
            S L A V E  _  M O D B U S
################################################################################################
*/
#include <Arduino.h>
// #define Enthernet
// #define WifiConnect
// #define ModbusSerial
// #define Master
// #define SerialPort
#define TEST

#define  ledPin  2 // onboard led 
#ifdef ModbusSerial

#define  buttonPin  0 // push button

unsigned int holdingRegs[ModbuS.HOLDING_REGS_SIZE*2]; // function 3 and 16 register array
////////////////////////////////////////////////////////////
#define baudrate 9600//19200
#define SLAVE_ID 1

#ifdef Master
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
enum
{
  PACKET1,
  PACKET2,
  // leave this last entry
  TOTAL_NO_OF_PACKETS
};

// Create an array of Packets for modbus_update()
Packet packets[TOTAL_NO_OF_PACKETS];

// Create a packetPointer to access each packet
// individually. This is not required you can access
// the array explicitly. E.g. packets[PACKET1].id = 2;
// This does become tedious though...
packetPointer packet1 = &packets[PACKET1];
packetPointer packet2 = &packets[PACKET2];

#else//slave
#include <SimpleModbusSlave.h>
SimpleModbusSlave NodeSlave;

#endif//Master


#endif//ModbusSerial

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
word regs_Master[ModbuS.HOLDING_REGS_SIZE*2];

void debugs();

void Modbus_Prog::modbus_setup(bool role) { 

  Serial.begin(115200);
  
#ifdef SerialPort
 ModbusSerial.begin(9600);
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
#ifdef ModbusSerial
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
#ifdef Master

  #ifdef TEST
  regs_Master[10] = 6;
  regs_Master[11] = 7;
  regs_Master[12] = 8;
  regs_Master[13] = 9;
  regs_Master[14] = 10;
  regs_Master[15] = 11;
  regs_Master[16] = 12;
  regs_Master[17] = 13;
  regs_Master[18] = 14;
  regs_Master[19] = 65535
;
  #endif//TEST

    // read 3 registers starting at address 0
  packet1->id = SLAVE_ID;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = 0;
  packet1->no_of_registers = 10 ;
  packet1->register_array = regs_Master;
  
  // write the 9 registers to the PLC starting at address 3
  packet2->id = SLAVE_ID;
  packet2->function = PRESET_MULTIPLE_REGISTERS;
  packet2->address = 0;
  packet2->no_of_registers = 20;
  packet2->register_array = regs_Master;
  
  // P.S. the register_array entries above can be different arrays
  
  // Initialize communication settings etc...
  modbus_configure(baudrate, timeout, polling, retry_count, TxEnablePin, packets, TOTAL_NO_OF_PACKETS);
  ModbusSerial.begin(baudrate);
  pinMode(connection_error_led, OUTPUT);
#else//Slave
  NodeSlave.modbus_configure(baudrate, SLAVE_ID, -1, HOLDING_REGS_SIZE*2, 0);
  ModbusSerial.begin(baudrate);
  #ifdef TEST
  // regs_Slave[0] = 1;
  // regs_Slave[1] = 2;
  // regs_Slave[2] = 3;
  // regs_Slave[3] = 4;
  // regs_Slave[4] = 5;
  // regs_Slave[5] = 6;
  // regs_Slave[6] = 7;
  // regs_Slave[7] = 8;
  // regs_Slave[8] = 9;
  // regs_Slave[9] = 10;
  #endif//TEST

#endif//Master

#endif//ModbusSerial

  pinMode(ledPin, OUTPUT);

    digitalWrite(ledPin, HIGH);delay(500);
  digitalWrite(ledPin, LOW);delay(500);
    digitalWrite(ledPin, HIGH);delay(500);
  digitalWrite(ledPin, LOW);delay(500);

#if defined(Enthernet) || defined(WifiConnect)
  mb.addCoil(LED_COIL);
  mb.addIsts(SWITCH_ISTS);
  mb.addHreg(ReQuen, 0x1234);
#endif//ModbusSerial
  for(int j = 0 ; j < HOLDING_REGS_SIZE ; j++){
    ModbuS.regs_last_WRITE[j] = ModbuS.regs_WRITE[j];
  }
}//modbus_setup


bool state0 = 0;
bool state1 = 1;
bool state2 = 0;
int count_mb = 0;
const long intervalupdate = 10000;
unsigned long previousMillisupdate = 0;
byte once = true;
const long interval_update = 3000;
unsigned long previousMillis_update = 0;


void Modbus_Prog::modbus_loop(bool role) {
#ifdef SerialPort
  if (ModbusSerial.available()) {
    int inByte = ModbusSerial.read();
    Serial.write(inByte);
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    ModbusSerial.write(inByte);
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
    state = mb.Coil(LED_COIL);
   
    mb.Ists(SWITCH_ISTS, state);
    mb.Hreg(ReQuen,random(200,5000));
   //Attach ledPin to LED_COIL register
   digitalWrite(ledPin, state);
#endif//ModbusSerial
#ifdef ModbusSerial
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
         unsigned long currentMillis_update = millis();
      if (currentMillis_update - previousMillis_update >= interval_update)
      {
        previousMillis_update = currentMillis_update;   
        // LOG("Parameter Read: ");for(byte i=0;i < HOLDING_REGS_SIZE; i++){LOG("|");LOG(String(ModbuS.regs_READ[i]));}LOGLN();
        // LOG("Parameter Write: ");for(byte i=0;i < HOLDING_REGS_SIZE; i++){LOG("|");LOG(String(ModbuS.regs_WRITE[i]));}LOGLN();

      }
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
#endif//ModbusSerial
}//modbus_loop




#endif//MASTER_MODBUS

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


        String Log = "Write Data\n";
        Log += "ID: " + String(ModbuS.regs_WRITE[ModbuS.BOARDID]) + " |";
        Log += "Network: " + String(ModbuS.regs_WRITE[ModbuS.NETID]) + " |";
        Log += "Run/Stop: " + String(ModbuS.regs_WRITE[ModbuS.RUNSTOP]) + " |";
        Log += "On/Off: " + String(ModbuS.regs_WRITE[ModbuS.ONOFF]) + " |";
        Log += "Plan:" + String(ModbuS.regs_WRITE[ModbuS.PLAN]) + " |";
        Log += "Plan set:" + String(ModbuS.regs_WRITE[ModbuS.PLANSET]) + " |";
        Log += "Result:" + String(ModbuS.regs_WRITE[ModbuS.RESULT]) + " |";
        Log += "Result set:" + String(ModbuS.regs_WRITE[ModbuS.RESULTSET]) + " |";
        Log += "Plan Limit:" + String(ModbuS.regs_WRITE[ModbuS.MAXPLAN]) + " |";
        Log += "Pcs/h:" + String(ModbuS.regs_WRITE[ModbuS.PCS]) + " |";
        Log += "Time:" + String(ModbuS.regs_WRITE[ModbuS.TIMEINC]) + " |";
        Log += "Type:" + String(ModbuS.regs_WRITE[ModbuS.TYPE]) + " |";
        Log += "Role:" + String(ModbuS.regs_WRITE[ModbuS.ROLE]) + " |";
        Log += "OnWifi:" + String(ModbuS.regs_WRITE[ModbuS.ONWIFI]) + " |";
        Log += "Delay Count:" + String(ModbuS.regs_WRITE[ModbuS.DELAYCOUNTER]) + " |";
        Log += "Com mode:" + String(ModbuS.regs_WRITE[ModbuS.COMMODE]);
        LOGLN(Log);
        
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
    Serial.println();
    Serial.println();
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

