/**********************************************************
 * @file    Com_Cfg.h
 * @brief   Cấu hình rút gọn cho AUTOSAR COM chạy trên MCU
 * @details Khai báo:
 *            - Kiểu ID cho Signal/SignalGroup/GroupSignal
 *            - Hướng I-PDU (RX/TX)
 *            - Kiểu dữ liệu tín hiệu (uint8/boolean/uint16)
 *            - Cấu trúc ánh xạ Signal ↔ I-PDU (byte/bit offset)
 *            - Cấu trúc cấu hình I-PDU (độ dài, hướng)
 *            - Các Symbolic ID cho ví dụ: VCU_Command (TX),
 *              Engine_Status (RX)
 *          Lưu ý triển khai:
 *            • Tất cả chạy trên MCU thật; không có phần mô phỏng PC.
 *            • Bản demo chỉ hỗ trợ tín hiệu đặt trọn trong 1 byte,
 *              1 nibble (4 bit) hoặc 1 bit; và 16-bit big-endian cho RX.
 *            • Packing/endianness nâng cao hoặc bit-length khác cần
 *              mở rộng thêm ở phần Com.c.
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/
#ifndef COM_CFG_H
#define COM_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"
#include "ComStack_Types.h"   /* PduIdType, PduInfoType */

#define CANID_ENGINE_DATA 0x200
#define CANID_VCU_COMMAND 0x100

/* ====================================================================
 * 1) CẤU HÌNH & BỘ ĐỆM I-PDU (DEMO)
 * ===================================================================*/
/* -------- I-PDU TX: VCU_Command (5 byte)
 *  Byte0: Throttle (u8)
 *  Byte1: GearSel  (u8)
 *  Byte2: DriveMode(u8)
 *  Byte3: Alive nibble (bits 0..3), bits 4..7 = 0
 *  Byte4: BrakeActive (u8: 0/1)
 */
extern uint8_t s_TxBuf_VcuCommand[5u];

/* -------- I-PDU RX: Engine_Status (2 byte)
 *  Byte0..1: EngineSpeedRpm (big-endian: rpm = (b0<<8) | b1)
 */
extern uint8_t s_RxBuf_EngStatus[2u];
/* =========================================================
 * 2) Hướng I-PDU
 * =======================================================*/
typedef enum {
    COM_PDU_DIR_RX = 0u,   /**< I-PDU nhận (Rx từ bus)  */
    COM_PDU_DIR_TX = 1u    /**< I-PDU phát (Tx ra bus)  */
} Com_PduDirection_e;

/* =========================================================
 * 3) Kiểu dữ liệu tín hiệu (demo)
 *    - COM_SIGTYPE_UINT8   : 1 byte
 *    - COM_SIGTYPE_BOOLEAN : 1 bit hoặc 1 byte (tuỳ cấu hình)
 *    - COM_SIGTYPE_UINT16  : 2 byte (ở ví dụ RX: big-endian)
 * =======================================================*/
typedef enum {
    COM_SIGTYPE_UINT8   = 0u,
    COM_SIGTYPE_BOOLEAN = 1u,
    COM_SIGTYPE_UINT16  = 2u
} Com_SignalType_e;

/* =========================================================
 * 4) Cấu hình Signal ↔ I-PDU
 *    - PduId      : I-PDU chứa tín hiệu
 *    - byteIndex  : vị trí byte trong I-PDU (0..Length-1)
 *    - bitOffset  : bit offset trong byte (0..7) cho bit/nibble
 *    - bitLength  : 1/4/8 (demo); 16 cho RX uint16 (đặt trọn 2 byte)
 *    - type       : kiểu dữ liệu (xem trên)
 *    - direction  : hướng tín hiệu (TX hoặc RX)
 *
 *  Ghi chú:
 *    • Với bitLength = 8: tín hiệu chiếm trọn 1 byte ở byteIndex.
 *    • Với bitLength = 4: tín hiệu chiếm nibble tại bitOffset (0..3).
 *    • Với bitLength = 1: tín hiệu là 1 bit tại bitOffset (0..7).
 *    • Với bitLength = 16 và type = UINT16 (ví dụ RX): tín hiệu chiếm
 *      2 byte liên tiếp bắt đầu tại byteIndex, và được hiểu big-endian
 *      ở ví dụ Com.c.
 * =======================================================*/
typedef struct {
    PduIdType           PduId;
    uint16_t            byteIndex;
    uint8_t             bitOffset;
    uint8_t             bitLength;
    Com_SignalType_e    type;
    Com_PduDirection_e  direction;
} Com_SignalCfgType;

/* =========================================================
 * 5) Cấu hình I-PDU
 *    - PduId     : ID tượng trưng của I-PDU
 *    - Length    : độ dài payload (byte)
 *    - direction : RX hoặc TX
 * =======================================================*/
typedef struct {
    PduIdType           PduId;
    PduLengthType       Length;
    Com_PduDirection_e  direction;
} Com_IPduCfgType;

/* =========================================================
 * 6) SYMBOLIC IDs (ví dụ phù hợp với RTE demo)
 *    - VCU_Command (TX): chứa các tín hiệu điều khiển từ ECU
 *    - Engine_Status (RX): ví dụ nhận tốc độ động cơ (rpm)
 * =======================================================*/

/* ---- TX: VCU_Command ---- */
#define ComConf_ComSignal_VCU_ThrottleReq_pct   ((Com_SignalIdType)0u)  /**< uint8, 0..100%    */
#define ComConf_ComSignal_VCU_GearSel           ((Com_SignalIdType)1u)  /**< uint8, 0..3 (P/R/N/D) */
#define ComConf_ComSignal_VCU_DriveMode         ((Com_SignalIdType)2u)  /**< uint8, 0..1 (ECO/NORMAL) */
#define ComConf_ComSignal_VCU_BrakeActive       ((Com_SignalIdType)3u)  /**< boolean (0/1)     */
#define ComConf_ComSignal_VCU_Alive             ((Com_SignalIdType)4u)  /**< uint8 nibble 0..15 */
#define ComConf_ComIPdu_VCU_Command             ((PduIdType)0u)         /**< I-PDU TX          */

/* ---- RX: Engine_Status ---- */
#define ComConf_ComSignal_EngineSpeedRpm        ((Com_SignalIdType)5u) /**< uint16 (BE)       */
#define ComConf_ComIPdu_Engine_Status           ((PduIdType)1u)         /**< I-PDU RX          */

/* ---- Số lượng phần tử (để kiểm tra biên, vòng lặp tra bảng) ---- */
#define COM_NUM_SIGNALS   (6u)   /**< 5 TX + 1 RX */
#define COM_NUM_IPDUS     (2u)   /**< 1 TX + 1 RX */

/* =========================================================
 * 7) BẢNG CẤU HÌNH & TRUY CẬP BUFFER (được định nghĩa ở Com.c)
 * =======================================================*/
/* -------- Bảng I-PDU (symbolic IDs do Com_Cfg.h cung cấp) -------- */
extern const Com_IPduCfgType Com_IPduCfg[COM_NUM_IPDUS];

/* -------- Bảng Signal (ví dụ demo) -------------------------------- */
extern const Com_SignalCfgType Com_SignalCfg[COM_NUM_SIGNALS];


/**
 * @brief   Lấy con trỏ buffer TX nội bộ của COM theo PduId.
 * @param   pduId   ID I-PDU TX.
 * @param   len     [out] Độ dài buffer (byte) nếu không NULL.
 * @return  Con trỏ buffer TX, hoặc NULL nếu pduId không phải TX.
 */
extern uint8_t* Com_GetTxBufferPtr(PduIdType pduId, PduLengthType* len);

/**
 * @brief   Lấy con trỏ buffer RX nội bộ của COM theo PduId.
 * @param   pduId   ID I-PDU RX.
 * @param   len     [out] Độ dài buffer (byte) nếu không NULL.
 * @return  Con trỏ buffer RX, hoặc NULL nếu pduId không phải RX.
 */
extern uint8_t* Com_GetRxBufferPtr(PduIdType pduId, PduLengthType* len);

#ifdef __cplusplus
}
#endif

#endif /* COM_CFG_H */
