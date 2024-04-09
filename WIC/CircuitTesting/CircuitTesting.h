#ifndef CIRCUIT_TESTING
#define CIRCUIT_TESTING
#include "Arduino.h"
#include "Config.h"

#ifdef CircuitTesting_UI
class CircuitTest
{
  byte SEGMENT_0 =         0x00; //OK-3  
  byte SEGMENT_1 =         0x01; //OK-3 
  byte SEGMENT_2 =         0x02; //OK-4
  byte SEGMENT_3 =         0x04; //OK-7
  byte SEGMENT_4 =         0x08; //OK-6
  byte SEGMENT_5 =         0x10; //OK-5
  byte SEGMENT_6 =         0x20; //OK-1
  byte SEGMENT_7 =         0x40; //OK-0
  byte SEGMENT_8 =         0x80; //OK-2


public:


void ShowOut();
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val0, uint8_t val1, uint8_t val2);
void PrintShift(uint8_t val0, uint8_t val1, uint8_t val2) ;

void testFileIO(fs::FS &fs, const char * path);
void deleteFile(fs::FS &fs, const char * path);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void appendFile(fs::FS &fs, const char * path, const char * message);
void writeFile(fs::FS &fs, const char * path, const char * message);
void readFile(fs::FS &fs, const char * path);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void CheckPin();
void testAllPin(byte steps);
void Setup();
void Loop();
};
#endif// CircuitTesting_UI
#endif//CIRCUIT_TESTING