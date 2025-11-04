/**********************************************
 * @file   : IoHwAb_Digital_Cfg.c
 * @brief  : Định nghĩa cấu hình cho IoHwAb Digital.
 * @version: 1.0
 * @date   : 2024-06-27
 * @author : Nguyễn Tuấn Khoa
 *********************************************/

#include "IoHwAb_Digital_Cfg.h"

/* Định nghĩa biến cấu hình IoHwAb cho Digital I/O */
const IoHwAb1_ConfigType IoHwAb1_Config = {
    .portConfig = &portConfig,
    .adcConfig = &Adc_Configs[0],
    .canConfig = &Can_Config,
};

