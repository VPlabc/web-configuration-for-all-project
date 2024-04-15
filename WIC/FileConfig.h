#ifndef _FILE_CONFIG_H_
#define _FILE_CONFIG_H_
#include <Arduino.h>
#include "config.h"
#include <FS.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>

typedef struct
{
    String ipAdress;
    String getway;
    String subnet;
    String primaryDNS;
    String secondaryDNS;
    String EthPort;
    String MQhost ;
    String MQport;
    String MQuser;
    String MQpass;
    String wssid;
    String wpass;
} RespondNetworkData;
String Config_path =  "/data/localStorage.json";
RespondNetworkData NetworkData;

#ifdef FILECONFIG

const char *PARAM_ADDRESS = "address";
const char *PARAM_GETWAY = "getway";
const char *PARAM_SUBNET = "subnet";
const char *PARAM_PDNS = "p_dns";
const char *PARAM_SDNS = "s_dns";
const char *PARAM_ETHPORT = "p_port";
const char *PARAM_MQHOST = "host";
const char *PARAM_MQPORT = "port";
const char *PARAM_MQUSER = "user";
const char *PARAM_MQPASS = "pass";
const char *PARAM_SSID = "ssid";
const char *PARAM_PASS = "passw";


void cmdwriteFile(fs::FS &fs, const char *path, const char *message);
String cmdreadFile(fs::FS &fs, const char *path);
void cmdcreateDir(fs::FS &fs, const char *path);
int getIpBlock(int index, String str);
IPAddress str2IP(String str);
String JsonConfigdata = "";
typedef struct
{
    RespondNetworkData GetRespond(fs::FS &fs, const char *path)
    {
        RespondNetworkData ret;
        if(JsonConfigdata == ""){JsonConfigdata = cmdreadFile(fs, path);LOGLN("Export data success!!");}
        // LOGLN(path);
        // LOGLN(data);
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, JsonConfigdata.c_str());
        if (error)
        {
           LOGLN("deserializeJson failed");
           LOGLN(error.f_str());
            return ret;
        }
        else
        {
            ret.ipAdress = doc[PARAM_ADDRESS].as<String>();
            ret.getway = doc[PARAM_GETWAY].as<String>();
            ret.subnet = doc[PARAM_SUBNET].as<String>();
            ret.primaryDNS = doc[PARAM_PDNS].as<String>();
            ret.secondaryDNS = doc[PARAM_SDNS].as<String>();
            ret.EthPort = doc[PARAM_ETHPORT].as<String>();
            ret.MQhost = doc[PARAM_MQHOST].as<String>();
            ret.MQport = doc[PARAM_MQPORT].as<String>();
            ret.MQuser = doc[PARAM_MQUSER].as<String>();
            ret.MQpass = doc[PARAM_MQPASS].as<String>();
            ret.wssid = doc[PARAM_SSID].as<String>();
            ret.wpass = doc[PARAM_PASS].as<String>();
           
        }
        return ret;
    }
    void SaveRespond(fs::FS &fs, const char *path,RespondNetworkData ret)
    {
        // RespondNetworkData ret;
        DynamicJsonDocument data(1024);
        data[PARAM_ADDRESS] = ret.ipAdress;
        data[PARAM_GETWAY] = ret.getway;
        data[PARAM_SUBNET] = ret.subnet;
        data[PARAM_PDNS] = ret.primaryDNS;
        data[PARAM_SDNS] = ret.secondaryDNS;
        data[PARAM_ETHPORT] = ret.EthPort;
        data[PARAM_MQHOST] = ret.MQhost;
        data[PARAM_MQPORT] = ret.MQport;
        data[PARAM_MQUSER] = ret.MQuser;
        data[PARAM_MQPASS] = ret.MQpass;
        data[PARAM_SSID] = ret.wssid;
        data[PARAM_PASS] = ret.wpass;

        String objectString;
        serializeJson(data, objectString);
        cmdwriteFile(SPIFFS, path, objectString.c_str());
    //    LOGLN(objectString);
    }
}NetworkFileConfig;


void cmdwriteFile(fs::FS &fs, const char *path, const char *message)
{
    LOGLN("Writing file: " + String(path));

    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
       LOGLN("- failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
       LOGLN("- file written");
    }
    else
    {
       LOGLN("- write failed");
    }
    file.close();
}
String cmdreadFile(fs::FS &fs, const char *path)
{
    NetworkFileConfig netconfig;
    String data = "";
    // LOGLN("Reading file: " + String(path));

    File file = fs.open(path);
    if (!file || file.isDirectory())
    {
       LOGLN("- failed to open file for reading");
        cmdcreateDir(fs, "/data");
        // RespondNetworkData NetworkData;
        netconfig.SaveRespond(SPIFFS, path, NetworkData);
        LOGLN("create file config");
    }

//    LOGLN("- read from file:");
    while (file.available())
    {
        data += (char)file.read();
    }
    file.close();
    return data;
}
void cmdcreateDir(fs::FS &fs, const char *path)
{
    Serial.printf("Creating Dir: %s\n", path);
    if (fs.mkdir(path))
    {
       LOGLN("Dir created");
    }
    else
    {
       LOGLN("mkdir failed");
    }
}

IPAddress str2IP(String str)
{
    IPAddress ret(getIpBlock(0, str), getIpBlock(1, str), getIpBlock(2, str), getIpBlock(3, str));
    return ret;
}
int getIpBlock(int index, String str)
{
    char separator = '.';
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = str.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (str.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? str.substring(strIndex[0], strIndex[1]).toInt() : 0;
}

void saveLocalStorage(RespondNetworkData NetworkData)
{
    NetworkFileConfig netconfig;
    netconfig.SaveRespond(SPIFFS, Config_path.c_str(), NetworkData);
}

RespondNetworkData localStorageExport()
{
    // RespondNetworkData NetworkData;
    NetworkFileConfig netconfig;
    NetworkData = netconfig.GetRespond(SPIFFS, Config_path.c_str());
    return NetworkData;
}
#endif//FILECONFIG


#endif//FileConfig