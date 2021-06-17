#include "Arduino.h"
#include <WiFi.h>



#define SlaveAddr  1

void ModTcp_WiFiConn(WiFiServer *s, char *network, char*password,IPAddress *ip,IPAddress *gateway,IPAddress *subnet);
void ModTcp_Task(void *pvParameters); 
byte ModTcp_RxTx(WiFiClient *client, uint8_t *pIn, uint8_t *pReg);
