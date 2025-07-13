#ifndef __CUSTOM_TYPEDEF_H
#define __CUSTOM_TYPEDEF_H

#include "stdbool.h"

// 数据名称宏
#define IMU_NAME "imu_data"
#define CHASSIS_FDB_SPEED_NAME "chassis_fdb_speed"
#define CHASSIS_REF_NAME "chassis_ref"
#define ROBOT_CMD_DATA_NAME "ROBOT_CMD_DATA"
#define USB_OFFLINE_NAME "usb_offline"
#define CALI_BUZZER_STATE_NAME "CaliBuzzerState"
#define PID_TO_VOFA_NAME "PID_TO_VOFA"
#define PID_GET_VOFA_NAME "PID_GET_VOFA"

typedef enum
{
    CALI_BUZZER_OFF = 0,
    CALI_BUZZER_BEGIN,
    CALI_BUZZER_MIDDLE_TIME,
    CALI_BUZZER_GIMBAL,
    CALI_BUZZER_IMU,
    CALI_BUZZER_CHASSIS,
} CaliBuzzerState_e;

typedef struct __Imu
{
    float yaw, pitch, roll;              // rad
    float yaw_vel, pitch_vel, roll_vel;  // rad/s
    float x_accel, y_accel, z_accel;     // m/s^2
} Imu_t;

typedef struct  // 底盘速度向量结构体
{
    float vx;  // (m/s) x方向速度
    float vy;  // (m/s) y方向速度
    float wz;  // (rad/s) 旋转速度
} ChassisSpeedVector_t;

/**
 * @brief  底盘期望
 */
typedef struct 
{
    float vx;
    float vy;
    float wz;
    uint8_t chassis_mode;
} ChassisReference_t;

typedef struct
{
    ChassisSpeedVector_t speed_vector;
    struct
    {
        float pitch;
        float yaw;
    } gimbal;

    struct
    {
        bool fire;
    } shoot;

} RobotCmdData_t;

// 需要调节的pid输出
typedef struct
{
    float angle_set;
    float angle_fdb;

    float angle_out;
    float angle_Pout;
    float angle_Iout;
    float angle_Dout;

    float speed_set;
    float speed_fdb;

    float speed_out;
    float speed_Pout;
    float speed_Iout;
    float speed_Dout;
} PidToVofa_t;

// pid参数设定值
typedef struct
{
    float angle_kp;                   // 比例增益
    float angle_ki;                   // 积分增益  
    float angle_kd;                   // 微分增益
    float angle_max_out;              // 最大输出限制
    float angle_max_iout;             // 最大积分输出限制
    
    float speed_kp;                   // 比例增益
    float speed_ki;                   // 积分增益
    float speed_kd;                   // 微分增益
    float speed_max_out;              // 最大输出限制
    float speed_max_iout;             // 最大积分输出限制
    
} PidGetVofa_t;

#endif  // __CUSTOM_TYPEDEF_H
