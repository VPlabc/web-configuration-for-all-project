#ifndef SIMPLE_MODBUS_SLAVE_H
#define SIMPLE_MODBUS_SLAVE_H

/*
 SimpleModbusSlave allows you to communicate
 to any slave using the Modbus RTU protocol.
 
 The crc calculation is based on the work published 
 by jpmzometa at 
 http://sites.google.com/site/jpmzometa/arduino-mbrt
 
 By Juan Bester : bester.juan@gmail.com
 
 The functions implemented are functions 3 and 16.
 read holding registers and preset multiple registers
 of the Modbus RTU Protocol, to be used over the Arduino serial connection.
 
 This implementation DOES NOT fully comply with the Modbus specifications.
 
 Specifically the frame time out have not been implemented according
 to Modbus standards. The code does however combine the check for
 inter character time out and frame time out by incorporating a maximum
 time out allowable when reading from the message stream.
 
 These library of functions are designed to enable a program send and
 receive data from a device that communicates using the Modbus protocol.
 
 SimpleModbusSlave implements an unsigned int return value on a call to modbus_update().
 This value is the total error count since the slave started. It's useful for fault finding.
 
 This code is for a Modbus slave implementing functions 3 and 16
 function 3: Reads the binary contents of holding registers (4X references)
 function 16: Presets values into a sequence of holding registers (4X references)
 
 All the functions share the same register array.
 
 Exception responses:
 1 ILLEGAL FUNCTION
 2 ILLEGAL DATA ADDRESS
 3 ILLEGAL DATA VALUE
 
 Note:  
 The Arduino serial ring buffer is 128 bytes or 64 registers.
 Most of the time you will connect the arduino to a master via serial
 using a MAX485 or similar.
 
 In a function 3 request the master will attempt to read from your
 slave and since 5 bytes is already used for ID, FUNCTION, NO OF BYTES
 and two BYTES CRC the master can only request 122 bytes or 61 registers.
 
 In a function 16 request the master will attempt to write to your 
 slave and since a 9 bytes is already used for ID, FUNCTION, ADDRESS, 
 NO OF REGISTERS, NO OF BYTES and two BYTES CRC the master can only write
 118 bytes or 59 registers.
 
 Using the FTDI converter ic the maximum bytes you can send is limited 
 to its internal buffer which is 60 bytes or 30 unsigned int registers. 
 
 Thus:
 
 In a function 3 request the master will attempt to read from your
 slave and since 5 bytes is already used for ID, FUNCTION, NO OF BYTES
 and two BYTES CRC the master can only request 54 bytes or 27 registers.
 
 In a function 16 request the master will attempt to write to your 
 slave and since a 9 bytes is already used for ID, FUNCTION, ADDRESS, 
 NO OF REGISTERS, NO OF BYTES and two BYTES CRC the master can only write
 50 bytes or 25 registers.
  
 Since it is assumed that you will mostly use the Arduino to connect to a 
 master without using a USB to Serial converter the internal buffer is set
 the same as the Arduino Serial ring buffer which is 128 bytes.
 
 The functions included here have been derived from the 
 Modbus Specifications and Implementation Guides
 
 http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf
 http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
 http://www.modbus.org/docs/PI_MBUS_300.pdf
*/

#include "Arduino.h"
// #include "config.h"
// #define ESP32_C3

#if defined(ESP32_C3)
#define ModbusSerial Serial1
#elif defined(ESP32)
#define ModbusSerial Serial2
#else

#include <SoftwareSerial.h>
 SoftwareSerial Slave_serial_ESP1(5,4);
#define ModbusSerial Slave_serial_ESP1
#endif//ESP32
// function definitions
//void modbus_configure(long baud, byte _slaveID, byte _TxEnablePins, unsigned int _holdingRegsSize, unsigned char _lowLatency);
//unsigned int modbus_update(unsigned int *holdingRegs);


#define BUFFER_SIZE 256//128 byte //64 register

typedef struct
{

private:
  unsigned int T1_5; // inter character time out
  unsigned int T3_5; // frame delay
  Stream *modemStream;
  // function definitions
  //void exceptionResponse(unsigned char exception);
  //unsigned int calculateCRC(unsigned char bufferSize);
  //void sendPacket(unsigned char bufferSize);

  void exceptionResponse(unsigned char exception)
  {
    errorCount++;       // each call to exceptionResponse() will increment the errorCount
    if (!broadcastFlag) // don't respond if its a broadcast message
    {
      frame[0] = slaveID;
      frame[1] = (function | 0x80); // set the MSB bit high, informs the master of an exception
      frame[2] = exception;
      unsigned int crc16 = calculateCRC(3); // ID, function + 0x80, exception code == 3 bytes
      frame[3] = crc16 >> 8;
      frame[4] = crc16 & 0xFF;
      sendPacket(5); // exception response is always 5 bytes ID, function + 0x80, exception code, 2 bytes crc
    }
  }

  unsigned int calculateCRC(byte bufferSize)
  {
    unsigned int temp, temp2, flag;
    temp = 0xFFFF;
    for (unsigned char i = 0; i < bufferSize; i++)
    {
      temp = temp ^ frame[i];
      for (unsigned char j = 1; j <= 8; j++)
      {
        flag = temp & 0x0001;
        temp >>= 1;
        if (flag)
          temp ^= 0xA001;
      }
    }
    // Reverse byte order.
    temp2 = temp >> 8;
    temp = (temp << 8) | temp2;
    temp &= 0xFFFF;
    return temp; // the returned value is already swopped - crcLo byte is first & crcHi byte is last
  }

  void sendPacket(unsigned char bufferSize)
  {
    if (TxEnablePins > 1)
      digitalWrite(TxEnablePins, HIGH);

    for (unsigned char i = 0; i < bufferSize; i++)
      ModbusSerial.write(frame[i]);// Serial.write(frame[i]);

    ModbusSerial.flush();// Serial.flush();

    // allow a frame delay to indicate end of transmission
    delayMicroseconds(T3_5);

    if (TxEnablePins > 1)
      digitalWrite(TxEnablePins, LOW);
  }

public:
  // frame[] is used to recieve and transmit packages.
  // The maximum serial ring buffer size is 128
  unsigned char frame[BUFFER_SIZE];
  unsigned int holdingRegsSize; // size of the register array
  unsigned char broadcastFlag;
  unsigned char slaveID;
  unsigned char function;
  unsigned char TxEnablePins;
  unsigned int errorCount;

  void modbus_configure(long baud, unsigned char _slaveID, unsigned char _TxEnablePins, unsigned int _holdingRegsSize, unsigned char _lowLatency)
  {
    slaveID = _slaveID;
    // modemStream = &_modemStream;
  
    // ModbusSerial.begin(baud);

    if (_TxEnablePins > 1)
    { // pin 0 & pin 1 are reserved for RX/TX. To disable set txenpin < 2
      TxEnablePins = _TxEnablePins;
      pinMode(TxEnablePins, OUTPUT);
      digitalWrite(TxEnablePins, LOW);
    }

    // Modbus states that a baud rate higher than 19200 must use a fixed 750 us
    // for inter character time out and 1.75 ms for a frame delay.
    // For baud rates below 19200 the timeing is more critical and has to be calculated.
    // E.g. 9600 baud in a 10 bit packet is 960 characters per second
    // In milliseconds this will be 960characters per 1000ms. So for 1 character
    // 1000ms/960characters is 1.04167ms per character and finaly modbus states an
    // intercharacter must be 1.5T or 1.5 times longer than a normal character and thus
    // 1.5T = 1.04167ms * 1.5 = 1.5625ms. A frame delay is 3.5T.
    // Added experimental low latency delays. This makes the implementation
    // non-standard but practically it works with all major modbus master implementations.

    if (baud == 1000000 && _lowLatency)
    {
      T1_5 = 1;
      T3_5 = 10;
    }
    else if (baud >= 115200 && _lowLatency)
    {
      T1_5 = 75;
      T3_5 = 175;
    }
    else if (baud > 19200)
    {
      T1_5 = 750;
      T3_5 = 1750;
    }
    else
    {
      T1_5 = 15000000 / baud; // 1T * 1.5 = T1.5
      T3_5 = 35000000 / baud; // 1T * 3.5 = T3.5
    }

    holdingRegsSize = _holdingRegsSize;
    errorCount = 0; // initialize errorCount
  }

  unsigned int modbus_update(unsigned int *holdingRegs)
  {
    unsigned char buffer = 0;
    unsigned char overflow = 0;

    while (ModbusSerial.available()/*Serial.available()*/)
    {
      // The maximum number of bytes is limited to the serial buffer size of 128 bytes
      // If more bytes is received than the BUFFER_SIZE the overflow flag will be set and the
      // serial buffer will be red untill all the data is cleared from the receive buffer.
      if (overflow)
        ModbusSerial.read(); //Serial.read();
      else
      {
        if (buffer == BUFFER_SIZE)
          overflow = 1;
        frame[buffer] = ModbusSerial.read(); //Serial.read();
        buffer++;
      }
      delayMicroseconds(T1_5); // inter character time out
    }

    // If an overflow occurred increment the errorCount
    // variable and return to the main sketch without
    // responding to the request i.e. force a timeout
    if (overflow)
      return errorCount++;

    // The minimum request packet is 8 bytes for function 3 & 16
    if (buffer > 6)
    {
      unsigned char id = frame[0];

      broadcastFlag = 0;

      if (id == 0)
        broadcastFlag = 1;

      if (id == slaveID || broadcastFlag) // if the recieved ID matches the slaveID or broadcasting id (0), continue
      {
        unsigned int crc = ((frame[buffer - 2] << 8) | frame[buffer - 1]); // combine the crc Low & High bytes
        if (calculateCRC(buffer - 2) == crc)                               // if the calculated crc matches the recieved crc continue
        {
          function = frame[1];
          unsigned int startingAddress = ((frame[2] << 8) | frame[3]); // combine the starting address bytes
          unsigned int no_of_registers = ((frame[4] << 8) | frame[5]); // combine the number of register bytes
          unsigned int maxData = startingAddress + no_of_registers;
          unsigned char index;
          unsigned char address;
          unsigned int crc16;

          // broadcasting is not supported for function 3
          if (!broadcastFlag && (function == 3))
          {
            if (startingAddress < holdingRegsSize) // check exception 2 ILLEGAL DATA ADDRESS
            {
              if (maxData <= holdingRegsSize) // check exception 3 ILLEGAL DATA VALUE
              {
                unsigned char noOfBytes = no_of_registers * 2;
                unsigned char responseFrameSize = 5 + noOfBytes; // ID, function, noOfBytes, (dataLo + dataHi) * number of registers, crcLo, crcHi
                frame[0] = slaveID;
                frame[1] = function;
                frame[2] = noOfBytes;
                address = 3; // PDU starts at the 4th byte
                unsigned int temp;

                for (index = startingAddress; index < maxData; index++)
                {
                  temp = holdingRegs[index];
                  frame[address] = temp >> 8; // split the register into 2 bytes
                  address++;
                  frame[address] = temp & 0xFF;
                  address++;
                }

                crc16 = calculateCRC(responseFrameSize - 2);
                frame[responseFrameSize - 2] = crc16 >> 8; // split crc into 2 bytes
                frame[responseFrameSize - 1] = crc16 & 0xFF;
                sendPacket(responseFrameSize);
              }
              else
                exceptionResponse(3); // exception 3 ILLEGAL DATA VALUE
            }
            else
              exceptionResponse(2); // exception 2 ILLEGAL DATA ADDRESS
          }
          else if (function == 6)
          {
            if (startingAddress < holdingRegsSize) // check exception 2 ILLEGAL DATA ADDRESS
            {
              unsigned int startingAddress = ((frame[2] << 8) | frame[3]);
              unsigned int regStatus = ((frame[4] << 8) | frame[5]);
              unsigned char responseFrameSize = 8;

              holdingRegs[startingAddress] = regStatus;

              crc16 = calculateCRC(responseFrameSize - 2);
              frame[responseFrameSize - 2] = crc16 >> 8; // split crc into 2 bytes
              frame[responseFrameSize - 1] = crc16 & 0xFF;
              sendPacket(responseFrameSize);
            }
            else
              exceptionResponse(2); // exception 2 ILLEGAL DATA ADDRESS
          }
          else if (function == 16)
          {
            // check if the recieved number of bytes matches the calculated bytes minus the request bytes
            // id + function + (2 * address bytes) + (2 * no of register bytes) + byte count + (2 * CRC bytes) = 9 bytes
            if (frame[6] == (buffer - 9))
            {
              if (startingAddress < holdingRegsSize) // check exception 2 ILLEGAL DATA ADDRESS
              {
                if (maxData <= holdingRegsSize) // check exception 3 ILLEGAL DATA VALUE
                {
                  address = 7; // start at the 8th byte in the frame

                  for (index = startingAddress; index < maxData; index++)
                  {
                    holdingRegs[index] = ((frame[address] << 8) | frame[address + 1]);
                    address += 2;
                  }

                  // only the first 6 bytes are used for CRC calculation
                  crc16 = calculateCRC(6);
                  frame[6] = crc16 >> 8; // split crc into 2 bytes
                  frame[7] = crc16 & 0xFF;

                  // a function 16 response is an echo of the first 6 bytes from the request + 2 crc bytes
                  if (!broadcastFlag) // don't respond if it's a broadcast message
                    sendPacket(8);
                }
                else
                  exceptionResponse(3); // exception 3 ILLEGAL DATA VALUE
              }
              else
                exceptionResponse(2); // exception 2 ILLEGAL DATA ADDRESS
            }
            else
              errorCount++; // corrupted packet
          }
          else
            exceptionResponse(1); // exception 1 ILLEGAL FUNCTION
        }
        else // checksum failed
          errorCount++;
      } // incorrect id
    }
    else if (buffer > 0 && buffer < 8)
      errorCount++; // corrupted packet

    return errorCount;
  }

} SimpleModbusSlave;

#endif
