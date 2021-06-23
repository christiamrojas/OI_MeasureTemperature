#ifndef TASK_LORA_H
#define TASK_LORA_H

#include <Wire.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h> 
#include <time.h>   
#include <stdlib.h> 
#include <arduino.h>

#include "Rtc.h"
#include "Global.h"
#include "loragw_conf.h"
#include "loragw_hal.h"
#include "loragw_reg.h"
//#include "loragw_aux.h"
#include "loragw_debug.h"

#define LORA_POWER_27       27
#define LORA_BW_125KHz      3
#define LORA_CR_4_5         1
#define LORA_PREAMBLE       8
#define LORA_SIZETXMSG      3
#define LORA_TX_RF_CHAIN    0 

void LoRa_Task(void *pvParameters); 

bool LoRa_GatewayInit(void);
bool LoRa_PacketReceived_Check(lgw_pkt_rx_s *rxpkt,int len_rxpkt, int *nb_pkt);
bool LoRa_PacketReceived_Process(lgw_pkt_rx_s *rxpkt, int *nb_pkt, RTC *rtc, uint8_t *reg);
void LoRa_Transmit_ConfigNode(Node_TxConfig_Struct *Node_TxConfig);

#endif