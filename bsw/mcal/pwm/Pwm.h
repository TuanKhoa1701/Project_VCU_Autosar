/**********************************************************
 * @file    Pwm.h
 * @brief   Pulse Width Modulation (PWM) Driver Header File
 * @details File này chứa các định nghĩa về kiểu dữ liệu và 
 *          khai báo các API của PWM Driver tuân theo chuẩn AUTOSAR.
 *          Driver này được thiết kế để điều khiển Timer PWM của STM32F103 
 *          sử dụng thư viện SPL.
 * @version 1.0
 * @date    2025-06-27
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#ifndef PWM_H
#define PWM_H
#include "stm32f10x.h"
#include "Std_Types.h"          /* Các kiểu dữ liệu chuẩn AUTOSAR */
#include "stm32f10x_tim.h"      /* Thư viện SPL: Timer PWM cho STM32F103 */

/**********************************************************
 * Định nghĩa các kiểu dữ liệu của PWM Driver
 **********************************************************/

/**********************************************************
 * @typedef Pwm_ChannelType
 * @brief   Kiểu dữ liệu cho một kênh PWM
 * @details Đây là kiểu định danh cho một kênh PWM cụ thể (0, 1, 2, ...)
 **********************************************************/
typedef uint8_t Pwm_ChannelType;

/**********************************************************
 * @typedef Pwm_PeriodType
 * @brief   Kiểu dữ liệu cho chu kỳ của PWM (tính bằng tick)
 **********************************************************/
typedef uint16_t Pwm_PeriodType;

/**********************************************************
 * @enum    Pwm_OutputStateType
 * @brief   Trạng thái đầu ra của kênh PWM (HIGH hoặc LOW)
 **********************************************************/
typedef enum {
    PWM_HIGH = 0x00,  /**< Đầu ra mức cao */
    PWM_LOW  = 0x01   /**< Đầu ra mức thấp */
} Pwm_OutputStateType;

/**********************************************************
 * @enum    Pwm_ChannelClassType
 * @brief   Kiểu kênh PWM (chu kỳ cố định, biến đổi, v.v.)
 **********************************************************/
typedef enum {
    PWM_VARIABLE_PERIOD      = 0x00,   /**< PWM period thay đổi được */
    PWM_FIXED_PERIOD         = 0x01,   /**< PWM period cố định */
    PWM_FIXED_PERIOD_SHIFTED = 0x02    /**< PWM period cố định, shifted */
} Pwm_ChannelClassType;

/********************************************************* 
 * @typedef Pwm_NotificationCbType
 * @brief   Kiểu hàm callback cho thông báo ngắt PWM
 * @details Hàm này sẽ được gọi khi có ngắt từ kênh PWM.
**********************************************************/
typedef void (*Pwm_NotificationCbType)  (void); /**< Kiểu hàm callback cho thông báo ngắt PWM */

/**********************************************************
 * @struct  Pwm_ChannelConfigType
 * @brief   Cấu trúc cấu hình cho từng kênh PWM
 **********************************************************/
typedef struct {
    TIM_TypeDef*              TIMx;             /**< Timer sử dụng (TIM1, TIM2, ...) */
    uint16_t                  Prescaler;        /**< Prescaler cho Timer (nếu cần) */
    uint8_t                   channel;          /**< Channel số (1, 2, 3, 4) */
    Pwm_ChannelClassType      classType;        /**< Loại kênh */
    Pwm_PeriodType            defaultPeriod;    /**< Chu kỳ mặc định */
    uint16_t                  defaultDutyCycle; /**< Duty Cycle mặc định (0x0000 - 0x8000) */
    Pwm_OutputStateType       polarity;         /**< Đầu ra ban đầu */
    Pwm_OutputStateType       idleState;        /**< Trạng thái khi idle */
    uint8_t                   notificationEnabled;         /**< Cờ bật thông báo ngắt (0 hoặc 1) */
    Pwm_NotificationCbType    NotificationCb;              /**< Callback notification (optional) */
} Pwm_ChannelConfigType;

/**********************************************************
 * @struct  Pwm_ConfigType
 * @brief   Cấu trúc cấu hình tổng thể cho driver PWM
 **********************************************************/
typedef struct {
    const Pwm_ChannelConfigType* Channels;    /**< Danh sách các cấu hình kênh */
    uint8_t                      NumChannels; /**< Số lượng kênh PWM */
} Pwm_ConfigType;
/********************************************************** 
 * @enum    Pwm_EdgeNotificationType
 * @brief   Loại cạnh để thông báo ngắt PWM
 * @details Các loại cạnh có thể thông báo ngắt:
 *          - PWM_RISING_EDGE: Thông báo khi có cạnh lên
 *         - PWM_FALLING_EDGE: Thông báo khi có cạnh xuống
 *         - PWM_BOTH_EDGES: Thông báo khi có cả hai cạnh
***********************************************************/
/**********************************************************
 * @enum    Pwm_EdgeNotificationType
 * @brief   Loại cạnh để thông báo ngắt PWM
 **********************************************************/
typedef  enum{
    PWM_RISING_EDGE = 0x01,
    PWM_FALLING_EDGE = 0x02,
    PWM_BOTH_EDGES = PWM_FALLING_EDGE | PWM_RISING_EDGE
} PWM_EdgeNotificationType;
/**********************************************************
 * Khai báo các API của PWM Driver (chuẩn AUTOSAR)
 **********************************************************/

/**********************************************************
 * @brief   Khởi tạo PWM driver với cấu hình chỉ định
 * @param   ConfigPtr: Con trỏ tới cấu hình PWM
 **********************************************************/
void Pwm_Init(const Pwm_ConfigType* ConfigPtr);

/**********************************************************
 * @brief   Giải phóng tài nguyên và tắt tất cả kênh PWM
 **********************************************************/
void Pwm_DeInit(void);

/**********************************************************
 * @brief   Cài đặt duty cycle cho kênh PWM
 * @param   ChannelNumber: Số thứ tự kênh PWM
 * @param   DutyCycle: Tỷ lệ (0x0000 - 0x8000, ứng với 0%-100%)
 **********************************************************/
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, uint16_t DutyCycle);

/**********************************************************
 * @brief   Đặt period và duty cycle cho kênh PWM (nếu hỗ trợ)
 * @param   ChannelNumber: Số thứ tự kênh PWM
 * @param   Period: Chu kỳ PWM (tính bằng tick timer)
 * @param   DutyCycle: Duty cycle (0x0000 - 0x8000)
 **********************************************************/
void Pwm_SetPeriodAndDuty(Pwm_ChannelType ChannelNumber, Pwm_PeriodType Period, uint16_t DutyCycle);

/**********************************************************
 * @brief   Đưa kênh PWM về trạng thái idle
 * @param   ChannelNumber: Số thứ tự kênh PWM
 **********************************************************/
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber);

/**********************************************************
 * @brief   Đọc trạng thái đầu ra hiện tại của kênh PWM
 * @param   ChannelNumber: Số thứ tự kênh PWM
 * @return  PWM_HIGH hoặc PWM_LOW
 **********************************************************/
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType ChannelNumber);

/**********************************************************
 * @brief   Tắt thông báo ngắt cho kênh PWM
 * @param   ChannelNumber: Số thứ tự kênh PWM
 **********************************************************/
void Pwm_DisableNotification(Pwm_ChannelType ChannelNumber);

/**********************************************************
 * @brief   Bật thông báo ngắt cạnh lên/xuống/cả 2 cho kênh PWM
 * @param[in]   ChannelNumber: Số thứ tự kênh PWM
 * @param[in]   Notification:  Loại cạnh cần thông báo
 **********************************************************/
void Pwm_EnableNotification(Pwm_ChannelType ChannelNumber, PWM_EdgeNotificationType Notification);

/**********************************************************
 * @brief   Lấy thông tin phiên bản của driver PWM
 * @param   versioninfo: Con trỏ tới cấu trúc Std_VersionInfoType để nhận thông tin phiên bản
 **********************************************************/
void Pwm_GetVersionInfo(Std_VersionInfoType* versioninfo);
#endif /* PWM_H */
