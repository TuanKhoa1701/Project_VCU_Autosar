/**********************************************************
 * @file    Rte.h
 * @brief   Runtime Environment (RTE) tối giản cho dự án demo
 * @details Cung cấp giao diện kiểu AUTOSAR giữa các SWC (Software
 *          Component) và lớp BSW (IoHwAb, COM). Thiết kế theo mô hình:
 *
 *          • Sender/Receiver (SR):
 *              - Rte_Write_*(): cổng Provide – SWC ghi dữ liệu dùng chung
 *              - Rte_Read_*():  cổng Require – SWC đọc dữ liệu dùng chung
 *
 *          • Client/Server (CS):
 *              - Rte_Call_*(): SWC gọi dịch vụ đồng bộ (IoHwAb)
 *
 *          • Proxy COM (Tx/Rx tối giản):
 *              - Rte_Write_*_VcuCmdTx_*(): ghi từng signal (shadow buffer)
 *              - Rte_Trigger_*_VcuCmdTx(): yêu cầu COM gửi khung (frame)
 *              - Rte_Com_Update_*FromPdu(): cập nhật dữ liệu vào RTE từ Rx PDU
 *
 *          Ghi chú:
 *            - Header KHÔNG chứa cơ chế log/hook yếu.
 *            - Phần hiện thực nằm trong Rte.c (hoặc các module tương ứng).
 *            - API mang phong cách AUTOSAR, trả về Std_ReturnType:
 *                 E_OK     = thao tác thành công
 *                 E_NOT_OK = lỗi tham số/nguồn chưa sẵn sàng/ngoài phạm vi
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#ifndef RTE_H
#define RTE_H

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================
 * 1) Kiểu chuẩn AUTOSAR + kiểu domain của RTE
 *    - Std_Types.h: boolean, uint8/16, Std_ReturnType, E_OK/E_NOT_OK
 *    - Rte_Type.h : Gear_e, DriveMode_e, Safe_s, EngineSpeedRpm_t...
 * =======================================================*/
#include "Std_Types.h"
#include "Rte_Types.h"
#include <stdio.h>  /* cho size_t */

/* (Tuỳ hệ thống) độ dài PDU Tx ví dụ – dùng khi cần đóng gói khung */
#ifndef RTE_VCUCMD_PDU_LEN
#define RTE_VCUCMD_PDU_LEN   8u
#endif

/* =========================================================
 * 2) Lifecycle
 * =======================================================*/
/**
 * @brief  Khởi tạo RTE: reset buffer, cờ cập nhật, proxy COM…
 * @note   Gọi trong InitTask trước khi các SWC sử dụng RTE.
 */
void Rte_Init(void);

/* =========================================================
 * 3) Sender/Receiver — Provide ports (SWC ghi dữ liệu)
 *    Quy ước trả về:
 *      - E_OK     : ghi thành công
 *      - E_NOT_OK : tham số NULL/ngoài phạm vi/không sẵn sàng
 * =======================================================*/
/**
 * @brief  PedalAcq cung cấp % đạp ga (0..100) vào RTE.
 * @param  v  Giá trị phần trăm (sẽ clamp 0..100 ở Rte.c nếu cần).
 */
Std_ReturnType Rte_Write_PedalAcq_PedalOut(uint8_t v);

/**
 * @brief  BrakeAcq cung cấp trạng thái bàn đạp phanh vào RTE.
 * @param  b  TRUE khi phanh đang tác dụng, FALSE nếu không.
 */
Std_ReturnType Rte_Write_BrakeAcq_BrakeOut(boolean b);

/**
 * @brief  GearSelector cung cấp vị trí số (P/R/N/D) vào RTE.
 * @param  g  Giá trị kiểu Gear_e.
 */
Std_ReturnType Rte_Write_GearSelector_GearOut(Gear_e g);

/**
 * @brief  DriveModeMgr cung cấp chế độ lái (ECO/NORMAL) vào RTE.
 * @param  m  Giá trị kiểu DriveMode_e.
 */
Std_ReturnType Rte_Write_DriveModeMgr_DriveModeOut(DriveMode_e m);

/**
 * @brief  SafetyManager cung cấp gói dữ liệu an toàn tổng hợp.
 * @param  s  Con trỏ tới Safe_s (không NULL).
 */
Std_ReturnType Rte_Write_SafetyManager_SafeOut(const Safe_s* s);

/**
 * @brief  (Tuỳ chọn) Lớp COM/diagnostic cập nhật tốc độ động cơ vào RTE.
 * @param  rpm  Tốc độ động cơ (vòng/phút).
 */
Std_ReturnType Rte_Write_Com_EngineSpeed(EngineSpeedRpm_t rpm);

/* =========================================================
 * 4) Sender/Receiver — Require ports (SWC đọc dữ liệu)
 *    Quy ước trả về:
 *      - E_OK     : đọc thành công, *out hợp lệ (đã từng được cập nhật)
 *      - E_NOT_OK : tham số NULL hoặc nguồn chưa từng cập nhật
 * =======================================================*/
/* SafetyManager đọc các tín hiệu đã được các acquisition SWC cung cấp */
Std_ReturnType Rte_Read_SafetyManager_PedalOut(uint8_t* v);
Std_ReturnType Rte_Read_SafetyManager_BrakeOut(boolean* b);
Std_ReturnType Rte_Read_SafetyManager_GearOut(Gear_e* g);
Std_ReturnType Rte_Read_SafetyManager_DriveModeOut(DriveMode_e* m);

/* CmdComposer đọc gói Safe_s do SafetyManager cung cấp */
Std_ReturnType Rte_Read_CmdComposer_SafeOut(Safe_s* s);

/* Ví dụ: MotorCtrl tiêu thụ tốc độ động cơ đã lưu trong RTE */
Std_ReturnType Rte_Read_MotorCtrl_EngineSpeed(EngineSpeedRpm_t* rpm);

/* =========================================================
 * 5) Client/Server — gọi dịch vụ IoHwAb (đồng bộ)
 *    Lưu ý:
 *      - RTE chỉ uỷ quyền xuống IoHwAb; driver chịu trách nhiệm thời gian.
 *      - Không gọi từ ISR nếu IoHwAb không hỗ trợ ngữ cảnh ISR.
 * =======================================================*/
/**
 * @brief  Đọc % đạp ga từ lớp IoHwAb (ADC/điện trở/mạch đo).
 * @param  pct  Con trỏ nhận giá trị (0..100).
 */
Std_ReturnType Rte_Call_PedalAcq_IoHwAb_Pedal_ReadPct(uint8_t* pct);

/**
 * @brief  Đọc trạng thái phanh từ IoHwAb (GPIO/công tắc).
 * @param  pressed  Con trỏ nhận TRUE/FALSE.
 */
Std_ReturnType Rte_Call_BrakeAcq_IoHwAb_Brake_Get(boolean* pressed);

/**
 * @brief  Đọc vị trí số và tính hợp lệ từ IoHwAb (cảm biến vị trí).
 * @param  gear   Con trỏ nhận giá trị Gear_e.
 * @param  valid  Con trỏ nhận TRUE nếu đọc hợp lệ.
 */
Std_ReturnType Rte_Call_GearSelector_IoHwAb_Gear_Get(Gear_e* gear, boolean* valid);

/**
 * @brief  Đọc chế độ lái từ IoHwAb (công tắc/ECU khác).
 * @param  mode  Con trỏ nhận DriveMode_e.
 */
Std_ReturnType Rte_Call_DriveModeMgr_IoHwAb_Mode_Get(DriveMode_e* mode);

/* =========================================================
 * 6) Proxy → COM cho PDU VCU_Command (Tx — theo từng signal)
 *    - Các API ghi từng signal vào shadow buffer trong RTE.
 *    - Rte_Trigger_* yêu cầu lớp COM phát frame ra bus (CAN/LIN…).
 *    - CRC/Checksum: xử lý ở COM nếu hệ thống yêu cầu, RTE không tính.
 * =======================================================*/
/**
 * @brief  Ghi signal Throttle Request (%).
 * @param  v  0..100.
 */
Std_ReturnType Rte_Write_CmdComposer_VcuCmdTx_ThrottleReq_pct(uint8_t v);

/**
 * @brief  Ghi signal Gear Select.
 * @param  gear  0..3 tương ứng Gear_e (P=0, R=1, N=2, D=3).
 */
Std_ReturnType Rte_Write_CmdComposer_VcuCmdTx_GearSel(uint8_t gear);

/**
 * @brief  Ghi signal Drive Mode.
 * @param  mode  0..1 tương ứng DriveMode_e (ECO=0, NORMAL=1).
 */
Std_ReturnType Rte_Write_CmdComposer_VcuCmdTx_DriveMode(uint8_t mode);

/**
 * @brief  Ghi signal Brake Active.
 * @param  b  TRUE/FALSE.
 */
Std_ReturnType Rte_Write_CmdComposer_VcuCmdTx_BrakeActive(boolean b);

/**
 * @brief  Ghi signal Alive Counter (4-bit thấp).
 * @param  nibble  0..15.
 */
Std_ReturnType Rte_Write_CmdComposer_VcuCmdTx_AliveCounter(uint8_t nibble);


/**
 * @brief  Yêu cầu COM phát các signal đã ghi (một khung truyền).
 * @note   RTE không đóng gói PDU/CRC; COM chịu trách nhiệm.
 */
Std_ReturnType Rte_Trigger_CmdComposer_VcuCmdTx(void);

/* =========================================================
 * 7) COM parser (Rx) — cập nhật dữ liệu vào RTE từ PDU nhận
 *    Ví dụ: parse frame chứa EngineSpeed và đẩy vào RTE.
 * =======================================================*/
/**
 * @brief  Cập nhật EngineSpeed vào RTE từ dữ liệu PDU nhận.
 * @param  data  Con trỏ buffer dữ liệu Rx.
 * @param  dlc   Số byte hợp lệ trong buffer.
 */
void Rte_Com_Update_EngineSpeedFromPdu(const uint16_t* data);

#ifdef __cplusplus
}
#endif
#endif /* RTE_H */
