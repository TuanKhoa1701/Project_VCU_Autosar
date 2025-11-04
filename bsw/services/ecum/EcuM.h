/**
 * @file      EcuM.h
 * @author    Nguyễn Tuấn Khoa
 * @details   Khai báo API cho module EcuM (ECU State Manager) phiên bản tối giản.
 *            Module này quản lý các trạng thái chính của ECU, tuân theo một luồng
 *            khởi động rút gọn của AUTOSAR Classic, bao gồm:
 *            1. main() gọi EcuM_Init()
 *            2. StartOS() được gọi, hệ điều hành khởi động Task Init.
 *            3. InitTask gọi EcuM_StartupTwo() để hoàn tất quá trình khởi động.
 *
 * @version   1.1
 * @date      2024-07-27
 */

#ifndef ECUM_H
#define ECUM_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Liệt kê các trạng thái hoạt động chính của ECU.
 * @details Các trạng thái này mô tả vòng đời của ECU từ khi chưa khởi tạo cho đến
 *          khi vận hành và tắt máy.
 */
typedef enum {
    ECU_STATE_UNINIT,           /**< Trạng thái ban đầu, trước khi EcuM_Init được gọi. */
    ECU_STATE_STARTUP_ONE,      /**< Pha khởi động 1 (Pre-OS), sau khi EcuM_Init() được gọi. */
    ECU_STATE_STARTUP_TWO,      /**< Pha khởi động 2 (Post-OS), trong khi EcuM_StartupTwo() đang chạy. */
    ECU_STATE_RUN,              /**< Trạng thái vận hành bình thường, hệ thống đã sẵn sàng. */
    ECU_STATE_SHUTDOWN          /**< Trạng thái tắt máy, chuẩn bị dừng hệ thống. */
} EcuM_stateType;

/**
 * @brief   Thực hiện pha khởi động đầu tiên (Pre-OS).
 * @details Hàm này được gọi từ `main()` trước khi hệ điều hành bắt đầu.
 *          Nó chuyển trạng thái ECU sang STARTUP_ONE.
 *  @file     : EcuM.h
 */
void EcuM_Init(void);

/**
 * @brief   Thực hiện pha khởi động thứ hai (Post-OS).
 * @details Hàm này được gọi từ một Task sau khi hệ điều hành đã chạy.
 *          Nó thực hiện các khởi tạo phần cứng mức thấp (ví dụ: clock, flash)
 *          bằng cách gọi `SystemInit()` và chuyển ECU sang trạng thái RUN.
 */
void EcuM_StartupTwo(void);

#ifdef __cplusplus
}
#endif

#endif /* ECUM_H */
