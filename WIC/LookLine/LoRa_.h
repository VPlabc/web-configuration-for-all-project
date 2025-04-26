#ifndef LORA_
#define LORA_
#include "config.h"
// #include "WIC.h"
#ifdef USE_LORA
#include "LookLine.h"
#ifdef USE_LORA
    #include "LoRa_E32.h"
#endif//USE_LORA
  uint8_t M0_ = 0;
  uint8_t M1_ = 0;
  uint8_t TX_ = 0;
  uint8_t RX_ = 0;
  String Str_Lora_CH = "";
  String Air_Rate = "";
  String Baud_Rate = "";
  String Lora_PWR = "";
void SetPinLoRa(uint8_t M0, uint8_t M1, uint8_t TX = 17, uint8_t RX = 16)
{
	M0_ = M0;
	M1_ = M1;
	TX_ = TX;
	RX_ = RX;
	LOGLN("LORA Pin M0:" + String(M0_) + "| M1:" + String(M1_) + "| TX:" + String(TX_) + "| RX:" + String(RX_));
}
extern void WriteLoRaConfig(byte CH, byte ID);
// void WriteLoRaConfig(byte CH,byte AirRate );
// void SetChanel(byte CH){
// 	WriteLoRaConfig(CH);
// }
// LoRa_E32 e32ttl100(16, 17, &Serial2, UART_BPS_RATE_9600, SERIAL_8N1); // e32 TX e32 RX
LoRa_E32 e32ttl100(&Serial2, -1, M0_, M1_, UART_BPS_RATE_9600);

void printParameters(struct Configuration configuration);

void ReadLoRaConfig()
{
    
  digitalWrite(M0_, HIGH);
  digitalWrite(M1_, HIGH);
	// Startup all pins and UART
	e32ttl100.begin();

	ResponseStructContainer c;
	c = e32ttl100.getConfiguration();
	// It's important get configuration pointer before all other operation
	Configuration configuration = *(Configuration*) c.data;
	LOG(c.status.getResponseDescription() + "| code: " + c.status.code + "| ");

	printParameters(configuration);

	c.close();
    digitalWrite(M0_, LOW);
    digitalWrite(M1_, LOW);
}
// void WriteLoRaConfig(byte CH,byte AirRate )
// {
//     digitalWrite(M0_, HIGH);
//     digitalWrite(M1_, HIGH);
// 	// Startup all pins and UART
// 	e32ttl100.begin();

// 	ResponseStructContainer c;
// 	c = e32ttl100.getConfiguration();
//     Configuration configuration = *(Configuration*) c.data;
//       ///*
// 	configuration.ADDL = 0x0;
// 	configuration.ADDH = 0x0;
// 	configuration.CHAN = CH;

// 	configuration.SPED.uartParity = MODE_00_8N1;
// 	configuration.SPED.uartBaudRate = UART_BPS_9600;
// 	configuration.SPED.airDataRate = AirRate;
// 	configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
// 	configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
// 	configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
// 	configuration.OPTION.fec = FEC_0_OFF;
// 	configuration.OPTION.transmissionPower = POWER_20;


// 	// Set configuration changed and set to not hold the configuration
// 	ResponseStatus rs = e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_LOSE);
// 	//LOGLN(rs.getResponseDescription());
// 	//LOGLN(rs.code);
// 	printParameters(configuration);
//   //*/
//     digitalWrite(M0_, LOW);
//     digitalWrite(M1_, LOW);
// }
void WriteLoRaConfig(byte CH, byte ID)
{
    digitalWrite(M0_, HIGH);
    digitalWrite(M1_, HIGH);
	// Startup all pins and UART
	e32ttl100.begin();

	ResponseStructContainer c;
	c = e32ttl100.getConfiguration();
    Configuration configuration = *(Configuration*) c.data;
      ///*
	configuration.ADDL = 0x0;
	configuration.ADDH = ID;
	configuration.CHAN = CH;

	configuration.OPTION.fec = FEC_1_ON;
	configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
	configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
	configuration.OPTION.transmissionPower = POWER_20;
	configuration.OPTION.wirelessWakeupTime = WAKE_UP_1250;

	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
	configuration.SPED.uartBaudRate = UART_BPS_9600;
	configuration.SPED.uartParity = MODE_00_8N1;
	

	// Set configuration changed and set to not hold the configuration
	ResponseStatus rs = e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
	//LOGLN(rs.getResponseDescription());
	//LOGLN(rs.code);
	printParameters(configuration);
  //*/

 	c.close();
    digitalWrite(M0_, LOW);
    digitalWrite(M1_, LOW);
}
void printParameters(struct Configuration configuration) {
	// if(GatewayTerminal){
  	// LOGLN("----------------------------------------");

	// LOG(F("HEAD : "));  LOG(configuration.HEAD, BIN);LOG(" ");LOG(configuration.HEAD, DEC);LOG(" ");LOGLN(configuration.HEAD, HEX);
	// LOGLN(F(" "));
	// LOG(F("AddH : "));  LOGLN(configuration.ADDH, BIN);
	// LOG(F("AddL : "));  LOGLN(configuration.ADDL, BIN);
	// LOG(F("Chan : "));  LOG(configuration.CHAN, DEC); LOG(" -> "); LOGLN(configuration.getChannelDescription());
	// LOGLN(F(" "));
	// LOG(F("SpeedParityBit     : "));  LOG(configuration.SPED.uartParity, BIN);LOG(" -> "); LOGLN(configuration.SPED.getUARTParityDescription());
	// LOG(F("SpeedUARTDatte  : "));  LOG(configuration.SPED.uartBaudRate, BIN);LOG(" -> "); LOGLN(configuration.SPED.getUARTBaudRate());
	// LOG(F("SpeedAirDataRate   : "));  LOG(configuration.SPED.airDataRate, BIN);LOG(" -> "); LOGLN(configuration.SPED.getAirDataRate());

	// LOG(F("OptionTrans        : "));  LOG(configuration.OPTION.fixedTransmission, BIN);LOG(" -> "); LOGLN(configuration.OPTION.getFixedTransmissionDescription());
	// LOG(F("OptionPullup       : "));  LOG(configuration.OPTION.ioDriveMode, BIN);LOG(" -> "); LOGLN(configuration.OPTION.getIODroveModeDescription());
	// LOG(F("OptionWakeup       : "));  LOG(configuration.OPTION.wirelessWakeupTime, BIN);LOG(" -> "); LOGLN(configuration.OPTION.getWirelessWakeUPTimeDescription());
	// LOG(F("OptionFEC          : "));  LOG(configuration.OPTION.fec, BIN);LOG(" -> "); LOGLN(configuration.OPTION.getFECDescription());
	// LOG(F("OptionPower        : "));  LOG(configuration.OPTION.transmissionPower, BIN);LOG(" -> "); LOGLN(configuration.OPTION.getTransmissionPowerDescription());

	// LOGLN("----------------------------------------");
//   }
  Str_Lora_CH = configuration.getChannelDescription();
  Air_Rate = configuration.SPED.getAirDataRate();
  Baud_Rate = configuration.SPED.getUARTBaudRate();
  Lora_PWR = configuration.OPTION.getTransmissionPowerDescription();

//   LOG("LoRa chanel:" + Str_Lora_CH);
//   LOG("| LoRa Air rate:" + Air_Rate);
//   LOG("| LoRa baudrate:" + Baud_Rate);
//   LOGLN("| LoRa Power:" + Lora_PWR);
}
#endif //USE_LORA
#endif//LORA_