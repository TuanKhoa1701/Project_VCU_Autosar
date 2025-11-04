/**********************************************************
 * @file    Os.h
 * @brief   API công khai của lớp OS (subset AUTOSAR/OSEK) cho STM32F103
 * @details Hỗ trợ vận hành thời gian thực nhẹ cho Cortex-M3:
 *          - SysTick 1ms (tăng TickType, alarm/schedule)
 *          - PendSV cho chuyển ngữ cảnh
 *          - PSP cho THREAD mode, MSP cho HANDLER mode
 *
 *          Nhóm API:
 *            1) Lifecycle: StartOS, ShutdownOS
 *            2) Task     : ActivateTask, TerminateTask, OS_Yield, OS_Delay
 *            3) Event    : WaitEvent, SetEvent, GetEvent, ClearEvent (Extended Task)
 *            4) Alarm    : SetRelAlarm/CancelAlarm (ms)
 *            5) Counter  : OS_TickCount()
 *            6) IOC demo : hàng đợi byte vòng (SR queued tối giản)
 *            7) Resource : mutex đơn giản (không có ceiling protocol)
 *            8) Schedule : ScheduleTable “lite”
 *            9) Arch     : glue phụ thuộc kiến trúc (SysTick/PendSV/bootstrap)
 *
 * @version  1.0
 * @date     2025-09-10
 * @author   Nguyễn Tuấn Khoa
 **********************************************************/
#ifndef OS_H
#define OS_H

#include "Os_Types.h"
#include "Os_Cfg.h"
#include "stm32f10x.h"
#include "cmsis_gcc.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C"
{
#endif
/* =========================================================
 * Macro định nghĩa thân TASK theo phong cách AUTOSAR
 *  - AUTOSAR chuẩn:  #define TASK(Name)  FUNC(void, OS_CODE) Name(void)
 *  - Bản này:        TASK(Name) mở rộng thành “void Name(void)”.
 * ======================================================= */
#ifndef TASK
#define TASK(Name) void Name(void)
#endif
    /* =========================================================
     * 1) LIFECYCLE
     * =======================================================*/
    /**
     * @brief  Khởi động OS.
     * @param  appMode  AppMode (OSDEFAULTAPPMODE nếu không dùng đa mode)
     * @return E_OK nếu khởi động thành công (thực tế thường không trả về)
     *
     * @details
     *  - Khởi tạo phần phụ thuộc kiến trúc: OS_Arch_Init(), OS_Arch_SystickConfig(...).
     *  - Gọi StartupHook() (nếu có).
     *  - Kích hoạt autostart task (ví dụ InitTask).
     *  - Chuyển quyền điều khiển sang Scheduler (OS_Arch_StartFirstTask()).
     *  - Thông thường không quay lại; kết thúc bằng ShutdownOS().
     *
     * @note   Gọi từ trạng thái reset/main trước khi có task nào chạy.
     */
    StatusType StartOS(AppModeType appMode);
    /**
     * @brief  Tắt OS, kết thúc hệ thống.
     * @param  error  Mã kết thúc (nguyên nhân)
     * @note   Gọi ShutdownHook(e) (nếu có), dừng SysTick, vô hiệu lịch, và quay về main().
     */
    void ShutdownOS(StatusType error);
    /* =========================================================
     * 2) TASK API
     * =======================================================*/
    /**
     * @brief  Kích hoạt một Task theo ID.
     * @param  tid  ID Task (xem Os_Cfg.h)
     * @return E_OK | E_OS_ID | E_OS_LIMIT | E_OS_STATE
     *
     * @details
     *  - Nếu Task đang SUSPENDED → chuyển về READY, gán entry/stack/priority.
     *  - Nếu đã RUNNING/READY và không cấu hình multi-activation → E_OS_LIMIT.
     *  - Có thể được gọi từ TASK hoặc ISR (Cat2) tùy hiện thực.
     */
    StatusType ActivateTask(TaskType tid);

    /* Prototype task */
extern void Task_Init(void);
extern void Task_A(void);
extern void Task_B(void);
extern void Task_Idle(void);
    /**
     * @brief  Kết thúc Task hiện tại.
     * @return E_OK | E_OS_STATE (nếu gọi khi không ở RUNNING)
     *
     * @details
     *  - Chuyển Task về SUSPENDED (Basic) hoặc READY (nếu cấu hình khác).
     *  - Gọi PostTaskHook() trước khi nhường CPU.
     */
    StatusType TerminateTask(void);
    /**
     * @brief  Nhường CPU tự nguyện (cooperative yield).
     * @note   Không thay đổi trạng thái (vẫn READY); chỉ kích PendSV để chuyển ngữ cảnh.
     */
    void OS_Yield(void);
    /**
     * @brief  Treo Task hiện tại một khoảng thời gian (ms).
     * @param  ms  Thời gian trì hoãn (mili-giây)
     * @note   Chỉ gọi ở ngữ cảnh TASK; không gọi từ ISR. Task chuyển sang WAITING.
     */
    void OS_Delay(uint32_t ms);
    /* =========================================================
     * 3) EVENT API (Extended Task)
     * =======================================================*/
    /**
     * @brief  Chờ tới khi bất kỳ bit trong mask được set.
     * @param  mask  Mặt nạ event mong đợi
     * @return E_OK | E_OS_STATE | E_OS_TIMEOUT (nếu có cơ chế timeout đi kèm)
     *
     * @details
     *  - Chỉ dùng cho Extended Task.
     *  - Không gọi từ ISR. Task chuyển sang WAITING cho tới khi (events & m) != 0.
     */
    StatusType WaitEvent(EventMaskType mask);
    /**
     * @brief  Set event cho Task t và đánh thức nếu Task đang WaitEvent().
     * @param  tid  Task ID
     * @param  mask  Event mask cần set
     * @return E_OK | E_OS_ID
     *
     * @details
     *  - Có thể gọi từ TASK hoặc ISR (Cat2).
     *  - OR bit vào events của Task t; nếu đang WAITING và trùng mask → chuyển READY.
     */
    StatusType SetEvent(TaskType tid, EventMaskType mask);
    /**
     * @brief  Đọc mặt nạ event hiện tại của Task t.
     * @param  tid   Task ID
     * @param  mask   [out] con trỏ nhận giá trị mask
     * @return E_OK | E_OS_ID
     */
    StatusType GetEvent(TaskType tid, EventMaskType *mask);
    /**
     * @brief  Xoá (clear) các bit event đã xử lý của Task hiện tại.
     * @param  mask  Mặt nạ cần xoá
     * @return E_OK | E_OS_STATE
     *
     * @note   Chỉ gọi bởi chính Task đang RUNNING (Extended Task).
     */
    StatusType ClearEvent(EventMaskType mask);
    /* =========================================================
     * 4) ALARM API (ms)
     * =======================================================*/
    /**
     * @brief  Đặt alarm tương đối (ms).
     * @param  alarm     Alarm ID
     * @param  offset    Trễ trước lần kích đầu tiên
     * @param  cycle     Chu kỳ kích tiếp theo (0 = one-shot)
     * @return E_OK | E_OS_ID | E_OS_STATE | E_OS_LIMIT
     *
     * @details
     *  - Mỗi tick (SysTick) OS cập nhật bộ đếm; tới hạn → kích hoạt Task map với Alarm.
     *  - Mapping Alarm→Task do phần triển khai OS/ cấu hình xác định.
     */
    StatusType SetRelAlarm(AlarmType alarm, TickType offset, TickType cycle);
    /***************************************************
     * @brief Đặt alarm tuyệt đối (ms).
     * @param  alarm     Alarm ID [0..ALARM_COUNT-1]
     * @param  start     Thời điểm kích lần đầu tiên (tính từ 0)
     * @param  cycle     Chu kỳ kích sau đó (0 = one-shot)
     * @return E_OK | E_OS_ID | E_OS_STATE | E_OS_LIMIT
     *****************************************************/
    StatusType SetAbsAlarm(AlarmType alarm, TickType start, TickType cycle);
    /**
     * @brief  Huỷ một alarm nếu đang hoạt động.
     * @param  alarm  Alarm ID
     * @return E_OK | E_OS_ID | E_OS_STATE
     */
    StatusType CancelAlarm(AlarmType alarm); 
    /* =========================================================
     * 5) COUNTER API
     * =======================================================*/

     /*********************************************************
     * @brief    Tăng giá trị của một counter.
     * @param[in] cid  ID của counter cần tăng.
     * @return:  E_OK nếu thành công, E_OS_ID nếu ID không hợp lệ.
     *
     * @details: API này thường được gọi từ một nguồn tick (ví dụ: ISR của SysTick) để
     *           tăng giá trị của counter được chỉ định. Logic tăng sẽ phụ thuộc vào
     *           cấu hình `ticks_per_base` của counter đó.
     **********************************************************/
    StatusType IncrementCounter(CounterTypeId cid);

    /**********************************************************
     * @brief  Lấy giá trị hiện tại của một counter.
     * @param  cid    ID của counter cần đọc.
     * @param  value  [out] Con trỏ để nhận giá trị của counter.
     * @return E_OK nếu thành công, E_OS_ID nếu ID hoặc con trỏ `value` không hợp lệ.
     *
     * @details Đọc và trả về giá trị hiện tại của counter được chỉ định.
     */
    StatusType GetCounterValue(CounterTypeId cid, TickRefType value);
    /* =========================================================
     * 6) SCHEDULE TABLE API
     * =======================================================*/
    /**
     * @brief Kích hoạt một Schedule Table sau một khoảng thời gian tương đối.
     * @param table_id ID của Schedule Table.
     * @param offset Thời gian trễ (tính bằng tick) trước khi bắt đầu.
     * @return E_OK nếu thành công.
     */
    StatusType StartScheduleTableRel(uint8_t table_id, TickType offset);

    /**
     * @brief Kích hoạt một Schedule Table tại một thời điểm tuyệt đối.
     * @param table_id ID của Schedule Table.
     * @param start Thời điểm bắt đầu tuyệt đối (tính bằng tick).
     * @return E_OK nếu thành công.
     */
    StatusType StartScheduleTableAbs(uint8_t table_id, TickType start);

    /**
     * @brief Dừng một Schedule Table đang hoạt động.
     * @param table_id ID của Schedule Table.
     * @return E_OK nếu thành công.
     */
    StatusType StopScheduleTable(uint8_t table_id);

    /**
     * @brief Đồng bộ hóa một Schedule Table đang chạy.
     * @param table_id ID của Schedule Table.
     * @param new_start_offset Offset mới để đồng bộ.
     * @return E_OK nếu thành công.
     */
    StatusType SyncScheduleTable(uint8_t table_id, TickType new_start_offset);

    /* =========================================================
     * 7) IOC API
     * =======================================================*/
    /**
     * @brief Gửi dữ liệu qua một kênh giao tiếp liên tác vụ (IOC).
     * @param ch Kênh IOC.
     * @param data Con trỏ tới dữ liệu cần gửi.
     * @return E_OK nếu thành công.
     */
    uint8_t Ioc_Send(uint8_t ch, uint16_t *data);

    /**
     * @brief Nhận dữ liệu từ một kênh IOC cho một task cụ thể.
     * @param ch Kênh IOC.
     * @param data Con trỏ để nhận dữ liệu.
     * @param receiver ID của Task nhận.
     * @return E_OK nếu có dữ liệu mới và nhận thành công.
     */
    uint8_t Ioc_Receive(uint8_t ch, uint16_t *data, TaskType receiver);
    
    /*******************************************************************
     * @brief  Khởi tạo một kênh giao tiếp liên tác vụ (IOC).
     * @param  channel   ID của kênh IOC cần khởi tạo (xem Ioc_Channels trong Os_Cfg.h).
     * @param  num       Số lượng Task sẽ nhận dữ liệu trên kênh này.
     * @param  receivers Con trỏ tới một mảng chứa ID của các Task nhận.
     * @details
     *  Hàm này thiết lập và chuẩn bị một kênh IOC để sẵn sàng cho việc gửi và nhận dữ liệu.
     *  Nó thực hiện các công việc sau:
     
     * @note   Hàm này không có cơ chế bảo vệ (vùng găng) và nên được gọi một lần duy nhất
     *         trong quá trình khởi tạo hệ thống (ví dụ: trong Task_Init) trước khi các Task khác bắt đầu gửi/nhận.
     *****************************************************************/
    void Ioc_Init(uint8_t channel, uint8_t num, TaskType* receivers);

    /* =========================================================
     * 8) HOOK API
     * =======================================================*/
    /**
     * @brief Hook khởi động hệ thống.
     * @details Hàm này được OS gọi một lần duy nhất trong quá trình thực thi `StartOS`,
     *          sau khi khởi tạo các cấu trúc dữ liệu nội bộ của OS nhưng trước khi
     *          scheduler bắt đầu chạy task đầu tiên.
     * @note Đây là nơi lý tưởng để khởi tạo các driver phần cứng (MCAL) và các module
     *       phần mềm cơ sở (BSW) mà ứng dụng cần.
     */
    void StartupHook(void);

    /**
     * @brief Hook tắt hệ thống.
     * @param error Mã lỗi cho biết lý do tắt máy.
     * @details Hàm này được OS gọi khi `ShutdownOS` được thực thi, ngay trước khi
     *          hệ thống dừng hoàn toàn.
     * @note Dùng để thực hiện các hành động dọn dẹp an toàn, ví dụ như đưa phần cứng
     *       về trạng thái an toàn, lưu dữ liệu quan trọng vào bộ nhớ non-volatile.
     */
    void ShutdownHook(StatusType error);

    /**
     * @brief Hook xử lý lỗi của OS.
     * @param error Mã lỗi được phát hiện bởi OS.
     * @details Hàm này được OS gọi khi một lỗi nghiêm trọng xảy ra (ví dụ: gọi API
     *          với tham số không hợp lệ, tràn tài nguyên).
     * @note Trong môi trường phát triển, hook này có thể được dùng để in log, dừng
     *       hệ thống để debug. Trong môi trường sản phẩm, nó có thể kích hoạt một
     *       cơ chế phục hồi hoặc chuyển sang trạng thái an toàn.
     */
    void ErrorHook(StatusType error);

    /**
     * @brief Hook được gọi trước khi một task bắt đầu thực thi.
     * @details Hàm này được scheduler gọi ngay trước khi chuyển ngữ cảnh để một task
     *          chuyển từ trạng thái READY sang RUNNING.
     * @note Hữu ích cho việc đo lường hiệu năng (bắt đầu timer), tracing, hoặc kiểm tra stack.
     */
    void PreTaskHook(void);

    /**
     * @brief Hook được gọi sau khi một task kết thúc thực thi.
     * @details Hàm này được scheduler gọi ngay sau khi một task rời khỏi trạng thái RUNNING
     *          (do bị preempt, tự kết thúc, hoặc chờ tài nguyên).
     * @note Hữu ích cho việc đo lường hiệu năng (dừng timer và ghi nhận thời gian thực thi).
     */
    void PostTaskHook(void);

    /* =========================================================
     * 9) Resource API
     * =======================================================*/
    /****************************************************************************************
     * @brief Yêu cầu và khóa một resource để bảo vệ vùng tranh chấp.
     * @param r Con trỏ tới khối điều khiển của resource.
     * @details Cơ chế này đảm bảo quyền truy cập độc quyền vào dữ liệu chia sẻ giữa các task.
     *          Tùy thuộc vào cấu hình, nó có thể được hiện thực bằng cách vô hiệu hóa ngắt
     *          hoặc sử dụng thuật toán ưu tiên như Priority Ceiling Protocol (PCP).
     *****************************************************************************************/
    void GetResource(OsResource *r);

    /**
     * @brief Mở khóa và giải phóng một resource đã được lấy trước đó.
     * @param r Con trỏ tới khối điều khiển của resource.
     * @details Phải được gọi bởi chính task đã lấy resource để các task khác có thể tiếp tục
     *          sử dụng.
     */
    void ReleaseResource(OsResource *r);

    StatusType Os_ConnectAlarm(AlarmType alarm, void (*cb)(void));
    StatusType Os_DisconnectAlarm(AlarmType alarm);

#ifdef __cplusplus
}
#endif
#endif /* OS_H */
