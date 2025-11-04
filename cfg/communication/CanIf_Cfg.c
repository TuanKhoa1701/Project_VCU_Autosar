#include "CanIf_Cfg.h"

CanIf_RoutingEntry RoutingTable[CANIF_NUM_TX_PDUS+CANIF_NUM_RX_PDUS] = {
        // Ánh xạ PDU ID 0 của CanIf sang CAN ID 0x100 (VCU_COMMAND) để truyền (TX)
        [0] = {.id = 0, .CanId = 0x123, .isTX = 1, .Hth = 0},
        // Ánh xạ PDU ID 1 của CanIf sang CAN ID 0x200 (ENGINE_STATUS) để nhận (RX)
        [1] = {.id = 1, .CanId = 0x200, .isTX = 0, .Hth = 0},
        // test loop back 
        [2] = {.id = 0, .CanId = 0x123, .isTX = 0, .Hth = 0}
};

void App_RxCallback(PduIdType LPduId, const PduInfoType* PduInfo){

    PduR_CanIfRxIndication(LPduId, PduInfo);
}


void App_TxConfirm(PduIdType TxPduID){
    // Chuyển tiếp xác nhận lên PduR
    PduR_CanIfTxConfirmation(TxPduID);
}

CanIf_ConfigType My_CanIf_Config = {
    .numControllers = 1,
    .controllerMode = {CANIF_CONTROLLER_STARTED},
    .numTxPdus = CANIF_NUM_TX_PDUS,
    .txPduMode = {CANIF_ONLINE, CANIF_ONLINE},
    .numRxPdus = CANIF_NUM_RX_PDUS,
    .rxPduMode = {CANIF_ONLINE},
    .numRoutingEntries = 2,
    .routingTable = RoutingTable,
    .txConfirmationCallback = App_TxConfirm,
    .rxIndicationCallback = App_RxCallback
};

void CanIf_TxConfirmation(Can_IdType CanId){
    PduIdType PduId = 0xFF; // Giá trị mặc định không hợp lệ
    const CanIf_ConfigType* config = &My_CanIf_Config;

    for(int i = 0; i < config->numRoutingEntries; i++){
        if(RoutingTable[i].CanId == CanId && RoutingTable[i].isTX){
            PduId = RoutingTable[i].id;
            break;
        }
    }

    if((config->txConfirmationCallback) && (PduId != 0xFF)) {
        config->txConfirmationCallback(PduId);
    }
}

void CanIf_RxIndication(const Can_HwType* Mailbox, const PduInfoType *PduInfoPtr) {
    PduIdType PduId ; 
    const CanIf_ConfigType* config = &My_CanIf_Config;

    // Duyệt bảng định tuyến để tìm PDU ID tương ứng với CAN ID đã nhận
    for(int i = 0; i < config->numRoutingEntries; i++){
        if(RoutingTable[i].CanId == Mailbox->CanId && RoutingTable[i].isTX == 0){
            PduId = RoutingTable[i].id;
            break;
        }
    }

    if((config->rxIndicationCallback) && (PduId != 0xFF)) {
        config->rxIndicationCallback(PduId, PduInfoPtr);
    }
}