/*
  WIC.h - WIC class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef WIC_H
#define WIC_H
//be sure correct IDE and settings are used for ESP8266 or ESP32
#if !(defined( ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32))
#error Oops!  Make sure you have 'ESP8266 or ESP32' compatible board selected from the 'Tools -> Boards' menu.
#endif

#include "Arduino.h"
#include "config.h"
#include "FS.h"



    // extern void sendMessage(String message);

class WIC
{
public:
    bool Auto = false;
    bool Manual = false;
#ifdef ServerUpdateFW
    bool checkFirmware();
    bool downloadFirmware();
    void updateFromFS(fs::FS &fs);
    void performUpdate(Stream &updateSource, size_t updateSize);
    void CheckFWloop();
#endif//ServerUpdateFW

    WIC();
    void begin(uint16_t startdelayms = 100, uint16_t recoverydelayms = 100);
    void process();
    #ifdef LOOKLINE_UI
    void SetDebug(bool state);
    #endif//LooklineUI
    bool GetSetup();
    void SetSetup(bool state);
};
#endif
