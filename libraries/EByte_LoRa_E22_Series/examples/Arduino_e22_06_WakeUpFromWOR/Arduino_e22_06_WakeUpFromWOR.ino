/*
 * EBYTE LoRa E22
 * Stay in sleep mode and wait a wake up WOR message
 *
 * You must configure the address with 0 3 23 with WOR receiver enable
 * and pay attention that WOR period must be the same of sender
 *
 * Pai attention e22 support RSSI, if you want use that functionality you must enable RSSI on configuration
 * configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED;
 *
 * and uncomment #define ENABLE_RSSI true in this sketch
 *
 * https://www.mischianti.org
 *
 * E22		  ----- Arduino
 * M0         ----- 7 (or HIGH)
 * M1         ----- 6 (or GND)
 * RX         ----- 4 (PullUP)
 * TX         ----- 5 (PullUP)
 * AUX        ----- 3  (PullUP)
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 *
 */

// with this DESTINATION_ADDL 2 you must set
// WOR SENDER configuration to the other device and
// WOR RECEIVER to this device
#define DESTINATION_ADDL 2

// If you want use RSSI uncomment //#define ENABLE_RSSI true
// and use relative configuration with RSSI enabled
//#define ENABLE_RSSI true

#include "Arduino.h"
#include "LoRa_E22.h"
#include "LowPower.h"

const int wakeUpPin = 3;

#define FPM_SLEEP_MAX_TIME           0xFFFFFFF
void wakeUp()
{
    Serial.println("WAKE!");
    // Disable external pin interrupt on wake up pin.
    detachInterrupt(1);
}


// ---------- esp8266 pins --------------
//LoRa_E22 e22ttl100(RX, TX, AUX, M0, M1);  // Arduino RX <-- e22 TX, Arduino TX --> e22 RX
//LoRa_E22 e22ttl(D3, D4, D5, D7, D6); // Arduino RX <-- e22 TX, Arduino TX --> e22 RX AUX M0 M1
//LoRa_E22 e22ttl(D2, D3); // Config without connect AUX and M0 M1

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(D2, D3); // Arduino RX <-- e22 TX, Arduino TX --> e22 RX
//LoRa_E22 e22ttl(&mySerial, D5, D7, D6); // AUX M0 M1
// -------------------------------------

// ---------- Arduino pins --------------
LoRa_E22 e22ttl(4, 5, 3, 7, 6); // Arduino RX <-- e22 TX, Arduino TX --> e22 RX AUX M0 M1
//LoRa_E22 e22ttl100(4, 5); // Config without connect AUX and M0 M1

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(4, 5); // Arduino RX <-- e22 TX, Arduino TX --> e22 RX
//LoRa_E22 e22ttl(&mySerial, 3, 7, 6); // AUX M0 M1
// -------------------------------------

// ---------- esp32 pins --------------
//LoRa_E22 e22ttl100(&Serial2, 18, 21, 19); //  RX AUX M0 M1

//LoRa_E22 e22ttl100(&Serial2, 22, 4, 18, 21, 19, UART_BPS_RATE_9600); //  esp32 RX <-- e22 TX, esp32 TX --> e22 RX AUX M0 M1
// -------------------------------------

//The setup function is called once at startup of the sketch
void setup()
{
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB
    }
    delay(100);

    e22ttl.begin();
	e22ttl.setMode(MODE_1_WOR);

    delay(1000);
    Serial.println();
    Serial.println("Start sleep!");
    delay(100);

    Serial.println(digitalRead(wakeUpPin));

    //wifi_station_disconnect(); //not needed
    // Allow wake up pin to trigger interrupt on low.
    attachInterrupt(1, wakeUp, LOW);

    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

    Serial.println();
    Serial.println("Wake and start listening!");

}

// The loop function is called in an endless loop
void loop()
{
    if (e22ttl.available()  > 1){
    	Serial.println("Message arrived!");
#ifdef ENABLE_RSSI
        ResponseContainer rs = e22ttl.receiveMessageRSSI();
#else
        ResponseContainer rs = e22ttl.receiveMessage();
#endif
        // First of all get the data
        String message = rs.data;

        Serial.println(rs.status.getResponseDescription());
        Serial.println(message);

        e22ttl.setMode(MODE_0_NORMAL);

    	delay(1000);

        e22ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "We have received the message!");
    }
}
