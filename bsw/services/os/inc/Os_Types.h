/**********************************************************
 * @file    Os_Types.h
 * @brief   Kiểu dữ liệu & cấu trúc lõi của OS (subset AUTOSAR/OSEK)
 * @details Khai báo các kiểu/hằng dùng chung cho OS chạy thực trên
 *          STM32F103 (ARM Cortex-M3), phù hợp mô hình:
 *            - SysTick 1ms (tăng TickType)
 *            - PendSV dùng cho chuyển ngữ cảnh
 *            - PSP cho THREAD mode, MSP cho HANDLER mode
 *
 *          Bao gồm:
 *            - StatusType, TickType, EventMaskType
 *            - TaskType/AlarmType (ID cấu hình)
 *            - Trạng thái Task (OsTaskState)
 *            - Prototype Task (TaskEntry_t)
 *            - Cấu trúc TCB_t tối giản
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#ifndef OS_TYPES_H
#define OS_TYPES_H
#include "Os_Cfg.h"
#ifdef __cplusplus
extern "C"
{
#endif
    /* =========================================================
     * 1) Kiểu dữ liệu cơ bản
     *    - StatusType     : mã trạng thái trả về của API OS.
     *    - TickType       : tick hệ thống (ms), tăng trong SysTick ISR.
     *    - EventMaskType  : mặt nạ sự kiện (mỗi bit là một Event).
     *    - TaskType       : ID của Task (ánh xạ từ Os_Cfg.h).
     *    - AlarmType      : ID của Alarm (ánh xạ từ Os_Cfg.h).
     *    - AlarmStateType : mã trạng thái của alarm
     *    - ResourceType   : mã trạng thái của Resource
     * =======================================================*/
    typedef uint8_t StatusType;
    typedef uint32_t TickType;
    typedef uint32_t EventMaskType;
    typedef uint8_t TaskType;
    typedef uint8_t AlarmType;
    typedef uint8_t CounterTypeId;
    typedef uint8_t AlarmStateType;
    typedef uint8_t ResourceType;
    /*
     * Gtái trị counter sẽ được trả về.
     */
    typedef TickType *TickRefType;

    /*
     * Chế độ mà khởi tạo OS
     */
    typedef uint8_t AppModeType;
#define OSDEFAULTAPPMODE ((AppModeType)0u) // chế độ bình thường

/* =========================================================
 * 2) Mã trạng thái cho API OS (subset phong cách AUTOSAR)
 *    - E_OK         : thành công.
 *    - E_OS_ID      : tham số ID không hợp lệ (Task/Alarm/Event/...).
 *    - E_OS_STATE   : không hợp lệ ở trạng thái hiện tại.
 *    - E_OS_LIMIT   : vượt giới hạn cấu hình/tài nguyên (vd: re-activate không cho phép).
 *    - E_OS_TIMEOUT : hết thời gian chờ (WaitEvent/Delay có timeout).
 *
 *  Lưu ý:
 *    - E_OK có thể đã được định nghĩa trong Std_Types.h (Std_ReturnType).
 *      Để tránh xung đột macro, chỉ định nghĩa nếu chưa có.
 * =======================================================*/
#ifndef E_OK
#define E_OK ((StatusType)0u)
#endif
#define E_OS_ID ((StatusType)1u)
#define E_OS_STATE ((StatusType)2u)
#define E_OS_LIMIT ((StatusType)3u)
#define E_OS_TIMEOUT ((StatusType)4u)
#define E_OS_NOFUNC ((StatusType)5u)
#define E_OS_VALUE ((StatusType)6u)

    /* =========================================================
     * 3) Trạng thái của Task
     *    - OS_TASK_SUSPENDED : chưa kích hoạt/đã kết thúc.
     *    - OS_TASK_READY     : sẵn sàng, chờ scheduler chọn.
     *    - OS_TASK_RUNNING   : đang thực thi.
     *    - OS_TASK_WAITING   : đang chờ (Event/Delay/Resource...).
     * =======================================================*/
    typedef enum
    {
        OS_TASK_SUSPENDED = 0,
        OS_TASK_READY,
        OS_TASK_RUNNING,
        OS_TASK_WAITING /* (Event/Delay/Resource) */
    } OsTaskState;
    /*=========================================================
     * 4) Hành động khi Alarm tới hạn (Alarm Action Type)
     *    - ALARMACTION_ACTIVATETASK : kích hoạt Task tương ứng.
     *    - ALARMACTION_SETEVENT     : đặt Event cho Extended Task.
     *    - ALARMACTION_CALLBACK     : gọi hàm callback (nội bộ).
     * =======================================================*/

    typedef enum
    {
        ALARMACTION_ACTIVATETASK,
        ALARMACTION_SETEVENT,
        ALARMACTION_CALLBACK
    } Alarm_ActionType;
    /*=========================================================
     * 5) Hành động khi Alarm tới hạn (Alarm Action Type)
     *    - ST_STOPPED           : Schedule Table đang dừng.
     *    - ST_WAITING_START     : Schedule Table đang ở trạng thái chờ được kích hoạt.
     *    - ST_RUNNING           : Schedule Table đang hoạt đông.
     * =======================================================*/
    typedef enum
    {
        ST_STOPPED = 0,
        ST_WAITING_START,
        ST_RUNNING
    } ScheduleTableState;

    /* =========================================================
     * 6) Prototype thân Task
     *    - Dạng hàm: void TaskX(void);
     *    - Ví dụ:   void Task_10ms(void) { /* ... */
    /* TerminateTask();
     * =======================================================*/
    typedef void (*TaskEntry_t)(void);

    /* =========================================================
     * 7) TCB — Task Control Block (tối giản)
     *    - sp           : con trỏ stack (PSP) lưu/khôi phục trong context switch.
     *    - stack_bottom : gốc stack (hỗ trợ debug/giám sát tràn).
     *    - events       : các bit Event hiện có của Task.
     *    - waitMask     : mặt nạ Event Task đang chờ trong WaitEvent().
     *    - state        : trạng thái hiện tại của Task.
     *    - isExtended   : 1 = Extended Task (hỗ trợ Event), 0 = Basic Task.
     *    - prio         : ưu tiên (0..31) — số lớn hơn = ưu tiên cao hơn.
     *    - delay        : ms còn chờ (OS_Delay hoặc timeout WaitEvent).
     *    - entry        : con trỏ hàm thân Task.
     *    - name         : tên phục vụ log/trace.
     *
     *  Gợi ý hiện thực trên Cortex-M3:
     *    - sp/stack_bottom nên căn 8-byte (AAPCS) để tránh HardFault.
     *    - `prio` có thể ánh xạ trực tiếp vào bitmap scheduler (0..31).
     *    - `delay` giảm theo tick SysTick 1ms.
     * =======================================================*/
    typedef struct
    {
        uint32_t *sp;             /* PSP lưu bởi context switch (THREAD mode).      */
        uint32_t *stack_bottom;   /* Điểm đáy stack để debug/kiểm tra tràn.         */
        TaskType id;              /* Task ID (ánh xạ từ Os_app_Cfg.h).              */
        EventMaskType SetEvent;     /* Event mask (Extended Task).                  */
        EventMaskType WaitEvent;   /* Mặt nạ Event đang chờ trong WaitEvent().      */
        OsTaskState state;        /* Trạng thái hiện tại của Task.                  */
        uint8_t ActivationCount;  /* Đếm số lần đã active                           */
        uint8_t OsTaskActivation; /* Giới hạn số lần activate                       */
        uint8_t prio;             /* Ưu tiên (0..31), 0 = thấp nhất.                */
        uint8_t base_prio;        /* Priority gốc                                   */
        uint8_t isExtended;       /* Khai báo Task là basic hay Extended Task       */
        TaskEntry_t entry;        /* Hàm thân Task (void TaskX(void)).              */
        const char *name;         /* Tên Task cho mục đích log/trace.               */
    } TCB_t;

    /* =========================================================
     * CẤU TRÚC ĐIỀU KHIỂN COUNTER
     *  - current_value     : giá trị hiện tại (tăng theo tick).
     *  - max_allowed_Value : giá trị tối đa (giới hạn tràn).
     *  - ticks_per_base    : số tick tương ứng 1 đơn vị base (thường là 1).
     *  - min_cycles        : chu kỳ tối thiểu (đơn vị base) cho alarm.
     *
     * Gợi ý:
     *  - Với SysTick 1ms, thường có: ticks_per_base = 1, min_cycles = 1.
     * =======================================================*/
    typedef struct
    {
        TickType current_value;
        TickType max_allowed_Value;
        TickType ticks_per_base;
        uint8_t min_cycles;
    } OsCounterCtl;

        /* =========================================================
     * CẤU TRÚC ĐIỀU KHIỂN ALARM
     *  - active : 1=đang hoạt động; 0=đã hủy/one-shot đã bắn xong
     *  - Expiry-tick : thời điểm tới hạn 
     *  - cycle  : chu kỳ (ms); 0 = one-shot
     *  - id     : Alarm ID (để switch/map sang Task tương ứng)
     *  - counter: con trỏ đến counter điều khiển alarm
     *  - action_type : kiểu hành động khi tới hạn
     *  - action  : thông tin hành động (activatetask/setevent/callback)
     * =======================================================*/
    typedef struct
    {
        AlarmStateType active;
        TickType Expiry_tick;
        TickType cycle;
        AlarmType id;
        OsCounterCtl *counter;
        Alarm_ActionType action_type;
        union
        {
            TaskType task_id; /* ALARMACTION_ACTIVATETASK */
            struct
            {
                TaskType task_id; /* ALARMACTION_SETEVENT */
                EventMaskType mask;
            } Set_event;
            TaskEntry_t callback; /* ALARMACTION_CALLBACK */
        } action;
    } OsAlarmCtl;
    /* ========================================================
     * ĐIỂM TỚI HẠN CỦA SCHEDULETABLE

     *=========================================================*/
    typedef struct
    {
        TickType offset;
        enum
        {
            SCH_ACTIVATETASK,
            SCH_SETEVENT,
            SCH_CALLBACK
        } action_type;
        union
        {
            TaskType task_id;
            struct
            {
                TaskType task_id;
                EventMaskType event;
            } set_event;
            TaskEntry_t func_callback;
        } action;
    } Expiry_Point;
    /* ========================================================
     * Cấu hình SCHEDULETABLE

     *=========================================================*/
    typedef struct
    {
        TickType start;
        TickType duration;
        uint8_t cyclic;
        uint8_t current_ep;
        uint8_t num_eps;
        Expiry_Point eps[MAX_EXPIRY_POINTS];
        OsCounterCtl *counter;
        ScheduleTableState state;
    } OsSchedCtl;
    /* ========================================================
     * Cấu trúc cho Resource

     *=========================================================*/
    typedef struct
    {
        volatile uint8_t locked;            // Resource locked?
        uint8_t ceilingPrio; // Ceiling Priority (PCP)
        TaskType owner;      // Task đang giữ resource (INVALID_TASK nếu rảnh)
    } OsResource;
    /* ========================================================
     * Cấu hình cho kênh IOC

     *=========================================================*/
    typedef struct
    {
        uint8_t used;                             
        uint8_t num_receivers;              
        TaskType receivers[4];              
        uint8_t head, count;
        uint16_t buffer[MAX_IOC_CHANNELS][IOC_BUFFER_SIZE]; 
        uint8_t tail[4];                   
    } OsIocCtrl;
#ifdef __cplusplus
}
#endif

#endif 
/* OS_TYPES_H */
