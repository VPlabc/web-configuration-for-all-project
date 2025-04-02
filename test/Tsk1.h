//Task1 :
void Task1code( void * pvParameters ) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {

    // digitalWrite(Vdc_Supply, 1);

   

    delay(200);

  }//End loop
}
