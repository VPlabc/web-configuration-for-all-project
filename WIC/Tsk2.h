#include "config.h"
#ifdef PLC_MASTER_UI
#include "PLC_IoT/PLC_Master.h"
PLC_MASTER PLC_prog;
#endif//PLC_MASTER_UI
#include <ModbusRTU.h>
#include "Modbus_RTU.h"
Modbus_Prog mb_task2;
ModbusRTU mbTks;
//Task2 :
void Task2code( void * pvParameters ) {
  LOG("Task2 running on core ");
  LOGLN(xPortGetCoreID());
// byte role = 0;
// byte bbuf;
// enum {slave,master};

  for (;;) {
//  if (!CONFIG::read_byte (EP_EEPROM_ROLE, &bbuf ) ) {} else {role = bbuf;}
// if(role == slave ){

// }
   
#ifdef PLC_MASTER_UI
PLC_prog.loop();
// mb_prog.loop();
#endif//PLC_MASTER_UI
  // LOG("Task1 running on core ");
  // LOGLN(xPortGetCoreID());
    // delay(200);

  }//End loop
}
