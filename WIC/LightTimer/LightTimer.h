#ifndef LightTimer_
#define LightTimer_
#include "config.h"
#ifdef HomeKit
#include <arduino_homekit_server.h>
#endif//HomeKit
class LightTimer
{
public:

#ifdef Switch_UI
int state = 0;
int analogInput = 0;
float vout = 0.0;
float vin = 0.0;
float R1 = 3300.0; // resistance of R1 (100K) -see text!
float R2 = 1000.0; // resistance of R2 (10K) - see text!
int value = 0;
float vdd;
long timew;
long times;
int timezone = 7;
int dst = 0;
byte hours, mins, secs;
byte lastHour = 0;
byte Timer[4][20];
int State[5][100];
/////////////////////////////
int SensorData[4][60];
byte CounterSensorLog = 0;
////////////////////////////
int last_brightness = 0;
//uint16_t brightness_Recvice = 0;
byte ChartHour[4][24];
byte ChartWeek[4][7];
byte ChartMoth[4][30];
int HourOfYearSensorLog = 0;
byte dayliy = 0;
int HumValue,HumSetU,HumSetD;
int AltSet = 0;
int wsClientNumber[5] = { -1, -1, -1, -1, -1};
int lastClientIndex = 0;
const int max_ws_client = 5;

int ByteConfig;
byte LogID = 0;
byte TimeDataLog = 0;

String URL_fw_Version = "";
String URL_fw_Bin = "";
  bool GatewayTerminal = true;
  bool STAModeNormal = true;
  int Mode = 0;
  byte ComMasterSlave = 1;
  byte SetupForBegin = 0;
  byte ModuleType = 0;



int humidity ;
int temperature ;
float humiditybefor ;
float temperaturebefor ;
boolean device_one_state = false;
byte states[10] = {0,0,0,0,0,0,0,0,0,0};
bool locks = true;

#define button 0//14 | Sinlink 12
#define Relay 12//Sinlink 4
#define LED_Relay 5//4 | Sinlink 13

#define LED_STT 2//

#define Debug_Ser Serial

    void wsSendToWeb();
    void setStatus(boolean st);
    void changestate();
    void reciverDataFromWeb(uint8_t *payload);
    void SaveBool();
    void LoadBool();
    void LoadData();
    void Save(byte ID, byte state);
    void Save(byte ID, byte hourSet,byte minSet,byte state,byte bright);
    void WriteValue(byte types, String usr);
    void voltRead();
    void readtime();
    void SetTime(String Data,byte ID);
    void SetBrightLight(byte Bright);
    void CaculaTime();
#endif//Switch_UI
    #ifdef HomeKit
    void cha_switch_on_setter(const homekit_value_t value);
    void Homekit_Setup();
    void Homekit_Loop();
    #endif//HomeKit
#ifdef ServerUpdateFW
    // bool checkFirmware();
    // bool downloadFirmware();
    // void updateFromFS(fs::FS &fs);
    // void performUpdate(Stream &updateSource, size_t updateSize);
    // void CheckFWloop();
#endif//ServerUpdateFW

    LightTimer();

    void Setup();
    void Loop();
    void Load();
    void Main();
};
#endif//#ifndef LightTimer_