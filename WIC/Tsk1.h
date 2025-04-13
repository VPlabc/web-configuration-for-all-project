
#include "config.h"
#include "WIC.h"
WIC wicTask1;
//Task1 :
void Task1code( void * pvParameters ) {
  LOG("Task1 running on core ");
  LOGLN(xPortGetCoreID());
  for (;;) {

#ifdef PLC_MASTER_UI
PLC_MASTER_Prog.loop();
// mb_prog.loop();
#endif//PLC_MASTER_UI
#ifdef LOOKLINE_UI
//////////////////////////////////////////////////////////////// loop 100mS
if(wicTask1.GetSetup()){
  /// @brief //// Loop 1 Seconds
  static uint32_t last_Loop10mS_update = 0;
  if (millis() - last_Loop10mS_update > (1 * 10))
  {   last_Loop10mS_update = millis();
      // LOGLN("Loop 10ms");
      #ifdef LOOKLINE_UI
      lookline_prog.TimerPlanInc();
      #endif// LOOKLINE_UI
  }
}
//////////////////////////////////////////////////////////////// loop 100mS
#endif//LOOKLINE_UI
    delay(10);

  }//End loop
}
