/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       referee.c/h
  * @brief      RM referee system data solve. RM裁判系统数据处理
  * @note
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Nov-11-2019     RM              1. done
  *  V1.6.4     Dec-03-2024     L-Win           2. 完成协议结构体更新
  *  V1.7.0     Feb-16-2025     L-Win           3. 完成协议结构体更新
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */

#ifndef REFEREE_H
#define REFEREE_H

#include "main.h"

#include "protocol.h"

// 帧头定义在protocol.h中定义

typedef enum {
    RED_HERO        = 1,
    RED_ENGINEER    = 2,
    RED_STANDARD_1  = 3,
    RED_STANDARD_2  = 4,
    RED_STANDARD_3  = 5,
    RED_AERIAL      = 6,
    RED_SENTRY      = 7,
    RED_DART        = 8,
    RED_RADAR       = 9,
    RED_OUTPOST     = 10,
    RED_BASE        = 11,
    BLUE_HERO       = 101,
    BLUE_ENGINEER   = 102,
    BLUE_STANDARD_1 = 103,
    BLUE_STANDARD_2 = 104,
    BLUE_STANDARD_3 = 105,
    BLUE_AERIAL     = 106,
    BLUE_SENTRY     = 107,
    BLUE_DART       = 108,
    BLUE_RADAR      = 109,
    BLUE_OUTPOST    = 110,
    BLUE_BASE       = 111,
} robot_id_t; // 机器人ID的枚举

typedef enum {
    PROGRESS_UNSTART     = 0,
    PROGRESS_PREPARE     = 1,
    PROGRESS_SELFCHECK   = 2,
    PROGRESS_5sCOUNTDOWN = 3,
    PROGRESS_BATTLE      = 4,
    PROGRESS_CALCULATING = 5,
} game_progress_t; // 比赛状态的枚举

//---------------------常规链路数据说明----------------------
// 0x0001 11 比赛状态数据 服务器→全体机器人
typedef struct
{
    uint8_t game_type : 4;      // 2:高校单项 4:3v3对抗 5: 1v1对抗
    uint8_t game_progress : 4;  // 0:未开始 1:准备 2:十五秒自检 3:五秒倒计时 4:比赛中 5:比赛结算
    uint16_t stage_remain_time; // 当前阶段剩余时间 单位秒
    uint64_t SyncTimeStamp;     // UNIX时间，裁判系统连接NTP服务器生效
} __attribute__((packed)) game_state_t;

// 0x0002 1 比赛结果数据 服务器→全体机器人
typedef struct
{
    uint8_t winner; // 0:平局 1:红方胜 2:蓝方胜
} __attribute__((packed)) game_result_t;

// 0x0002 32 机器人血量数据 服务器→全体机器人
typedef struct
{
    uint16_t red_1_robot_HP; // 英雄，若未上场或罚下，值为0
    uint16_t red_2_robot_HP;
    uint16_t red_3_robot_HP; // 步兵
    uint16_t red_4_robot_HP;
    uint16_t reserved_0;
    uint16_t red_7_robot_HP; // 哨兵
    uint16_t red_outpost_HP;
    uint16_t red_base_HP;     // 基地
    uint16_t blue_1_robot_HP; // 英雄
    uint16_t blue_2_robot_HP;
    uint16_t blue_3_robot_HP; // 步兵
    uint16_t blue_4_robot_HP;
    uint16_t reserved_1;
    uint16_t blue_7_robot_HP; // 哨兵
    uint16_t blue_outpost_HP;
    uint16_t blue_base_HP; // 基地
} __attribute__((packed)) game_robot_HP_t;

// 0x0101 4 场地事件数据固定以1Hz频率发送 服务器→己方全体机器人
typedef struct
{
    uint32_t event_type; // bit30-31 中心增益点占领 0:未占领 1:已方占领 2:敌方占领 3:双方占领
} __attribute__((packed)) event_data_t;

// v1.7.0串口协议不存在
// 0x0102 4 补给站动作标识数据，补给站弹丸释放时触发发送 服务器→己方全体机器人
//  typedef __packed struct
//  {
//    uint8_t reserved;
//    uint8_t supply_robot_id;
//    uint8_t supply_projectile_step;
//    uint8_t supply_projectile_num;
//  } ext_supply_projectile_action_t;

// v1.6.4串口协议不存在
//  typedef __packed struct //0x0103
//  {
//      uint8_t supply_projectile_id;
//      uint8_t supply_robot_id;
//      uint8_t supply_num;
//  } ext_supply_projectile_booking_t;

// 0x0104 3 裁判警告数据，己方判罚/判负时触发发送，其余时间以1Hz频率发送 服务器→被判罚方全体机器人
typedef struct
{
    uint8_t level;              // 最后一次判罚等级 1:双黄 2:黄 3:红 4:判负
    uint8_t offending_robot_id; // 最后一次判罚机器人ID 0:双黄 1:红一 101:蓝一
    uint8_t count;              // 己方最后一次受到判罚的违规机器人对应判罚等级的违规次数。（开局默认为0。）
} __attribute__((packed)) referee_warning_t;

// 0x0105 3 飞镖发射相关数据，固定以1Hz频率发送 服务器→己方全体机器人
typedef struct
{
    uint8_t dart_remaining_time; // 己方飞镖发射剩余时间，单位：秒
    uint16_t dart_info;          // bit 0-1： 最近一次己方飞镖击中的目标，开局默认为0，1为击中前哨站，
                                 // 2为击中基地固定目标，3为击中基地随机目标
                                 // bit 2-4： 对方最近被击中的目标累计被击中计数，开局默认为0，
                                 // 至多为4 bit 5-6： 飞镖此时选定的击打目标，开局默认或未选定/选定前哨站时为0，
                                 // 选中基地固定目标为1，选中基地随机目标为2 bit 7-15：保留
} __attribute__((packed)) dart_info_t;

// 0x0201 13 机器人性能体系数据，固定以10Hz频率发送  主控模块→对应机器人
typedef struct
{
    uint8_t robot_id;                            // 本机器人ID
    uint8_t robot_level;                         // 机器人等级
    uint16_t current_HP;                         // 机器人当前血量
    uint16_t maximum_HP;                         // 机器人血量上限
    uint16_t shooter_barrel_cooling_value;       // 机器人枪口热量每秒冷却值
    uint16_t shooter_barrel_heat_limit;          // 机器人枪口热量上限
    uint16_t chassis_power_limit;                // 机器人底盘功率上限
    uint8_t power_management_gimbal_output : 1;  // 电源管理模块的输出情况： bit 0：gimbal口输出：0为无输出，1为24V输出
    uint8_t power_management_chassis_output : 1; // bit 1：chassis口输出：0为无输出，1为24V输出
    uint8_t power_management_shooter_output : 1; // bit 2：shooter口输出：0为无输出，1为24V输出 bit 3-7：保留
} __attribute__((packed)) robot_status_t;

// 0x0202 16 实时底盘功率和枪口热量数据，固定以50Hz频率发送 主控模块→对应机器人
typedef struct
{
    uint16_t reserved_0;                 // 保留位
    uint16_t reserved_1;                 // 保留位
    float reserved_2;                    // 保留位
    uint16_t buffer_energy;              // 缓冲能量（单位：J）
    uint16_t shooter_17mm_1_barrel_heat; // 第1个17mm发射机构的枪口热量
    uint16_t shooter_17mm_2_barrel_heat; // 第2个17mm发射机构的枪口热量
    uint16_t shooter_42mm_barrel_heat;   // 42mm发射机构的枪口热量
} __attribute__((packed)) power_heat_data_t;

// 0x0203 16  机器人位置数据，固定以1Hz频率发送 主控模块→对应机器人
typedef struct
{
    float x;     // 本机器人位置x坐标，单位：m
    float y;     // 本机器人位置y坐标，单位：m
    float angle; // 本机器人测速模块的朝向，单位：度。正北为0度
} __attribute__((packed)) robot_pos_t;

// 0x0204 6 机器人增益数据，固定以3Hz频率发送 服务器→对应机器人
typedef struct
{
    uint8_t recovery_buff;      // 机器人回血增益（百分比，值为10表示每秒恢复血量上限的10%）
    uint8_t cooling_buff;       // 机器人枪口冷却倍率（直接值，值为5表示5倍冷却）
    uint8_t defence_buff;       // 机器人防御增益（百分比，值为50表示50%防御增益）
    uint8_t vulnerability_buff; // 机器人负防御增益（百分比，值为30表示-30%防御增益）
    uint16_t attack_buff;       // 机器人攻击增益（百分比，值为50表示50%攻击增益）
    uint8_t remaining_energy;   // bit 0-4：机器人剩余能量值反馈，以 16 进制标识机器人剩余能量值比例，仅
                                // 在机器人剩余能量小于 50%时反馈，其余默认反馈 0x32
                                // bit 0：在剩余能量≥50%时为 1，其余情况为 0
                                // bit 1：在剩余能量≥30%时为 1，其余情况为 0
                                // bit 2：在剩余能量≥15%时为 1，其余情况为 0
                                // bit 3： 在剩余能量≥5%时为 1，其余情况为 0Bit4：在剩余能量≥1%
                                // 时为 1，其余情况为 0
} __attribute__((packed)) buff_t;

// v1.7.0串口协议不存在
// 0x0205 2 空中支援时间数据，固定以1Hz频率发送 服务器→己方空中机器人
//  typedef __packed struct
//  {
//    uint8_t airforce_status;                  //空中机器人状态（0为正在冷却，1为冷却完毕，2为正在空中支援）
//    uint8_t time_remain;                      //此状态的剩余时间（单位为：秒，向下取整，即冷却时间剩余1.9秒时，此值为1） 若冷却时间为0，但未呼叫空中支援，则该值为0
//  } air_support_data_t;

// 0x0206 1  伤害状态数据，伤害发生后发送 主控模块→对应机器人
// 受伤害情况为机器人裁判系统本地判定，即时发送，但实际是否受到对应伤害受规则条例影响，请以服务器最终判定为准。
typedef struct
{
    uint8_t armor_id : 4;            // bit 0-3：当扣血原因为装甲模块被弹丸攻击、受撞击、离线或测速模块离线时，该4 bit组成的数值为装甲模块或测速模块的ID编号；当其他原因导致扣血时，该数值为0
    uint8_t HP_deduction_reason : 4; // bit 4-7：血量变化类型 0：装甲模块被弹丸攻击导致扣血 1：裁判系统重要模块离线导致扣血 2：射击初速度超限导致扣血 3：枪口热量超限导致扣血 4：底盘功率超限导致扣血 5：装甲模块受到撞击导致扣血
} __attribute__((packed)) hurt_data_t;

// 0x0207 7  实时射击数据，弹丸发射后发送 主控模块→对应机器人
typedef struct
{
    uint8_t bullet_type;         // 弹丸类型： 1：17mm弹丸 2：42mm弹丸
    uint8_t shooter_number;      // 发射机构ID： 1：第1个17mm发射机构  2：第2个17mm发射机构 3：42mm发射机构
    uint8_t launching_frequency; // 弹丸射频（单位：Hz）
    float initial_speed;         // 弹丸初速度（单位：m/s）
} __attribute__((packed)) shoot_data_t;

// 0x0208 6 允许发弹量，固定以10Hz频率发送 服务器→己方英雄、步兵、哨兵、空中机器人
typedef struct
{
    uint16_t projectile_allowance_17mm; // 17mm弹丸允许发弹量
    uint16_t projectile_allowance_42mm; // 42mm弹丸允许发弹量
    uint16_t remaining_gold_coin;       // 剩余金币数量
} __attribute__((packed)) projectile_allowance_t;

// 0x0209 4 机器人RFID模块状态，固定以3Hz频率发送 服务器→己方装有RFID模块的机器人
// 注：基地增益点、高地增益点、飞坡增益点、前哨站增益点、资源岛增益点、补血点、兑换区、中心增益点（仅适用于RMUL）和哨兵巡逻区的RFID卡仅在赛内生效。在赛外，即使检测到对应的RFID卡，对应值也为0。
typedef struct
{
    uint32_t rfid_status; // bit位值为1/0的含义：是否已检测到该增益点RFID卡
                          // bit 0：己方基地增益点 bit 1：己方环形高地增益点 bit 2：对方环形高地增益点
                          // bit 3：己方R3/B3梯形高地增益点 bit 4：对方R3/B3梯形高地增益点
                          // bit 5：己方R4/B4梯形高地增益点 bit 6：对方R4/B4梯形高地增益点
                          // bit 7：己方能量机关激活点 bit 8：己方飞坡增益点（靠近己方一侧飞坡前）
                          // bit 9：己方飞坡增益点（靠近己方一侧飞坡后） bit 10：对方飞坡增益点（靠近对方一侧飞坡前）
                          // bit 11：对方飞坡增益点（靠近对方一侧飞坡后） bit 12：己方前哨站增益点
                          // bit 13：己方补血点（检测到任一均视为激活） bit 14：己方哨兵巡逻区
                          // bit 15：对方哨兵巡逻区 bit 16：己方大资源岛增益点 bit 17：对方大资源岛增益点
                          // bit 18：己方兑换区 bit 19：中心增益点（仅RMUL适用） bit 20-31：保留
} __attribute__((packed)) rfid_status_t;

// 0x020A 6 飞镖选手端指令数据，固定以3Hz频率发送 服务器→己方飞镖机器人
typedef struct
{
    uint8_t dart_launch_opening_status; // 当前飞镖发射站的状态： 1：关闭 2：正在开启或者关闭中 0：已经开启
    uint8_t reserved;                   // 保留位
    uint16_t target_change_time;        // 切换击打目标时的比赛剩余时间，单位：秒，无/未切换动作，默认为0。
    uint16_t latest_launch_cmd_time;    // 最后一次操作手确定发射指令时的比赛剩余时间，单位：秒，初始值为0。
} __attribute__((packed)) dart_client_cmd_t;

// 0x020B 40 地面机器人位置数据，固定以1Hz频率发送 服务器→己方哨兵机器人
typedef struct
{
    float hero_x;       // 己方英雄机器人位置x轴坐标，单位：m
    float hero_y;       // 己方英雄机器人位置y轴坐标，单位：m
    float engineer_x;   // 己方工程机器人位置x轴坐标，单位：m
    float engineer_y;   // 己方工程机器人位置y轴坐标，单位：m
    float standard_3_x; // 己方3号步兵机器人位置x轴坐标，单位：m
    float standard_3_y; // 己方3号步兵机器人位置y轴坐标，单位：m
    float standard_4_x; // 己方4号步兵机器人位置x轴坐标，单位：m
    float standard_4_y; // 己方4号步兵机器人位置y轴坐标，单位：m
    float standard_5_x; // 己方5号步兵机器人位置x轴坐标，单位：m
    float standard_5_y; // 己方5号步兵机器人位置y轴坐标，单位：m
} __attribute__((packed)) ground_robot_position_t;

// 0x020C 6 雷达标记进度数据，固定以1Hz频率发送 服务器→己方雷达机器人
// 在对应机器人被标记进度≥100 时发送 1，被标记进度<100 时发送 0。
typedef struct
{
    uint8_t mark_progress; // bit 0：对方 1 号英雄机器人易伤情况
                           // bit 1：对方 2 号工程机器人易伤情况
                           // bit 2：对方 3 号步兵机器人易伤情况
                           // bit 3：对方 4 号步兵机器人易伤情况
                           // bit 4：对方哨兵机器人易伤情况
} __attribute__((packed)) radar_mark_data_t;

// 0x020D 6 哨兵自主决策信息同步，固定以1Hz频率发送 服务器→己方哨兵机器人
typedef struct
{
    uint32_t sentry_info;   // bit 0-10：除远程兑换外，哨兵机器人成功兑换的允许发弹量，开局为 0，
                            // 在哨兵机器人成功兑换一定允许发弹量后，该值将变为哨兵机器人成功兑换的允许发弹量值。
                            // bit 11-14：哨兵机器人成功远程兑换允许发弹量的次数，开局为 0，
                            // 在哨兵机器人成功远程兑换允许发弹量后，该值将变为哨兵机器人成功远程兑换允许发弹量的次数。
                            // bit 15-18：哨兵机器人成功远程兑换血量的次数，开局为 0，在哨兵机器人成功远程兑换血量后，
                            // 该值将变为哨兵机器人成功远程兑换血量的次数。
                            // bit 19：哨兵机器人当前是否可以确认免费复活，可以确认免费复活时值为1，否则为0。
                            // bit 20：哨兵机器人当前是否可以兑换立即复活，可以兑换立即复活时值为1，否则为0。
                            // bit 21-  m       ：哨兵机器人当前若兑换立即复活需要花费的金币数。
                            // bit 31：保留。
    uint16_t sentry_info_2; // bit 0：哨兵当前是否处于脱战状态，处于脱战状态时为1，否则为0。 bit 1-11：队伍17mm允许发弹量的剩余可兑换数。 bit 12-15：保留。
} __attribute__((packed)) sentry_info_t;

// 0x020E 1 雷达自主决策信息同步，固定以1Hz频率发送 服务器→己方雷达机器人
typedef struct
{
    uint8_t radar_info; // bit 0-1：雷达是否拥有触发双倍易伤的机会，开局为0，数值为雷达拥有触发双倍易伤的机会，至多为2 bit 2：对方是否正在被触发双倍易伤 0：对方未被触发双倍易伤 1：对方正在被触发双倍易伤 bit 3-7：保留
} __attribute__((packed)) radar_info_t;

/**
 * 机器人交互数据通过常规链路发送，其数据段包含一个统一的数据段头结构。数据段头结构包括内容ID、
 * 发送者和接收者的ID、内容数据段。机器人交互数据包的总长不超过127个字节，减去frame_header、
 * cmd_id和frame_tail的9个字节以及数据段头结构的6个字节，故机器人交互数据的内容数据段最大
 * 为112个字节。
 * 每1000毫秒，英雄、工程、步兵、空中机器人、飞镖能够接收数据的上限为3720字节，雷达和哨兵机器
 * 人能够接收数据的上限为5120字节。
 * 由于存在多个内容ID，但整个cmd_id上行频率最大为30Hz，请合理安排带宽。
 */
// 0x0301 127 机器人交互数据，发送方触发发送，频率上限为30Hz
typedef struct
{
    uint16_t data_cmd_id;   // 子内容ID.需为开放的子内容ID
    uint16_t sender_id;     // 发送者ID.需与自身ID匹配，ID编号详见附录
    uint16_t receiver_id;   // 接收者ID.仅限己方通信 需为规则允许的多机通讯接收者 若接收者为选手端，则仅可发送至发送者对应的选手端 ID编号详见附录
    uint8_t user_data[122]; // 内容数据段. x最大为112
} __attribute__((packed)) robot_interaction_data_t;
/*
 *   子内容ID     内容数据段长度      功能说明
 * 0x0200~0x02FF     x≤112        机器人之间通信
 *    0x0100           2          选手端删除图层
 *    0x0101          15        选手端绘制一个图形
 *    0x0102          30        选手端绘制两个图形
 *    0x0103          75        选手端绘制五个图形
 *    0x0104         105        选手端绘制七个图形
 *    0x0110          45        选手端绘制字符图形
 *    0x0120           4         哨兵自主决策指令
 *    0x0121           1         雷达自主决策指令
 */
// 子内容ID：0x0100
typedef struct
{
    uint8_t delete_type; // 删除操作 0：空操作 1：删除图层 2：删除所有
    uint8_t layer;       // 图层数 图层数：0~9
} __attribute__((packed)) interaction_layer_delete_t;

// 子内容ID：0x0101
typedef struct
{
    uint8_t figure_name[3];    // 图形名 在图形删除、修改等操作中，作为索引
    uint32_t operate_tpye : 3; // bit 0-2：图形操作 0：空操作 1：增加 2：修改 3：删除
    uint32_t figure_tpye : 3;  // bit 3-5：图形类型 0：直线 1：矩形 2：正圆 3：椭圆 4：圆弧 5：浮点数 6：整型数 7：字符
    uint32_t layer : 4;        // bit 6-9：图层数（0~9）
    uint32_t color : 4;        // bit 10-13：颜色 0：红/蓝（己方颜色） 1：黄色 2：绿色 3：橙色 4：紫红色 5：粉色 6：青色 7：黑色  8：白色
    uint32_t details_a : 9;    // bit 14-31：根据绘制的图形不同，含义不同，详见“表 2-26 图形细节参数说明”
    uint32_t details_b : 9;    // 同上
    uint32_t width : 10;       // bit 0-9：线宽，建议字体大小与线宽比例为10：1
    uint32_t start_x : 11;     // bit 10-20：起点/圆心x坐标
    uint32_t start_y : 11;     // bit 21-31：起点/圆心y坐标
    uint32_t details_c : 10;   // 根据绘制的图形不同，含义不同，详见“表 2-26 图形细节参数说明”
    uint32_t details_d : 11;   // 同上
    uint32_t details_e : 11;   // 同上
} __attribute__((packed)) interaction_figure_t;

/**
 * 表 2-26 图形细节参数说明
 * 类型   |details_a| details_b |details_c |  details_d  | details_e
 * 直线   |    -    |     -     |    -     |   终点x坐标  |  终点y坐标
 * 矩形   |    -    |     -     |    -     | 对角顶点x坐标| 对角顶点y坐标
 * 正圆   |    -    |     -     |   半径   |       -     |      -
 * 椭圆   |    -    |     -     |    -     |   x半轴长度  |   y半轴长度
 * 圆弧   | 起始角度 |  终止角度 |    -     |    x半轴长度 |    y半轴长度
 * 浮点数 | 字体大小 |  无作用   |    该值除以1000即实际显示值
 * 整型数 | 字体大小 |    -      |     32位整型数，int32_t
 * 字符   | 字体大小 |  字符长度 |    -     |       -     |       -
 *
 * 提示：
 *  - 角度值含义为：0°指12点钟方向，顺时针绘制；
 *  - 屏幕位置：（0,0）为屏幕左下角（1920，1080）为屏幕右上角；
 *  - 浮点数：整型数均为32位，对于浮点数，实际显示的值为输入的值/1000，如在details_c、details_d、details_e对应的字节输入1234，选手端实际显示的值将为1.234。
 *  - 即使发送的数值超过对应数据类型的限制，图形仍有可能显示，但此时不保证显示的效果。
 */

// 子内容ID：0x0102
typedef struct
{
    interaction_figure_t interaction_figure[2]; // 每个大小15字节 有2个图形 与0x0101的数据段相同
} __attribute__((packed)) interaction_figure_2_t;

// 子内容ID：0x0103
typedef struct
{
    interaction_figure_t interaction_figure[5]; // 每个大小15字节 有5个图形 与0x0101的数据段相同
} __attribute__((packed)) interaction_figure_3_t;

// 子内容ID：0x0104
typedef struct
{
    interaction_figure_t interaction_figure[7]; // 每个大小15字节 有7个图形 与0x0101的数据段相同
} __attribute__((packed)) interaction_figure_4_t;

// 找不到graphic_data_struct_t结构体，故注释
// 子内容ID：0x0110
/**
 * 大小     说明          备注
 *  2    数据的内容ID    0x0110
 *  2      送者的ID   需要校验发送者的ID正确性
 *  2    接收者的ID   需要校验接收者的ID正确性，仅支持发送机器人对应的选手端
 *  15    字符配置    详见图形数据介绍
 *  30     字符       -
 */
// typedef __packed struct
// {
//   graphic_data_struct_t  grapic_data_struct;
//   uint8_t data[30];
// } ext_client_custom_character_t;

// 子内容ID：0x0120 哨兵自主决策指令
/**
 * bit 0：哨兵机器人是否确认复活
 *      0表示哨兵机器人确认不复活，即使此时哨兵的复活读条已经完成
 *      1表示哨兵机器人确认复活，若复活读条完成将立即复活
 * bit 1：哨兵机器人是否确认兑换立即复活
 *      0表示哨兵机器人确认不兑换立即复活；
 *      1表示哨兵机器人确认兑换立即复活，若此时哨兵机器人符合兑换立即复活的规则要求，则会立即消耗金币兑换立即复活
 * bit 2-12：哨兵将要兑换的发弹量值，开局为0，修改此值后，哨兵在补血点即可兑换允许发弹量。此值的变化需要单调递增，否则视为不合法。
 *        示例：此值开局仅能为0，此后哨兵可将其从0修改至X，则消耗X金币成功兑换X允许发弹量。此后哨兵可将其从X修改至X+Y，以此类推。
 * bit 13-16：哨兵远程兑换发弹量的请求次数，开局为0，修改此值即可请求远程兑换发弹量。 此值的变化需要单调递增且每次仅能增加1，否则视为不合法。
 *        示例：此值开局仅能为0，此后哨兵可将其从0修改至1，则消耗金币远程兑换允许发弹量。此后哨兵可将其从1修改至2，以此类推。
 * bit 17-20：哨兵远程兑换血量的请求次数，开局为0，修改此值即可请求远程兑换血量。 此值的变化需要单调递增且每次仅能增加1，否则视为不合法。
 *        示例：此值开局仅能为0，此后哨兵可将其从0修改至1，则消耗金币远程兑换血量。此后哨兵可将其从1修改至2，以此类推。
 *     在哨兵发送该子命令时，服务器将按照从相对低位到相对高位的原则依次处理这些指令，直至全部成功或不能处理为止。
 *        示例：若队伍金币数为0，此时哨兵战亡，“是否确认复活”的值为1，“是否确认兑换立即复活”的值为1，“确认兑换的允许发弹量值”为100。（假定之前哨兵未兑换过允许发弹量）由于此时队伍金币数不足以使哨兵兑换立即复活，则服务器将会忽视后续指令，等待哨兵发送的下一组指令。
 * bit 21-31：保留
 */
typedef struct
{
    uint32_t sentry_cmd;
} __attribute__((packed)) sentry_cmd_t;

// 子内容ID：0x0121 雷达自主决策指令
/**雷达是否确认触发双倍易伤
 * 开局为0，修改此值即可请求触发双倍易伤，若此时雷达拥有触发双倍易伤的机会，则可触发。 此值的变化需要单调递增且每次仅能增加1，否则视为不合法。
 *    示例：此值开局仅能为0，此后雷达可将其从0修改至1，若雷达拥有触发双倍易伤的机会，则触发双倍易伤。此后雷达可将其从1修改至2，以此类推。
 * 若雷达请求双倍易伤时，双倍易伤正在生效，则第二次双倍易伤将在第一次双倍易伤结束后生效。
 */
typedef struct
{
    uint8_t radar_cmd;
} __attribute__((packed)) radar_cmd_t;

//------------------小地图交互数据----------------------
// 0x0303 15 选手端小地图交互数据，选手端触发发送  选手端点击→服务器→发送方选择的己方机器人 常规链路
typedef struct
{
    float target_position_x; // 0 4 目标位置 x 轴坐标，单位 m 当发送目标机器人 ID 时，该值为 0
    float target_position_y; // 4 4 目标位置 y 轴坐标，单位 m 当发送目标机器人 ID 时，该值为 0
    uint8_t cmd_keyboard;    // 8 1 云台手按下的键盘按键通用键值 无按键按下， 则为 0
    uint8_t target_robot_id; // 9 1 对方机器人 ID 当发送坐标数据时，该值为 0
    uint16_t cmd_source;     // 10 2 信息来源 ID 信息来源的 ID， ID 对应关系详见附录
} __attribute__((packed)) map_command_t;

// 0x0305 24 选手端小地图接收雷达数据，频率上限为 5Hz 雷达→服务器→己方所有选手端 常规链路
typedef struct
{
    uint16_t hero_position_x;       // 0 2 英雄机器人x位置坐标，单位：cm
    uint16_t hero_position_y;       // 2 2 英雄机器人y位置坐标，单位：cm
    uint16_t engineer_position_x;   // 4 2 工程机器人x位置坐标，单位：cm
    uint16_t engineer_position_y;   // 6 2 工程机器人y位置坐标，单位：cm
    uint16_t infantry_3_position_x; // 8 2 3号步兵机器人x位置坐标，单位：cm
    uint16_t infantry_3_position_y; // 10 2 3号步兵机器人y位置坐标，单位：cm
    uint16_t infantry_4_position_x; // 12 2 4号步兵机器人x位置坐标，单位：cm
    uint16_t infantry_4_position_y; // 14 2 4号步兵机器人y位置坐标，单位：cm
    uint16_t infantry_5_position_x; // 16 2 5号步兵机器人x位置坐标，单位：cm
    uint16_t infantry_5_position_y; // 18 2 5号步兵机器人y位置坐标，单位：cm
    uint16_t sentry_position_x;     // 20 2 哨兵机器人x位置坐标，单位：cm
    uint16_t sentry_position_y;     // 22 2 哨兵机器人y位置坐标，单位：cm
} __attribute__((packed)) map_robot_data_t;
/*备注*/
// 当x、y超出边界时显示在对应边缘处，
// 当x、y均为0时，视为未发送此机器人坐标。

// 0x0307 103 选手端小地图接收哨兵数据，频率上限为 1Hz 哨兵/半自动控制机器人→对应操作手选手端 常规链路
typedef struct
{
    uint8_t intention;         // 0 1 1：到目标点攻击 2：到目标点防守 3：移动到目标点
    uint16_t start_position_x; // 1 2 路径起点x轴坐标，单位：dm
    uint16_t start_position_y; // 3 2 路径起点y轴坐标，单位：dm
    int8_t delta_x[49];        // 5 49 路径点x轴增量数组，单位：dm
    int8_t delta_y[49];        // 54 49 路径点y轴增量数组，单位：dm
    uint16_t sender_id;        // 103 2 发送者ID
} __attribute__((packed)) map_data_t;
/*备注*/
// start_position：小地图左下角为坐标原点，水平向右为X轴正方向，竖直向上为Y轴正方向。显示位置将按照场地尺寸与小地图尺寸等比缩放，超出边界的位置将在边界处显示
// delta：增量相较于上一个点位进行计算，共49个新点位，X与Y轴增量对应组成点位
// sender_id：需与自身ID匹配，ID编号详见附录

// 0x0308 34 选手端小地图接收机器人数据，频率上限为 3Hz 己方机器人→己方选手端 常规链路
typedef struct
{
    uint16_t sender_id;    // 0 2 发送者的ID
    uint16_t receiver_id;  // 2 2 接收者的ID
    uint8_t user_data[30]; // 4 30 字符
} __attribute__((packed)) custom_info_t;
/*备注*/
// sender_id: 需要校验发送者的ID正确性
// receiver_id: 需要校验接收者的ID正确性，仅支持发送己方选手端
// user_data: 以utf-16 格式编码发送，支持显示中文。编码发送时请注意数据的大小端问题

//--------------------图传链路数据说明---------------------------
// 0x0302 30 自定义控制器与机器人交互数据，发送方触发发送，频率上限为30Hz 自定义控制器→选手端图传连接的机器人 图传链路
typedef struct
{
    uint8_t data[4]; // 0 30 自定义数据
} __attribute__((packed)) custom_robot_data_t;

// 0x0309 30 自定义控制器接收机器人数据，频率上限为 10Hz 己方机器人→对应操作手选手端连接的自定义控制器 图传链路
typedef struct
{
    uint8_t data[4]; // 0 30 自定义数据
} __attribute__((packed)) robot_custom_data_t;

// 0x0304 12 键鼠遥控数据，固定 30Hz 频率发送 选手端→选手端图传连接的机器人 图传链路
typedef struct
{
    int16_t mouse_x;          // 0 2 鼠标x轴移动速度，负值标识向左移动
    int16_t mouse_y;          // 2 2 鼠标y轴移动速度，负值标识向下移动
    int16_t mouse_z;          // 4 2 鼠标滚轮移动速度，负值标识向后滚动
    int8_t left_button_down;  // 6 1 鼠标左键是否按下：0为未按下；1为按下
    int8_t right_button_down; // 7 1 鼠标右键是否按下：0为未按下，1为按下
    uint16_t keyboard_value;  // 8 2 键盘按键信息，每个bit对应一个按键，0为未按下，1为按下：
                              //  bit 0：W键
                              //  bit 1：S键
                              //  bit 2：A键
                              //  bit 3：D键
                              //  bit 4：Shift键
                              //  bit 5：Ctrl键
                              //  bit 6：Q键
                              //  bit 7：E键
                              //  bit 8：R键
                              //  bit 9：F键
                              //  bit 10：G键
                              //  bit 11：Z键
                              //  bit 12：X键
                              //  bit 13：C键
                              //  bit 14：V键
                              //  bit 15：B键
    uint16_t reserved_0;      // 10 2 保留位
} __attribute__((packed)) remote_control_t;

//----------------------非链路数据说明-----------------------------
// 0x0306 8 自定义控制器与选手端交互数据，发送方触发发送，频率上限为 30Hz 自定义控制器→选手端 -
typedef struct
{
    uint16_t key_value;       // 0 2 键盘键值：
                              // bit 0-7：按键1键值
                              // bit 8-15：按键2键值
    uint16_t x_position : 12; // 2 2
                              // bit 0-11：鼠标X轴像素位置
                              // bit 12-15：鼠标左键状态
    uint16_t mouse_left : 4;
    uint16_t y_position : 12; // 4 2
                              // bit 0-11：鼠标Y轴像素位置
                              // bit 12-15：鼠标右键状态
    uint16_t mouse_right : 4;
    uint16_t reserved_0; // 6 2 保留位
} __attribute__((packed)) custom_client_data_t;
/*备注*/
// key_value: 仅响应选手端开放的按键 使用通用键值，支持2键无冲，键值顺序变更不会改变按下状态，若无新的按键信息，将保持上一帧数据的按下状态
// 位置信息使用绝对像素点值（赛事客户端使用的分辨率为1920×1080，屏幕左上角为（0，0）） 鼠标按键状态1为按下，其他值为未按下，仅在出现鼠标图标后响应该信息，若无新的鼠标信息，选手端将保持上一帧数据的鼠标信息，当鼠标图标消失后该数据不再保持

extern void init_referee_struct_data(void);
extern void referee_data_solve(uint8_t *frame);

extern uint8_t get_team_color(void);
extern uint8_t get_detect_color(void);

extern void get_power_heat_data_mode(uint16_t *value, int mode);
extern uint16_t get_heat_auto(void);
extern uint16_t get_heat_limit(void);
extern void get_robot_info_id(uint8_t *value);
extern void get_robot_info_hp(uint16_t *value);
extern int get_robot_shoot_power(void);
extern void get_all_robot_hp_mode(uint16_t *value, int mode);
extern uint8_t get_game_status_progress(void);
extern void get_game_status_time(uint16_t *value);
extern void get_robot_shoot_speed(float *value);
extern uint16_t get_allowance(void);

uint8_t get_center_buff_down(void);
int get_is_center_buff_down(void);

game_state_t *get_game_status_p(void);
/**附录 ID编号说明
 *
 * 机器人ID编号如下所示：
 * 1：红方英雄机器人
 * 2：红方工程机器人
 * 3/4/5：红方步兵机器人（与机器人ID 3~5对应）
 * 6：红方空中机器人
 * 7：红方哨兵机器人
 * 8：红方飞镖
 * 9：红方雷达
 * 10：红方前哨站
 * 11：红方基地
 * 101：蓝方英雄机器人
 * 102：蓝方工程机器人
 * 103/104/105：蓝方步兵机器人（与机器人ID 3~5对应）
 * 106：蓝方空中机器人
 * 107：蓝方哨兵机器人
 * 108：蓝方飞镖
 * 109：蓝方雷达
 * 110：蓝方前哨站
 * 111：蓝方基地
 *
 * 选手端ID如下所示：
 * 0x0101：红方英雄机器人选手端
 * 0x0102：红方工程机器人选手端
 * 0x0103/0x0104/0x0105：红方步兵机器人选手端（与机器人ID 3~5对应）
 * 0x0106：红方空中机器人选手端
 * 0x016A：蓝方空中机器人选手端
 * 0x0165：蓝方英雄机器人选手端
 * 0x0166：蓝方工程机器人选手端
 * 0x0167/0x0168/0x0169：蓝方步兵机器人选手端（与机器人ID 3~5对应）
 * 0x8080：裁判系统服务器（用于哨兵和雷达自主决策指令）
 */
#endif
