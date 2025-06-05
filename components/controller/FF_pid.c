#include "FF_pid.h"

#include "main.h"

#define LimitMax(input, max)       \
    {                              \
        if (input > max) {         \
            input = max;           \
        } else if (input < -max) { \
            input = -max;          \
        }                          \
    }

/**
  * @brief          pid struct data init
  * @param[out]     pid: PID结构数据指针
  * @param[in]      mode: PID_POSITION:普通PID
  *                 PID_DELTA: 差分PID
  * @param[in]      PID: 0: kp, 1: ki, 2:kd
  * @param[in]      max_out: pid最大输出
  * @param[in]      max_iout: pid最大积分输出
  * @param[in]      max_iout: pid最大积分输出
  * @retval         none
  */
void FF_PID_init(FF_pid_type_def * FF_pid, uint8_t mode, const fp32 PID[3], fp32 max_out, fp32 max_iout)
{
    if (FF_pid == NULL || PID == NULL) {
        return;
    }
    FF_pid->Kp = PID[0];
    FF_pid->Ki = PID[1];
    FF_pid->Kd = PID[2];
    FF_pid->max_out = max_out;
    FF_pid->max_iout = max_iout;
    FF_pid->error[0] = FF_pid->error[1] = FF_pid->Pout = FF_pid->Iout = FF_pid->Dout = FF_pid->out =
        0.0f;
}

/**
  * @brief          pid计算
  * @param[out]     pid: PID结构数据指针
  * @param[in]      ref: 反馈数据
  * @param[in]      set: 设定值
  * @param[in]      Sys_est:系统估计值
  * @retval         pid输出
  */
fp32 FF_PID_calc(FF_pid_type_def * FF_pid, fp32 ref, fp32 set,fp32 Sys_est)
{
    if (FF_pid == NULL) {
        return 0.0f;
    }
    FF_pid->error[1] = FF_pid->error[0];
    FF_pid->set = set;
    FF_pid->fdb = ref;
    FF_pid->error[0] = set - ref;
        FF_pid->Pout = FF_pid->Kp * FF_pid->error[0];
        FF_pid->Iout += FF_pid->Ki * FF_pid->error[0];
        FF_pid->Dout = FF_pid->Kd * (FF_pid->error[0]-FF_pid->error[1]);
        LimitMax(FF_pid->Iout, FF_pid->max_iout);
        FF_pid->out = FF_pid->Pout + FF_pid->Iout + FF_pid->Dout + Sys_est;
        LimitMax(FF_pid->out, FF_pid->max_out);
    return FF_pid->out;

}
