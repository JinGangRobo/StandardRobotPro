/**
  ****************************(C) COPYRIGHT 2024 Polarbear****************************
  * @file       SupCap.c/h
  * @brief      超级电容相关部分定义
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Nov-29-2024     Penguin         1. 完成。
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2024 Polarbear****************************
  */

#ifndef SUPCAP_H
#define SUPCAP_H

#include "struct_typedef.h"
#include "stdbool.h"

typedef enum
{
    NORMAL          = 1,   // 正常工作状态
    ERR_NO_POW,            // 无输入电源错误
    ERR_NO_CAP,            // 电容未连接错误
    ERR_CAP_HV,
    ERR_SHORT_CIRCUIT      // 输出短路错误
} supcap_status_e;         

/** 
 * @brief        通用超级电容结构体
 * @note         包括电容的信息、状态量和控制量
 */
typedef struct
{
    supcap_status_e state;      // 状态标志
    bool offline;               // 超级电容是否离线 0-在线 1-离线
    float voltage_B;            // (V)电容侧电压
    float power;                // (W)底盘功率
    uint32_t last_fdb_time;     //上次反馈时间
} SupCap_s;

extern void SupCapInit(SupCap_s * p_sup_cap, uint8_t can);

#endif
/************************ END OF FILE ************************/
