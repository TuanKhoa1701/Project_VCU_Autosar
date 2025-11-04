#include "Adc_cfg.h"
#include "Adc.h"
#include <stdio.h>
Adc_ValueGroupType Adc_Group_Buffer[ADC_MAX_GROUPS];
void Adc_Notification_callback(void)
{
    // Ví dụ: bật LED khi có thông báo từ ADC
    Adc_ReadGroup(0, &Adc_Group_Buffer[0]);
    // printf("ADC Notification callback!\n");
}
void DMA_Notification_callback(void)
{
    // Ví dụ: bật LED khi có thông báo từ ADC
    Adc_ReadGroup(0, &Adc_Group_Buffer[0]);
    // printf("DMA ADC Notification callback!\n");
}
void ADC_isrHandler(void)
{
    for (uint8_t group = 0; group < ADC_MAX_GROUPS; group++)
    {
        Adc_GroupDefType *grp = &Adc_GroupConfigs[group];
        Adc_ConfigType *cfg = &Adc_Configs[grp->AdcInstance];

        if (cfg->NotificationEnabled == ADC_NOTIFICATION_ENABLED)
        {
            if (grp->AdcInstance == ADC_INSTANCE_1 && ADC_GetITStatus(ADC1, ADC_IT_EOC))
            {
                ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);

                if (cfg->InitCallback)
                    cfg->InitCallback();
            }
            else if (grp->AdcInstance == ADC_INSTANCE_2 && ADC_GetITStatus(ADC2, ADC_IT_EOC))
            {
                ADC_ClearITPendingBit(ADC2, ADC_IT_EOC);

                if (cfg->InitCallback)
                    cfg->InitCallback();
            }
        }
    }
}
void DMA_ADC_isrHandler(void)
{
    for (uint8_t group = 0; group < ADC_MAX_GROUPS; group++)
    {
        Adc_GroupDefType *grp = &Adc_GroupConfigs[group];
        Adc_ConfigType *cfg = &Adc_Configs[grp->AdcInstance];

        if (grp->Dma_Notification == DMA_ADC_NOTIFICATION_ENABLED)
        {
            if (DMA_GetITStatus(DMA1_IT_TC1))
            {
                DMA_ClearITPendingBit(DMA1_IT_TC1);

                if (cfg->InitCallback)
                    cfg->InitCallback();
            }
        }
    }
}
Adc_ConfigType Adc_Configs[1] = {
    {.AdcInstance = ADC_INSTANCE_1,
     .ClockPrescaler = 2,
     .ConversionMode = ADC_CONV_MODE_CONTINUOUS,
     .TriggerSource = ADC_TRIGGER_SOFTWARE,
     .NotificationEnabled = ADC_NOTIFICATION_DISABLED,
     .NumChannels = 1,
     .ResultAlignment = ADC_RESULT_ALIGNMENT_RIGHT,
     .InitCallback = DMA_Notification_callback,
     .Channels = {
         {.Channel = ADC_Channel_0, .SamplingTime = ADC_SampleTime_1Cycles5, .Rank = 1}}}};
Adc_GroupDefType Adc_GroupConfigs[ADC_MAX_GROUPS] = {
    {.id = 0,
     .AdcInstance = ADC_INSTANCE_1,
     .Channels = {ADC_Channel_1},
     .Priority = 0,
     .NumChannels = 1,
     .Status = ADC_IDLE,
     .Result = Adc_Group_Buffer,
     .Adc_StreamEnableType = 1,
     .Adc_StreamBufferSize = 1,
     .Adc_StreamBufferMode = ADC_STREAM_BUFFER_CIRCULAR,
     .Dma_Notification = DMA_ADC_NOTIFICATION_DISABLED}};
