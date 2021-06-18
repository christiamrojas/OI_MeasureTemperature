#ifndef GLOBAL_H
#define GLOBAL_H

#include "Arduino.h"
#include <WiFi.h>

//#define LoRa_Debug
//#define LoRa_Debug_Memory

//#define Modbus_Debug
//#define Modbus_Debug_Memory

//#define Bt_Debug
//#define Bt_Debug_Memory
//#define Bt_Review_Devices

#define EE_Size             140
#define EE_Addr_Ssid        0
#define EE_Addr_Password    64
#define EE_Addr_Ip          128
#define EE_Addr_Gateway     132
#define EE_Addr_Subnet      136


#define PIN_GW_RST          17    
#define PIN_LED_PWR         32
#define PIN_LED_MSG         25

typedef struct
{    
    uint8_t NodeAddr;
    uint8_t FrequencyId;
    uint8_t SpreadFactor;    
    uint16_t SampleTime;
    bool Update;     
}Node_TxConfig_Struct;

#endif