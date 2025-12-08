#include "./BSP/PWM/bsp_pwm.h"
#include "./BSP/INA240/bsp_ina240.h"
#include "./BSP/TIMER/btim.h"
#include "./BSP/motor/bsp_motor.h"
#include "stdbool.h"
#include <math.h>

extern ADC_HandleTypeDef hadc1;
extern float INA240_Current_A;
extern float INA240_Current_B;
extern float Iref_A;
extern float Iref_B;


// ---- 电流闭环参数 ----
#define PWM_MIN       0.0f       // 最小占空比
#define PWM_MAX       0.950f       // 最大占空比

// ---- PI控制参数（根据系统调试调整）----
#define KP            1.5f
#define KI            0.1f      // 积分增益与采样周期有关
#define CURRENT_LPF_K 0.2f  // 电流滤波
#define CURRENT_DEADBAND 0.05f  // 死区，减少抖动
#define LOOP_PERIOD_S 0.001f      // 控制周期（1ms）
#define FILTER_COEF   0.1f
#define I_ERR_DEADZONE   0.02f      // 电流误差死区 [A]，小于这个就不积分（自己按实际改）
#define INT_LIMIT        PWM_MAX    // 积分最大值（和输出同量纲）

// ---- 内部状态 ----
float pwm_outA = 0.1f;
float integral_A = 0.0f;
float integral_B = 0.0f;
float filt_A = 0.0f;
float filt_B = 0.0f;
float uA = 0.0f;
float uB = 0.0f;

//电流环
void CurrentLoop_Update(void)
{
		float iA = INA240_Current_A;
    float iB = INA240_Current_B;

    // 一阶低通滤波
    filt_A += FILTER_COEF * (iA - filt_A);
    filt_B += FILTER_COEF * (iB - filt_B);

    float eA = Iref_A - filt_A;
    float eB = Iref_B - filt_B;

    // 只用 P
    uA = KP * eA;
    uB = KP * eB;


    //================ A 相积分 =================
    integral_A += eA * KI * LOOP_PERIOD_S;

    if (integral_A >  PWM_MAX) integral_A =  PWM_MAX;
    if (integral_A < -PWM_MAX) integral_A = -PWM_MAX;

    // PI 输出
    uA = KP * eA + integral_A;

    // 输出限幅
    if (uA >  PWM_MAX) uA =  PWM_MAX;
    if (uA < -PWM_MAX) uA = -PWM_MAX;

//    //================ B 相积分 =================
    integral_B += eB * KI * LOOP_PERIOD_S;

    if (integral_B >  PWM_MAX) integral_B =  PWM_MAX;
    if (integral_B < -PWM_MAX) integral_B = -PWM_MAX;

    uB = KP * eB + integral_B;

    if (uB >  PWM_MAX) uB =  PWM_MAX;
    if (uB < -PWM_MAX) uB = -PWM_MAX;


    TIM_PWM_SetDuty(uA, uB);
}
