#ifndef PDUR_CFG_H
#define PDUR_CFG_H

#include "PduR.h"
#include "CanIf_Cfg.h"
#include "Com_Cfg.h"

#define PDUR_NUM_COM_TX_ROUTES 2
#define PDUR_NUM_CAN_RX_ROUTES 2
#define PDUR_NUM_CAN_TX_ROUTES 2

#define CANIFCONF_PDU_VCU_COMMAND 0X00U
#define CANIFCONF_PDU_ENGINE_STATUS 0X01u

extern const PduR_Route_1to1_Type ComTxRoutingTable[PDUR_NUM_COM_TX_ROUTES];
extern const PduR_Route_1to1_Type CanIfRxRoutingTable[PDUR_NUM_CAN_RX_ROUTES];
// Nếu có bảng CanIfTxRoutingTable, bạn cũng sẽ khai báo extern ở đây
// extern const PduR_Route_1to1_Type CanIfTxRoutingTable[PDUR_NUM_CAN_TX_ROUTES];

extern const PduR_PBConfigType PduR_Config;

#endif /* PDUR_CFG_H */
