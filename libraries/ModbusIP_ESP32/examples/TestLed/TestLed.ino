/*
  Modbus-Arduino Example - Test Led (Modbus IP ESP32)
  Control a Led on GPIO0 pin using Write Single Coil Modbus Function
  Copyright by André Sarmento Barbosa
  http://github.com/andresarmento/modbus-arduino
*/

#include <Modbus.h>
#include <ModbusIP_ESP32.h>

//Modbus Registers Offsets (0-9999)
const int LED_COIL = 100;
const int SWITCH_ISTS = 101;

//Used Pins
const int ledPin = 2; //GPIO0

//ModbusIP object
ModbusIP mb;

void setup() {
  Serial.begin(115200);

  mb.config("Hoang Vuong", "91919191");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(ledPin, OUTPUT);
  mb.addCoil(LED_COIL);
}

void loop() {
   //Call once inside loop() - all magic here
   mb.task();

   //Attach ledPin to LED_COIL register
   digitalWrite(ledPin, mb.Coil(LED_COIL));

   mb.Ists(SWITCH_ISTS, digitalRead(LED_COIL));
}
