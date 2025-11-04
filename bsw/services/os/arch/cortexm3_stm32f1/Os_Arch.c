/*
 * ============================================================
 *  OS Port Layer (Cortex-M3 / STM32F10x)
 *  - Dựa vào CMSIS: stm32f10x.h + core_cm3.h
 *  - Không hiện thực PendSV_Handler/SVC_Handler (để trong ASM riêng)
 *  - Gọi lại kernel os_on_tick() mỗi nhịp SysTick
 * ============================================================
 */
#include "Os_Arch.h"
#include "Os.h"
#include "stm32f10x.h" /* CMSIS device header: SystemCoreClock, SysTick, NVIC_*, IRQn defs */
#include "core_cm3.h"

/* ============================================================
 *  Ghi chú về ưu tiên ngắt trên ARMv7-M (Cortex-M3):
 *  - Giá trị PRIORITY càng LỚN → ưu tiên CÀNG THẤP.
 *  - Khuyến nghị: PendSV để THẤP NHẤT để tránh preempt trong lúc đổi ngữ cảnh.
 *  - SysTick nhỉnh hơn PendSV một chút (cao hơn ưu tiên), để có thể lên lịch rồi
 *    yêu cầu PendSV thực thi sau.
 *  - SVCall ở mức trung bình (dùng để "launch" task đầu tiên và các syscall).
 *
 *  __NVIC_PRIO_BITS định nghĩa số bit ưu tiên có hiệu lực (STM32F1 thường = 4).
 *  NVIC_SetPriority() dùng giá trị "thô", không cần tự dịch bit.
 * ============================================================
 */
/* ====== (tuỳ chọn) macro encode nếu bạn muốn thao tác byte ưu tiên thô ====== */
static inline uint32_t prv_prio_lowest(void) {
    return (1u << __NVIC_PRIO_BITS) - 1u; /* ví dụ: 4 bit → 0x0F */
}
/* --------------------------------------------------------------------------
 * Exception priorities (0 = highest, 15 = lowest on STM32F1 with 4-bit prio):
 *   - PendSV: lowest priority (15) → runs after all other ISRs, ideal for context switch
 *   - SysTick: slightly above PendSV (14) → tick can pend a switch which happens right after
 *   - SVCall: above SysTick (13) → cooperative yield via SVC can request a switch cleanly
 * -------------------------------------------------------------------------- */
static void Os_Arch_SetCorePriorities(void)
{
    NVIC_SetPriority(PendSV_IRQn, 0xFF);  /* lowest */
    NVIC_SetPriority(SysTick_IRQn, 0xFE); /* lower than most, higher than PendSV */
    NVIC_SetPriority(SVCall_IRQn, 0xFD);  /* above SysTick */
    __set_BASEPRI(0);                     // đừng chặn theo mức
}

/* --------------------------------------------------------------------------
 * Configure SysTick to fire at `tick_hz`.
 * - Uses HCLK as the clock source.
 * - Ensures LOAD fits the 24-bit SysTick counter (clamps if necessary).
 * - Enables interrupt and the counter.
 * -------------------------------------------------------------------------- */
void Os_Arch_Init( )
{
    /* 1) Ưu tiên: PendSV thấp nhất; SysTick ngay trên; SVCall ở giữa */
    Os_Arch_SetCorePriorities();
    /* 2) Bật căn chỉnh stack 8-byte khi vào ngắt (theo AAPCS) */
    SCB->CCR |= SCB_CCR_STKALIGN_Msk;
    /* 3) Bật SysTick theo OS_TICK_HZ (mặc định 1000 Hz nếu không đổi) */
    OS_Arch_SystickConfig(OS_TICK_HZ);
}
/* (Tùy chọn) Cấu hình lại SysTick theo tần số tùy ý (Hz) */

void OS_Arch_SystickConfig(uint32_t hz){
            /* Compute reload from current core clock */
    /* If your startup doesn't call SystemInit()/SystemCoreClockUpdate() before main,
       ensure SystemCoreClock has the correct value here. */
    uint32_t reload = SystemCoreClock / hz;
    if (reload == 0u)
        {
        reload = 1u;
        }
    if (reload > 0x00FFFFFFu)
        {
        reload = 0x00FFFFFFu;
        } /* clamp to 24-bit max */

    /* Stop SysTick while reconfiguring */
    SysTick->CTRL = 0u;

    /* Program reload and clear current value */
    SysTick->LOAD = (reload - 1u);
    SysTick->VAL = 0u;

    /* CLKSOURCE = 1 (HCLK), TICKINT = 1 (cho phép IRQ), ENABLE = 1 (bật) */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;
}


/* =========================================================
 * 4) OS_Arch_TriggerPendSV — yêu cầu chuyển ngữ cảnh
 * =======================================================*/
/**
 * @brief  Đặt bit PENDSVSET để kích hoạt PendSV khi phù hợp.
 * @note   Gọi được từ TASK hoặc từ ISR khác.
 */
void Os_Arch_TriggerPendSV(void){
    /* Xoá BASEPRI (nếu kernel từng dùng), cho phép mọi mức ưu tiên */
    __set_BASEPRI(0);
    __DSB();
    __ISB();

    /* Set bit PENDSVSET trong ICSR */
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

    __DSB();
    __ISB();
}

void Os_Arch_StartFirstTask(void){
    __ASM volatile("svc 0");
}
