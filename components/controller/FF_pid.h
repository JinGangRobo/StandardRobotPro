#ifndef FF_PID_H
#define FF_PID_H
#include "pid.h"
#include "struct_typedef.h"
#include "main.h"

typedef struct
{
    //PID 三参数
    fp32 Kp;
    fp32 Ki;
    fp32 Kd;

    // fp32 Sys_est; //系统估计值，此值为输出预估值，将与pid输出值直接叠加，受max_out限制
    fp32 max_out;   //最大输出
    fp32 max_iout;  //最大积分输出

    fp32 set;
    fp32 fdb;

    fp32 out;
    fp32 Pout;
    fp32 Iout;
    fp32 Dout;
    fp32 error[2];  //误差项 0最新 1上一次 2上上次

} FF_pid_type_def;
fp32 FF_PID_calc(pid_type_def * FF_pid, fp32 ref, fp32 set,fp32 Sys_est);
void FF_PID_init(FF_pid_type_def * FF_pid, uint8_t mode, const fp32 PID[3], fp32 max_out, fp32 max_iout);
#endif
