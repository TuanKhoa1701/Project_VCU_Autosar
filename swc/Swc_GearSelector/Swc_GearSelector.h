/**********************************************************
 * @file    Swc_GearSelector.h
 * @brief   SWC – Gear Selector (đọc & quản lý vị trí cần số)
 * @details Thành phần phần mềm chịu trách nhiệm:
 *          - Đọc vị trí số thô từ IoHwAb (công tắc/cảm biến) thông
 *            qua RTE (Client/Server).
 *          - Lọc nhiễu/debounce theo thời gian.
 *          - Phát hành (publish) vị trí số ổn định qua RTE
 *            (Sender/Receiver – Provide port).
 *
 *          Chu kỳ thực thi đề xuất: 10 ms (Run10ms).
 *
 *          Phụ thuộc:
 *            - Std_Types.h  (kiểu chuẩn AUTOSAR)
 *            - Rte.h, Rte_Type.h (API/kiểu RTE)
 *
 * @version 1.0
 * @date    2025-09-10 
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#ifndef SWC_GEARSELECTOR_H
#define SWC_GEARSELECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"

/**
 * @brief Khởi tạo nội bộ SWC (seed giá trị ban đầu, reset bộ lọc).
 * @note  Gọi trong InitTask sau khi Rte_Init().
 */
void Swc_GearSelector_Init(void);

/**
 * @brief Hàm định kỳ 10 ms: đọc vị trí số, debounce, publish ra RTE.
 * @note  Gọi từ Task_10ms (hoặc lịch tương đương).
 */
void Swc_GearSelector_Run10ms(void);

#ifdef __cplusplus
}
#endif
#endif /* SWC_GEARSELECTOR_H */
