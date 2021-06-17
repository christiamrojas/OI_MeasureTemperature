
# include "Rtc.h"

bool RTC::Init(void)
{
    Wire.begin(Pin_Sda,Pin_Scl,200000);
    delay(100);
    strcpy(txt_datetime,"00:00:00 00/00/00\0");
    #ifdef Rtc_write_datetime
    if(!Write(default_hour,default_minute,default_second,
                 default_day, default_month, default_year)) return false;
    #endif
    for (uint8_t i=0;i<3;i++)
        if (Read()) return true;    
    return false;
}

bool RTC::Write(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, uint8_t year)
{ 
    byte dat_rtc[6],d;
    bool ok;

    dat_rtc[0]  = second;
    dat_rtc[1]  = minute;
    dat_rtc[2]  = hour | 0x80;
    dat_rtc[3]  = day;
    dat_rtc[4]  = month;
    dat_rtc[5]  = year;
    d           = 0x90;
  
    ok  = I2C_write(0x6F,7,&d,1);         // Set Status
    ok &= I2C_write(0x6F,0,dat_rtc,6);    // Set time-date
    ok &= I2C_write(0x6F,7,&d,1);         // Read Status
    ok &= (d==0x90);

    return ok;
}

bool RTC::Read(void)
{
    uint8_t d,temp[6];
    bool ok;

    ok = I2C_read(0x6F,7,&d,1);         // Read Status
    ok &= (d==0x90);

    ok &= I2C_read(0x6F,0,temp,6);      // Read time-date
    ok &= ((temp[0] & 0x80)==0);
    ok &= ((temp[1] & 0x80)==0);
    ok &= ((temp[2] & 0xc0)==0x80);
    ok &= ((temp[3] & 0xc0)==0);
    ok &= ((temp[4] & 0xe0)==0);
    if (!ok) return false; 

    hour   = temp[2] & 0x3f;
    minute = temp[1];
    second = temp[0];
    day    = temp[3];
    month  = temp[4];
    year   = temp[5];
    /*
    Serial.print(hour,HEX); Serial.print(" ");
    Serial.print(minute,HEX); Serial.print(" ");
    Serial.print(second,HEX); Serial.print(" ");

    Serial.print(day,HEX); Serial.print(" ");
    Serial.print(month,HEX); Serial.print(" ");
    Serial.print(year,HEX); Serial.println("");*/

    return true;
}


bool RTC::Bcd2Txt(void)
{ 
    char Yh,Yl,Mh,Ml,Dh,Dl,Hh,Hl,MIh,MIl,Sh,Sl;
    /*
    Serial.print(hour,HEX); Serial.print(" ");
    Serial.print(minute,HEX); Serial.print(" ");
    Serial.print(second,HEX); Serial.print(" ");

    Serial.print(day,HEX); Serial.print(" ");
    Serial.print(month,HEX); Serial.print(" ");
    Serial.print(year,HEX); Serial.println("");*/

    if (!ByteBcd2Txt(year,&Yh,&Yl)) return false;
    if (!ByteBcd2Txt(month,&Mh,&Ml)) return false;    
    if (!ByteBcd2Txt(day,&Dh,&Dl)) return false;        
    if (!ByteBcd2Txt(hour,&Hh,&Hl)) return false;
    if (!ByteBcd2Txt(minute,&MIh,&MIl)) return false;    
    if (!ByteBcd2Txt(second,&Sh,&Sl)) return false;

  

    txt_datetime[0]=Hh;   txt_datetime[1]=Hl;   
    txt_datetime[3]=MIh;  txt_datetime[4]=MIl;  
    txt_datetime[6]=Sh;   txt_datetime[7]=Sl;   
    txt_datetime[9]=Dh;   txt_datetime[10]=Dl;  
    txt_datetime[12]=Mh;  txt_datetime[13]=Ml;
    txt_datetime[15]=Yh;  txt_datetime[16]=Yl;  
    
    return true;
}

bool RTC::ByteBcd2Txt(byte bcd,char *H,char *L)
{
  *H=((bcd>>4)&0xf)|0x30;
  if (*H>'9') return false;
  *L=(bcd & 0xf)|0x30;
  if (*L>'9') return false;
  return true;
}

bool RTC::I2C_read(uint8_t device, uint8_t dir, uint8_t *data, uint8_t n)
{
    bool ok; 
    byte i; 

    Wire.beginTransmission(device);
    ok = (Wire.write(dir)==1);
    ok &= (Wire.endTransmission()==0);
    ok &= (Wire.requestFrom(device,n)==n);
    for (i=0;i<n;i++)    
        data[i]=Wire.read();        
    /*
    for (i=0;i<n;i++){
        Serial.print(data[i],HEX); Serial.print(" ");
    }
    Serial.println("");*/
    return ok;
}

bool RTC::I2C_write(byte device,byte dir, byte *data,byte n)
{ 
    bool ok;
  
    Wire.beginTransmission(device); 
    ok = (Wire.write(dir)==1);
    ok &= (Wire.write(data,n)==n) ;
    ok &= (Wire.endTransmission()==0);
    return ok;
}