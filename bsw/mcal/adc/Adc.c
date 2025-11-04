#include "Adc.h"
#include "Std_Types.h"
#include "Adc_cfg.h"

void Adc_Init(const Adc_ConfigType *ConfigPtr)
{
    const Adc_ConfigType *cfg = ConfigPtr;
    for (uint8_t i = 0; i < ConfigPtr->NumChannels; i++)
    {
        // 2. Clock cho ADC
        if (cfg->AdcInstance == ADC_INSTANCE_1)
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        else if (cfg->AdcInstance == ADC_INSTANCE_2)
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

        // 3. Init ADC
        switch (cfg->ClockPrescaler)
        {
        case 2:
            RCC_ADCCLKConfig(RCC_PCLK2_Div2);
            break;
        case 4:
            RCC_ADCCLKConfig(RCC_PCLK2_Div4);
            break;
        case 6:
            RCC_ADCCLKConfig(RCC_PCLK2_Div6);
            break;
        case 8:
            RCC_ADCCLKConfig(RCC_PCLK2_Div8);
            break;
        default:
            RCC_ADCCLKConfig(RCC_PCLK2_Div2); // Mặc định
            break;
        }
        ADC_InitTypeDef ADC_InitStructure;
        ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
        ADC_InitStructure.ADC_ContinuousConvMode = (cfg->ConversionMode == ADC_CONV_MODE_CONTINUOUS) ? ENABLE : DISABLE;
        // ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
        ADC_InitStructure.ADC_ScanConvMode = DISABLE;
        ADC_InitStructure.ADC_ExternalTrigConv = (cfg->TriggerSource == ADC_TRIGGER_SOFTWARE) ? ADC_ExternalTrigConv_None : cfg->TriggerSource;

        if (cfg->ResultAlignment == ADC_RESULT_ALIGNMENT_RIGHT)
            ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
        else
            ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Left;

        ADC_InitStructure.ADC_NbrOfChannel = cfg->NumChannels;

        if (cfg->AdcInstance == ADC_INSTANCE_1)
        {
            ADC_Init(ADC1, &ADC_InitStructure);
            ADC_Cmd(ADC1, ENABLE);
        }
        else
        {
            ADC_Init(ADC2, &ADC_InitStructure);
            ADC_Cmd(ADC2, ENABLE);
        }
        // 4. Setup kênh
        for (uint8_t ch = 0; ch < cfg->NumChannels; ch++)
        {
            if (cfg->AdcInstance == ADC_INSTANCE_1)
                ADC_RegularChannelConfig(ADC1, cfg->Channels[ch].Channel, cfg->Channels[ch].Rank, cfg->Channels[ch].SamplingTime);
            else
                ADC_RegularChannelConfig(ADC2, cfg->Channels[ch].Channel, cfg->Channels[ch].Rank, cfg->Channels[ch].SamplingTime);
        }

        // 6. Enable
        for (uint8_t i = 0; i < ADC_MAX_GROUPS; i++)
        {
            Adc_GroupConfigs[i].Status = ADC_IDLE;
            Adc_GroupConfigs[i].Result = NULL;
        }
    }
}

void Adc_DeInit(void)
{
    ADC_DeInit(ADC1);
    ADC_DeInit(ADC2);
    for (uint8_t i = 0; i < ADC_MAX_GROUPS; i++)
    {
        Adc_GroupConfigs[i].Status = ADC_IDLE;
        Adc_GroupConfigs[i].Result = NULL;
    }
}

void Adc_StartGroupConversion(Adc_GroupType Group)
{
    Adc_GroupDefType *grp = &Adc_GroupConfigs[Group];
    grp->Status = ADC_BUSY;

    if (grp->Adc_StreamEnableType == 1)
    {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        DMA_InitTypeDef DMA_InitStruct;
        DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
        DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)grp->Result;
        DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
        DMA_InitStruct.DMA_BufferSize = grp->NumChannels;
        DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
        DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
        DMA_InitStruct.DMA_Mode = (grp->Adc_StreamBufferMode == ADC_STREAM_BUFFER_CIRCULAR) ? DMA_Mode_Circular : DMA_Mode_Normal;
        DMA_InitStruct.DMA_Priority = DMA_Priority_High;
        DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
        DMA_Init(DMA1_Channel1, &DMA_InitStruct);
        // ngắt DMA
        if (grp->Dma_Notification == DMA_ADC_NOTIFICATION_ENABLED)
        {
            DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
            NVIC_EnableIRQ(DMA1_Channel1_IRQn);
        }

        DMA_Cmd(DMA1_Channel1, ENABLE);

        if (grp->AdcInstance == ADC_INSTANCE_1)
        {
            ADC_DMACmd(ADC1, ENABLE);
            ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        }
        else
            ADC_DMACmd(ADC2, ENABLE);
            ADC_SoftwareStartConvCmd(ADC2, ENABLE);
    }
    else
    {
        if (grp->AdcInstance == ADC_INSTANCE_1)
            ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        else
            ADC_SoftwareStartConvCmd(ADC2, ENABLE);
    }
}

void Adc_StopGroupConversion(Adc_GroupType Group)
{
    Adc_GroupDefType *grp = &Adc_GroupConfigs[Group];

    if (grp->AdcInstance == ADC_INSTANCE_1)
        ADC_SoftwareStartConvCmd(ADC1, DISABLE);
    else
        ADC_SoftwareStartConvCmd(ADC2, DISABLE);

    grp->Status = ADC_IDLE;
}

Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr)
{
    Adc_GroupDefType *grp = &Adc_GroupConfigs[Group];
    Adc_ConfigType *cfg = &Adc_Configs[grp->AdcInstance];

    if (grp->Status != ADC_BUSY)
        return E_NOT_OK;

    if (grp->AdcInstance == ADC_INSTANCE_1)
        *DataBufferPtr = ADC_GetConversionValue(ADC1);
    else
        *DataBufferPtr = ADC_GetConversionValue(ADC2);

    if (cfg->ConversionMode != ADC_CONV_MODE_CONTINUOUS)
        grp->Status = ADC_COMPLETED;
    else
        grp->Status = ADC_BUSY;

    if (grp->Result != NULL)
        grp->Result[0] = *DataBufferPtr;

    return E_OK;
}

Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr)
{
    if (Group >= ADC_MAX_GROUPS || DataBufferPtr == NULL)
        return E_NOT_OK;

    Adc_GroupConfigs[Group].Result = DataBufferPtr;
    return E_OK;
}

void Adc_EnableHardwareTrigger(Adc_GroupType Group) { (void)Group; }
void Adc_DisableHardwareTrigger(Adc_GroupType Group) { (void)Group; }

void Adc_EnableGroupNotification(Adc_GroupType Group)
{
    Adc_ConfigType *cfg = &Adc_Configs[Adc_GroupConfigs[Group].AdcInstance];
    cfg->NotificationEnabled = ADC_NOTIFICATION_ENABLED;

    if (Adc_GroupConfigs[Group].AdcInstance == ADC_INSTANCE_1)
    {
        ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
        NVIC_EnableIRQ(ADC1_2_IRQn);
    }
    else
    {
        ADC_ITConfig(ADC2, ADC_IT_EOC, ENABLE);
        NVIC_EnableIRQ(ADC1_2_IRQn);
    }
}

void Adc_DisableGroupNotification(Adc_GroupType Group)
{
    Adc_ConfigType *cfg = &Adc_Configs[Adc_GroupConfigs[Group].AdcInstance];
    cfg->NotificationEnabled = ADC_NOTIFICATION_DISABLED;

    if (Adc_GroupConfigs[Group].AdcInstance == ADC_INSTANCE_1)
        ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
    else
        ADC_ITConfig(ADC2, ADC_IT_EOC, DISABLE);
}

Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group)
{
    return Adc_GroupConfigs[Group].Status;
}

Std_ReturnType Adc_GetStreamLastPointer(Adc_GroupType Group, Adc_ValueGroupType **PtrToSampleAddress)
{
    if (Adc_GroupConfigs[Group].Result == NULL || PtrToSampleAddress == NULL)
        return E_NOT_OK;

    *PtrToSampleAddress = &Adc_GroupConfigs[Group].Result[0];
    return E_OK;
}

Std_ReturnType Adc_SetPowerState(Adc_GroupType group, Adc_PowerStateType state)
{

    Adc_InstanceType ADC_Instance = Adc_GroupConfigs[group].AdcInstance;
    ADC_TypeDef *ADCx = (ADC_Instance == ADC_INSTANCE_1) ? ADC1 : ADC2;
    switch (state)
    {
    case ADC_POWERSTATE_OFF:
        ADC_Cmd(ADCx, DISABLE);
        return E_OK;
    case ADC_POWERSTATE_ON:
        ADC_Cmd(ADCx, ENABLE);
        return E_OK;
    case ADC_POWERSTATE_LOWPOWER:
        // VD: giảm prescaler, tắt clock khi idle
        //RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
        return E_OK;
    default:
        return E_NOT_OK;
    }
}

void Adc_GetVersionInfo(Std_VersionInfoType *VersionInfo)
{
    if (VersionInfo != NULL)
    {
        VersionInfo->vendorID = 1234;
        VersionInfo->moduleID = 1;
        VersionInfo->sw_major_version = 1;
        VersionInfo->sw_minor_version = 0;
        VersionInfo->sw_patch_version = 0;
    }
}
