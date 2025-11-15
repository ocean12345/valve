/**
  ******************************************************************************
  * 文件名称: bsp_encoder.h
  * 作    者: 硬石嵌入式开发团队
  * 版    本: V1.0
  * 编写日期: 2024-10-08
  * 功    能: -
  ******************************************************************************
  * 说明：
  * 本例程配套硬石stm32开发板YS-F4PSTD使用。
  *
  * 淘宝：
  * 论坛：http://www.ing10bbs.com
  * 版权归硬石嵌入式开发团队所有，请勿商用。
  ******************************************************************************
  */

#ifndef __BSP_ENCODER_H__
#define __BSP_ENCODER_H__
/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"


/* 类型定义 ------------------------------------------------------------------*/



/* 宏定义 --------------------------------------------------------------------*/



#define ENCODER_TIMx                        TIM4
#define ENCODER_TIM_RCC_CLK_ENABLE()        __HAL_RCC_TIM4_CLK_ENABLE()
#define ENCODER_TIM_RCC_CLK_DISABLE()       __HAL_RCC_TIM4_CLK_DISABLE()

#define ENCODER_TIM_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define ENCODER_TIM_CH1_PIN                 GPIO_PIN_6
#define ENCODER_TIM_CH1_GPIO                GPIOB
#define ENCODER_TIM_CH2_PIN                 GPIO_PIN_7
#define ENCODER_TIM_CH2_GPIO                GPIOB

#define ENCODER_TIM_GPIO_AF                 GPIO_AF2_TIM4

#define TIM_ENCODERMODE_TIx                 TIM_ENCODERMODE_TI12

#define ENCODER_TIM_IRQn                    TIM4_IRQn
#define ENCODER_TIM_IRQHANDLER              TIM4_IRQHandler

// 定义定时器预分频，定时器实际时钟频率为：84MHz/（ENCODER_TIMx_PRESCALER+1）
#define ENCODER_TIM_PRESCALER               0  // 


// 使用16bits 的计数器作为编码器计数
#define ENCODER_TIM_PERIOD                0xFFFF
#define CNT_MAX                           65536




/* 扩展变量 ------------------------------------------------------------------*/

extern TIM_HandleTypeDef htimx_Encoder;
extern int32_t OverflowCount ;//定时器溢出次数



/* 函数声明 ------------------------------------------------------------------*/

void ENCODER_TIMx_Init(void);



#endif /* bsp_encoder.h */
