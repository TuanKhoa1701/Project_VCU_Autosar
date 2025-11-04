/**********************************************
 * @file   : IoHwAb_Adc_Cfg.h
 * @brief  : IoHwAb ADC Driver Configuration Header File (AUTOSAR)
 * @details: Khai báo kiểu cấu hình cho ADC trong IoHwAb.
 * @version: 1.0
 * @date   : 2024-06-27
 * @author : Nguyễn Tuấn Khoa
 *********************************************/
#ifndef __IOHWAB_DIGITAL_CFG_H__
#define __IOHWAB_DIGITAL_CFG_H__
#include "Port_cfg.h"
#include "Adc_Cfg.h"
#include "Pwm_Lcfg.h"
#include "Can_Cfg.h"

extern const Port_ConfigType portConfig;
typedef enum
{
    IoHwAb_CHANNEL_Button = 1,
    IoHwAb_CHANNEL_Led = 12,
} IoHwAb_SignalType;
typedef struct
{
    Port_ConfigType *portConfig;
    Adc_ConfigType *adcConfig;
    Can_ConfigType *canConfig;
} IoHwAb1_ConfigType;

/* Khai báo biến cấu hình, định nghĩa sẽ nằm trong file .c */
extern const IoHwAb1_ConfigType IoHwAb1_Config;

#endif /* __IOHWAB_DIGITAL_CFG_H__ */