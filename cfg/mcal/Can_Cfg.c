#include "Can_Cfg.h"
#include "Can.h"
#include <stdio.h>
static void (*rxCallback)(const Can_HwType* Mailbox, const PduInfoType* PduInfoPtr) = 0;
static void (*txCallback)(PduIdType) =0;

const Can_ConfigType Can_Config = {
    .Basic_Config = {
        .CAN_Prescaler = 6,
        .CAN_Mode = CAN_Mode_Silent_LoopBack,
        .CAN_BS1 = CAN_BS1_6tq,
        .CAN_BS2 = CAN_BS2_8tq,
        .CAN_SJW = CAN_SJW_1tq,
        .CAN_TTCM = DISABLE,
        .CAN_ABOM = ENABLE,
        .CAN_AWUM = ENABLE,
        .CAN_NART = DISABLE,
        .CAN_RFLM = DISABLE,
        .CAN_TXFP = ENABLE,
    },
    .Filter_Config = {
        .Can_FilterIdHigh = (0x123 << 5),
        .Can_FilterIdLow = 0,
        .Can_FilterMaskIdHigh = 0xFFE0,
        .Can_FilterMaskIdLow = 0,
        .Can_FilterNumber = 0,
        .Can_FilterMode = CAN_FilterMode_IdMask,
        .Can_FilterScale = CAN_FilterScale_32bit,
        .Can_FilterFIFOAssignment = CAN_FIFO0,
        .Can_FilterActivation = ENABLE,
    },
    .NotificationEnable = ENABLE,
};

void Can_RegisterRxCallback(void(*cb)(const Can_HwType* Mailbox, const PduInfoType* PduInfoPtr)){
    rxCallback = cb;
}
void Can_RegisterTxCallback(void(*cb)(PduIdType TxPduID)){
    txCallback = cb;
}

void USB_LP_CAN1_RX0_IRQHandler(void){
    if(CAN_GetITStatus(CAN1, CAN_IT_FMP0) == SET){
        CanRxMsg RxMessage;
        CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
        if(rxCallback){
        /*  Đóng gói dữ liệu thành gói tin Pdu*/
            PduInfoType PduInfo;
            PduInfo.SduDataPtr = (uint8_t*)RxMessage.Data;
            PduInfo.SduLength = RxMessage.DLC;
        /* Cập nhật Can ID*/
            Can_HwType CAN;
            CAN.CanId = RxMessage.StdId;
            
            rxCallback(&CAN, &PduInfo);
        }
    CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    }
}
void USB_HP_CAN1_TX_IRQHandler(void){
    if(CAN_GetITStatus(CAN1, CAN_IT_TME) == SET){
    //     if(CAN_TransmitStatus(CAN1, CAN_TXMAILBOX_0)){
            
    //  }
    CAN_ClearITPendingBit(CAN1, CAN_IT_TME);
    }
}

