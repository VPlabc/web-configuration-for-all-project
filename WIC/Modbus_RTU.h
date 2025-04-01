#ifndef MODBUS_
#define MODBUS_
#include <config.h>
#include "HardwareSerial.h"

#include "DFRobot_RTU.h"
#include <ModbusRTU.h>
#ifdef MASTER_MODBUS
#endif//MASTER_MODBUS

class Modbus_Prog
{
public:
bool MB_connect = false;
DFRobot_RTU Modbus_Master;
ModbusRTU mb;
//////////////// registers of your slave ///////////////////
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



// uint8_t coils[30];
// uint8_t discreteInputs[30];
//////////////////////// các thanh ghi Data cho 4 máy
uint16_t holdingRegisters[120];//0-30 PLC Data /30-34 total Plan / 34-74 product name 
// uint16_t inputRegisters[20];// data tu web gui ve public 0-30 PLC Data /30-34 total Plan / 34-74 product name 
uint16_t ReadRegTemporary[10];//thanh ghi Read tạm thời
// uint16_t WriteRegTemporary[10];//thanh ghi Write tạm thời

// The data from the PLC will be stored
// in the regs array

// unsigned int regs_Data[HOLDING_REGS_SIZE*2];
uint16_t* getInputRegs();
uint16_t* getOutputRegs();
void update();
void initupdate();
void modbusSet(uint16_t addr, uint16_t value);
void modbusWriteBuffer(uint16_t addrOffset, uint16_t *value);
void connectModbus(bool update);
bool getStart();
void modbus_setup(bool role) ;
void modbus_loop(bool role) ;
void debugs();
void Write_PLC();
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