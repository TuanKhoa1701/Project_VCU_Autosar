#ifndef CAN_CFG_H
#define CAN_CFG_H
#include "Can.h"
#define CAN_MAX_TX_MAILBOX 3u

extern Can_HwType MailBox[CAN_MAX_TX_MAILBOX];
extern const Can_ConfigType Can_Config;

void Can_RegisterRxCallback(void(*cb)(const Can_HwType* Mailbox, const PduInfoType* PduInfoPtr));
void Can_RegisterTxCallback(void(*cb)(PduIdType TxPduID));


void USB_LP_CAN1_RX0_IRQHandler(void);
#endif /* CAN_CFG_H */
