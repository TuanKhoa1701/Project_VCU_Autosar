/**********************************************************
 * @file    Os_Taks.c
 * @brief   Hạt nhân OS tối giản (Cortex-M3, STM32F103)
 * @details Scheduler round-robin trong cùng mức ưu tiên,
 *          tick 1ms, hỗ trợ Activate/Terminate, Delay,
 *          Event (ở module khác), Alarm, và chuyển ngữ
 *          cảnh bằng PendSV. Không có idle task riêng;
 *          khi không còn READY sẽ WFI chờ ngắt.
 *
 *          Lưu ý thiết kế:
 *            - PSP cho THREAD mode, MSP cho HANDLER mode.
 *            - Khung stack ban đầu của task tuân AAPCS:
 *              [xPSR, PC, LR, R12, R3, R2, R1, R0] + (R4-R11).
 *            - LR trong khung HW trỏ tới trampoline để
 *              Task return → TerminateTask().
 *            - OS_TASK_STACK_WORDS nên CHẴN để PSP 8-byte aligned.
 *
 * @version  1.0
 * @date     2025-09-10
 **********************************************************/

#include "Os.h"
#include "Os_Arch.h"
#include "Os_Cfg.h"
/* =========================================================
 * 1) Cấu hình stack cho từng task
 *    - Mỗi task có 1 stack riêng (full-descending).
 *    - OS_TASK_STACK_WORDS nên là số CHẴN để bảo đảm 8-byte align.
 * =======================================================*/

#ifndef OS_TASK_STACK_WORDS
#define OS_TASK_STACK_WORDS    256U
#endif

static uint32_t stack[OS_MAX_TASKS][OS_TASK_STACK_WORDS];
#define STACK_TOP(tid)   (&stack[tid][OS_TASK_STACK_WORDS])
/* =================TEST_Schedule_Table======================*/
void SetMode_Normal(void) {return 0;}
void SetMode_Warning(void) {return 0;}
void SetMode_Off(void) {return 0;}
/*===========================================================
 *2) Khai báo sử dụng các hàm counter/alarm/schedule table
=============================================================*/
    // static TaskEntry_t g_task_entry    [OS_MAX_TASKS];
    // static void        *g_task_arg     [OS_MAX_TASKS];
    // static uint32_t    *g_stack_top    [OS_MAX_TASKS];
    extern Std_ReturnType IncrementCounter(CounterTypeId cid);
    extern OsAlarmCtl alarm_tbl[OS_MAX_ALARMS];
    extern OsCounterCtl Counter_tbl[OS_MAX_COUNTERS];
    extern void os_alarm_tick(void);
    extern void Os_Alarm_Init(void);
    extern void ScheduleTable_tick(CounterTypeId cid);
    extern void Os_SchedTbl_Init(void);
/* =========================================================
 * 3) Khai báo thân Task do ứng dụng cung cấp (nếu định nghĩa trong file Os_Cfg rồi thì thôi)
 * ========================================================= */
// extern void Task_Init(void);
// extern void Task_A(void);
// extern void Task_B(void);
// extern void Task_C(void);
// extern void Task_Idle(void);

/* =========================================================
 * 4) READY Queue (ring buffer) – chừa 1 ô để phân biệt FULL/EMPTY
 * ========================================================= */
static uint8_t ready_q[OS_MAX_TASKS];
static uint8_t rq_head = 0u;
static uint8_t rq_tail = 0u;

static inline void rq_reset(void){
    rq_head = 0;
    rq_tail = 0;
}
static inline bool rq_empty(void){
    return (rq_head == rq_tail);
}
static inline bool rq_full(void){
    return (uint8_t)((rq_tail + 1) % OS_MAX_TASKS) == rq_head;
}
static inline bool rq_push(uint8_t tid){
    if(rq_full()){
        return false;
    }
    ready_q[rq_tail] = tid;
    rq_tail = (uint8_t)((rq_tail + 1) % OS_MAX_TASKS);
    return true;
}
static inline bool rq_pop_raw(uint8_t *out_tid){
    if(rq_empty()) 
        return false;
    *out_tid = ready_q[rq_head];
    rq_head = (uint8_t)((rq_head + 1u) % OS_MAX_TASKS);
    return true;
}
/* =========================================================
 * 4) Trạng thái runtime (export để module khác dùng)
 *    - g_current: TCB đang RUNNING (PendSV khôi phục từ đây)
 *    - g_next   : TCB mục tiêu sắp chuyển sang
 *    - s_curr_idx: chỉ số task hiện hành (hỗ trợ API khác)
 * =======================================================*/
    TCB_t tcb[OS_MAX_TASKS];
    volatile TCB_t *g_current = NULL;
    volatile TCB_t *g_next    = NULL;
   // uint8_t current_idx =-1;

/* =========================================================
 * 5) Fallback WFI (nếu vắng CMSIS)
 * =======================================================*/

 #ifndef __WFI
#define __WFI() __asm volatile ("wfi")
#endif

/* =========================================================
 * 6) Trampoline
 *    - Bảo đảm task không rơi vào EXC_RETURN ngẫu nhiên.
 * =======================================================*/

static void task_exit_trampoline(void)
{
      for (;;) {
        __WFI(); /* ngủ vĩnh viễn */
    }
}

/* =========================================================
 * 7) os_task_stack_init()
 *    - Dựng khung stack ban đầu đúng thứ tự do HW pop sau
 *      khi thoát exception 
 * =======================================================*/

 uint32_t *os_task_stack_init(TaskEntry_t entry, void *arg, uint32_t *top){

    /* 1) Căn chỉnh 8 byte: yêu cầu AAPCS + đảm bảo khi vào ISR/HW stack */
    uint32_t *sp = (uint32_t*)((uintptr_t)top & ~((uintptr_t)0x07));
    /* 2) ---- HW-stacked frame ----
     *  Thứ tự "unstack" của HW khi EXC_RETURN:
     *    R0, R1, R2, R3, R12, LR, PC, xPSR
     *
     *  Ta push theo thứ tự ngược lại để khi pop ra đúng:
     *    xPSR → PC → LR → R12 → R3 → R2 → R1 → R0
     */
    *(--sp) = 0x01000000u;                           /* xPSR: T-bit=1 (Thumb) */
    *(--sp) = ((uint32_t)entry) | 1u;                /* PC: địa chỉ hàm entry | 1 */
    *(--sp) = ((uint32_t)task_exit_trampoline)|1u;   /* LR: nếu entry return → thoát */
    *(--sp) = 0x12121212u;                           /* R12 */
    *(--sp) = 0x03030303u;                           /* R3  */
    *(--sp) = 0x02020202u;                           /* R2  */
    *(--sp) = 0x01010101u;                           /* R1  */
    *(--sp) = (uint32_t)arg;                         /* R0  (tham số truyền vào entry) */
    /* 3) ---- SW-saved frame (R4..R11) ----
     *  Đặt ngay bên dưới HW-frame. Khi LDMIA {r4-r11}, con trỏ PSP sẽ tiến
     *  đến &R0 (đầu HW-frame), đúng kỳ vọng của PendSV restore.
     *  Giá trị khởi tạo 0 là đủ (không bắt buộc).
     */
    *(--sp) = 0x11111111u; /* R11 */
    *(--sp) = 0x10101010u; /* R10 */
    *(--sp) = 0x09090909u; /* R9  */
    *(--sp) = 0x08080808u; /* R8  */
    *(--sp) = 0x07070707u; /* R7  */
    *(--sp) = 0x06060606u; /* R6  */
    *(--sp) = 0x05050505u; /* R5  */
    *(--sp) = 0x04040404u; /* R4  */

    /* 4) Trả về địa chỉ &R4 (đầu SW-frame) để nạp vào TCB->sp.
     *    Sau khi PendSV restore SW-frame, PSP sẽ = &R0 (đầu HW-frame).
     */
    return sp;
}
/* =========================================================
 * 8) schedule()
 * ---------------------------------------------------------
 * Mục tiêu:
 *   - Chọn "next" (TCB kế tiếp) để chạy.
 *   - Nếu READY queue rỗng → chọn IDLE.
 *   - Đặt yêu cầu đổi ngữ cảnh bằng PendSV (trì hoãn tới cuối ISR).
 *
 * Bối cảnh gọi:
 *   - Có thể được gọi trong ISR (ví dụ từ os_on_tick) hoặc từ Thread
 *     (ví dụ TerminateTask). Nếu gọi từ Thread, caller nên bọc vùng
 *     tới hạn (tắt IRQ ngắn hạn) trước/sau khi gọi để tránh race
 *     với ISR cũng đang thao tác READY queue.
 *
 * Quy ước:
 *   - g_next: con trỏ TCB của task sẽ được chuyển tới (vé chuyển cảnh).
 *     + Nếu g_next != NULL: đã có chuyển ngữ cảnh pending → không
 *       chọn thêm (tránh ghi đè vé cũ).
 *   - rq_pop_raw(): pop 1 task ID từ READY queue (không tự bọc IRQ).
 *   - TASK_IDLE: task rỗi, KHÔNG enqueue; chỉ được chọn khi queue rỗng.
 *   - os_trigger_pendsv(): đặt bit PENDSVSET để yêu cầu PendSV chạy.
 *
 * Yêu cầu với PendSV_Handler:
 *   - Sau khi chuyển xong: g_current = g_next; g_next = NULL;
 *     (xóa vé) để lần sau schedule() có thể đặt vé mới.
 *
 * Trả về:
 *   - true luôn (vì nếu không có READY thì vẫn chọn IDLE).
 * ========================================================= */
static bool schedule(void)
{
    if(g_next != NULL) return true;

    uint8_t tid;
    TCB_t *next = NULL;
    bool ok = rq_pop_raw(&tid);

    if(!ok){
        // Không có READY -> đừng Pendsv, chuyển sang idle
        next = &tcb[TASK_IDLE];
    } else {
        next = &tcb[tid];
        if(next->state != OS_TASK_READY){
            // lỗi logic: task không ở trạng thái READY
            next = &tcb[TASK_IDLE];
        } else {
            next -> state = OS_TASK_RUNNING;
        }
    }
    g_next = next;
    //PreTaskHook();
    __DSB(); __ISB();
    Os_Arch_TriggerPendSV();
    return true;
}

/* =========================================================
 *  9) ActivateTask(): DORMANT → READY (không kích chồng)
 * ========================================================= */

 StatusType ActivateTask(uint8_t tid){
    if(tid >= OS_MAX_TASKS || tid == TASK_IDLE) return E_OS_ID;

    __disable_irq();
    TCB_t *t = &tcb[tid];
    if(t->state == OS_TASK_SUSPENDED || t -> state == OS_TASK_WAITING){
        /*  Quan trọng dựng lại PSP để task lại từ đầu entry*/
        t->sp = os_task_stack_init(t->entry, 0, STACK_TOP(tid));
        t->state = OS_TASK_READY;
        (void)rq_push(tid);

        if(g_current == NULL){
            g_current = &tcb[tid];
            tcb[TASK_IDLE].sp = os_task_stack_init(tcb[TASK_IDLE].entry, 0, STACK_TOP(TASK_IDLE));
            //g_next = g_current;
        };
        
        if((g_current == &tcb[TASK_IDLE]) && (g_next == NULL)){
            __DSB(); __ISB();
            Os_Arch_TriggerPendSV();
        }
    }
    __enable_irq();
    return E_OK;
 }
/* =========================================================
 *  10) TerminateTask(): Task tự kết thúc → DORMANT và chuyển lịch
 * ========================================================= */
StatusType TerminateTask(void){

    __disable_irq();
    TCB_t *cur = (TCB_t *) g_current;
    
    if (cur){
        cur -> state = OS_TASK_SUSPENDED;
    }
    //PostTaskHook();
    __enable_irq();
    (void)schedule();


    for(;;){
        __NOP();
    }
}
/* ===========================================================
 * 11) ChainTask(): Kết thúc Task hiện tại và kích hoạt task tiếp theo
 * =============================================================*/
void ChainTask(TaskType tid){
    TerminateTask();
    ActivateTask(tid);
}

 
/* =========================================================
 *  os_on_tick(): gọi mỗi nhịp SysTick (ISR context)
 *   - Tăng tick, quét Alarm → ActivateTask() khi đến hạn
 *   - Run-to-completion: chỉ schedule ngay khi current là IDLE
 * ========================================================= */

void os_on_tick(void)
{
    (void)IncrementCounter(0); // Sử dụng hàm đã có để tăng counter
    /* Quét mọi alarm (ISR: atomic với thread) */
    os_alarm_tick();
    //ScheduleTable_tick(0);
    /* Giảm latency: nếu chưa có pending switch và đang ở IDLE → chọn ngay */
    if ((g_next == NULL) && (g_current == &tcb[TASK_IDLE])) {
        (void)schedule();
    }
}

/* =========================================================
 * 12) Cấu hình tĩnh các Task (ứng dụng cung cấp)
 * ========================================================= */
const TCB_t Os_TaskConfig [OS_MAX_TASKS]={
    [TASK_INIT] = {.entry = Task_Init, .name = "InitTask", .id = TASK_INIT, .prio = 1u, .isExtended =0u},
    [TASK_A]    = {.entry = Task_A,    .name = "Task_A",   .id = TASK_A,    .prio = 2u, .isExtended =0u},
    [TASK_B]    = {.entry = Task_B,    .name = "Task_B",   .id = TASK_B,    .prio = 1u, .isExtended =1u},
    [TASK_IDLE] = {.entry = Task_Idle, .name = "Task_Idle",.id = TASK_IDLE, .prio = 1u, .isExtended =1u},
    [TASK_C]    = {.entry = Task_C,    .name = "Task_C",   .id = TASK_C,    .prio = 1u, .isExtended =1u}
};

/* =========================================================
 * 13) StartOS(appMode) / ShutdownOS(e)
 *     - StartOS:
 *         + Copy cấu hình tĩnh → runtime (SUSPENDED)
 *         + Init Arch, cấu hình SysTick theo OS_TICK_HZ
 *         + Autostart: ActivateTask(InitTask_ID)
 *         + OS_Arch_StartFirstTask() (không quay lại)
 *     - ShutdownOS:
 *         + Gọi ShutdownHook(e), tắt IRQ và dừng.
 * =======================================================*/
StatusType StartOS(AppModeType appMode){

    (void) appMode;

    for(int i=0; i < OS_MAX_TASKS; i++){
        tcb[i] = Os_TaskConfig[i];
        tcb[i].state = OS_TASK_SUSPENDED;
        tcb[i].SetEvent = 0u;
        tcb[i].WaitEvent = 0u;
    }

    Os_Alarm_Init();
    //StartupHook();

    Os_Arch_Init();

    (void)ActivateTask(TASK_INIT);

    //Os_SchedTbl_Init();
    Os_Arch_StartFirstTask();

    for(;;){
        __NOP();
    }
}

void ShutdownOS(StatusType error){
        ShutdownHook(error);
      __asm volatile ("cpsid i\nb ."); /* tắt IRQ và dừng vô hạn */

}
