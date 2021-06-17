#ifndef GLOBAL_H
#define GLOBAL_H

//#define Lora_Debug
//#define Modbus_Debug

#include "Arduino.h"
#include <WiFi.h>

typedef struct
{
    uint16_t year;   
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t millisecs;
}Rtc_struct;

#define PinLed 2

#endif