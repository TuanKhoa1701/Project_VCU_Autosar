#include "Com_Cfg.h"
#include "Com.h"
#include <stdio.h>

/* ====================================================================
 * 1) ĐỊNH NGHĨA CÁC BIẾN CẤU HÌNH (từ Com_Cfg.h)
 *    Đây là nơi duy nhất các biến này được định nghĩa trong toàn bộ dự án.
 * ===================================================================*/
uint8_t s_TxBuf_VcuCommand[5u];
uint8_t s_RxBuf_EngStatus[2u];

const Com_IPduCfgType Com_IPduCfg[COM_NUM_IPDUS] =
{
    /* TX: VCU_Command */
    {
        .PduId     = ComConf_ComIPdu_VCU_Command,
        .Length    = (PduLengthType)sizeof(s_TxBuf_VcuCommand),
        .direction = COM_PDU_DIR_TX
    },
    /* RX: Engine_Status */
    {
        .PduId     = ComConf_ComIPdu_Engine_Status,
        .Length    = (PduLengthType)sizeof(s_RxBuf_EngStatus),
        .direction = COM_PDU_DIR_RX
    }
};

const Com_SignalCfgType Com_SignalCfg[COM_NUM_SIGNALS] =
{
    [ComConf_ComSignal_VCU_ThrottleReq_pct] = { .PduId = ComConf_ComIPdu_VCU_Command,   .byteIndex = 0u, .bitOffset = 0u, .bitLength =  8u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX }, /* VCU_ThrottleReq_pct */
    [ComConf_ComSignal_VCU_GearSel]         = { .PduId = ComConf_ComIPdu_VCU_Command,   .byteIndex = 1u, .bitOffset = 0u, .bitLength =  8u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX }, /* VCU_GearSel */
    [ComConf_ComSignal_VCU_DriveMode]       = { .PduId = ComConf_ComIPdu_VCU_Command,   .byteIndex = 2u, .bitOffset = 0u, .bitLength =  8u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX }, /* VCU_DriveMode */
    [ComConf_ComSignal_VCU_BrakeActive]     = { .PduId = ComConf_ComIPdu_VCU_Command,   .byteIndex = 3u, .bitOffset = 0u, .bitLength =  8u, .type = COM_SIGTYPE_BOOLEAN, .direction = COM_PDU_DIR_TX }, /* VCU_BrakeActive (0/1) */
    [ComConf_ComSignal_VCU_Alive]           = { .PduId = ComConf_ComIPdu_VCU_Command,   .byteIndex = 4u, .bitOffset = 0u, .bitLength =  4u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX }, /* VCU_Alive (nibble) */
    [ComConf_ComSignal_EngineSpeedRpm]      = { .PduId = ComConf_ComIPdu_Engine_Status, .byteIndex = 0u, .bitOffset = 0u, .bitLength =  8u, .type = COM_SIGTYPE_UINT8,  .direction = COM_PDU_DIR_RX }  /* EngineSpeedRpm (BE) */
};