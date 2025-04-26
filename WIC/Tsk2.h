
#ifdef PLC_MASTER_UI
#include <ModbusRTU.h>
#include "Modbus_RTU.h"
Modbus_Prog mb_task2;
ModbusRTU mbTks;
byte role = 0;
byte bbuf;
#endif// PLC_MASTER_UI

#include "WIC.h"
WIC wicTask2;
//Task2 :
void Task2code( void * pvParameters ) {
  LOG("Loop Server running on core ");
  LOGLN(xPortGetCoreID());

enum {slave,master};

   bool Tskonece2 = true;
   bool Tskonece1 = true;
  for (;;) {
#ifdef PLC_MASTER_UI
 if (!CONFIG::read_byte (EP_EEPROM_ROLE, &bbuf ) ) {} else {role = bbuf;}
 if(role == slave ){

}
 #endif//PLC_MASSTER_UI

  //  #ifdef LOOKLINE_UI
   if(wicTask2.GetSetup()){
      if (Tskonece1){Tskonece1 = false;if(looklineDebug)ESPCOM::println(F("Wifi Server Working..."), PRINTER_PIPE);
        #ifdef MQTT_USE
         if(WiFi.status() == WL_CONNECTED)PLC_MASTER_Prog.connectMQTT(1);
         #endif//MQTT
      }
            web_interface->web_server.handleClient();
            socket_server->loop();
                       
      if (WiFi.getMode() != WIFI_OFF){
  #ifdef CAPTIVE_PORTAL_FEATURE
        if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
        {
            dnsServer.processNextRequest();
            if (Tskonece2){Tskonece2 = false;if(looklineDebug)ESPCOM::println(F("Wifi Portal Working..."), PRINTER_PIPE)  ;

            
            }
        }
  #endif//CAPTIVE_PORTAL_FEATURE
    }
   }
  //  #endif//LOOKLINE_UI

  // LOG("Task1 running on core ");
  // LOGLN(xPortGetCoreID());
    delay(200);

  }//End loop
}
