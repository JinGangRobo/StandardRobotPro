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

  #include "referee.h"
  #include "string.h"
  #include "stdio.h"
  #include "CRC8_CRC16.h"
  #include "protocol.h"
  
  #define max(a,b) ((a) > (b) ? (a) : (b))
  
  frame_header_struct_t referee_receive_header;               //接收帧头
  
  game_state_t game_state;                                    //比赛状态
  game_result_t game_result;                                  //比赛结果   
  game_robot_HP_t game_robot_HP;                              //机器人血量
  event_data_t field_event;                                   //场地事件
  referee_warning_t referee_warning;                          //裁判系统警告 
  dart_info_t dart_info;                                      //飞镖信息
  robot_status_t robot_state;                                 //机器人状态    
  power_heat_data_t power_heat_data;                          //机器人功率
  robot_pos_t robot_pos;                                      //机器人位置
  buff_t buff;                                                //增益数据
  hurt_data_t robot_hurt;                                     //机器人伤害
  shoot_data_t shoot_data;                                    //发射数据
  projectile_allowance_t projectile_allowance;                //弹药余量
  rfid_status_t rfid_status;                                  //RFID状态
  dart_client_cmd_t dart_client_cmd;                          //飞镖客户端命令
  ground_robot_position_t ground_robot_position;              //地面机器人位置
  radar_mark_data_t radar_mark_data;                          //雷达标记数据
  sentry_info_t sentry_info;                                  //哨兵自主决策信息同步
  radar_info_t radar_info;                                    //雷达自主决策信息同步
  robot_interaction_data_t student_interactive_data_t;        //机器人交互数据
  
  
  /**
   * 初始化裁判结构体数据
   * 
   * 本函数负责将与裁判系统相关的所有结构体数据初始化为零
   * 这是为了确保程序在运行开始时，所有与比赛相关的数据结构都处于一个已知的初始状态
   * 防止未初始化的数据导致的不确定行为或错误
   */
  void init_referee_struct_data(void)
  {
      // 初始化接收帧头结构体
      memset(&referee_receive_header, 0, sizeof(frame_header_struct_t));
  
      // 初始化游戏状态结构体
      memset(&game_state, 0, sizeof(game_state_t));

      // 初始化比赛结果结构体
      memset(&game_result, 0, sizeof(game_result_t));
  
      // 初始化游戏机器人生命值结构体
      memset(&game_robot_HP, 0, sizeof(game_robot_HP_t));
  
      // 初始化场地事件结构体
      memset(&field_event, 0, sizeof(event_data_t));

      // 初始化裁判系统警告结构体
      memset(&referee_warning, 0, sizeof(referee_warning_t));

      // 初始化飞镖信息结构体
      memset(&dart_info, 0, sizeof(dart_info_t));
  
      // 初始化机器人状态结构体
      memset(&robot_state, 0, sizeof(robot_status_t));

      // 初始化电源热量数据结构体
      memset(&power_heat_data, 0, sizeof(power_heat_data_t));

      // 初始化机器人位置结构体
      memset(&robot_pos, 0, sizeof(robot_pos_t));

      // 初始化增益数据结构体
      memset(&buff, 0, sizeof(buff_t));
  
      // 初始化机器人伤害结构体
      memset(&robot_hurt, 0, sizeof(hurt_data_t));

      // 初始化射击数据结构体
      memset(&shoot_data, 0, sizeof(shoot_data_t));
  
      // 初始化弹药剩余数据结构体
      memset(&projectile_allowance, 0, sizeof(projectile_allowance_t));

      //RFID初始化
      memset(&rfid_status, 0, sizeof(rfid_status_t));

      // 初始化飞镖客户端命令结构体
      memset(&dart_client_cmd, 0, sizeof(dart_client_cmd_t));

      // 初始化地面机器人位置结构体
      memset(&ground_robot_position, 0, sizeof(ground_robot_position_t));

      // 初始化雷达标记数据结构体
      memset(&radar_mark_data, 0, sizeof(radar_mark_data_t));

      // 初始化哨兵自主决策信息同步结构体
      memset(&sentry_info, 0, sizeof(sentry_info_t));

      // 初始化雷达自主决策信息同步结构体
      memset(&radar_info, 0, sizeof(radar_info_t));

      // 初始化学生互动数据结构体
      memset(&student_interactive_data_t, 0, sizeof(robot_interaction_data_t));
      
  }
  
  /**
   * 解析 referee 数据帧
   * 
   * @param frame 指向接收到的数据帧的指针
   * 
   * 此函数根据数据帧中的 cmd_id 解析数据帧中的信息，并将其存储在相应的结构体中
   * 它首先复制帧头和 cmd_id，然后根据 cmd_id 的值执行相应的 case 语句，
   * 将数据帧中相应部分的数据复制到全局变量中
   */
  void referee_data_solve(uint8_t *frame)
  {
      // 命令ID，用于识别数据帧的类型
      uint16_t cmd_id = 0;
  
      // 用于跟踪当前数据帧解析的索引位置
      uint8_t index = 0;
  
      // 复制帧头信息到全局变量
      memcpy(&referee_receive_header, frame, sizeof(frame_header_struct_t));
  
      // 更新索引位置，以便解析 cmd_id
      index += sizeof(frame_header_struct_t);
  
      // 从数据帧中复制 cmd_id
      memcpy(&cmd_id, frame + index, sizeof(uint16_t));
      // 更新索引位置，以便解析后续数据
      index += sizeof(uint16_t);
  
      // 根据 cmd_id 解析数据帧中的数据
      switch (cmd_id)
      {
          case GAME_STATE_CMD_ID:
          {
              // 解析并存储游戏状态信息
              memcpy(&game_state, frame + index, sizeof(game_state_t));
          }
          break;
          case GAME_RESULT_CMD_ID:
          {
              // 解析并存储比赛结果信息
              memcpy(&game_result, frame + index, sizeof(game_result_t));
          }
          case GAME_ROBOT_HP_CMD_ID:
          {
              // 解析并存储机器人血量信息
              memcpy(&game_robot_HP, frame + index, sizeof(game_robot_HP_t));
          }
          break;
          case FIELD_EVENTS_CMD_ID:
          {
              // 解析并存储场地事件信息
              memcpy(&field_event, frame + index, sizeof(event_data_t));
          }
          break;
          case REFEREE_WARNING_CMD_ID:
          {
              // 解析并存储裁判系统警告信息
              memcpy(&referee_warning, frame + index, sizeof(referee_warning_t));
          }
          break;
          case DART_FIRE_CMD_ID:
          {
              // 解析并存储飞镖信息
              memcpy(&dart_info, frame + index, sizeof(dart_info_t));
          }
          break;
          case ROBOT_STATE_CMD_ID:
          {
              // 解析并存储机器人状态信息
              memcpy(&robot_state, frame + index, sizeof(robot_status_t));
          }
          break;
          case POWER_HEAT_DATA_CMD_ID:
          {
              // 解析并存储电量热量数据信息
              memcpy(&power_heat_data, frame + index, sizeof(power_heat_data_t));
          }
          break;
          case ROBOT_POS_CMD_ID:
          {
              // 解析并存储机器人位置信息
              memcpy(&robot_pos, frame + index, sizeof(robot_pos_t));
          }
          break;
          case BUFF_MUSK_CMD_ID:
          {
              // 解析并存储增益数据信息
              memcpy(&buff, frame + index, sizeof(buff_t));
          }
          break;
          case ROBOT_HURT_CMD_ID:
          {
              // 解析并存储机器人伤害信息
              memcpy(&robot_hurt, frame + index, sizeof(hurt_data_t));
          }
          break;
          case SHOOT_DATA_CMD_ID:
          {
              // 解析并存储射击数据信息
              memcpy(&shoot_data, frame + index, sizeof(shoot_data_t));
          }
          break;
          case BULLET_REMAINING_CMD_ID:
          {
              memcpy(&projectile_allowance, frame + index, sizeof(projectile_allowance_t));
          }
          break;
          case RFID_CMD_ID:
          {
              memcpy(&rfid_status, frame + index, sizeof(rfid_status_t));
          }
          break;
          case DART_CLIENT_CMD_ID:
          {
              // 解析并存储飞镖客户端命令信息
              memcpy(&dart_client_cmd, frame + index, sizeof(dart_client_cmd_t));
          }
          break;
          case RADAR_DATA_CMD_ID:
          {
              // 解析并存储雷达标记数据信息
              memcpy(&ground_robot_position, frame + index, sizeof(ground_robot_position_t));
          }
          break;
          case RADAR_MARK_DATA_CMD_ID:
          {
              // 解析并存储哨兵数据
              memcpy(&radar_mark_data, frame + index, sizeof(radar_mark_data_t));
          }
          break;
          case SENTRY_INFO_DATA_CMD_ID:
          {
              // 解析并存储哨兵自主决策信息
              memcpy(&sentry_info, frame + index, sizeof(sentry_info_t));
          }
          break;
          case SENTRY_MARK_INFO_DATA_CMD_ID:
          {
              // 解析并存储雷达自主决策信息
              memcpy(&radar_info, frame + index, sizeof(radar_info_t));
          }
          break;
          case RADAR_INFO_DATA_CMD_ID:
          {
              // 解析并存储学生互动数据信息
              memcpy(&student_interactive_data_t, frame + index, sizeof(robot_interaction_data_t));
          }
          break;
          default:
          {
              // 默认情况下不执行任何操作
              break;
          }
      }
  }
  
  
  /**
   * @brief 反馈自身机器人颜色
   * @return 0为红色,1为蓝色,2为未知
   */
  uint8_t get_team_color(void)
  {
      switch (robot_state.robot_id) {
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7:
          case 8:
          case 9:
          case 10:
          case 11:
              return 0;
          case 101:
          case 102:
          case 103:
          case 104:
          case 105:
          case 106:
          case 107:
          case 108:
          case 109:
          case 110:
          case 111:
              return 1;
          default:
              return 2;
      }
  }
  
  /**
   * 获取需要检测到的颜色
   * @return uint8_t 返回需要检测到的颜色值
   */
  uint8_t get_detect_color(void) {
      uint8_t detect_color = get_team_color();
      return (detect_color != 2) ? !detect_color : detect_color;
  }
  
  /**
   * 根据模式获取对应的热数据
   * 
   * @param value 指向存储数据的指针，通过它返回对应的热值
   * @param mode 指定的模式，决定获取哪种口径射击器的热值
   * 
   * 此函数根据提供的模式参数，将对应的射击器加热值赋给*value
   * 它支持三种不同口径射击器的加热数据获取，分别是17毫米单管、17毫米双管和42毫米单管
   */
  void get_robot_info_id(__packed uint8_t* value){
      *value = robot_state.robot_id;
  }
  void get_robot_info_hp(__packed uint16_t* value){
      *value = robot_state.current_HP;
  }
  
  int get_robot_shoot_power(void){
      return robot_state.power_management_shooter_output;
  }
  
  uint16_t get_allowance(void){
      return projectile_allowance.projectile_allowance_17mm;
  }
  
  /**
   * @brief 获取特定模式下的炮管热量数据
   * @param value 指向存储热量数据的指针，数据为`__packed uint16_t`类型。
   * @param mode 模式参数，决定获取哪种炮管的热量数据：
   *             1 - 获取17mm炮管1的热量数据
   *             2 - 获取17mm炮管2的热量数据
   *             3 - 获取42mm炮管的热量数据
   *             4 - 获取17mm炮管1和炮管2中较大的热量数据
   */
  void get_power_heat_data_mode(__packed uint16_t* value, int mode){
      if(mode == 1) *value = power_heat_data.shooter_17mm_1_barrel_heat;
      else if(mode == 2) *value = power_heat_data.shooter_17mm_2_barrel_heat;
      else if(mode == 3) *value = power_heat_data.shooter_42mm_barrel_heat;
      else if(mode == 4) *value = max(power_heat_data.shooter_17mm_1_barrel_heat, power_heat_data.shooter_17mm_2_barrel_heat);
      else if(mode == 5) {
          *value = max(power_heat_data.shooter_17mm_1_barrel_heat, power_heat_data.shooter_17mm_2_barrel_heat);
          *value = max(*value, power_heat_data.shooter_42mm_barrel_heat);
      }
  }
  
  uint16_t get_heat_auto(void){
      uint16_t heat_max = max(power_heat_data.shooter_17mm_1_barrel_heat, power_heat_data.shooter_17mm_2_barrel_heat);
      return max(heat_max, power_heat_data.shooter_42mm_barrel_heat);
  }
  
  uint16_t get_heat_limit(void){
      return robot_state.shooter_barrel_heat_limit;
  }
  
  void get_all_robot_hp_mode(__packed uint16_t* value, int mode){
      if(mode == 1) *value = game_robot_HP.red_1_robot_HP;
      else if(mode == 2) *value = game_robot_HP.red_2_robot_HP;
      else if(mode == 3) *value = game_robot_HP.red_3_robot_HP;
      else if(mode == 4) *value = game_robot_HP.red_4_robot_HP;
      else if(mode == 7) *value = game_robot_HP.red_7_robot_HP;
      else if(mode == 8) *value = game_robot_HP.red_base_HP;
      else if(mode == 9) *value = game_robot_HP.red_outpost_HP;
      else if(mode == 11) *value = game_robot_HP.blue_1_robot_HP;
      else if(mode == 12) *value = game_robot_HP.blue_2_robot_HP;
      else if(mode == 13) *value = game_robot_HP.blue_3_robot_HP;
      else if(mode == 14) *value = game_robot_HP.blue_4_robot_HP;
      else if(mode == 17) *value = game_robot_HP.blue_7_robot_HP;
      else if(mode == 18) *value = game_robot_HP.blue_base_HP;
      else if(mode == 19) *value = game_robot_HP.blue_outpost_HP;
  
  }
  
  uint8_t get_game_status_progress(void){
      return game_state.game_progress;
  }
  
  void get_game_status_time(__packed uint16_t* value){
      *value = game_state.stage_remain_time;
  }
  
  void get_robot_shoot_speed(__packed float* value){
      *value = shoot_data.initial_speed;
  }
  
  int get_is_center_buff_down(void){
      return (field_event.event_type) ? 1 : 0;
  }
  
  int get_is_center_rfid(void){
      // 创建掩码并读取第 n 位
      uint32_t mask = 1 << 19;
      return (rfid_status.rfid_status & mask) ? 1 : 0;
  }
  
  uint8_t get_center_buff_down(void){
      if(get_is_center_rfid())return 4;
      return field_event.event_type;
  }
  
  game_state_t* get_game_status_p(void){
      return &game_state;
  }