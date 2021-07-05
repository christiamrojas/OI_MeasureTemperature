//------------------------------------------------------------------------
// Libraries
//------------------------------------------------------------------------
#include "Task_ModBus.h"

void ModTcp_ClientConnected(WiFiClient *client,uint8_t *pReg)
{    
    static uint8_t timeout,i,cad[128];
    uint8_t d;

    timeout = i = 0;

    while(client->connected())
    {
      vTaskDelay(100/portTICK_PERIOD_MS);
      if(client->available()==0) continue;      
      #ifdef Modbus_Debug_Memory                   
        Serial.print("Stack memory remain(Words): ");Serial.println(uxTaskGetStackHighWaterMark(NULL));
        Serial.print("Heap memory total  (Bytes): ");Serial.println(xPortGetFreeHeapSize());
        #endif
      do{
        while(client->available()!=0){
          d = client->read();
          #ifdef Modbus_Debug 
          Serial.print(d,HEX);
          Serial.print(" ");
          #endif
          cad[i++] = d; 
        }          
        vTaskDelay(50/portTICK_PERIOD_MS);        
        if(i>=12) break; 
        if((++timeout)>(3000/50)) break; 
      }while(true);
      
      if( (i!=12) || (timeout>3000/50)) {
        timeout = i = 0;
        continue;
      }
      timeout = i = 0;
                                  
      if(ModTcp_RxTx(client,cad,pReg)==0)          
        Serial.println("Modbus: Incorrect format");
      else
        Serial.println("Modbus: Responded");
    }

    Serial.println("Modbus: Client disconnected");

}

uint8_t ModTcp_RxTx(WiFiClient *client, uint8_t *pIn, uint8_t *pReg)
{
    uint8_t device_id;
    
    if ((pIn[4]!=0)||(pIn[5]!=6))             return 0;     // Length = 6
    device_id = pIn[6];                                       
    if ( (device_id>32) || (device_id==0) )   return 0;     // Id [0:31]         
    if (pIn[7]!=3)          return 0;                       // Function Holding Register
    if ((pIn[8]!=0x0f)||(pIn[9]!=0xa0))       return 0;     // Address 4000
    if ((pIn[10]!=0)||(pIn[11]!=8))           return 0;     // 8 words
            
    pIn[5]= 16+3;                                           // Length
    pIn[8]= 16;                                             // Byte count
    for (uint8_t i=0;i<16;i++)                              // Data
      pIn[9+i]=pReg[16*device_id+i];
    
    pReg[16*device_id] = 0xff;                              // Data register readed
    client->write(pIn,25);

    #ifdef Modbus_Debug
    Serial.println("");
    for(uint8_t i=0;i<25;i++)
    {
      Serial.print(pIn[i],HEX);
      Serial.print(" ");
    }
    Serial.println("");
    #endif

    return 1;
}    

uint8_t ModTcp_WiFiInit(WiFiServer *s)
{
    uint8_t octet[4];
    char WiFi_Ssid[64],WiFi_Password[64];
    uint16_t timeout_s;
    
    Eeprom_Read(EE_Addr_Ip,4,octet);
    IPAddress ip(octet[0],octet[1],octet[2],octet[3]);
    
    #ifdef WiFi_Config_Debug
    Serial.print(octet[0]); Serial.print(" ");
    Serial.print(octet[1]); Serial.print(" ");
    Serial.print(octet[2]); Serial.print(" ");
    Serial.print(octet[3]); Serial.println("");
    #endif
    
    Eeprom_Read(EE_Addr_Gateway,4,octet);    
    IPAddress gateway(octet[0],octet[1],octet[2],octet[3]);
    #ifdef WiFi_Config_Debug
    Serial.print(octet[0]); Serial.print(" ");
    Serial.print(octet[1]); Serial.print(" ");
    Serial.print(octet[2]); Serial.print(" ");
    Serial.print(octet[3]); Serial.println("");
    #endif
    
    Eeprom_Read(EE_Addr_Subnet,4,octet);
    IPAddress subnet(octet[0],octet[1],octet[2],octet[3]);
    #ifdef WiFi_Config_Debug
    Serial.print(octet[0]); Serial.print(" ");
    Serial.print(octet[1]); Serial.print(" ");
    Serial.print(octet[2]); Serial.print(" ");
    Serial.print(octet[3]); Serial.println("");
    #endif

    Eeprom_Read(EE_Addr_Ssid,0,(uint8_t*)WiFi_Ssid);
    #ifdef WiFi_Config_Debug
    Serial.println(WiFi_Ssid);
    #endif

    Eeprom_Read(EE_Addr_Password,0,(uint8_t*)WiFi_Password);
    #ifdef WiFi_Config_Debug
    Serial.println(WiFi_Password);
    #endif
  
    Serial.print("WiFi: ");
    if(WiFi.config(ip,gateway,subnet))
      Serial.println("Static IP OK");
    else
      Serial.println("Static IP failed");

    timeout_s = 30;
    WiFi.begin(WiFi_Ssid,WiFi_Password);
    
    while( WiFi.status()!=WL_CONNECTED){
      vTaskDelay(500/portTICK_PERIOD_MS);
      if((--timeout_s)==0) return 3;
      Serial.println("");
    }
    Serial.print("WiFi: Connected to network");
    Serial.print(" - IP address = ");
    Serial.println(WiFi.localIP());

    s->begin();
    return 0;
}
