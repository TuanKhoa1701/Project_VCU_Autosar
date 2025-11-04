/**********************************************************
 * @file    Pwm_Lcfg.h
 * @brief   PWM Driver Configuration Header File (AUTOSAR)
 * @details Khai báo extern biến cấu hình tổng của PWM Driver cho STM32F103.
 * @version 1.0
 * @date    2024-06-27
 * @author  Nguyễn tuấn Khoa
 **********************************************************/
#ifndef PWM_LCFG_H
#define PWM_LCFG_H
#include "Pwm.h"

/**********************************************************
 * @brief   Hàm callback cho kênh PWM Channel 0
 * @details Hàm này sẽ được gọi khi có ngắt từ kênh PWM Channel 0.
 *          Có thể dùng để bật LED, debug, hoặc các tác vụ khác.
 *          Ví dụ: Pwm_EnableNotification(0, PWM_BOTH_EDGES);
 * @note    Cần định nghĩa hàm này trong mã nguồn chính của bạn.    

************************************************************/

void Pwm_Channel0_Notification(void);
void PWM_isrHandler(TIM_TypeDef* TIMx);
/**********************************************************
 * @brief   Biến cấu hình tổng cho PWM Driver
 **********************************************************/
extern const Pwm_ConfigType PwmDriverConfig;
extern const Pwm_ChannelConfigType PwmChannelsConfig[];

#endif /* PWM_LCFG_H */
