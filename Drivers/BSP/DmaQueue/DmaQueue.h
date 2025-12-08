#ifndef DMAQUEUE_H
#define DMAQUEUE_H
#include "./SYSTEM/sys/sys.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "string.h"

typedef struct {
    uint8_t data[64];  // 最大帧长可调整
    uint16_t len;
} TxMsg_t;

extern QueueHandle_t UartTxQueue;
extern volatile uint8_t uart_dma_busy;


void UART_DMATask(void *argument);
void UART_SendFrame(uint8_t *buf, uint16_t len);

#endif // DMAQUEUE_H

