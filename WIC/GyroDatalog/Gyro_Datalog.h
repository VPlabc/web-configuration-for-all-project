#ifndef GyroDatalog_
#define GyroDatalog_

#define FC_Gyro

#include "config.h"
#include <MPU9250.h>
#include <quaternionFilters.h>
#include <MPU9250.h>
#include <IMUResult.h>
#include <IMUWriter.h>
#ifdef Gyro_UI
class GyroDatalog
{
public:

    void Setup();
    void Loop();
    void Read_Sensor();
    void SaveData();
    void readResult(IMUResult result, String &payload);
    void Start();
    void Start_PPM();
};
#endif//Gyro_UI
#endif//GyroDatalog_