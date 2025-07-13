/**
  ****************************(C) COPYRIGHT 2024 Polarbear*************************
  * @file       usb_task.c/h
  * @brief      通过USB串口与上位机通信
  * @note
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Jun-24-2024     Penguin         1. done

  @verbatim
  =================================================================================

  =================================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2024 Polarbear*************************
*/

#include "usb_task.h"

#include <stdbool.h>
#include <string.h>

#include "CRC8_CRC16.h"
#include "cmsis_os.h"
#include "data_exchange.h"
#include "macro_typedef.h"
#include "usb_debug.h"
#include "usb_device.h"
#include "usb_typdef.h"
#include "usbd_cdc_if.h"
#include "usbd_conf.h"
#include "supervisory_computer_cmd.h"
#include "gimbal.h"

#if INCLUDE_uxTaskGetStackHighWaterMark
uint32_t usb_high_water;
#endif

#define USB_TASK_CONTROL_TIME 1 // ms

#define USB_OFFLINE_THRESHOLD 100 // ms
#define USB_CONNECT_CNT 10

#define SEND_DURATION_RobotStateInfo 10 // ms

#define USB_RX_DATA_SIZE 256 // byte
#define USB_RECEIVE_LEN 150  // byte
#define HEADER_SIZE 4        // byte

#define CheckDurationAndSend(send_name)                                                \
    do                                                                                 \
    {                                                                                  \
        if ((HAL_GetTick() - LAST_SEND_TIME.##send_name) >= SEND_DURATION_##send_name) \
        {                                                                              \
            LAST_SEND_TIME.##send_name = HAL_GetTick();                                \
            UsbSend##send_name##Data();                                                \
        }                                                                              \
    } while (0)

// Variable Declarations
static uint8_t USB_RX_BUF[USB_RX_DATA_SIZE];

static const Imu_t *IMU;

// 判断USB连接状态用到的一些变量
static bool USB_OFFLINE = true;
static uint32_t RECEIVE_TIME = 0;
static uint32_t CONTINUE_RECEIVE_CNT = 0;

// 数据发送结构体
static SendDataRobotStateInfo_s SEND_DATA_ROBOT_STATE_INFO;

// 数据接收结构体
static ReceiveDataRobotCmd_s RECEIVE_ROBOT_CMD_DATA;

// 发送数据间隔时间
typedef struct
{
    uint32_t RobotStateInfo;
} LastSendTime_t;

static LastSendTime_t LAST_SEND_TIME;

/*******************************************************************************/
/* Main Function                                                               */
/*******************************************************************************/

static void UsbSendData(void);
static void UsbReceiveData(void);
static void UsbInit(void);

/*******************************************************************************/
/* Send Function                                                               */
/*******************************************************************************/

static void UsbSendRobotStateInfoData(void);

/******************************************************************/
/* Task                                                           */
/******************************************************************/

/**
 * @brief      USB任务主函数
 * @param[in]  argument: 任务参数
 * @retval     None
 */
void usb_task(void const *argument)
{
    Publish(&USB_OFFLINE, USB_OFFLINE_NAME);

    MX_USB_DEVICE_Init();
    
    vTaskDelay(10); // 等待USB设备初始化完成
    UsbInit();

    while (1)
    {
        UsbSendData();
        UsbReceiveData();

        if (HAL_GetTick() - RECEIVE_TIME > USB_OFFLINE_THRESHOLD)
        {
            USB_OFFLINE = true;
            CONTINUE_RECEIVE_CNT = 0;
        }
        else if (CONTINUE_RECEIVE_CNT > USB_CONNECT_CNT)
        {
            USB_OFFLINE = false;
        }
        else
        {
            CONTINUE_RECEIVE_CNT++;
        }

        vTaskDelay(USB_TASK_CONTROL_TIME);

#if INCLUDE_uxTaskGetStackHighWaterMark
        usb_high_water = uxTaskGetStackHighWaterMark(NULL);
#endif
    }
}

/*******************************************************************************/
/* Main Function                                                               */
/*******************************************************************************/

/**
 * @brief      USB初始化
 * @param      None
 * @retval     None
 */
static void UsbInit(void)
{
    // 订阅数据
    IMU = Subscribe(IMU_NAME); // 获取IMU数据指针

    // 数据置零
    memset(&LAST_SEND_TIME, 0, sizeof(LastSendTime_t));
    memset(&RECEIVE_ROBOT_CMD_DATA, 0, sizeof(ReceiveDataRobotCmd_s));

    /*******************************************************************************/
    /* Serial                                                                     */
    /*******************************************************************************/

    // 1.初始化机器人信息数据包
    // 帧头部分
    SEND_DATA_ROBOT_STATE_INFO.frame_header.sof = PACKET_VERSION;
    SEND_DATA_ROBOT_STATE_INFO.frame_header.len = (uint8_t)(sizeof(SendDataRobotStateInfo_s) - 6);
    SEND_DATA_ROBOT_STATE_INFO.frame_header.id = ROBOT_STATE_DATA_INFO_SEND_ID;
    append_CRC8_check_sum( // 添加帧头 CRC8 校验位
        (uint8_t *)(&SEND_DATA_ROBOT_STATE_INFO.frame_header), sizeof(SEND_DATA_ROBOT_STATE_INFO.frame_header));
    // 数据部分
    SEND_DATA_ROBOT_STATE_INFO.data.roll = 0;
    SEND_DATA_ROBOT_STATE_INFO.data.pitch = 0;
    SEND_DATA_ROBOT_STATE_INFO.data.yaw = 0;
    SEND_DATA_ROBOT_STATE_INFO.data.encoder_down = 0;
    SEND_DATA_ROBOT_STATE_INFO.data.encoder_up = 0;
    SEND_DATA_ROBOT_STATE_INFO.data.is_super_cap_work = 0;
    SEND_DATA_ROBOT_STATE_INFO.data.super_cap_voltage = 0;
}

/**
 * @brief      用USB发送数据
 * @param      None
 * @retval     None
 */
static void UsbSendData(void)
{
    // 发送RobotStateInfo数据
    CheckDurationAndSend(RobotStateInfo);
}

/**
 * @brief      USB接收数据
 * @param      None
 * @retval     None
 */
static void UsbReceiveData(void)
{
    static uint32_t len = USB_RECEIVE_LEN;
    static uint8_t *rx_data_start_address = USB_RX_BUF; // 接收数据包时存放于缓存区的起始位置
    static uint8_t *rx_data_end_address;                // 接收数据包时存放于缓存区的结束位置
    uint8_t *sof_address = USB_RX_BUF;

    // 计算数据包的结束位置
    rx_data_end_address = rx_data_start_address + USB_RECEIVE_LEN;
    // 读取数据
    USB_Receive(rx_data_start_address, &len); // Read data into the buffer

    while (sof_address <= rx_data_end_address)
    { // 解析缓冲区中的所有数据包
        // 寻找帧头位置
        while (*(sof_address) != PACKET_VERSION && (sof_address <= rx_data_end_address))
        {
            sof_address++;
        }
        // 判断是否超出接收数据范围
        if (sof_address > rx_data_end_address)
        {
            break; // 退出循环
        }

        if (*(sof_address) == PACKET_VERSION)
        {
            // 检查CRC8校验
            bool crc8_ok = verify_CRC8_check_sum(sof_address, HEADER_SIZE);
            if (crc8_ok)
            {
                uint8_t data_len = sof_address[1];
                uint8_t data_id = sof_address[2];
                // 检查整包CRC16校验 4: header size, 2: crc16 size
                bool crc16_ok = verify_CRC16_check_sum(sof_address, 4 + data_len + 2);
                if (crc16_ok)
                {
                    switch (data_id)
                    {
                    case ROBOT_CMD_DATA_RECEIVE_ID:
                    {
                        memcpy(&RECEIVE_ROBOT_CMD_DATA, sof_address, sizeof(ReceiveDataRobotCmd_s));
                    }
                    break;
                    default:
                        break;
                    }
                }
                sof_address += (data_len + HEADER_SIZE + 2);
            }
        }
        else
        {
            sof_address++;
        }
    }
    // 更新下一次接收数据的起始位置
    if (sof_address > rx_data_start_address + USB_RECEIVE_LEN)
    {
        // 缓冲区中没有剩余数据，下次接收数据的起始位置为缓冲区的起始位置
        rx_data_start_address = USB_RX_BUF;
    }
    else
    {
        uint16_t remaining_data_len = USB_RECEIVE_LEN - (sof_address - rx_data_start_address);
        // 缓冲区中有剩余数据，下次接收数据的起始位置为缓冲区中剩余数据的起始位置
        rx_data_start_address = USB_RX_BUF + remaining_data_len;
        // 将剩余数据移到缓冲区的起始位置
        memcpy(USB_RX_BUF, sof_address, remaining_data_len);
    }
}

/*******************************************************************************/
/* Send Function                                                               */
/*******************************************************************************/

/**
 * @brief 发送机器人信息数据
 * @param duration 发送周期
 */
static void UsbSendRobotStateInfoData(void)
{
    SEND_DATA_ROBOT_STATE_INFO.data.roll = IMU->roll;
    SEND_DATA_ROBOT_STATE_INFO.data.pitch = IMU->pitch;
    SEND_DATA_ROBOT_STATE_INFO.data.yaw = IMU->yaw;

    SEND_DATA_ROBOT_STATE_INFO.data.is_super_cap_work = 0;
    SEND_DATA_ROBOT_STATE_INFO.data.super_cap_voltage = 0;
    SEND_DATA_ROBOT_STATE_INFO.data.encoder_up = 0;
    SEND_DATA_ROBOT_STATE_INFO.data.encoder_down = 0;

    append_CRC16_check_sum((uint8_t *)&SEND_DATA_ROBOT_STATE_INFO, sizeof(SendDataRobotStateInfo_s));
    USB_Transmit((uint8_t *)&SEND_DATA_ROBOT_STATE_INFO, sizeof(SendDataRobotStateInfo_s));
}

/*------------------------------ End of File ------------------------------*/
