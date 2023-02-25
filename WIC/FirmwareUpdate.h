#ifndef UpdateFW_
#define UpdateFW_
#include <Arduino.h>

#define ServerUpdateFW
class UpdateFW
{
public:
const String FirmwareVer={"14.9.9.1"};
 void repeatedCall();
 void ShowMess(String txt);
 void FirmwareUpdate();
 byte FirmwareVersionCheck(void);
 
};
extern UpdateFW UPDATEFW;
#endif//UpdateFW_