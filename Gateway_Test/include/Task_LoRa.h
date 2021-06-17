#ifndef TASK_LORA_H
#define TASK_LORA_H


#include "LoRa.h"
#include "Pins.h"

void LoRa_Task(void *pvParameters); 
void LoRa_Init(void);
bool Lora_ParseNodePacket(uint8_t *data, uint8_t *Reg, Rtc_struct *rtc, uint8_t *node);
bool Lora_CheckTxConfig(uint8_t *dev);

#endif