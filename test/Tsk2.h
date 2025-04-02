#include "Modbus_RTU.h"
Modbus_Prog MB_RTU;
//Task2 :
void Task2code( void * pvParameters ) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
#ifdef ModbusCom
MB_RTU.mb.task();
#endif//ModbusCom


 
delay(200);

  }//End loop
}
