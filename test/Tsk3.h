//Task3 :
void Task3code( void * pvParameters ) {
  Serial.print("Task3 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
#ifdef ModbusCom
    

    delay(10);


  }//End loop
  #endif//ModbusCom
}
