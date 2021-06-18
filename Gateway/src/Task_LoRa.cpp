//------------------------------------------------------------------------
// Libraries
//------------------------------------------------------------------------

#include "Task_LoRa.h"

const uint32_t FrequencyTable[8] = {916800000,917000000,917200000,917400000,917600000,917800000,918000000,918200000};

void LoRa_Transmit_ConfigNode(Node_TxConfig_Struct *Node_TxConfig)
{
    static struct lgw_pkt_tx_s txpkt;
  
    if(Node_TxConfig->Update == false)  return;
    
    memset(&txpkt, 0, sizeof(txpkt));
    
    txpkt.freq_hz     = FrequencyTable[Node_TxConfig->FrequencyId];
    txpkt.tx_mode     = IMMEDIATE;
    txpkt.rf_chain    = LORA_TX_RF_CHAIN;
    txpkt.rf_power    = LORA_POWER_27;
    txpkt.modulation  = MOD_LORA;
    txpkt.bandwidth   = LORA_BW_125KHz;        
    txpkt.datarate    = 1<<(Node_TxConfig->SpreadFactor-6);
    txpkt.coderate    = LORA_CR_4_5;
    txpkt.invert_pol  = false;
    txpkt.preamble    = LORA_PREAMBLE;
    txpkt.size        = LORA_SIZETXMSG;
  
    txpkt.payload[0]  = Node_TxConfig->NodeAddr;
    txpkt.payload[1]  = (Node_TxConfig->SampleTime>>8) & 0xff;
    txpkt.payload[2]  = Node_TxConfig->SampleTime & 0xff;

    uint8_t status,timeout_counter=0; 
    do {
      vTaskDelay(25/portTICK_PERIOD_MS);
      timeout_counter++;
      lgw_status(TX_STATUS, &status);
                
    } while ( (status != TX_FREE) && (timeout_counter<(5000/25)) );
    if(timeout_counter>(5000/25))
    {
      Node_TxConfig->Update = false; 
      Serial.println("LoRa: TX timeout");
      return; 
    }
     
    
    if(lgw_send(txpkt)==LGW_HAL_ERROR)
      Serial.println("LoRa: TX KO");
    else
      Serial.println("LoRa: TX OK");

    Node_TxConfig->Update = false; 
}

bool LoRa_PacketReceived_Check(lgw_pkt_rx_s *rxpkt,int len_rxpkt, int *nb_pkt)
{           
    *nb_pkt = lgw_receive(len_rxpkt,rxpkt);
    if(*nb_pkt==LGW_HAL_ERROR) return false;
    if(*nb_pkt==0) return false;
    return true;
}

bool LoRa_PacketReceived_Process(lgw_pkt_rx_s *rxpkt, int *nb_pkt, RTC *rtc, uint8_t *reg)
{
    struct lgw_pkt_rx_s *p;
        
    for(uint8_t i=0; i<*nb_pkt; i++)   
    {
      p = &rxpkt[i];

      if( (p->status != STAT_CRC_OK) || (p->size!=10) )
      {      
        uint8_t t;
        for(uint16_t j=0; j<p->size; j++)        
          t = p->payload[j];
          continue; 
      }

      uint8_t   id = (p->payload[0]);
      uint16_t  addr = id<<4;
      uint16_t  temp;      
      
      reg[addr]   = rtc->hour;
      reg[addr+1] = rtc->minute;
      reg[addr+2] = rtc->second;
      reg[addr+3] = rtc->day;
      reg[addr+4] = rtc->month;
      reg[addr+5] = rtc->year;

      temp = (p->payload[1]<<8) + p->payload[2];      
      if( (temp & 0x02) == 1 )  reg[addr+14] = 0xff;
      if( (temp & 0x01) == 1 )  temp = 0xffff;
      else temp >>= 4; 
      reg[addr+6] = (temp>>8) & 0xff;
      reg[addr+7] = temp & 0xff;
      #ifdef LoRa_Debug
      Serial.print(temp);
      Serial.print(" ");
      #endif


      temp = (p->payload[3]<<8) + p->payload[4];
      if( (temp & 0x02) == 1 )  reg[addr+14] = 0xff;
      if( (temp & 0x01) == 1 )  temp = 0xffff;
      else temp >>= 4; 
      reg[addr+8] = (temp>>8) & 0xff;
      reg[addr+9] = temp & 0xff;
      #ifdef LoRa_Debug
      Serial.print(temp);
      Serial.print(" ");
      #endif

      temp = (p->payload[5]<<8) + p->payload[6];
      if( (temp & 0x02) == 1 )  reg[addr+14] = 0xff;
      if( (temp & 0x01) == 1 )  temp = 0xffff;
      else temp >>= 4; 
      reg[addr+10] = (temp>>8) & 0xff;
      reg[addr+11] = temp & 0xff;
      #ifdef LoRa_Debug
      Serial.print(temp);
      Serial.print(" ");
      #endif

      temp = (p->payload[7]<<8) + p->payload[8];
      if( (temp & 0x02) == 1 )  reg[addr+14] = 0xff;
      if( (temp & 0x01) == 1 )  temp = 0xffff;
      else temp >>= 4; 
      reg[addr+12] = (temp>>8) & 0xff;
      reg[addr+13] = temp & 0xff;
      #ifdef LoRa_Debug
      Serial.print(temp);
      Serial.print(" ");
      #endif

      int n; uint8_t t;
      
      if(id==0){        // Between [0xd7:0xbb]
        n = ((p->payload[9]-0xbb)*100/0x1c);
        if(n>100) n=100;
        else if(n<0) n=0;
        t = (uint8_t)(n);
      }
      else{             // Between [0xdc:0xc1]
        n = ((p->payload[9]-0xc1)*100/0x1b);
        if(n>100) n=100;
        else if(n<0) n=0;
        t = (uint8_t)(n);      
      }
      reg[addr+15] = t;
      #ifdef LoRa_Debug
      Serial.print(t);
      Serial.println("");
      #endif
      
    }

    return true;
}

//------------------------------------------------------------------------
bool LoRa_GatewayInit(void)
{    
    LoRa_SX1301_Configuration();          
  
    if(lgw_start() != LGW_HAL_SUCCESS){
      Serial.println("LoRa: Failed to start Gateway");
      return false; 
    }      
    Serial.println("LoRa: Gateway started");
    return true;       
}