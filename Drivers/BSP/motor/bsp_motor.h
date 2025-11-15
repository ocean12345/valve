#ifndef __BSP_MOTOR_H__
#define __BSP_MOTOR_H__

#include "./SYSTEM/sys/sys.h"


#define STEPMOTOR_A_POS_PORT GPIOA
#define STEPMOTOR_A_POS_PIN  GPIO_PIN_0
#define STEPMOTOR_A_NEG_PORT GPIOA
#define STEPMOTOR_A_NEG_PIN  GPIO_PIN_3
#define STEPMOTOR_B_POS_PORT GPIOB
#define STEPMOTOR_B_POS_PIN  GPIO_PIN_0
#define STEPMOTOR_B_NEG_PORT GPIOB
#define STEPMOTOR_B_NEG_PIN  GPIO_PIN_1

void StepMotor_Init(void);
void StepMotor_SetSpeed(uint32_t speed_hz);
void StepMotor_Start(int dir);
void StepMotor_Stop(void);
void StepMotor_Step(void);   // 供定时器中断调用

#endif
