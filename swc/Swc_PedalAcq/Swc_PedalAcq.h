/**********************************************************
 * @file    Swc_PedalAcq.h
 * @brief   SWC – Pedal Acquisition (đọc & lọc % đạp ga)
 * @details Thành phần phần mềm chịu trách nhiệm:
 *          - Đọc % đạp ga thô từ lớp IoHwAb (ADC/mạch đo) thông
 *            qua RTE (Client/Server).
 *          - Lọc nhiễu bằng EMA (exponential moving average).
 *          - Giới hạn tốc độ thay đổi (rate limit) để tránh “giật”.
 *          - Phát hành (publish) % ga ổn định qua RTE
 *            (Sender/Receiver – Provide port).
 *
 *          Chu kỳ thực thi đề xuất: 10 ms (Run10ms).
 *
 *          Phụ thuộc:
 *            - Std_Types.h  (kiểu chuẩn AUTOSAR)
 *            - Rte.h        (API RTE – Rte_Call/Rte_Write)
 *
 * @version 1.0
 * @date    2025-09-10 
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#ifndef SWC_PEDALACQ_H
#define SWC_PEDALACQ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"

/**
 * @brief Khởi tạo nội bộ SWC (seed giá trị ban đầu, reset bộ lọc).
 * @note  Gọi trong InitTask sau khi Rte_Init().
 */
void Swc_PedalAcq_Init(void);

/**
 * @brief Hàm định kỳ 10 ms: đọc % ga, lọc EMA, rate-limit, publish.
 * @note  Gọi từ Task_10ms (hoặc lịch tương đương).
 */
void Swc_PedalAcq_Run10ms(void);

#ifdef __cplusplus
}
#endif
#endif /* SWC_PEDALACQ_H */
