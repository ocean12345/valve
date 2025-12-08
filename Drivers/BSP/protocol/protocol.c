#include "./BSP/protocol/protocol.h"
#include "./SYSTEM/usart/usart.h"
#include "freertos_demo.h"
#include "./BSP/TIMER/btim.h"
#include "./BSP/motor/bsp_motor.h"

extern float ResistanceA;
extern float ResistanceB;
extern uint8_t control_flag;
// 校验计算：从dev_id到data最后一字节，异或
static uint8_t Protocol_CalcChecksum(const ProtocolFrame_t *frame)
{
    uint8_t sum = 0;
    sum ^= frame->dev_id;
    sum ^= frame->cmd;
    sum ^= frame->data_len;
    for (uint8_t i = 0; i < frame->data_len; i++) {
        sum ^= frame->data[i];
    }
    return sum;
}

// 协议帧组包
uint8_t Protocol_Pack(ProtocolFrame_t *frame, uint8_t *out_buf)
{
    if (!frame || !out_buf) return 0;
    uint8_t idx = 0;
    out_buf[idx++] = frame->head;
    out_buf[idx++] = frame->dev_id;
    out_buf[idx++] = frame->cmd;
    out_buf[idx++] = frame->data_len;
    for (uint8_t i = 0; i < frame->data_len; i++) {
        out_buf[idx++] = frame->data[i];
    }
    frame->checksum = Protocol_CalcChecksum(frame);
    out_buf[idx++] = frame->checksum;
    out_buf[idx++] = frame->tail;
    return idx;
}

// 协议解析与处理
bool Protocol_Parse(const uint8_t *buf, uint8_t len, ProtocolFrame_t *out)
{
    if (!buf || len < 6) return false; // 最小帧长: 头+ID+CMD+LEN+CHK+尾+无数据
    if (buf[0] != PROTOCOL_FRAME_HEAD) return false;
    if (buf[len-1] != PROTOCOL_FRAME_TAIL) return false;
    ProtocolFrame_t frame;
    frame.head = buf[0];
    frame.dev_id = buf[1];
    frame.cmd = buf[2];
    frame.data_len = buf[3];
    if (frame.data_len > 8 || (6 + frame.data_len) != len) return false;
    for (uint8_t i = 0; i < frame.data_len; i++) {
        frame.data[i] = buf[4 + i];
    }
    frame.checksum = buf[4 + frame.data_len];
    frame.tail = buf[5 + frame.data_len];
    if (frame.checksum != Protocol_CalcChecksum(&frame)) return false;
		
		if(out)
			*out = frame;
		return true;
}

void HandleCommand(ProtocolFrame_t *frame)
{
		switch(frame->cmd)
		{
			case 0x10:
				TIM_Step_Enable();
				control_flag = 1;
				break;
			case 0x20:
				TIM_Step_Disable();
				control_flag = 0;
				CurrentSend(0x30,0);
				CurrentSend(0x40,0);
				break;
			case 0x30:
				TIM_Step_Disable();
				control_flag = 0;
				MeasureResistance();
				ResistanceSend(0x50,ResistanceA);
				ResistanceSend(0x60,ResistanceB);
				break;
			case 0x40:
				SetDir(frame->data[0]);
				break;
		}
}


