/* 包含头文件 ----------------------------------------------------------------*/
#include "./BSP/encoder/bsp_encoder.h"


/* 私有变量 ------------------------------------------------------------------*/

int32_t OverflowCount = 0;//定时器溢出次数
/* Timer handler declaration */
TIM_HandleTypeDef    htimx_Encoder;


void ENCODER_TIMx_Init(void)
{
  /* Timer Encoder Configuration Structure declaration */
  TIM_Encoder_InitTypeDef sEncoderConfig;

  ENCODER_TIM_RCC_CLK_ENABLE();
  htimx_Encoder.Instance            = ENCODER_TIMx;
  htimx_Encoder.Init.Prescaler      = ENCODER_TIM_PRESCALER;
  htimx_Encoder.Init.CounterMode    = TIM_COUNTERMODE_UP;
  htimx_Encoder.Init.Period         = ENCODER_TIM_PERIOD;
  htimx_Encoder.Init.ClockDivision  = TIM_CLOCKDIVISION_DIV1;

  sEncoderConfig.EncoderMode        = TIM_ENCODERMODE_TIx;        // SMS，设定是TI1的边沿计数还是TI2的边沿计数，还是都计数。注，F4的宏里，TIM_ENCODERMODE_TI1表示SMS=001，是在TI2通道的边沿信号计数。TIM_ENCODERMODE_TI2时同样，是在TI1通道的边沿信号计数，反的。                            
  sEncoderConfig.IC1Polarity        = TIM_ICPOLARITY_FALLING;      // CC1P，选择计数方向。
  sEncoderConfig.IC1Selection       = TIM_ICSELECTION_DIRECTTI;   // CC1S，输入来源选择，此处TI1~TI4对应IC1~4                       
  sEncoderConfig.IC1Prescaler       = TIM_ICPSC_DIV1;             // IC1PSC，对IC1进行分频，多少次事件(有效边沿)才执行一次捕获            
  sEncoderConfig.IC1Filter          = 13;                         // IC1F，输入捕获数字滤波器
  
  sEncoderConfig.IC2Polarity        = TIM_ICPOLARITY_RISING;      // 另一输入通道配置计数方向
  sEncoderConfig.IC2Selection       = TIM_ICSELECTION_DIRECTTI;   // CC2S
  sEncoderConfig.IC2Prescaler       = TIM_ICPSC_DIV1; 
  sEncoderConfig.IC2Filter          = 13;
  __HAL_TIM_SET_COUNTER(&htimx_Encoder,0);
  
  HAL_TIM_Encoder_Init(&htimx_Encoder, &sEncoderConfig);

  __HAL_TIM_CLEAR_IT(&htimx_Encoder, TIM_IT_UPDATE);  // 清除更新中断标志位
  __HAL_TIM_URS_ENABLE(&htimx_Encoder);               // 仅允许计数器溢出才产生更新中断
  __HAL_TIM_ENABLE_IT(&htimx_Encoder,TIM_IT_UPDATE);  // 使能更新中断
  
  HAL_NVIC_SetPriority(ENCODER_TIM_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(ENCODER_TIM_IRQn);
}

void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef* htim_base)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if(htim_base->Instance==ENCODER_TIMx)
  {
    /* 基本定时器外设时钟使能 */
    ENCODER_TIM_GPIO_CLK_ENABLE();

    /* 定时器通道1功能引脚IO初始化 */
    GPIO_InitStruct.Pin       = ENCODER_TIM_CH1_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = ENCODER_TIM_GPIO_AF;
    HAL_GPIO_Init(ENCODER_TIM_CH1_GPIO, &GPIO_InitStruct);
    
    /* 定时器通道2功能引脚IO初始化 */
    GPIO_InitStruct.Pin       = ENCODER_TIM_CH2_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = ENCODER_TIM_GPIO_AF;
    HAL_GPIO_Init(ENCODER_TIM_CH2_GPIO, &GPIO_InitStruct);
  }
}

void HAL_TIM_Encoder_MspDeInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==ENCODER_TIMx)
  {
    /* 基本定时器外设时钟禁用 */
    ENCODER_TIM_RCC_CLK_DISABLE();
    
    HAL_GPIO_DeInit(ENCODER_TIM_CH1_GPIO, ENCODER_TIM_CH1_PIN);
    HAL_GPIO_DeInit(ENCODER_TIM_CH2_GPIO, ENCODER_TIM_CH2_PIN);
  }
}

void ENCODER_TIM_IRQHANDLER(void)
{
  HAL_TIM_IRQHandler(&htimx_Encoder);
}




