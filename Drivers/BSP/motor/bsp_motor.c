#include "./BSP/motor/bsp_motor.h"
#include "./BSP/TIMER/btim.h"

volatile int8_t step_index = 0;
static volatile int8_t step_dir = 1; // 1=正转 -1=反转
static volatile int32_t g_step_pos = 0;   // 全局位置计数（单位：步）
float Iref_A;
float Iref_B;

#define MICROSTEP 16
#define I_MAX     2.5f   // 峰值电流，可以改

const float sin_tbl[16] = {
    0.0000f,  0.3827f,  0.7071f,  0.9239f,
    1.0000f,  0.9239f,  0.7071f,  0.3827f,
    0.0000f, -0.3827f, -0.7071f, -0.9239f,
   -1.0000f, -0.9239f, -0.7071f, -0.3827f
};

const float cos_tbl[16] = {
    1.0000f,  0.9239f,  0.7071f,  0.3827f,
    0.0000f, -0.3827f, -0.7071f, -0.9239f,
   -1.0000f, -0.9239f, -0.7071f, -0.3827f,
    0.0000f,  0.3827f,  0.7071f,  0.9239f
};

// 执行一步（由定时器中断触发）
void StepMotor_Step(void)
{
		step_index += step_dir;

    if (step_index >= MICROSTEP) step_index = 0;
    else if (step_index < 0)     step_index = MICROSTEP - 1;

    /* 根据查表更新目标电流 */
    Iref_A = I_MAX * sin_tbl[step_index];
    Iref_B = I_MAX * cos_tbl[step_index];
}

void StepMotor_Start(int dir)
{
    step_dir = dir;
    TIM_Step_Enable();  // 启动换相定时器
}

void StepMotor_Stop(void)
{
    TIM_Step_Disable();
}

void StepMotor_SetSpeed(uint32_t speed_hz)
{
    TIM_Step_SetFreq(speed_hz);
}
