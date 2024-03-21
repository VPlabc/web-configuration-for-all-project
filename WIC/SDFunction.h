#ifndef SDFunctions_h
#define SDFunctions_h
#include "config.h"
#include "Arduino.h"
// #ifdef Valve_UI
#define TIMER_INTER

#include <SD.h>
/* ------------------------ SENSORS DATA --------------------------------------- */

#define MESH_ID               6734922
#define GROUP_SWITCH          1
#define GROUP_HT              2
#define GROUP_MOTION          3
#define battery_cutoff_volt   2.9
////////////////// RF Type
#define LoRa      0
#define MQTT      1
#define MESH      2
#define RS485com  3
///////////////////////////
////////////////// Board Type
#define ModeGateway   0
#define ModeNode      1
#define LooklineV130  2
#define LooklineV140  3
#define LooklineV141  4
////////////////// Command
#define FeedbackCmd 0 //request feedback
#define SleepCmd    1 //request Sleep
#define OpenCmd     1 //Open
#define CloseCmd    0 //Close

#define         NUM_SENSORS 20
class SDFunction 
{
public:
    SDFunction();
static bool sd_card_found;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    /* --------------------------- SD CARD -------------------------------------------- */
    static File root;
    bool opened = false;  
    
    uint64_t GetTotalSize();
    void Setup();
    static void saveMemoryToFile();
    static void readMemoryFromFile();
    static String printDirectory(File dir, int numTabs);
    static bool loadFromSDCARD(String path);
    static void handleRawFile();
    static void handleNames();
    static void handleRetrySD();
    static void handleDeleteSensor();
    static void handleDeleteFile();
    static String JsonRaw();
    static void TestSDCard();
///////////////////////////////////////// 
};
// #endif//Valve_UI
#endif//SDFunctions_h