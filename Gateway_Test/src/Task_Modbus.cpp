//------------------------------------------------------------------------
// Libraries
//------------------------------------------------------------------------
#include "Task_ModBus.h"

byte ModTcp_RxTx(WiFiClient *client, uint8_t *pIn, uint8_t *pReg)
{
    uint8_t i;
    
    if ((pIn[4]!=0)||(pIn[5]!=6)) return 0;         // Longitud 6
    if (pIn[6]!=SlaveAddr) return 0;                // Slave ID
    if (pIn[7]!=3) return 0;                        // Holding Register

    if ((pIn[8]!=0x0f) && (pIn[9]!=0xa0))
    {
      if ((pIn[10]!=0)||(pIn[11]!=8)) return 0;     // 8 words
      pIn[5]= 16+3;                                 // Length
      pIn[8]= 16;                                   // Byte count
      for (i=0;i<16;i++)                            // Data
        pIn[9+i]=pReg[i];    

      pReg[0] = 0xff;
      client->write(pIn,25);
      
      
      /*
      Serial.println("");
      for(i=0;i<41;i++)
      {
        Serial.print(pIn[i],HEX);
        Serial.print(" ");
      }
      Serial.println("");*/

      return 1;
    }

    else if((pIn[8]!=0x0f) && (pIn[9]!=0xa8))
    {
      if ((pIn[10]!=0)||(pIn[11]!=8)) return 0;     // 8 words
      pIn[5]= 16+3;                                 // Length
      pIn[8]= 16;                                   // Byte count
      for (i=0;i<16;i++)                            // Data
        pIn[9+i]=pReg[16+i];    

      pReg[16] = 0xff;
      client->write(pIn,25);      
      return 1;
    }
    
    else
      return 0;   // Direccion 4000          
}    

void ModTcp_WiFiConn(WiFiServer *s, char *network, char*password,IPAddress *ip,IPAddress *gateway,IPAddress *subnet)
{
    if (!WiFi.config(*ip,*gateway,*subnet)) {
      Serial.println("Static IP failed to configure");
    }
    WiFi.begin(network, password);
    while(WiFi.status()!=WL_CONNECTED){
      delay(500);
      Serial.print(".");      
    }
    Serial.println("");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    s->begin();
}