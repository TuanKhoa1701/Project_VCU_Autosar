/***********************************************************
 * @file   IoHwAb_Digital.c
 * @brief  IoHwAb Digital Driver Source File (AUTOSAR)
 * @details: Cài đặt các hàm cho IoHwAb Digital.
 * @version: 1.0
 * @date   : 2024-06-27
 * @author : Nguyễn Tuấn Khoa
 *************************************************************/

#include "IoHwAb_Digital.h"
#include <stdio.h>>
void IoHwAb_Init1(const IoHwAb1_ConfigType *cfg)
{
    //printf("IoHwAb_Init1\n");
    // khởi tạo Port
    Port_Init(cfg->portConfig);
    // Khởi tạo ADC
    Adc_Init(cfg->adcConfig);
    // Khởi tạo CAN
    Can_Init(cfg->canConfig);
}

Std_ReturnType IoHwAb_Digital_ReadChannel(IoHwAb_SignalType id,Dio_LevelType *Value)
{

    if (Value == NULL)
    {
        return E_NOT_OK;
    }
    *Value = DIO_ReadChannel(id);

    for (int i = 0; i < 10; i++)
    {
        if (DIO_ReadChannel(id) != *Value)
        {
            *Value = FALSE;
            break;
        }
    }
    return E_OK;
}

Std_ReturnType IoHwAb_Digital_WriteSignal(IoHwAb_SignalType id,Dio_LevelType Value)
{

    DIO_WriteChannel(id, Value);

    if (DIO_ReadChannel(id) != Value)
    {
        return E_NOT_OK;
    }
    return E_OK;
}
