//-----------------------------------------------
//  Libraries
//-----------------------------------------------
#include "Global.h"
#include "Eeprom_esp.h"
#include "Rtc.h"

#include "Task_ModBus.h"
#include "Task_LoRa.h"
#include "Task_Btserial.h"

#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include"esp_gap_bt_api.h"
#include "esp_err.h"

//-----------------------------------------------
//  Global variables
//-----------------------------------------------

uint8_t Register[16*32];                // Modbus TCP Registers
Node_TxConfig_Struct Node_TxConfig;     // Node setting to transmit
WiFiServer server(80);                  // TCP server for Modbus
BluetoothSerial Btserial;               // Bluetooth class
RTC Rtc;                                // Real time clock class
uint8_t status=0;                       // Status of devices

#ifdef Bt_Review_Devices
#define REMOVE_BONDED_DEVICES 1   // <- Set to 0 to view all bonded devices addresses, set to 1 to remove
#define PAIR_MAX_DEVICES 20
uint8_t pairedDeviceBtAddr[PAIR_MAX_DEVICES][6];
char bda_str[18];

char *bda2str(const uint8_t* bda, char *str, size_t size)
{
  if (bda == NULL || str == NULL || size < 18) {
    return NULL;
  }
  sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
          bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
  return str;
}
#endif
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

    #ifndef Bt_Review_Devices
    Serial.print("BLUETOOTH: ");
    if(Btserial.begin("GW_Diacsa")==true)
        Serial.println("OK");
    else
        Serial.println("KO");
    #endif
    
    #ifdef Bt_Review_Devices
    if(!btStart())  Serial.println("Failed to initialize controller");
    if(esp_bluedroid_init() != ESP_OK) Serial.println("Failed to initialize bluedroid");       
    if(esp_bluedroid_enable() != ESP_OK) Serial.println("Failed to enable bluedroid");
    Serial.print("ESP32 bluetooth address: "); Serial.println(bda2str(esp_bt_dev_get_address(), bda_str, 18));
    
    int count = esp_bt_gap_get_bond_device_num();
    if(!count)
        Serial.println("No bonded device found.");
    else {
        Serial.print("Bonded device count: "); Serial.println(count);

        esp_err_t tError =  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
        if(ESP_OK == tError) {
            for(int i = 0; i < count; i++) {
                Serial.print("Found bonded device # "); Serial.print(i); Serial.print(" -> ");
                Serial.println(bda2str(pairedDeviceBtAddr[i], bda_str, 18));     
                if(REMOVE_BONDED_DEVICES) {
                esp_err_t tError = esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
                if(ESP_OK == tError)
                    Serial.print("Removed bonded device # "); 
                else 
                    Serial.print("Failed to remove bonded device # ");          
                Serial.println(i);
                }
            }        
        }
    }
    while(1);
    #endif

    Node_TxConfig.Update = false; 
    
    for(uint16_t d=0;d<(16*32); d++)
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

    if(ModTcp_WiFiInit(&server)==0)
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
    
    if(LoRa_GatewayInit()==true)
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