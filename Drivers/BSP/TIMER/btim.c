#include "math.h"
#include "./BSP/LED/led.h"
#include "./BSP/TIMER/btim.h"
#include "./BSP/encoder/bsp_encoder.h"
#include "./BSP/motor/bsp_motor.h"
#include "./BSP/PWM/bsp_pwm.h"
#include "FreeRTOS.h"
#include "task.h"


#define PWM_MIN       0.05f
#define PWM_MAX       0.20f

extern uint8_t adc_flag;
extern float INA240_Current_A;
extern float INA240_Current_B;
extern uint8_t control_flag;

TIM_HandleTypeDef htim7;   // 用于步进换相
TIM_HandleTypeDef htim5;   // PWM定时器
TIM_HandleTypeDef htim3;   // PWM定时器

void TIM_Step_Init(void)
{
    __HAL_RCC_TIM7_CLK_ENABLE();

		htim7.Instance = TIM7;
    htim7.Init.Prescaler = 84 - 1;        // 1MHz 时基
    htim7.Init.Period = 1000 - 1;         // 默认 1kHz 步进频率
    htim7.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim7.Init.CounterMode = 0;           // 基础定时器无效项，填 0 更直观

    HAL_TIM_Base_Init(&htim7);

    HAL_NVIC_SetPriority(TIM7_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);

    //HAL_TIM_Base_Start_IT(&htim7);
}

void TIM_Step_SetFreq(uint32_t freq)
{
if (freq < 1) freq = 1;
    uint32_t period = 1000000UL / freq;
    if (period < 50) period = 50;

    __HAL_TIM_SET_AUTORELOAD(&htim7, period - 1);
}

void TIM_Step_Enable(void)
{
    __HAL_TIM_SET_COUNTER(&htim7, 0);
    HAL_TIM_Base_Start_IT(&htim7);
}

void TIM_Step_Disable(void)
{
    HAL_TIM_Base_Stop_IT(&htim7);
}

//----------------------------------------------------
// === TIM5：PWM 控制相电流 ===
//----------------------------------------------------
void TIM_PWM_Init(void)
{
	// --- 1. 开启时钟 ---
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_TIM5_CLK_ENABLE();

  // --- 2. 配置 GPIOA.3 为定时器复用功能 ---
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;          // 复用推挽输出
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;       // 复用为 TIM5_CH4
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

 // --- 3. 配置 TIM5 为 PWM 模式 ---
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 84 - 1;        // 1MHz计数频率 (84MHz / 84)
  htim5.Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
  htim5.Init.Period = 25 - 1;           // 20kHz PWM (1MHz / 50)
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_PWM_Init(&htim5);	
	
	// === CH4 用作PWM输出 ===
  TIM_OC_InitTypeDef sConfigOC = {0};
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;                           
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	
	HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_4);

	HAL_NVIC_SetPriority(TIM5_IRQn, 1, 3); /* 抢占1，子优先级3，组2 */
  HAL_NVIC_EnableIRQ(TIM5_IRQn);         /*开启ITM3中断*/
	
	
	HAL_TIM_Base_Start_IT(&htim5);
  // === 启动PWM ===
	HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4);
}


void TIM3_PWM_Init(void)
{
/* 1. 开启 GPIOB 和 TIM3 时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    /* 2. 配置 PB0/PB1 为 TIM3_CH3 / TIM3_CH4 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;   // **必须是 AF2**
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 3. TIM3 基本 PWM 参数 */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 84 - 1;      // 1MHz 时基
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 25 - 1;       // 20kHz PWM
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim3);

    /* 4. CH3 / CH4 输出 50% PWM */
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 5;              
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3);
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4);

		/* 5. 配置 ADC 触发用 OC1（不输出，只产生事件） */
		TIM_OC_InitTypeDef sTrigOC = {0};
		sTrigOC.OCMode = TIM_OCMODE_PWM1;
		sTrigOC.Pulse = htim3.Init.Period / 2;   // ? 在 ARR/2 触发
		HAL_TIM_OC_ConfigChannel(&htim3, &sTrigOC, TIM_CHANNEL_1);

		/* 6. 配置 TRGO = OC1REF（HAL 方式） */
		TIM_MasterConfigTypeDef sMasterConfig = {0};
		sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC1REF;
		sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
		HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

		/* 7. 启动 OC1 （产生 TRGO） */
		HAL_TIM_OC_Start(&htim3, TIM_CHANNEL_1);
		
    /* 8. 启动 PWM */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
}



//占空比限制
static inline float ClampDuty(float duty)
{
    if (duty > 0.0f)
    {
        if (duty < PWM_MIN) duty = PWM_MIN;
        if (duty > PWM_MAX) duty = PWM_MAX;
    }
    else if (duty < 0.0f)
    {
        if (duty > -PWM_MIN) duty = -PWM_MIN;
        if (duty < -PWM_MAX) duty = -PWM_MAX;
    }
    else
    {
        duty = 0.0f;
    }
    return duty;
}

//
void PWM_Output_A(float duty)
{
    duty = ClampDuty(duty);

    uint32_t period = htim5.Init.Period;
    uint32_t pwm = (uint32_t)(fabsf(duty) * period);
	
    if (duty > 0)
    {
        // 正向电流：A+ = PWM，A- = 0
        __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_4, pwm); 
        __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, 0);
    }
    else if (duty < 0)
    {
        // 反向电流：A+ = 0，A- = PWM
        __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_4, 0); 
        __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, pwm);
    }
    else
    {
        // 浮空
        __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_4, 0);
        __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, 0);
    }
}


void PWM_Output_B(float duty)
{
    duty = ClampDuty(duty);

    uint32_t period = htim3.Init.Period;
    uint32_t pwm = (uint32_t)(fabsf(duty) * period);

    if (duty > 0)
    {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pwm);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
    }
    else if (duty < 0)
    {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, pwm);
    }
    else
    {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
    }
}

void TIM_PWM_SetDuty(float dutyA, float dutyB)
{
    PWM_Output_A(dutyA);
    PWM_Output_B(dutyB);
}


// === TIM6 中断服务函数 ===
void TIM7_IRQHandler(void)
{
		if (__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE) != RESET)
    {
        if (__HAL_TIM_GET_IT_SOURCE(&htim7, TIM_IT_UPDATE) != RESET)
        {
            __HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
            StepMotor_Step();   // 每次定时溢出走一步
        }
    }
}


void TIM5_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim5);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
		if (htim->Instance == TIM5)      // TIM5: 20kHz PWM
    {
        static uint8_t loop_div = 0;

        // 每 10 次 PWM 周期跑一次电流环 → 20k / 4 = 5kHz
        if (++loop_div >= 4)
        {
            loop_div = 0;
            if (adc_flag&&control_flag)   // 确保本周期 ADC 已经采完
            {
                adc_flag = 0;
                CurrentLoop_Update();   
            }
        }
    }
		if (htim->Instance == TIM2)
		{
			if(__HAL_TIM_IS_TIM_COUNTING_DOWN(&htimx_Encoder))    // 读CR1的DIR位，0为计数器递增，1为计数器递减，若非中心对齐模式，则仅表示为电机正或反的转动方向              
				OverflowCount--;       //向下计数溢出
			else
				OverflowCount++;       //向上计数溢出
		}
}

