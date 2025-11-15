#ifndef __BSP_INA240_H__
#define __BSP_INA240_H__

#include "./SYSTEM/sys/sys.h"

void INA240_ADC_Init(void);
float INA240_GetVoltage(void);
float INA240_GetCurrent(void);
float INA240_GetCurrentAvg(uint8_t samples);
int adc_read_raw_blocking(uint32_t *raw);

#endif
