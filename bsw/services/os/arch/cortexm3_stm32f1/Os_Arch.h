/**********************************************************
 * @file    Os_Arch.h
 * @brief   Lớp glue phụ thuộc kiến trúc cho OS (Cortex-M3, STM32F103)
 * @details Cung cấp các primitive ở mức CPU/exception để kernel OS vận hành:
 *          - Khởi tạo kiến trúc (PSP/MSP, ưu tiên PendSV/SysTick)
 *          - Gọi PendSV để chuyển ngữ cảnh
 *          - Khởi động task đầu tiên (exception return → THREAD mode, PSP)
 *          - Cấu hình SysTick theo tần số Hz
 *          - Hook ISR tick ở C (os_tick_handler) do handler ASM/C gọi tới
 *
 *          Gợi ý triển khai (CMSIS):
 *            - Ưu tiên PendSV thấp nhất (ví dụ 0xF-lowest với 4 bit)
 *              NVIC_SetPriority(PendSV_IRQn, 0xF);
 *            - SysTick thấp nhưng cao hơn PendSV (ví dụ 0xE):
 *              NVIC_SetPriority(SysTick_IRQn, 0xE);
 *            - Cấu hình SysTick 1 kHz:
 *              SysTick_Config(SystemCoreClock / 1000u);
 *            - Kích PendSV: SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
 *
 *          Dòng chảy khởi động điển hình:
 *            StartOS():
 *              OS_Arch_Init();
 *              OS_Arch_SystickConfig(1000);     // 1ms/tick
 *              // chuẩn bị TCB đầu tiên (stack frame fake + PSP)
 *              OS_Arch_StartFirstTask();        // không quay lại
 *
 * @version  1.0
 * @date     2025-09-10
 * @author   Nguyễn Tuấn Khoa
 **********************************************************/
#ifndef OS_ARCH_H
#define OS_ARCH_H

#include "Std_Types.h"
#include "Os_Types.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

    /**********************************************************
     * OS_Arch_Init
     *  - Thiết lập những thứ “một lần” ở mức CPU:
     *      * Chọn PSP cho THREAD mode (CONTROL.SPSEL = 1)
     *      * Đặt ưu tiên PendSV thấp nhất, SysTick thấp (nhưng > PendSV)
     *      * Tắt/clear các pending không mong muốn
     *  - Không bật SysTick ở đây nếu muốn cấu hình Hz riêng: dùng
     *    OS_Arch_SystickConfig() để bật theo tần số mong muốn.
     **********************************************************/
    void Os_Arch_Init(void);

    /**********************************************************
     * OS_Arch_SystickConfig
     *  @param hz  Tần số ngắt SysTick (Hz), ví dụ 1000 cho 1ms
     *  - Thiết lập tải lại SysTick theo SystemCoreClock/hz
     *  - Đặt ưu tiên SysTick (thấp hơn các ngắt thời gian thực gấp),
     *    nhưng cao hơn PendSV để luôn xử lý tick trước khi switch.
     *  - Bật SysTick (CLKSOURCE=CPU, TICKINT=1, ENABLE=1)
     **********************************************************/
    void OS_Arch_SystickConfig(uint32_t hz);

    /* --------------------------------------------------------------
     * Start first task in Thread mode using PSP.
     *  - 'psp' must point to a stack prebuilt with:
     *      [r4..r11 saved area][xPSR, PC, LR, r12, r3, r2, r1, r0]
     *  - Does not return.
     * -------------------------------------------------------------- */
    void Os_Arch_StartFirstTask(void);

    /**********************************************************
     * OS_Arch_TriggerPendSV
     *  - Yêu cầu chuyển ngữ cảnh “trì hoãn” (deferred) bằng PendSV:
     *      SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
     *  - Gọi được ở ngữ cảnh TASK hoặc ISR khác (ví dụ sau khi tick 
     *    làm một task READY với ưu tiên cao hơn).
     **********************************************************/
    void Os_Arch_TriggerPendSV(void);

#ifdef __cplusplus
}
#endif

#endif /* OS_ARCH_H */
