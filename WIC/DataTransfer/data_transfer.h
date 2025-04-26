#ifndef _DATA_TRANSFER_H_
#define _DATA_TRANSFER_H_

#include <Arduino.h>

//
typedef struct
{
    unsigned int TotalValve;
    unsigned int StartAddress;
    //unsigned int EndAddress;
    //unsigned int ValveId;
    unsigned char ValveId;
    unsigned int RegisReadNo;
    unsigned int RegisWriteNo;
    unsigned int *RequestBuffer;
    unsigned int *RespondBuffer;

    //
    void setup(unsigned int _totalValve, unsigned int _startAddress, unsigned int *_requestBuffer)
    {
        TotalValve = _totalValve;
        StartAddress = _startAddress;
        RequestBuffer = _requestBuffer;
    }

    unsigned int GetRegisReadNo(int _valveID)
    {
        //RegisReadNo = (_valveID + 1) * 2 + StartAddress;
        RegisReadNo = StartAddress + _valveID;
        return RegisReadNo;
    }

    unsigned int GetRegisWriteNo(int _valveID)
    {
        //RegisWriteNo = (_valveID + 1) * 2 + StartAddress + 1;
        RegisReadNo = StartAddress + _valveID + 100;
        return RegisWriteNo;
    }

} DataValveTransfer;

typedef struct
{
    byte PowerCap;
    boolean OpenState;
    boolean CloseState;
    boolean Connected;
} RespondValveData;

typedef struct
{
    boolean Bit0;
    boolean Bit1;
    boolean Bit2;
    boolean Bit3;
    boolean Bit4;
    boolean Bit5;
    boolean Bit6;
    boolean Bit7;
} RespondBitData;

typedef struct
{
    byte Byte1;
    byte Byte2;
} RespondByte;

typedef struct
{
    char char1;
    char char2;
} RespondChar;

typedef struct
{
    //boolean SetValve;
    union
    {
        struct
        {
            unsigned char sleep_time : 4;
            bool close_cmd : 1;
            bool open_cmd : 1;
            unsigned _reserved : 10;
        };
        uint16_t register_data;
    };
} RequestValveData;

typedef struct
{
    RespondValveData DecodeRespond(unsigned int _data)
    {
        RespondValveData ret;
        ret.PowerCap = _data & B11111111;

        //Serial.print("PowerCap "); Serial.println(ret.PowerCap);
        ret.OpenState = (_data >> 9) & B00000001;

        //Serial.print("OpenState "); Serial.println(ret.OpenState);
        ret.CloseState = (_data >> 8) & B00000001;

        //Serial.print("CloseState "); Serial.println(ret.CloseState);
        ret.Connected = (_data >> 10) & B00000001;
        return ret;
    }

    RespondBitData DecodeBitRespond(unsigned int _data)
    {
        RespondBitData ret;

        ret.Bit0 = _data & B00000001;
        ret.Bit1 = _data & B00000010;
        ret.Bit2 = _data & B00000100;
        ret.Bit3 = _data & B00001000;
        ret.Bit4 = _data & B00010000;
        ret.Bit5 = _data & B00100000;
        ret.Bit6 = _data & B01000000;
        ret.Bit7 = _data & B10000000;

        return ret;
    }


    uint16_t EncodeWord(byte byte1, byte byte2)
    {
        int ret = 0;
        ret = byte1;
        ret = ret << 8;
        ret = ret | byte2;
        return ret;
    }

    RespondChar DecodeWord(uint16_t  _data)
    {
        RespondChar ret;
        ret.char1 = _data & B11111111;
        ret.char2 = (_data >> 8) & B11111111;
        return ret;
    }
    RespondByte DecodeInt(unsigned int _data)
    {
        RespondByte ret;
        ret.Byte2 = _data & B11111111;
        ret.Byte1 = (_data >> 8) & B11111111;

        return ret;
    }

    uint32_t Encodeuint32(uint16_t byte1, uint16_t byte2)
    {
        uint32_t ret = 0;
        ret = byte1;
        ret = ret << 16;
        ret = ret | byte2;
        return ret;
    }

    unsigned int EncodeInt(byte byte1, byte byte2)
    {
        int ret = 0;
        ret = byte1;
        ret = ret << 8;
        ret = ret | byte2;
        return ret;
    }

    unsigned int EncodeRespond(boolean connect, boolean _openState, boolean _closeState, byte _powerCap)
    {
        int ret = 0;
        byte byte_one = 0;
        byte byte_two = 0;
        byte_two = _powerCap;
        byte_one = _openState;
        byte_one = byte_one << 1;
        byte_one = byte_one | _closeState;
        byte_one = byte_one << 2;
        byte_one = byte_one | connect;
        ret = byte_one;
        ret = ret << 8;
        ret = ret | byte_two;
        return ret;
    }

    unsigned int EncodeBitRespond(boolean bit0, boolean bit1, boolean bit2, boolean bit3, boolean bit4, boolean bit5, boolean bit6, boolean bit7)
    {
        int ret = 0;
        byte byte_one = 0;
        if (bit0)byte_one |= 1 << 0;
        if (bit1)byte_one |= 1 << 1;
        if (bit2)byte_one |= 1 << 2;
        if (bit3)byte_one |= 1 << 3;
        if (bit4)byte_one |= 1 << 4;
        if (bit5)byte_one |= 1 << 5;
        if (bit6)byte_one |= 1 << 6;
        if (bit7)byte_one |= 1 << 7;
        ret = byte_one;
        return ret;
    }

    unsigned int EncodeRespond(RespondValveData _data)
    {
        int ret = 0;
        byte byte_one = 0;
        byte byte_two = 0;
        byte_two = _data.PowerCap; // _powerCap;

        byte_one = _data.Connected; //_Connected;
        byte_one = byte_one << 1;
        byte_one |= _data.OpenState; //_openState;
        byte_one = byte_one << 1;
        byte_one |= _data.CloseState; //_closeState;

        //tong hop
        ret = byte_one;
        ret = ret << 8;
        ret = ret | byte_two;
        return ret;
    }

    RequestValveData DecodeRequest(unsigned int _data)
    {
        RequestValveData ret;
        ret.register_data = _data;
        //ret.SetValve = _data & B00000001;
        return ret;
    }

    unsigned int EncodeRequest(boolean _cmd)
    {
        unsigned int ret = B00000000 | _cmd;
        return ret;
    }

    unsigned int EncodeRequest(RequestValveData _cmd)
    {
        unsigned int ret = 0;
        //them bit dieu khien open_cmd
        ret |= _cmd.open_cmd;
        //them bit dieu khien close_cmd
        ret = ret << 1;
        ret |= _cmd.close_cmd;
        //them thoi gian ngu
        ret = ret << 4;
        ret |= _cmd.sleep_time;
        return ret;
    }

} DataTransfer;

/*Cấu trúc lệnh đơn giản để ép kiểu dữ liệu để rán vào các bit xác định trong 16 bit của word
cách dùng:
DataValveMonitor dataValveMonitor;

//chuyển đõi giá trị từ các
dataValveMonitor.power_cap;
dataValveMonitor.close_state;
dataValveMonitor.open_state;
dataValveMonitor.connected;
//=>
var data = dataValveMonitor.register_data
//có thể chuyển ngược lại
*/
typedef struct
{
    union
    {
        struct
        {
            unsigned char power_cap : 8;
            bool close_state : 1;
            bool open_state : 1;
            bool connected : 1;
            unsigned char _reserved : 5;
        };
        uint16_t register_data;
    };
} DataValveMonitor;

typedef struct
{
    union
    {
        struct
        {
            unsigned char sleep_time : 4;
            bool open_cmd : 1;
            bool close_cmd : 1;
            unsigned int _reserved : 10;
        };
        uint16_t register_data;
    };
} DataValveControl;

void monitorDataControlValve(uint16_t id, uint16_t data);
void monitorDataStateValve(uint16_t id, uint16_t data);

#endif