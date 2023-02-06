
#include <Arduino.h>
#include "config.h"
#ifdef LOOKLINE_UI

// #include "wificonf.h"
#include "espcom.h"
// #include "webinterface.h"
// #include "command.h"
#include "WIC.h"
WIC looklineWIC;
#include "LookLine.h"
 Command cmdLookLine;
 LOOKLINE_PROG Lookline_PROG;
#include "7SegModule.h"
ESPResponseStream espresponse;
#include "FirmwareUpdate.h"
UpdateFW UDFWLookLine;
#include "syncwebserver.h"
WebSocketsServer * socket_servers;

#ifdef USE_LORA
    #include "LoRa_E32.h"
#endif//USE_LORA
#ifdef MQTT_Mode
    #include <PubSubClient.h>
    WiFiClient client;
    PubSubClient mqtt(client);
#endif//MQTT_Mode

byte DispMode = Main;
byte role = 0;
#include "MeshNetwork.h"
#include "MQTT.h"
#include "LoRa.h"
#define LoRa_Seri Serial2
#define PC_Seri Serial
#include "Task_Prog.h"
TaskPin taskPins;

bool newFWdetec = false;
byte Debug = true;

int CountOT_m=0;
int CountOT_Hm=0;
int CountOT_Lm=0;
int PLAN=0;
int RESULT=0;
byte ModuleType=0;
byte AmountNode = 0;
byte CheckLoss = 0;
  bool setupEn = false;
  byte NodeRun = 1;

  int NodeID =0;

  byte BoardIDs = 01;
  byte NetIDs = 01;

  uint8_t SHCP = 0;
  uint8_t STCP = 0;
  //////////////////
  uint8_t DATA1 = 0;
  //////////////////
  uint8_t DATA2 = 0;
  /////////////////
  uint8_t DATA3 = 0;
  ////// LED
  uint8_t Signal_LED = 0;
  uint8_t Startus_LED = 0;
  uint8_t M0 = 0;
  uint8_t M1 = 0;

  byte TimeSent = 10;
  byte MonitorMode = 0;
  byte ComMasterSlave = 1;
  byte Lora_CH = 0;
  int DisplayMode = Main;
  byte ComMode = LoRa;
  byte WiFiMode = 0;
  int counter2 = 0;//nhap nhay
  int counter3 = 0;//connected 
  int Counter4 = 0;//GatewayTimeoutSend 
  int Counter5 = 0;//GatewayTimeoutfeedback 
  int Counterstatus = 10000;//GatewayTimeout status
  byte countSer;
  // bool setupEn;
  bool done;
  // byte NodeID;
  int IDSent = 0;
  char buffer[250];
bool LookLineOnce = true;
bool LookLineOnce1 = true;
  int Time = 10;

 #ifdef __cplusplus
  extern "C" {
 #endif
  uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
void LOOKLINE_PROG::caculaOT()
{
  float nums = 0;
  if (PLAN > RESULT && pcsInShift > 0)
  {
    //Debug_Ser.println("Step1");
    CountOT_m = (PLAN - RESULT)*100;
    //Debug_Ser.println("Step2");
    nums = CountOT_m / pcsInShift;
    //Debug_Ser.println("Step3");
  //Debug_Ser.println(nums);
    CountOT_Hm = nums / 100;
    //Debug_Ser.println("Step4");
    CountOT_Lm = nums - (CountOT_Hm * 100);
    //Debug_Ser.println("Step5");
  }
  else
  {
    CountOT_m = 0;
    CountOT_Hm = 0;
    CountOT_Lm = 0;
  //ActionOT = CountOT;
  }
}

void LOOKLINE_PROG::UpdateLookLineData(){
  CONFIG::read_byte(EP_EEPROM_ID, &BoardIDs);
  CONFIG::read_byte(EP_EEPROM_NETID, &NetIDs);
  CONFIG::read_byte(EP_EEPROM_CHANELS, &Lora_CH);
  CONFIG::read_buffer(EP_EEPROM_PLAN,(byte *) &PLAN, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_PLAN_SET,(byte *) &PLanSet, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_RESULT,(byte *) &RESULT, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_RESULT_SET,(byte *) &ResultSet, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_PLANMAX,(byte *) &PlanLimit, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_PCS,(byte *) &pcsInShift, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_TIME_PLAN,(byte *) &Time, INTEGER_LENGTH);
  CONFIG::read_buffer(EP_EEPROM_TIMESENT,(byte *) &TimeSent, INTEGER_LENGTH);
  CONFIG::read_byte(EP_EEPROM_AMOUNTNODE, &AmountNode);
  CONFIG::read_byte(EP_EEPROM_RUN, &NodeRun);
  CONFIG::read_byte (EP_EEPROM_ROLE, &role);
  CONFIG::read_byte (EP_WIFI_MODE, &WiFiMode);
  CONFIG::read_byte (EP_EEPROM_DEBUG, &Debug);
  SetChanel(Lora_CH);
  if(Debug)LOGLN("Update Data");
}

void LOOKLINE_PROG::DebugOut(String msg,byte output){//1 = Web / 2 = Serial
if(output == 1) ESPCOM::webprint(msg);
if(output == 2) ESPCOM::print(msg, DEBUG_PIPE, &espresponse);
}

void LOOKLINE_PROG::LookLineInitB(int pos,byte Mode){
CONFIG::read_byte(pos, &Mode);
if(pos == EP_EEPROM_ROLE){if(Debug)LOGLN("Role is :" + String(Mode));LookLineOnce1 = true;}
if(pos == EP_EEPROM_COM_MODE){if(Debug)LOGLN("Com mode is :" + String(Mode));ComMode = Mode;}
if(pos == EP_EEPROM_UPDATE_MODE){if(Debug)LOGLN("Update mode is :" + String(Mode));}
if(pos == EP_EEPROM_AMOUNTNODE){if(Debug)LOGLN("Amount node is :" + String(Mode));AmountNode = Mode;}
if(pos == EP_EEPROM_RUN){if(Debug)LOGLN("Run set is :" + String(Mode));NodeRun = Mode;
    caculaOT();SerDisplay(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);
  }
if(pos == EP_EEPROM_ON_OFF){if(Debug)LOGLN("On/Off set is :" + String(Mode));}
if(pos == EP_EEPROM_CHANELS){if(Debug)LOGLN("Chanel set is :" + String(Mode));SetChanel(Mode);Lora_CH = Mode;}
if(pos == EP_EEPROM_MODULE_TYPE){if(Debug)LOGLN("Set Module type :" + String(Mode));PinMapInit();LookLineOnce1 = true;}
if(pos == EP_EEPROM_TIMESENT){if(Debug)LOGLN("Time Sent :" + String(Mode));}
if(pos == EP_EEPROM_DEBUG){if(Debug)LOGLN("Debuf Mode:" + String(Mode));}
// EP_EEPROM_TIMESENT
}


void LOOKLINE_PROG::LookLineInitI(int pos,int Mode){
CONFIG::read_buffer(pos,(byte*) &Mode, INTEGER_LENGTH);
if(pos == EP_EEPROM_TIME_PLAN){if(Debug)LOGLN("Time for Plan is :" + String(Mode));Time = Mode;}
if(pos == EP_EEPROM_TIMESENT){if(Debug)LOGLN("TIME sent is :" + String(Mode));TimeSent = Mode;}
if(pos == EP_EEPROM_PLAN){if(Debug)LOGLN("Plan is :" + String(Mode));PLAN = Mode;
    caculaOT();SerDisplay(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);
}
if(pos == EP_EEPROM_PLAN_SET){if(Debug)LOGLN("Plan set is :" + String(Mode));PLanSet = Mode;}
if(pos == EP_EEPROM_RESULT){if(Debug)LOGLN("Result is :" + String(Mode));RESULT = Mode;
      caculaOT();SerDisplay(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);

}
if(pos == EP_EEPROM_RESULT_SET){if(Debug)LOGLN("Result set is :" + String(Mode));ResultSet = Mode;}
if(pos == EP_EEPROM_PCS){if(Debug)LOGLN("Result set is :" + String(Mode));pcsInShift = Mode;}
if(pos == EP_EEPROM_COUNTER_DELAY){if(Debug)LOGLN("Counter delay :" + String(Mode));SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);}
SerDisplay();
}
void LOOKLINE_PROG::PinMapInit(){
  
CONFIG::read_byte(EP_EEPROM_MODULE_TYPE, &ModuleType);
if(ModuleType == ModGateway){
  uint8_t MapPin1[10] = {27,14, 2,15,19,25,26,18, 5, 4};// main board gateway V14
  for(byte i = 0 ; i < 10; i++){MapPin[i] = MapPin1[i];}
  CONFIG::write_byte (EP_EEPROM_ROLE, GATEWAY) ;
 if(Debug){if(Debug)LOGLN("Lookline Gateway V14");}//Debug
}//if(ModuleType == MoGateway){
else{//lookline main V14
  uint8_t MapPin0[10] = {25,26, 2,15,19,27,13,22,21, 4};// main board lookline V14
  for(byte i = 0 ; i < 10; i++){MapPin[i] = MapPin0[i];}
  CONFIG::write_byte (EP_EEPROM_ROLE, NODE) ;
}
  SHCP = MapPin[0];
  STCP = MapPin[1];
  DATA1 = MapPin[2];
  DATA2 = MapPin[3];
  DATA3 = MapPin[4];
  Signal_LED = MapPin[5];
  Startus_LED = MapPin[6];
  M0 = MapPin[7];
  M1 = MapPin[8];
  PWR = MapPin[9];

  if(Debug)LOGLN("pinmap --- SHCP:" + String(SHCP) + " STCP:" + String(STCP)  + " DATA1:" + String(DATA1)   + " DATA2:" + String(DATA2) + " DATA3:" + String(DATA3) + " M0:" + String(M0) + " M1:" + String(M1) + " X0:" + String(X0)  + " X1:" + String(X1) + " X2:" + String(X2)  + " X3:" + String(X3)  + " X4:" + String(X4) + " Status_LED:" + String(Startus_LED) + " Signal_LED:" + String(Signal_LED) ) ;

  pinMode(PWR, OUTPUT);
  digitalWrite(PWR, HIGH);

  pinMode(SHCP, OUTPUT);
  pinMode(STCP, OUTPUT);
  pinMode(DATA1, OUTPUT);
  pinMode(DATA2, OUTPUT);
  pinMode(DATA3, OUTPUT);
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(0, INPUT_PULLUP);
  pinMode(X0, INPUT_PULLUP);
  pinMode(X1, INPUT_PULLUP);
  pinMode(X2, INPUT_PULLUP);
  pinMode(X3, INPUT_PULLUP);
  pinMode(X4, INPUT_PULLUP);
  
  //SW_Mode

  pinMode(Signal_LED, OUTPUT);
  pinMode(Startus_LED,OUTPUT);
}

void ShowParameters()
{
    if(Debug)LOGLN("Counter4:" + String(Counter4) + " |Counter5:" + String(Counter5) + " |Role: " + String(role) + " |Amount node: " + String(AmountNode) + " | Module type: " + String(ModuleType) + "| Com Mode: " + String(ComMode) + "| M0:" + String(digitalRead(M0)) + "| M1:" + String(digitalRead(M1) + "| CH:" + String(Lora_CH)));
}
void LOOKLINE_PROG::displayMode(byte Mode){
  DispMode = Mode;
}

void LOOKLINE_PROG::SetPlan(int SetPlans){
  PLAN = SetPlans;
  caculaOT();SerDisplay(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);
}

void LOOKLINE_PROG::SetResult(int SetResults){
  RESULT = SetResults;
  caculaOT();SerDisplay(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);
}

void LOOKLINE_PROG::SetRun(byte SetRuns){
  if(SetRuns < 2){
  NodeRun = SetRuns;if(Debug)LOGLN("Run/Stop Lookline");
  LOG("Run:"+ String(NodeRun));
  }
  if(SetRuns == 2){
    if(Debug)LOGLN("On/Off Lookline");
    SetValue(10,  10,  10,  10, 0);
  }

}

void LOOKLINE_PROG::SetDone(){
  done = true;
}



void LOOKLINE_PROG::setup() {
  Serial.begin(112500);
  Serial2.begin(9600);
  UpdateLookLineData();PinMapInit();
        // if(Debug)LOGLN("Main Data1--" + String(DATA1));//2
        // if(Debug)LOGLN("Main Data2--" + String(DATA2));//15
        // if(Debug)LOGLN("Main Data3--" + String(DATA3));//19
        // if(Debug)LOGLN("Main STCP--" + String(STCP));//26
        // if(Debug)LOGLN("Main SHCP--" + String(SHCP));//25
  SetPin(DATA1, DATA2, DATA3, SHCP, STCP, BoardIDs, NetIDs, Lora_CH, Startus_LED, TimeSent);
  PrintSeg(Seg[0], Seg[0], Seg1[0]);latch();
  SetPinLoRa( M0,  M1,  16,  17);
  ReadLoRaConfig();
  if(ModuleType != ModGateway){ SetPin7Seg(DATA1, DATA2, DATA3, SHCP, STCP);}
  // #if defined(DEBUG_WIC) && defined(DEBUG_OUTPUT_SERIAL)
  //           // CONFIG::InitBaudrate(DEFAULT_BAUD_RATE);
  //           // delay(2000);
  //           LOG("\r\nDebug Serial set\r\n")
  //           DebugOut("\r\nSet serial baudrate\r\n", OUPUT);
  // #endif
  DebugOut("Initialized.", OUPUT);
          String monitor = "";
            if(ComMode == MESH){ monitor += "Communica: MESH |";}
            if(ComMode == MQTT){ monitor += "Communica: MQTT |";}
            if(ComMode == LoRa){ monitor += "Communica: Lora |";}
            if(ComMode == RS485com){ monitor += "Communica: RS485 |";}

            monitor += "firmware:";
            monitor += UDFWLookLine.FirmwareVer;
            monitor += "\nX0: ";
            monitor += String(analogRead(X0));
            monitor += "  X1:";
            monitor += String(analogRead(X1));
            monitor += "  X2:";
            monitor += String(analogRead(X2));
            monitor += "  X3:";
            monitor += String(analogRead(X3));
            monitor += "  X4:";
            monitor += String(analogRead(X4));
         if(Debug)LOGLN(monitor);
  // String URL_FW = "http://";
  // CONFIG::write_string (EP_EEPROM_URL_FW, URL_FW.c_str() ) ;
  // CONFIG::write_string (EP_EEPROM_URL_VER, URL_FW.c_str() ) ;

  /////// Test
#ifdef TestDisplayIntro
if(ModuleType != ModGateway){  
  for (int i = 0; i < 10; i++)
  {
    // TestDisplay(i);
    displays(i*1111, i*1111, i*1111, i*1111, taskPin.Data1, taskPin.Data2, taskPin.Data3, taskPin.SHCP, taskPin.STCP, 0);latch();

    // #ifdef TEST_MODE
    digitalWrite(Startus_LED, digitalRead(Startus_LED) ^ 1);
    // #endif//TEST_MODE
    delay(300);
  }
caculaOT(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);
}
#endif//not dev
SerDisplay();

  Serial2.begin(9600);
}
/* ############################ Loop ############################################# */


// #define TEST_LORA


  void LoRaCommu();


void LOOKLINE_PROG::loop()
{
  //  Counterstatus++;if(Counterstatus > 10000){Counterstatus = 0;LookLineOnce1 = true;
  // if(Debug)LOGLN("Status");UpdateLookLineData();
  // }
  if(LookLineOnce){LookLineOnce = false; DebugOut("Lookline Run.", OUPUT);}
  TaskDisplay(DispMode);
  TaskInPut();
    // if (role == NODE || role == REPEARTER ){digitalWrite(Startus_LED, NodeRun);}
  #ifdef ModbusSlave
  modbus_loop();
  #endif//#ifdef ModbusSlave

////////////////////////////////// COMUNICATION //////////////////////////////////
  #ifdef RS485
  if(ComMode == RS485com && Mode > 0){if(ComMasterSlave == false){RS485COM();}}//
  #endif//RS485
  #ifdef MQTT_Mode
  if(ComMode == MQTT){MQTT_loop();}
  #endif//MQTT_Mode
  #ifdef USE_LORA
  if(ComMode == LoRa){LoRaCommu();}
  #endif//LoRa_Network
  #ifdef Mesh_Network
  if(ComMode == MESH){}
  #endif//Mesh_Network

  digitalWrite(M0, LOW);digitalWrite(M1, LOW);
  
  /////////////////////////////////// Gateway mode ///////////////////////////
  if(LookLineOnce1){LookLineOnce1 = false; ShowParameters();}
  if(role == GATEWAY && AmountNode > 0 && ModuleType == ModGateway){
    if(Counter4 == 0){
      Counter4++;NodeRun = false;
       if(Debug)LOGLN("Send to node " + String(IDSent));
      if(ModuleType == ModGateway){
      digitalWrite(Startus_LED, HIGH);delay(100);digitalWrite(Startus_LED, LOW);
      }//if(ModuleType == ModGateway){
    #ifdef TEST_LORA
    LoRa_Seri.println("Send to node " + String(IDSent));
    #else
      done = true;
      buffer[4] = '0';buffer[5] = '4';
      buffer[6] = '0';buffer[7] = '0';
      Data_Proccess();
    #endif//  #ifdef USE_LORA
    }
    if(Counter4 >= 6){
      Counter4 = 0;
    }
    if(Counter5 == TimeSent){
      CheckLoss++;if(CheckLoss > 5)CheckLoss = 0;
      IDSent++;if(IDSent > AmountNode){IDSent = 0;}
      Counter4++;
      Counter5 = 0;
    }
    if(Counter5 == (TimeSent/3)){
      IDSent++;if(IDSent > AmountNode){IDSent = 0;}
      Counter4++;
    }
    if(Counter5 == (TimeSent/3) + (TimeSent/3)){
      IDSent++;if(IDSent > AmountNode){IDSent = 0;}
      Counter4++;
    }
  }
///////////////////////////////////////////////////////////////////////////////////
}









  int Data[5][100];
  int LastTime[100];
  int MaxTime[100];

void LOOKLINE_PROG::SetParameter(int Plan, int taskPLanSet, int Result, int ResultSet, int taskTime, int taskpcsInShift, int taskPass, int taskDotIn){
  PLAN = Plan;PLanSet = taskPLanSet;RESULT = Result;ResultSet = ResultSet;Time = taskTime;pcsInShift = taskpcsInShift;Pass = taskPass;DotIn = taskDotIn;
  if(Debug)LOGLN("Set parameters from gateway")
}

#define TempDisplay

bool OnceCheck = true;
int totalInterruptCounterLookline = 0; 
void LOOKLINE_PROG::TimerPlanInc()
{
  if(WiFi.status() == WL_CONNECTED && OnceCheck){OnceCheck = false;
    // socket_servers->loop();
    if(UDFWLookLine.FirmwareVersionCheck() == 1){
      String s = "STATUS: New version";
      if(Debug)LOGLN(s);
      looklineWIC.checkFW();
      // socket_servers->broadcastTXT(s);
    }
  }
  // && (ComMode == LoRa || ComMode == MESH || ComMode == MQTT || ComMode == RS485com)
  if(role == GATEWAY){ Counter5++;
  // if(Debug)LOGLN("Gateway");
    for(byte i = 0 ; i < AmountNode; i++){
      LastTime[i]++;if(LastTime[i] > 10000)LastTime[i] = 10000;
      Data[2][i] = LastTime[i]/10;
      if(MaxTime[i] < LastTime[i]){MaxTime[i] = LastTime[i];}
      }
    }
totalInterruptCounterLookline++;
  ///*
  if (role == NODE || role == REPEARTER )
  { ////LED for Wifi Config
    countLED++;
    if (role == NODE)
    {
      if (countLED > 1 && countLED < 10)
      {
        // digitalWrite(Startus_LED, LOW);
        // digitalWrite(Signal_LED, LOW);
      }
      if (countLED > 10 && countLED < 100)
      {
        // digitalWrite(Startus_LED, HIGH);
        // digitalWrite(Signal_LED, HIGH);
      }
      if (countLED > 100)
      {
        countLED = 0;
      }
      if (countLED % 10 == 9)
      {
        connected(counter3++);
        if (counter3 > 3)
          counter3 = 0;
      }
    }
    if (role == REPEARTER)
    {
      if (countLED > 1000)
      {
        countLED = 0;
        // digitalWrite(Startus_LED, digitalRead(Startus_LED) ^ 1);
        // digitalWrite(Signal_LED, digitalRead(Signal_LED) ^ 1);
      }
    }
  }
  
  if (role == NODE || role == REPEARTER )
  { //////Run/Stop Plan
    if (NodeRun == true)
    {
      digitalWrite(Startus_LED, HIGH);
      counter2++;
      int TimeCacu = 0;
      if (DotIn == 0)
      {
        TimeCacu = (Time*10) * 1;
      }
      if (DotIn == 1)
      {
        TimeCacu = (Time*10) * 10;
      }
      if (DotIn == 2)
      {
        TimeCacu = (Time*10) * 100;
      }
      if (DotIn == 3)
      {
        TimeCacu = (Time*10) * 1000;
      }
      
      if (counter2 >= Time)
      {
        counter2 = 0;
        //digitalWrite(Signal_LED, digitalRead(Signal_LED) ^ 1);
        PLAN = PLAN + PLanSet;//MODBUS_Write();
        caculaOT(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);SerDisplay();
        if(PLAN % 10 == 9){
                if (!CONFIG::write_buffer (EP_EEPROM_PLAN, (const byte *) &PLAN, INTEGER_LENGTH) ) {
                  LOG("save Plan failed");
                }
           CONFIG::read_buffer(EP_EEPROM_PLAN, (byte *) &PLAN, INTEGER_LENGTH);
        }
        
        if (PLAN > PlanLimit)
        {
          PLAN = PlanLimit;
        }
        //
      }
    }
    else{
      digitalWrite(Startus_LED, LOW);
    }
    countLED++;
    if (countLED > 100)
      {//Monitor
        if (NodeRun == false){caculaOT(); SetValue(PLAN,  RESULT,  CountOT_Hm,  CountOT_Lm, NodeRun);SerDisplay();}
        // if(NodeRun){LOG("Runing");}else{LOG("Stopping");} if(Debug)LOGLN(" | Time: "+ String(Time));
        countLED = 0;
      }
  }
    if(totalInterruptCounterLookline >= 100){
      
    // if(Debug)LOGLN("TimerPlanInc | Role:" + String(role));
    SerDisplay();
    if(MonitorMode == Main){
    #ifdef TempDisplay
      // SerDisplay();
    #endif// TempDisplay 
    } 
    if(MonitorMode == 1){
     #ifdef MQTT_Mode
      if(ComMode == MQTT && Mode != 3 && countermqtt > 50){  
        #ifdef TempDisplay
        SerDisplay();    
        #endif// TempDisplay 
        }
      #endif//MQTT_Mode  
    }
    totalInterruptCounterLookline = 0;
    }
  //*/
}





void LoRaCommu()
{
   digitalWrite(M0, LOW);digitalWrite(M1, LOW);
  #ifdef USE_LORA
  while (LoRa_Seri.available())
  {    
    char inChar = (char)LoRa_Seri.read();
    LOG(inChar);

        //if (inChar == '\n') {
        if (setupEn == false)
        {
          buffer[countSer] = (char)inChar;
          //LoRaInput += (char)inChar;
          countSer++;
          if (countSer > 50)
          {
            setupEn = true;
            inChar = 0;
            countSer = 0;
          }
        }
    if (inChar == '\n')
      {   
      if(Debug)LOGLN("Done");
      #ifdef TEST_LORA
        // LOG(LoRa_Seri.read()); 
      if (role == NODE || role == REPEARTER )
      { 
        LoRa_Seri.write(LoRa_Seri.read());
      }
      #else//TEST_LORA
        done = true;
        setupEn = true;
        String ids = "";
        ids += buffer[0];
        ids += buffer[1];
        ids += buffer[2];
        ids += buffer[3];
        NodeID = ids.toInt();
        Data_Proccess();
        LoRa_Seri.flush();
        inChar = 0;
        countSer = 0;
        setupEn = false;
    #endif//TEST_LORA
      }
  }
  #endif//LoRa_Network
}

void Data_Proccess()
{
if(done == true){
        done = false;
    String cmds = String(buffer[4]) + String(buffer[5]);
    String Length = String(buffer[6]) + String(buffer[7]);
    // if(Debug)LOGLN("Cmd:" + String(cmds.toInt()));
    // if(Debug)LOGLN("Length:" + String(Length.toInt()));
    if (cmds.toInt() == cmdLookLine.updateStt && (role == NODE || role == REPEARTER))
    {
      if (Length.toInt() == 12)
      {
        if(NodeID == BoardIDs)
        {
          // if(Debug){
            if(Debug)LOGLN("Sent to gateway " );
            // if(ComMode == MQTT){ LOG("  Topic:");if(Debug)LOGLN(String(Lookline_PROG.TopicOut));}
            // else{if(Debug)LOGLN();}
          // }
          // if(Debug){
            #ifdef MQTT_Mode
              if(ComMode == MQTT && Mode != 3){  
                if(countermqtt > 50){
                String sentData = "Sent to gateway  Topic:" + String( TopicOut);
                for(int c = 0 ; c < sentData.length()+2;sentData.toCharArray(Buffer, c++));
                mqtt.publish("/TopicOut",buffer);
                //client.publish(TopicOut,buffer);
                delay(100);
                }
              }
              #endif//MQTT_Mode  
          // }
          String id = "";
          id += String((BoardIDs / 1000) % 10);
          id += String((BoardIDs / 100) % 10);
          id += String((BoardIDs / 10) % 10);
          id += String((BoardIDs / 1) % 10);

          String StringPlan = "";
          StringPlan += (PLAN / 1000) % 10;
          StringPlan += (PLAN / 100) % 10;
          StringPlan += (PLAN / 10) % 10;
          StringPlan += (PLAN / 1) % 10;

          String StringResult = "";
          StringResult += (RESULT / 1000) % 10;
          StringResult += (RESULT / 100) % 10;
          StringResult += (RESULT / 10) % 10;
          StringResult += (RESULT / 1) % 10;
          String State = "";
          if(NodeRun){State ="1" + String(WiFiMode);}else{State = "0" + String(WiFiMode);}
          String sentData = id + "04" + "18" + StringPlan + StringResult + State;
          #ifdef Mesh_Network 
            if(ComMode == MESH && Mode == 0){broadcast(sentData);}
          #endif//Mesh_Network   
          #ifdef RS485
            if(ComMode == RS485com){RS485_Ser.println(sentData);RS485_Ser.flush();}
          #endif//RS485    
          #ifdef USE_LORA 
            if(ComMode == LoRa){
              digitalWrite(M0, LOW);digitalWrite(M1, LOW);
              LoRa_Seri.println(sentData);LoRa_Seri.flush();
              if(Debug)LOGLN(sentData);
            }
          #endif//LoRa_Network  
          #ifdef MQTT_Mode
            if(ComMode == MQTT && countermqtt > 50){  
            for(int c = 0 ; c < sentData.length()+2;sentData.toCharArray(Buffer, c++));
            mqtt.publish(TopicIn,buffer);
            //client.publish(TopicOut,buffer);
            }
          #endif//MQTT_Mode
          // LOG("data Sent:");
          // if(Debug)LOGLN(buffer);
          if(Debug)LOGLN("sent to gateway");
          if(Debug)LOGLN("__________________________________");
        }//if(NodeID == BoardIDs)
        //payload [IDh,IDl,01,16,PlanH,PlanL,ResultH,ResultL,TimeH,TimeL,PlanSH,PlanSL,ResultSH,ResultSL,PassH,PassL,PCSH,PCSL,DOT]
      }//if (Length.toInt() == 12)
      
    }//(cmds.toInt() == cmdLookLine.updateStt)
    if(cmds.toInt() == cmdLookLine.setup)
    {
      if (Length.toInt() == 16){
        if(NodeID == BoardIDs){
           String StringPlan = "";
          StringPlan += (buffer[8] / 1000) % 10;
          StringPlan += (buffer[9] / 100) % 10;
          StringPlan += (buffer[10] / 10) % 10;
          StringPlan += (buffer[11] / 1) % 10;
           PLAN = StringPlan.toInt();
          String StringResult = "";
          StringResult += (buffer[12] / 1000) % 10;
          StringResult += (buffer[13] / 100) % 10;
          StringResult += (buffer[14] / 10) % 10;
          StringResult += (buffer[15] / 1) % 10; 
            RESULT = StringResult.toInt();
          String StringTime = "";
          StringTime += (buffer[16] / 1000) % 10;
          StringTime += (buffer[17] / 100) % 10;
          StringTime += (buffer[18] / 10) % 10;
          StringTime += (buffer[19] / 1) % 10; 
            int taskTime = StringTime.toInt();
          String PlanSets = "";
          PlanSets += (buffer[20] / 1000) % 10;
          PlanSets += (buffer[21] / 100) % 10;
          PlanSets += (buffer[22] / 10) % 10;
          PlanSets += (buffer[23] / 1) % 10; 
           int  taskPLanSet = PlanSets.toInt();
          String ResultSets = "";
          ResultSets += (buffer[24] / 1000) % 10;
          ResultSets += (buffer[25] / 100) % 10;
          ResultSets += (buffer[26] / 10) % 10;
          ResultSets += (buffer[27] / 1) % 10; 
           int  ResultSet = ResultSets.toInt();
          String StringPass = "";
          StringPass += (buffer[28] / 1000) % 10;
          StringPass += (buffer[29] / 100) % 10;
          StringPass += (buffer[30] / 10) % 10;
          StringPass += (buffer[31] / 1) % 10; 
           int  taskPass = StringPass.toInt();
          String PcsShift = "";
          PcsShift += (buffer[32] / 1000) % 10;
          PcsShift += (buffer[33] / 100) % 10;
          PcsShift += (buffer[34] / 10) % 10;
          PcsShift += (buffer[35] / 1) % 10; 
           int  taskpcsInShift = PcsShift.toInt();
          String StringDot = "";
          StringDot += (buffer[36] / 10) % 10;
          StringDot += (buffer[37] / 1) % 10;
           int  taskDotIn = StringDot.toInt();
          Lookline_PROG.SetParameter(PLAN, taskPLanSet, RESULT, ResultSet, taskTime, taskpcsInShift, taskPass, taskDotIn);
            // WriteAll();WriteRebootValue();
        }//if(NodeID == BoardIDs)
      }//if (Length.toInt() == 38)
    }//if(cmds.toInt() == cmdLookLine.setup)
    if (cmds.toInt() == cmdLookLine.updateFw && role == NODE && WiFiMode == true)//For gateway mode
    {
      // Mode = 3;
    }
    if (cmds.toInt() == cmdLookLine.request && role == GATEWAY)//For gateway mode
    {
      if (Length.toInt() == 0)
        {
          String id = "";
          id += String((IDSent / 1000) % 10);
          id += String((IDSent / 100) % 10);
          id += String((IDSent / 10) % 10);
          id += String((IDSent / 1) % 10);
          String sentData = id + "00" + "120000";
          #ifdef Mesh_Network 
            if(ComMode == MESH && Mode == 0) broadcast(sentData);
          #endif//Mesh_Network   
          #ifdef RS485
            if(ComMode == RS485com){RS485_Ser.println(sentData);RS485_Ser.flush();}
          #endif//RS485    
          #ifdef USE_LORA 
            if(ComMode == LoRa){digitalWrite(M0, LOW);digitalWrite(M1, LOW);
            LoRa_Seri.println(sentData);LoRa_Seri.flush();
            // if(Debug)LOGLN(sentData);
            setupEn = false;
            }
          #endif//LoRa_Network  
          #ifdef MQTT_Mode
            if(ComMode == MQTT && countermqtt > 50){  
            for(byte c = 0 ; c < sentData.length()+2;sentData.toCharArray(Buffer, c++));
            mqtt.publish(TopicOut,buffer);
            }
          #endif//MQTT_Mode
        } //if (Length.toInt() == 0)
      if (Length.toInt() >= 16)
      {
        CONFIG::read_byte(EP_EEPROM_MODULE_TYPE, &ModuleType);
        if(ModuleType == ModGateway){
        digitalWrite(taskStatus_LED, HIGH);delay(100);digitalWrite(taskStatus_LED, LOW);
        }//if(taskModuleType == ModGateway){
        String ID = String(buffer[0]) + String(buffer[1]) + String(buffer[2]) + String(buffer[3]); 
          if(ID.toInt() == IDSent){
            PC_Seri.println(String(buffer));counter5 = TimeSent;delay(300);counter5 = TimeSent - 2;
            String plan = String(buffer[8]) + String(buffer[9]) + String(buffer[10]) + String(buffer[11]);
            String result = String(buffer[12]) + String(buffer[13]) + String(buffer[14]) + String(buffer[15]);
            String Runs = String(buffer[16]);
            String Modes = String(buffer[17]);
            Data[0][IDSent] = plan.toInt();
            Data[1][IDSent] = result.toInt();
            Data[3][IDSent] = Runs.toInt();
            Data[4][IDSent] = Modes.toInt();
            //Data[3][IDSent] = 0;//taskLastTime[IDSent];
            taskLastTime[IDSent] = 0;
          }
      }//if (Length.toInt() == 16)      
    }//if (cmds.toInt() == cmdLookLine.request)
  }//Done = true
  
}

void LOOKLINE_PROG::SerDisplay()
{
      if(MonitorMode == Main){
        CONFIG::read_byte(EP_EEPROM_ID, &BoardIDs);
        CONFIG::read_byte(EP_EEPROM_NETID, &NetIDs);
        CONFIG::read_byte(EP_EEPROM_CHANELS, &Lora_CH);
        // CONFIG::read_byte(EP_EEPROM_RUN, &NodeRun);
        CONFIG::read_byte(EP_EEPROM_DEBUG, &Debug);
        CONFIG::read_byte(EP_EEPROM_AMOUNTNODE, &AmountNode);
        CONFIG::read_byte(EP_EEPROM_TIMESENT, &TimeSent);
        CONFIG::read_byte(EP_EEPROM_MODULE_TYPE, &ModuleType);
        CONFIG::read_byte(EP_EEPROM_ROLE, &role);
        CONFIG::read_buffer(EP_EEPROM_TIME_PLAN,(byte*) &Time, INTEGER_LENGTH);
        String Log = "";
        Log = "| Board ID: "+ String(BoardIDs);
        Log +=" | Network ID: "+ String(NetIDs);
        Log +=" | Chanel : "+ String(Lora_CH);
        Log += "| M0:" +  String(M0) + "(" + String(digitalRead(M0)) + ")| M1:" +  String(M1) +  "(" + String(digitalRead(M1)) + ")";
        if(role == NODE || role == REPEARTER){
          if(NodeRun){Log +="| Runing";}else{Log +="| Stopping";} 
          Log +=" | Time: "+ String(Time) + "| ";
          if(DispMode == Test){Log +="Disp: Test ";}
          if(DispMode == Setting_){Log +="Disp: Setting ";}
          if(DispMode == ConFi){Log +="Disp: ConFi ";}
          if(DispMode == Online){Log +="Disp: Online ";}
          if(DispMode == SLEEP){Log +="Disp: SLEEP ";}
          if(DispMode == CLEAR){Log +="Disp: Clear ";}
          Log +="|Plan: ";Log +=PLAN;
          Log +="|  Result: ";Log +=RESULT;
          Log +="|  O.T: ";Log +=CountOT_Hm;Log +="."; Log +=CountOT_Lm;   
          Log +="| Startus_LED pin:" + String(Startus_LED) + "(" + digitalRead(Startus_LED) + ")";
        } else{
          Log += "| Amount node: " + String(AmountNode);
        }
        Log +="  |Temp: ";
        Log +=(temprature_sens_read() - 32) / 1.8;
        Log +=" C |";
        if(Debug)LOGLN(Log);
        // ShowParameters();
      }
      // if(MonitorMode == 1){
      //   String strmoni = "";
      //   if(DisplayMode == Main){strmoni = "Disp:Main";}
      //   if(DisplayMode == Test){strmoni = "Disp: Test |";}
      //   if(DisplayMode == Setting){strmoni = "Disp: Setting |";}
      //   if(DisplayMode == ConFi){strmoni = "Disp: ConFi |";}
      //   if(DisplayMode == Online){strmoni = "Disp: Online |";}
      //   if(DisplayMode == SLEEP){strmoni = "Disp: SLEEP |";}
      //   if(DisplayMode == CLEAR){strmoni = "Disp: Clear |";}
      //   for(int c = 0 ; c < strmoni.length();strmoni.toCharArray(Buffer, c++));
      //   mqtt.publish("/TopicOut", Buffer );delay(100);
      //   strmoni =  "Plan:  " + String(Plan);
      //   strmoni += " |Result:  " + String(Result);
      //   strmoni += " |O.T:  " + String((CountOT_H *100)+ CountOT_L);
      //   strmoni += " |Temp: " + String((temprature_sens_read() - 32) / 1.8) + "*C"; 
      //   for(int c = 0 ; c < strmoni.length();strmoni.toCharArray(Buffer, c++));
      //   mqtt.publish("/TopicOut", Buffer );
  
      // }
}

void LOOKLINE_PROG::ConfigJsonProcess(String Input)
{ 
  /*
   int SetupForBegin = 0;
  long PlanLimit =  9999;
  long Plan =       0;
  long Result =     0;
  long ActionOT =   0;
  //long ResultCon = 0;
  long PLanSet =    1;//boi so Plan
  long ResultSet =  1;//boi so Result
  long pcsInShift = 1;//số sản phẩm chạy theo Plan
  long Time = 10;
  int DotIn = 0;
  bool NodeRun = true;
  bool lock = true;//IR Lock mode
  */
  DynamicJsonDocument doc(1500);
  deserializeJson(doc, Input);
  JsonObject obj = doc.as<JsonObject>();
  //{"Command":"Config","Run":1,"time":10,"Pla":1,"Res":0,"PSet":1,"RSet":1,"pass":0,"DotI":0,"PlL":9999,"PCS":10,"ID":1,"TS":10,"MM":0,"CMS":0,"QN":5,"GT":1,"STAN":1,"ILCH":410,"DFC":1000}

  if(obj["Command"].as<String>() == "Config"){ 
      if(Debug)LOGLN("Config Readding");
      Time = obj["time"].as<String>().toInt();
      PLanSet = obj["PSet"].as<String>().toInt();
      ResultSet = obj["RSet"].as<String>().toInt();
      Pass1 =  obj["pass"].as<String>().toInt();
      DotIn = obj["DotI"].as<String>().toInt();
      PlanLimit = obj["PlL"].as<String>().toInt();
      pcsInShift = obj["PCS"].as<String>().toInt(); 
      //BoardIDs = obj["ID"].as<String>().toInt();
      TimeSent = obj["TS"].as<String>().toInt();
      MonitorMode = obj["MM"].as<String>().toInt();
      ComMasterSlave = obj["CMS"].as<String>().toInt();
      AmountNode = obj["QN"].as<String>().toInt();
      GatewayTerminal = obj["GT"].as<String>().toInt();
      STAModeNormal = obj["STAN"].as<String>().toInt();
      Lora_CH = obj["ILCH"].as<String>().toInt();
      delayForCounter = obj["DFC"].as<String>().toInt();
      Pass3 =  obj["pass1"].as<String>().toInt();
  }// if(obj["Command"].as<String>() == "Config"){


}//void ConfigJsonProcess(String Input)


#endif//LOOKLINE_UI