/**********************************************************
 * @file    Com.c
 * @brief   AUTOSAR COM (phiên bản tối giản chạy trên MCU)
 * @details Thực thi các dịch vụ COM cốt lõi để gói/mở gói tín hiệu:
 *          - TX: Com_SendSignal() ghi vào shadow của I-PDU; với
 *                IPDU chế độ Triggered thì ứng dụng gọi thêm
 *                Com_TriggerIPDUSend() để phát ngay qua PduR.
 *          - RX: Com_RxIndication() (do PduR gọi) cập nhật buffer,
 *                ứng dụng đọc bằng Com_ReceiveSignal().
 *
 *          Phạm vi/giới hạn:
 *            • Không in log, không cấp phát động, không DM/MDT/Filter,
 *              không TxMode periodic. Đủ dùng cho pattern
 *              Rte_Write_*() + Com_TriggerIPDUSend().
 *            • Cấu hình (symbolic IDs, mapping signal↔IPDU, bit/byte)
 *              thường nằm ở Com_Cfg.c/h; ở bản demo này đặt cục bộ
 *              để đơn giản hoá.
 *            • Module thiết kế cho **MCU thật** (không mô phỏng PC).
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#include "Com.h"
#include <string.h>   /* memset, memcpy */
#include <stdio.h>

/**
 * @brief RPM tối đa tương ứng với 100% tốc độ nhận được.
 */
static uint16_t Speed;
static bool Speed_is_Received = FALSE;
static bool Stop;
static bool Stop_is_Received = FALSE;
/* --------------------------------------------------------------------
 * Lower layer (chuẩn AUTOSAR):
 *  - COM gọi PduR_ComTransmit() để yêu cầu truyền I-PDU TX.
 *  - PduR sẽ gọi ngược Com_RxIndication() khi có I-PDU RX hoàn tất.
 * ------------------------------------------------------------------ */
extern Std_ReturnType PduR_ComTransmit(PduIdType TxPduId, const PduInfoType* info);

/* -------- Truy cập bộ đệm theo PduId --------------------------------
 *  Trả về con trỏ buffer nội bộ và (tuỳ chọn) chiều & độ dài.
 * ------------------------------------------------------------------ */
static uint8_t* prv_get_pdu_buf(PduIdType pduId, PduLengthType* len, Com_PduDirection_e* dir)
{
    for (uint16_t i = 0u; i < COM_NUM_IPDUS; ++i)
    {
        if (Com_IPduCfg[i].PduId == pduId)
        {
            if (len) *len = Com_IPduCfg[i].Length;
            if (dir) *dir = Com_IPduCfg[i].direction;

            if (pduId == ComConf_ComIPdu_VCU_Command)   { return s_TxBuf_VcuCommand; }
            if (pduId == ComConf_ComIPdu_Engine_Status) { return s_RxBuf_EngStatus;  }
            break;
        }
    }
    return NULL;
}

/* ====================================================================
 * 2) TIỆN ÍCH PACK/UNPACK TRONG MIỀN BYTE
 * ===================================================================*/
static inline void put_u8(uint8_t* dst, uint16_t byteIndex, uint8_t v)
{
    dst[byteIndex] = v;
}

static inline uint8_t get_u8(const uint8_t* src, uint16_t byteIndex)
{
    return src[byteIndex];
}

/* Ghi nibble (4 bit) vào vị trí bitOffset (0..3) trong một byte */
static void put_nibble(uint8_t* dst, uint16_t byteIndex, uint8_t bitOffset, uint8_t v4)
{
    const uint8_t mask = (uint8_t)(0x0Fu << bitOffset);
    const uint8_t val  = (uint8_t)((v4 & 0x0Fu) << bitOffset);
    dst[byteIndex] = (uint8_t)((dst[byteIndex] & (uint8_t)(~mask)) | val);
}

/* Ghi 1 bit (bitLength=1) tại bitOffset (0..7) */
static void put_bit(uint8_t* dst, uint16_t byteIndex, uint8_t bitOffset, boolean b)
{
    const uint8_t mask = (uint8_t)(1u << bitOffset);
    if (b) { dst[byteIndex] = (uint8_t)(dst[byteIndex] |  mask); }
    else   { dst[byteIndex] = (uint8_t)(dst[byteIndex] & (uint8_t)(~mask)); }
}

/* Đọc big-endian 16-bit: (b0<<8) | b1 */
static inline uint16_t get_be16(const uint8_t* src, uint16_t byteIndex)
{
    return (uint16_t)((((uint16_t)src[byteIndex]) << 8) | (uint16_t)src[byteIndex + 1u]);
}

/* ====================================================================
 * 3) LIFECYCLE
 * ===================================================================*/
void Com_Init(void)
{
    (void)memset(s_TxBuf_VcuCommand, 0, sizeof(s_TxBuf_VcuCommand));
    (void)memset(s_RxBuf_EngStatus,  0, sizeof(s_RxBuf_EngStatus));
    //printf("Com_Init\n");
}

void Com_DeInit(void)
{
    /* Không giữ trạng thái động; không cần xử lý gì thêm. */
}

/* ====================================================================
 * 4) API TX
 * ===================================================================*/
Std_ReturnType Com_SendSignal(Com_SignalIdType id, const void* dataPtr)
{
    if (dataPtr == NULL)  return E_NOT_OK; 

    /* Tra cấu hình signal theo id (demo: id là chỉ số tuyến tính) */
    const Com_SignalCfgType* cfg = &Com_SignalCfg[id];

    if ((cfg == NULL) || (cfg->direction != COM_PDU_DIR_TX)) { return E_NOT_OK; }

    PduLengthType len = 0u; 
    Com_PduDirection_e d;

    uint8_t* pdu = prv_get_pdu_buf(cfg->PduId, &len, &d);
    if ((pdu == NULL) || (d != COM_PDU_DIR_TX)) { return E_NOT_OK; }
    if (cfg->byteIndex >= len) { return E_NOT_OK; }

    switch (cfg->bitLength)
    {
        case 8u:
        {
            if (cfg->type == COM_SIGTYPE_BOOLEAN)
            {
                const uint8_t v = (*(const boolean*)dataPtr) ? 1u : 0u;
                put_u8(pdu, cfg->byteIndex, v);
            }
            else
            {
                put_u8(pdu, cfg->byteIndex, *(const uint8_t*)dataPtr);
            }
        } break;

        case 4u:
        {
            const uint8_t v4 = (uint8_t)(*(const uint8_t*)dataPtr & 0x0Fu);
            put_nibble(pdu, cfg->byteIndex, cfg->bitOffset, v4);
        } break;

        case 1u:
        {
            const boolean b = (*(const boolean*)dataPtr) ? TRUE : FALSE;
            put_bit(pdu, cfg->byteIndex, cfg->bitOffset, b);
        } break;

        default:
            /* Demo này chỉ hỗ trợ 1/4/8 bit (và 16 bit cho RX) */
            return E_NOT_OK;
    }

    return E_OK;
}

/**
 * @brief  Kích phát gửi I-PDU ngay (đường TX).
 * @note   Dùng cho IPDU chế độ Triggered/Direct. Với Periodic, việc gửi do
 *         nền tảng lập lịch riêng đảm nhiệm (không hiện thực ở demo này).
 */
Std_ReturnType Com_TriggerIPDUSend(PduIdType pduId)
{
    PduLengthType len = 0u; Com_PduDirection_e d;
    uint8_t* buf = prv_get_pdu_buf(pduId, &len, &d);
    if ((buf == NULL) || (d != COM_PDU_DIR_TX)) { return E_NOT_OK; }

    PduInfoType info;
    info.SduDataPtr = buf;
    info.SduLength  = len;

    return PduR_ComTransmit(pduId, &info);
}
/* ====================================================================
 * 5) API RX
 * ===================================================================*/

void Com_RxIndication(PduIdType ComRxPduId, const PduInfoType* PduInfoPtr){

    PduLengthType len = 0u;
    Com_PduDirection_e dir;
    uint8_t* buf = prv_get_pdu_buf(ComRxPduId, &len, &dir);

    /* Kiểm tra PDU hợp lệ và có dữ liệu để xử lý */
    if ((buf == NULL) || (dir != COM_PDU_DIR_RX) || (PduInfoPtr == NULL) || (PduInfoPtr->SduDataPtr == NULL))
    {
        return;
    }

    /* Sao chép dữ liệu nhận được vào buffer nội bộ của COM */
    PduLengthType bytes_to_copy = (PduInfoPtr->SduLength < len) ? PduInfoPtr->SduLength : len;
    (void)memcpy(buf, PduInfoPtr->SduDataPtr, bytes_to_copy);

    /* Xử lý các tín hiệu trong PDU vừa nhận (ví dụ) */
    if (ComRxPduId == ComConf_ComIPdu_Engine_Status)
    {
        /* Giả sử tín hiệu tốc độ động cơ được mã hóa trong 2 byte đầu tiên (big-endian) */
        Speed = get_u8(buf, 0); /* Đọc 2 byte từ đầu buffer */
    }
}
void Com_TxConfirmation(PduIdType ComTxPduId){
    (void)ComTxPduId;
}
Std_ReturnType Com_ReceiveSignal(Com_SignalIdType id, void* dataPtr){
    if (dataPtr == NULL)  return E_NOT_OK;

    const Com_SignalCfgType* cfg = &Com_SignalCfg[id];
    if ((id > COM_NUM_SIGNALS) || (cfg->direction != COM_PDU_DIR_RX)) return E_NOT_OK;

    if(id == ComConf_ComSignal_EngineSpeedRpm){
        *(uint16_t*)dataPtr = Speed;
        return E_OK;
    }
    else return E_NOT_OK;
}
