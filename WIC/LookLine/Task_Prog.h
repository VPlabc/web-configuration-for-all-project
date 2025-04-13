#include "LookLine.h"
#include "7SegModule.h"
#include "config.h"
// #define Develop
#define LoRa_Ser Serial2

void Data_Proccess();
typedef struct TaskPin{
    uint8_t Data1;
    uint8_t Data2;
    uint8_t Data3;
    uint8_t SHCP;
    uint8_t STCP;
    uint8_t M0;
    uint8_t M1;
}TaskPin;
TaskPin taskPin;
int delayForCounter = 1000;
int taskPlan = 0;
int taskResult = 0;
int  taskCountOT_H = 0;
int  taskCountOT_L = 0;
bool TaskPinOnce = true;
byte taskBoardID = 0;
byte taskStatus_LED = 0;
byte taskRole = 0;
byte taskComMode = 0;
byte taskWiFiMode = 0;
byte taskAmountNode = 0 ;
byte taskModuleType = 0 ;
int taskIDSent = 0;
bool taskRun = 0;
byte taskTimeSent = 0;
int16_t counter5 = 0;
  int taskData[5][100];
  int taskLastTime[100];
  int taskMaxTime[100];
///////////////////////////// LoRa ////////////////////////////
//TimerFlag FlagLora[2];
  int taskADDh = 0;
  int taskADDl = taskBoardID;
  int taskNetID = 1;
  int parameter[17] = {taskADDh, taskADDl, taskNetID, 0x61, 0x03, 0x49, 0x4f};
  byte taskChanel = 0;

void SetPin(uint8_t Data1, uint8_t Data2, uint8_t Data3, uint8_t SHCP , uint8_t STCP, byte ID, byte NetID, byte Int_Lora_CH, byte Status_LED, byte TimeSent)
{
    taskPin.Data1 = Data1;
    taskPin.Data2 = Data2;
    taskPin.Data3 = Data3;
    taskPin.STCP = STCP;
    taskPin.SHCP = SHCP;
    taskBoardID = ID;
    taskNetID = NetID;
    taskChanel = Int_Lora_CH;
    taskStatus_LED = Status_LED;
    taskTimeSent = TimeSent;
    pinMode(Status_LED, OUTPUT);
    LOG("LoRa Chanel:" + String(taskChanel) + "|");
    CONFIG::read_byte(EP_EEPROM_ROLE, &taskRole);
    CONFIG::read_byte(EP_EEPROM_COM_MODE, &taskComMode);
    CONFIG::read_byte(EP_WIFI_MODE, &taskWiFiMode);
    CONFIG::read_byte (EP_EEPROM_AMOUNTNODE , &taskAmountNode);
    CONFIG::read_byte (EP_EEPROM_MODULE_TYPE , &taskModuleType);
    CONFIG::read_buffer (EP_EEPROM_COUNTER_DELAY,  (byte *) &delayForCounter, INTEGER_LENGTH);
    
}
void IncNodeTime()//For Gateway
{
      if(taskRole == GATEWAY && taskWiFiMode == 1){ counter5++;
      for(byte i = 0 ; i < taskAmountNode; i++){
        taskLastTime[i]++;if(taskLastTime[i] > 10000)taskLastTime[i] = 10000;
        taskData[2][i] = taskLastTime[i]/10;
        if(taskMaxTime[i] < taskLastTime[i]){taskMaxTime[i] =taskLastTime[i];}
        }
      }
      for(byte node = 0 ; node < 100 ; node++){taskData[3][node]++;}
}
void SetValue(int Plan_V, int Result_V, int OTh_V, int OTl_V, bool Run)
{
  taskPlan = Plan_V;
  taskResult = Result_V;
  taskCountOT_H = OTh_V;
  taskCountOT_L = OTl_V;
  taskRun = Run;
  // if (role == NODE || role == REPEARTER ){LOGLN("SST LED:" + String(Run));
  //   digitalWrite(taskStatus_LED, taskRun);
  // }
  CONFIG::read_buffer (EP_EEPROM_COUNTER_DELAY,  (byte *) &delayForCounter, INTEGER_LENGTH);

}
void TaskDisplay(byte mode)
{
  int TIMES = 10000;
#ifdef AutoRunStop
  DateTime now = rtc.now();
  /*
      LOGLN("Time:");
      LOG(now.hour(), DEC);
      LOG(':');F
      LOG(now.minute(), DEC);
      LOG(':');
      LOG(now.second(), DEC);
      LOGLN();
      //*/

  for (int j = 0; j < 10; j++)
  {
    if (now.hour() == WorkTime[j][0] & now.minute() == WorkTime[j][1] & now.second() == WorkTime[j][2])
    {
      Lookline_PROG.GetRun() = false;
    }
    if (now.hour() == WorkTime[j][3] & now.minute() == WorkTime[j][4] & now.second() == WorkTime[j][5])
    {
      Lookline_PROG.GetRun() = true;
    }
  }
#endif //ifdef AutoLookline_PROG.RunStop
#ifdef TEST_MODE
  if (Lookline_PROG.GetRun() == true & Lookline_PROG.DispMode == Main)
  {
    digitalWrite(Lookline_PROG.Startus_LED, LOW);
  }
  if (Lookline_PROG.GetRun() == false & Lookline_PROG.DispMode == Main)
  {
    digitalWrite(Lookline_PROG.Startus_LED, HIGH);
  }
  if (Lookline_PROG.DispMode != Main)
  {
    digitalWrite(Lookline_PROG.Startus_LED, HIGH);
  }
#endif                 //TEST_MODE \
                       ///*
  switch (mode) //0 main display||1 test||2 setting||3 ConFi||4 Read ID||
  {
  //////////////////////////////////////////////////// MAIN DISPLAY
  case Main:
    ////////////////////////////// 29/8/19 nhan giu de reset /////////////////////////
    Lookline_PROG.counter1++;
    if (Lookline_PROG.counter1 > TIMES)
    {
      Lookline_PROG.counter1 = 0;
      Lookline_PROG.counterOUT++;
    }
    if (Lookline_PROG.counterOUT > 1)
    {
      Lookline_PROG.counterOUT = 0;
      Lookline_PROG.countReset = 0;
      Lookline_PROG.countUPD = 0;
    }
    /////////////////////////////////////////////////////////////////
        // CONFIG::read_buffer(EP_EEPROM_PLAN,(byte *) &taskPlan, INTEGER_LENGTH);
        // CONFIG::read_buffer(EP_EEPROM_RESULT,(byte *) &taskResult, INTEGER_LENGTH);
        // if(TaskPinOnce){TaskPinOnce = false;
        //     LOGLN("Data1--" + String(taskPin.Data1));
        //     LOGLN("Data2--" + String(taskPin.Data2));
        //     LOGLN("Data3--" + String(taskPin.Data3));
        //     LOGLN("SHCP--" + String(taskPin.SHCP));
        //     LOGLN("STCP--" + String(taskPin.STCP));
        // }
        
    displays(taskPlan, taskResult, taskCountOT_H, taskCountOT_L, taskPin.Data1, taskPin.Data2, taskPin.Data3, taskPin.SHCP, taskPin.STCP, 0);
    //displays(Lookline_PROG.Result, 4, Lookline_PROG.DATA2, Lookline_PROG.SHCP, taskPin.STCP, 0);
    //displayOT();
    break;
  //////////////////////////////////////////////////// TEST DISPLAY
  case Test:
    Lookline_PROG.counter1++;
    if (Lookline_PROG.counter1 > TIMES)
    {
      Lookline_PROG.counter1 = 0;
      Lookline_PROG.counterOUT++;
    }
    if (Lookline_PROG.counterOUT > 30)
    {
      Lookline_PROG.counterOUT = 0;
      DispMode = 0;
    }
    if (Lookline_PROG.numCode < 10)
    {
      TestDisplay(Lookline_PROG.numCode);
    }
    break;
  //////////////////////////////////////////////////// SETING DISPLAY
  case Setting_:
    //if(Lookline_PROG.SetupPro)

    digitalWrite(taskPin.STCP, LOW);
    Lookline_PROG.counter1++;
    if (Lookline_PROG.counter1 > TIMES)
    {
      Lookline_PROG.counter1 = 0;
      Lookline_PROG.counterOUT++;
    }
    if (Lookline_PROG.counterOUT > 30)
    {
      Lookline_PROG.counterOUT = 0;
      DispMode = 0;
    }
    if (Lookline_PROG.numCode < 10)
    {
      if (Lookline_PROG.SetupStep == 0)
      { //nothing
        Lookline_PROG.ValueSet0 = Lookline_PROG.numCode;
      }
      if (Lookline_PROG.SetupStep == 1)
      { //nothing
        Lookline_PROG.ValueSet1 = Lookline_PROG.numCode;
      }
      if (Lookline_PROG.SetupStep == 2)
      { //nothing
        Lookline_PROG.ValueSet2 = Lookline_PROG.numCode;
      }
      if (Lookline_PROG.SetupStep == 3)
      { //nothing
        Lookline_PROG.ValueSet3 = Lookline_PROG.numCode;
      }

      Lookline_PROG.numCode = 10;
      Lookline_PROG.SetupStep++;
      if (Lookline_PROG.SetupStep > 3)
        Lookline_PROG.SetupStep = 0;
    }

    if (Lookline_PROG.SetupPro == 4)
    {
      if (Lookline_PROG.SetupStep == 0 && Lookline_PROG.counter1 > TIMES / 2)
      {
        PrintSeg(ConvertToLed7Segment('P'), Seg[10], ConvertToLed7Segment('-'));
      }
      else
      {
        PrintSeg(ConvertToLed7Segment('P'), ConvertToLed7Segment('-'), ConvertToLed7Segment('-'));
      }
      if (Lookline_PROG.SetupStep == 1 && Lookline_PROG.counter1 > TIMES / 2)
      {
        PrintSeg(ConvertToLed7Segment('r'), Seg[10], ConvertToLed7Segment('-'));
      }
      else
      {
        PrintSeg(ConvertToLed7Segment('r'), ConvertToLed7Segment('-'), ConvertToLed7Segment('-'));
      }
      if (Lookline_PROG.SetupStep == 2 && Lookline_PROG.counter1 > TIMES / 2)
      {
        PrintSeg(ConvertToLed7Segment('o'), Seg[10], ConvertToLed7Segment('-'));
      }
      else
      {
        PrintSeg(ConvertToLed7Segment('o'), ConvertToLed7Segment('-'), ConvertToLed7Segment('-'));
      }
      if (Lookline_PROG.SetupStep == 3 && Lookline_PROG.counter1 > TIMES / 2)
      {
        PrintSeg(ConvertToLed7Segment(Lookline_PROG.SetupPro % 10), Seg[10], ConvertToLed7Segment('-'));
      }
      else
      {
        PrintSeg(ConvertToLed7Segment(Lookline_PROG.SetupPro % 10), ConvertToLed7Segment('-'), ConvertToLed7Segment('-'));
      }
    }
    if (Lookline_PROG.SetupPro != 4)
    {
      if (Lookline_PROG.SetupStep == 0 && Lookline_PROG.counter1 > TIMES / 2)
      {
        PrintSeg(ConvertToLed7Segment('P'), Seg[10], ConvertToLed7Segment('-'));
      }
      else
      {
        if (Lookline_PROG.DotIn == 3 && Lookline_PROG.SetupPro == 6)
        {
          PrintSeg(ConvertToLed7Segment('P'), ConvertToLed7Segment(Lookline_PROG.ValueSet0) | SEGMENT_H, ConvertToLed7Segment('-'));
        }
        else
        {
          PrintSeg(ConvertToLed7Segment('P'), Seg[Lookline_PROG.ValueSet0], ConvertToLed7Segment('-'));
        }
      }
      if (Lookline_PROG.SetupStep == 1 && Lookline_PROG.counter1 > TIMES / 2)
      {
        PrintSeg(ConvertToLed7Segment('r'), Seg[10], ConvertToLed7Segment('-'));
      }
      else
      {
        if (Lookline_PROG.DotIn == 2 && Lookline_PROG.SetupPro == 6)
        {
          PrintSeg(ConvertToLed7Segment('r'), ConvertToLed7Segment(Lookline_PROG.ValueSet1) | SEGMENT_H, ConvertToLed7Segment('-'));
        }
        else
        {
          PrintSeg(ConvertToLed7Segment('r'), Seg[Lookline_PROG.ValueSet1], ConvertToLed7Segment('-'));
        }
      }
      if (Lookline_PROG.SetupStep == 2 && Lookline_PROG.counter1 > TIMES / 2)
      {
        PrintSeg(ConvertToLed7Segment('o'), Seg[10], ConvertToLed7Segment('-'));
      }
      else
      {
        if (Lookline_PROG.DotIn == 1 && Lookline_PROG.SetupPro == 6)
        {
          PrintSeg(ConvertToLed7Segment('o'), ConvertToLed7Segment(Lookline_PROG.ValueSet2) | SEGMENT_H, ConvertToLed7Segment('-'));
        }
        else
        {
          PrintSeg(ConvertToLed7Segment('o'), Seg[Lookline_PROG.ValueSet2], ConvertToLed7Segment('-'));
        }
      }
      if (Lookline_PROG.SetupStep == 3 && Lookline_PROG.counter1 > TIMES / 2)
      {
        PrintSeg(ConvertToLed7Segment(Lookline_PROG.SetupPro % 10), Seg[10], ConvertToLed7Segment('-'));
      }
      else
      {
        if (Lookline_PROG.DotIn == 0 && Lookline_PROG.SetupPro == 6)
        {
          PrintSeg(ConvertToLed7Segment(Lookline_PROG.SetupPro % 10), ConvertToLed7Segment(Lookline_PROG.ValueSet3) | SEGMENT_H, ConvertToLed7Segment('-'));
        }
        else
        {
          PrintSeg(ConvertToLed7Segment(Lookline_PROG.SetupPro % 10), Seg[Lookline_PROG.ValueSet3], ConvertToLed7Segment('-'));
        }
      }
    } //if(Lookline_PROG.SetupPro == 4){

    /*
    if(Lookline_PROG.SetupPro == 10){
    shiftOut(Lookline_PROG.DATA1, Lookline_PROG.SHCP, LSBFIRST, ConvertToLed7Segment('e'));
    shiftOut(Lookline_PROG.DATA1, Lookline_PROG.SHCP, LSBFIRST, ConvertToLed7Segment('M'));
    shiftOut(Lookline_PROG.DATA1, Lookline_PROG.SHCP, LSBFIRST, ConvertToLed7Segment('i'));
    shiftOut(Lookline_PROG.DATA1, Lookline_PROG.SHCP, LSBFIRST, ConvertToLed7Segment('t'));
    }
    else{
    shiftOut(Lookline_PROG.DATA1, Lookline_PROG.SHCP, LSBFIRST, ConvertToLed7Segment(Lookline_PROG.SetupPro%10));
    shiftOut(Lookline_PROG.DATA1, Lookline_PROG.SHCP, LSBFIRST, ConvertToLed7Segment('o'));
    shiftOut(Lookline_PROG.DATA1, Lookline_PROG.SHCP, LSBFIRST, ConvertToLed7Segment('r'));
    shiftOut(Lookline_PROG.DATA1, Lookline_PROG.SHCP, LSBFIRST, ConvertToLed7Segment('P'));
    }
    if(Lookline_PROG.SetupPro == 4)
    {
      if(Lookline_PROG.SetupStep == 3 &&Lookline_PROG.counter1 > TIMES/2){shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, Seg[10]);}
      else{shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, ConvertToLed7Segment('-'));}
      if(Lookline_PROG.SetupStep == 2 &&Lookline_PROG.counter1 > TIMES/2){shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, Seg[10]);}
      else{shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, ConvertToLed7Segment('-'));}
      if(Lookline_PROG.SetupStep == 1 &&Lookline_PROG.counter1 > TIMES/2){shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, Seg[10]);}
      else{shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, ConvertToLed7Segment('-'));}
      if(Lookline_PROG.SetupStep == 0 &&Lookline_PROG.counter1 > TIMES/2){shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, Seg[10]);}
      else{shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, ConvertToLed7Segment('-'));}
    }
    else
    {
      if(Lookline_PROG.SetupStep == 3 &&Lookline_PROG.counter1 > TIMES/2){shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, Seg[10]);}
      else{if(Lookline_PROG.SetupPro == 6 & Lookline_PROG.DotIn == 4){shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, (ConvertToLed7Segment(Lookline_PROG.ValueSet3) | SEGMENT_H));}}//else
      if(Lookline_PROG.SetupStep == 2 &&Lookline_PROG.counter1 > TIMES/2){shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, Seg[10]);}
      else{if(Lookline_PROG.SetupPro == 6 & Lookline_PROG.DotIn == 1){shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, (ConvertToLed7Segment(Lookline_PROG.ValueSet2) | SEGMENT_H));}}//else
      if(Lookline_PROG.SetupStep == 1 &&Lookline_PROG.counter1 > TIMES/2){shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, Seg[10]);}
      else{if(Lookline_PROG.SetupPro == 6 & Lookline_PROG.DotIn == 2){shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, (ConvertToLed7Segment(Lookline_PROG.ValueSet1) | SEGMENT_H));}}//else
      if(Lookline_PROG.SetupStep == 0 &&Lookline_PROG.counter1 > TIMES/2){shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, Seg[10]);}
      else{if(Lookline_PROG.SetupPro == 6 & Lookline_PROG.DotIn == 3){shiftOut(Lookline_PROG.DATA2, Lookline_PROG.SHCP, LSBFIRST, (ConvertToLed7Segment(Lookline_PROG.ValueSet0) | SEGMENT_H));}}//else
      
    }
    */
    latch();
    break;
  /////////////////////////////////////////////////// SET ID DISPLAY
  case ConFi:

    digitalWrite(taskPin.STCP, LOW);
  if(taskModuleType == ModuleV140 || taskModuleType == ModuleV130){
    PrintSeg(Seg[10], Seg[10], Seg[10]);
      PrintSeg(ConvertToLed7Segment('C'), ConvertToLed7Segment('F'), Seg[10]);
      PrintSeg(ConvertToLed7Segment('o'), ConvertToLed7Segment('i'), Seg[10]);
      PrintSeg(ConvertToLed7Segment('N'), Seg[10], Seg[10]);

  }//if(taskModuleType == ModuleV140){

  if(taskModuleType == ModuleV141){
    PrintSeg(Seg[10], Seg[10], Seg[10]);
      PrintSeg(ConvertToLed7Segment('N'), ConvertToLed7Segment('i'), Seg[10]);
      PrintSeg(ConvertToLed7Segment('o'), ConvertToLed7Segment('F'), Seg[10]);
      PrintSeg(ConvertToLed7Segment('C'), Seg[10], Seg[10]);
  }//if(taskModuleType == ModuleV141){  
    latch();
    break;
  /////////////////////////////////////////////////// SET ID DISPLAY
  case CLEAR:

    digitalWrite(taskPin.STCP, LOW);
  if(taskModuleType == ModuleV140 || taskModuleType == ModuleV130){   
    PrintSeg(Seg[10], Seg[10], Seg[10]);
      PrintSeg(ConvertToLed7Segment('C'), ConvertToLed7Segment('r'), Seg[Lookline_PROG.ValueSet2]);
      PrintSeg(ConvertToLed7Segment('L'), ConvertToLed7Segment('i'), Seg[Lookline_PROG.ValueSet1]);
      PrintSeg(ConvertToLed7Segment('E'), Seg[10], Seg[Lookline_PROG.ValueSet0]);
      PrintSeg(ConvertToLed7Segment('A'), Seg[10], Seg[10]);
  }///if(taskModuleType == ModuleV140){
  if(taskModuleType == ModuleV141){
    PrintSeg(Seg[10], Seg[10], Seg[10]);
      PrintSeg(ConvertToLed7Segment('A'), ConvertToLed7Segment('r'), Seg[10]);
      PrintSeg(ConvertToLed7Segment('E'), ConvertToLed7Segment('-'), Seg[10]);
      PrintSeg(ConvertToLed7Segment('L'), Seg[10], Seg[10]);
      PrintSeg(ConvertToLed7Segment('C'), Seg[10], Seg[10]);
  }//if(taskModuleType == ModuleV141){ 
    latch();
    break;  
    /////////////////////////////////////////////////// UPDATE
  case UPDATE:

    digitalWrite(taskPin.STCP, LOW);
  if(taskModuleType == ModuleV140 || taskModuleType == ModuleV130){   
    //   PrintSeg(ConvertToLed7Segment('U'), ConvertToLed7Segment('T'), Seg[Lookline_PROG.ValueSet2]);
    //   PrintSeg(ConvertToLed7Segment('P'), ConvertToLed7Segment('E'), Seg[Lookline_PROG.ValueSet1]);
    //   PrintSeg(ConvertToLed7Segment('D'), Seg[10], Seg[Lookline_PROG.ValueSet0]);
    //   PrintSeg(ConvertToLed7Segment('A'), Seg[10], Seg[10]);
    // PrintSeg(Seg[10], Seg[10], Seg[10]);
  }///if(taskModuleType == ModuleV140){
      PrintSeg(Seg[10], Seg[10], Seg[10]);
      PrintSeg(ConvertToLed7Segment('A'), ConvertToLed7Segment('E'), Seg[10]);
      PrintSeg(ConvertToLed7Segment('D'), ConvertToLed7Segment('T'), Seg[10]);
      PrintSeg(ConvertToLed7Segment('P'), Seg[10], Seg[10]);
      PrintSeg(ConvertToLed7Segment('U'), Seg[10], Seg[10]);
  if(taskModuleType == ModuleV141){
    PrintSeg(Seg[10], Seg[10], Seg[10]);
      PrintSeg(ConvertToLed7Segment('A'), ConvertToLed7Segment('E'), Seg[10]);
      PrintSeg(ConvertToLed7Segment('D'), ConvertToLed7Segment('T'), Seg[10]);
      PrintSeg(ConvertToLed7Segment('P'), Seg[10], Seg[10]);
      PrintSeg(ConvertToLed7Segment('U'), Seg[10], Seg[10]);
  }//if(taskModuleType == ModuleV141){ 
    latch();
    break;
    ////////////////////////////////////////////////////////////////////////////////////////////
  case Online:
 
  if(taskModuleType == ModuleV140 || taskModuleType == ModuleV130){  
    PrintSeg(ConvertToLed7Segment('O'), Seg[10], Seg[10]);
    if (Lookline_PROG.SetupStep == 0 && Lookline_PROG.counter1 > TIMES / 2)
    {
      PrintSeg(ConvertToLed7Segment('n'), ConvertToLed7Segment('N'), Seg[10]);
    }
    else
    {
      PrintSeg(ConvertToLed7Segment('n'), ConvertToLed7Segment('N'), Seg[Lookline_PROG.ValueSet2]);
    }
    if (Lookline_PROG.SetupStep == 1 && Lookline_PROG.counter1 > TIMES / 2)
    {
      PrintSeg(ConvertToLed7Segment('L'), ConvertToLed7Segment('e'), Seg[10]);
    }
    else
    {
      PrintSeg(ConvertToLed7Segment('L'), ConvertToLed7Segment('e'), Seg[Lookline_PROG.ValueSet1]);
    }
    if (Lookline_PROG.SetupStep == 2 && Lookline_PROG.counter1 > TIMES / 2)
    {
      PrintSeg(ConvertToLed7Segment('i'), Seg[10], Seg[10]);
    }
    else
    {
      PrintSeg(ConvertToLed7Segment('i'), Seg[10], Seg[Lookline_PROG.ValueSet0]);
    }
  }///if(taskModuleType == ModuleV140){
  if(taskModuleType == ModuleV141){
    PrintSeg(ConvertToLed7Segment('i'), Seg[10], Seg[10]);
    if (Lookline_PROG.SetupStep == 0 && Lookline_PROG.counter1 > TIMES / 2)
    {
      PrintSeg(ConvertToLed7Segment('i'), ConvertToLed7Segment('e'), Seg[10]);
    }
    else
    {
      PrintSeg(ConvertToLed7Segment('L'), ConvertToLed7Segment('e'), Seg[Lookline_PROG.ValueSet2]);
    }
    if (Lookline_PROG.SetupStep == 1 && Lookline_PROG.counter1 > TIMES / 2)
    {
      PrintSeg(ConvertToLed7Segment('n'), ConvertToLed7Segment('N'), Seg[10]);
    }
    else
    {
      PrintSeg(ConvertToLed7Segment('n'), ConvertToLed7Segment('N'), Seg[Lookline_PROG.ValueSet1]);
    }
    if (Lookline_PROG.SetupStep == 2 && Lookline_PROG.counter1 > TIMES / 2)
    {
      PrintSeg(ConvertToLed7Segment('O'), Seg[10], Seg[10]);
    }
    else
    {
      PrintSeg(ConvertToLed7Segment('O'), Seg[10], Seg[Lookline_PROG.ValueSet0]);
    }
  }//if(taskModuleType == ModuleV141){ 
    latch();
    break;
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  case SLEEP:
    PrintSeg(Seg[10], Seg[10], Seg[10]);
    PrintSeg(Seg[10], Seg[10], Seg[10]);
    PrintSeg(Seg[10], Seg[10], Seg[10]);
    PrintSeg(Seg[10], Seg[10], Seg[10]);
    latch();
    break;
    //////////////////////////////////////////////////////////////////////////////////////////////////////
  } //switch
  //*/
}



#ifdef LoRaNetwork
  if (oldStage0 != digitalRead(SW_Mode0) || oldStage1 != digitalRead(SW_Mode1))
  {
    locks = false;
  }

  ///*
  if (digitalRead(SW_Mode0) == HIGH && digitalRead(SW_Mode1) == HIGH)
  {
    oldStage0 = digitalRead(SW_Mode0);
    oldStage1 = digitalRead(SW_Mode1);
    if (locks == false)
    {
      LOGLN();
      LOGLN("LoRa mode: Normal");
      LOGLN();
      locks = true;
    } // print notication
    digitalWrite(taskPin.M0, LOW);
    digitalWrite(taskPin.M1, LOW);
    //IDh,IDl,Cmd,Result_h,Result_l
    while (LoRa_Debug_Ser.available())
    {
      inChar = (char)LoRa_Ser.read();

      //if (inChar == '\n') {
      if (setupEn == false)
      {
        LoRadata[countSer - 1] = (char)inChar;
        LoRaInput += (char)inChar;
        countSer++;
        if (countSer > 50)
        {
          setupEn = true;
          inChar = 0;
          countSer = 0;
        }
      }
      //if(setupEn == false){
      //if(setupEn){JsonProcess(dataIn);dataIn = "";setupEn = false;}//setupEn
      yield();
    }
    //Serial availible
    if (inChar == '\n')
    {
      //LOGLN();
      //LOG("recieve:");LOGLN(LoRaInput);
      //LOG("recieve:");LOGLN(LORADATA.Encodebyte(LoRaIn[0]-48,LoRaIn[1]-48));
      setupEn = true;
      //  ID cmd leg plan result
      // 0001,00,00,0000,0000

      String ids = "";
      ids += LoRadata[-1];
      ids += LoRadata[0];
      ids += LoRadata[1];
      ids += LoRadata[2];
      int Id = ids.toInt();
      //LOG("id:");LOGLN(Id);
      
      if (BoardID == Id)
      {
        LOGLN("______________________________");
        String cmds = String(LoRadata[4]) + String(LoRadata[5]);
        //LOG("cmd:");LOGLN(cmds.toInt());

        if (cmds.toInt() == COMMAND.updateStt)
        {
          String id = "";
          id += String((BoardID / 1000) % 10);
          id += String((BoardID / 100) % 10);
          id += String((BoardID / 10) % 10);
          id += String((BoardID / 1) % 10);

          String StringPlan = "";
          StringPlan += (taskPlan / 1000) % 10;
          StringPlan += (taskPlan / 100) % 10;
          StringPlan += (taskPlan / 10) % 10;
          StringPlan += (taskPlan / 1) % 10;

          String StringResult = "";
          StringResult += (taskResult / 1000) % 10;
          StringResult += (taskResult / 100) % 10;
          StringResult += (taskResult / 10) % 10;
          StringResult += (taskResult / 1) % 10;
          String sentData = id + "00" + "16" + StringPlan + StringResult;
          LoRa_LOGLN(sentData);
          LOG("data Sent:");
          LOGLN(sentData);
          LoRa_LOGLN();
          LOGLN("sent to gateway");
          LOGLN("__________________________________");
        }
        //payload [IDh,IDl,Cmd,16,PlanH,PlanL,ResultH,ResultL,TimeH,TimeL,PlanSH,PlanSL,ResultSH,ResultSL,PassH,PassL,PCSH,PCSL,DOTH,DOTL]
      }
      inChar = 0;
      countSer = 0;
      setupEn = false;
      LoRaInput = "";
    } //if (inChar == '\n') {
    //*/
  }
  //while(digitalRead(SW_Mode0) == LOW && digitalRead(SW_Mode1) == HIGH){

  if (digitalRead(SW_Mode0) == LOW && digitalRead(SW_Mode1) == HIGH)
  {
    oldStage0 = digitalRead(SW_Mode0);
    oldStage1 = digitalRead(SW_Mode1);
    if (locks == false)
    {
      LOGLN();
      LOGLN("LoRa mode: Config");
      LOGLN();
      locks = true;
    } // print notication
    digitalWrite(taskPin.M0, HIGH);
    digitalWrite(taskPin.M1, HIGH);

    while (LoRa_Ser.available())
    {
      Debug_Ser.write((byte)LoRa_Debug_Ser.read());
    }
    while (Debug_Ser.available())
    {
      LoRa_Debug_Ser.write((byte)Debug_Ser.read());
    }
  }
  //while(digitalRead(SW_Mode0) == LOW && digitalRead(SW_Mode1) == HIGH){

  if (digitalRead(SW_Mode0) == HIGH && digitalRead(SW_Mode1) == LOW)
  {
    oldStage0 = digitalRead(SW_Mode0);
    oldStage1 = digitalRead(SW_Mode1);
    if (locks == false)
    {
      LOGLN();
      LOGLN("LoRa mode: debug");
      LOGLN();
      locks = true;
    } // print notication
    digitalWrite(taskPin.M0, LOW);
    digitalWrite(taskPin.M1, LOW);
    while (LoRa_Ser.available())
    {
      Serial.write((byte)LoRa_Ser.read());
    }
    while (Debug_Ser.available())
    {
      LoRa_Ser.write((byte)Serial.read());
    }
  }

  if (oldStage0 != digitalRead(SW_Mode0) || oldStage1 != digitalRead(SW_Mode1))
  {
    locks = false;
  }
  //while(digitalRead(SW_Mode0) == HIGH && digitalRead(SW_Mode1) == LOW){
#endif//LoRaNetwork
// } //LoRaCommu


void MeshCommu()
{
  #ifdef Mesh_Network
  if(taskComMode == MESH){
  MeshLoop();
  }
  #endif//def Mesh_Network
}


void RS485COM()
{
#ifdef RS485
if (RS485_Ser.available())
{    
  inChar = (char)RS485_Ser.read();
  //LOG(inChar);
      //if (inChar == '\n') {
      if (setupEn == false)
      {
       taskbuffer[countSer] = (char)inChar;
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
      done = true;
      setupEn = true;
      String ids = "";
      ids +=taskbuffer[0];
      ids +=taskbuffer[1];
      ids +=taskbuffer[2];
      ids +=taskbuffer[3];
      Id = ids.toInt();
      Data_Proccess();
      RS485_Ser.flush();
      inChar = 0;
      countSer = 0;
      setupEn = false;
    }
}
      
#endif//RS485
}
void MODBUS_Write(){
  #ifdef ModbusSlave
  holdingRegs[BoardID*2] = Plan;
  holdingRegs[BoardID*2+1] = Result;
  #endif//ModbusSlave
}

int16_t counter4 = 0;
void gatewayProtocol()
{
  //Send first data
        counter4++;

      if (counter4 > 900000)
      {
        counter4 = 0;
        LOGLN("Send");
      }
}




int counter = 0;//boot hardware
int counterHold = 0;//boot hardware
int counter2Pin = 0;// 2 pin resetting
int minValue = 500;
int maxValue = 2000;
int p = 0;
int o = 0;
bool LogOnce = true;
void TaskInPut()
{
  /*
  if(digitalRead(0) == 0){
    while(digitalRead(0) == 0){delay(10);
      counter++;if(counter > 1000){
      Mode = 1;
      WriteAll();
      LOG("upload mode");
      delay(3000);
      ESP.restart();
      }
    }
    counter = 0;
    Mode = 3;WriteAll();LOG("Turn ON AP Config");delay(3000);ESP.restart();
  }
  //*/
#ifndef Develop
#ifndef Gateway

if(taskRole == NODE){
  ///*
  //////////////////////////////////////////////////////////////////////
#ifdef USE_X0_X4  
  if (analogRead(X0) > maxValue && analogRead(X1) > maxValue && analogRead(X2) > maxValue && analogRead(X3) > maxValue && analogRead(X4) > maxValue){LogOnce = true;}
  if (analogRead(X0) > maxValue || analogRead(X4) > maxValue)
  {
    // LOGLN("update InputState");
    p++;if(p > 6000)p=6000;
    if (p == delayForCounter/3)
    { 

    }
  }
  else{
      if(LogOnce){LogOnce = false;LOGLN("X0:" + String(analogRead(X0)) + "|X4:" + String(analogRead(X4)));}
  }
  if (analogRead(X0) <= minValue || analogRead(X4) <= minValue)
  {
      while(analogRead(X0) <= minValue || analogRead(X4) <= minValue){delay(10);
      // if(LogOnce){LogOnce = false;LOGLN("X0:" + String(analogRead(X0)) + "|X4:" + String(analogRead(X4)));}
          counter2Pin++;
        if(counter2Pin > 700){//5S
          DispMode = Main;//Confi 
          TaskDisplay(DispMode);
          // TaskDisplay();
          // TaskSerial();
          // TaskServerpro();
          // Gateway = true;taskModuleType = ModGateway;
          // WriteRebootValue();
          // delay(1000);
          // ESP.restart();
          break;
        }
        if(counter2Pin > 500 && counter2Pin < 700){//5S
          DispMode = ConFi;//Confi 
          TaskDisplay(DispMode);
          }
        if(counter2Pin > 300 && counter2Pin < 500 && Lookline_PROG.GetRun() == false){//3S
          DispMode = CLEAR;//Clear 
          TaskDisplay(DispMode);
        }
      }
        if(counter2Pin > 300 && counter2Pin < 500 && Lookline_PROG.GetRun() == false){//3S
            Lookline_PROG.SetPlan(0);Lookline_PROG.SetResult(0);counter2Pin = 0;
            DispMode = Main;//Confi 
        }
        if(counter2Pin > 500 && counter2Pin < 700){//5S
          DispMode = ConFi;//Confi 
          //LOGLN("online");UDF = true;WCF = true;APC = true;
          // Lookline_PROG.Mode = 2;WriteRebootValue();LOG("AP Config");delay(3000);ESP.restart();
          }
          if(counter2Pin > 300 && counter2Pin < 500 && Lookline_PROG.GetRun() == false){//3S
            Lookline_PROG.SetPlan(0);Lookline_PROG.SetResult(0);
          }
        if(counter2Pin < 500){// under 5S
        if(Lookline_PROG.GetRun() == 1){
          Lookline_PROG.SetRun(0);Lookline_PROG.sendDataLookline();counter2Pin = 0;
          // WriteValue();
#ifdef DEBUG_
          LOGLN("Stop");
          delay(500);
#endif //#if DEBUG_
        }
        else{
          Lookline_PROG.SetRun(1);Lookline_PROG.sendDataLookline();counter2Pin = 0;
          // totalInterruptCounter = 0;
          // WriteValue();
#ifdef DEBUG_
          LOGLN("Run");
          delay(500);
#endif //#if DEBUG_
        }
          // if(Lookline_PROG.Mode == 1){Lookline_PROG.Mode = 0;WriteRebootValue();LOG("Cancel");delay(1000);ESP.restart();}
      }
    if (p > delayForCounter/3)
    {
      p = 0;counter2Pin = 0;
#ifdef DEBUG_
      LOGLN("X0");
#endif //#if DEBUG_
    }  //if(o > delayForCounter/3){
  }
#endif//USE_X0_X4
  //////////////////////////////////////////////////////////////////////
  
  //////////////////////////////////////////////////////////////////////
  if (analogRead(X1) > maxValue && analogRead(X2) > maxValue && analogRead(X3) > maxValue)
  {
    o++;if(o > 6000)o=6000;
    if (o == delayForCounter/3)
    {
      int taskResultSet = 0;
      CONFIG::read_buffer(EP_EEPROM_RESULT_SET,(byte *) &taskResultSet, INTEGER_LENGTH);
      taskResult = taskResult + taskResultSet;Lookline_PROG.SetResult(taskResult);
      if(taskResult % 10 == 9)CONFIG::write_buffer(EP_EEPROM_RESULT,(byte *) &taskResult, INTEGER_LENGTH);
    }
  }
  else{
      if(LogOnce){LogOnce = false;LOGLN("X1:" + String(analogRead(X1)) + "|X2:" + String(analogRead(X2)) + "|X3:" + String(analogRead(X3)));}
      // counter++;if(counter > 100){counter = 0;counterHold++;LOGLN(counterHold);
      // }delay(10);
      // if(counterHold > 60){counterHold = 0;
      // LOGLN("gateway detect");
      //   CONFIG::write_byte(EP_EEPROM_MODULE_TYPE,1); delay(3000);ESP.restart();
      // }
  }
  ///////////////////////////////////////////////////////////////////////
  if (analogRead(X1) <= minValue)
  {
    if(LogOnce){LogOnce = false;LOGLN("X1:" + String(analogRead(X1)));}

    if (o > delayForCounter/3)
    {
      o = 0;
#ifdef DEBUG_
      LOGLN("X1");
#endif //#if DEBUG_
    }//if (o > delayForCounter/3)
  }    //if (analogRead(X1) <= minValue)

  if (analogRead(X2) <= minValue){
    if(LogOnce){LogOnce = false;LOGLN("X2:" + String(analogRead(X2)));}
    if (o > delayForCounter/3)
    {
      o = 0;
#ifdef DEBUG_
      LOGLN("X2");
#endif //#if DEBUG_
    }//if (o > delayForCounter/3)
  }    //if (analogRead(X2) <= minValue)

  if (analogRead(X3) <= minValue)
  {
    if(LogOnce){LogOnce = false;LOGLN("X3:" + String(analogRead(X3)));}
    if (o > delayForCounter/3)
    {
      o = 0;
#ifdef DEBUG_
      LOGLN("X3");
#endif //#if DEBUG_
    }  //if(o > delayForCounter/3){
  }    //if (analogRead(X3) <= minValue)  

  
#endif//Gateway
}//if(Gateway == false){
#endif//Develop
}//task input