#include "config.h"
#ifdef USE_LoRa
#include "LoRaFunc.h"
#include "config.h"
#include "LoRa_E22.h"
#include "Valve.h"

    #ifdef ESP32
    LoRa_E22 e22ttl(&Serial2, -1, 15, 14, UART_BPS_RATE_9600);
    #endif//ESSP32
    #ifdef ESP8266
    LoRa_E22 e22ttl(&LoRa_Ser, D5, D3, D0, UART_BPS_RATE_9600);
    #endif//ESSP8266
    
#ifdef DEBUG_FLAG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(x) Serial.printf(x)
#else
#define debug(x)
#define debugln(x)
#define debugf(x)
#endif

String LoraFunc::Lora_CH = "";
String LoraFunc::Air_Rate = "";
String LoraFunc::Lora_RSSI ="";
String LoraFunc::Lora_PWR = "";
String LoraFunc::Baud_Rate = "";
sensor_data     sensorsLora[NUM_SENSORS];
String GetLoraProtocol(byte type);


    void NormalProtocol();
    void FixedNDProtocol();
    void FixedGWProtocol();
    void WORGWProtocol();
    void WORNDProtocol();
    void BroadcastProtocol1();
    void BroadcastProtocol2();
    void BroadcastProtocol3();
    Configuration configuration;

ResponseStatus LoraFunc::loraSend(const void *message, const uint8_t size)
{
    ResponseStatus ls = e22ttl.sendMessage((uint8_t *)message, size);
    return ls;
}

void LoraFunc::LoraRead()
{
    if (e22ttl.available() > 1) {
          //timeLora = timeLora1 = 0;
          byte rssis = 0;
          // read the String message
      #ifdef ENABLE_RSSI
          ResponseStructContainer rsc = e22ttl.receiveMessageRSSI(sizeof(Message));
      #else
          ResponseStructContainer rsc = e22ttl.receiveMessage(sizeof(Message));
      #endif
        // Is something goes wrong print error
        if (rsc.status.code!=1){
          //debugln(rsc.status.getResponseDescription());
        }else{
          // Print the data received
          //debugln(rsc.status.getResponseDescription());
          message = *(Message*) rsc.data;
        //   Serial.println("ID:");
        //   Serial.println(message.idh, DEC);
        //   Serial.println(message.idl, DEC);
        //   Serial.println("State:");
        //   Serial.println(message.state, DEC);
        //   Serial.println("Bat:");
        //   Serial.println(message.batteryh, DEC);
        //   Serial.println(message.batteryl, DEC);
        //   Serial.println("Bat12:");
        //   Serial.println(message.batteryh12, DEC);
        //   Serial.println(message.batteryl12, DEC);
      #ifdef ENABLE_RSSI
            rssis = rsc.rssi;
                Valve::sensorMessageReceived(rsc.rssi,message.idh,message.idl,message.state,message.batteryl,message.batteryh,message.batteryl12,message.batteryh12);
      #else
                Valve::sensorMessageReceived(0);
      #endif
                Valve::saveSensorData(rsc.rssi,message.idh,message.idl,message.state,message.batteryl,message.batteryh,message.batteryl12,message.batteryh12);
          debugln();            
        //   debugln("Byte1 " + String(RequestMessage.code[0]));
        //   debugln("Byte2 " + String(RequestMessage.code[1]));
        //   debugln("Byte3 " + String(RequestMessage.code[2]));

                last_Sent_timestamp = millis();
                debugln();delay(1000);
          }
        }//if (e22ttl.available()>1) {
}
void ReadLoRaConfig()
{
	// Startup all pins and UART
	e22ttl.begin();
	ResponseStructContainer c;
	c = e22ttl.getConfiguration();
	// It's important get configuration pointer before all other operation
	Configuration configuration = *(Configuration*) c.data;
	//debugln(c.status.getResponseDescription());
	//debugln(c.status.code);
    read_LoRa_Config(configuration);
	//printParameters(configuration);
	c.close();
}
void loraSetup()
{
    

    byte bbuf = 0;
    int ibuf = 920;
// Startup all pins and UART
	// Startup all pins and UART
	e22ttl.begin();
	configuration.ADDL = 0x00;
	configuration.ADDH = 0x00;
	configuration.NETID = 0x00;
    //////////////////////////////////////////// Chanel
    
    if(CONFIG::read_buffer (EP_LORA_CHANEL,  (byte *) &ibuf, INTEGER_LENGTH)){}
	configuration.CHAN = ibuf - OPERATING_FREQUENCY;
    //Serial.println("CH:"+String(ibuf));
    //////////////////////////////////////////// Protocol
    if(CONFIG::read_byte(EP_LORA_PROTOCOL,&bbuf)){}
    String Protocol = GetLoraProtocol(bbuf);
    //Serial.println("Protocol:"+ Protocol);
    //////////////////////////////////////////// RSSI
    if(CONFIG::read_byte(EP_LORA_RSSI,&bbuf)){}
    configuration.TRANSMISSION_MODE.enableRSSI = bbuf;
    //Serial.println("RSSI:"+String(bbuf));
    //////////////////////////////////////////// AirRate
    if(CONFIG::read_byte(EP_LORA_AIRRATE,&bbuf)){}
    configuration.SPED.airDataRate = bbuf;
    //Serial.println("Speed AirRate:"+String(bbuf));
    
    //configuration.SPED
    // Set configuration changed and set to not hold the configuration
	ResponseStatus rs = e22ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
	//Serial.println(rs.getResponseDescription());
	//Serial.println(rs.code);

	ResponseStructContainer c;
	c = e22ttl.getConfiguration();
	// It's important get configuration pointer before all other operation
	configuration = *(Configuration*) c.data;
    c.close();
    //ReadLoRaConfig();
}


#define LoRainfo
void printParameters(struct Configuration configuration) {
#ifdef LoRainfo
Serial.println("----------------------------------------");

	Serial.print(F("HEAD : "));  Serial.print(configuration.COMMAND, HEX);Serial.print(" ");Serial.print(configuration.STARTING_ADDRESS, HEX);Serial.print(" ");Serial.println(configuration.LENGHT, HEX);
	Serial.println(F(" "));
	Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, HEX);
	Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, HEX);
	Serial.print(F("NetID : "));  Serial.println(configuration.NETID, HEX);
	Serial.println(F(" "));
	Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
	Serial.println(F(" "));
	Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
	Serial.print(F("SpeedUARTDatte     : "));  Serial.print(configuration.SPED.uartBaudRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRateDescription());
	Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRateDescription());
	Serial.println(F(" "));
	Serial.print(F("OptionSubPacketSett: "));  Serial.print(configuration.OPTION.subPacketSetting, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getSubPacketSetting());
	Serial.print(F("OptionTranPower    : "));  Serial.print(configuration.OPTION.transmissionPower, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());
	Serial.print(F("OptionRSSIAmbientNo: "));  Serial.print(configuration.OPTION.RSSIAmbientNoise, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getRSSIAmbientNoiseEnable());
	Serial.println(F(" "));
	Serial.print(F("TransModeWORPeriod : "));  Serial.print(configuration.TRANSMISSION_MODE.WORPeriod, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getWORPeriodByParamsDescription());
	Serial.print(F("TransModeTransContr: "));  Serial.print(configuration.TRANSMISSION_MODE.WORTransceiverControl, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getWORTransceiverControlDescription());
	Serial.print(F("TransModeEnableLBT : "));  Serial.print(configuration.TRANSMISSION_MODE.enableLBT, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getLBTEnableByteDescription());
	Serial.print(F("TransModeEnableRSSI: "));  Serial.print(configuration.TRANSMISSION_MODE.enableRSSI, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getRSSIEnableByteDescription());
	Serial.print(F("TransModeEnabRepeat: "));  Serial.print(configuration.TRANSMISSION_MODE.enableRepeater, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getRepeaterModeEnableByteDescription());
	Serial.print(F("TransModeFixedTrans: "));  Serial.print(configuration.TRANSMISSION_MODE.fixedTransmission, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getFixedTransmissionDescription());


	Serial.println("----------------------------------------");

    //Serial.println("Protocol:"+String(user_setting.Protocol));
#endif//LoraInfo
 }
void read_LoRa_Config(struct Configuration configuration)
{
  LoraFunc::Lora_CH = configuration.getChannelDescription();
  LoraFunc::Air_Rate = configuration.SPED.getAirDataRateDescription();
  LoraFunc::Baud_Rate = configuration.SPED.getUARTBaudRateDescription();
  LoraFunc::Lora_PWR = configuration.OPTION.getTransmissionPowerDescription();
  LoraFunc::Lora_RSSI = configuration.TRANSMISSION_MODE.getRSSIEnableByteDescription();

}
void LoraFunc::printModuleInformation(struct ModuleInformation moduleInformation) {
	Serial.println("----------------------------------------");
	Serial.print(F("HEAD: "));  Serial.print(moduleInformation.COMMAND, HEX);Serial.print(" ");Serial.print(moduleInformation.STARTING_ADDRESS, HEX);Serial.print(" ");Serial.println(moduleInformation.LENGHT, DEC);

	Serial.print(F("Model no.: "));  Serial.println(moduleInformation.model, HEX);
	Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
	Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
	Serial.println("----------------------------------------");

}




String GetLoraProtocol(byte type)
{
        String Name = "";
    switch(type)
    {
        case Normal: NormalProtocol();Name = "Normal";break;
        case FixedND: FixedNDProtocol();Name = "FixedND";break;
        case FixedGW: FixedGWProtocol();Name = "FixedGW";break;
        case WORGW: WORGWProtocol();Name = "WORGW"; break;
        case WORND: WORNDProtocol();Name = "WORND"; break;
        case Broadcast1: BroadcastProtocol1();Name = "Broadcast1"; break;
        case Broadcast2: BroadcastProtocol2();Name = "Broadcast2"; break;
        case Broadcast3: BroadcastProtocol3();Name = "Broadcast3"; break;

        default:break;
    }
    return Name;
}



void NormalProtocol()
{
    
    //Serial.println("TRANSPARENT");
//	----------------------- DEFAULT TRANSPARENT -----------------------
    configuration.SPED.uartBaudRate = UART_BPS_9600;
    configuration.SPED.uartParity = MODE_00_8N1;

    configuration.OPTION.subPacketSetting = SPS_240_00;
    configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_ENABLED;
    configuration.OPTION.transmissionPower = POWER_30;

    configuration.TRANSMISSION_MODE.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
    configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_DISABLED;
    configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
    configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_RECEIVER;
    configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
}
void FixedNDProtocol()
{
    //Serial.println(" FIXED SENDER ");
    //	----------------------- FIXED SENDER -----------------------
    configuration.SPED.uartBaudRate = UART_BPS_9600;
    configuration.SPED.uartParity = MODE_00_8N1;

    configuration.OPTION.subPacketSetting = SPS_240_00;
    configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
    configuration.OPTION.transmissionPower = POWER_30;

    configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
    configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_DISABLED;
    configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
    configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_TRANSMITTER;
    configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
	
}
void FixedGWProtocol()
{   
    //Serial.println(" FIXED RECEIVER ");
    //	----------------------- FIXED RECEIVER -----------------------
    configuration.SPED.uartBaudRate = UART_BPS_9600;
    configuration.SPED.uartParity = MODE_00_8N1;

    configuration.OPTION.subPacketSetting = SPS_240_00;
    configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
    configuration.OPTION.transmissionPower = POWER_30;

    configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
    configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_DISABLED;
    configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
    configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_RECEIVER;
    configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
}
void WORGWProtocol()
{
    //Serial.println(" WOR SENDER  ");
    //	----------------------- WOR SENDER -----------------------
    configuration.SPED.uartBaudRate = UART_BPS_9600;
    configuration.SPED.uartParity = MODE_00_8N1;

    configuration.OPTION.subPacketSetting = SPS_240_00;
    configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
    configuration.OPTION.transmissionPower = POWER_30;

    configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
    configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_DISABLED;
    configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
    configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_TRANSMITTER;
    configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
}
void WORNDProtocol()
{
    //Serial.println(" WOR RECEIVER  ");
	//	----------------------- WOR RECEIVER -----------------------
    configuration.SPED.uartBaudRate = UART_BPS_9600;
    configuration.SPED.uartParity = MODE_00_8N1;

    configuration.OPTION.subPacketSetting = SPS_240_00;
    configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
    configuration.OPTION.transmissionPower = POWER_30;

    configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
    configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_DISABLED;
    configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
    configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_RECEIVER;
    configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
}
void BroadcastProtocol1()
{
    //Serial.println(" BROADCAST MESSAGE 1  ");
	//	----------------------- BROADCAST MESSAGE 1 -----------------------
    configuration.SPED.uartBaudRate = UART_BPS_9600;
    configuration.SPED.uartParity = MODE_00_8N1;

    configuration.OPTION.subPacketSetting = SPS_240_00;
    configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
    configuration.OPTION.transmissionPower = POWER_30;

    configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
    configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_DISABLED;
    configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
    configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_RECEIVER;
    configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
}
void BroadcastProtocol2()
{
    //Serial.println(" BROADCAST MESSAGE 2  ");
	//	----------------------- BROADCAST MESSAGE 2 -----------------------
    configuration.SPED.uartBaudRate = UART_BPS_9600;
    configuration.SPED.uartParity = MODE_00_8N1;

    configuration.OPTION.subPacketSetting = SPS_240_00;
    configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
    configuration.OPTION.transmissionPower = POWER_30;

    configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
    configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_DISABLED;
    configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
    configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_RECEIVER;
    configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
}

void BroadcastProtocol3()
{

    //Serial.println(" BROADCAST MESSAGE 3  ");
	//	----------------------- BROADCAST MESSAGE 2 -----------------------
    configuration.SPED.uartBaudRate = UART_BPS_9600;
    configuration.SPED.uartParity = MODE_00_8N1;

    configuration.OPTION.subPacketSetting = SPS_240_00;
    configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
    configuration.OPTION.transmissionPower = POWER_30;

    configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
    configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_DISABLED;
    configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
    configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_RECEIVER;
    configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
}
#endif//USE_LoRa