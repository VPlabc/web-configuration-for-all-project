#include "data_transfer.h"
DataTransfer dataTrans;


void monitorDataControlValve(uint16_t id, uint16_t regs_data)
{
    DataValveControl _data;
    _data.register_data = regs_data;
    //Serial.println();
    //Serial.println("*** COMMAND FORM WEB ***");
    Serial.print("Valve ID: ");
    Serial.print(id);
    Serial.print("     open_cmd: ");
    Serial.print(_data.open_cmd);
    Serial.print("     close_cmd: ");
    Serial.print(_data.close_cmd);
    Serial.print("      Register_data : ");
    Serial.println(_data.register_data, BIN);
    Serial.println();
}

void monitorDataStateValve(uint16_t id, uint16_t regs_data)
{
    DataValveMonitor _data;
    _data.register_data = regs_data;
    //Serial.println("*** STAGE OF VALVE ***");
    Serial.print("Valve ID: ");
    Serial.print(id);
    Serial.print("      connect : ");
    Serial.print(_data.connected);
    Serial.print("     open_stage: ");
    Serial.print(_data.open_state);
    Serial.print("     close_stage: ");
    Serial.print(_data.close_state);
    Serial.print("      power : ");
    Serial.print(_data.power_cap);
    Serial.print("      Register_data : ");
    Serial.println(_data.register_data, BIN);
    Serial.println();
}
