/**********************************************
 * @file   : IoHwAb_Adc_Cfg.c
 * @brief  : Định nghĩa cấu hình cho IoHwAb ADC.
 * @version: 1.0
 * @date   : 2024-06-27
 * @author : Nguyễn Tuấn Khoa
 *********************************************/

#include "IoHwAb_ADC_Cfg.h"

/* Định nghĩa biến cấu hình cho IoHwAb ADC */
const IoHwAb0_ConfigType IoHwAb0_Config = {
    .portConfig = &portConfig,
    .adcConfig = &Adc_Configs[0],
};
