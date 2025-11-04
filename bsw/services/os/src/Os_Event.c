/**********************************************************
 * @file    Os_Event.c
 * @brief   Event APIs cho Extended Task (AUTOSAR-like) trên STM32F103
 * @details Hiện thực 4 API tiêu chuẩn:
 *            - WaitEvent(mask)
 *            - SetEvent(t, mask)
 *            - GetEvent(t, *mask)
 *            - ClearEvent(mask)
 *
 *          Quy ước/Ngữ nghĩa (theo AUTOSAR/OSEK, rút gọn):
 *            - Extended Task mới được dùng Event. Basic Task gọi → lỗi E_OS_STATE.
 *            - WaitEvent(mask): nếu (events & mask) != 0 → trả ngay E_OK;
 *              ngược lại task chuyển sang WAITING với waitMask=mask, nhường CPU,
 *              tới khi có bất kỳ bit trong mask được set (qua SetEvent).
 *            - SetEvent(t, mask): OR bit vào events của task t; nếu t đang
 *              WAITING và (events & waitMask) != 0 → t về READY và clear waitMask.
 *              Có thể gọi từ TASK hoặc ISR. Nên kích chuyển ngữ cảnh khi ưu tiên
 *              của t cao hơn (ở đây gọi OS_Arch_TriggerPendSV() đơn giản).
 *            - GetEvent(t,*mask): đọc event hiện tại của task t (không clear).
 *            - ClearEvent(mask): xóa các bit trong events của CHÍNH task hiện tại.
 *
 *          Tính đồng bộ:
 *            - Event thường được chạm từ cả ISR (SetEvent) và Task (Wait/Clear/Get),
 *              vì thế cần vùng găng ngắn để tránh race conditions.
 *              Ở đây dùng cặp macro OS_CRIT_ENTER/EXIT (tắt/bật IRQ) cực ngắn.
 *
 * @version  1.0
 * @date     2025-07-10
 * @author   Nguyễn Tuấn Khoa
 **********************************************************/
#include "Os.h"
#include "Os_Arch.h"
#include "Os_Cfg.h"

extern TCB_t tcb[OS_MAX_TASKS];
extern volatile TCB_t *g_current;

/* =========================================================
 * SetEvent(t, mask)
 *  - OR bit vào events của task t.
 *  - Nếu t đang WAITING & trùng đợi → READY + clear waitMask.
 *  - Kích PendSV để có thể chuyển ngữ cảnh sớm.
 * =======================================================*/
/********************************************
 * @brief  Đặt Event cho Exteneded Task
 * @param[in]  t  Task ID (0..TASK_COUNT-1)
 * @param[in]  m  Event mask cần set
 * @return E_OK | E_OS_ID | E_OS_STATE
 * @note   Có thể gọi từ TASK hoặc ISR.Theo chuẩn, SetEvent 
 *          lên Basic Task là lỗi → E_OS_STATE.
 ********************************************/
StatusType SetEvent(TaskType id, EventMaskType mask){
    if(id >= OS_MAX_TASKS) return E_OS_ID;

    TCB_t *tc = &tcb[id];
    if(!tc->isExtended) 
        return E_OS_STATE; 
    __disable_irq();
    tc->SetEvent |= mask;
    __enable_irq();
    if(tc->state == OS_TASK_WAITING && (tc->SetEvent & tc->WaitEvent)){
        ActivateTask(tc ->id);
        tc->WaitEvent = 0;
        (void)Os_Arch_TriggerPendSV();
    }
    return E_OK;
}
/* =========================================================
 * WaitEvent(mask)
 *  - Extended Task chờ bất kỳ bit trong mask được set.
 *  - Nếu đã có sẵn bit → trả ngay E_OK (KHÔNG tự clear).
 *  - Nếu chưa có → set waitMask, chuyển trạng thái WAITING, nhường CPU.
 * =======================================================*/
/**********************************************************
 * @brief  Chờ Event (Extended Task))
 * @param  m  Mặt nạ event mong đợi
 * @return E_OK | E_OS_STATE
 * @note   Chỉ gọi trong ngữ cảnh TASK (không gọi từ ISR).
 *         Sau khi trả về E_OK do được đánh thức, ứng dụng 
 *         thường gọi ClearEvent(m) tương ứng để xóa các bit đã xử lý.
 **********************************************************/
StatusType WaitEvent(EventMaskType mask){
    TCB_t *tc = &tcb[g_current->id];

    if(!tc->isExtended) 
        return E_OS_STATE;    
    __disable_irq();

    if((tc->SetEvent & mask) != 0){
        __enable_irq();
        return E_OK;
    }
    else{
        tc -> WaitEvent = mask;
        tc -> state = OS_TASK_WAITING;
    }
    __enable_irq();
    return E_OK;
}
/********************************************
 * @brief Lấy Event hiện tại của Task
 * @param[in] id  Task ID
 * @param[out] event con trỏ nhận Event mask
 * @return Event mask của hiện tại
*********************************************/
StatusType GetEvent(TaskType id, EventMaskType *event){
    *event = tcb[id].SetEvent;
    return E_OK;
}
/******************************************
 * @brief Xoá Event của Task hiện tại
 * @param [in] mask Mặt nạ các bit cần xoá
 * @return void
 ******************************************/
StatusType ClearEvent(EventMaskType mask){
    TCB_t *t = &tcb[g_current->id];
    t->SetEvent &= ~ mask;
    return E_OK;
}