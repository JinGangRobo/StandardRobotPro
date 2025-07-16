/**
  ****************************(C) COPYRIGHT 2024 Polarbear****************************
  * @file       shoot_fric.c/h
  * @brief      使用摩擦轮的发射机构控制器。
  * @note       包括初始化，目标量更新、状态量更新、控制量计算与直接控制量的发送
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Apr-1-2024      Penguin         1. done
  *  V1.0.1     Apr-16-2024     Penguin         1. 完成基本框架
  *  V1.1.0     2025-1-15       CJH             1. 实现基本功能
  *  V2.0.0     2025-3-3        CJH             1. 兼容了达妙4310拨弹盘和大疆2006拨弹盘
  *                                             2. 完善了单发功能，上位机火控功能
  *                                             3. 增加了热量限制
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2024 Polarbear****************************
*/

#include "shoot_fric_trigger.h"
#include "data_exchange.h"

#if (SHOOT_TYPE == SHOOT_FRIC_TRIGGER)

static Shoot_s SHOOT = {
    .mode = LOAD_STOP,
    .state = FRIC_NOT_READY,
    .fric_flag = 0,
    .move_flag = 0,
    .ecd_count = 0,
    .shoot_flag = 0,
    .heat = 0,
    .heat_limit = 0,
};

uint8_t fric_ui;
fp32 delta;
int date;
fp32 all_error;               // 总误差
int all_Transmission_ratio_z; // 电机到拨弹盘的总传动比的整数
int COUNT;                    // 拨弹盘转半圈所需的电机圈数的整数
int COUNT_error;              // 误差

// ---------测试---------
#if(aaa1==1)
static const PidGetVofa_t *PID_GET_VOFA_DATA; // PID调节数据指针
static PidToVofa_t PID_TO_VOFA_DATA = {0};
#endif
//-----------------------

/*-------------------- Publish --------------------*/

/**
 * @brief          发布射击相关数据
 * @param[in]      none
 * @retval         none
 */
#if(aaa1==1)
void ShootPublish(void)
{
    // 发布PID调试数据到VOFA
    Publish(&PID_TO_VOFA_DATA, PID_TO_VOFA_NAME);
}
#endif
/*-------------------- Init --------------------*/

/**
 * @brief          初始化
 * @param[in]      none
 * @retval         none
 */
void ShootInit(void)
{
#if(aaa1==1)
  PID_GET_VOFA_DATA = Subscribe(PID_GET_VOFA_NAME);
#endif
  // 获取遥控器指针
  SHOOT.rc = get_remote_control_point();

  // 摩擦轮相关
  MotorInit(&SHOOT.fric_motor[0], FRIC_MOTOR_R_ID, FRIC_MOTOR_R_CAN, FRIC_MOTOR_TYPE, 1, 1.0f, 0);  // 初始化R摩擦轮电机结构体
  MotorInit(&SHOOT.fric_motor[1], FRIC_MOTOR_L_ID, FRIC_MOTOR_L_CAN, FRIC_MOTOR_TYPE, -1, 1.0f, 0); // 初始化L摩擦轮电机结构体
  MotorInit(&SHOOT.fric_motor[2], FRIC_MOTOR_U_ID, FRIC_MOTOR_U_CAN, FRIC_MOTOR_TYPE, 1, 1.0f, 0);  // 初始化U摩擦轮电机结构体

  const fp32 pid_fric[3] = {FRIC_SPEED_PID_KP, FIRC_SPEED_PID_KI, FRIC_SPEED_PID_KD}; // 摩擦轮速度环

  PID_init(&SHOOT.fric_pid[0], PID_POSITION, pid_fric, FRIC_PID_MAX_OUT, FRIC_PID_MAX_IOUT);
  PID_init(&SHOOT.fric_pid[1], PID_POSITION, pid_fric, FRIC_PID_MAX_OUT, FRIC_PID_MAX_IOUT);
  PID_init(&SHOOT.fric_pid[2], PID_POSITION, pid_fric, FRIC_PID_MAX_OUT, FRIC_PID_MAX_IOUT); // 摩擦轮初始化pid

  // 拨弹盘相关
  MotorInit(&SHOOT.trigger_motor, TRIGGER_MOTOR_ID, TRIGGER_MOTOR_CAN, TRIGGER_MOTOR_TYPE, 1, 1.0f, 0); // 初始化拨弹盘电机结构体
  if (TRIGGER_MOTOR_TYPE == DJI_M2006)
  {
    const fp32 pid_angel_trigger[3] = {TRIGGER_ANGEL_PID_KP, TRIGGER_ANGEL_PID_KI, TRIGGER_ANGEL_PID_KD}; // 拨弹盘角度环
    const fp32 pid_speed_trigger[3] = {TRIGGER_SPEED_PID_KP, TRIGGER_SPEED_PID_KI, TRIGGER_SPEED_PID_KD}; // 拨弹盘速度环

    PID_init(&SHOOT.trigger_angel_pid, PID_POSITION, pid_angel_trigger, TRIGGER_ANGEL_PID_MAX_OUT, TRIGGER_ANGEL_PID_MAX_IOUT);
    PID_init(&SHOOT.trigger_speed_pid, PID_POSITION, pid_speed_trigger, TRIGGER_SPEED_PID_MAX_OUT, TRIGGER_SPEED_PID_MAX_IOUT); // 拨弹盘初始化pid
  }
  else if (TRIGGER_MOTOR_TYPE == DJI_M3508)
  {
    const fp32 pid_angel_trigger[3] = {TRIGGER_ANGEL_PID_KP, TRIGGER_ANGEL_PID_KI, TRIGGER_ANGEL_PID_KD}; // 拨弹盘角度环
    const fp32 pid_speed_trigger[3] = {TRIGGER_SPEED_PID_KP, TRIGGER_SPEED_PID_KI, TRIGGER_SPEED_PID_KD}; // 拨弹盘速度环

    PID_init(&SHOOT.trigger_angel_pid, PID_POSITION, pid_angel_trigger, TRIGGER_ANGEL_PID_MAX_OUT, TRIGGER_ANGEL_PID_MAX_IOUT);
    PID_init(&SHOOT.trigger_speed_pid, PID_POSITION, pid_speed_trigger, TRIGGER_SPEED_PID_MAX_OUT, TRIGGER_SPEED_PID_MAX_IOUT); // 拨弹盘初始化pid
  }
  else if (TRIGGER_MOTOR_TYPE == DM_4310)
  {
    const fp32 pid_angel_trigger[3] = {TRIGGER_ANGEL_PID_KP, TRIGGER_ANGEL_PID_KI, TRIGGER_ANGEL_PID_KD}; // 拨弹盘角度环

    PID_init(&SHOOT.trigger_angel_pid, PID_POSITION, pid_angel_trigger, TRIGGER_ANGEL_PID_MAX_OUT, TRIGGER_ANGEL_PID_MAX_IOUT); // 拨弹盘初始化pid
  }

  all_Transmission_ratio_z = all_Transmission_ratio;
  COUNT = all_Transmission_ratio_z / 2.0f;
  if (all_Transmission_ratio_z / 2.0f == COUNT)
  {
    COUNT_error = 1;
  }
  else
  {
    COUNT_error = 0;
  }

  COUNT++;
}

/*-------------------- Set mode --------------------*/

/**
 * @brief          设置模式
 * @param[in]      none
 * @retval         none
 */
void ShootSetMode(void)
{
  /*键鼠遥控器控制方式初版----------------------------*/
  if (switch_is_down(SHOOT.rc->rc.s[SHOOT_MODE_CHANNEL])) // 下档防止误触
  {
    SHOOT.state = FRIC_NOT_READY;
    SHOOT.mode = LOAD_STOP;
  }

  else if (switch_is_mid(SHOOT.rc->rc.s[SHOOT_MODE_CHANNEL]))
  {
    // 测试使用
    SHOOT.state = FRIC_READY;
    SHOOT.mode = LOAD_STOP;
    // // 设置摩擦轮状态位
    // // if(SHOOT.rc->key.v & KEY_PRESSED_OFFSET_Q || GetScCmdFricOn())//Q启动摩擦轮
    // // {
    // //   SHOOT.fric_flag = 1;
    // // }
    // // else if(SHOOT.rc->key.v & KEY_PRESSED_OFFSET_E || !GetScCmdFricOn())//E关闭摩擦轮
    // // {
    // //   SHOOT.fric_flag = 0;
    // // }
    // // if(SHOOT.rc->key.v & KEY_PRESSED_OFFSET_Q)//Q启动摩擦轮
    // // {
    // //   SHOOT.fric_flag = 1;
    // // }
    // // else if(SHOOT.rc->key.v & KEY_PRESSED_OFFSET_E)//E关闭摩擦轮
    // // {
    // //   SHOOT.fric_flag = 0;
    // // }

    // // 测试状态设置摩擦轮状态位
    // SHOOT.fric_flag = 1;

    // if (SHOOT.fric_flag)
    // {
    //     SHOOT.state = FRIC_READY;
    // }
    // else
    // {
    //     SHOOT.state = FRIC_NOT_READY;
    // }

    // // 设置开火状态位
    // // if (SHOOT.rc->mouse.press_l && SHOOT.shoot_flag==0)
    // // {
    // //   SHOOT.mode = LAOD_BULLET;
    // // }
    // // else if (SHOOT.rc->mouse.press_r || GetScCmdFire())
    // // {
    // //   SHOOT.mode = LOAD_BURSTFIRE;
    // // }
    // // else
    // // {
    // //   SHOOT.mode = LOAD_STOP;
    // // }

    // // SHOOT.shoot_flag = SHOOT.rc->mouse.press_l;

    // // if (SHOOT.move_flag)
    // // {
    // //   SHOOT.mode = LAOD_BULLET;
    // //}

    // if (SHOOT.rc->mouse.press_l && !SHOOT.shoot_flag) // 左键单发
    // {
    //   SHOOT.mode = LAOD_BULLET;
    // }
    // else if (SHOOT.rc->mouse.press_r) // 右键连发
    // {
    //     // 这里根据上位机指令判断，暂时不需要
    //     // if (GetScCmdFire())
    //     // {
    //       SHOOT.mode = LOAD_BURSTFIRE;
    //     // }
    //     // else
    //     // {
    //     //   SHOOT.mode = LOAD_STOP;
    //     // }
    // }
    // else
    // {
    //   SHOOT.mode = LOAD_STOP;
    // }

    // SHOOT.shoot_flag = SHOOT.rc->mouse.press_l;

    // if (SHOOT.move_flag)
    // {
    //   SHOOT.mode = LAOD_BULLET;
    // }

    // // 右键连发计时
    // if (SHOOT.rc->mouse.press_l)
    // {
    //   if (SHOOT.mr_time < 180)
    //   {
    //     SHOOT.mr_time++;
    //   }
    //   else
    //   {
    //     SHOOT.mode = LOAD_BURSTFIRE;
    //     SHOOT.move_flag = 0;
    //   }
    // }
    // else
    // {
    //   SHOOT.mr_time = 0;
    // }
  }
  else if (switch_is_up(SHOOT.rc->rc.s[SHOOT_MODE_CHANNEL])) // 上档连发
  {
    // 清弹
    SHOOT.state = FRIC_READY;
    SHOOT.mode = LOAD_BURSTFIRE; // LOAD_BURSTFIRE;

    // 上位机测试
    //  SHOOT.state = FRIC_READY;

    // if (GetScCmdFire())
    // {
    //   SHOOT.mode = LOAD_BURSTFIRE;
    // }
    // else
    // {
    //   SHOOT.mode = LOAD_STOP;
    // }
  }

  // 防堵转
  if (SHOOT.mode == LOAD_BURSTFIRE || SHOOT.mode == LAOD_BULLET)
  {
    if (SHOOT.block_time >= BLOCK_TIME)
    {
      // 不启动拨弹轮暂时注释
      SHOOT.mode = LOAD_BLOCK;
    }

    // 堵转时间计时
    if (fabs(SHOOT.last_trigger_vel) < BLOCK_TRIGGER_SPEED && SHOOT.block_time < BLOCK_TIME)
    {
      SHOOT.block_time++;
      SHOOT.reverse_time = 0;
    }
    else if (SHOOT.block_time == BLOCK_TIME && SHOOT.reverse_time < REVERSE_TIME)
    {
      SHOOT.reverse_time++;
    }
    else
    {
      SHOOT.block_time = 0;
    }
  }

  // //过热保护
  // if (fabs(SHOOT.last_fric_vel) < FRIC_SPEED_LIMIT)
  // {
  //   SHOOT.mode = LOAD_STOP;
  //   fric_ui = 0;
  // } else {
  //   fric_ui = 1;
  // }

  // 热量限制
  if (TRIGGER_MOTOR_TYPE == DJI_M2006)
  {
    SHOOT.heat = get_heat_auto();
    SHOOT.heat_limit = get_heat_limit();
  }
  else if (TRIGGER_MOTOR_TYPE == DJI_M3508)
  {
    SHOOT.heat = get_heat_auto();
    SHOOT.heat_limit = get_heat_limit();
  }
  else if (TRIGGER_MOTOR_TYPE == DM_4310)
  {
    SHOOT.heat = get_heat_auto();
    SHOOT.heat_limit = get_heat_limit();
  }

  // if ((SHOOT.heat + SHOOT_HEAT_REMAIN_VALUE) > SHOOT.heat_limit)
  // {
  //   SHOOT.mode = LOAD_STOP;
  // }

  // 安全档
  if ((switch_is_down(SHOOT.rc->rc.s[1])))
  {
    SHOOT.mode = LOAD_STOP;
    SHOOT.state = FRIC_NOT_READY;
  }

  // 遥控器离线保护
  if (toe_is_error(DBUS_TOE))
  {
    SHOOT.state = FRIC_NOT_READY;
    SHOOT.mode = LOAD_STOP;
  }
}

/*-------------------- Observe --------------------*/

/**
 * @brief          更新状态量
 * @param[in]      none
 * @retval         none
 */
void ShootObserver(void)
{
#if(aaa1==1)
  //---------测试代码---------
  PID_TO_VOFA_DATA.angle_set = SHOOT.trigger_angel_pid.set;
  PID_TO_VOFA_DATA.angle_fdb = SHOOT.trigger_angel_pid.fdb;
  PID_TO_VOFA_DATA.angle_out = SHOOT.trigger_angel_pid.out;
  PID_TO_VOFA_DATA.angle_Pout = SHOOT.trigger_angel_pid.Pout;
  PID_TO_VOFA_DATA.angle_Iout = SHOOT.trigger_angel_pid.Iout;
  PID_TO_VOFA_DATA.angle_Dout = SHOOT.trigger_angel_pid.Dout;
  PID_TO_VOFA_DATA.speed_set = SHOOT.trigger_speed_pid.set;
  PID_TO_VOFA_DATA.speed_fdb = SHOOT.trigger_speed_pid.fdb;
  PID_TO_VOFA_DATA.speed_out = SHOOT.trigger_speed_pid.out;
  PID_TO_VOFA_DATA.speed_Pout = SHOOT.trigger_speed_pid.Pout;
  PID_TO_VOFA_DATA.speed_Iout = SHOOT.trigger_speed_pid.Iout;
  PID_TO_VOFA_DATA.speed_Dout = SHOOT.trigger_speed_pid.Dout;
  // ----------------------------
#endif

  GetMotorMeasure(&SHOOT.trigger_motor);
  GetMotorMeasure(&SHOOT.fric_motor[0]);
  GetMotorMeasure(&SHOOT.fric_motor[1]);
  GetMotorMeasure(&SHOOT.fric_motor[2]);

  SHOOT.FDB.fric_speed_fdb_R = SHOOT.fric_motor[0].fdb.vel;
  SHOOT.FDB.fric_speed_fdb_L = SHOOT.fric_motor[1].fdb.vel;
  SHOOT.FDB.fric_speed_fdb_U = SHOOT.fric_motor[2].fdb.vel;

  SHOOT.FDB.trigger_speed_fdb = SHOOT.trigger_motor.fdb.vel;

  if (TRIGGER_MOTOR_TYPE == DJI_M2006)
  {

    // 拨弹轮点击编码器值规整
    if (SHOOT.trigger_motor.fdb.ecd - SHOOT.last_ecd > HALF_ECD_RANGE)
    {
      SHOOT.ecd_count--;
      all_error += error1;
    }
    else if (SHOOT.trigger_motor.fdb.ecd - SHOOT.last_ecd < -HALF_ECD_RANGE)
    {
      SHOOT.ecd_count++;
      all_error -= error1;
    }

    if (SHOOT.ecd_count == COUNT)
    {
      SHOOT.ecd_count = -COUNT + COUNT_error;
    }
    else if (SHOOT.ecd_count == -COUNT + COUNT_error)
    {
      SHOOT.ecd_count = COUNT;
    }

    // 计算输出轴角度
    SHOOT.FDB.trigger_angel_fdb = (SHOOT.ecd_count * ECD_RANGE + SHOOT.trigger_motor.fdb.ecd + all_error) * 2 * PI / (all_Transmission_ratio * ECD_RANGE);
    SHOOT.FDB.trigger_angel_fdb = theta_format(SHOOT.trigger_motor.fdb.pos);
    // 记录上一个ecd值
    SHOOT.last_ecd = SHOOT.trigger_motor.fdb.ecd;

    // 电机圈数重置， 因为输出轴旋转一圈， 电机轴旋转 36圈，将电机轴数据处理成输出轴数据，用于控制输出轴角度
    // if(FULL_COUNT%2 == 0)
    //{
    //  if (SHOOT.trigger_motor.fdb.ecd - SHOOT.last_ecd > HALF_ECD_RANGE)
    //  {
    //      SHOOT.ecd_count--;
    //  }
    //  else if (SHOOT.trigger_motor.fdb.ecd - SHOOT.last_ecd < -HALF_ECD_RANGE)
    //  {

    //     SHOOT.ecd_count++;
    // }

    // if (SHOOT.ecd_count == FULL_COUNT)
    // {
    //     SHOOT.ecd_count = -(FULL_COUNT - 1);
    // }
    // else if (SHOOT.ecd_count == -FULL_COUNT)
    // {
    //     SHOOT.ecd_count = FULL_COUNT-1;
    // }
    //}
    // //电机圈数重置， 因为输出轴旋转一圈， 电机轴旋转 51圈，将电机轴数据处理成输出轴数据，用于控制输出轴角度
    // else
    // {
    //   if (SHOOT.trigger_motor.fdb.ecd - SHOOT.last_ecd > HALF_ECD_RANGE)
    //   {
    //       SHOOT.ecd_count--;
    //   }
    //   else if (SHOOT.trigger_motor.fdb.ecd - SHOOT.last_ecd < -HALF_ECD_RANGE)
    //   {

    //       SHOOT.ecd_count++;
    //   }

    //   if (SHOOT.ecd_count == FULL_COUNT)
    //   {
    //       SHOOT.ecd_count = -FULL_COUNT;
    //   }
    //   else if (SHOOT.ecd_count == -FULL_COUNT)
    //   {
    //       SHOOT.ecd_count = FULL_COUNT;
    //   }
    //}
  }
  else if (TRIGGER_MOTOR_TYPE == DJI_M3508)
  {

    // 拨弹轮点击编码器值规整
    if (SHOOT.trigger_motor.fdb.ecd - SHOOT.last_ecd > HALF_ECD_RANGE)
    {
      SHOOT.ecd_count--;
    }
    else if (SHOOT.trigger_motor.fdb.ecd - SHOOT.last_ecd < -HALF_ECD_RANGE)
    {
      SHOOT.ecd_count++;
    }

    if (SHOOT.ecd_count == FULL_COUNT)
    {
      SHOOT.ecd_count = -(FULL_COUNT - 1);
    }
    else if (SHOOT.ecd_count == -FULL_COUNT)
    {
      SHOOT.ecd_count = FULL_COUNT - 1;
    }

    // 计算输出轴角度
    SHOOT.FDB.trigger_angel_fdb = (SHOOT.ecd_count * ECD_RANGE + SHOOT.trigger_motor.fdb.ecd) * MOTOR_ECD_TO_ANGLE;

    // 记录上一个ecd值
    SHOOT.last_ecd = SHOOT.trigger_motor.fdb.ecd;

    // 电机圈数重置， 因为输出轴旋转一圈， 电机轴旋转 36圈，将电机轴数据处理成输出轴数据，用于控制输出轴角度
    // if(FULL_COUNT%2 == 0)
    //{
    //  if (SHOOT.trigger_motor.fdb.ecd - SHOOT.last_ecd > HALF_ECD_RANGE)
    //  {
    //      SHOOT.ecd_count--;
    //  }
    //  else if (SHOOT.trigger_motor.fdb.ecd - SHOOT.last_ecd < -HALF_ECD_RANGE)
    //  {

    //     SHOOT.ecd_count++;
    // }

    // if (SHOOT.ecd_count == FULL_COUNT)
    // {
    //     SHOOT.ecd_count = -(FULL_COUNT - 1);
    // }
    // else if (SHOOT.ecd_count == -FULL_COUNT)
    // {
    //     SHOOT.ecd_count = FULL_COUNT-1;
    // }
    //}
    // //电机圈数重置， 因为输出轴旋转一圈， 电机轴旋转 51圈，将电机轴数据处理成输出轴数据，用于控制输出轴角度
    // else
    // {
    //   if (SHOOT.trigger_motor.fdb.ecd - SHOOT.last_ecd > HALF_ECD_RANGE)
    //   {
    //       SHOOT.ecd_count--;
    //   }
    //   else if (SHOOT.trigger_motor.fdb.ecd - SHOOT.last_ecd < -HALF_ECD_RANGE)
    //   {

    //       SHOOT.ecd_count++;
    //   }

    //   if (SHOOT.ecd_count == FULL_COUNT)
    //   {
    //       SHOOT.ecd_count = -FULL_COUNT;
    //   }
    //   else if (SHOOT.ecd_count == -FULL_COUNT)
    //   {
    //       SHOOT.ecd_count = FULL_COUNT;
    //   }
    //}
  }
  else if (TRIGGER_MOTOR_TYPE == DM_4310)
  {
    SHOOT.FDB.trigger_angel_fdb = theta_format(SHOOT.trigger_motor.fdb.pos);
  }

  // 记录上一个拨弹盘vel,用于堵转模式判断
  SHOOT.last_trigger_vel = SHOOT.trigger_motor.fdb.vel;

  // 记录上一个摩擦轮vel,用于过热保护
  SHOOT.last_fric_vel = SHOOT.fric_motor[0].fdb.vel;
}

/*-------------------- Reference --------------------*/

/**
 * @brief          更新目标量
 * @param[in]      none
 * @retval         none
 */
void ShootReference(void)
{

  // 摩擦轮目标量设置
  switch (SHOOT.state)
  {
  case FRIC_NOT_READY:
    SHOOT.REF.fric_speed_ref_R = 0.0f;
    SHOOT.REF.fric_speed_ref_L = 0.0f;
    SHOOT.REF.fric_speed_ref_U = 0.0f;
    break;

  case FRIC_READY:
    SHOOT.REF.fric_speed_ref_R = FRIC_R_SPEED;
    SHOOT.REF.fric_speed_ref_L = FRIC_L_SPEED;
    SHOOT.REF.fric_speed_ref_U = FRIC_U_SPEED;
    break;

  default:
    break;
  }

  // 拨弹盘目标量设置
  switch (SHOOT.mode)
  {
  case LOAD_STOP:
    SHOOT.REF.trigger_speed_ref = 0.0f;
    break;

  case LAOD_BULLET:
    if (TRIGGER_MOTOR_TYPE == DJI_M2006)
    {
      date++;
      if (SHOOT.move_flag == 0 && date > 1600)
      {
        SHOOT.REF.trigger_angel_ref = theta_format(SHOOT.REF.trigger_angel_ref - 2 * PI / BULLET_NUM);
        date = 0;
      }
      // if (SHOOT.move_flag == 0)
      // {
      //   SHOOT.REF.trigger_angel_ref = theta_format(SHOOT.FDB.trigger_angel_fdb + 2 * PI / BULLET_NUM / TRIGGER_REDUCTION_RATIO );
      // }

      // if (theta_format(SHOOT.REF.trigger_angel_ref - SHOOT.FDB.trigger_angel_fdb) > 0.01f)
      if (theta_format(SHOOT.REF.trigger_angel_ref - SHOOT.FDB.trigger_angel_fdb) < -0.01f)
      {
        SHOOT.move_flag = 1;
      }
      else
      {
        SHOOT.move_flag = 0;
      }
    }
    else if (TRIGGER_MOTOR_TYPE == DJI_M3508)
    {
      if (SHOOT.move_flag == 0)
      {
        SHOOT.REF.trigger_angel_ref = theta_format(SHOOT.FDB.trigger_angel_fdb + 2 * PI / BULLET_NUM / TRIGGER_REDUCTION_RATIO);
      }

      // if (theta_format(SHOOT.REF.trigger_angel_ref - SHOOT.FDB.trigger_angel_fdb) > 0.01f)
      if (theta_format(SHOOT.REF.trigger_angel_ref - SHOOT.FDB.trigger_angel_fdb) < -0.01f)

      {
        SHOOT.move_flag = 1;
      }
      else
      {
        SHOOT.move_flag = 0;
      }
    }
    else if (TRIGGER_MOTOR_TYPE == DM_4310)
    {
      if (SHOOT.move_flag == 0)
      {
        SHOOT.REF.trigger_angel_ref = theta_format(SHOOT.FDB.trigger_angel_fdb - 2.0f * PI / BULLET_NUM / TRIGGER_REDUCTION_RATIO);
      }

      if (theta_format(SHOOT.FDB.trigger_angel_fdb - SHOOT.REF.trigger_angel_ref) > 0.01f)
      {
        SHOOT.move_flag = 1;
      }
      else
      {
        SHOOT.move_flag = 0;
      }
    }
    break;

  case LOAD_BURSTFIRE:
    SHOOT.REF.trigger_speed_ref = TRIGGER_SPEED;
    break;

  case LOAD_BLOCK:
    SHOOT.REF.trigger_speed_ref = REVERSE_SPEED;
    break;

  default:
    break;
  }
}

/*-------------------- Console --------------------*/

/**
 * @brief          计算控制量
 * @param[in]      none
 * @retval         none
 */
void ShootConsole(void)
{

  // ----------测试----------

  //-------------------------


  SHOOT.fric_motor[0].set.curr = PID_calc(&SHOOT.fric_pid[0], SHOOT.FDB.fric_speed_fdb_R, SHOOT.REF.fric_speed_ref_R);
  SHOOT.fric_motor[1].set.curr = PID_calc(&SHOOT.fric_pid[1], SHOOT.FDB.fric_speed_fdb_L, SHOOT.REF.fric_speed_ref_L);
  SHOOT.fric_motor[2].set.curr = PID_calc(&SHOOT.fric_pid[2], SHOOT.FDB.fric_speed_fdb_U, SHOOT.REF.fric_speed_ref_U);
  // PID_calc(&SHOOT.fric_pid[0], SHOOT.FDB.fric_speed_fdb_R,SHOOT.REF.fric_speed_ref_R);

  if (TRIGGER_MOTOR_TYPE == DJI_M2006)
  {
    if (SHOOT.mode == LOAD_STOP)
    {
      SHOOT.trigger_motor.set.curr = PID_calc(&SHOOT.trigger_speed_pid, SHOOT.FDB.trigger_speed_fdb, SHOOT.REF.trigger_speed_ref);
    }
    else if (SHOOT.mode == LOAD_BURSTFIRE)
    {
      SHOOT.trigger_motor.set.curr = PID_calc(&SHOOT.trigger_speed_pid, SHOOT.FDB.trigger_speed_fdb, SHOOT.REF.trigger_speed_ref);
    }
    else if (SHOOT.mode == LAOD_BULLET)
    {
      delta = theta_format(SHOOT.REF.trigger_angel_ref - SHOOT.FDB.trigger_angel_fdb);

      SHOOT.REF.trigger_speed_ref = PID_calc(&SHOOT.trigger_angel_pid, 0, delta);
      SHOOT.trigger_motor.set.curr = PID_calc(&SHOOT.trigger_speed_pid, SHOOT.FDB.trigger_speed_fdb, SHOOT.REF.trigger_speed_ref);
    }
    else if (SHOOT.mode == LOAD_BLOCK)
    {
      SHOOT.trigger_motor.set.curr = PID_calc(&SHOOT.trigger_speed_pid, SHOOT.FDB.trigger_speed_fdb, SHOOT.REF.trigger_speed_ref);
    }
  }
  if (TRIGGER_MOTOR_TYPE == DJI_M3508)
  {
    if (SHOOT.mode == LOAD_STOP)
    {
      SHOOT.trigger_motor.set.curr = PID_calc(&SHOOT.trigger_speed_pid, SHOOT.FDB.trigger_speed_fdb, SHOOT.REF.trigger_speed_ref);
    }
    else if (SHOOT.mode == LOAD_BURSTFIRE)
    {
      SHOOT.trigger_motor.set.curr = PID_calc(&SHOOT.trigger_speed_pid, SHOOT.FDB.trigger_speed_fdb, SHOOT.REF.trigger_speed_ref);
    }
    else if (SHOOT.mode == LAOD_BULLET)
    {
      delta = theta_format(SHOOT.REF.trigger_angel_ref - SHOOT.FDB.trigger_angel_fdb);

      SHOOT.REF.trigger_speed_ref = PID_calc(&SHOOT.trigger_angel_pid, 0, delta);
      SHOOT.trigger_motor.set.curr = PID_calc(&SHOOT.trigger_speed_pid, SHOOT.FDB.trigger_speed_fdb, SHOOT.REF.trigger_speed_ref);
    }
    else if (SHOOT.mode == LOAD_BLOCK)
    {
      SHOOT.trigger_motor.set.curr = PID_calc(&SHOOT.trigger_speed_pid, SHOOT.FDB.trigger_speed_fdb, SHOOT.REF.trigger_speed_ref);
    }
  }
  else if (TRIGGER_MOTOR_TYPE == DM_4310)
  {
    if (SHOOT.mode == LOAD_STOP)
    {
      SHOOT.trigger_motor.set.vel = SHOOT.REF.trigger_speed_ref;
    }
    else if (SHOOT.mode == LOAD_BURSTFIRE)
    {
      SHOOT.trigger_motor.set.vel = SHOOT.REF.trigger_speed_ref;
    }
    else if (SHOOT.mode == LAOD_BULLET)
    {
      delta = theta_format(SHOOT.REF.trigger_angel_ref - SHOOT.FDB.trigger_angel_fdb);
      SHOOT.trigger_motor.set.vel = PID_calc(&SHOOT.trigger_angel_pid, 0, delta);
    }
    else if (SHOOT.mode == LOAD_BLOCK)
    {
      SHOOT.trigger_motor.set.vel = SHOOT.REF.trigger_speed_ref;
    }
  }
}

/*-------------------- Cmd --------------------*/

/**
 * @brief          发送控制量
 * @param[in]      none
 * @retval         none
 */
void ShootSendCmd(void)
{
  // 为两条CAN总线和两种标准ID创建发送数组
  int16_t can1_0x200[4] = {0, 0, 0, 0}; // CAN1 ID 1-4
  int16_t can1_0x1FF[4] = {0, 0, 0, 0}; // CAN1 ID 5-8
  int16_t can2_0x200[4] = {0, 0, 0, 0}; // CAN2 ID 1-4
  int16_t can2_0x1FF[4] = {0, 0, 0, 0}; // CAN2 ID 5-8

  bool need_send_can1_0x200 = false;
  bool need_send_can1_0x1FF = false;
  bool need_send_can2_0x200 = false;
  bool need_send_can2_0x1FF = false;

  // 处理三个摩擦轮电机
  for (int i = 0; i < 3; ++i)
  {
    uint8_t motor_id = SHOOT.fric_motor[i].id;
    uint8_t can_bus = SHOOT.fric_motor[i].can;
    int16_t current = SHOOT.fric_motor[i].set.curr;

    // 根据CAN总线和ID范围分配
    if (can_bus == 1)
    {
      if (motor_id >= 1 && motor_id <= 4)
      {
        can1_0x200[motor_id - 1] = current;
        need_send_can1_0x200 = true;
      }
      else if (motor_id >= 5 && motor_id <= 8)
      {
        can1_0x1FF[motor_id - 5] = current;
        need_send_can1_0x1FF = true;
      }
    }
    else if (can_bus == 2)
    {
      if (motor_id >= 1 && motor_id <= 4)
      {
        can2_0x200[motor_id - 1] = current;
        need_send_can2_0x200 = true;
      }
      else if (motor_id >= 5 && motor_id <= 8)
      {
        can2_0x1FF[motor_id - 5] = current;
        need_send_can2_0x1FF = true;
      }
    }
  }

  // 处理拨弹轮电机（仅DJI电机需要CAN发送）
  if (TRIGGER_MOTOR_TYPE == DJI_M2006 || TRIGGER_MOTOR_TYPE == DJI_M3508)
  {
    uint8_t trigger_id = SHOOT.trigger_motor.id;
    uint8_t trigger_can = SHOOT.trigger_motor.can;
    int16_t trigger_current = SHOOT.trigger_motor.set.curr;

    if (trigger_can == 1)
    {
      if (trigger_id >= 1 && trigger_id <= 4)
      {
        can1_0x200[trigger_id - 1] = trigger_current;
        need_send_can1_0x200 = true;
      }
      else if (trigger_id >= 5 && trigger_id <= 8)
      {
        can1_0x1FF[trigger_id - 5] = trigger_current;
        need_send_can1_0x1FF = true;
      }
    }
    else if (trigger_can == 2)
    {
      if (trigger_id >= 1 && trigger_id <= 4)
      {
        can2_0x200[trigger_id - 1] = trigger_current;
        need_send_can2_0x200 = true;
      }
      else if (trigger_id >= 5 && trigger_id <= 8)
      {
        can2_0x1FF[trigger_id - 5] = trigger_current;
        need_send_can2_0x1FF = true;
      }
    }
  }

  // 发送所有需要的CAN命令
  if (need_send_can1_0x200)
  {
    CanCmdDjiMotor(1, 0x200, can1_0x200[0], can1_0x200[1], can1_0x200[2], can1_0x200[3]);
  }

  if (need_send_can1_0x1FF)
  {
    CanCmdDjiMotor(1, 0x1FF, can1_0x1FF[0], can1_0x1FF[1], can1_0x1FF[2], can1_0x1FF[3]);
  }

  if (need_send_can2_0x200)
  {
    CanCmdDjiMotor(2, 0x200, can2_0x200[0], can2_0x200[1], can2_0x200[2], can2_0x200[3]);
  }

  if (need_send_can2_0x1FF)
  {
    CanCmdDjiMotor(2, 0x1FF, can2_0x1FF[0], can2_0x1FF[1], can2_0x1FF[2], can2_0x1FF[3]);
  }

  // 处理达妙电机（独立发送）
  if (TRIGGER_MOTOR_TYPE == DM_4310)
  {
    if (SHOOT.trigger_motor.fdb.state == DM_STATE_DISABLE)
    {
      DmEnable(&SHOOT.trigger_motor);
    }
    DmMitCtrlVelocity(&SHOOT.trigger_motor, TRIGGER_SPEED_MIT_KD);
  }
}

#endif // SHOOT_TYPE == SHOOT_FRIC
