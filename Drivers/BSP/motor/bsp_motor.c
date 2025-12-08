#include "./BSP/motor/bsp_motor.h"
#include "./BSP/TIMER/btim.h"
#include "math.h"

volatile int8_t step_index = 0;
static volatile int8_t step_dir = 1; // 1=正转 -1=反转
static volatile int32_t g_step_pos = 0;   // 全局位置计数（单位：步）
float Iref_A;
float Iref_B;
float theta = 0.0f;   // 电角，范围 0~1，对应 0~360°
float theta_step = 0.0f;
float target_speed;
float speed_rpm;
float current_speed;

int a_max = 100;
uint8_t microstep = 64;     // 当前细分
#define I_MAX     2.5f   // 峰值电流，可以改
#define STEP_ISR_FREQ 20000
#define ELECTRICAL_REV_PER_MECH_REV 50

// 64步表
const float sin64[64] = {
     0.0000f,  0.0980f,  0.1951f,  0.2903f,
     0.3827f,  0.4714f,  0.5556f,  0.6344f,
     0.7071f,  0.7730f,  0.8315f,  0.8819f,
     0.9239f,  0.9569f,  0.9808f,  0.9952f,
     1.0000f,  0.9952f,  0.9808f,  0.9569f,
     0.9239f,  0.8819f,  0.8315f,  0.7730f,
     0.7071f,  0.6344f,  0.5556f,  0.4714f,
     0.3827f,  0.2903f,  0.1951f,  0.0980f,
     0.0000f, -0.0980f, -0.1951f, -0.2903f,
    -0.3827f, -0.4714f, -0.5556f, -0.6344f,
    -0.7071f, -0.7730f, -0.8315f, -0.8819f,
    -0.9239f, -0.9569f, -0.9808f, -0.9952f,
    -1.0000f, -0.9952f, -0.9808f, -0.9569f,
    -0.9239f, -0.8819f, -0.8315f, -0.7730f,
    -0.7071f, -0.6344f, -0.5556f, -0.4714f,
    -0.3827f, -0.2903f, -0.1951f, -0.0980f
};

const float cos64[64] = {
     1.0000f,  0.9952f,  0.9808f,  0.9569f,
     0.9239f,  0.8819f,  0.8315f,  0.7730f,
     0.7071f,  0.6344f,  0.5556f,  0.4714f,
     0.3827f,  0.2903f,  0.1951f,  0.0980f,
     0.0000f, -0.0980f, -0.1951f, -0.2903f,
    -0.3827f, -0.4714f, -0.5556f, -0.6344f,
    -0.7071f, -0.7730f, -0.8315f, -0.8819f,
    -0.9239f, -0.9569f, -0.9808f, -0.9952f,
    -1.0000f, -0.9952f, -0.9808f, -0.9569f,
    -0.9239f, -0.8819f, -0.8315f, -0.7730f,
    -0.7071f, -0.6344f, -0.5556f, -0.4714f,
    -0.3827f, -0.2903f, -0.1951f, -0.0980f,
     0.0000f,  0.0980f,  0.1951f,  0.2903f,
     0.3827f,  0.4714f,  0.5556f,  0.6344f,
     0.7071f,  0.7730f,  0.8315f,  0.8819f,
     0.9239f,  0.9569f,  0.9808f,  0.9952f
};

void SetDir(int8_t dir)
{
	step_dir = dir;
}


void StepMotor_UpdateMicrostep(float rpm)
{
    uint8_t new_step;
    if (rpm < 80) new_step = 64;
    else if (rpm < 160) new_step = 32;
		else if (rpm < 320) new_step = 16;
    else if (rpm < 700) new_step = 8;
		else new_step = 4;

    microstep = new_step;
}
int idx;

void StepMotor_Step(void)
{
//    // 推进 θ
//    theta += theta_step * step_dir;
//    if (theta >= 1.0f) theta -= 1.0f;
//    if (theta <  0.0f) theta += 1.0f;

//    // 计算查表索引
//    int base = 64 / microstep;               // 4:16, 8:8, 16:4
//    int idx  = ((int)(theta * microstep)) & (microstep - 1);
//	  int tbl_idx = (int)(theta * 64.0f) & 63;

//	
//    float s = sin64[idx * base];
//    float c = cos64[idx * base];

//    Iref_A = s * I_MAX;
//    Iref_B = c * I_MAX;
	  theta += theta_step * step_dir;
    if (theta >= 1.0f) theta -= 1.0f;
    if (theta <  0.0f) theta += 1.0f;

    // 完全根据 θ 来查表 —— 正确方式
    int tbl_idx = (int)(theta * 64.0f) & 63;

    float s = sin64[tbl_idx];
    float c = cos64[tbl_idx];

    Iref_A = s * I_MAX;
    Iref_B = c * I_MAX;
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

void StepMotor_SetSpeed(float rpm)
{
		speed_rpm = rpm;
	
    float rps = rpm / 60.0f;
    float electrical_freq = rps * ELECTRICAL_REV_PER_MECH_REV;

    // 电角每秒多少周期 / TIM7频率 = 每次中断推进多少θ
    theta_step = electrical_freq / STEP_ISR_FREQ;
		 
    if (theta_step < 0.0000001f)
        theta_step = 0.0f;
		
    // 限制 θ_step 范围
    if (theta_step > 1.0f)
        theta_step = 1.0f;
}

void S_SetTargetSpeed(float rpm)
{
//		if(current_speed<30) 
//		{
//			StepMotor_SetSpeed(30);
//			current_speed = 30;
//		}
    if (rpm > S_MAX_SPEED_RPM) rpm = S_MAX_SPEED_RPM;
    if (rpm < -S_MAX_SPEED_RPM) rpm = -S_MAX_SPEED_RPM;

    target_speed = rpm;
}


void S_Update(void)
{
		if(current_speed>200) a_max=600;
    const float dt = 1.0f / S_UPDATE_FREQ;

    if (current_speed < target_speed) {
        current_speed += a_max * dt;
        if (current_speed > target_speed) current_speed = target_speed;
    } else {
        current_speed = target_speed;
    }

    StepMotor_SetSpeed(current_speed);
}

void StepMotor_SetPos(int32_t pos)
{
	
}
