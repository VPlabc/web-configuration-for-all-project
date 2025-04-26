#include <Arduino.h>

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













void ModbusTCPSetup(){
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
}


void ModbusTCPLoop(){
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
}