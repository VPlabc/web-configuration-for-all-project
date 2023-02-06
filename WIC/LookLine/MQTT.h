#ifndef MQTT_
#define MQTT_
#include "LookLine/LookLine.h"

#ifdef MQTT_Mode

////////////////////////////////////////////////////////////////
  //0 main display||1 test||2 setting||3 ConFi||4 Read ID||
#define Main    0
#define Test    1
#define Setting 2
#define ConFi   3
#define Online  4
#define SLEEP   5
#define CLEAR   6
#define UPDATE  7

byte countFail = 0;
byte timeOut = 0;

void callback(char* topic, byte* payload, unsigned int length) {
        
        String PayLoad = "";

        for (int i=0;i<length;i++) {
          //Serial.print((char)payload[i]);
          PayLoad += (char)payload[i];
        }
       // LOG("mqtt received: ");
        LOG((char*)topic);
        //LOG(" - ");
        //LOGLN(PayLoad);
     String Topic ="";byte role = 0;
     CONFIG::read_byte (EP_EEPROM_ROLE, &role) ;
    if(role == GATEWAY){Topic = TopicOut;}else{Topic = TopicIn;}
        if(String(topic) == Topic){
          if (PayLoad == "Up")
          {
            if(DisplayMode == Main){
            // Mode = 3;WriteAll();delay(3000);ESP.restart();
            }
            if(DisplayMode == SLEEP){
            //Mode = 2;WriteAll();delay(3000);ESP.restart();
            // LOGLN("OTA online"); Mode = 1;WifiSetup();
            Mode = 2;
            }
          }
          if (PayLoad == "Sleep")
          {
            //EEPROM.write(0,0);EEPROM.commit();esp_restart();
            if (DisplayMode == Main)
            {
              lock = true;
              DisplayMode = SLEEP;
              SetupPro = 0;
      #ifdef DEBUG_
              LOGLN("Sleep");
      #endif //#if DEBUG_
            }
            LOGLN("online");//Mode = 1;
          }  
        if (PayLoad == "Setup")
        {
          //Mode = 2;WriteAll();delay(3000);ESP.restart();
          LOGLN("OTA online"); 
          // Mode = 1;WifiSetup();
          Mode = 2;
          // SetupDisplay();
          delay(1000);
          DisplayMode = Setting;
    #ifdef DEBUG_
          LOGLN("Setup");
    #endif //#if DEBUG_
        }//OTA
        if (PayLoad == "OK")
        {
          switch (DisplayMode)
          {
          case Setting: //setup
    #ifdef DEBUG_
            LOGLN("SetSave");
    //LOGLN(BoardID);
    #endif             //#if DEBUG_
            // SetSave(); //LOGLN("Saved");
            // ProInc();  //LOGLN("next program");
            // SetLoad(); //LOGLN("Load value");
            break;
          case Main: //main
            Run = true;
            delay(100);
            // WriteAll();
    #ifdef DEBUG_
            LOGLN("Run");
    #endif //#if DEBUG_
            //Mode = 0;WriteAll();delay(3000);ESP.restart();
          case ConFi: //ConFi
    #ifdef DEBUG_
            //LOGLN("Set ID Save");
    #endif //#if DEBUG_
            //SetupPro = 11;
            //SetSave();
            //lock = true;
            DisplayMode = Main;
            break;
          case Online:
            if (BoardID == (ValueSet0 * 100 + ValueSet1 * 10 + ValueSet2))
            {
              DisplayMode = Setting;
    #ifdef DEBUG_
              LOGLN("read OK");
    #endif //#if DEBUG_
            }
            else
            {
              lock = true;
              DisplayMode = Main;
              SetupPro = 0;
    #ifdef DEBUG_
              LOGLN("Cancel");
    #endif //#if DEBUG_
            }

            break;
          case SLEEP:
            lock = true;
            DisplayMode = Main;
            SetupPro = 0;
    #ifdef DEBUG_
            LOGLN("Wakeup");
    #endif //#if DEBUG_
            // Mode = 0;WriteAll();delay(3000);ESP.restart();
            break;
          default:
            break;
          }
        }//OK cmd
        if (PayLoad == "Stop")
        {
          Run = false;
    #ifdef DEBUG_
          LOGLN("Stop");
          delay(100);
          WriteAll();
    #endif //#if DEBUG_
    #ifdef TEST_MODE
            DisplayMode = Test;
            testFlash();
    #ifdef DEBUG_
            LOGLN("Test");
    #endif //#if DEBUG_
    #else
            countReset++;
            if (countReset > 3)
            {
              countReset = 0;
              Plan = 0;
              Result = 0;
    #ifdef DEBUG_
              LOGLN("Clear");
    #endif //DEBUG_
            }
    #endif //TEST_MODE
          
          if (DisplayMode == Test)
          {
            // ReadAll();
          }
          if (DisplayMode == Setting || DisplayMode == Online || DisplayMode == ConFi)
          {
            lock = true;
            DisplayMode = Main;
            SetupPro = 0;
    #ifdef DEBUG_
            LOGLN("Cancel");
    #endif //#if DEBUG_
          }
        }
      }//if(topic == "cmd"){
        
        if(PayLoad[0] == '{'){ConfigJsonProcess(PayLoad);}
        
        //LOG("mqtt received: ");
        LOGLN(PayLoad);
        for(byte c = 0 ; c < PayLoad.length()+2;PayLoad.toCharArray(buffer, c++));
        done = true;   
        String ids = "";
        ids += buffer[0];
        ids += buffer[1];
        ids += buffer[2];
        ids += buffer[3];
        Id = ids.toInt();
        Data_Proccess();
}
void reconnect() {
  // Loop until we're reconnected
  if(countermqtt > 50){
    if (!mqtt.connected()) {
      //LOG("Attempting MQTT connection...");
      // Attempt to connect
      if (mqtt.connect("vuletech")) {
        //LOGLN("connected");
        // Once connected, publish an announcement...
        //mqtt.publish("/TopicOut","hello world");
        // ... and resubscribe
        
        if(ComMasterSlave == true){
          //mqtt.publish("/TopicOut","Gateway Ver 14");
          mqtt.subscribe(TopicIn);
        }
        if(ComMasterSlave == false){
          //mqtt.publish("/TopicIn","lookline Ver 14");
          mqtt.subscribe(TopicOut);
        }
        //
      } else {
        countermqtt = 0;
        if(countermqtt == 0){
        LOG("failed, rc=");
        LOG(mqtt.state());
        LOGLN(" try again in 5 seconds");
        }//if(countermqtt == 0){
        // Wait 5 seconds before retrying
      }
    }//if (!mqtt.connected()) {
  }//if(countermqtt > 50){
}

void MQTT_setup() {
if(WiFi.status() == WL_CONNECTED  && Mode != 3){   
  #ifdef DEBUG_
  LOGLN("broker connecting to  " + String(mqtt_server)+ "  " + String (MQTTPort));
  #endif//#ifdef DEBUG_
if(mqtt_server != "" || MQTTPort != 0 ) {   
  mqtt.setServer(mqtt_server, MQTTPort);
  mqtt.setCallback(callback);
}
  timeOut = 0;
#ifdef DEBUG_
  LOGLN("broker connected!");
#endif//#ifdef DEBUG_
 





     //if(Mode = 2){
  //mqtt.publish("/monitor", "Node: " + String( BoardID) + " IP:" + WiFi.localIP().toString());
  //}
//#ifdef MQTT_Mode
      
    }//if(mqtt_server != NULL || MQTTPort != 0 ) {
      // else{TaskServerpro();}
  }//if(WiFi.status == WL_CONNECTED){  


void MQTT_loop() {  
  if(WiFi.status() == WL_CONNECTED && Mode != 3){ 
    if(mqtt_server != "" || MQTTPort != 0 ) {
    if (!mqtt.connected()) {
      reconnect();
    }
    mqtt.loop();
    }
    // else{TaskServerpro();}
  }//if(WiFi.status == WL_CONNECTED){
}

#endif//MQTT_Mode

#endif//MQTT_

/*
if(MonitorMode == 3){
          if(ComMode == MQTT && Mode != 3){  
            //for(int c = 0 ; c < sentData.length();sentData.toCharArray(Buffer, c++));
            String monitor = "__________________________________________";
                monitor += "EEPROM Read:";
                monitor += "Board ID:";
                monitor += BoardID;
                monitor += "\nPlan:";
                monitor += Plan;
                monitor += "  PLanSet:";
                monitor += PLanSet;
                monitor += "\nResult:";
                monitor += Result;
                monitor += "  ResultSet:";
                monitor += ResultSet;
                monitor += "\nTime:";
                monitor += Time;
                monitor += "  dotin:";
                monitor += DotIn;
                monitor += "\npcsInShift:";
                monitor += pcsInShift;
                monitor += "  Plan Limit:";
                monitor += PlanLimit;
                monitor += "  pass1:";
                monitor += Pass1;
                monitor += "\nssid:";
                monitor += ssid;
                monitor += "   password:";
                monitor += password;
                #ifdef MQTT_Mode
                if(ComMode == MQTT){ 
                monitor += "\nhost:";
                monitor += mqtt_server;
                monitor += "Port:";
                monitor += MQTTPort;
                monitor += "\nTopic sub:";
                monitor += TopicIn;
                monitor += "Topic pub:";
                monitor += TopicOut;
                }
                #endif//MQTT_Mode
                monitor += "\n__________________________________________";
                monitor += "\nMode:";
                if(Mode == 3){monitor += "AP Config |";}
                if(Mode == 2){monitor += "OTA online |";}//Web server OTA
                #ifdef UpdateFw
                if(Mode == 1){monitor += "online |";}
                #endif//UpdateFw 
                if(Mode == 0){monitor += "offline |";DisplayMode = Main;
                #ifdef Mesh_Network
                if(ComMode == MESH){ monitor += "Communica: MESH |";}
                #endif//Mesh_Network
                #ifdef MQTT_Mode
                if(ComMode == MQTT){ monitor += "Communica: MQTT |";}
                #endif//MQTT_Mode
                }     
                #ifdef LoRa_Network
                if(ComMode == LoRa){ monitor += "Communica: Lora |";}
                #endif//LoRa_Network

                if(ComMode == RS485com){ monitor += "Communica: RS485 |";}

                monitor += "firmware:";
                monitor += FirmwareVer;
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
            mqtt.publish("/TopicOut", monitor);
            //client.publish(TopicOut, Buffer);
            }
          #endif//MQTT_Mode  
      }//if(MonitorMode == 3){
*/