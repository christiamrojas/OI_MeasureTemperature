//------------------------------------------------------------------------
// Libraries
//------------------------------------------------------------------------

#include "Task_LoRa.h"

//------------------------------------------------------------------------
// LoRa definitions
//------------------------------------------------------------------------
#define LORA_BW               125E3         //Bandwith
#define LORA_SP               12            //Spreading Factor
#define LORA_CHANNEL          915E6         //Canal
#define LORA_SYNCWORD         0x12          
#define LORA_CR               5             //Coding rate (4/x)
#define LORA_PL               8             //Preamble length (x+4)
#define LORA_ADDRESS          4
#define LORA_SEND_TO_ADDRESS  2

//------------------------------------------------------------------------
bool Lora_ParseNodePacket(uint8_t *data, uint8_t *Reg, Rtc_struct *rtc, uint8_t *node)
{      
    if( (data[0]!=0) && (data[0]!=1))   return false;
    
    uint8_t dev = data[0];    
    uint8_t t;
    uint16_t temp;    
    //Serial.print("Device: "); Serial.println(dev);

    *node = dev;
    Reg[16*dev]   = rtc->hour;
    Reg[16*dev+1] = rtc->minute;
    Reg[16*dev+2] = rtc->second;
    Reg[16*dev+3] = rtc->day;
    Reg[16*dev+4] = rtc->month;
    Reg[16*dev+5] = rtc->year;

    uint8_t *Register = &Reg[dev*16+6];

    temp = (data[1]<<8) + data[2];
    if((temp & 0x02)==1) Reg[28] &= ~(1<<dev);
    if((temp & 0x01)==1) temp = 0xffff;
    else  temp>>=4;        
    Register[dev*10]   = (temp>>8) & 0xff;
    Register[dev*10+1] = temp & 0xff;
    #ifdef Lora_Debug
      Serial.print(temp);
      Serial.print(" ");
    #endif

    temp = (data[3]<<8) + data[4];
    if((temp & 0x02)==1) Reg[28] &= ~(1<<dev);
    if(temp & 0x01) temp = 0xffff;
    else  temp>>=4;        
    Register[dev*10+2] = (temp>>8) & 0xff;
    Register[dev*10+3] = temp & 0xff;
    #ifdef Lora_Debug
      Serial.print(temp);
      Serial.print(" ");
    #endif

    temp = (data[5]<<8) + data[6];
    if((temp & 0x02)==1) Reg[28] &= ~(1<<dev);
    if(temp & 0x01) temp = 0xffff;
    else  temp>>=4;        
    Register[dev*10+4] = (temp>>8) & 0xff;
    Register[dev*10+5] = temp & 0xff;
    #ifdef Lora_Debug
      Serial.print(temp);
      Serial.print(" ");
    #endif
            
    temp = (data[7]<<8) + data[8];
    if((temp & 0x02)==1) Reg[28] &= ~(1<<dev);
    if(temp & 0x01) temp = 0xffff;
    else  temp>>=4;        
    Register[dev*10+6] = (temp>>8) & 0xff;
    Register[dev*10+7] = temp & 0xff; 
    #ifdef Lora_Debug
      Serial.print(temp);
      Serial.print(" ");
    #endif

    int n;      
    Register[dev*10+8] = 0;
    if(dev==0)  // Between [0xd7:0xbb]
    {
      n = ((data[9]-0xbb)*100/0x1c);            
      if(n>100) n=100;
      else if(n<0) n=0;
      t = (uint8_t)(n);
    }     
    else        // Between [0xdc:0xc1]
    {  
      n = ((data[9]-0xc1)*100/0x1b);            
      if(n>100) n=100;
      else if(n<0) n=0;
      t = (uint8_t)(n);      
    }
    Register[dev*10+9] = t;
    #ifdef Lora_Debug
      Serial.print(t);
      Serial.print(" ");
      Serial.println("");
    #endif

    return true; 
}

//------------------------------------------------------------------------
bool Lora_CheckTxConfig(uint8_t *dev)
{
    if( (dev[1]!=0xff))
    {
        uint16_t t;            
        LoRa.beginPacket();
        LoRa.write(dev[0]);
        t = dev[1]*60;
        LoRa.write(t&0xff);
        LoRa.write((t>>8)&0xff);
        LoRa.endPacket();
        dev[1] = 0xff;
        LoRa.receive();
        return true;
    }

    if( (dev[3]!=0xff))
    {
        uint16_t t;            
        LoRa.beginPacket();
        LoRa.write(dev[2]);
        t = dev[3]*60;
        LoRa.write(t&0xff);
        LoRa.write((t>>8)&0xff);
        LoRa.endPacket();
        dev[3] = 0xff;
        LoRa.receive();
        return true;
    }

    return false;
}

//------------------------------------------------------------------------
void  LoRa_Init(void)
{
    pinMode(LED,OUTPUT);
    pinMode(DIO0_1, INPUT);
    pinMode(DIO1_1, INPUT);
    pinMode(DIO2_1, INPUT);
    pinMode(RST1, OUTPUT);
    pinMode(SS1, OUTPUT);
    digitalWrite(SS1,HIGH);

    LoRa.setPins(SCK, MISO, MOSI, SS1, RST1, DIO0_1);
    while(!LoRa.begin(LORA_CHANNEL)){
      Serial.print(".");
      delay(500);
    }
    Serial.println("");
        
    LoRa.setSyncWord(LORA_SYNCWORD);              //Seteamos la dirección de sincronización
    LoRa.setSpreadingFactor(LORA_SP);             //Seteamos el Spreading Factor (SP)
    LoRa.setSignalBandwidth(LORA_BW);             //Seteamos El ancho de banda
    LoRa.setCodingRate4(LORA_CR);                 //Seteamos el Coding rate (4/(x-4))
    LoRa.setPreambleLength(LORA_PL);              //Seteamos la longitud del preambulo (x+4)
    LoRa.enableCrc();          
    Serial.println("Module configured finished");
    Serial.println();
}