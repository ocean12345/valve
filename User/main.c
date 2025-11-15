#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/SRAM/sram.h"
#include "./MALLOC/malloc.h"
#include "./BSP/encoder/bsp_encoder.h"
#include "./BSP/motor/bsp_motor.h"
#include "./BSP/INA240/bsp_ina240.h"
#include "./BSP/TIMER/btim.h"
#include "freertos_demo.h"


extern TIM_HandleTypeDef htim2;

int main(void)
{
    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(336, 8, 2, 7); /* 设置时钟,168Mhz */
		HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    delay_init(168);                    /* 延时初始化 */
    usart_init(115200);                 /* 串口初始化为115200 */
    led_init();                         /* 初始化LED */
    lcd_init();                         /* 初始化LCD */
    key_init();                         /* 初始化按键 */
    sram_init();                        /* SRAM初始化 */
		//StepMotor_Init();
    TIM_Step_Init();
    TIM_PWM_Init();
	  TIM3_PWM_Init();
		HAL_Delay(1);
		INA240_ADC_Init();
		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
		HAL_NVIC_SetPriority(ADC_IRQn, 6, 0);
		HAL_NVIC_EnableIRQ(ADC_IRQn);
		HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 6, 0);
		HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

	  HAL_TIM_Encoder_Start(&htimx_Encoder, TIM_CHANNEL_ALL);
    my_mem_init(SRAMIN);                /* 初始化内部SRAM内存池 */
    my_mem_init(SRAMEX);                /* 初始化外部SRAM内存池 */
    my_mem_init(SRAMCCM);               /* 初始化内部CCM内存池 */

    freertos_demo();                    /* 运行FreeRTOS例程 */
}
