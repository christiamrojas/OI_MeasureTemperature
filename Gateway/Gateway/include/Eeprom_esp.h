#ifndef EEPROM_ESP_H
#define EEPROM_ESP_H

#include "EEPROM.h"
#include "Arduino.h"

void Eeprom_Read(uint16_t addr,uint16_t n,uint8_t *pout);
void Eeprom_Write(uint16_t addr,uint16_t n, uint8_t *pin);

#endif