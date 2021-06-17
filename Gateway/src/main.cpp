//-----------------------------------------------
//  Libraries
//-----------------------------------------------
#include "Global.h"
#include "Eeprom_esp.h"
#include "Rtc.h"

#include "Task_ModBus.h"
#include "Task_LoRa.h"
#include "Task_Btserial.h"

//-----------------------------------------------
//  Global variables
//-----------------------------------------------

uint8_t Register[16*32+1];              // Modbus TCP Registers
Node_TxConfig_Struct Node_TxConfig;     // Node setting to transmit
WiFiServer server(80);                  // TCP server for Modbus
BluetoothSerial Btserial;               // Bluetooth class
RTC Rtc;                                // Real time clock class
uint8_t status=0;                       // Status of devices

//-----------------------------------------------
//  Main Program
//-----------------------------------------------
void setup() 
{    
    pinMode(PIN_GW_RST,OUTPUT);
    pinMode(PIN_LED_PWR,OUTPUT);
    pinMode(PIN_LED_MSG,OUTPUT);
    digitalWrite(PIN_GW_RST,HIGH);
    digitalWrite(PIN_LED_PWR,LOW);
    digitalWrite(PIN_LED_MSG,LOW);

    delay(500);
    Serial.begin(115200);      
    Serial.println("SERIAL:    OK");

    Serial.print("EEPROM:    ");
    if (EEPROM.begin(EE_Size))  
        Serial.println("OK");
    else
        Serial.println("KO");
    delay(1000);

    Serial.print("RTC:       ");
    if(Rtc.Init())
        Serial.println("OK");
    else
        Serial.println("KO");

    Serial.print("BLUETOOTH: ");
    if(Btserial.begin("GW_Diacsa")==true)
        Serial.println("OK");
    else
        Serial.println("KO");

    Node_TxConfig.Update = false; 
    
    for(uint16_t d=0;d<(16*32+1); d++)
      Register[d] = 0xff;
                
    xTaskCreatePinnedToCore(LoRa_Task,"Task_LoRaReception",16384,NULL,2,NULL,1);
    xTaskCreatePinnedToCore(ModTcp_Task,"Task_ModbusTcp",8192,NULL,2,NULL,0);
    xTaskCreatePinnedToCore(Btserial_Task,"Task_Btserial",4096,NULL,2,NULL,0);
    
    vTaskDelete(NULL);    
}

void loop(){}

//------------------------------------------------------------------------
// Modbus TCP Task
//------------------------------------------------------------------------
void ModTcp_Task(void *pvParameters) 
{       
    static WiFiClient Client;

    ModTcp_WiFiInit(&server);
    status |= 1;

    while(true)
    {
        vTaskDelay(100/portTICK_PERIOD_MS);
        Client = server.available();        
        if(!Client) continue;
        ModTcp_ClientConnected(&Client,Register); 
        #ifdef Modbus_Debug_Memory                   
        Serial.print("Stack memory remain(Words): ");Serial.println(uxTaskGetStackHighWaterMark(NULL));
        Serial.print("Heap memory total  (Bytes): ");Serial.println(xPortGetFreeHeapSize());
        #endif
    }
}

//------------------------------------------------------------------------
//  Lora task
//------------------------------------------------------------------------
void LoRa_Task(void *pvParameters)
{  
    struct lgw_pkt_rx_s rxpkt[16]; 
    int nb_pkt, len_rxpkt;
    
    if(LoRa_GatewayInit()==false){
        vTaskDelay(1000/portTICK_PERIOD_MS);
        ESP.restart();        
    }
    status |= 2; 

    while(true)
    {
        vTaskDelay(100/portTICK_PERIOD_MS);
        LoRa_Transmit_ConfigNode(&Node_TxConfig);
        len_rxpkt = (sizeof(rxpkt) / sizeof((rxpkt)[0]));
        if(LoRa_PacketReceived_Check(rxpkt,len_rxpkt,&nb_pkt)==false) continue;
        digitalWrite(PIN_LED_MSG,HIGH);
        LoRa_PacketReceived_Process(rxpkt,&nb_pkt,&Rtc,Register);        
        vTaskDelay(200/portTICK_PERIOD_MS);
        digitalWrite(PIN_LED_MSG,LOW);
        #ifdef LoRa_Debug_Memory
        Serial.print("Stack memory remain(Words): ");Serial.println(uxTaskGetStackHighWaterMark(NULL));
        Serial.print("Heap memory total  (Bytes): ");Serial.println(xPortGetFreeHeapSize());
        #endif
    }
}

void Btserial_Task(void *pvParameters)
{        
    while(true)
    {
        vTaskDelay(500/portTICK_PERIOD_MS);
        if(status==3)   digitalWrite(PIN_LED_PWR,HIGH);
        else            digitalWrite(PIN_LED_PWR,!digitalRead(PIN_LED_PWR));        
        Rtc.Read();
        #ifdef Bt_Debug
        Rtc.Bcd2Txt();
        Serial.println(Rtc.txt_datetime);
        #endif
        Btserial_CheckCommand(&Btserial,&Rtc,&Node_TxConfig);

        #ifdef Bt_Debug_Memory
        Serial.print("Remaining stack memory (Words): ");Serial.println(uxTaskGetStackHighWaterMark(NULL));
        Serial.print("Heap memory (Bytes): ");Serial.println(xPortGetFreeHeapSize());
        #endif
    }
}