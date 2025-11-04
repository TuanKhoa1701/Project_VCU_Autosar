/*************************************
 * @file IoHwAb_Adc.c
 * @brief IoHwAb ADC Driver Source File (AUTOSAR)
 * @details Cài đặt các hàm cho IoHwAb ADC
 * @version 1.0
 * @date 09-09-2025
 * @author Nguyễn Tuấn Khoa
 **************************************/
#include "IoHwAb_adc.h"

static uint16_t Temperature_RawValue = 0;
static uint16_t Temperature_ScaledValue = 0;
Adc_ValueGroupType ADC_Group_Buffer[ADC_MAX_GROUPS];

void IoHwAb_Init0(const IoHwAb0_ConfigType *cfg)
{

    // khởi tạo Port
    Port_Init(cfg->portConfig);
    // Khởi tạo Adc
    Adc_Init(cfg->adcConfig);
    //   Adc_EnableGroupNotification(0);
}

Std_ReturnType IoHwAb_ReadRaw_0(uint16_t *Temperature_RawValue)
{
    if (Temperature_RawValue == NULL)
    {
        return E_NOT_OK;
    }
    Adc_GroupType grp = Adc_GroupConfigs->id;
    Adc_StartGroupConversion(grp);
    Adc_ReadGroup(0, &ADC_Group_Buffer[0]);
    *Temperature_RawValue = ADC_Group_Buffer[0];
    return E_OK;
}
Std_ReturnType IoHwAb_ReadScaleValue_0(uint16_t *Temperature_ScaledValue)
{
    if (Temperature_ScaledValue == NULL)
    {
        return E_NOT_OK;
    }
    if (IoHwAb_ReadRaw_0(&Temperature_RawValue) != E_OK)
    {
        return E_NOT_OK;
    }
    else
    {
        // Giả sử giá trị thô từ ADC là từ 0 đến 1023 (10-bit ADC)
        // và nhiệt độ tương ứng là từ -40 đến 125 độ C
        *Temperature_ScaledValue = (Temperature_RawValue * 165) / 1023 - 40;
        return E_OK;
    }
}