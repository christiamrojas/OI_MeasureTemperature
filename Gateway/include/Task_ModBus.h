#include "Arduino.h"
#include "Global.h"
#include "Eeprom_esp.h"

#include <WiFi.h>

void ModTcp_Task(void *pvParameters); 

uint8_t ModTcp_WiFiInit(WiFiServer *s);
uint8_t ModTcp_RxTx(WiFiClient *client, uint8_t *pIn, uint8_t *pReg);
void ModTcp_ClientConnected(WiFiClient *client,uint8_t *pReg);
