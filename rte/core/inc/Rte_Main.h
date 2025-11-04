#ifndef RTE_MAIN_H
#define RTE_MAIN_H

#include "Rte.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* ===========================================================
     * RTE Lifecycle API  (khai báo – thân hàm đặt ở rte/core/src/Rte.c)
     * =========================================================== */
    void Rte_Tick10ms(void); /* Được OS gọi mỗi 10ms (nếu có) */
    /** Khởi động RTE (khởi tạo buffers, trạng thái, binding, ...) */
    Std_ReturnType Rte_Start(void);

    /** Dừng RTE (giải phóng hoặc đưa về trạng thái an toàn trước khi tắt ECU) */
    Std_ReturnType Rte_Stop(void);

    /** Chạy nhóm initialization runnables (nếu có) / khởi tạo SWC mức RTE */
    void Rte_Init_Core(void);

    /** Cho phép các Timing/Background Event bắt đầu chạy (ví dụ tick 10ms) */
    void Rte_StartTiming(void);

    /* (tùy chọn) Nếu bạn phân nhóm init khác nhau, có thể thêm:
     * void Rte_Init_<GroupA>(void);
     * void Rte_Init_<GroupB>(void);
     */

#ifdef __cplusplus
}
#endif

#endif /* RTE_MAIN_H */
