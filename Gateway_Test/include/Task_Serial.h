#ifndef TASK_SERIAL_H
#define TASK_SERIAL_H

#include "Arduino.h"
#include "Global.h"

void Serial_Task(void *pvParameters);
bool Serial_UpdateTime(uint8_t *p, Rtc_struct *rtc);
bool Serial_UpdateDeviceSampling(uint8_t *p, uint8_t *devSamp);

#endif