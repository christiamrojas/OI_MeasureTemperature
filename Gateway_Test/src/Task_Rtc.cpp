#include "Task_Rtc.h"

void  Rtc_UpdateRtc(Rtc_struct *rtc)
{
    static unsigned long LastMillis=0;
    static unsigned long CurrentMillis=0;

    CurrentMillis = millis();
    long millisDiff = CurrentMillis - LastMillis;
    
    if(millisDiff > 0)
    {
        rtc->millisecs += millisDiff;
        LastMillis = CurrentMillis;
        if(rtc->millisecs >= 1000)
        {
            rtc->second++;
            rtc->millisecs = 0;
        }
    }
    if(rtc->second == 60)
    {
        rtc->minute++;
        rtc->second = 0;
    }
    if(rtc->minute == 60)
    {
        rtc->hour++;
        rtc->minute = 0;
    }
    if(rtc->hour == 24)
    {
        rtc->day++;
        rtc->hour = 0;
    }
    
    if((rtc->month == 1) || (rtc->month == 3) || (rtc->month == 5) || (rtc->month == 7) || (rtc->month == 8) || (rtc->month == 10) || (rtc->month == 12))
    {
        if(rtc->day == 32)
        {
            rtc->month++;
            rtc->day = 1;
        }
    }
    else if((rtc->month == 2))
    {
        if(rtc->day == 29)
        {
            rtc->month++;
            rtc->day = 1;
        }
    }
    else
    {
        if(rtc->day == 31)
        {
            rtc->month++;
            rtc->day = 1;
        }
    }
    if(rtc->month == 13)
    {
        rtc->year++;
        rtc->month = 1;
    }



}
