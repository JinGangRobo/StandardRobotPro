#ifndef NPID_H
#define NPID_H
#include "struct_typedef.h"

typedef struct
{
    float setpoint;  /*设定值*/
    float kcoef;     /*神经元输出比例*/
    float kp;        /*比例学习速度*/
    float ki;        /*积分学习速度*/
    float kd;        /*微分学习速度*/
    float lasterror; /*前一拍偏差*/
    float preerror;  /*前两拍偏差*/
    float deadband;  /*死区*/
    float result;    /*输出值*/
    float output;    /*百分比输出值*/
    float maximum;   /*输出值的上限*/
    float minimum;   /*输出值的下限*/
    float wp;        /*比例加权系数*/
    float wi;        /*积分加权系数*/
    float wd;        /*微分加权系数*/
} neural_pid;

void npid_init(neural_pid *vPID, float vMax, float vMin);
void npid_calc(neural_pid *vPID, float pv);
static void learning_rules(neural_pid *vPID, float zk, float uk, float *xi);

#endif