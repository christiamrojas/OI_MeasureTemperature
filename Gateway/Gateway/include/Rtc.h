#ifndef RTC_H
#define RTC_H

//#define Rtc_write_datetime
#define default_hour        0x02
#define default_minute      0x26
#define default_second      0x00
#define default_day         0x15
#define default_month       0x06
#define default_year        0x21

#define Pin_Sda             21        
#define Pin_Scl             22

#include <Wire.h>
#include "Global.h"

class RTC
{
    public:
        char txt_datetime[19];
        uint8_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;

        bool Init(void);
        bool Read(void);
        bool Write(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, uint8_t year);
        
        bool Bcd2Txt(void);
        bool ByteBcd2Txt(byte bcd,char *H,char *L);
    
    private:
        bool I2C_read(uint8_t device, uint8_t dir, uint8_t *data, uint8_t n);
        bool I2C_write(byte device,byte dir, byte *data,byte n);
};

#endif