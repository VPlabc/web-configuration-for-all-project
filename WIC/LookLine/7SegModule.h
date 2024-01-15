#ifndef ModuleType_
#define ModuleType_
#include "LookLine.h"
 
void latch();
void shiftOut4Row(uint8_t dataPin0, uint8_t dataPin1, uint8_t dataPin2, uint8_t clockPin, uint8_t bitOrder, uint8_t val0, uint8_t val1, uint8_t val2);
void PrintSeg(uint8_t val0, uint8_t val1, uint8_t val2);
void displays(int Value0, int Value1, int OTvalue_H, int OTvalue_L,byte dataPin0, byte dataPin1,byte dataPin2,byte clockPin,byte latchPin,int dotint1);

////////////////////////////// Board type 
#ifdef LED7segBoardV11
  #define SEGMENT_A         0x40 //OK  //m� vi tri thanh A theo format 595
  #define SEGMENT_D         0x02 //OK
  #define SEGMENT_C         0x04 //OK
  #define SEGMENT_G         0x08 //OK
  #define SEGMENT_F         0x10 //OK
  #define SEGMENT_H         0x20 //OK
  #define SEGMENT_E         0x01 //OK
  #define SEGMENT_B         0x80 //OK
#endif //LED7segBoardV11

#ifdef LED7segBoardV12
  #define SEGMENT_G         0x80 //OK-0
  #define SEGMENT_F         0x40 //OK-1
  #define SEGMENT_H         0x20 //OK-2
  #define SEGMENT_A         0x10 //OK-3 
  #define SEGMENT_B         0x08 //OK-4
  #define SEGMENT_E         0x04 //OK-5
  #define SEGMENT_D         0x02 //OK-6
  #define SEGMENT_C         0x01 //OK-7
#endif //LED7segBoardV12

#ifdef LED7segBoardV14


  byte SEGMENT_A =         0x0; //OK-3 
  byte SEGMENT_B =         0x0; //OK-4
  byte SEGMENT_C =         0x0; //OK-7
  byte SEGMENT_D =         0x0; //OK-6
  byte SEGMENT_E =         0x0; //OK-5
  byte SEGMENT_F =         0x0; //OK-1
  byte SEGMENT_G =         0x0; //OK-0
  byte SEGMENT_H =         0x0; //OK-2
  byte LED7SEG[8] = {SEGMENT_A,SEGMENT_B,SEGMENT_C,SEGMENT_D,SEGMENT_E,SEGMENT_F,SEGMENT_G,SEGMENT_H};
  //                        A_____B_____C_____D_____E_____F_____G_____H
  byte V13_7SegCode[8] = {0x10, 0x08, 0x01, 0x02, 0x04, 0x40, 0x80, 0x20};
  byte V14_7SegCode[8] = {0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40};
  //v13
  // byte SEGMENT_G =         0x80; //OK-0
  // byte SEGMENT_F =         0x40; //OK-1
  // byte SEGMENT_H =         0x20; //OK-2
  // byte SEGMENT_A =         0x10; //OK-3 
  // byte SEGMENT_B =         0x08; //OK-4
  // byte SEGMENT_E =         0x04; //OK-5
  // byte SEGMENT_D =         0x02; //OK-6
  // byte SEGMENT_C =         0x01; //OK-7
  //v14
  // byte SEGMENT_B         0x01 //OK-7
  // byte SEGMENT_C         0x02 //OK-6
  // byte SEGMENT_D         0x04 //OK-5
  // byte SEGMENT_E         0x08 //OK-4
  // byte SEGMENT_F         0x10 //OK-3
  // byte SEGMENT_G         0x20 //OK-2
  // byte SEGMENT_H         0x40 //OK-1
  // byte SEGMENT_A         0x80 //OK-0 
  
#endif //LED7segBoardV14

byte NUMBER_0  =        (SEGMENT_D|SEGMENT_E|SEGMENT_F|SEGMENT_A|SEGMENT_B|SEGMENT_C);
byte NUMBER_1  =        (SEGMENT_B|SEGMENT_C);
byte NUMBER_2  =        (SEGMENT_D|SEGMENT_E|SEGMENT_G|SEGMENT_A|SEGMENT_B);
byte NUMBER_3  =        (SEGMENT_A|SEGMENT_B|SEGMENT_C|SEGMENT_D|SEGMENT_G);
byte NUMBER_4  =        (SEGMENT_C|SEGMENT_G|SEGMENT_B|SEGMENT_F);
byte NUMBER_5  =        (SEGMENT_A|SEGMENT_C|SEGMENT_D|SEGMENT_F|SEGMENT_G);
byte NUMBER_6  =        (SEGMENT_D|SEGMENT_C|SEGMENT_E|SEGMENT_A|SEGMENT_F|SEGMENT_G);
byte NUMBER_7  =        (SEGMENT_C|SEGMENT_A|SEGMENT_B);
byte NUMBER_8  =        (SEGMENT_D|SEGMENT_E|SEGMENT_F|SEGMENT_A|SEGMENT_B|SEGMENT_C|SEGMENT_G);
byte NUMBER_9  =        (SEGMENT_D|SEGMENT_B|SEGMENT_G|SEGMENT_C|SEGMENT_F|SEGMENT_A);
byte CHARACTER_A =       (SEGMENT_B|SEGMENT_C|SEGMENT_A|SEGMENT_E|SEGMENT_F|SEGMENT_G);
byte CHARACTER_b =       (SEGMENT_D|SEGMENT_C|SEGMENT_G|SEGMENT_F|SEGMENT_E);
byte CHARACTER_P =       (SEGMENT_B|SEGMENT_E|SEGMENT_G|SEGMENT_A|SEGMENT_F);
byte CHARACTER_O =       NUMBER_0;
byte CHARACTER_PASSWORD =       SEGMENT_G;
byte Seg1[11];
byte Seg[11];

// #define NUMBER_0_13          (SEGMENT_D13|SEGMENT_E13|SEGMENT_F13|SEGMENT_A13|SEGMENT_B13|SEGMENT_C13)
// #define NUMBER_1_13          (SEGMENT_B13|SEGMENT_C13)
// #define NUMBER_2_13          (SEGMENT_D13|SEGMENT_E13|SEGMENT_G13|SEGMENT_A13|SEGMENT_B13)
// #define NUMBER_3_13          (SEGMENT_A13|SEGMENT_B13|SEGMENT_C13|SEGMENT_D13|SEGMENT_G13)
// #define NUMBER_4_13          (SEGMENT_C13|SEGMENT_G13|SEGMENT_B13|SEGMENT_F13)
// #define NUMBER_5_13          (SEGMENT_A13|SEGMENT_C13|SEGMENT_D13|SEGMENT_F13|SEGMENT_G13)
// #define NUMBER_6_13          (SEGMENT_D13|SEGMENT_C13|SEGMENT_E13|SEGMENT_A13|SEGMENT_F13|SEGMENT_G13)
// #define NUMBER_7_13          (SEGMENT_C13|SEGMENT_A13|SEGMENT_B13)
// #define NUMBER_8_13          (SEGMENT_D13|SEGMENT_E13|SEGMENT_F13|SEGMENT_A13|SEGMENT_B13|SEGMENT_C13|SEGMENT_G13)
// #define NUMBER_9_13          (SEGMENT_D13|SEGMENT_B13|SEGMENT_G13|SEGMENT_C13|SEGMENT_F13|SEGMENT_A13)
// #define LED_OFF_13   0

#define ALL_SEGMENT_OFF   0
#define LED_OFF   0

typedef struct SegPin{
    uint8_t segData1;
    uint8_t segData2;
    uint8_t segData3;
    uint8_t segSHCP;
    uint8_t segSTCP;
}SegPin;
SegPin segPin;

byte Debug7Seg = 1;
byte sevenSegModuleType = 0;
//const byte Seg13[11] = {NUMBER_0_13,NUMBER_1_13,NUMBER_2_13,NUMBER_3_13,NUMBER_4_13,NUMBER_5_13,NUMBER_6_13,NUMBER_7_13,NUMBER_8_13,NUMBER_9_13,LED_OFF_13};

void SetPin7Seg(uint8_t Data1, uint8_t Data2, uint8_t Data3, uint8_t SHCP , uint8_t STCP)
{

    LOG("7Seg Pin: ");
    segPin.segData1 = Data1;
    segPin.segData2 = Data2;
    segPin.segData3 = Data3;
    segPin.segSTCP = STCP;
    segPin.segSHCP = SHCP;
  pinMode(segPin.segSHCP , OUTPUT);
  pinMode(segPin.segSTCP, OUTPUT);
  pinMode(segPin.segData1, OUTPUT);
  pinMode(segPin.segData2, OUTPUT);
  pinMode(segPin.segData3, OUTPUT);

            // LOGLN("Data1--" + String(segPin.segData1));
            // LOGLN("Data2--" + String(segPin.segData2));
            // LOGLN("Data3--" + String(segPin.segData3));
            // LOGLN("SHCP--" + String(segPin.segSHCP));
            // LOGLN("STCP--" + String(segPin.segSTCP));
CONFIG::read_byte(EP_EEPROM_MODULE_TYPE, &sevenSegModuleType);
CONFIG::read_byte(EP_EEPROM_DEBUG, &Debug7Seg);
if(sevenSegModuleType == ModuleV130){//Clock Lacth Data
 if(Debug7Seg){LOGLN("LED7 seg Board V13.0");}//Debug 
}//if(sevenSegModuleType == ModuleV130){ 
if(sevenSegModuleType == ModuleV140){
 if(Debug7Seg){LOGLN("LED7 seg Board V14.0");}//Debug 
}//if(sevenSegModuleType == ModuleV140){  
if(sevenSegModuleType == ModuleV141){
 if(Debug7Seg){LOGLN("LED7 seg Board V14.1");}//Debug 
}//if(sevenSegModuleType == ModuleV14){

}

//////////////////////////////////////////////////////////////////////////
void shiftOut4Row(uint8_t dataPin0, uint8_t dataPin1, uint8_t dataPin2, uint8_t clockPin, uint8_t bitOrder, uint8_t val0, uint8_t val1, uint8_t val2) {
    uint8_t i;
    pinMode(dataPin0, OUTPUT);
    for(i = 0; i < 8; i++) {
        if(bitOrder == LSBFIRST){
            digitalWrite(dataPin0, !!(val0 & (1 << i)));
            digitalWrite(dataPin1, !!(val1 & (1 << i)));
            digitalWrite(dataPin2, !!(val2 & (1 << i)));
        }
        else{
            digitalWrite(dataPin0, !!(val0 & (1 << (7 - i))));
            digitalWrite(dataPin1, !!(val1 & (1 << (7 - i))));
            digitalWrite(dataPin2, !!(val2 & (1 << (7 - i))));
        }
        digitalWrite(clockPin, HIGH);
        digitalWrite(clockPin, LOW);
    }
}

void PrintSeg(uint8_t val0, uint8_t val1, uint8_t val2) {
  shiftOut4Row(segPin.segData1,segPin.segData2,segPin.segData3, segPin.segSHCP, LSBFIRST,  val0, val1, val2);
}

void AddData7Seg(byte type,bool boardbig);
void REMOVE_ZERO();

void displays(int Value0, int Value1, int OTvalue_H, int OTvalue_L,byte dataPin0, byte dataPin1,byte dataPin2,byte clockPin,byte latchPin,int dotint1) {
  
  byte *array0= new byte[4];//Lookline_PROG.Plan
  byte *array1= new byte[4];//Lookline_PROG.Result 
  byte *array2= new byte[4];//O.T
  if(sevenSegModuleType == ModuleV130){ 
    AddData7Seg(0,true);
    AddData7Seg(0,false);
  }
  if(sevenSegModuleType == ModuleV140){ 
    AddData7Seg(1,true);
    AddData7Seg(0,false);
  }
  if(sevenSegModuleType == ModuleV141){ 
    AddData7Seg(1,true);
    AddData7Seg(1,false);
  }
  for (byte i = 0; i < 4; i++) {
    //Lấy các chữ số từ phải quá trái
    array0[i] = (byte)(Value0 % 10UL);
    Value0 = (unsigned long)(Value0 /10UL);
  }
  for (byte i = 0; i < 4; i++) {
    //Lấy các chữ số từ phải quá trái
    array1[i] = (byte)(Value1 % 10UL);
    Value1 = (unsigned long)(Value1 /10UL);
  }
  //caculaOT();//not enb 
  if (OTvalue_H > 100)
  {   
  if(sevenSegModuleType == ModuleV130){ 
    array2[1] = Seg1[OTvalue_H % 10];
    array2[2] = Seg1[(OTvalue_H / 10) % 10];
    array2[3] = Seg1[((OTvalue_H / 100) % 10)];
  }//if(sevenSegModuleType == ModuleV141)
  if(sevenSegModuleType == ModuleV140){ 
    array2[2] = Seg1[OTvalue_H % 10];
    array2[1] = Seg1[(OTvalue_H / 10) % 10];
    array2[0] = Seg1[((OTvalue_H / 100) % 10)];
  }//if(sevenSegModuleType == ModuleV140){ 
    
  if(sevenSegModuleType == ModuleV141){ 
    array2[1] = Seg1[OTvalue_H % 10];
    array2[2] = Seg1[(OTvalue_H / 10) % 10];
    array2[3] = Seg1[((OTvalue_H / 100) % 10)];
  }//if(sevenSegModuleType == ModuleV141){ 
}
  else if (OTvalue_H > 10)
  {  
    if(sevenSegModuleType == ModuleV130){ 
      array2[1] = Seg1[(OTvalue_L/ 10)%10];
      array2[2] = Seg1[(OTvalue_H % 10)] | SEGMENT_H;
      array2[3] = Seg1[(OTvalue_H / 10)%10];
    }//if(sevenSegModuleType == ModuleV140){  
    if(sevenSegModuleType == ModuleV140){ 
      array2[2] = Seg1[(OTvalue_L/ 10)%10];
      array2[1] = Seg1[(OTvalue_H % 10)] | SEGMENT_H;
      array2[0] = Seg1[(OTvalue_H / 10)%10];
    }//if(sevenSegModuleType == ModuleV140){ 

    if(sevenSegModuleType == ModuleV141){ 
      array2[1] = Seg1[(OTvalue_L/ 10)%10];
      array2[2] = Seg1[(OTvalue_H % 10)] | SEGMENT_H;
      array2[3] = Seg1[(OTvalue_H / 10)%10];
    }//if(sevenSegModuleType == ModuleV141){
  }
  else
  {
    if(sevenSegModuleType == ModuleV130){ 
      array2[1] = Seg1[OTvalue_L% 10];
      array2[2] = Seg1[(OTvalue_L/ 10)%10];
      array2[3] = Seg1[(OTvalue_H % 10)] | SEGMENT_H;
    }//if(sevenSegModuleType == ModuleV140){ 
    if(sevenSegModuleType == ModuleV140){ 
      array2[2] = Seg1[OTvalue_L% 10];
      array2[1] = Seg1[(OTvalue_L/ 10)%10];
      array2[0] = Seg1[(OTvalue_H % 10)] | SEGMENT_H;
    }//if(sevenSegModuleType == ModuleV140){ 
      
    if(sevenSegModuleType == ModuleV141){ 
      array2[1] = Seg1[OTvalue_L% 10];
      array2[2] = Seg1[(OTvalue_L/ 10)%10];
      array2[3] = Seg1[(OTvalue_H % 10)] | SEGMENT_H;
    }//if(sevenSegModuleType == ModuleV141){ 
  }

  digitalWrite(latchPin, LOW);

  if(sevenSegModuleType == ModuleV130){ 
    for (byte i = 0; i < 4; i++) {
    if (array0[1] == 0 && array0[2] == 0 && array0[3] == 0) { array0[3] = 10; array0[2] = 10; array0[1] = 10;}//hang ngan = 0 thi ko hien thi
    if (array0[1] >= 0 && array0[2] == 0 && array0[3] == 0) { array0[3] = 10; array0[2] = 10;}//hang tram = 0 thi hang tram va ngan ko hien thi
    if (array0[1] >= 0 && array0[2] >= 0 && array0[3] == 0) { array0[3] = 10; }//hang chuc = 0 thi hang tram, ngan&&chuc ko hien thi
   //if(dotint == 0) shiftOut(dataPin, clockPin, LSBFIRST, Seg[array[i]]);//xuat data ra led ko có chấm
    if (array1[1] == 0 && array1[2] == 0 && array1[3] == 0) { array1[3] = 10; array1[2] = 10; array1[1] = 10;}//hang ngan = 0 thi ko hien thi
    if (array1[1] >= 0 && array1[2] == 0 && array1[3] == 0) { array1[3] = 10; array1[2] = 10;}//hang tram = 0 thi hang tram va ngan ko hien thi
    if (array1[1] >= 0 && array1[2] >= 0 && array1[3] == 0) { array1[3] = 10; }//hang chuc = 0 thi hang tram, ngan&&chuc ko hien thi
   //if(dotint == 0) shiftOut(dataPin, clockPin, LSBFIRST, Seg[array[i]]);//xuat data ra led ko có chấm
     if(i == dotint1 && dotint1 > 0){ 
       shiftOut4Row(dataPin0,dataPin1,dataPin2, clockPin, LSBFIRST, Seg[array0[i]], (Seg[array1[i]] | SEGMENT_H), array2[i]); }// vi tri dặt dấu chấm là "dotint0"
     else {
       shiftOut4Row(dataPin0,dataPin1,dataPin2, clockPin, LSBFIRST, Seg[array0[i]], Seg[array1[i]], array2[i]); 
     }
    }
  }//if(sevenSegModuleType == ModuleV30){ 
  if(sevenSegModuleType == ModuleV140){ 
    for (int i = 4; i > -1; i--){
    if (array0[1] == 0 && array0[2] == 0 && array0[3] == 0) { array0[3] = 10; array0[2] = 10; array0[1] = 10;}//hang ngan = 0 thi ko hien thi
    if (array0[1] >= 0 && array0[2] == 0 && array0[3] == 0) { array0[3] = 10; array0[2] = 10;}//hang tram = 0 thi hang tram va ngan ko hien thi
    if (array0[1] >= 0 && array0[2] >= 0 && array0[3] == 0) { array0[3] = 10; }//hang chuc = 0 thi hang tram, ngan&&chuc ko hien thi
   //if(dotint == 0) shiftOut(dataPin, clockPin, LSBFIRST, Seg[array[i]]);//xuat data ra led ko có chấm
    if (array1[1] == 0 && array1[2] == 0 && array1[3] == 0) { array1[3] = 10; array1[2] = 10; array1[1] = 10;}//hang ngan = 0 thi ko hien thi
    if (array1[1] >= 0 && array1[2] == 0 && array1[3] == 0) { array1[3] = 10; array1[2] = 10;}//hang tram = 0 thi hang tram va ngan ko hien thi
    if (array1[1] >= 0 && array1[2] >= 0 && array1[3] == 0) { array1[3] = 10; }//hang chuc = 0 thi hang tram, ngan&&chuc ko hien thi
   //if(dotint == 0) shiftOut(dataPin, clockPin, LSBFIRST, Seg[array[i]]);//xuat data ra led ko có chấm
     if(i == dotint1 && dotint1 > 0){ 
       shiftOut4Row(dataPin0,dataPin1,dataPin2, clockPin, LSBFIRST, Seg[array0[i]], (Seg[array1[i]] | SEGMENT_H), array2[i]); }// vi tri dặt dấu chấm là "dotint0"
     else {
       shiftOut4Row(dataPin0,dataPin1,dataPin2, clockPin, LSBFIRST, Seg[array0[i]], Seg[array1[i]], array2[i]);
     }
    }   
  }//if(sevenSegModuleType == ModuleV140 || sevenSegModuleType == ModuleV130){ 
  if(sevenSegModuleType == ModuleV141){ 
    for (byte i = 0; i < 4; i++) {
    if (array0[1] == 0 && array0[2] == 0 && array0[3] == 0) { array0[3] = 10; array0[2] = 10; array0[1] = 10;}//hang ngan = 0 thi ko hien thi
    if (array0[1] >= 0 && array0[2] == 0 && array0[3] == 0) { array0[3] = 10; array0[2] = 10;}//hang tram = 0 thi hang tram va ngan ko hien thi
    if (array0[1] >= 0 && array0[2] >= 0 && array0[3] == 0) { array0[3] = 10; }//hang chuc = 0 thi hang tram, ngan&&chuc ko hien thi
   //if(dotint == 0) shiftOut(dataPin, clockPin, LSBFIRST, Seg[array[i]]);//xuat data ra led ko có chấm
    if (array1[1] == 0 && array1[2] == 0 && array1[3] == 0) { array1[3] = 10; array1[2] = 10; array1[1] = 10;}//hang ngan = 0 thi ko hien thi
    if (array1[1] >= 0 && array1[2] == 0 && array1[3] == 0) { array1[3] = 10; array1[2] = 10;}//hang tram = 0 thi hang tram va ngan ko hien thi
    if (array1[1] >= 0 && array1[2] >= 0 && array1[3] == 0) { array1[3] = 10; }//hang chuc = 0 thi hang tram, ngan&&chuc ko hien thi
   //if(dotint == 0) shiftOut(dataPin, clockPin, LSBFIRST, Seg[array[i]]);//xuat data ra led ko có chấm
     if(i == dotint1 && dotint1 > 0){ 
       shiftOut4Row(dataPin0,dataPin1,dataPin2, clockPin, LSBFIRST, Seg[array0[i]], (Seg[array1[i]] | SEGMENT_H), array2[i]); }// vi tri dặt dấu chấm là "dotint0"
     else {
       shiftOut4Row(dataPin0,dataPin1,dataPin2, clockPin, LSBFIRST, Seg[array0[i]], Seg[array1[i]], array2[i]); 
     }
    }
  }//if(sevenSegModuleType == ModuleV141){ 
   
    
  digitalWrite(latchPin, HIGH);
  free(array0);free(array1);free(array2);
}


void connectting(int steps)
{
  if(steps == 0){
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);  
  }
  if(steps == 1){
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10],ConvertToLed7Segment('-'));  
  }
  if(steps == 2){
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10],ConvertToLed7Segment('-'));
    PrintSeg(Seg[10], Seg[10],ConvertToLed7Segment('-'));    
  }
  if(steps == 3){
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10],ConvertToLed7Segment('-'));
    PrintSeg(Seg[10], Seg[10],ConvertToLed7Segment('-'));
    PrintSeg(Seg[10], Seg[10],ConvertToLed7Segment('-'));    
  }
  if(steps == 4){
    PrintSeg(Seg[10], Seg[10],ConvertToLed7Segment('-'));
    PrintSeg(Seg[10], Seg[10],ConvertToLed7Segment('-'));
    PrintSeg(Seg[10], Seg[10],ConvertToLed7Segment('-'));
    PrintSeg(Seg[10], Seg[10],ConvertToLed7Segment('-'));   
  }
    
  latch();
}


void SaveDisplay()
{
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], ConvertToLed7Segment('-'), Seg1[10]);  
   delay(500);latch();   
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], ConvertToLed7Segment('-'), Seg1[10]);
    PrintSeg(Seg[10], ConvertToLed7Segment('-'), Seg1[10]);    
  delay(500);latch();
    PrintSeg(Seg[10], Seg[10], Seg[10]);
    PrintSeg(Seg[10], ConvertToLed7Segment('-'), Seg1[10]);
    PrintSeg(Seg[10], ConvertToLed7Segment('-'), Seg1[10]);
    PrintSeg(Seg[10], ConvertToLed7Segment('-'), Seg1[10]);   
  delay(500);latch();    
    PrintSeg(Seg[10], ConvertToLed7Segment(Lookline_PROG.SetupPro%10), Seg1[10]);
    PrintSeg(Seg[10], ConvertToLed7Segment('-'), Seg1[10]);
    PrintSeg(Seg[10], ConvertToLed7Segment('-'), Seg1[10]);
    PrintSeg(Seg[10], ConvertToLed7Segment('-'), Seg1[10]);     
  delay(500);latch();
}

void connected(int steps)
{
  if(steps == 0){
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], ConvertToLed7Segment('-'), Seg1[10]);  
  }
  if(steps == 1){
    PrintSeg(Seg[10], Seg[10], Seg1[10]); 
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], ConvertToLed7Segment('-'), Seg1[10]); 
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
  }
  if(steps == 2){
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], ConvertToLed7Segment('-'), Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);  
  }
  if(steps == 3){
    PrintSeg(Seg[10], ConvertToLed7Segment('-'), Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
    PrintSeg(Seg[10], Seg[10], Seg1[10]);  
  }
    
  latch();
}

void SetLoad()
{
  if(Lookline_PROG.SetupPro == 11){   
    Lookline_PROG.ValueSet0 = (Lookline_PROG.BoardID/100)%10;
    Lookline_PROG.ValueSet1 = (Lookline_PROG.BoardID/10)%10;
    Lookline_PROG.ValueSet2 = (Lookline_PROG.BoardID/1)%10;
  }
  if(Lookline_PROG.SetupPro == 10){ 
    Lookline_PROG.SetupPro = 0;
    #ifdef AutoRunStop 
    DateLookline_PROG.Time now = rtc.now(); 
    Lookline_PROG.ValueSet0 = (now.hour()/10)%10;
    Lookline_PROG.ValueSet1 = (now.hour()/1)%10;
    Lookline_PROG.ValueSet2 = (now.minute()/10)%10;
    Lookline_PROG.ValueSet3 = (now.minute()/1)%10;
    #endif//AutoRunStop
  }
  if(Lookline_PROG.SetupPro ==9){
    Lookline_PROG.ValueSet0 = (Lookline_PROG.Pass1/1000)%10;
    Lookline_PROG.ValueSet1 = (Lookline_PROG.Pass1/100)%10;
    Lookline_PROG.ValueSet2 = (Lookline_PROG.Pass1/10)%10;
    Lookline_PROG.ValueSet3 = (Lookline_PROG.Pass1/1)%10;
  }
  
  if(Lookline_PROG.SetupPro == 8){
    // Lookline_PROG.ValueSet0 = (Lookline_PROG.ResultSet/1000)%10;
    // Lookline_PROG.ValueSet1 = (Lookline_PROG.ResultSet/100)%10;
    // Lookline_PROG.ValueSet2 = (Lookline_PROG.ResultSet/10)%10;
    // Lookline_PROG.ValueSet3 = (Lookline_PROG.ResultSet/1)%10;
  }    
  if(Lookline_PROG.SetupPro == 7){
    // Lookline_PROG.ValueSet0 = (Lookline_PROG.PLanSet/1000)%10;
    // Lookline_PROG.ValueSet1 = (Lookline_PROG.PLanSet/100)%10;
    // Lookline_PROG.ValueSet2 = (Lookline_PROG.PLanSet/10)%10;
    // Lookline_PROG.ValueSet3 = (Lookline_PROG.PLanSet/1)%10;
  }
  if(Lookline_PROG.SetupPro == 6){
    // Lookline_PROG.ValueSet0 = (Lookline_PROG.Time/1000)%10;
    // Lookline_PROG.ValueSet1 = (Lookline_PROG.Time/100)%10;
    // Lookline_PROG.ValueSet2 = (Lookline_PROG.Time/10)%10;
    // Lookline_PROG.ValueSet3 = (Lookline_PROG.Time/1)%10;
  }
  if(Lookline_PROG.SetupPro == 5){
    // Lookline_PROG.ValueSet0 = (Lookline_PROG.PLAN/1000)%10;
    // Lookline_PROG.ValueSet1 = (Lookline_PROG.PLAN/100)%10;
    // Lookline_PROG.ValueSet2 = (Lookline_PROG.PLAN/10)%10;
    // Lookline_PROG.ValueSet3 = (Lookline_PROG.PLAN/1)%10;
  }
  if(Lookline_PROG.SetupPro == 3){
    // Lookline_PROG.ValueSet0 = (Lookline_PROG.PlanLimit/1000)%10;
    // Lookline_PROG.ValueSet1 = (Lookline_PROG.PlanLimit/100)%10;
    // Lookline_PROG.ValueSet2 = (Lookline_PROG.PlanLimit/10)%10;
    // Lookline_PROG.ValueSet3 = (Lookline_PROG.PlanLimit/1)%10;
  }  
  if(Lookline_PROG.SetupPro == 2){
    // Lookline_PROG.ValueSet0 = (Lookline_PROG.pcsInShift/1000)%10;
    // Lookline_PROG.ValueSet1 = (Lookline_PROG.pcsInShift/100)%10;
    // Lookline_PROG.ValueSet2 = (Lookline_PROG.pcsInShift/10)%10;
    // Lookline_PROG.ValueSet3 = (Lookline_PROG.pcsInShift/1)%10;
  }
  if(Lookline_PROG.SetupPro == 1){
    // Lookline_PROG.ValueSet0 = (Lookline_PROG.RESULT/1000)%10;
    // Lookline_PROG.ValueSet1 = (Lookline_PROG.RESULT/100)%10;
    // Lookline_PROG.ValueSet2 = (Lookline_PROG.RESULT/10)%10;
    // Lookline_PROG.ValueSet3 = (Lookline_PROG.RESULT/1)%10;
  }
  if(Lookline_PROG.SetupPro == 0){
    Lookline_PROG.ValueSet0 = 0;
    Lookline_PROG.ValueSet1 = 0;
    Lookline_PROG.ValueSet2 = 0;
    Lookline_PROG.ValueSet3 = 0;
  }
}

// void ProInc()
// {
//   Lookline_PROG.SetupPro++;if(lock == false){if(Lookline_PROG.SetupPro > 10)Lookline_PROG.SetupPro = 0;}if(lock){if(Lookline_PROG.SetupPro > 4)Lookline_PROG.SetupPro = 0;}
// }

// void SetSave()
// {  
//   switch(Lookline_PROG.SetupPro)
//   {
//   case 4:
//     if(Lookline_PROG.ValueSet0 == (Pass/1000)%10){
//       if(Lookline_PROG.ValueSet1 == (Pass/100)%10){
//         if(Lookline_PROG.ValueSet2 == (Pass/10)%10){
//           if(Lookline_PROG.ValueSet3 == (Pass/1)%10){
//             lock = false;LOGLN("unlock");SetLoad();delay(100);SaveDisplay();
//         }}}}
//     if(Lookline_PROG.ValueSet0 == (Lookline_PROG.Pass1/1000)%10){
//       if(Lookline_PROG.ValueSet1 == (Lookline_PROG.Pass1/100)%10){
//         if(Lookline_PROG.ValueSet2 == (Lookline_PROG.Pass1/10)%10){
//           if(Lookline_PROG.ValueSet3 == (Lookline_PROG.Pass1/1)%10){
//             lock = false;LOGLN("unlock");SetLoad();delay(100);SaveDisplay();
//         }}}}
//     if(Lookline_PROG.ValueSet0 == (Pass2/1000)%10){
//       if(Lookline_PROG.ValueSet1 == (Pass2/100)%10){
//         if(Lookline_PROG.ValueSet2 == (Pass2/10)%10){
//           if(Lookline_PROG.ValueSet3 == (Pass2/1)%10){
//            ///Wifi on
//               //countUPD++;if(countUPD > 10){countUPD = 0;
//               delay(100);
//                Mode = 2;WriteAll();LOGLN("SetOnline");esp_restart();
//               //}
//         }}}}        
//   break;  
//   case 11:
//     Lookline_PROG.BoardID = Lookline_PROG.ValueSet0 *100 + Lookline_PROG.ValueSet1 *10 + Lookline_PROG.ValueSet2;delay(100);WriteAll();SaveDisplay();
//     LOG("{\"Command\":\"ConFi\",\"ID\":");LOG(Lookline_PROG.BoardID);LOGLN("}");
//   break;
//   case 10:
//    #ifdef AutoRunStop
//    hh = Lookline_PROG.ValueSet0 *10 + Lookline_PROG.ValueSet1; mm = Lookline_PROG.ValueSet2 *10 + Lookline_PROG.ValueSet3;ss = 0;d = m = yy = 1;
//    rtc.adjust(DateLookline_PROG.Time(yy,m,d,hh,mm,ss));
//    #endif//AutoRunStop
//   break; 
//   case 9:
//     Lookline_PROG.Pass1 = Lookline_PROG.ValueSet0 *1000 + Lookline_PROG.ValueSet1 *100 + Lookline_PROG.ValueSet2 *10 + Lookline_PROG.ValueSet3;delay(100);WriteAll();SaveDisplay();
//   break;
//   case 8://Lookline_PROG.ResultSet
//     Lookline_PROG.ResultSet = Lookline_PROG.ValueSet0 *1000 + Lookline_PROG.ValueSet1 *100 + Lookline_PROG.ValueSet2 *10 + Lookline_PROG.ValueSet3;if(Lookline_PROG.ResultSet < 1){Lookline_PROG.ResultSet = 1;}delay(100);WriteAll();SaveDisplay();
//   break;  
//   case 7://Lookline_PROG.PlanSet
//     Lookline_PROG.PlanSet = Lookline_PROG.ValueSet0 *1000 + Lookline_PROG.ValueSet1 *100 + Lookline_PROG.ValueSet2 *10 + Lookline_PROG.ValueSet3;if(Lookline_PROG.PlanSet < 1){Lookline_PROG.PlanSet = 1;}delay(100);WriteAll();SaveDisplay();
//   break;
//   case 6://Lookline_PROG.Time 
  
//     Lookline_PROG.Time = Lookline_PROG.ValueSet0 *1000 + Lookline_PROG.ValueSet1 *100 + Lookline_PROG.ValueSet2 *10 + Lookline_PROG.ValueSet3;if(DotIn == 1){Lookline_PROG.Time = Lookline_PROG.Time *10;}if(DotIn == 2){Lookline_PROG.Time = Lookline_PROG.Time *100;}if(DotIn == 3){Lookline_PROG.Time = Lookline_PROG.Time *1000;} delay(100);WriteAll();SaveDisplay();
//   break;
//   case 5://Lookline_PROG.Plan
//     Lookline_PROG.Plan = Lookline_PROG.ValueSet0 *1000 + Lookline_PROG.ValueSet1 *100 + Lookline_PROG.ValueSet2 *10 + Lookline_PROG.ValueSet3;delay(100);WriteAll();SaveDisplay();
//   break;
//   case 3://Lookline_PROG.PlanLimit
//     Lookline_PROG.PlanLimit = Lookline_PROG.ValueSet0 *1000 + Lookline_PROG.ValueSet1 *100 + Lookline_PROG.ValueSet2 *10 + Lookline_PROG.ValueSet3;delay(100);WriteAll();SaveDisplay();
//   break;
//   case 2://Lookline_PROG.pcsInShift
//     Lookline_PROG.pcsInShift = Lookline_PROG.ValueSet0 *1000 + Lookline_PROG.ValueSet1 *100 + Lookline_PROG.ValueSet2 *10 + Lookline_PROG.ValueSet3;delay(100);WriteAll();SaveDisplay();
//   break;
//   case 1://Lookline_PROG.Result
//     Lookline_PROG.Result = Lookline_PROG.ValueSet0 *1000 + Lookline_PROG.ValueSet1 *100 + Lookline_PROG.ValueSet2 *10 + Lookline_PROG.ValueSet3;delay(100);WriteAll();SaveDisplay();
//   break;
//   }
// }

//#endif //ESP32Mode




uint8_t ConvertToLed7Segment(uint8_t value)
{
  uint8_t segmentValue = 0;
  //segmentValue = SEGMENT_H;
  if ((value == 0) || (value == '0'))      segmentValue = NUMBER_0;
  else if ((value == 1) || (value == '1'))  segmentValue = NUMBER_1;
  else if ((value == 2) || (value == '2'))  segmentValue = NUMBER_2;
  else if ((value == 3) || (value == '3'))  segmentValue = NUMBER_3;
  else if ((value == 4) || (value == '4'))   segmentValue = NUMBER_4;
  else if ((value == 5) || (value == '5'))  segmentValue = NUMBER_5;
  else if ((value == 6) || (value == '6'))   segmentValue = NUMBER_6;
  else if ((value == 7) || (value == '7'))  segmentValue = NUMBER_7;
  else if ((value == 8) || (value == '8')) segmentValue = NUMBER_8;
  else if ((value == 9) || (value == '9'))   segmentValue = NUMBER_9;
  else if (value == 'A') segmentValue = SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_E | SEGMENT_F | SEGMENT_G;
  else if (value == 'B') segmentValue = SEGMENT_F | SEGMENT_E | SEGMENT_D | SEGMENT_C | SEGMENT_G;
  else if (value == 'C') segmentValue = SEGMENT_A | SEGMENT_F | SEGMENT_E | SEGMENT_D;
  else if (value == 'c') segmentValue = SEGMENT_G | SEGMENT_E | SEGMENT_D;
  else if ((value == 'D') || (value == 'd')) segmentValue = SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_G;
  else if ((value == 'E') || (value == 'e'))  segmentValue = SEGMENT_A | SEGMENT_F | SEGMENT_G | SEGMENT_E | SEGMENT_D;
  else if ((value == 'F') || (value == 'f')) segmentValue = SEGMENT_E | SEGMENT_F | SEGMENT_A | SEGMENT_G;
  else if ((value == 'G') || (value == 'g')) segmentValue = SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_F | SEGMENT_G;
  else if (value == 'H')  segmentValue = SEGMENT_E | SEGMENT_F | SEGMENT_B | SEGMENT_C | SEGMENT_G;
  else if (value == 'h')  segmentValue = SEGMENT_E | SEGMENT_F | SEGMENT_G | SEGMENT_C;
  else if (value == 'I')  segmentValue = SEGMENT_F | SEGMENT_E;
  else if (value == 'i')  segmentValue = SEGMENT_C;
  else if (value == 'J')  segmentValue = SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E;
  else if ((value == 'K') || (value == 'k'))  segmentValue = SEGMENT_E | SEGMENT_F | SEGMENT_B | SEGMENT_C;
  else if (value == 'L')  segmentValue = SEGMENT_F | SEGMENT_E | SEGMENT_D;
  else if ((value == 'M') || (value == 'm'))segmentValue = SEGMENT_A | SEGMENT_C | SEGMENT_E;
  else if ((value == 'N') || (value == 'n'))segmentValue = SEGMENT_E | SEGMENT_G | SEGMENT_C;
  else if (value == 'O') segmentValue = SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F;
  else if (value == 'o') segmentValue = SEGMENT_G | SEGMENT_C | SEGMENT_D | SEGMENT_E;
  else if (value == 'P')segmentValue = SEGMENT_A | SEGMENT_B | SEGMENT_E |SEGMENT_F | SEGMENT_G | SEGMENT_F;
  else if (value == 'Q')segmentValue = SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_F | SEGMENT_G;
  else if (value == 'R')  segmentValue = SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_E | SEGMENT_F | SEGMENT_G;
  else if (value == 'r')  segmentValue = SEGMENT_E | SEGMENT_G;
  else if ((value == 'S') || (value == 's'))  segmentValue = SEGMENT_A | SEGMENT_F | SEGMENT_G | SEGMENT_C | SEGMENT_D;
  else if ((value == 'T') || (value == 't'))  segmentValue = SEGMENT_F | SEGMENT_G | SEGMENT_E | SEGMENT_D;
  else if (value == 'U')segmentValue = SEGMENT_F | SEGMENT_E | SEGMENT_D | SEGMENT_C | SEGMENT_B;
  else if ((value == 'V') || (value == 'v'))segmentValue = SEGMENT_E | SEGMENT_D | SEGMENT_C;


  else if (value == 'u')segmentValue = SEGMENT_E | SEGMENT_D | SEGMENT_C;
  else if ((value == 'X') || (value == 'x')) segmentValue = SEGMENT_E | SEGMENT_F | SEGMENT_G | SEGMENT_B | SEGMENT_C;
  else if ((value == 'Y') || (value == 'y')) segmentValue = SEGMENT_F | SEGMENT_G | SEGMENT_B | SEGMENT_C | SEGMENT_D;
  else if ((value == 'Z') || (value == 'z')) segmentValue = SEGMENT_A | SEGMENT_B | SEGMENT_G | SEGMENT_E | SEGMENT_D;
  else if ((value == 'W') || (value == 'w')) segmentValue = SEGMENT_E | SEGMENT_D | SEGMENT_C;
  else if (value == '.') segmentValue = SEGMENT_H;
  //else if (value == ('t' || LED7_74HC595_DIGIT_DOT))  segmentValue = SEGMENT_F | SEGMENT_G | SEGMENT_E | SEGMENT_D | SEGMENT_H;
  else if (value == '-')segmentValue = SEGMENT_G;
  else if (value == ' ')segmentValue = 0;
  else if (value == ALL_SEGMENT_OFF)segmentValue = 0;
  else if (value == LED_OFF)segmentValue = 0;
  return segmentValue;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AddData7Seg(byte type,bool boardbig)
{
if(type == 0){for(byte l = 0 ; l < 8 ; l++){LED7SEG[l] = V13_7SegCode[l];}}
if(type == 1){for(byte l = 0 ; l < 8 ; l++){LED7SEG[l] = V14_7SegCode[l];}}

SEGMENT_A = LED7SEG[0];
SEGMENT_B = LED7SEG[1];
SEGMENT_C = LED7SEG[2];
SEGMENT_D = LED7SEG[3];
SEGMENT_E = LED7SEG[4];
SEGMENT_F = LED7SEG[5];
SEGMENT_G = LED7SEG[6];
SEGMENT_H = LED7SEG[7];

NUMBER_0  =        (SEGMENT_D|SEGMENT_E|SEGMENT_F|SEGMENT_A|SEGMENT_B|SEGMENT_C);
NUMBER_1  =        (SEGMENT_B|SEGMENT_C);
NUMBER_2  =        (SEGMENT_D|SEGMENT_E|SEGMENT_G|SEGMENT_A|SEGMENT_B);
NUMBER_3  =        (SEGMENT_A|SEGMENT_B|SEGMENT_C|SEGMENT_D|SEGMENT_G);
NUMBER_4  =        (SEGMENT_C|SEGMENT_G|SEGMENT_B|SEGMENT_F);
NUMBER_5  =        (SEGMENT_A|SEGMENT_C|SEGMENT_D|SEGMENT_F|SEGMENT_G);
NUMBER_6  =        (SEGMENT_D|SEGMENT_C|SEGMENT_E|SEGMENT_A|SEGMENT_F|SEGMENT_G);
NUMBER_7  =        (SEGMENT_C|SEGMENT_A|SEGMENT_B);
NUMBER_8  =        (SEGMENT_D|SEGMENT_E|SEGMENT_F|SEGMENT_A|SEGMENT_B|SEGMENT_C|SEGMENT_G);
NUMBER_9  =        (SEGMENT_D|SEGMENT_B|SEGMENT_G|SEGMENT_C|SEGMENT_F|SEGMENT_A);
CHARACTER_A =       (SEGMENT_B|SEGMENT_C|SEGMENT_A|SEGMENT_E|SEGMENT_F|SEGMENT_G);
CHARACTER_b =       (SEGMENT_D|SEGMENT_C|SEGMENT_G|SEGMENT_F|SEGMENT_E);
CHARACTER_P =       (SEGMENT_B|SEGMENT_E|SEGMENT_G|SEGMENT_A|SEGMENT_F);
CHARACTER_O =       NUMBER_0;
CHARACTER_PASSWORD =       SEGMENT_G;

if(boardbig){
Seg[0] = NUMBER_0;
Seg[1] = NUMBER_1;
Seg[2] = NUMBER_2;
Seg[3] = NUMBER_3;
Seg[4] = NUMBER_4;
Seg[5] = NUMBER_5;
Seg[6] = NUMBER_6;
Seg[7] = NUMBER_7;
Seg[8] = NUMBER_8;
Seg[9] = NUMBER_9;
Seg[10] = LED_OFF;
}
else{
Seg1[0] = NUMBER_0;
Seg1[1] = NUMBER_1;
Seg1[2] = NUMBER_2;
Seg1[3] = NUMBER_3;
Seg1[4] = NUMBER_4;
Seg1[5] = NUMBER_5;
Seg1[6] = NUMBER_6;
Seg1[7] = NUMBER_7;
Seg1[8] = NUMBER_8;
Seg1[9] = NUMBER_9;
Seg1[10] = LED_OFF;
}
}



///////////////////////////////////////////////////// 7SEG FUNCTION /////////////////////////////////////////////////////////////
void latch()
{
    digitalWrite(segPin.segSTCP, HIGH); digitalWrite(segPin.segSTCP, LOW);
}
void OffAll()
{
    PrintSeg(Seg[10], Seg[10], Seg1[10]);
}
void testFlash()
{
  for(int j = 0 ; j < 3 ; j++)
  {
    for(int i = 0 ; i< 4 ; i++)
    {
      OffAll();
    }
    latch();delay(300);
    for(int i = 0 ; i< 4 ; i++)
    {
      PrintSeg(Seg[0], Seg[0], Seg1[0]);
    }
    latch();delay(300);
  }
    
}


void TestDisplay(int codes)
{
    int Result = 1111 * codes;
    // digitalWrite(segPin.segSTCP, LOW); digitalWrite(segPin.segSHCP, LOW);
    shiftOut4Row(segPin.segData1,segPin.segData2,segPin.segData3,segPin.segSHCP,LSBFIRST,ConvertToLed7Segment(Result / 1000 % 10),ConvertToLed7Segment(Result / 1000 % 10),Seg[Result / 1000 % 10]);
    shiftOut4Row(segPin.segData1,segPin.segData2,segPin.segData3,segPin.segSHCP,LSBFIRST,ConvertToLed7Segment(Result / 100 % 10),ConvertToLed7Segment(Result / 100 % 10),Seg[Result / 100 % 10]);
    shiftOut4Row(segPin.segData1,segPin.segData2,segPin.segData3,segPin.segSHCP,LSBFIRST,ConvertToLed7Segment((Result / 10) % 10),ConvertToLed7Segment((Result / 10) % 10),Seg[(Result / 10) % 10]);
    shiftOut4Row(segPin.segData1,segPin.segData2,segPin.segData3,segPin.segSHCP,LSBFIRST,ConvertToLed7Segment(Result % 10),ConvertToLed7Segment(Result % 10),Seg[Result % 10]);

    latch();
}

void SetupDisplay()
{
  
    PrintSeg(ConvertToLed7Segment('S'), ConvertToLed7Segment('-'), ConvertToLed7Segment('-'));
    PrintSeg(ConvertToLed7Segment('E'), ConvertToLed7Segment('-'), ConvertToLed7Segment('-'));
    PrintSeg(ConvertToLed7Segment('T'), ConvertToLed7Segment('U'), ConvertToLed7Segment('-'));
    PrintSeg(ConvertToLed7Segment('-'), ConvertToLed7Segment('P'), ConvertToLed7Segment('-'));
/*
    shiftOut(segPin.segData1, segPin.segSHCP, LSBFIRST, ConvertToLed7Segment('-'));
    shiftOut(segPin.segData1, segPin.segSHCP, LSBFIRST, ConvertToLed7Segment('T'));
    shiftOut(segPin.segData1, segPin.segSHCP, LSBFIRST, ConvertToLed7Segment('E'));
    shiftOut(segPin.segData1, segPin.segSHCP, LSBFIRST, ConvertToLed7Segment('S'));

    shiftOut(segPin.segData2, segPin.segSHCP, LSBFIRST, ConvertToLed7Segment('P'));
    shiftOut(segPin.segData2, segPin.segSHCP, LSBFIRST, ConvertToLed7Segment('U'));
    shiftOut(segPin.segData2, segPin.segSHCP, LSBFIRST, ConvertToLed7Segment('-'));
    shiftOut(segPin.segData2, segPin.segSHCP, LSBFIRST, ConvertToLed7Segment('-'));

    shiftOut(segPin.segData3, segPin.segSHCP, LSBFIRST, ConvertToLed7Segment('-'));
    shiftOut(segPin.segData3, segPin.segSHCP, LSBFIRST, ConvertToLed7Segment('-'));
    shiftOut(segPin.segData3, segPin.segSHCP, LSBFIRST, ConvertToLed7Segment('-'));
    */
  latch();
}  
#endif//sevenSegModuleType_
 