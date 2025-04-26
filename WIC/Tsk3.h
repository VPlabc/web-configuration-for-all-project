#include "config.h"
#ifdef MQTT_USE
#include "MQTTcom.h"
MQTTCOM MQTTTask3;
#endif//MQTT_USE
#include "WIC.h"
WIC wicTask3;
//Task2 :
void Task3code( void * pvParameters ) {
  LOG("Loop SD card running on core ");
  LOGLN(xPortGetCoreID());

enum {slave,master};

   bool Tskonece2 = true;
   bool Tskonece1 = true;
  for (;;) {
#ifdef PLC_MASTER_UI

 #endif//PLC_MASSTER_UI

  //  #ifdef LOOKLINE_UI
   if(wicTask3.GetSetup()){
      
        #ifdef MQTT_USE
         
         #endif//MQTT
      }
  //  #endif//LOOKLINE_UI

  // LOG("Task1 running on core ");
  // LOGLN(xPortGetCoreID());
    delay(200);

  }//End loop
}
