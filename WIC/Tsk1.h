#include "config.h"
#ifdef PLC_MASTER_UI
#include "PLC_IoT/PLC_Master.h"
PLC_MASTER mb_prog;
#endif//PLC_MASTER_UI
#include "WIC.h"
WIC wicTask1;
//Task1 :
void Task1code( void * pvParameters ) {
  LOG("Loop appli running on core ");
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
  static uint32_t last_Loop100mS_update = 0;
  if (millis() - last_Loop100mS_update > (1 * 100))
  {   last_Loop100mS_update = millis();
      // LOGLN("Loop 100ms");
      #ifdef LOOKLINE_UI
      lookline_prog.TimerPlanInc();
      #endif// LOOKLINE_UI
  }
}
//////////////////////////////////////////////////////////////// loop 100mS
#endif//LOOKLINE_UI
    delay(200);

  }//End loop
}
