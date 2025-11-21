#ifndef __FREERTOS_DEMO_H
#define __FREERTOS_DEMO_H

#include "stm32f4xx_hal.h"

void freertos_demo(void);
void MeasureResistance(void);
void CurrentSend(uint8_t cmd,float current_value);
void ResistanceSend(uint8_t cmd,float Resistance_value);

#endif
