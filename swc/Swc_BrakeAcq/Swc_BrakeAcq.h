/**********************************************************
 * @file    Swc_BrakeAcq.h
 * @brief   SWC – Brake Acquisition (đọc & lọc tín hiệu phanh)
 * @details Thành phần phần mềm (Software Component) chịu trách
 *          nhiệm:
 *            - Lấy mẫu tín hiệu phanh thô từ lớp IoHwAb (DIO/cảm biến)
 *              thông qua RTE (Client/Server).
 *            - Lọc nhiễu/debounce theo thời gian.
 *            - Phát hành (publish) trạng thái phanh ổn định qua RTE
 *              (Sender/Receiver – Provide port).
 *
 *          Chu kỳ thực thi đề xuất: 10 ms (Run10ms).
 *
 *          Phụ thuộc:
 *            - Std_Types.h (kiểu chuẩn AUTOSAR).
 *            - Rte.h (API RTE – Rte_Call/Rte_Write).
 *            - IoHwAb (được gọi gián tiếp qua RTE).
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#ifndef SWC_BRAKEACQ_H
#define SWC_BRAKEACQ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"

/**
 * @brief Khởi tạo nội bộ SWC (trạng thái, debounce, seed giá trị ban đầu).
 * @note  Gọi trong InitTask sau khi Rte_Init().
 */
void Swc_BrakeAcq_Init(void);

/**
 * @brief Hàm định kỳ 10 ms: đọc tín hiệu phanh, debounce, publish ra RTE.
 * @note  Gọi từ Task_10ms (hay lịch tương đương).
 */
void Swc_BrakeAcq_Run10ms(void);

#ifdef __cplusplus
}
#endif
#endif /* SWC_BRAKEACQ_H */
