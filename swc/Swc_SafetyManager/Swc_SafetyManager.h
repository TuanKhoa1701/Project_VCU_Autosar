/**********************************************************
 * @file    Swc_SafetyManager.h
 * @brief   SWC – Safety Manager (tổng hợp & áp ràng buộc an toàn)
 * @details Thành phần phần mềm chịu trách nhiệm:
 *          - Thu thập tín hiệu đã qua lọc/đảm bảo từ các SWC acquisition
 *            (Pedal, Brake, Gear, DriveMode) thông qua RTE (SR-Require).
 *          - Áp ràng buộc an toàn mức ứng dụng, bao gồm:
 *              + Interlock chuyển số: **Từ P sang R/D hoặc từ R/D về P
 *                bắt buộc đạp phanh** (brakeActive = TRUE).
 *              + Brake override: giới hạn % ga khi đang đạp phanh.
 *              + Timeout nguồn dữ liệu: fallback về giá trị an toàn.
 *          - Cung cấp gói dữ liệu an toàn Safe_s qua RTE (SR-Provide).
 *
 *          Chu kỳ thực thi đề xuất: 10 ms (Run10ms).
 *
 *          Phụ thuộc:
 *            - Std_Types.h, Rte.h, Rte_Type.h
 *
 * @version 1.1
 * @date    2025-09-10 
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#ifndef SWC_SAFETYMANAGER_H
#define SWC_SAFETYMANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"

/**
 * @brief Khởi tạo Safety Manager (reset trạng thái, seed mặc định).
 * @note  Gọi trong InitTask sau khi Rte_Init() và trước khi các SWC khác
 *        phụ thuộc vào Safe_s.
 */
void Swc_SafetyManager_Init(void);

/**
 * @brief Hàm định kỳ 10 ms: đọc tín hiệu, áp ràng buộc, xuất Safe_s.
 * @note  Gọi từ Task_10ms (hoặc lịch tương đương).
 */
void Swc_SafetyManager_Run10ms(void);

#ifdef __cplusplus
}
#endif
#endif /* SWC_SAFETYMANAGER_H */
