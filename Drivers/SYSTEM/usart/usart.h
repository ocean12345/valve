#ifndef _USART_H
#define _USART_H

#include "stdio.h"
#include "./SYSTEM/sys/sys.h"

/*******************************************************************************************************/
/* 引脚 和 串口 定义 
 * 默认是针对USART1的.
 * 注意: 通过修改这12个宏定义,可以支持USART1~UART7任意一个串口.
 */

#define USART_TX_GPIO_PORT              GPIOC
#define USART_TX_GPIO_PIN               GPIO_PIN_10
#define USART_TX_GPIO_AF                GPIO_AF8_UART4
#define USART_TX_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)   /* 发送引脚时钟使能 */

#define USART_RX_GPIO_PORT              GPIOC
#define USART_RX_GPIO_PIN               GPIO_PIN_11
#define USART_RX_GPIO_AF                GPIO_AF8_UART4
#define USART_RX_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)   /* 接收引脚时钟使能 */

#define USART_UX                        UART4
#define USART_UX_IRQn                   UART4_IRQn
#define USART_UX_IRQHandler             UART4_IRQHandler
#define USART_UX_CLK_ENABLE()           do{ __HAL_RCC_UART4_CLK_ENABLE(); }while(0)  /* USART1 时钟使能 */

/*******************************************************************************************************/

#define UART_RX_BUF_SIZE 64
#define USART_EN_RX     1                       /* 使能（1）/禁止（0）串口1接收 */
#define RXBUFFERSIZE    1                       /* 缓存大小 */

extern UART_HandleTypeDef g_uart1_handle;       /* UART句柄 */

void usart_init(uint32_t baudrate);             /* 串口初始化函数 */
void DmaInit(void);
void UART_InitReceive(void);
void UART_RxHandler(void);

#endif







