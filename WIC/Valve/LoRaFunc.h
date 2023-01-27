#ifndef LoRaFunc_h
#define LoRaFunc_h
#include "Arduino.h"
#ifdef USE_LoRa
#include "LoRa_E22.h"

extern void loraSetup();

#define         NUM_SENSORS 20
    ////////////////// LORA Protocol
    #define Normal        0
    #define FixedGW       1
    #define FixedND       2
    #define WORGW         3
    #define WORND         4
    #define Broadcast1    5
    #define Broadcast2    6
    #define Broadcast3    7
    
    #define Master 1
    #define Slave 0

    extern void read_LoRa_Config(struct Configuration configuration);
    extern void printParameters(struct Configuration configuration);
    extern void ReadLoRaConfig();

class LoraFunc 
{
public:
    // ---------- esp32 pins --------------
    //LoRa_E22 e22ttl(&LoRa_Ser, AUX, M0, M1); //  RX AUX M0 M1

    // -------------------------------------

    //String GetLoraProtocol(byte type);
    void printModuleInformation(struct ModuleInformation moduleInformation);
    ResponseStatus loraSend(const void *message, const uint8_t size);
    void LoraRead();
    struct Message {
        byte idh = 0;
        byte idl = 0;
        byte state;
        byte batteryl;
        byte batteryh;
        byte batteryl12;
        byte batteryh12;
    } message;


    struct requestMessage {
        byte code[6];
    } RequestMessage;

///////////////////////////////
    static int Int_Lora_CH;
    static String Lora_CH;
    static String Air_Rate;
    static String Baud_Rate;
    static String Lora_PWR;
    static String Lora_RSSI;
    
};
#endif//USE_LoRa
#endif//LoRaFunc_h