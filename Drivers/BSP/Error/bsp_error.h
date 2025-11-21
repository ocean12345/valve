#ifndef __BSP_ERROR_H__
#define __BSP_ERROR_H__

#include "stm32f4xx.h"

typedef uint8_t ErrorCode_t;
extern volatile ErrorCode_t g_error_code;

#define ERR_OK                  0x00
#define ERR_PWM_FAIL            0x01
#define ERR_ADC_ZERO            0x02
#define ERR_ADC_TOO_SMALL       0x03
#define ERR_ADC_TOO_BIG         0x04
#define ERR_DIV_ZERO            0x05
#define ERR_INVALID_RESULT      0x06

// ÉèÖÃ/Çå³ý´íÎóµÄºê
#define SET_ERR(code)    do { g_error_code = (code); } while(0)
#define CLEAR_ERR()      do { g_error_code = ERR_OK; } while(0)
#define HAS_ERR()        (g_error_code != ERR_OK)

#endif // BSP_ERROR_H

