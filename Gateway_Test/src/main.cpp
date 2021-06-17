//-----------------------------------------------
//  Libraries
//-----------------------------------------------
#include "Global.h"

#include "Task_ModBus.h"
#include "Task_LoRa.h"
#include "Task_Rtc.h"
#include "Task_Serial.h"

uint8_t Register[64];
uint8_t DevSamp[8];

char* ssid     = "";
char* password = "Diacsa4723128";
WiFiServer server(80);
IPAddress ip(192, 168, 10, 10);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);
Rtc_struct Rtc;


//-----------------------------------------------
//  Main Program
//-----------------------------------------------
void setup() 
{
  Serial.begin(115200);
  delay(100);
  pinMode(PinLed,OUTPUT);
  digitalWrite(PinLed,LOW);

  xTaskCreate(ModTcp_Task,"Task_ModbusTcp",20480,NULL,2,NULL);
  xTaskCreate(LoRa_Task,"Task_LoRaReception",10240,NULL,2,NULL);
  xTaskCreate(Rtc_Task,"Task_Rtc",4096,NULL,2,NULL);
  xTaskCreate(Serial_Task,"Task_Serial",4096,NULL,2,NULL);

}

void loop() {}

//------------------------------------------------------------------------
// Modbus TCP Task
//------------------------------------------------------------------------
void ModTcp_Task(void *pvParameters) 
{
    uint8_t cad[128];
    uint8_t b,start,idx;
    WiFiClient client;
    unsigned long tini;
    long t;

    start = idx = 0;
    ModTcp_WiFiConn(&server,ssid,password,&ip,&gateway,&subnet);
    
    while(1)
    {
        vTaskDelay(0);
        client=server.available();
        if(!client) continue;
        Serial.println("Client connected");

        while(client.connected())
        {                     
            vTaskDelay(0);
            if(client.available()==0) continue;
            b = client.read();   
            #ifdef Modbus_Debug 
                Serial.print(b,HEX);
                Serial.print(" ");
            #endif

            if(start==0)
            {   tini = millis();
                start=1; 
            }
            
            t = millis()-tini;
            if((t<0)||(t>5000))
            {
                idx=0;
                start=0;
            }

            if( (b==0xff) && (idx==0) ) continue;
            cad[idx++] = b;
            
            if(idx<12) continue;
            idx = 0;
                
            if(ModTcp_RxTx(&client,cad,Register)==0)          
                Serial.println("Request: Incorrect format");
            else
                Serial.println("Request: Responded");
        }
        Serial.println("Client disconnected");
    }
}




//------------------------------------------------------------------------
// Lora task
//------------------------------------------------------------------------
void LoRa_Task(void *pvParameters)
{      
    int packetLen;
    uint8_t data[256],i,d;
    
    DevSamp[1] = DevSamp[3] = 0xff;
    for(uint8_t d=0;d<64;d++)   Register[d] = 0xff;

    LoRa_Init();        
    LoRa.receive();
    
    while(1)
    {
        delay(10);

        Lora_CheckTxConfig(DevSamp);

        packetLen = LoRa.parsePacket();
        if(packetLen==0) continue;
        if(packetLen!=10){
          Serial.println("LoRa: Packet length error");
          while(LoRa.available())                        
              d = LoRa.read();          
          continue;
        }

        #ifdef Lora_Debug   
        Serial.print("LoRa Debug: ");
        #endif

        digitalWrite(PinLed,HIGH);
        while(LoRa.available()){
            data[i]= LoRa.read();
            #ifdef Lora_Debug
                Serial.print(data[i],HEX);
                Serial.print(" ");
            #endif
            i++;
        }
        i=0;
        #ifdef Lora_Debug   
        Serial.println("");
        #endif

        uint8_t node;
        if(Lora_ParseNodePacket(data,Register,&Rtc,&node)==true)
        {
            Serial.print("Node-");
            Serial.print(node);   
            Serial.print(" ");
            Serial.print(Register[node*16]);Serial.print(":");Serial.print(Register[node*16+1]);Serial.print(":");Serial.print(Register[node*16+2]);Serial.print(" ");
            Serial.print(Register[node*16+3]);Serial.print("/");Serial.print(Register[node*16+4]);Serial.print("/");Serial.print(Register[node*16+5]);
            Serial.print(" - ");
            for(uint8_t j=0;j<10;j++)
            {
                Serial.print(Register[16*node+2*j+6],HEX); Serial.print(Register[16*node+2*j+7],HEX);
                Serial.print(" ");
            }
            Serial.println("");            
        }
        
        digitalWrite(PinLed,LOW);
        
    }
}

void  Rtc_Task(void *pvParameters)
{
    Rtc.day     = 3;
    Rtc.month   = 5;
    Rtc.year    = 21;
    Rtc.hour    = 1;
    Rtc.minute  = 20;
    Rtc.second  = 03;

    while(1)
    {
        vTaskDelay(105);
        Rtc_UpdateRtc(&Rtc);
    }

    
}

void Serial_Task(void *pvParameters)
{

    uint8_t cadRx[50],i;

    while(1)
    {
        vTaskDelay(100);
        if(Serial.available()==0) continue;
        
        while(Serial.available()!=0)
        {
            if(i>=50) i=0;
            cadRx[i++] = Serial.read();
            if( (cadRx[i-1]==10) ) break;
        }        
        i=0;

        if(Serial_UpdateTime(cadRx, &Rtc)==true) continue;
        if(Serial_UpdateDeviceSampling(cadRx,DevSamp)==true) continue;
        Serial.println("Unknow command");

    }

}