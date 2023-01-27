#ifndef UpdateFW_
#define UpdateFW_
#include <Arduino.h>

#define ServerUpdateFW
class UpdateFW
{
public:
const String FirmwareVer={"1.1"};
 void repeatedCall();
 void ShowMess(String txt);
 byte FirmwareVersionCheck(void);
};
extern UpdateFW UPDATEFW;
#endif//UpdateFW_