#include "./BSP/INA240/bsp_ina240.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"
#include "./BSP/TIMER/btim.h"
#include "./SYSTEM/usart/usart.h"


#define INA240_REF_VOLT  1.65f
#define INA240_GAIN      50.0f     // INA240A2 增益20V/V
#define SHUNT_RESISTOR   0.01f     // 10mΩ 采样电阻
#define ADC_REF_VOLT     3.3f
#define ADC_RESOLUTION   4095.0f
#define CURRENT_LIMIT_A    2.5f       // 限流阈值
#define HYSTERESIS_RATIO   0.9f       // 恢复阈值比例（2.25A以下再恢复）

/* --- 内部变量 --- */
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

volatile uint16_t adc_buf[2];       // 存放A、B两相原始值
volatile float INA240_Current_A = 0.0f;
volatile float INA240_Current_B = 0.0f;
volatile float vout = 0.0f;\
volatile uint8_t adc_flag = 0;
extern TIM_HandleTypeDef htim4;


/* 初始化 ADC1，通道 12 (PC2) */
void INA240_ADC_Init(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // PC2 -> ADC123_IN12
    // PC3 -> ADC123_IN13
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // --- DMA2 Stream0, Channel0 通常映射到 ADC1 ---
    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;          // 循环模式
    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_adc1);
    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);

    // --- ADC1 配置 ---
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = ENABLE;            // 启用扫描模式
    hadc1.Init.ContinuousConvMode = DISABLE;     // 禁止连续模式
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T5_CC2; // TIM5触发
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 2;              // 两个通道
    hadc1.Init.DMAContinuousRequests = ENABLE;   // 启用DMA请求
    hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;  // 每序列结束触发一次中断

    HAL_ADC_Init(&hadc1);

    // --- 配置通道顺序 ---
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = ADC_CHANNEL_12;            // A相
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    sConfig.Channel = ADC_CHANNEL_13;            // B相
    sConfig.Rank = 2;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    // --- 启动DMA模式采样 ---
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, 2);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
				// 取 DMA 数据
        uint16_t rawA = adc_buf[0];
        uint16_t rawB = adc_buf[1];

        float vA = rawA * ADC_REF_VOLT / ADC_RESOLUTION;
        float vB = rawB * ADC_REF_VOLT / ADC_RESOLUTION;

        float iA = (vA - INA240_REF_VOLT) / (INA240_GAIN * SHUNT_RESISTOR);
        float iB = (vB - INA240_REF_VOLT) / (INA240_GAIN * SHUNT_RESISTOR);

        // 简单低通滤波
        static float iA_f = 0.0f;
        static float iB_f = 0.0f;
        const float alpha = 0.3f;

        iA_f = iA_f + alpha * (iA - iA_f);
        iB_f = iB_f + alpha * (iB - iB_f);

        INA240_Current_A = iA_f;
        INA240_Current_B = iB_f;

        adc_flag = 1;  // 通知别的任务
    }
}



