#include <Arduino.h>
#include "config.h"
#ifdef Gyro_UI
#include <WIC.h>
WIC wics;


#include <MPU9250.h>
#include <quaternionFilters.h>
#include <MPU9250.h>
#include <IMUResult.h>
#include <IMUWriter.h>
#include "Gyro_Datalog.h"

#include <SPI.h>
#include <SD.h>

#include <EEPROM.h>
///////////////////////////////////////////////////////////////////
//Debug information
///////////////////////////////////////////////////////////////////
#define serialDebug true  // Set to true to get Serial output for debugging
#define baudRate 115200
// serialDebug
//calibrateMagnetometer
// samplingRateInMillis
//address
///////////////////////////////////////////////////////////////////
//Determines how often we sample and send data
///////////////////////////////////////////////////////////////////
#define samplingRateInMillis 1
#define batchSize 32 //Number or events to send to server in single batch.  This is meant to optimize for amount of memory on heap because we may not be able to load entire contents of eeprom into memory to send to server.
#define MAX_BACKOFF kbits_256 * 1024 / sizeof(IMUResult) / batchSize  //we backoff attempting to conect to MQTT Server because it a synchronous call which takes a few seconds.
uint32_t backOffFactor = 1;
byte address = 0;
uint32_t RealTime = 0;
///////////////////////////////////////////////////////////////////
//Setup for the Accelerometer
///////////////////////////////////////////////////////////////////
#define declination 15.93  //http://www.ngdc.noaa.gov/geomag-web/#declination . This is the declinarion in the easterly direction in degrees.  
#define calibrateMagnetometer false  //Setting requires requires you to move device in figure 8 pattern when prompted over serial port.  Typically, you do this once, then manually provide the calibration values moving forward.
MPU9250 myIMU;
IMUWriter writer(kbits_256, 1, 64, 0x50);  //These are the arguments needed for extEEPROM library.  See their documentation at https://github.com/JChristensen/extEEPROM
IMUResult magResult, accResult, gyroResult, orientResult;

///////////////////////////////////////////////////////////////////
//FC Gyro
//////////////////////////////////////////////////////////////////

#define CPU_MHZ 80
#define CHANNEL_NUMBER 8  //set the number of chanels
#define CHANNEL_DEFAULT_VALUE 1100  //set the default servo value
#define FRAME_LENGTH 22500  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PULSE_LENGTH 300  //set the pulse length
#define onState 0  //set polarity of the pulses: 1 is positive, 0 is negative
#define sigPin D1 //set PPM signal output pin on the arduino
#define DEBUGPIN D4
#define LEDstt -1

volatile unsigned long next;
volatile unsigned int ppm_running=1;

int ppm[CHANNEL_NUMBER];


unsigned int alivecount=0;
///////////////////////FC Gyro////////////////////////////
  void inline ppmISR(void){
  static boolean state = true;

  if (state) {  //start pulse
    digitalWrite(sigPin, onState);
    next = next + (PULSE_LENGTH * CPU_MHZ);
    state = false;
    alivecount++;
  } 
  else{  //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;
  
    digitalWrite(sigPin, !onState);
    state = true;

    if(cur_chan_numb >= CHANNEL_NUMBER){
      cur_chan_numb = 0;
      calc_rest = calc_rest + PULSE_LENGTH;// 
      next = next + ((FRAME_LENGTH - calc_rest) * CPU_MHZ);
      calc_rest = 0;
      digitalWrite(DEBUGPIN, !digitalRead(DEBUGPIN));
    }
    else{
      next = next + ((ppm[cur_chan_numb] - PULSE_LENGTH) * CPU_MHZ);
      calc_rest = calc_rest + ppm[cur_chan_numb];
      cur_chan_numb++;
    }     
  }
  timer0_write(next);
}
/* --------------------------- SD CARD -------------------------------------------- */


    void GyroDatalog::Setup(){
        Serial.begin(baudRate);
        Serial.setDebugOutput(true); //Used for more verbose wifi debugging

#ifndef FC_Gyro
        //Start IMU.  Assumes default SDA and SCL pins 4,5 respectively.
        myIMU.begin();

        //This tests communication between the accelerometer and the ESP8266.  Dont continue until we get a successful reading.
        //It is expected that the WHO_AM_I_MPU9250 register should return a value of 0x71.
        //If it fails to do so try the following:
        //1) Turn power off to the ESP8266 and restart.  Try this a few times first.  It seems to resolve the issue most of the time.  If this fails, then proceed to the followingn steps.
        //2) Go to src/MPU9250.h and change the value of ADO from 0 to 1
        //3) Ensure your i2c lines are 3.3V and that you haven't mixed up SDA and SCL
        //4) Run an i2c scanner program (google it) and see what i2c address the MPU9250 is on.  Verify your value of ADO in src/MPU9250.h is correct.
        //5) Some models apparently expect a hex value of 0x73 and not 0x71.  If that is the case, either remove the below check or change the value fro 0x71 to 0x73.
        byte c;
        do
        {
            c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
            if (c != 0x70)
            {
            Serial.println("Failed to communicate with MPU9250");
            Serial.print("WHO_AM_I returned ");
            Serial.println(c, HEX);
            delay(500);
            }
        } while (c != 0x70);

        Serial.println("Successfully communicated with MPU9250");


        // Calibrate gyro and accelerometers, load biases in bias registers, then initialize MPU.
        myIMU.calibrate();  
        myIMU.init();
        if (calibrateMagnetometer)
            myIMU.magCalibrate();
        else
            myIMU.setMagCalibrationManually(-166, 16, 663);    //Set manually with the results of magCalibrate() if you don't want to calibrate at each device bootup.
                                                            //Note that values will change as seasons change and as you move around globe.  These values are for zip code 98103 in the fall.

        Serial.println("Accelerometer ready");
#endif//FC_Gyro
        // WiFi.begin(ssid,password);
        // client.setServer(server, port);
        

        // accResult.setName("acc");
        // gyroResult.setName("gyro");
        // magResult.setName("mag");
        // orientResult.setName("orien");
        
        Serial.println("Sate:" + String(CONFIG::GetState()));
        pinMode(LEDstt, OUTPUT);
        CONFIG::read_byte(Address_File, &address);
//////////////////////// FC Gyro /////////////////////////////////
#ifdef FC_Gyro
  pinMode(sigPin,OUTPUT);
  digitalWrite(sigPin, !onState); //set the PPM signal pin to the default state (off)
  pinMode(DEBUGPIN,OUTPUT);
  digitalWrite(DEBUGPIN, !onState); //set the PPM signal pin to the default state (off)

        noInterrupts();
        timer0_isr_init();
        timer0_attachInterrupt(ppmISR);
        next=ESP.getCycleCount()+1000;
        timer0_write(next);
        for(int i=0; i<CHANNEL_NUMBER; i++){
            ppm[i]= CHANNEL_DEFAULT_VALUE;
        }
        interrupts();
#endif//FC_Gyro            
    }

    uint32_t lastSample = 0;
    uint32_t LEDlastSample = 0;
    void GyroDatalog::Loop(){
        if(CONFIG::GetState() == 1){
        #ifdef FC_Gyro
         if (millis() - LEDlastSample > 500)
        {
            //digitalWrite(LEDstt, !digitalRead(LEDstt)); LEDlastSample = millis();
        }
            ppm[0]=1500; ppm[1]=1100; ppm[2]=1500; ppm[3]=1500;
            ppm[4]=1800; ppm[5]=1100; ppm[6]=1800; ppm[7]=1800;
            alivecount=0;
        #else
        Read_Sensor();
        #endif//FC_Gyro
        }
        else{
        #ifdef FC_Gyro
            ppm[0]=1500; ppm[1]=1100; ppm[2]=1500; ppm[3]=1500;
            ppm[4]=1100; ppm[5]=1100; ppm[6]=1100; ppm[7]=1100;
            alivecount=0;
        #endif//FC_Gyro 
        }
      if(alivecount>1000){
        for(int i=0; i<4;i++){
          ppm[i]=900;
        }
        for(int i=4; i<8;i++){
          ppm[i]=1100;
        }
      }
      yield();
  }
    void GyroDatalog::Read_Sensor(){
#ifndef FC_Gyro
        myIMU.updateTime();
        // If intPin goes high, all data registers have new data
        // On interrupt, check if data ready interrupt
        if (myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
        {
            myIMU.readAccelData(&accResult);
            myIMU.readGyroData(&gyroResult);
            //myIMU.readMagData(&magResult);
        }

        // Must be called before updating quaternions!
        
        MahonyQuaternionUpdate(&accResult, &gyroResult, &magResult, myIMU.deltat);
        readOrientation(&orientResult, declination);
        if (millis() - LEDlastSample > 500)
        {
            digitalWrite(LEDstt, !digitalRead(LEDstt)); LEDlastSample = millis();
        }
        if (millis() - lastSample > samplingRateInMillis)
        {

        lastSample = millis();
    
            RealTime++;
            if (serialDebug == false)
            {
                Serial.print(RealTime);
                  Serial.print(",");
                gyroResult.printResult();
                  Serial.print(",");
                accResult.printResult();
                  Serial.println("");
                //magResult.printResult();
                //orientResult.printResult();
            }
        GyroDatalog::SaveData();
            Serial.print("Gyro Z:" + String(gyroResult.getZComponent()));
            Serial.print("|   Speed:" );
            Serial.println(myIMU.deltat , 5);
            myIMU.sumCount = 0;
            myIMU.sum = 0;

        }  
 #endif //FC_Gyro       
    }
//
void GyroDatalog::Start()
{
    if (!SD.begin(SDCard_CS, SPI_FULL_SPEED)) {Serial.println("SD card not found");}
    else{
            address++;RealTime = 0;CONFIG::write_byte(Address_File, address);
            Serial.println("Address: " + String(address));
            Serial.println("tscale:0.022");
            Serial.print("gscale:");
            Serial.println(myIMU.getGres(), 11);
            Serial.print("ascale:");
            Serial.println(myIMU.getAres(), 11);
            // Serial.print("mscale:");
            // Serial.println(myIMU.getMres(), 10);
            String NameFile = "VideoFile"+ String(address) + ".gcsv";
            File log_file = SD.open( NameFile,  FILE_WRITE );
            log_file.println("GYROFLOW IMU LOG");
            log_file.println("version,1.1");
            log_file.println("id,VPlab");
            log_file.println("orientation,yxz");
            log_file.println("tscale,0.01375");
            log_file.print("gscale,");
            log_file.println(myIMU.getGres(), 11);
            log_file.print("ascale,");
            log_file.println(myIMU.getAres(), 11);
            // log_file.print("mscale,");
            // log_file.println(myIMU.getMres(), 10);
            log_file.println("t,gx,gy,gz,ax,ay,az");
            log_file.flush();
            log_file.close();
    }
}
    void GyroDatalog::SaveData(){
            String NameFile = "VideoFile"+ String(address) + ".gcsv";
            File log_file = SD.open( NameFile, FILE_WRITE );
            log_file.print(RealTime);
            log_file.print(",");
            log_file.print(gyroResult.SaveResult());
            log_file.print(",");
            log_file.print(accResult.SaveResult());
            // log_file.print(",");
            // log_file.print(magResult.SaveResult());
            log_file.println("");
            log_file.flush();
            log_file.close();
            //saveMemoryToFile();
    }
    void GyroDatalog::readResult(IMUResult result, String &payload)
    {   
    String resName;
    result.getName(resName);

    float vals[3];
    String names[3];

    names[0]=resName+"_x";
    names[1]=resName+"_y";
    names[2]=resName+"_z";

    vals[0] = result.getXComponent();
    vals[1] = result.getYComponent();
    vals[2] = result.getZComponent();

    //iotpipe.jsonifyResult(&vals[0], &names[0], 3, payload);
    }

void GyroDatalog::Start_PPM()
{
   if(ppm_running==0)
  {
    noInterrupts();
    timer0_isr_init();
    timer0_attachInterrupt(ppmISR);
    next=ESP.getCycleCount()+1000;
    timer0_write(next);
    for(int i=0; i<CHANNEL_NUMBER; i++){
      ppm[i]= CHANNEL_DEFAULT_VALUE;
    }
    ppm_running=1;
    interrupts();
  }
}
#endif//Gyro_UI