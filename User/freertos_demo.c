#include "freertos_demo.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/USART/usart.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/encoder/bsp_encoder.h"
#include "./BSP/motor/bsp_motor.h"
#include "./BSP/INA240/bsp_ina240.h"
#include "./BSP/TIMER/btim.h"
#include "./BSP/PWM/bsp_pwm.h"
#include "./BSP/protocol/protocol.h"
#include "string.h"
#include "math.h"
#include "./BSP/Error/bsp_error.h"
/*FreeRTOS*********************************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "stdbool.h"

__IO int64_t CaptureNumber=0;     // 输入捕获数

/* 扩展变量 ------------------------------------------------------------------*/

extern __IO uint32_t uwTick;
extern int32_t OverflowCount ;    // 定时器溢出次数 
extern float INA240_Current_A;
extern float INA240_Current_B;
extern uint8_t adc_flag;
extern float vout;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim7;
extern UART_HandleTypeDef g_uart1_handle;
extern ProtocolFrame_t rxFrame;
extern int LastPos;
extern int NewPos;
extern uint8_t SpeedFlag;

int PosChange;
uint8_t control_flag = 0;
uint8_t ErrorCode = 0x00;
float ResistanceA=0.0f;
float ResistanceB=0.0f;

static uint8_t txbufA[PROTOCOL_FRAME_MAX_LEN];
static uint8_t txbufB[PROTOCOL_FRAME_MAX_LEN];
static uint8_t* buf_prepare = txbufA;
static uint8_t* buf_send    = txbufB;
/******************************************************************************************************/
/*FreeRTOS配置*/

/* START_TASK 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define START_TASK_PRIO 1                   /* 任务优先级 */
#define START_STK_SIZE  128                 /* 任务堆栈大小 */
TaskHandle_t            StartTask_Handler;  /* 任务句柄 */
void start_task(void *pvParameters);        /* 任务函数 */

/* TASK1 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define TASK1_PRIO      2                   /* 任务优先级 */
#define TASK1_STK_SIZE  128                 /* 任务堆栈大小 */
TaskHandle_t            Task1Task_Handler;  /* 任务句柄 */
void task1(void *pvParameters);             /* 任务函数 */

/* TASK2 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define TASK2_PRIO      2                   /* 任务优先级 */
#define TASK2_STK_SIZE  1024                 /* 任务堆栈大小 */
TaskHandle_t            Task2Task_Handler;  /* 任务句柄 */
void task2(void *pvParameters);             /* 任务函数 */

/* TASK3 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define TASK3_PRIO      2                   /* 任务优先级 */
#define TASK3_STK_SIZE  512                 /* 任务堆栈大小 */
TaskHandle_t            Task3Task_Handler;  /* 任务句柄 */
void task3(void *pvParameters);             /* 任务函数 */


void StepMotor_PrintPinState(void);
void MeasureCurrent(void);
void CurrentControl(void);
void EncoderSend(void);
void CurrentSend(uint8_t cmd,float current_value);
void MeasureResistance(void);
void ResistanceSend(uint8_t cmd,float Resistance_value);
void SpeedSend(void);
void TargetSpeed(void);
void ErrorSend(void);
/******************************************************************************************************/

void freertos_demo(void)
{
    
    xTaskCreate((TaskFunction_t )start_task,            /* 任务函数 */
                (const char*    )"start_task",          /* 任务名称 */
                (uint16_t       )START_STK_SIZE,        /* 任务堆栈大小 */
                (void*          )NULL,                  /* 传入给任务函数的参数 */
                (UBaseType_t    )START_TASK_PRIO,       /* 任务优先级 */
                (TaskHandle_t*  )&StartTask_Handler);   /* 任务句柄 */
    vTaskStartScheduler();
}


void start_task(void *pvParameters)
{

    taskENTER_CRITICAL();           /* 进入临界区 */
    /* 创建任务1 */
    xTaskCreate((TaskFunction_t )task1,
                (const char*    )"task1",
                (uint16_t       )TASK1_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK1_PRIO,
                (TaskHandle_t*  )&Task1Task_Handler);
    /* 创建任务2 */
    xTaskCreate((TaskFunction_t )task2,
                (const char*    )"task2",
                (uint16_t       )TASK2_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK2_PRIO,
                (TaskHandle_t*  )&Task2Task_Handler);
								/* 创建任务3 */
    xTaskCreate((TaskFunction_t )task3,
                (const char*    )"task3",
                (uint16_t       )TASK3_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK3_PRIO,
                (TaskHandle_t*  )&Task3Task_Handler);
    vTaskDelete(StartTask_Handler); /* 删除开始任务 */
    taskEXIT_CRITICAL();            /* 退出临界区 */
}

int temp=1;
int speed = 50;
void task1(void *pvParameters)
{
	  vTaskDelay(pdMS_TO_TICKS(5000));
		TIM_Step_Enable();
		control_flag = 1;  
//		StepMotor_SetSpeed(30);
    while (1)
    {
//		speed += temp;

//    if(speed >= 550) temp = -1;
//    if(speed <= 30)  temp = 1;
		
//		StepMotor_SetSpeed(60);
		EncoderSend();
		S_SetTargetSpeed(400);
		SpeedSend();
    vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void task2(void *pvParameters)
{   
//		TIM_Step_Enable();
//		control_flag = 1;   
		while (1)
    {
//			if(SpeedFlag)
//			{
//				SpeedFlag = 0;
//				SpeedSend();
//				vTaskDelay(pdMS_TO_TICKS(3));
//				TargetSpeed();
//				vTaskDelay(pdMS_TO_TICKS(3));
//			}
//			if(g_error_code)
//			{
//				ErrorSend();
//				g_error_code = ERR_OK;
//				vTaskDelay(pdMS_TO_TICKS(3));
//			}
  			vTaskDelay(pdMS_TO_TICKS(5));
    }
}


void MeasureCurrent(void)
{
		printf("current2:%f,%f\n",INA240_Current_A,INA240_Current_B);
		uint32_t arr = htim3.Init.Period + 1;   // TIM3 的周期（比如 25）
		uint32_t ccr1 = __HAL_TIM_GET_COMPARE(&htim5, TIM_CHANNEL_4); // PB0 = B+
		uint32_t ccr2 = __HAL_TIM_GET_COMPARE(&htim5, TIM_CHANNEL_1); // PB1 = B-
		uint32_t ccr3 = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_3); // PB0 = B+
		uint32_t ccr4 = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_4); // PB1 = B-
	  float dutyA_pos = (float)ccr1 / arr;
    float dutyA_neg = (float)ccr2 / arr;
    float dutyB_pos = (float)ccr3 / arr;
    float dutyB_neg = (float)ccr4 / arr;

    printf("A+ = %.2f%%   A- = %.2f%%   B+ = %.2f%%   B- = %.2f%%\r\n",
	      dutyA_pos * 100.0f,
        dutyA_neg * 100.0f,
        dutyB_pos * 100.0f,
        dutyB_neg * 100.0f);
}

void task3(void *pvParameters)
{  
		control_flag = 1;
	while (1)
    {

//			CurrentSend(0x30,INA240_Current_A);
//			CurrentSend(0x40,INA240_Current_B);
//			UART_RxHandler();
//			uint32_t cnt = __HAL_TIM_GET_COUNTER(&htim7);
//			StepMotor_PrintPinState();
//			MeasureCurrent();
			vTaskDelay(pdMS_TO_TICKS(50));
    }
}

extern int8_t step_index;
void StepMotor_PrintPinState(void)
{
    uint8_t a = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    uint8_t b = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
    uint8_t c = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
    uint8_t d = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);

    printf("a+=%d  a-=%d  b+=%d  b-=%d index=%d\r\n", a, b, c, d,step_index);
}

void EncoderSend(void)
{
		CaptureNumber = (OverflowCount*CNT_MAX) + __HAL_TIM_GET_COUNTER(&htimx_Encoder);
		const int64_t temp = CaptureNumber;
		ProtocolFrame_t frame;
		frame.head = PROTOCOL_FRAME_HEAD;//AA
    frame.dev_id = PROTOCOL_DEV_ID;//01
    frame.cmd = 0x20; //
		frame.data_len = 8;//08
		memcpy(frame.data, &temp, 8); 
		frame.tail = PROTOCOL_FRAME_TAIL;//55
		uint8_t txbuf[PROTOCOL_FRAME_MAX_LEN];
    uint8_t txlen = Protocol_Pack(&frame, txbuf);
		HAL_UART_Transmit(&g_uart1_handle, txbuf, txlen, 100);
}

void CurrentSend(uint8_t cmd,float current_value)
{
		float temp = current_value;
		ProtocolFrame_t frame;
		frame.head = PROTOCOL_FRAME_HEAD;//AA
    frame.dev_id = PROTOCOL_DEV_ID;//01
    frame.cmd = cmd;
		frame.data_len = 4;//04
		memcpy(frame.data, &temp, 4); 
		frame.tail = PROTOCOL_FRAME_TAIL;//55
		uint8_t txbuf[PROTOCOL_FRAME_MAX_LEN];
    uint8_t txlen = Protocol_Pack(&frame, txbuf);
		HAL_UART_Transmit(&g_uart1_handle, txbuf, txlen, 100);
}

void SpeedSend(void)
{
		uint8_t* tmp = buf_prepare;
    buf_prepare = buf_send;
    buf_send = tmp;

    PosChange = NewPos - LastPos;
		float Speed = PosChange*0.75;//rpm  
    ProtocolFrame_t frame;
    frame.head = PROTOCOL_FRAME_HEAD;
    frame.dev_id = PROTOCOL_DEV_ID;
    frame.cmd = 0x70;
    frame.data_len = 4;
    memcpy(frame.data, &Speed, 4);
    frame.tail = PROTOCOL_FRAME_TAIL;

    uint8_t txlen = Protocol_Pack(&frame, buf_prepare);
    HAL_UART_Transmit_DMA(&g_uart1_handle, buf_send, txlen);	
}

void TargetSpeed(void)
{
		uint8_t* tmp = buf_prepare;
    buf_prepare = buf_send;
    buf_send = tmp;

		float Speed = 1000.0/4.0*0.3;//rpm       1000/4*1.8/360*60
    ProtocolFrame_t frame;
    frame.head = PROTOCOL_FRAME_HEAD;
    frame.dev_id = PROTOCOL_DEV_ID;
    frame.cmd = 0x80;
    frame.data_len = 4;
    memcpy(frame.data, &Speed, 4);
    frame.tail = PROTOCOL_FRAME_TAIL;

    uint8_t txlen = Protocol_Pack(&frame, buf_prepare);
    HAL_UART_Transmit_DMA(&g_uart1_handle, buf_send, txlen);	
}


void ResistanceSend(uint8_t cmd,float Resistance_value)
{
		float temp = Resistance_value;
		ProtocolFrame_t frame;
		frame.head = PROTOCOL_FRAME_HEAD;//AA
    frame.dev_id = PROTOCOL_DEV_ID;//01
    frame.cmd = cmd;
		frame.data_len = 4;//04
		memcpy(frame.data, &temp, 4); 
		frame.tail = PROTOCOL_FRAME_TAIL;//55
		uint8_t txbuf[PROTOCOL_FRAME_MAX_LEN];
    uint8_t txlen = Protocol_Pack(&frame, txbuf);
		HAL_UART_Transmit(&g_uart1_handle, txbuf, txlen, 100);
}


void MeasureResistance(void)
{
		float sampleA = 0.0f;
		float sampleB = 0.0f;
	//取二十次均值
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_4, 3); 
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, 0);
		vTaskDelay(pdMS_TO_TICKS(50));
		for(int i=0;i<20;i++)
		{
			sampleA += INA240_Current_A;
			vTaskDelay(pdMS_TO_TICKS(50));
		}
		__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_4, 0); 
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, 0);
		sampleA /= 20.0f;
		float duty = 4.0f/25.0f;
		float Vbus = 24.0f;
		float Veq = Vbus * duty;
		ResistanceA = fabsf(Veq/sampleA);
		vTaskDelay(pdMS_TO_TICKS(50));
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 3);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
		vTaskDelay(pdMS_TO_TICKS(50));
		for(int i=0;i<20;i++)
		{
			sampleB += INA240_Current_B;
			vTaskDelay(pdMS_TO_TICKS(50));
		}
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
		sampleB /= 20.0f;
		ResistanceB = fabsf(Veq/sampleB);
}

void ErrorSend(void)
{
		ProtocolFrame_t frame;
		frame.head = PROTOCOL_FRAME_HEAD;//AA
    frame.dev_id = PROTOCOL_DEV_ID;//01
    frame.cmd = 0x05;
		frame.data_len = 1;//01
		frame.data[0] = g_error_code;
		frame.tail = PROTOCOL_FRAME_TAIL;//55
		uint8_t txbuf[PROTOCOL_FRAME_MAX_LEN];
    uint8_t txlen = Protocol_Pack(&frame, txbuf);
		HAL_UART_Transmit(&g_uart1_handle, txbuf, txlen, 100);
}

