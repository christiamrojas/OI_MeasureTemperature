#ifndef TASK_SERIAL_H
#define TASK_SERIAL_H

#include "Arduino.h"
#include "BluetoothSerial.h"

#include "Eeprom_esp.h"
#include "Global.h"
#include "Rtc.h"
#include "Str.h"

void Btserial_Task(void *pvParameters);

void Btserial_CheckCommand(BluetoothSerial *Btserial, RTC *rtc, Node_TxConfig_Struct *Node_TxConfig);
bool Btserial_CheckCommand_SetDateTime(char *p, BluetoothSerial *Btserial, RTC *rtc);
bool Btserial_CheckCommand_SetTxPeriod(char *p, BluetoothSerial *Btserial, Node_TxConfig_Struct *Node_TxConfig);
bool Btserial_CheckCommand_SetWiFi(char *p, BluetoothSerial *Btserial);

#endif