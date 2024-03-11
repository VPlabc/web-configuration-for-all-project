#include <Arduino.h>
#include "config.h"
#include "Modbus_RTU.h"
Modbus_Prog ModbuS;

#ifdef PLC_MASTER_UI
#include "PLC_IoT/PLC_Master.h"
PLC_MASTER pm;
#endif//PLC_MASTER_UI
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
#define dataSize 32

const int16_t offsetM0 = 2048;//
const int16_t offsetD0 = 4096;//

const int16_t Write_Coil = offsetM0+0;
const int16_t Read_Coil = offsetM0+100;
const int16_t RegWrite = offsetD0+2500;//Register Write
const int16_t RegRead = offsetD0+2000;//Register Read

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
#define SerialPort
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


#ifdef Ethernet_W5500
#include <SPI.h>
#include <Ethernet.h> // lib_deps = arduino-libraries/Ethernet@^2.0.0
#include <ModbusIP_ESP32.h>

ModbusIP Mb;
#define     ETH_RST        -1

// MgsModbus Mb;
int inByte = 0; // incoming serial byte

// Ethernet settings (depending on MAC and Local network)
byte mac[] = {0x90, 0xA2, 0xDA, 0x0E, 0x94, 0xB5 };

void ethernetReset(const uint8_t resetPin)
{
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, HIGH);
    delay(250);
    digitalWrite(resetPin, LOW);
    delay(50);
    digitalWrite(resetPin, HIGH);
    delay(350);
}
#endif//Ethernet_W5500
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

#include <Modbus.h>
#include <ModbusIP_ESP32.h>
//Modbus Registers Offsets (0-9999)
// const int Write_Coil = 100;
// const int Read_Coil = 200;
// const int RegWrite = 300;
// //Used Pins
// const int ledPin = 2; //GPIO0
#endif//Ethernet




// void Modbus_Prog::modbus_write_update(int16_t HOLDING_REGS_Data[]) 
// {
//   for(byte i=0;i < HOLDING_REGS_SIZE; i++){ModbuS.regs_WRITE[i] = HOLDING_REGS_Data[i];}
// }
// void Modbus_Prog::modbus_read_update(int16_t HOLDING_REGS_Data[]) 
// {
// for(byte i=0;i < HOLDING_REGS_SIZE; i++){ModbuS.regs_READ[i] = HOLDING_REGS_Data[i];}
//   // if(ModbuS.regs_WRITE[ONOFF] < 2){ModbuS.regs_READ[ONOFF] = HOLDING_REGS_Data[ONOFF];}
// } 


void Modbus_Prog::modbus_write_setParameter(int pos,int Value, int cmd) {
  // if(pos == HOLDING_REGS_SIZE+1 && Value == 0 && cmd == 0){ModbuS.regs_WRITE[CMD] = 2;}
  // else{ ModbuS.regs_WRITE[pos] = Value;ModbuS.regs_WRITE[cmd] = cmd;debugs();}
  
} 


// #ifdef MASTER_MODBUS

enum{Slave, Master};


#define SLAVE_ID 1



/// @brief /// Register address
bool ModbusRole = Master;


void debugs();



void Modbus_Prog::modbus_setup(bool ModbusRole) { 
#ifdef PLC_MASSTER_UI
  #ifndef MCP_USE
  pinMode(BTN_SET,    INPUT_PULLUP);
  #endif//MCP_USE
  pinMode(BootButton, INPUT_PULLUP);
  // Serial.begin(115200);
 #endif//PLC_MASTER_PROG 
#ifdef SerialPort
  LOGLN("_________________________________________ MODBUS RTU ________________________________________");
#ifdef PLC_MASTER_UI
Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2,false);
#else
Serial2.begin(9600, SERIAL_8N1, 16, 17,false);
 #endif//PLC_MASTER_PROG 
initupdate();
  if(ModbusRole == Master){
  Modbus_Master.setTimeoutTimeMs(100);
  Modbus_Master.begin(&Serial2);
  }
  if(ModbusRole ==Slave){
    mb.begin(&Serial2);
    mb.slave(SLAVE_ID);
    mb.addHreg(RegWrite, 0, dataSize);
    mb.addHreg(RegRead, 0, dataSize);
#ifdef Ethernet_W5500
    Mb.addHreg(RegWrite, 0, dataSize);
    Mb.addHreg(RegRead, 0, dataSize);
#endif//Ethernet_W5500
    // mb.addCoil(Read_Coil ,0, 30);
    // mb.addCoil(Write_Coil,0, 30);
  }
  // LOGLN("________________________________________________________________________________________");
#endif//SerialPort
#ifdef WifiConnect
  // mb.config("Hoang Vuong", "91919191");

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   LOG(".");
  // }
  LOGLN("");
  LOGLN("WiFi connected");
  LOGLN("Modbus IP address: ");
  LOGLN(WiFi.localIP());
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

  LOGLN("");
  LOGLN("enthernet connected");
  LOGLN(ETH.localIP());
#endif//Eth
#ifdef Ethernet_W5500
 // serial setup
  // Serial.begin(9600);
  LOGLN("_________________________________________ ETHERNET ________________________________________");
    SPI.begin(SCLK, MISO, MOSI,-1);
    ethernetReset(ETH_RST);
    Ethernet.init(SCS);
    byte ip_buf[4];
    //LOG ("Static mode\r\n")
    //get the IP
    LOG ("IP value:")
    if (!CONFIG::read_buffer (EP_MODBUS_IP_VALUE, ip_buf, IP_LENGTH) ) {
        LOG ("Error\r\n");
    }
    IPAddress local_ip (ip_buf[0], ip_buf[1], ip_buf[2], ip_buf[3]);
    LOG (local_ip.toString() )
    LOG ("\r\nGW value:")
    // get the gateway
    if (!CONFIG::read_buffer (EP_MODBUS_GATEWAY_VALUE, ip_buf, IP_LENGTH) ) {
        LOG ("Error\r\n");
    }
    IPAddress gateway (ip_buf[0], ip_buf[1], ip_buf[2], ip_buf[3]);
    LOG (gateway.toString() )
    LOG ("\r\nMask value:")
    //get the mask
    if (!CONFIG::read_buffer (EP_MODBUS_MASK_VALUE, ip_buf, IP_LENGTH) ) {
        LOG ("Error Mask value\r\n");
    }
    IPAddress subnet (ip_buf[0], ip_buf[1], ip_buf[2], ip_buf[3]);
    LOG (subnet.toString() )
    LOG ("\r\n")
    //apply according active wifi mode
    LOG ("Set IP\r\n")

  // initialize the ethernet device
  Ethernet.begin(mac, local_ip, gateway, subnet);   // start etehrnet interface
  LOGLN("Ethernet interface started"); 

  // print your local IP address:
  LOG("My IP address: ");
  LOGLN(Ethernet.localIP());
  LOGLN();
  LOGLN("_________________________________________________________________________________________");


#endif//ETHER_W5500
// if(ModbusRole == 0){
    // read 5 registers starting at address 0
//Modbus Setup

}



bool MB_Update_Once = true;
bool MB_Update_Once1 = true;
bool Reg_Update_Once = true;
bool Reg_Update_Once1 = true;

 uint16_t regs_WRITE[30];
 uint16_t regs_READ[30];

// Function to return an array of 30 elements
uint16_t* Modbus_Prog::getInputRegs() {return inputRegisters;}
uint16_t* Modbus_Prog::getOutputRegs() {return holdingRegisters;}

bool MB_Update_Data = false;
uint16_t MB_Update_Address = 0;
uint16_t MB_Update_Value = 0;

void Modbus_Prog::update(){ Reg_Update_Once1 = true;}
void Modbus_Prog::initupdate(){Reg_Update_Once = true;}

void Modbus_Prog::modbusSet(uint16_t addr, uint16_t value){regs_WRITE[addr] = value;inputRegisters[addr] = value;MB_Update_Value = value;MB_Update_Address = addr;MB_Update_Data = 1;update();
LOGLN("Modbus addr:"+String(addr)+" value:"+String(value));}

void Modbus_Prog::connectModbus(bool update){ MB_connect = update;}
void Modbus_Prog::setModbusupdateState(bool state){ MB_Update_Data = state;}

bool Modbus_Prog::getModbusupdateState(){ return MB_Update_Data;}
uint16_t Modbus_Prog::getModbusupdateData(){return MB_Update_Value;}
uint16_t Modbus_Prog::getModbusupdateAddr(){ return MB_Update_Address;}

void Modbus_Prog::Write_PLC(uint16_t addrPLC, uint16_t valuePLC){
  LOGLN("modbus Address: " + String(addrPLC) + " | modbus value: " + String(valuePLC) 
  //+ " |PLC addres Read:" + String(RegRead) + " |PLC addres Write:" + String(RegWrite)
  );
  Modbus_Master.writeHoldingRegister(SLAVE_ID  , RegRead +addrPLC, valuePLC);

}

void Modbus_Prog::modbus_loop(bool ModbusRole) {
  // #ifdef MASTER_MODBUS
  if(ModbusRole == Master && MB_connect == true){
    Modbus_Master.readCoilsRegister(SLAVE_ID  , Read_Coil, dataSize ,coils,dataSize);//Read Coil
    delay(5);
    Modbus_Master.readHoldingRegister(SLAVE_ID  , RegRead ,holdingRegisters ,dataSize);//Read holdingRegisters
    delay(5);
  #ifdef Ethernet_W5500
      Mb.task();
      for(int i = 0 ; i < dataSize ; i++){
        // coils[i] = mb.Coil(Write_Coil+i);//Read Coil
        holdingRegisters[i] = Mb.Hreg(RegWrite+i);
        // mb.Coil(Read_Coil +i, discreteInputs[i]);//Write Coil
        Mb.Hreg(RegRead+i,inputRegisters[i]);//Write REG
        // LOG("inputRegisters: ");for (int i = 0; i < dataSize; i++){LOG(inputRegisters[i]);LOG(" ");}LOGLN("");
      }
  #endif//Ethernet_W5500
  }//ktgamesRPB
      if (millis() - previousMillis_update >= interval_update)
      {
        previousMillis_update = millis();   
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
        if(Lookline_modbus.Get_ModbusRole() == GATEWAY){
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
  if(ModbusRole == Master && MB_connect == true){
    Modbus_Master.writeCoilsRegister(SLAVE_ID , Write_Coil , dataSize, discreteInputs, dataSize);//Write Coil 
    for (int i = 0; i < dataSize; i++){inputRegisters[i] = regs_WRITE[i];}
  // inputRegisters = ;
    Modbus_Master.writeHoldingRegister(SLAVE_ID  , RegWrite , inputRegisters, dataSize);//Write REG
      // LOG("inputRegisters: ");for (int i = 0; i < dataSize; i++){LOG(inputRegisters[i]);LOGLN(" ");}
    #if defined(Enthernet) || defined(WifiConnect) 
      //Call once inside loop() - all magic here
      IPmb.task();
      for(int i = 0 ; i < dataSize ; i++){
        // coils[i] = mb.Coil(Write_Coil+i);//Read Coil
        holdingRegisters[i] = IPmb.Hreg(RegWrite+i);
        // mb.Coil(Read_Coil +i, discreteInputs[i]);//Write Coil
        IPmb.Hreg(RegRead+i,inputRegisters[i]);//Write REG
        // LOG("inputRegisters: ");for (int i = 0; i < dataSize; i++){LOG(inputRegisters[i]);LOG(" ");}LOGLN("");
      }
    #endif// defined(Enthernet) || defined(WifiConnect) 
  }
  if(ModbusRole == Slave ){

      mb.task();
      
    if(MB_Update_Once1){MB_Update_Once1 = false;LOGLN("Update Modbus");}
    for(int i = 0 ; i < dataSize ; i++){
      // coils[i] = mb.Coil(Write_Coil+i);//Read Coil
      holdingRegisters[i] = mb.Hreg(RegWrite+i);
      // mb.Coil(Read_Coil +i, discreteInputs[i]);//Write Coil
      mb.Hreg(RegRead+i,inputRegisters[i]);//Write REG
      // LOG("inputRegisters: ");for (int i = 0; i < dataSize; i++){LOG(inputRegisters[i]);LOG(" ");}LOGLN("");
    }
  }else{MB_Update_Once1 = true;}
  #if defined(Enthernet) || defined(WifiConnect) 
   //Call once inside loop() - all magic here
   IPmb.task();
  for(int i = 0 ; i < dataSize ; i++){
    // coils[i] = mb.Coil(Write_Coil+i);//Read Coil
    holdingRegisters[i] = IPmb.Hreg(RegWrite+i);
    // mb.Coil(Read_Coil +i, discreteInputs[i]);//Write Coil
    IPmb.Hreg(RegRead+i,inputRegisters[i]);//Write REG
    // LOG("inputRegisters: ");for (int i = 0; i < dataSize; i++){LOG(inputRegisters[i]);LOG(" ");}LOGLN("");
  }
#endif// defined(Enthernet) || defined(WifiConnect) 
   //Attach ledPin to Write_Coil register
  //  digitalWrite(ledPins, coils[0]);

  static unsigned long previousMillis = 0;
  const long interval = 1000;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // for (int i = 0; i < dataSize; i++){inputRegisters[i] = random(0,100);}
    // if(ModbusRole == Slave){LOGLN("slave");}
    // if(ModbusRole == Master){LOGLN("master");}
    //   LOG("holdingRegisters: ");
    //   // Your code here to print to serial every 2 seconds
    //   LOG(holdingRegisters[0]);
    //   LOG(" ");
    //   LOGLN(holdingRegisters[1]);
    //   // Inserted code
    //   LOG("coils: ");
    //   for (int i = 0; i < 2; i++) {
    //     LOG(coils[i]);
    //     if (i < 1) {
    //       LOG(" ");
    //     }
    //   }
    //   LOGLN();
    //   LOG("inputRegisters: ");
    //   LOG(inputRegisters[0]);
    //   LOG(" ");
    //   LOGLN(inputRegisters[1]);
    //   LOG("discreteInputs: ");
    //   LOG(discreteInputs[0]);
    //   LOG(" ");
    //   LOGLN(discreteInputs[1]);
    //   LOGLN("================================================");
  }
#endif//SerialPort

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
    // int Pcs,int TimeInc,int ComMode,int Type,int ModbusRole,
    // int OnWifi,int DelayCounter);
// #ifdef DEBUG
//   unsigned long currentMillis_1 = millis();
//   if (currentMillis_1 - previousMillis_1 >= interval_1)
//   {
//     previousMillis_1 = currentMillis_1;
//     LOGLN("DATA");
        
//     for (int i = 0; i < HOLDING_REGS_SIZE*2; i++)
//       {
//         LOG("regs_D " + String(i) + ": ");
//         LOGLN(holdingRegs[i]);
//       }
//    }
//     #endif//
        // LOGLN("DATA");
        // for (int i = 0; i < HOLDING_REGS_SIZE*2; i++)
        //   {
        //     LOG("regs_D " + String(i) + ": ");
        //     LOGLN(holdingRegs[i]);
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
      LOGLN("CONFIG INFO");
      LOGLN("Baud rate: " + String(baudrate));
      LOGLN("Time out: " + String(timeouts));
      LOGLN("Retry time: " + String(retry_count));
      LOGLN("Polling: " + String(polling));
      LOGLN("----------------------------------------------------------------");
    }

  }
}