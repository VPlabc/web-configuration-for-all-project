#ifndef MODBUS_
#define MODBUS_
#include <config.h>
#include <SimpleModbusMaster.h>

#ifdef MASTER_MODBUS
#endif//MASTER_MODBUS


// #ifdef SLAVE_MODBUS
/////////////// registers of your slave ///////////////////
enum 
{     
  // just add or remove registers and you're good to go...
  // The first register starts at address 0
  RGS1,
  RGS2,
  RGS3,
  RGS4,
  RGS5,
  RGS6,
  RGS7,
  RGS8,
  RGS9,
  RGS10,
  RGS11,
  RGS12,
  RGS13,
  RGS14,
  RGS15,
  RGS16,
  RGS17,
  RGS18,
  RGS19,
  RGS20,
  RGS21,
  RGS22,
  RGS23,
  RGS24,
  RGS25,
  RGS26,
  RGS27,
  RGS28,
  RGS29,
  RGS30,
  RGS31,
  RGS32,
  // leave this one
  TOTAL_REGS_SIZE 
  // total number of registers for function 3 and 16 share the same register array
};

// #endif//SLAVES_MODBUS




enum
{
  PACKET1,
  PACKET2,
  // leave this last entry
  TOTAL_NO_OF_PACKETS
};

////////////////////////////////////////////////////////////
//   extern void handleRoot();

class Modbus_Prog
{
public:
//////////////// registers of your slave ///////////////////
enum 
{     
  BOARDID,
  NETID,
  RUNSTOP,
  ONOFF,
  PLAN,
  PLANSET,
  RESULT,
  RESULTSET,
  MAXPLAN,
  PCS,
  TIMEINC,
  DELAYCOUNTER,
  ROLE,
  _RSSI,  
  COMMODE,
  TYPE,
  ONWIFI,
  CMD,   
  // Plan1,   
  // Plan2,
  // Plan3,
  // Plan4,
  // Plan5,
  // Plan6,
  // Plan7,
  // Plan8,
  // Plan9,
  // Plan10,
  // Plan11,
  // Plan12,
  // Plan13,
  // Plan14,
  // Plan15,
  // Plan16,
  // Plan17,
  // Plan18,
  // Plan19,
  // Plan20,
  // Plan21,
  // Plan22,
  // Plan23,
  // Plan24,
  // EVAG1,
  // EVAG2,
  // EVAG3,
  // EVAG4,
  // EVAG5,
  // EVAG6,
  // EVAG7,
  // EVAG8,
  // EVAG9,
  // EVAG10,
  // EVAG11,
  // EVAG12,
  // EVAG13,
  // EVAG14,
  // EVAG15,
  // EVAG16,
  // EVAG17,
  // EVAG18,
  // EVAG19,
  // EVAG20,
  // EVAG21,
  // EVAG22,
  // EVAG23,
  // EVAG24,
  // EVAG25,
  // EVAG26,
  // EVAG27,
  // EVAG28,
  // EVAGY1,
  // EVAGY2,
  // EVAGY3,
  // EVAGY4,
  // EVAGY5,
  // EVAGY6,
  // EVAGY7,
  // EVAGY8,
  // EVAGY9,
  // EVAGY10,
  // EVAGY11,
  // EVAGY12,
  HOLDING_REGS_SIZE // leave this one
};
//////////////////// Port information ///////////////////
// #define baudrate 9600
#define timeouts 300
#define polling 10 // the scan rate

// If the packets internal retry register matches
// the set retry count then communication is stopped
// on that packet. To re-enable the packet you must
// set the "connection" variable to true.
#define retry_count 500 

// used to toggle the receive/transmit pin on the driver
#define TxEnablePin -1 
Packet packets[TOTAL_NO_OF_PACKETS];
packetPointer packet1 = &packets[PACKET1];
packetPointer packet2 = &packets[PACKET2];
// This is the easiest way to create new packets
// Add as many as you want. TOTAL_NO_OF_PACKETS
// is automatically updated.
#ifdef MASTER_MODBUS
// Create an array of Packets for modbus_update()

#else//Slave

#endif//MASTER_MODBUS

 int regs_WRITE[HOLDING_REGS_SIZE];
 int regs_last_WRITE[HOLDING_REGS_SIZE];
 int regs_READ[HOLDING_REGS_SIZE];
 int regs_last_READ[HOLDING_REGS_SIZE];
// The data from the PLC will be stored
// in the regs array

unsigned int regs_Data[HOLDING_REGS_SIZE*2];

void modbus_setup(bool role) ;
void modbus_loop(bool role) ;
void debugs();
void modbus_write_update(int HOLDING_REGS_Data[]);
void modbus_read_update(int HOLDING_REGS_Data[]) ;            
void modbus_write_setParameter(int pos,int Value, int cmd);
void Clear_Lookline_Value();
void GetData(int regs_READ[]);
private:



};

#endif//MODBUS_