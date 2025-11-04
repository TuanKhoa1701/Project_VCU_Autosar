#include "PduR_Cfg.h"

// Định nghĩa các bảng định tuyến
const PduR_Route_1to1_Type ComTxRoutingTable[PDUR_NUM_COM_TX_ROUTES] = {
    {.srcPduId = ComConf_ComIPdu_VCU_Command, .desPduId = CANIFCONF_PDU_VCU_COMMAND},
    {.srcPduId = ComConf_ComIPdu_VCU_Command, .desPduId = CANIFCONF_PDU_ENGINE_STATUS}
};

const PduR_Route_1to1_Type CanIfRxRoutingTable[PDUR_NUM_CAN_RX_ROUTES] = {
    {.srcPduId = CANIFCONF_PDU_VCU_COMMAND, .desPduId = ComConf_ComIPdu_Engine_Status},
};

// Định nghĩa cấu trúc cấu hình chính của PduR
const PduR_PBConfigType PduR_Config = {
    .ComTxRoutingTable = ComTxRoutingTable,
    .CanIfRxRoutingTable = CanIfRxRoutingTable,
    .CanIfTxRoutingTable = NULL // Giả sử không có bảng Tx Confirmation routing trong ví dụ này
};