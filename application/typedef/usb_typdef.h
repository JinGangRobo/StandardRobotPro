#ifndef USB_TYPEDEF_H
#define USB_TYPEDEF_H

#include "attribute_typedef.h"
#include "remote_control.h"
#include "struct_typedef.h"

#define DEBUG_PACKAGE_NUM 10

// usb通讯版本号
#define PACKET_VERSION                  ((uint8_t)0x31)
// debug 专用版本号
#define PACKET_DEBUG_VERSION            ((uint8_t)0x21)

#define DEBUG_DATA_SEND_ID              ((uint8_t)0x01)
#define IMU_DATA_SEND_ID                ((uint8_t)0x02)
#define PID_DEBUG_DATA_SEND_ID          ((uint8_t)0x03)
#define ROBOT_STATE_DATA_INFO_SEND_ID   ((uint8_t)0x3B)

#define PID_DEBUG_DATA_RECEIVE_ID       ((uint8_t)0x01)
#define VIRTUAL_RC_DATA_RECEIVE_ID      ((uint8_t)0x02)
#define ROBOT_CMD_DATA_RECEIVE_ID       ((uint8_t)0x3A)

/*-------------------- 下面定义 vofa 的命令接收参数 ID --------------------*/
// 电机pid参数
#define VOFA_ABS_KP_ID                              ((uint8_t)0x11)
#define VOFA_ABS_KI_ID                              ((uint8_t)0x12)
#define VOFA_ABS_KD_ID                              ((uint8_t)0x13)
#define VOFA_SPE_KP_ID                              ((uint8_t)0x14)
#define VOFA_SPE_KI_ID                              ((uint8_t)0x15)
#define VOFA_SPE_KD_ID                              ((uint8_t)0x16)
//调试电机id
#define VOFA_GIMBAL_PITCH_MOTOR_ID                  ((uint8_t)0x17)
#define VOFA_GIMBAL_YAW_MOTOR_ID                    ((uint8_t)0x18)
#define VOFA_SHOOT_TRIGGER_MOTOR_ID                 ((uint8_t)0x19)
#define VOFA_SHOOT_L_MOTOR_ID                       ((uint8_t)0x1A)
#define VOFA_SHOOT_R_MOTOR_ID                       ((uint8_t)0x1B)


typedef struct
{
    uint8_t sof; // 数据帧起始字节，为当前协议版本号
    uint8_t len;  // 数据段长度
    uint8_t id;   // 数据段id
    uint8_t crc;  // 数据帧头的 CRC8 校验
} __packed__ FrameHeader_t;
/*-------------------- Send --------------------*/

// 串口调试数据包
typedef struct
{
    float data[DEBUG_PACKAGE_NUM];          // 小端浮点数组
    unsigned char tail[4];          // 帧尾固定值
} __packed__ SendDataDebug_s;

// IMU 数据包
typedef struct
{
    FrameHeader_t frame_header; // 数据段id = 0x02
    struct
    {
        float yaw;   // rad
        float pitch; // rad
        float roll;  // rad

        float yaw_vel;   // rad/s
        float pitch_vel; // rad/s
        float roll_vel;  // rad/s

        float x_accel; // m/s^2
        float y_accel; // m/s^2
        float z_accel; // m/s^2
    } __packed__ data;
    uint16_t crc;
} __packed__ SendDataImu_s;

// PID调参数据包
typedef struct
{
    FrameHeader_t frame_header; // 数据段id = 0x05
    struct
    {
        float fdb;
        float ref;
        float pid_out;
    } __packed__ data;
    uint16_t crc;
} __packed__ SendDataPidDebug_s;

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

// PID调参数据包
typedef struct
{
    uint8_t motor_id;

    uint8_t abs_kp;
    uint8_t abs_ki;
    uint8_t abs_kd;

    uint8_t spe_kp;
    uint8_t spe_ki;
    uint8_t spe_kd;
} __packed__ ReceiveDataPidDebug_s;

// 虚拟遥控器数据包
typedef struct
{
    FrameHeader_t frame_header; // 数据段id = 0x03
    RC_ctrl_t data;
    uint16_t crc;
} __packed__ ReceiveDataVirtualRc_s;
#endif // USB_TYPEDEF_H
