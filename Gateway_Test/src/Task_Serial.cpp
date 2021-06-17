#include "Task_Serial.h"

//  Format
//  DevSampling=02-10 (Device-Time)
bool Serial_UpdateDeviceSampling(uint8_t *p, uint8_t *devSamp)
{
    if(strncmp((char*)p,"DevSampling=",12)!=0){        
        return false;
    }
      
    if( (p[14]!='-') || (p[17]!=13) ){
        Serial.println("DevSampling: Incorrect format");
        return true;
    }    

    if( (p[12]!='0') || ((p[13]!='0') && (p[13]!='1')) ||      
        (p[15]<'0') || (p[15]>'9') || (p[16]<'0') || (p[16]>'9') )
    {
        Serial.println("DevSampling: Incorrect format");
        return true;
    }    

    uint8_t t,s;
    t = p[13] & 0x0f;
    s = (p[15] & 0x0f)*10 + (p[16]&0x0f);
    
    if(devSamp[(t<<1)+1]>60)
    {
        Serial.println("DevSampling: Time beyond limits");
        return true;
    }

    devSamp[t<<1] = t;
    devSamp[(t<<1)+1] = s;
    Serial.println("DevSampling: Updated");

    return true;
}

//  Format
//  RTC=15:05:10 04/05/21
bool Serial_UpdateTime(uint8_t *p, Rtc_struct *rtc)
{
    uint8_t year, month, day, hour, minute, second;

    if(strncmp((char*)p,"RTC=",4)!=0){        
        return false; 
    }

    if( (p[6]!=':')  || (p[9]!=':') || (p[12]!=' ') ||
        (p[15]!='/') || (p[18]!='/') || (p[21]!=13) )
    {
        Serial.println("RTC: Incorrect format");
        return true; 
    }
    /*
    hour    = ((p[4]<<4)  & 0xf0) | (p[5]  & 0x0f);
    minute  = ((p[7]<<4)  & 0xf0) | (p[8]  & 0x0f);
    second  = ((p[10]<<4) & 0xf0) | (p[11] & 0x0f);
    day     = ((p[13]<<4) & 0xf0) | (p[14] & 0x0f);
    month   = ((p[16]<<4) & 0xf0) | (p[17] & 0x0f);
    year    = ((p[19]<<4) & 0xf0) | (p[20] & 0x0f);*/

    hour    = (p[4]&0x0f)*10  + (p[5]  & 0x0f);
    minute  = (p[7]&0x0f)*10  + (p[8]  & 0x0f);
    second  = (p[10]&0x0f)*10 + (p[11]  & 0x0f);
    day     = (p[13]&0x0f)*10 + (p[14]  & 0x0f);
    month   = (p[16]&0x0f)*10 + (p[17]  & 0x0f);
    year    = (p[19]&0x0f)*10 + (p[20]  & 0x0f);


    Serial.println(hour);
    Serial.println(minute);
    Serial.println(second);
    Serial.println(day);
    Serial.println(month);
    Serial.println(year);
    

    rtc->year   = year;
    rtc->month  = month;
    rtc->day    = day;
    rtc->hour   = hour;     
    rtc->minute = minute;
    rtc->second = second;

    Serial.println("RTC: Updated");
    return true;
}