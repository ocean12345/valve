#include "./BSP/DmaQueue/DmaQueue.h"

QueueHandle_t UartTxQueue;
volatile uint8_t uart_dma_busy = 0;
static uint8_t dma_tx_buffer[32];

void UART_SendFrame(uint8_t *buf, uint16_t len)
{
    TxMsg_t msg;
    memcpy(msg.data, buf, len);
    msg.len = len;

    xQueueSend(UartTxQueue, &msg, 3); // 立即加入队列（不阻塞）
}

void UART_DMATask(void *argument)
{
    TxMsg_t msg;

    for (;;)
    {
        if (xQueueReceive(UartTxQueue, &msg, portMAX_DELAY) == pdPASS)
        {
            while (uart_dma_busy)
                vTaskDelay(1);

            uart_dma_busy = 1;

            memcpy(dma_tx_buffer, msg.data, msg.len);

            HAL_UART_Transmit_DMA(&g_uart1_handle, dma_tx_buffer, msg.len);
        }
    }
}
