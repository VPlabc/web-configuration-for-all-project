#include "PLC_IoT/PLC_Master.h"
PLC_MASTER mb_prog;
//Task1 :
void Task1code( void * pvParameters ) {
  LOG("Task1 running on core ");
  LOGLN(xPortGetCoreID());
  for (;;) {

#ifdef PLC_MASTER_UI
// PLC_MASTER_Prog.loop();
mb_prog.loop();
#endif//PLC_MASTER_UI
    delay(200);

  }//End loop
}