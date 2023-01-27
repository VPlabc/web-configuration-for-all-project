

#include <Arduino.h>
#include "config.h"
#ifdef CircuitTesting_UI
#include "CircuitTesting.h"
#include "FS.h"
#include "SPIFFS.h"
#include "ClickButton.h"

#ifdef ESP_OLED_FEATURE
 #include <Wire.h>  
 #include "SSD1306.h" 
#include "OLEDDisplayUi.h"
#include "images.h"

// Pin InPut X10 -- 34
// Pin Input NO  -- 33
// Pin Input COM -- 32
// Pin Input NC  -- 35


byte Step = 0;
byte Page = 0;
byte LineTest = 0; // line test
bool Set = 0;
bool TestEnab = false;
byte TestStep = 0;
byte ReTest = 0;
byte TestOK = 0;
String TestStatus = "";
String status = "";
String Pins = "";
int wait = 0;

#ifdef CircuitTesting_UI
byte button = 15;//
byte LED_STT = 2;//
byte DataPin = 27;//
byte LacthPin =  26;
byte ClkPin = 25;
byte Input1 = 34;
//10(NO)-14/11(COM)-32/12(NC)-35/
byte Input_NO = 33;
byte Input_COM = 32;
byte Input_NC  = 35;
byte QuantityPin = 0;
String PinName[20] = { "1", "Line", "3", "4", "5", "6", "7", "NO", "COM", "NC", "Neutral", "PE", "", "", "", "", "", "", "", ""};
String PinName1[20] = { "KCTL1.1", "Line1", "KCTL1.3", "KCTL1.4", "KCTL1.5", "KCTL1.6", "KCTL1.7", "NO1", "COM1", "NC1", "Neutral", "PE", "", "", "", "", "", "", "", ""};
String PinName2[20] = { "KCTL2.1", "Line2", "KCTL2.3", "KCTL2.4", "KCTL2.5", "KCTL2.6", "KCTL2.7", "NO1", "COM1", "NC1", "Neutral", "PE", "", "", "", "", "", "", "", ""};
String PinName3[20] = { "KCTL3.1", "Line3", "KCTL3.3", "KCTL3.4", "KCTL3.5", "KCTL3.6", "KCTL3.7", "NO1", "COM1", "NC1", "Neutral", "PE", "", "", "", "", "", "", "", ""};
String ExcelData1 = "";
String ExcelData2 = "";
String ExcelData3 = "";
#endif//CircuitTesting_UI

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 21, 22);
// SH1106 display(0x3c, D3, D5);

OLEDDisplayUi ui     ( &display );





/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true

void CircuitTest::listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        LOG("- failed to open directory\n");
        return;
    }
    if(!root.isDirectory()){
        LOG(" - not a directory\n");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            LOG("  DIR : \n");
            LOG(file.name()+'\n');
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            LOG("  FILE: ");
            LOG(file.name());
            LOG("\tSIZE: ");
            LOG(file.size()+'\n');
        }
        file = root.openNextFile();
    }
}

void CircuitTest::readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        LOG("- failed to open file for reading\n");
        return;
    }

    LOG("- read from file:\n");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void CircuitTest::writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        LOG("- failed to open file for writing\n");
        return;
    }
    if(file.print(message)){
        LOG("- file written\n");
    } else {
        LOG("- write failed\n");
    }
    file.close();
}

void CircuitTest::appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        LOG("- failed to open file for appending\n");
        return;
    }
    if(file.print(message)){
        LOG("- message appended\n");
    } else {
        LOG("- append failed\n");
    }
    file.close();
}

void CircuitTest::renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\r\n", path1, path2);
    if (fs.rename(path1, path2)) {
        LOG("- file renamed\n");
    } else {
        LOG("- rename failed\n");
    }
}

void CircuitTest::deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        LOG("- file deleted\n");
    } else {
        LOG("- delete failed\n");
    }
}

void CircuitTest::testFileIO(fs::FS &fs, const char * path){
    Serial.printf("Testing file I/O with %s\r\n", path);

    static uint8_t buf[512];
    size_t len = 0;
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        LOG("- failed to open file for writing\n");
        return;
    }

    size_t i;
    LOG("- writing" );
    uint32_t start = millis();
    for(i=0; i<2048; i++){
        if ((i & 0x001F) == 0x001F){
          LOG(".");
        }
        file.write(buf, 512);
    }
    LOG("");
    uint32_t end = millis() - start;
    Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
    file.close();

    file = fs.open(path);
    start = millis();
    end = start;
    i = 0;
    if(file && !file.isDirectory()){
        len = file.size();
        size_t flen = len;
        start = millis();
        LOG("- reading" );
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            if ((i++ & 0x001F) == 0x001F){
              LOG(".");
            }
            len -= toRead;
        }
        LOG("");
        end = millis() - start;
        Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
        file.close();
    } else {
        LOG("- failed to open file for reading\n");
    }
}

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  //display->drawString(128, 0, String(millis()));
  display->drawString(128, 0, "VPlab");
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  if(Page == 0){display->drawString(0, 0, WiFi.localIP().toString());}
  else{display->drawString(0, 0, status);}

}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y

  if(Set == 0 && Page == 0){display->drawXbm(x + 34, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);}
  if(Set == 1 && Page == 0){
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 20 + y, "Server host");
    display->setFont(ArialMT_Plain_10);
    display->drawString(0 + x, 34 + y, "http://tester.local");
  }

}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demonstrates the 3 included default sizes. The fonts come from SSD1306Fonts.h file
  // Besides the default fonts there will be a program to convert TrueType fonts into this format
  
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    if(Set == 1 && Page == 1){
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 11 + y, String(TestStatus));
    display->setFont(ArialMT_Plain_10);
    display->drawString(0 + x, 34 + y, "Pin " + String(Step) + " --- Pin " + Pins);
  }
  if(Set == 0){
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 20 + y, "Circuit Tester");
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 34 + y, "Original ");
  }
}

void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    if(Set == 1 && Page == 2){
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 11 + y, String(TestStatus));
    display->setFont(ArialMT_Plain_10);
    display->drawString(0 + x, 34 + y, "Pin " + String(Step) + " --- Pin " + Pins);
  }
  if(Set == 0){
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 20 + y, "Circuit Tester");
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 34 + y, "Prototype ");
  }
}

void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
  display->setTextAlignment(TEXT_ALIGN_LEFT);
     display->setTextAlignment(TEXT_ALIGN_LEFT);
    if(Set == 1 && Page == 3){
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 11 + y, String(TestStatus));
    display->setFont(ArialMT_Plain_10);
    display->drawString(0 + x, 34 + y, "Pin " + String(Step) + " --- Pin " + Pins);
  }
  if(Set == 0){
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 20 + y, "Circuit Tester");
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 34 + y, "Offical ");
  }
 }

void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Text alignment demo
  display->setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 11 + y, "Step:");
  display->drawString(64 + x, 11 + y, String(Step));

  // The coordinates define the center of the text
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 22 + y, "Center aligned (64,22)");

  // The coordinates define the right end of the text
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128 + x, 33 + y, "Right aligned (128,33)");
}
void drawFrame6(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

}
void drawFrame7(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

}
void drawFrame8(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

}
void drawFrame9(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

}
void drawFrame10(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

}

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3, drawFrame4, drawFrame5, drawFrame6, drawFrame7, drawFrame8, drawFrame9, drawFrame10 };

// how many frames are there?
int frameCount = 4;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;

#endif//OLED_FEATURE



// the LED
const int ledPin = LED_STT;
int ledState = 0;

// the Button
const int buttonPin1 = button;
ClickButton button1(buttonPin1, LOW, CLICKBTN_PULLUP);

// Arbitrary LED function 
int LEDfunction = 0;

//////////////////////////////////////////////////////////////////////////

void CircuitTest::shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val0, uint8_t val1, uint8_t val2) {
    uint8_t i;

  digitalWrite(LacthPin, LOW);
    for(i = 0; i < 8; i++) {
        if(bitOrder == LSBFIRST){
            digitalWrite(dataPin, !!(val0 & (1 << i)));
        }
        else{
            digitalWrite(dataPin, !!(val0 & (1 << (7 - i))));
        }
        digitalWrite(clockPin, HIGH);
        digitalWrite(clockPin, LOW);
    }   
    for(i = 0; i < 8; i++) {
        if(bitOrder == LSBFIRST){
            digitalWrite(dataPin, !!(val1 & (1 << i)));
        }
        else{
            digitalWrite(dataPin, !!(val1 & (1 << (7 - i))));
        }
        digitalWrite(clockPin, HIGH);
        digitalWrite(clockPin, LOW);
    }    
    for(i = 0; i < 8; i++) {
        if(bitOrder == LSBFIRST){
            digitalWrite(dataPin, !!(val2 & (1 << i)));
        }
        else{
            digitalWrite(dataPin, !!(val2 & (1 << (7 - i))));
        }
        digitalWrite(clockPin, HIGH);
        digitalWrite(clockPin, LOW);
    }
}

void CircuitTest::PrintShift(uint8_t val0, uint8_t val1, uint8_t val2) {
  shiftOut(DataPin, ClkPin, LSBFIRST, val0, val1, val2);
}
void CircuitTest::ShowOut() {
  digitalWrite(LacthPin, HIGH);
  digitalWrite(LacthPin, LOW);
}

  long runs = 0;
void CircuitTest::Setup(){
    byte bbuf = 0;
    CONFIG::read_byte (EP_Pin_1, &bbuf);
    Input1    = bbuf;
    CONFIG::read_byte (EP_Pin_2, &bbuf);
    Input_NO  = bbuf;
    CONFIG::read_byte (EP_Pin_3, &bbuf);
    Input_COM = bbuf;
    CONFIG::read_byte (EP_Pin_4, &bbuf);
    Input_NC  = bbuf;
    CONFIG::read_byte (QuaPin, &bbuf);
    QuantityPin = bbuf;
    pinMode(button, INPUT_PULLUP);
    pinMode(Input1, INPUT_PULLUP);
    pinMode(Input_NO, INPUT_PULLUP);
    pinMode(Input_COM, INPUT_PULLUP);
    pinMode(Input_NC, INPUT_PULLUP);
    pinMode(LED_STT, OUTPUT);
    pinMode(DataPin, OUTPUT);
    pinMode(LacthPin, OUTPUT);
    pinMode(ClkPin, OUTPUT);
    Step = 0;
    button1.debounceTime   = 20;   // Debounce timer in ms
    button1.multiclickTime = 250;  // Time limit for multi clicks
    button1.longClickTime  = 1000; // time until "held-down clicks" register
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        LOG("SPIFFS Mount Failed\n");
        return;
    }
    
    // listDir(SPIFFS, "/", 0);
    // writeFile(SPIFFS, "/hello.txt", "Hello ");
    // appendFile(SPIFFS, "/hello.txt", "World!\r\n");
    // readFile(SPIFFS, "/hello.txt");
    // renameFile(SPIFFS, "/hello.txt", "/foo.txt");
    // readFile(SPIFFS, "/foo.txt");
    // deleteFile(SPIFFS, "/foo.txt");
    // testFileIO(SPIFFS, "/test.txt");
    // deleteFile(SPIFFS, "/test.txt");
    //LOG( "Test complete\n" );
    //OLEDDisplay
    // The ESP is capable of rendering 60fps in 80Mhz mode
	// but that won't give you much time for anything else
	// run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(60);

	// Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();

  display.flipScreenVertically();
    ui.disableAutoTransition();
  for(byte i = 0 ; i < 21; i++){
    digitalWrite(DataPin, LOW);
    digitalWrite(ClkPin, HIGH);
    digitalWrite(LacthPin, HIGH);
    digitalWrite(ClkPin, LOW);
    digitalWrite(LacthPin, LOW);
  }
  for(byte i = 0 ; i < 21; i++){
    CheckPin();Step++;
    delay(50);
  }
  Step = 0;CheckPin();
}
int timers = 0;
int times = 0;
int TimeUpdate = 5;
void CircuitTest::Loop(){
      // Update button state
  button1.Update();
timers++;
  // Save click codes in LEDfunction, as click codes are reset at next Update()
  if (button1.clicks != 0) LEDfunction = button1.clicks;
  if(button1.clicks == 1){//LOG("1 Click \n");
  //if(Set == 1 && Page == 4){Step++;CheckPin();}
  if(Set == 0){Page++;if(Page > frameCount - 1){Page = 0;}ui.transitionToFrame(Page);}
  if(Set == 1){LineTest++;if(LineTest > 2){LineTest = 0;}
    if(LineTest == 0){status = "Line 1";}
    if(LineTest == 1){status = "Line 2";}
    if(LineTest == 2){status = "Line 3";}
  }
  LEDfunction = 0;}

  // blink faster if double clicked
  if(LEDfunction == 2){//LOG("2 Click \n");
  //if(Set == 1 && Page == 4){Step--;CheckPin();}
  if(Set == 0){Page--;if(Page < 0 || Page > frameCount - 1){Page = 3;}ui.transitionToFrame(Page);}
  if(Set == 1){LineTest--;if(LineTest < 0){LineTest = 2;}
    if(LineTest == 0){status = "Line 1";}
    if(LineTest == 1){status = "Line 2";}
    if(LineTest == 2){status = "Line 3";}
  }  
  LEDfunction = 0;}

  // blink even faster if triple clicked
  if(LEDfunction == 3) {//LOG("3 Click \n");
  LEDfunction = 0;status = "3 Click";
    }

  // blink even faster if triple clicked
  if(LEDfunction == 4) {//LOG("4 Click \n");
  LEDfunction = 0;status = "4 Click";}
  
  // blink even faster if triple clicked
  if(LEDfunction == 5) {//LOG("5 Click \n");
  LEDfunction = 0;status = "5 Click";}

  // slow blink (must hold down button. 1 second long blinks)
  if(LEDfunction == -1){//LOG("Long  hold 1s\n");
  LEDfunction = 0;
    if(Page == 4 && Set == 1){Set = !Set;if(Set){status = "Select Tab 4";}else{status = "Back";}}
    if((Page == 1 || Page == 2 || Page == 3) && Set == 1){TestEnab = !TestEnab;
        if(TestEnab){TimeUpdate = 20;TestStatus = "Test Starting...";status = "";digitalWrite(ledPin,HIGH);
            if(LineTest == 0){ExcelData1 = "\n\nLine 1\nPin In,Pin Out,Status\n";}
            if(LineTest == 1){ExcelData2 = "\n\nLine 2\nPin In,Pin Out,Status\n";}
            if(LineTest == 2){ExcelData3 = "\n\nLine 3\nPin In,Pin Out,Status\n";}
        }
        else{TestStatus = "Test Stop!";status = "Test Stop";ReTest = TestStep = 0;}
    }
  }

  // slower blink (must hold down button. 2 second loong blinks)
  if(LEDfunction == -2){//LOG("Long  hold 2s\n");
  LEDfunction = 0;
    Set = !Set;
    if(Set == true &&  Page == 1){status = "Select Original ";TestStatus = "Ready";}if(Set == false &&  Page == 1){status = "Back ";}
    if(Set == true &&  Page == 2){status = "Select Prototype ";TestStatus = "Ready";}if(Set == false &&  Page == 2){status = "Back ";}
    if(Set == true &&  Page == 3){status = "Select Offical ";TestStatus = "Ready";}if(Set == false &&  Page == 3){status = "Back ";}

  }

  if(LEDfunction == -3){//LOG("Long  hold 2s\n");
  LEDfunction = 0;ui.enableAutoTransition();status = "Auto slide";}

  // even slower blink (must hold down button. 3 second looong blinks)
  if(LEDfunction == -4){//LOG("Long  hold 3s\n");
  LEDfunction = 0;ui.disableAutoTransition();status = "Off slide";}


  // update the LED
  static uint32_t last_update = 0;
    uint32_t now_ = millis();
    if (now_ - last_update > (TimeUpdate * 100)) {
        if(TestEnab == false){TimeUpdate = 5;last_update = now_;digitalWrite(ledPin,!digitalRead(ledPin));}
        if(TestEnab){
            wait++;if(wait > 10){wait = 0;testAllPin(TestStep);}
            if(TestStep > QuantityPin){TestStep = 0;TestEnab = 0;
            if(LineTest == 0){TestStatus = "Line 1 Done";}
            if(LineTest == 1){TestStatus = "Line 2 Done";}
            if(LineTest == 2){TestStatus = "Line 3 Done";}
            //ExcelData;
            if(Page == 1){
                if(LineTest == 0){writeFile(SPIFFS, "/Original Board.csv", ExcelData1.c_str());}
                if(LineTest == 1){appendFile(SPIFFS, "/Original Board.csv", ExcelData2.c_str());}
                if(LineTest == 2){appendFile(SPIFFS, "/Original Board.csv", ExcelData3.c_str());}
            }
            if(Page == 2){
                if(LineTest == 0){writeFile(SPIFFS, "/Prototype Board.csv", ExcelData1.c_str());}
                if(LineTest == 1){appendFile(SPIFFS, "/Prototype Board.csv", ExcelData2.c_str());}
                if(LineTest == 2){appendFile(SPIFFS, "/Prototype Board.csv", ExcelData3.c_str());}
            }
            if(Page == 3){
                if(LineTest == 0){writeFile(SPIFFS, "/Offical Board.csv", ExcelData1.c_str());}
                if(LineTest == 1){appendFile(SPIFFS, "/Offical Board.csv", ExcelData2.c_str());}
                if(LineTest == 2){appendFile(SPIFFS, "/Offical Board.csv", ExcelData3.c_str());}
            }
            PrintShift(SEGMENT_0, SEGMENT_0, SEGMENT_0);ShowOut();
            }
        }
    if(Set == 1 && (Page == 1 ||Page == 2 ||Page == 3)){
        if(LineTest == 0){status = "Line 1";}
        if(LineTest == 1){status = "Line 2";}
        if(LineTest == 2){status = "Line 3";}
    }  
    }
//   static uint32_t last_status_update = 0;
//     uint32_t now_status = millis();
//     if (now_status - last_status_update > (50 * 1000)) {
//         status="";
//     }
//   static uint32_t last_test_update = 0;
//     uint32_t now_test = millis();
//     if (now_test - last_test_update > (100 * 1000)) {

//     }
//   static uint32_t last_tests_update = 0;
//     uint32_t now_tests = millis();
//     if (now_tests - last_tests_update > (10 * 100)) {
//         if(TestEnab){

//         }
//     }
    int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }
}
///ghi file excel 
bool done = false;
bool done1 = false;
void INPUT1(byte steps) {
if(digitalRead(Input1) == 0 && steps > 0){Pins = PinName[steps-1];digitalWrite(ledPin,LOW);if(done == false){done = true;TestOK++;}}
}
void INPUT_NO(byte steps){
    if(digitalRead(Input_NO) == 0 && steps > 0){Pins = "NO";digitalWrite(ledPin,LOW);if(done == false){done = true;TestOK++;}}
}
void INPUT_COM(byte steps){
    if(digitalRead(Input_COM) == 0 && steps > 0){Pins = "COM";digitalWrite(ledPin,LOW);if(done == false){done = true;TestOK++;}}
}
void INPUT_NC(byte steps){
    if(digitalRead(Input_NC) == 0 && steps > 0){Pins = "NC";digitalWrite(ledPin,LOW);if(done == false){done = true;TestOK++;}}
}
void CircuitTest::testAllPin(byte steps){
if(ReTest > 10){ReTest = 0;TestStep++;}
if(ReTest == 0 || ReTest == 2 || ReTest == 7){ done1 = false;PrintShift(SEGMENT_0, SEGMENT_0, SEGMENT_0);ShowOut();digitalWrite(ledPin,HIGH);}
if((ReTest == 1 || ReTest == 3)&& done1 == false){
    done1 = true;Step = steps;CheckPin();done = false;
    INPUT1(steps);INPUT_NO(steps);INPUT_COM(steps);INPUT_NC(steps);
}
if(ReTest == 8){
                // Pin In           Pin Out           Status
    if(steps > 0){
        if(LineTest == 0){ExcelData1 += String(steps) + "," + Pins + "," + TestStatus + "\n";}
        if(LineTest == 1){ExcelData2 += String(steps) + "," + Pins + "," + TestStatus + "\n";}
        if(LineTest == 2){ExcelData3 += String(steps) + "," + Pins + "," + TestStatus + "\n";}
    }
    TestStatus = "";Pins = "";TestOK = 0;done = false;
}
if(TestOK == 2){TestStatus = "OK";}
ReTest++;
}

void CircuitTest::CheckPin(){
    if(Step == 0){ PrintShift(SEGMENT_0, SEGMENT_0, SEGMENT_0);ShowOut();}
    if(Step == 1){ PrintShift(SEGMENT_7, SEGMENT_0, SEGMENT_0);ShowOut();}
    if(Step == 2){ PrintShift(SEGMENT_6, SEGMENT_0, SEGMENT_0);ShowOut();}
    if(Step == 3){ PrintShift(SEGMENT_0, SEGMENT_2, SEGMENT_0);ShowOut();}
    if(Step == 4){ PrintShift(SEGMENT_0, SEGMENT_4, SEGMENT_0);ShowOut();}
    if(Step == 5){ PrintShift(SEGMENT_0, SEGMENT_7, SEGMENT_0);ShowOut();}
    if(Step == 6){ PrintShift(SEGMENT_0, SEGMENT_6, SEGMENT_0);ShowOut();}
    if(Step == 7){ PrintShift(SEGMENT_0, SEGMENT_0, SEGMENT_2);ShowOut();}
    if(Step == 8){ PrintShift(SEGMENT_0, SEGMENT_0, SEGMENT_4);ShowOut();}
    if(Step == 9){ PrintShift(SEGMENT_0, SEGMENT_0, SEGMENT_7);ShowOut();}
    if(Step == 10){PrintShift(SEGMENT_0, SEGMENT_0, SEGMENT_6);ShowOut();}
    if(Step == 11){PrintShift(SEGMENT_8, SEGMENT_0, SEGMENT_0);ShowOut();}
    if(Step == 12){PrintShift(SEGMENT_5, SEGMENT_0, SEGMENT_0);ShowOut();}
    if(Step == 13){PrintShift(SEGMENT_0, SEGMENT_1, SEGMENT_0);ShowOut();}
    if(Step == 14){PrintShift(SEGMENT_0, SEGMENT_3, SEGMENT_0);ShowOut();}
    if(Step == 15){PrintShift(SEGMENT_0, SEGMENT_8, SEGMENT_0);ShowOut();}
    if(Step == 16){PrintShift(SEGMENT_0, SEGMENT_5, SEGMENT_0);ShowOut();}
    if(Step == 17){PrintShift(SEGMENT_0, SEGMENT_0, SEGMENT_1);ShowOut();}
    if(Step == 18){PrintShift(SEGMENT_0, SEGMENT_0, SEGMENT_3);ShowOut();}
    if(Step == 19){PrintShift(SEGMENT_0, SEGMENT_0, SEGMENT_8);ShowOut();}
    if(Step == 20){PrintShift(SEGMENT_0, SEGMENT_0, SEGMENT_5);ShowOut();}
    if(Step == 21){PrintShift(SEGMENT_1, SEGMENT_0, SEGMENT_0);ShowOut();}
    if(Step == 23){PrintShift(SEGMENT_2, SEGMENT_0, SEGMENT_0);ShowOut();}
    if(Step == 24){PrintShift(SEGMENT_3, SEGMENT_0, SEGMENT_0);ShowOut();}
    if(Step == 25){PrintShift(SEGMENT_4, SEGMENT_0, SEGMENT_0);ShowOut();}
}
#endif//Circuiting