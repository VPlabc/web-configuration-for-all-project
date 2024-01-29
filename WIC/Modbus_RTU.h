#ifndef MODBUS_
#define MODBUS_
#include <config.h>
#include "HardwareSerial.h"

#include "DFRobot_RTU.h"
#include <ModbusRTU.h>
#ifdef MASTER_MODBUS
#endif//MASTER_MODBUS


// #ifdef SLAVE_MODBUS
/////////////// registers of your slave ///////////////////
// enum 
// {     
//   // just add or remove registers and you're good to go...
//   // The first register starts at address 0
//   RGS1,
//   RGS2,
//   RGS3,
//   RGS4,
//   RGS5,
//   RGS6,
//   RGS7,
//   RGS8,
//   RGS9,
//   RGS10,
//   RGS11,
//   RGS12,
//   RGS13,
//   RGS14,
//   RGS15,
//   RGS16,
//   RGS17,
//   RGS18,
//   RGS19,
//   RGS20,
//   RGS21,
//   RGS22,
//   RGS23,
//   RGS24,
//   RGS25,
//   RGS26,
//   RGS27,
//   RGS28,
//   RGS29,
//   RGS30,
//   // RGS31,
//   // RGS32,
//   // leave this one
//   TOTAL_REGS_SIZE 
//   // total number of registers for function 3 and 16 share the same register array
// };

// #endif//SLAVES_MODBUS




// enum
// {
//   PACKET1,
//   PACKET2,
//   // leave this last entry
//   TOTAL_NO_OF_PACKETS
// };

////////////////////////////////////////////////////////////
//   extern void handleRoot();

class Modbus_Prog
{
public:
bool MB_connect = false;
DFRobot_RTU Modbus_Master;
ModbusRTU mb;
//////////////// registers of your slave ///////////////////
// enum 
// {     
//   // just add or remove registers and your good to go...
//   // The first register starts at address 0
//   // BOARDID,
//   // NETID,
//   // RUNSTOP,
//   // ONOFF,
//   // PLAN,
//   // PLANSET,
//   // RESULT,
//   // RESULTSET,
//   // MAXPLAN,
//   // PCS,
//   // TIMEINC,
//   // DELAYCOUNTER,
//   // ROLE,
//   // COMMODE,
//   // TYPE,
//   // ONWIFI,
//   // RSSI,  
//   // CMD,   
//   HOLDING_REGS_SIZE = 60// leave this one
//   // total number of registers for function 3 and 16 share the same register array
//   // i.e. the same address space
// };
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



uint8_t coils[30];
uint8_t discreteInputs[30];
uint16_t holdingRegisters[60];
uint16_t inputRegisters[60];// data tu web gui ve public

// The data from the PLC will be stored
// in the regs array

// unsigned int regs_Data[HOLDING_REGS_SIZE*2];
uint16_t* getInputRegs();
uint16_t* getOutputRegs();
void update();
void initupdate();
void modbusSet(uint16_t addr, uint16_t value);
void connectModbus(bool update);
bool getStart();
void modbus_setup(bool role) ;
void modbus_loop(bool role) ;
void debugs();
void Write_PLC(uint16_t addrPLC, uint16_t valuePLC);
void setModbusupdateState(bool state);
bool getModbusupdateState();
uint16_t getModbusupdateData();
uint16_t getModbusupdateAddr();
// void modbus_write_update(int16_t HOLDING_REGS_Data[]);
// void modbus_read_update(int16_t HOLDING_REGS_Data[]) ;            
void modbus_write_setParameter(int pos,int Value, int cmd);
void Clear_Lookline_Value();
private:



};

#endif//MODBUS_