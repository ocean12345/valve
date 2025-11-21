#ifndef __BTIM_H
#define __BTIM_H

#include "./SYSTEM/sys/sys.h"

 
void TIM_Speed_Init(void);
void TIM_Step_Init(void);
void TIM_Step_SetFreq(uint32_t freq);
void TIM_Step_Enable(void);
void TIM_Step_Disable(void);


// 控制H桥 PWM 占空比（相电流）
void TIM_PWM_Init(void);
void TIM3_PWM_Init(void);
static inline float ClampDuty(float duty);
void PWM_Output_A(float duty);
void PWM_Output_B(float duty);
void TIM_PWM_SetDuty(float dutyA,float dutyB);


#endif

















