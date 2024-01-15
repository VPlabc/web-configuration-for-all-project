#ifndef LOOKLINE_
#define LOOKLINE_
#include "config.h"
#ifdef LOOKLINE_UI
#include <Arduino.h>
#include <ArduinoJson.h>
//#include <ArduinoOTA.h>


#define LED7segBoardV14

#ifdef DEBUG_FLAG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(x) Serial.printf(x)
#else
#define debug(x)
#define debugln(x)
#define debugf(x)
#endif

#define WIFIMODE 1
#define MESHMODE 2

#define WEB 1
#define SER 2
#define OUPUT SER

  #define X0 32
  #define X1 35  
  //Counter0
  #define X2 34
  //Counter1
  #define X3 39
  //learning
  #define X4 36
  ////// Switch
  #define SW_Mode0 23
  #define SW_Mode1 33
 
////////////////////////////////////////////////////////////////
  //0 main display||1 test||2 setting||3 ConFi||4 Read ID||
#define Main    0
#define Test    1
#define Setting_ 2
#define ConFi   3
#define Online  4
#define SLEEP   5
#define CLEAR   6
#define UPDATE  7
/////////////////////////////////////// Communication 
#define LoRa 0
#define MESH 1
#define MQTT 2
#define RS485com 3
////////////////////////////////////////////////////////////////
  //0 main display||1 test||2 setting||3 ConFi||4 Read ID||
#define AutoDetect    0
#define ModGateway    1
#define ModuleV140    2
#define ModuleV141    3
#define ModuleV130    4
#define ModuleV129    5
//////////////////////////////////////// Role Type
#define NODE        0
#define GATEWAY     1
#define REPEARTER   2
#define MASTER      3
/////////////////////////////////////// Comunication Command
#define UPDATEcmd 0
#define OKcmd 1
#define ONWIFIcmd 2

#define         NUM_LOOKLINES 20
typedef struct Command{
    byte updateStt = 0;
    byte setup = 1;
    byte repeater = 2;
    byte readConfig = 3;
    byte request = 4;
    byte updateFw = 5;
}Command;
typedef struct struct_command_message {
    byte Command;       //1
    byte networkID;       //1
    byte nodeID;          //1
    byte category;        //1
    byte time;            //1
} struct_command_message;

typedef struct struct_Parameter_message {
    byte networkID;       //1
    byte nodeID;          //1
    int PLAN;             //4
    int RESULT;           //4
    byte state;           //1
    byte Mode;            //1
    byte RSSI;            //1
    byte Com;            //1
    byte WiFi;            //1
    byte Cmd;            //1
    byte type;            //1
    int Nodecounter;
} struct_Parameter_message;


  extern void handleLooklineRaw();
  extern void Data_Proccess(char byte_buffer[]);


// uint8_t  incomingData[sizeof(msg1)];
// size_t   received_msg_length;

class LOOKLINE_PROG
{
public:

void DebugOut(String msg,byte output);
void PinMapInit();
void LookLineInitB(int pos,byte Mode);
void LookLineInitI(int pos,int Mode);
void SetLookineValue();
void setup();
void loop();
void SerDisplay();
void ConfigJsonProcess(String Input);
// void Data_Proccess();
void TimerPlanInc();
void UpdateLookLineData();
void caculaOT();
void displayMode(byte Mode);
void SetParameter(int taskPlan, int taskPLanSet, int taskResult, int taskResultSet, int taskTime, int taskpcsInShift, int taskPass, int taskDotIn);
void SetPlan(int SetPlans);
void SetResult(int SetResults);
void SetRun(byte SetRuns);
void Set_Init_UI(String auths);
byte EncodeRespondByte(boolean a, boolean b, boolean c, boolean d, boolean e, boolean f, boolean g, boolean h);
unsigned int EncodeRespond(byte bytel,byte byteh);
// void Data_Proccess(char byte_buffer[]);
void SetDone();
void SetStart(bool START);
void SetConfig(bool CONFIG);
byte GetRun();
byte GetDebug();
bool GetFW();

  int delayForCounter = 1000;
  int SetupForBegin = 0;
  int Mode = 0;



  int DotIn = 0;
  byte Run = 1;
  bool lock = true;//IR Lock mode
  int SetupPro = 0;//Setup program
  int SetupStep = 0;//Setup step
  int numCode = 10;///IR number
  int ValueSet0 = 0;
  int ValueSet1 = 0;
  int ValueSet2 = 0;
  int ValueSet3 = 0;
  int Pass = 2709;
  int Pass1 = 0;
  int Pass2 = 9318;
  int Pass3 = 1524;
  byte BoardID = 01;
  byte NetID = 01;
  int ConnectFail = 0;
  ///// EEPROM adress
  int SetupForBeginAdress = 0;
  long counter1 = 0;//thoi gian cap nhat data len web
  long countermqtt = 50;//Timeoutmqtt connnection 
  int counterOUT = 0;
  long countReset = 0;
  int countUPD = 0;
  int countLED = 0;
//   bool Setting = false;
  bool reads = false;
  bool reads1 = false;
  // bool UDF = false;
  // bool WCF = false;
  // bool APC = false;
  bool GatewayTerminal = true;
  bool STAModeNormal = true;
  bool CheckFW = false;
  //byte Type = 0;//0 nothing/1 MQTT+STA /2 Lora+AP /3 RS485+AP 
  unsigned int Size = 0;

  // char Buffer[50];
  // char buffer[250];
  String Error = "";
//   String RunMode = "";
  String DataIn = "";
  // bool done = false;


  uint8_t PWR = 0;
  uint8_t MapPin[10] = {25,26, 2,15,19,22,21,27,13, 4};

    long lastMsg = 0;
    char msg[50];
    int value = 0;
    
    char TopicIn[20] = "/TopicSub";
    char TopicOut[20] = "/TopicPub";
    char mqtt_server[50];
    int MQTTPort = 1883;
private:
 

//////////////////////////////////////////////////////////////////////////

};

uint8_t ConvertToLed7Segment(uint8_t value);

extern LOOKLINE_PROG Lookline_PROG;
#endif//LOOKLINE_UI
#endif//LOOKLINE_