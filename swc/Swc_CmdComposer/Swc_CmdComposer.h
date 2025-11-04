/**********************************************************
 * @file    Swc_CmdComposer.h
 * @brief   SWC – Command Composer (biên soạn lệnh VCU)
 * @details Thành phần phần mềm chịu trách nhiệm:
 *          - Nhận gói an toàn Safe_s (throttle/gear/mode/brake) qua RTE.
 *          - Ánh xạ các trường sang từng signal của VCU_Command Tx.
 *          - Tăng alive counter 4-bit và yêu cầu COM phát frame.
 *
 *          Thiết kế:
 *            • Input (RTE SR-Require)  : Safe_s từ SafetyManager.
 *            • Output (RTE → COM)      : các signal VCU_Command theo từng API.
 *            • Chu kỳ thực thi đề xuất : 10 ms (Run10ms).
 *
 *          Ràng buộc:
 *            - Không tính CRC/checksum trong SWC; COM sẽ đảm trách.
 *            - Có thể thêm chiến lược bão hoà/giới hạn nếu cần.
 *
 * @version 1.0
 * @date    2025-09-10 
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#ifndef SWC_CMDCOMPOSER_H
#define SWC_CMDCOMPOSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"

/**
 * @brief Khởi tạo CmdComposer (reset alive counter, seed trạng thái).
 * @note  Gọi trong InitTask sau Rte_Init() và sau SafetyManager_Init().
 */
void Swc_CmdComposer_Init(void);

/**
 * @brief Hàm định kỳ 10 ms: đọc Safe_s, ghi signal VCU_Command và trigger gửi.
 * @note  Gọi từ Task_10ms (hoặc lịch tương đương).
 */
void Swc_CmdComposer_Run10ms(void);

void Swc_CmdComposer_ReadEngineRPM(const uint16_t* data);
#ifdef __cplusplus
}
#endif
#endif /* SWC_CMDCOMPOSER_H */
