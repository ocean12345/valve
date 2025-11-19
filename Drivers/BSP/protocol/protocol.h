#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

// 协议帧结构体
#define PROTOCOL_FRAME_HEAD   0xAA
#define PROTOCOL_FRAME_TAIL   0x55
#define PROTOCOL_DEV_ID       0x01  // 可根据实际修改

// 命令码定义
#define STATE_SWITCH      0x10
#define POSITION_CONTROL  0x11
#define PRESSURE_CONTROL  0x12
#define ERROR_REPORT      0x13

// 状态切换命令内容
#define CODE_INIT 0x00
#define CODE_POSITION 0x01
#define CODE_PRESSURE 0x02
#define CODE_ERROR 0x03

#define PROTOCOL_FRAME_MAX_LEN  32

typedef struct {
    uint8_t head;
    uint8_t dev_id;
    uint8_t cmd;
    uint8_t data[16]; // 命令内容，长度可调整
    uint8_t data_len;
    uint8_t checksum;
    uint8_t tail;
} ProtocolFrame_t;

// 协议解析与处理
bool Protocol_Parse(const uint8_t *buf, uint8_t len, ProtocolFrame_t *out);
// 协议帧组包
uint8_t Protocol_Pack(ProtocolFrame_t *frame, uint8_t *out_buf);
void HandleCommand(ProtocolFrame_t *frame);
void PrintFrameData(const ProtocolFrame_t *f);


#endif // PROTOCOL_H
