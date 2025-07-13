#ifndef USB_TYPEDEF_H
#define USB_TYPEDEF_H

#include "attribute_typedef.h"
#include "remote_control.h"
#include "struct_typedef.h"

#define DEBUG_PACKAGE_NUM 10

// usb通讯版本号
#define PACKET_VERSION                  ((uint8_t)0x31)

#define DEBUG_DATA_SEND_ID              ((uint8_t)0x01)
#define ROBOT_STATE_DATA_INFO_SEND_ID   ((uint8_t)0x3B)

#define ROBOT_CMD_DATA_RECEIVE_ID       ((uint8_t)0x3A)

typedef struct
{
    uint8_t sof; // 数据帧起始字节，为当前协议版本号
    uint8_t len;  // 数据段长度
    uint8_t id;   // 数据段id
    uint8_t crc;  // 数据帧头的 CRC8 校验
} __packed__ FrameHeader_t;

/*-------------------- Send --------------------*/

// PID调节数据发送结构体(vofa justfloat协议)
typedef struct
{
    float data[DEBUG_PACKAGE_NUM];  // 浮点数据数组
    unsigned char tail[4];          // 帧尾固定值{0x00, 0x00, 0x80, 0x7f}
} __packed__ SendDataPidTuning_s;

// 机器人信息数据包
typedef struct
{
    FrameHeader_t frame_header; // 数据段id = 0x03
    struct
    {
        float roll;
        float pitch;
        float yaw;

        uint16_t encoder_up;
        uint16_t encoder_down;

        float super_cap_voltage;
        bool is_super_cap_work;
    } __packed__ data;
    uint16_t crc;
} __packed__ SendDataRobotStateInfo_s;

/*-------------------- Receive --------------------*/
typedef struct RobotCmdData
{
    FrameHeader_t frame_header; // 数据段id = 0x01
    struct
    {
        float pitch;
        float yaw;
        bool fire;

        float vx;
        float vy;
    } __packed__ data;
    uint16_t checksum;
} __packed__ ReceiveDataRobotCmd_s;

// PID参数设置接收结构体
typedef struct
{
    FrameHeader_t frame_header;     // 数据段id = 0x05
    struct
    {
        uint8_t pid_type;           // PID类型(PidTuningType_e)
        uint8_t motor_id;           // 电机ID(当pid_type为电机类型时使用)
        uint8_t loop_type;          // 环路类型(PidLoopType_e，双环时使用)
        
        float kp;                   // 比例增益
        float ki;                   // 积分增益  
        float kd;                   // 微分增益
        float max_out;              // 最大输出限制
        float max_iout;             // 最大积分输出限制
        
        float target;               // 目标值设定
        uint8_t enable;             // PID使能标志
    } __packed__ data;
    uint16_t crc;
} __packed__ ReceiveDataPidParam_s;

#endif // USB_TYPEDEF_H
