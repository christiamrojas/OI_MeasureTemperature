#include "Eeprom_esp.h"

void Eeprom_Write(uint16_t addr,uint16_t n, uint8_t *pin)
{
    if(n==0)
    {
        uint8_t d; 

        while(true)
        {
            d = pin[n++];
            EEPROM.write(addr++,d);
            if(d==0)
            {
                EEPROM.commit();
                return; 
            }             
        }
    }
    else
    {
        for(uint16_t i=0;i<n;i++)
            EEPROM.write(addr+i,pin[i]);
        EEPROM.commit();
    }
}

void Eeprom_Read(uint16_t addr,uint16_t n,uint8_t *pout)
{
    if(n==0)
    {
        uint8_t d;
        
        while(true)
        {
            d = EEPROM.read(addr++);            
            pout[n++] = d;
            if(d==0) return; 
        }
    }
    else
    {
        for(uint16_t i=0;i<n;i++)
            pout[i] = EEPROM.read(addr+i);
    }
}


