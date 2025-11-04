#ifndef CANIF_CFG_H
#define CANIF_CFG_H
#include "CanIf.h"
#include "PduR.h" // Cần include để biết prototype của PduR_CanIfRxIndication
#include "Com.h" // For Com_TxConfirmation and Com_RxIndication

#define CANIF_NUM_TX_PDUS 4
#define CANIF_NUM_RX_PDUS 1

extern CanIf_RoutingEntry RoutingTable[CANIF_NUM_TX_PDUS+CANIF_NUM_RX_PDUS];

void App_RxCallback(PduIdType LPduId, const PduInfoType* PduInfo);
void App_TxConfirm(PduIdType TxPduID);

extern CanIf_ConfigType My_CanIf_Config;

/***************************************************************************
 * @brief Callback xác nhận truyền, được gọi bởi lớp CanDrv.
 * @details Khi CanDrv hoàn tất việc truyền một frame, nó sẽ gọi hàm này.
 *          CanIf tìm PDU ID tương ứng với CAN ID đã truyền và gọi callback của lớp trên (PduR).
 * @param[in] CanId CAN ID của frame vừa được truyền thành công.
 *****************************************************************************/
void CanIf_TxConfirmation(Can_IdType CanId);

/**
 * @brief Callback chỉ báo nhận dữ liệu, được gọi bởi lớp CanDrv.
 * @details Khi CanDrv nhận được một frame mới, nó sẽ gọi hàm này.
 *          CanIf tìm PDU ID tương ứng với CAN ID đã nhận và gọi callback của lớp trên (PduR)
 *          để chuyển dữ liệu lên.
 * @param[in] Mailbox Con trỏ tới cấu trúc mô tả phần cứng đã nhận frame (không dùng trong logic này).
 * @param[in] PduInfoPtr Con trỏ tới thông tin PDU (ID, data, length) đã nhận.
 */
void CanIf_RxIndication(const Can_HwType* Mailbox, const PduInfoType *PduInfoPtr);

#endif