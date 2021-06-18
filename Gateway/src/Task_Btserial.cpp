#include "Task_Btserial.h"

void Btserial_CheckCommand(BluetoothSerial *Btserial, RTC *rtc, Node_TxConfig_Struct *Node_TxConfig)
{
    static char cad[128];
    static uint8_t idx=0;

    do{
        if(Btserial->available()==0) return;
        if(idx==127) idx=0;
        cad[idx++] = Btserial->read();    
    }while( (cad[idx-1]!=13) && (cad[idx-1]!=10) );

    idx = 0;
    
    if(Btserial_CheckCommand_SetDateTime(cad,Btserial,rtc)) return;
    if(Btserial_CheckCommand_SetWiFi(cad,Btserial))   return; 
    if(Btserial_CheckCommand_SetTxPeriod(cad,Btserial,Node_TxConfig)) return; 
    
}
// Channel = 916800000,917000000,917200000,917400000,917600000,917800000,918000000,918200000
//Example: Node=01 6 12 0100\n (Node Channel SpreadF Sampling)
bool Btserial_CheckCommand_SetTxPeriod(char *cad, BluetoothSerial *Btserial, Node_TxConfig_Struct *Node_TxConfig)
{
    if(strncmp(cad,"Node=",5)!=0)   return false;
    if( (cad[7]!=' ')  || (cad[9]!=' ') || (cad[12]!=' ') )   return false;
    
    uint16_t d; 
    d = (cad[5]-'0')*10 + cad[6]-'0';
    if( (d>32) || (d==0) )    return false; 
    Node_TxConfig->NodeAddr = (uint8_t)(d)-1; 

    d = cad[8]-'0';
    if(d>8)    return false; 
    Node_TxConfig->FrequencyId = (uint8_t)(d); 

    d = (cad[10]-'0')*10 + cad[11]-'0';
    if( (d>13) || (d<7) ) return false; 
    Node_TxConfig->SpreadFactor = (uint8_t)(d); 

    d = (cad[13]-'0')*1000 + (cad[14]-'0')*100 + (cad[15]-'0')*10 + (cad[16]-'0');
    if(d>3600) return false; 
    Node_TxConfig->SampleTime = d; 

    Node_TxConfig->Update = true; 

    Btserial->println("Node: Config send it");
    return true;
}
//Example: WiFi=CRISTIAN 09876543 192.168.1.250 192.168.1.1 255.255.255.0\n
bool Btserial_CheckCommand_SetWiFi(char *cad, BluetoothSerial *Btserial)
{     
    char *p;
    int n;

    char ssid[64];   
    char password[64];
    char ip[32];
    char gateway[32];
    char subnet[32];

    uint8_t octect[4];
    
    if(strncmp(cad,"WiFi=",5)!=0)           return false;             
    p = &cad[5];
    if(!str_getstring(ssid,p,&n,' '))       return false;

    p = &p[n];
    if(!str_getstring(password,p,&n,' '))   return false;

    p = &p[n];
    if(!str_getstring(ip,p,&n,' '))         return false;

    p = &p[n];
    if(!str_getstring(gateway,p,&n,' '))    return false;
    
    p = &p[n];
    if(!str_getstring(subnet,p,&n,13))      return false;
    
    if(str_getip(ip,octect)==false)         return false; 
    Eeprom_Write(EE_Addr_Ip,4,octect);

    if(str_getip(gateway,octect)==false)    return false; 
    Eeprom_Write(EE_Addr_Gateway,4,octect);

    if(str_getip(subnet,octect)==false)     return false; 
    Eeprom_Write(EE_Addr_Subnet,4,octect);

    Eeprom_Write(EE_Addr_Ssid,0,(uint8_t*)ssid);
    Eeprom_Write(EE_Addr_Password,0,(uint8_t*)password);

    Btserial->println("WiFi: Updated");

    vTaskDelay(2000);

    ESP.restart();

    return true;
}
//Example: RTC=13:00:10 16/06/21\n
bool Btserial_CheckCommand_SetDateTime(char *p, BluetoothSerial *Btserial, RTC *rtc)
{
    static uint8_t year, month, day, hour, minute, second;

    if(strncmp(p,"RTC=",4)!=0)   return false; 

    if( (p[6]!=':')  || (p[9]!=':') || (p[12]!=' ') ||
        (p[15]!='/') || (p[18]!='/') || (p[21]!=13) )
    return false; 

    hour    = ((p[4]  & 0x0f)<<4) | (p[5]  & 0x0f);
    minute  = ((p[7]  & 0x0f)<<4) | (p[8]  & 0x0f);
    second  = ((p[10] & 0x0f)<<4) | (p[11] & 0x0f);
    day     = ((p[13] & 0x0f)<<4) | (p[14] & 0x0f);
    month   = ((p[16] & 0x0f)<<4) | (p[17] & 0x0f);
    year    = ((p[19] & 0x0f)<<4) | (p[20] & 0x0f);

    if( (hour>0x23) | (minute>0x59) | (second>0x59) )   return false;
    if( (day>0x31) | (month>0x12) | (year>0x99) )       return false;
    
    if(rtc->Write(hour,minute,second, day,month,year)==true) 
    {        
        Btserial->println("RTC: Updated");
        return true;
    }

    Btserial->println("RTC: Error");
    return false;    
}
