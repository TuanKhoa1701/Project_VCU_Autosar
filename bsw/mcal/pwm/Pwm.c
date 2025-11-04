/**********************************************************
 * @file    Pwm.c
 * @brief   Pulse Width Modulation (PWM) Driver Source File
 * @details Hiện thực các API của PWM Driver chuẩn AUTOSAR cho STM32F103, sử dụng SPL.
 *          Quản lý chức năng PWM, không cấu hình chân GPIO.
 * 
 * @version 1.0
 * @date    2025-06-27
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "Pwm.h"
#include <stddef.h>
#include "Pwm_Lcfg.h" // Cấu hình kênh PWM
#include "misc.h"
/* ===============================
 *     Static Variables & Defines
 * =============================== */

/* Lưu trữ cấu hình driver PWM hiện tại */
static const Pwm_ConfigType* Pwm_CurrentConfigPtr = NULL;

/* Trạng thái đã khởi tạo của driver PWM */
static uint8_t PWM_ISINITIALISED = 0;

/* ===============================
 *        Function Definitions
 * =============================== */

/**********************************************************
 * @brief   Khởi tạo PWM driver với cấu hình chỉ định
 * @details Khởi tạo tất cả timer/kênh PWM theo cấu hình. 
 *          Phần cấu hình chân GPIO phải thực hiện riêng.
 *
 * @param[in] ConfigPtr Con trỏ tới cấu hình PWM
 **********************************************************/
void Pwm_Init(const Pwm_ConfigType* ConfigPtr)
{
    if (ConfigPtr == NULL) return;

    Pwm_CurrentConfigPtr = ConfigPtr;
    for (uint8_t i = 0; i < ConfigPtr->NumChannels; i++)
    {
        const Pwm_ChannelConfigType* channelConfig = &ConfigPtr->Channels[i];

   /*bật clock cho timer*/
        if(channelConfig -> TIMx == TIM2) {
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
        } else if(channelConfig -> TIMx == TIM3) {
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
        } else if(channelConfig -> TIMx == TIM4) {
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
        } else if(channelConfig -> TIMx == TIM1) {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
        }

        /* Cấu hình timer */
    
        TIM_TimeBaseInitTypeDef TIM_InitStructure;
        TIM_InitStructure.TIM_Prescaler = channelConfig->Prescaler; // Prescaler
        TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up; // Chế độ đếm lên
        TIM_InitStructure.TIM_Period = channelConfig->defaultPeriod; // Chu kỳ (ARR)
        TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // Phân chia xung đồng hồ
        TIM_InitStructure.TIM_RepetitionCounter = 0; // Chỉ cần 0 cho chế độ PWM
        TIM_TimeBaseInit(channelConfig->TIMx, &TIM_InitStructure); // Khởi tạo timer

        /*Cấu hình chế độ PWM*/
        TIM_OCInitTypeDef TIM_OCInitStructure;
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; // Chế độ PWM1
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // Bật trạng thái đầu ra
        TIM_OCInitStructure.TIM_Pulse = ((uint32_t)channelConfig->defaultPeriod * channelConfig->defaultDutyCycle) >> 15; // Giá trị so
        TIM_OCInitStructure.TIM_OCPolarity = (channelConfig->polarity == PWM_HIGH) ? TIM_OCPolarity_High : TIM_OCPolarity_Low; // Chế độ phân cực
        TIM_OCInitStructure.TIM_OCIdleState = (channelConfig->idleState == PWM_HIGH) ? TIM_OCIdleState_Set : TIM_OCIdleState_Reset; // Trạng thái idle
        switch (channelConfig->channel) {
        case 1:
            TIM_OC1Init(channelConfig->TIMx, &TIM_OCInitStructure);
            TIM_OC1PreloadConfig(channelConfig->TIMx, TIM_OCPreload_Enable); // Bật preload cho CCR1
            break;
        case 2:
            TIM_OC2Init(channelConfig->TIMx, &TIM_OCInitStructure);
            TIM_OC2PreloadConfig(channelConfig->TIMx, TIM_OCPreload_Enable);        // Bật preload cho CCR2    
            break;
        case 3:
            TIM_OC3Init(channelConfig->TIMx, &TIM_OCInitStructure);
            TIM_OC3PreloadConfig(channelConfig->TIMx, TIM_OCPreload_Enable);
            break;
        case 4:
            TIM_OC4Init(channelConfig->TIMx, &TIM_OCInitStructure);
            TIM_OC4PreloadConfig(channelConfig->TIMx, TIM_OCPreload_Enable);
            break;
        default:
            /* Không hỗ trợ channel khác */
            continue;
        }
        /* Bật timer */
        TIM_ARRPreloadConfig(channelConfig->TIMx, ENABLE); // Bật preload cho ARR
        TIM_Cmd(channelConfig->TIMx, ENABLE);
        
        /* Nếu là TIM1 (advanced), enable main output */
        if (channelConfig->TIMx == TIM1) {
            TIM_CtrlPWMOutputs(TIM1, ENABLE);
        }
    }
    PWM_ISINITIALISED = 1;
}

/**********************************************************
 * @brief   Dừng tất cả kênh PWM và giải phóng tài nguyên
 **********************************************************/
void Pwm_DeInit(void)
{
    if (!PWM_ISINITIALISED) return;
    for (uint8_t i = 0; i < Pwm_CurrentConfigPtr->NumChannels; i++)
    {
        const Pwm_ChannelConfigType* channelConfig = &Pwm_CurrentConfigPtr->Channels[i];
        TIM_Cmd(channelConfig->TIMx, DISABLE);
        if (channelConfig->TIMx == TIM1) {
            TIM_CtrlPWMOutputs(TIM1, DISABLE);
        }
    }
    PWM_ISINITIALISED = 0;
}

/**********************************************************
 * @brief   Đặt duty cycle cho một kênh PWM
 * @details Chỉ thay đổi duty cycle, không thay đổi period.
 *
 * @param[in] ChannelNumber Số thứ tự kênh PWM
 * @param[in] DutyCycle     Duty cycle mới (0x0000 - 0x8000)
 **********************************************************/
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, uint16_t DutyCycle)
{
    if (!PWM_ISINITIALISED || ChannelNumber >= Pwm_CurrentConfigPtr->NumChannels) return;
    const Pwm_ChannelConfigType* channelConfig = &Pwm_CurrentConfigPtr->Channels[ChannelNumber];
    uint16_t period = channelConfig->TIMx->ARR;
    uint16_t compareValue = ((uint32_t)period * DutyCycle) >> 15;
    switch (channelConfig->channel) {
    case 1: channelConfig->TIMx->CCR1 = compareValue; break;
    case 2: channelConfig->TIMx->CCR2 = compareValue; break;
    case 3: channelConfig->TIMx->CCR3 = compareValue; break;
    case 4: channelConfig->TIMx->CCR4 = compareValue; break;
    default: break;
    }
}

/**********************************************************
 * @brief   Đặt period và duty cycle cho một kênh PWM (nếu hỗ trợ)
 * @details Thay đổi đồng thời period (ARR) và duty cycle (CCR).
 *
 * @param[in] ChannelNumber Số thứ tự kênh PWM
 * @param[in] Period        Chu kỳ PWM mới (tick timer)
 * @param[in] DutyCycle     Duty cycle mới (0x0000 - 0x8000)
 **********************************************************/
void Pwm_SetPeriodAndDuty(Pwm_ChannelType ChannelNumber, Pwm_PeriodType Period, uint16_t DutyCycle)
{
    if (!PWM_ISINITIALISED || ChannelNumber >= Pwm_CurrentConfigPtr->NumChannels) return;
    const Pwm_ChannelConfigType* channelConfig = &Pwm_CurrentConfigPtr->Channels[ChannelNumber];
    if (channelConfig->classType != PWM_VARIABLE_PERIOD) return;
    channelConfig->TIMx->ARR = Period;
    uint16_t compareValue = ((uint32_t)Period * DutyCycle) >> 15;
    switch (channelConfig->channel) {
    case 1: channelConfig->TIMx->CCR1 = compareValue; break;
    case 2: channelConfig->TIMx->CCR2 = compareValue; break;
    case 3: channelConfig->TIMx->CCR3 = compareValue; break;
    case 4: channelConfig->TIMx->CCR4 = compareValue; break;
    default: break;
    }
}

/**********************************************************
 * @brief   Đưa kênh PWM về trạng thái idle (tắt output)
 **********************************************************/
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber)
{
    if (!PWM_ISINITIALISED || ChannelNumber >= Pwm_CurrentConfigPtr->NumChannels) return;
    const Pwm_ChannelConfigType* channelConfig = &Pwm_CurrentConfigPtr->Channels[ChannelNumber];
    switch (channelConfig->channel) {
    case 1: channelConfig->TIMx->CCR1 = 0; break;
    case 2: channelConfig->TIMx->CCR2 = 0; break;
    case 3: channelConfig->TIMx->CCR3 = 0; break;
    case 4: channelConfig->TIMx->CCR4 = 0; break;
    default: break;
    }
}

/**********************************************************
 * @brief   Đọc trạng thái đầu ra hiện tại của kênh PWM
 * @return  PWM_HIGH nếu compare enable, ngược lại PWM_LOW
 **********************************************************/
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType ChannelNumber)
{
    if (!PWM_ISINITIALISED || ChannelNumber >= Pwm_CurrentConfigPtr->NumChannels) return PWM_LOW;
    const Pwm_ChannelConfigType* channelConfig = &Pwm_CurrentConfigPtr->Channels[ChannelNumber];
    uint16_t isOutputEnabled = 0;
    switch (channelConfig->channel) {
    case 1: isOutputEnabled = (channelConfig->TIMx->CCER & TIM_CCER_CC1E); break;
    case 2: isOutputEnabled = (channelConfig->TIMx->CCER & TIM_CCER_CC2E); break;
    case 3: isOutputEnabled = (channelConfig->TIMx->CCER & TIM_CCER_CC3E); break;
    case 4: isOutputEnabled = (channelConfig->TIMx->CCER & TIM_CCER_CC4E); break;
    default: break;
    }
    return (isOutputEnabled) ? PWM_HIGH : PWM_LOW;
}

/**********************************************************
 * @brief   Tắt thông báo ngắt cho kênh PWM
 * @details Nếu sử dụng callback hoặc ngắt, tắt tại đây.
 *
 * @param[in] ChannelNumber Số thứ tự kênh PWM
 **********************************************************/
void Pwm_DisableNotification(Pwm_ChannelType ChannelNumber)
{
    if (!PWM_ISINITIALISED || ChannelNumber >= Pwm_CurrentConfigPtr->NumChannels) return;
    const Pwm_ChannelConfigType* channelConfig = &Pwm_CurrentConfigPtr->Channels[ChannelNumber];
    switch (channelConfig->channel) {
    case 1: TIM_ITConfig(channelConfig->TIMx, TIM_IT_CC1, DISABLE); break;
    case 2: TIM_ITConfig(channelConfig->TIMx, TIM_IT_CC2, DISABLE); break;
    case 3: TIM_ITConfig(channelConfig->TIMx, TIM_IT_CC3, DISABLE); break;
    case 4: TIM_ITConfig(channelConfig->TIMx, TIM_IT_CC4, DISABLE); break;
    default: break;
    }
}

/**********************************************************
 * @brief   Lấy thông tin phiên bản của driver PWM
 * @details Trả về thông tin phiên bản module PWM.
 *
 * @param[out] versioninfo Con trỏ tới Std_VersionInfoType để nhận version
 **********************************************************/
void Pwm_GetVersionInfo(Std_VersionInfoType* versioninfo)
{
    if (versioninfo == NULL) return;
    versioninfo->vendorID = 0x1234;
    versioninfo->moduleID = 0xABCD;
    versioninfo->sw_major_version = 1;
    versioninfo->sw_minor_version = 0;
    versioninfo->sw_patch_version = 0;
}
/********************************************************** 
 * @brief   Bật thông báo ngắt cho kênh PWM
 * @details Bật thông báo ngắt cho kênh PWM với loại cạnh cụ thể
 * @param[in] ChannelID Số thứ tự kênh PWM
 * @param[in] Notification Loại cạnh cần thông báo (rising, falling, both edges)
***********************************************************/
void Pwm_EnableNotification(Pwm_ChannelType ChannelID, PWM_EdgeNotificationType Notification)
{
 if (ChannelID >= PwmDriverConfig.NumChannels) return;
    Pwm_ChannelConfigType *cfg = (Pwm_ChannelConfigType*)&PwmChannelsConfig[ChannelID];
    TIM_TypeDef *TIMx = cfg->TIMx;
    uint16_t cc_flag = TIM_IT_CC1 << (cfg->channel - 1);
    // cfg->notificationEnabled = 1;

    if (Notification & PWM_RISING_EDGE)
        TIM_ITConfig(TIMx, cc_flag, ENABLE);
    if (Notification & PWM_FALLING_EDGE)
        TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);

    IRQn_Type irq = (TIMx == TIM2)? TIM2_IRQn :
                    (TIMx == TIM3)? TIM3_IRQn :
                    (TIMx == TIM4)? TIM4_IRQn : (IRQn_Type)0xFF;
    if (irq != (IRQn_Type)0xFF) {
        NVIC_InitTypeDef n;
        n.NVIC_IRQChannel = irq;
        n.NVIC_IRQChannelPreemptionPriority = 1;
        n.NVIC_IRQChannelSubPriority = 0;
        n.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&n);
    }

}
