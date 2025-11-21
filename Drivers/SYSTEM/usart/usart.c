#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/protocol/protocol.h"

/* 如果使用os,则包括下面的头文件即可 */
//#if SYS_SUPPORT_OS
//#include "os.h"                               /* os 使用 */
//#endif

/******************************************************************************************/
/* 加入以下代码, 支持printf函数, 而不需要选择use MicroLIB */

#if 1
#if (__ARMCC_VERSION >= 6010050)                    /* 使用AC6编译器时 */
__asm(".global __use_no_semihosting\n\t");          /* 声明不使用半主机模式 */
__asm(".global __ARM_use_no_argv \n\t");            /* AC6下需要声明main函数为无参数格式，否则部分例程可能出现半主机模式 */

#else
/* 使用AC5编译器时, 要在这里定义__FILE 和 不使用半主机模式 */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

/* 不使用半主机模式，至少需要重定义_ttywrch\_sys_exit\_sys_command_string函数,以同时兼容AC6和AC5模式 */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* 定义_sys_exit()以避免使用半主机模式 */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

/* FILE 在 stdio.h里面定义. */
FILE __stdout;

/* 重定义fputc函数, printf函数最终会通过调用fputc输出字符串到串口 */
int fputc(int ch, FILE *f)
{
    while ((UART4->SR & 0X40) == 0);               /* 等待上一个字符发送完成 */

    UART4->DR = (uint8_t)ch;                       /* 将要发送的字符 ch 写入到DR寄存器 */
    return ch;
}
#endif
/***********************************************END*******************************************/
    
#if USART_EN_RX                                     /* 如果使能了接收 */
uint8_t uart_rx_byte;               // 单字节接收缓存
uint8_t uart_ring_buf[UART_RX_BUF_SIZE];
volatile uint16_t uart_write_idx = 0;
volatile uint16_t uart_read_idx = 0;

DMA_HandleTypeDef hdma_uart4_tx;
UART_HandleTypeDef g_uart1_handle;                  /* UART句柄 */
ProtocolFrame_t rxframe;

void usart_init(uint32_t baudrate)
{
    g_uart1_handle.Instance = USART_UX;                         /* USART1 */
    g_uart1_handle.Init.BaudRate = baudrate;                    /* 波特率 */
    g_uart1_handle.Init.WordLength = UART_WORDLENGTH_8B;        /* 字长为8位数据格式 */
    g_uart1_handle.Init.StopBits = UART_STOPBITS_1;             /* 一个停止位 */
    g_uart1_handle.Init.Parity = UART_PARITY_NONE;              /* 无奇偶校验位 */
    g_uart1_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;        /* 无硬件流控 */
    g_uart1_handle.Init.Mode = UART_MODE_TX_RX;                 /* 收发模式 */
    HAL_UART_Init(&g_uart1_handle);                             /* HAL_UART_Init()会使能UART1 */
    
    /* 该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量 */
    HAL_UART_Receive_IT(&g_uart1_handle, &uart_rx_byte, RXBUFFERSIZE);
}

void DmaInit(void)
{
		__HAL_RCC_DMA1_CLK_ENABLE();

    // 2. 配置 UART4_TX 对应的 DMA1 Stream4 Channel4
    hdma_uart4_tx.Instance = DMA1_Stream4;
    hdma_uart4_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_uart4_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_uart4_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart4_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart4_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart4_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart4_tx.Init.Mode = DMA_NORMAL;
    hdma_uart4_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_uart4_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    HAL_DMA_Init(&hdma_uart4_tx);

    // 3. 开启 DMA 中断
    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_init_struct;
    if(huart->Instance == USART_UX)                             /* 如果是串口1，进行串口1 MSP初始化 */
    {
        USART_UX_CLK_ENABLE();                                  /* USART1 时钟使能 */
        USART_TX_GPIO_CLK_ENABLE();                             /* 发送引脚时钟使能 */
        USART_RX_GPIO_CLK_ENABLE();                             /* 接收引脚时钟使能 */

        gpio_init_struct.Pin = USART_TX_GPIO_PIN;               /* TX引脚 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                /* 复用推挽输出 */
        gpio_init_struct.Pull = GPIO_PULLUP;                    /* 上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;          /* 高速 */
        gpio_init_struct.Alternate = USART_TX_GPIO_AF;          /* 复用为USART1 */
        HAL_GPIO_Init(USART_TX_GPIO_PORT, &gpio_init_struct);   /* 初始化发送引脚 */

        gpio_init_struct.Pin = USART_RX_GPIO_PIN;               /* RX引脚 */
        gpio_init_struct.Alternate = USART_RX_GPIO_AF;          /* 复用为USART1 */
        HAL_GPIO_Init(USART_RX_GPIO_PORT, &gpio_init_struct);   /* 初始化接收引脚 */

				// 把 DMA 绑定到 UART4
        __HAL_LINKDMA(huart, hdmatx, hdma_uart4_tx);
#if USART_EN_RX
        HAL_NVIC_EnableIRQ(USART_UX_IRQn);                      /* 使能USART1中断通道 */
        HAL_NVIC_SetPriority(USART_UX_IRQn, 3, 3);              /* 抢占优先级3，子优先级3 */
#endif
    }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == UART4)             /* 如果是串口4 */
    {   
        uart_ring_buf[uart_write_idx++] = uart_rx_byte;
        if (uart_write_idx >= UART_RX_BUF_SIZE)
            uart_write_idx = 0;

        // 继续接收下一个字节
        HAL_UART_Receive_IT(&g_uart1_handle, &uart_rx_byte, 1); 
    }
}

// 处理缓冲区数据，查找完整协议帧
void UART_RxHandler(void)
{
    while (uart_read_idx != uart_write_idx)
    {
        static uint8_t frame_buf[32];
        static uint8_t idx = 0;

        uint8_t byte = uart_ring_buf[uart_read_idx++];
        if (uart_read_idx >= UART_RX_BUF_SIZE)
            uart_read_idx = 0;
        // 帧头同步
        if (idx == 0)
        {
            if (byte == PROTOCOL_FRAME_HEAD)
                frame_buf[idx++] = byte;
            continue;
        }

        frame_buf[idx++] = byte;
        // 最小长度 6
        if (idx >= 6)
        {
            uint8_t data_len = frame_buf[3];
            uint8_t expected_len = 4 + data_len + 2;

            if (idx == expected_len)
            {
                // 完整帧解析
								if (Protocol_Parse(frame_buf, idx, &rxframe))
								{
										HandleCommand(&rxframe);
								}
                idx = 0;
            }
            else if (idx > expected_len)
            {
                // 异常丢弃
                idx = 0;
            }
        }
    }
}


void USART_UX_IRQHandler(void)
{ 
//#if SYS_SUPPORT_OS                              /* 使用OS */
//    OSIntEnter();    
//#endif

    HAL_UART_IRQHandler(&g_uart1_handle);       /* 调用HAL库中断处理公用函数 */

//#if SYS_SUPPORT_OS                              /* 使用OS */
//    OSIntExit();
//#endif
}

#endif


 

 




