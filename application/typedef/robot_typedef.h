#ifndef ROBOT_TYPEDEF_H
#define ROBOT_TYPEDEF_H

// clang-format off
// 可用底盘硬件类型
#define CHASSIS_NONE            0  // 无底盘
#define CHASSIS_MECANUM_WHEEL   1  // 麦克纳姆轮底盘
#define CHASSIS_OMNI_WHEEL      2  // 全向轮底盘

// 可用云台硬件类型
#define GIMBAL_NONE                0  // 无云台
#define GIMBAL_YAW_PITCH_DIRECT    1  // yaw-pitch电机直连云台

// 可用的发射机构硬件类型
#define SHOOT_NONE               0  // 无发射机构
#define SHOOT_FRIC_TRIGGER       1  // 摩擦轮+拨弹盘发射机构

// 可用机械臂硬件类型
#define MECHANICAL_ARM_NONE              0  // 无机械臂
#define MECHANICAL_ARM_ENGINEER_ARM      1  // 工程机械臂

// 可用自定义控制器硬件类型
#define CUSTOM_CONTROLLER_NONE         0  // 无自定义控制器
#define CUSTOM_CONTROLLER_ENGINEER     1  // 工程用的自定义控制器

// 定义控制模式
#define SINGLE_CONTROL 0    //单板控制
#define DOUBLE_CONTROL 1    //双板控制

// 可用调参模式
#define TUNING_NONE     0
#define TUNING_CHASSIS  1
#define TUNING_GIMBAL   2
#define TUNING_SHOOT    3

// 校准数据来源
#define CALI_FROM_FLASH 1
#define CALI_FROM_USB   2
#define CALI_FROM_CODE  3

// 自定义控制器类型
#define CC_RECEIVER 0  // 接收器
#define CC_SENDER   1  // 发送器

// 遥控器类型
#define RC_DT7      0  // DT7遥控器

// C板id
#define C_BOARD_DEFAULT                  1  // C板默认id
#define C_BOARD_OMMI_INFANTRY_CHASSIS    2  // 全向轮步兵底盘C板
#define C_BOARD_OMMI_INFANTRY_GIMBAL     3  // 全向轮步兵云台C板
#define C_BOARD_OMNI_SENTINEIL_CHASSIS   4  // 全向轮哨兵底盘C板
#define C_BOARD_OMNI_SENTINEIL_GIMBAL    5  // 全向轮哨兵云台C板
#define C_BOARD_MECANUM_HERO_CHASSIS     6  // 麦克纳姆轮英雄底盘C板
#define C_BOARD_MECANUM_HERO_GIMBAL      7  // 麦克纳姆轮英雄云台C板
#define C_BOARD_ENGINEER_CHASSIS         8  // (单板)工程C板

// 可用电机类型
typedef enum __MotorType {
    DJI_M2006 = 0,
    DJI_M3508,
    DJI_M6020,
    CYBERGEAR_MOTOR,
    DM_8009,
    DM_4310,
    DM_4340,
    MF_9025,
} MotorType_e;
// clang-format on

#endif /* ROBOT_TYPEDEF_H */
/*------------------------------ End of File ------------------------------*/
