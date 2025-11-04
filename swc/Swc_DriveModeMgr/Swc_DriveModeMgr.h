/**********************************************************
 * @file    Swc_DriveModeMgr.h
 * @brief   SWC – Drive Mode Manager (đọc & quản lý chế độ lái)
 * @details Thành phần phần mềm chịu trách nhiệm:
 *          - Lấy mẫu chế độ lái thô từ lớp IoHwAb (công tắc/ECU khác)
 *            thông qua RTE (Client/Server).
 *          - Lọc nhiễu/debounce theo thời gian để có giá trị ổn định.
 *          - Phát hành (publish) chế độ lái ổn định qua RTE
 *            (Sender/Receiver – Provide port).
 *
 *          Chu kỳ thực thi đề xuất: 10 ms (Run10ms).
 *
 *          Phụ thuộc:
 *            - Std_Types.h  (kiểu chuẩn AUTOSAR)
 *            - Rte.h        (API RTE – Rte_Call/Rte_Write/Rte_Read)
 *
 * @version 1.0
 * @date    2025-09-10 
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#ifndef SWC_DRIVEMODEMGR_H
#define SWC_DRIVEMODEMGR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"

/**
 * @brief Khởi tạo nội bộ SWC (seed giá trị ban đầu, reset bộ lọc).
 * @note  Gọi trong InitTask sau khi Rte_Init().
 */
void Swc_DriveModeMgr_Init(void);

/**
 * @brief Hàm định kỳ 10 ms: đọc chế độ lái, debounce, publish ra RTE.
 * @note  Gọi từ Task_10ms (hoặc lịch tương đương).
 */
void Swc_DriveModeMgr_Run10ms(void);

#ifdef __cplusplus
}
#endif
#endif /* SWC_DRIVEMODEMGR_H */
