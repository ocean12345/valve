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
#define I_REF         1.5f        // 目标电流（A）
#define PWM_MIN       0.05f       // 最小占空比
#define PWM_MAX       0.90f       // 最大占空比

// ---- PI控制参数（根据系统调试调整）----
#define KP            0.2f
#define KI            1.0f      // 积分增益与采样周期有关
#define CURRENT_LPF_K 0.2f  // 电流滤波
#define CURRENT_DEADBAND 0.05f  // 死区，减少抖动
#define LOOP_PERIOD_S 0.001f      // 控制周期（1ms）

// ---- 内部状态 ----
float pwm_outA = 0.1f;
float integral_A = 0.0f;
float integral_B = 0.0f;
float filt_A = 0.0f;
float filt_B = 0.0f;
float uA = 0.0f;
float uB = 0.0f;

// ---- 电流闭环更新函数 ----
void CurrentLoop_Update(void)
{
//		// (1) 把 INA240 的负电流统一为正方向
//    float i_measA = -INA240_Current_A;

//    // (2) 电流低通滤波
//    filt_currA += CURRENT_LPF_K * (i_measA - filt_currA);

//    // (3) 误差：目标 - 实际
//    float errorA = I_REF - filt_currA;

//    // (4) 死区
//    if (fabsf(errorA) < CURRENT_DEADBAND)
//        errorA = 0.0f;

//    // 5) 先计算 P+I
//    integralA += errorA * KI * LOOP_PERIOD_S;

//    float u = KP * errorA + integralA;

//    // 6) 限幅 + 简单 anti-windup（超限时把积分拉回一点）
//    if (u > PWM_MAX) {
//        u = PWM_MAX;
//        // 拉一点积分，避免长期顶死
//        integralA = PWM_MAX - KP * errorA;
//    }
//    else if (u < PWM_MIN) {
//        u = PWM_MIN;
//        integralA = PWM_MIN - KP * errorA;
//    }

//    pwm_outA = u;

//    TIM_PWM_SetDuty(pwm_outA,0.15);
		
		
		float iA = -INA240_Current_A;
    float iB = -INA240_Current_B;

    // 低通滤波
    filt_A += 0.2f * (iA - filt_A);
    filt_B += 0.2f * (iB - filt_B);

    float eA = Iref_A - filt_A;
    float eB = Iref_B - filt_B;

    integral_A += eA * KI * LOOP_PERIOD_S;
    integral_B += eB * KI * LOOP_PERIOD_S;

    uA = KP * eA + integral_A;
    uB = KP * eB + integral_B;
		
//		uA = Iref_A;
//		uB = Iref_B;
    // 限幅
    if (uA > PWM_MAX) { uA = PWM_MAX; integral_A -= 0.001f; }
    if (uA < -PWM_MAX) { uA = -PWM_MAX; integral_A -= 0.001f; }

    if (uB > PWM_MAX) { uB = PWM_MAX; integral_B -= 0.001f; }
    if (uB < -PWM_MAX) { uB = -PWM_MAX; integral_B -= 0.001f; }

    TIM_PWM_SetDuty(uA, uB);
}
