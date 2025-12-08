#ifndef __BSP_MOTOR_H__
#define __BSP_MOTOR_H__

#include "./SYSTEM/sys/sys.h"

#define S_MAX_SPEED_RPM      1000.0f     // 最大速度
#define S_MAX_ACCEL_RPM_S     700.0f     // 最大加速度 (rpm/s)
#define S_MAX_JERK_RPM_S2    25000.0f    // 最大加加速度 (rpm/s^2) —— 建议比你原来大一截
#define S_UPDATE_FREQ        1000.0f     // 控制循环频率 Hz

// 速度/加速度的收敛阈值（防止抖动）
#define S_SPEED_EPS          0.5f        // 允许目标速度误差，比如 0.5 rpm
#define S_ACCEL_EPS          1.0f        // 允许加速度误差


#define STEPMOTOR_A_POS_PORT GPIOA
#define STEPMOTOR_A_POS_PIN  GPIO_PIN_0
#define STEPMOTOR_A_NEG_PORT GPIOA
#define STEPMOTOR_A_NEG_PIN  GPIO_PIN_3
#define STEPMOTOR_B_POS_PORT GPIOB
#define STEPMOTOR_B_POS_PIN  GPIO_PIN_0
#define STEPMOTOR_B_NEG_PORT GPIOB
#define STEPMOTOR_B_NEG_PIN  GPIO_PIN_1

void S_Init(void);
void StepMotor_Init(void);
void StepMotor_UpdateMicrostep(float speed_rpm);
void StepMotor_SetSpeed(float rpm);
void StepMotor_Start(int dir);
void StepMotor_Stop(void);
void StepMotor_Step(void);   // 供定时器中断调用
void SetDir(int8_t dir);
void S_Update(void);
void S_SetTargetSpeed(float rpm);


#endif
