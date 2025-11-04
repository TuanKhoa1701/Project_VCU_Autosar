/********************************************************************************
 *  @file     : EcuM.c (ECU State Manager)
 *  @author   : Nguyễn Tuấn Khoa
 *  @details  : Triển khai tối giản của module EcuM (ECU State Manager).
 *              Module này quản lý các trạng thái khởi động và vận hành cơ bản
 *              của ECU, tuân theo luồng rút gọn của AUTOSAR Classic.
 *              - EcuM_Init(): Thực hiện pha khởi động đầu tiên (Pre-OS).
 *              - EcuM_StartupTwo(): Thực hiện pha khởi động thứ hai (Post-OS),
 *                bao gồm khởi tạo phần cứng mức thấp.
 *
 ********************************************************************************/

#include "EcuM.h"
#include "stm32f10x.h" /* Để sử dụng SystemInit() */
#include <stdio.h>     /* Để sử dụng printf() cho mục đích log */
    
/**
 * @brief Biến lưu trữ trạng thái hiện tại của ECU.
 * @details Trạng thái này được quản lý bởi các hàm của EcuM và có thể được
 *          truy vấn bởi các module khác (nếu có API EcuM_GetState).
 */
static EcuM_stateType EcuM_State = ECU_STATE_UNINIT;

/**
 * @brief   Thực hiện pha khởi động đầu tiên của EcuM (Pre-OS).
 * @details Hàm này được gọi từ `main()` trước khi `StartOS()` được gọi.
 *          Nó chuyển trạng thái ECU sang `ECU_STATE_STARTUP_ONE`.
 *          Trong một hệ thống đầy đủ, pha này có thể bao gồm việc khởi tạo
 *          các driver cơ bản và kiểm tra cấu hình.
 */
void EcuM_Init(void)
{
    EcuM_State = ECU_STATE_STARTUP_ONE;
    printf("[EcuM] init -> StartupOne\n");
}

/**
 * @brief   Thực hiện pha khởi động thứ hai của EcuM (Post-OS).
 * @details Hàm này thường được gọi bởi một Task khởi tạo (ví dụ: InitTask)
 *          sau khi OS đã bắt đầu.
 *          - Chuyển trạng thái ECU sang `ECU_STATE_STARTUP_TWO`.
 *          - Gọi `SystemInit()` (từ CMSIS) để cấu hình clock hệ thống (RCC, PLL)
 *            và Flash latency.
 *          - Chuyển trạng thái ECU sang `ECU_STATE_RUN` để báo hiệu hệ thống
 *            đã sẵn sàng hoạt động.
 */
void EcuM_StartupTwo(void)
{
    EcuM_State = ECU_STATE_STARTUP_TWO;
    printf("[EcuM] StartupTwo -> Run\n");

    /* Khởi tạo clock hệ thống (PLL, HCLK, PCLKs) theo cấu hình trong system_stm32f10x.c */
    SystemInit();

    EcuM_State = ECU_STATE_RUN;
    printf("[EcuM] State: Run\n");
}
