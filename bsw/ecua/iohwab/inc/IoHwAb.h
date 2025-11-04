/**********************************************************
 * @file    IoHwAb.h
 * @brief   I/O Hardware Abstraction (IoHwAb) – Giao diện dịch vụ phần cứng
 * @details Lớp trừu tượng phần cứng theo phong cách AUTOSAR dùng cho dự án:
 *          - Cung cấp API **đồng bộ** để SWC truy cập phần cứng qua RTE
 *            (Client/Server), ẩn chi tiết MCU/driver (ADC/GPIO/SPL/HAL).
 *          - Chuẩn hoá dữ liệu về đơn vị logic hệ thống:
 *              • Pedal (ga)      : 0..100 (%)
 *              • Brake (phanh)   : boolean (TRUE khi đạp)
 *              • Gear (hộp số)   : Gear_e (P/R/N/D) + cờ hợp lệ
 *              • Drive Mode      : DriveMode_e (ECO/NORMAL)
 *
 *          Quy ước:
 *            - Tất cả API trả về Std_ReturnType:
 *                 E_OK     = thành công, *out hợp lệ
 *                 E_NOT_OK = lỗi/tham số NULL/driver chưa sẵn sàng
 *            - Con trỏ đầu ra (*out) **không được NULL**.
 *            - API mặc định **không tái nhập** (non-reentrant) trừ khi
 *              hiện thực có bảo vệ (IRQ lock/mutex/critical section).
 *            - Khuyến nghị gọi từ Task định kỳ (ví dụ 10 ms), không gọi
 *              từ ISR trừ khi driver đảm bảo đủ nhanh/an toàn.
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#ifndef IOHWAB_H
#define IOHWAB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"   /* boolean, uint8/16, Std_ReturnType, E_OK/E_NOT_OK */
#include "Rte_Types.h"    /* Gear_e, DriveMode_e (tách riêng để tránh vòng include) */
#include <stddef.h>

#define ADC_GROUP_PEDAL 0u
/* =========================================================
 * 1) Pedal – % đạp ga (0..100)
 * ---------------------------------------------------------
 * Mô tả:
 *   Đọc vị trí bàn đạp ga từ phần cứng (ADC/cảm biến) và trả về
 *   phần trăm 0..100 đã được tuyến tính hoá/clamp ở tầng driver.
 *
 * @param[out] pct  Con trỏ nhận giá trị % (0..100)
 * @return     E_OK nếu đọc thành công; E_NOT_OK nếu lỗi/NULL
 * =======================================================*/
Std_ReturnType IoHwAb_Pedal_ReadPct(uint8_t* pct);

/* =========================================================
 * 2) Brake – trạng thái bàn đạp phanh
 * ---------------------------------------------------------
 * Mô tả:
 *   Đọc tín hiệu công tắc/cảm biến phanh và quy về boolean.
 *
 * @param[out] pressed  TRUE nếu phanh đang tác dụng, ngược lại FALSE
 * @return     E_OK nếu đọc thành công; E_NOT_OK nếu lỗi/NULL
 * =======================================================*/
Std_ReturnType IoHwAb_Brake_Get(boolean* pressed);

/* =========================================================
 * 3) Gear – vị trí cần số (P/R/N/D) + cờ hợp lệ
 * ---------------------------------------------------------
 * Mô tả:
 *   Đọc cảm biến vị trí số và ánh xạ sang enum Gear_e. Một số trạng thái
 *   quá độ có thể được báo invalid để SWC xử lý debounce/logic an toàn.
 *
 * @param[out] gear   Giá trị Gear_e (GEAR_P/GEAR_R/GEAR_N/GEAR_D)
 * @param[out] valid  TRUE nếu giá trị hợp lệ; FALSE nếu không chắc chắn
 * @return     E_OK nếu đọc thành công; E_NOT_OK nếu lỗi/NULL
 * =======================================================*/
Std_ReturnType IoHwAb_Gear_Get(Gear_e* gear, boolean* valid);

/* =========================================================
 * 4) Drive Mode – chế độ lái (ECO/NORMAL)
 * ---------------------------------------------------------
 * Mô tả:
 *   Đọc trạng thái công tắc/chế độ từ ECU khác rồi quy về DriveMode_e.
 *
 * @param[out] mode  DRIVEMODE_ECO hoặc DRIVEMODE_NORMAL
 * @return     E_OK nếu đọc thành công; E_NOT_OK nếu lỗi/NULL
 * =======================================================*/
Std_ReturnType IoHwAb_Mode_Get(DriveMode_e* mode);

#ifdef __cplusplus
}
#endif
#endif /* IOHWAB_H */
