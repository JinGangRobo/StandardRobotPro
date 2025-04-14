#ifndef ROBOMASTER_PROTOCOL_H
#define ROBOMASTER_PROTOCOL_H

#include "struct_typedef.h"

#define HEADER_SOF 0xA5
#define REF_PROTOCOL_FRAME_MAX_SIZE         128

#define REF_PROTOCOL_HEADER_SIZE            sizeof(frame_header_struct_t)
#define REF_PROTOCOL_CMD_SIZE               2
#define REF_PROTOCOL_CRC16_SIZE             2
#define REF_HEADER_CRC_LEN                  (REF_PROTOCOL_HEADER_SIZE + REF_PROTOCOL_CRC16_SIZE)
#define REF_HEADER_CRC_CMDID_LEN            (REF_PROTOCOL_HEADER_SIZE + REF_PROTOCOL_CRC16_SIZE + sizeof(uint16_t))
#define REF_HEADER_CMDID_LEN                (REF_PROTOCOL_HEADER_SIZE + sizeof(uint16_t))

#pragma pack(push, 1)

typedef enum
{
    GAME_STATE_CMD_ID                 = 0x0001,//比赛状态数据
    GAME_RESULT_CMD_ID                = 0x0002,//比赛结果数据
    GAME_ROBOT_HP_CMD_ID              = 0x0003,//机器人血量数据
    FIELD_EVENTS_CMD_ID               = 0x0101,//场地事件数据
    REFEREE_WARNING_CMD_ID            = 0x0104,//裁判警告数据
    DART_FIRE_CMD_ID                  = 0x0105,//飞镖发射相关数据
    ROBOT_STATE_CMD_ID                = 0x0201,//机器人性能体系数据
    POWER_HEAT_DATA_CMD_ID            = 0x0202,//实时底盘缓冲能量和射击热量数据
    ROBOT_POS_CMD_ID                  = 0x0203,//机器人位置数据
    BUFF_MUSK_CMD_ID                  = 0x0204,//机器人增益和底盘能量数据
    ROBOT_HURT_CMD_ID                 = 0x0206,//伤害状态数据
    SHOOT_DATA_CMD_ID                 = 0x0207,//实时射击数据
    BULLET_REMAINING_CMD_ID           = 0x0208,//允许发弹量
    RFID_CMD_ID                       = 0x0209,//机器人 RFID 模块状态
    DART_CLIENT_CMD_ID                = 0x020A,//飞镖选手端指令数据
    RADAR_DATA_CMD_ID                 = 0x020B,//雷达数据
    RADAR_MARK_DATA_CMD_ID            = 0x020C,//哨兵数据
    SENTRY_INFO_DATA_CMD_ID           = 0x020D,//哨兵自主决策信息同步
    SENTRY_MARK_INFO_DATA_CMD_ID      = 0x020E,//雷达自主决策信息同步
    RADAR_INFO_DATA_CMD_ID            = 0x0301,//机器人交互数据
    CONTROLLER_ROBOT_INTERATE_CMD_ID  = 0x0302,//自定义控制器与机器人交互数据
    PLAYER_MAP_INTERACTIVE_CMD_ID     = 0x0303,//选手端小地图交互数据
    KEYBOARD_MOUSE_REMOTE_CMD_ID      = 0x0304,//键鼠遥控数据
    PLAYER_MAP_ACCEPT_RADAR_CMD_ID    = 0x0305,//选手端小地图接收雷达数据
    CONTROLLER_PLAYER_INTERATE_CMD_ID = 0x0306,//自定义控制器与选手端交互数据
    PLAYER_MAP_ACCEPT_SENTRY_CMD_ID   = 0x0307,//选手端小地图接收哨兵数据
    PLAYER_MAP_ACCEPT_ROBOT_CMD_ID    = 0x0308,//选手端小地图接收机器人数据
    CONTROLLER_ACCEPT_ROBOT_CMD_ID    = 0x0309,//自定义控制器接收机器人数据
    IDCustomData,
}referee_cmd_id_t;

// frame_header 格式 
typedef  struct
{
  uint8_t SOF;            //数据帧起始字节，固定值为0xA5
  uint16_t data_length;   //数据帧中data的长度
  uint8_t seq;            //包序号
  uint8_t CRC8;           //帧头CRC8校验
} frame_header_struct_t;

typedef enum
{
  STEP_HEADER_SOF  = 0,
  STEP_LENGTH_LOW  = 1,
  STEP_LENGTH_HIGH = 2,
  STEP_FRAME_SEQ   = 3,
  STEP_HEADER_CRC8 = 4,
  STEP_DATA_CRC16  = 5,
} unpack_step_e;

//解包结构体
typedef struct
{
  frame_header_struct_t *p_header;                            //5-byte
  uint16_t       data_len;
  uint8_t        protocol_packet[REF_PROTOCOL_FRAME_MAX_SIZE];
  unpack_step_e  unpack_step;
  uint16_t       index;
} unpack_data_t;

#pragma pack(pop)

#endif //ROBOMASTER_PROTOCOL_H
