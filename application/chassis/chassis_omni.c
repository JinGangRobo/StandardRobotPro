/**
  ****************************(C) COPYRIGHT 2024 Polarbear****************************
  * @file       chassis_omni.c/h
  * @brief      全向轮底盘控制器。
  * @note       包括初始化，目标量更新、状态量更新、控制量计算与直接控制量的发送
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0   2025.03.03       Harry_Wong        1.重新构建全向轮底盘，完成单底盘控制
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2024 Polarbear****************************
*/

#include "robot_param.h"
#if (CHASSIS_TYPE == CHASSIS_OMNI_WHEEL)
#include "chassis_omni.h"
#include "CAN_receive.h"
#include "chassis.h"
#include "usb_task.h"
#include "motor.h"
#include "detect_task.h"
#include "gimbal.h"
#include "usb_debug.h"

#include <math.h>
#include <string.h>

Chassis_s chassis;
float date;
PID_t chassis_pid;

/*-------------------- Init --------------------*/

/**
 * @brief          初始化
 * @param[in]      none
 * @retval         none
 */
void ChassisInit(void)
{
    // 获取遥控器指针
    chassis.rc = get_remote_control_point();

    // step2 PID数据清零，设置PID参数
    const static fp32 wheel_vel[3] = {KP_OMNI_VEL, KI_OMNI_VEL, KD_OMNI_VEL};
    for (int i = 0; i < 4; ++i)
    {
        PID_init(&chassis_pid.wheel_velocity[i], PID_POSITION, wheel_vel, MAX_OUT_OMNI_VEL, MAX_IOUT_OMNI_VEL);
    }

    const static fp32 gimbal_follow[3] = {KP_CHASSIS_FOLLOW_GIMBAL, KI_CHASSIS_FOLLOW_GIMBAL, KD_CHASSIS_FOLLOW_GIMBAL};
    PID_init(&chassis_pid.follow, PID_POSITION, gimbal_follow, MAX_OUT_CHASSIS_FOLLOW_GIMBAL, MAX_IOUT_CHASSIS_FOLLOW_GIMBAL);

    // step3 初始化电机
    MotorInit(&chassis.wheel[0], WHEEL_1_ID, WHEEL_1_CAN, WHEEL_1_MOTOR_TYPE, WHEEL_1_DIRECTION, WHEEL_1_RATIO, WHEEL_1_MODE);
    MotorInit(&chassis.wheel[1], WHEEL_2_ID, WHEEL_2_CAN, WHEEL_2_MOTOR_TYPE, WHEEL_2_DIRECTION, WHEEL_2_RATIO, WHEEL_2_MODE);
    MotorInit(&chassis.wheel[2], WHEEL_3_ID, WHEEL_3_CAN, WHEEL_3_MOTOR_TYPE, WHEEL_3_DIRECTION, WHEEL_3_RATIO, WHEEL_3_MODE);
    MotorInit(&chassis.wheel[3], WHEEL_4_ID, WHEEL_4_CAN, WHEEL_4_MOTOR_TYPE, WHEEL_4_DIRECTION, WHEEL_4_RATIO, WHEEL_4_MODE);

    // step4 初始模式设置
    chassis.mode = ROBO_ZERO_FORCE;
}

/*-------------------- Observe --------------------*/

/**
 * @brief          更新状态量
 * @param[in]      none
 * @retval         none
 */
void ChassisObserver(void)
{

    for (int i = 0; i < 4; ++i)
    {
        GetMotorMeasure(&chassis.wheel[i]);
    }

    for (int i = 0; i < 4; ++i)
    {
        chassis.feedback[i] = chassis.wheel[i].fdb.vel;
    }

    chassis.yaw_delta = GetGimbalDeltaYawMid();
}

/*-------------------- Reference --------------------*/

/**
 * @brief         . 更新目标量
 * @param[in]      none
 * @retval         none
 */
void ChassisReference(void)
{

#if (CONTROL_TYPE == SINGLE_CONTROL)
    chassis.reference.vx = GetDt7RcCh(0) * RC_TO_VECTOR_SCALE;
    chassis.reference.vy = GetDt7RcCh(1) * RC_TO_VECTOR_SCALE;
    chassis.reference.chassis_mode = GetDt7RcSw(0);

#elif (CONTROL_TYPE == DOUBLE_CONTROL)
    GetBoardInfo(&chassis.reference);
#endif

    // 在不同行为模式下，将云台坐标系下的值映射到底盘坐标系，供底盘解算，并设置绕z轴方向的速度值
    float sin_yaw = sin(chassis.yaw_delta);
    float cos_yaw = cos(chassis.yaw_delta);
    date=chassis.reference.vx;
    chassis.reference.vx = chassis.reference.vx * cos_yaw - chassis.reference.vy * sin_yaw;
    chassis.reference.vy = date * sin_yaw + chassis.reference.vy * cos_yaw;

    if (chassis.reference.chassis_mode == ROBO_CHASSIS_FOLLOW_GIMBAL_YAW)
    {
        chassis.reference.wz = 0;//PID_calc(&chassis_pid.follow, chassis.yaw_delta, 0);
    }

    else if (chassis.reference.chassis_mode == ROBO_SPIN)
    {
        chassis.reference.wz = 5;
    }
}

/*-------------------- Console --------------------*/

/**
 * @brief          计算控制量
 * @param[in]      none
 * @retval         none
 */
void ChassisConsole(void)
{
    chassis.set[0] = (sqrt(2) * (chassis.reference.vx - chassis.reference.vy) - WHEEL_CENTER_DISTANCE * chassis.reference.wz) / WHEEL_RADIUS * chassis.wheel[0].reduction_ratio;
    chassis.set[1] = (sqrt(2) * (chassis.reference.vx + chassis.reference.vy) - WHEEL_CENTER_DISTANCE * chassis.reference.wz) / WHEEL_RADIUS * chassis.wheel[1].reduction_ratio;
    chassis.set[2] = (sqrt(2) * (-chassis.reference.vx + chassis.reference.vy) - WHEEL_CENTER_DISTANCE * chassis.reference.wz) / WHEEL_RADIUS * chassis.wheel[2].reduction_ratio;
    chassis.set[3] = (sqrt(2) * (-chassis.reference.vx - chassis.reference.vy) - WHEEL_CENTER_DISTANCE * chassis.reference.wz) / WHEEL_RADIUS * chassis.wheel[3].reduction_ratio;

    for (int i = 0; i < 4; ++i)
    {
        chassis.wheel[i].set.curr = PID_calc(&chassis_pid.wheel_velocity[i], chassis.feedback[i], chassis.set[i]);
        // chassis.wheel[i].set.curr = PID_calc(&chassis_pid.wheel_velocity[i], chassis.feedback[i], 50);
    }
}

/*-------------------- Cmd --------------------*/

/**
 * @brief          发送控制量
 * @param[in]      none
 * @retval         none
 */

void ChassisSendCmd(void)
{
    CanCmdDjiMotor(CHASSIS_CAN, CHASSIS_STDID, chassis.wheel[3].set.curr, chassis.wheel[0].set.curr, chassis.wheel[1].set.curr, chassis.wheel[2].set.curr);
}

#endif
