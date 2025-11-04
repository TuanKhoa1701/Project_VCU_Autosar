/**********************************************************
 * @file    Os_SchedTbl.c
 * @brief   Triển khai Schedule Table theo chuẩn AUTOSAR (subset).
 * @details Cung cấp cơ chế lập lịch các hành động (kích hoạt Task, đặt Event,
 *          gọi callback) tại các thời điểm định trước (expiry points).
 *          Hoạt động dựa trên một Counter của OS.
 *
 *          API chính:
 *          - StartScheduleTableRel(): Bắt đầu một schedule table sau một khoảng thời gian tương đối.
 *          - StartScheduleTableAbs(): Bắt đầu một schedule table tại một thời điểm tuyệt đối.
 *          - StopScheduleTable(): Dừng một schedule table đang chạy.
 *          - SyncScheduleTable(): Đồng bộ hóa một schedule table đang chạy.
 *          - ScheduleTable_tick(): Được gọi bởi OS tick để xử lý các schedule table.
 *
 *          Mô hình thời gian:
 *            - Mỗi schedule table có một `duration` (chu kỳ) và có thể lặp lại (`cyclic`).
 *            - Các `Expiry_Point` được định nghĩa bằng `offset` so với thời điểm bắt đầu của chu kỳ.
 *            - `ScheduleTable_tick()` được gọi trong ngữ cảnh ngắt (tick của OS) để kiểm tra
 *              và thực thi các hành động khi đến `offset`.
 *
 *          An toàn đồng thời:
 *            - Các API (Start/Stop/Sync) thường được gọi từ Task.
 *            - `ScheduleTable_tick` được gọi từ ISR.
 *            - Sử dụng `__disable_irq()` và `__enable_irq()` để bảo vệ các vùng dữ liệu
 *              quan trọng (ví dụ: `state`, `start`) khỏi race condition.
 *
 *          Lưu ý thiết kế:
 *            - Các Expiry Point nên được sắp xếp theo thứ tự `offset` tăng dần để tối ưu xử lý.
 *            - Việc thực thi hành động (ActivateTask, SetEvent) không nằm trong vùng găng
 *              để giảm thiểu thời gian vô hiệu hóa ngắt.
 *
 * @version  1.1
 * @date     2025-09-10
 * @author   Nguyễn Tuấn Khoa
 **********************************************************/
#include "Os.h"
#include "Os_Arch.h"

OsSchedCtl Schedule_Table_List[OS_MAX_SchedTbl];
extern OsCounterCtl Counter_tbl[OS_MAX_COUNTERS];

static inline TickType diff_wrap(TickType cur, TickType start, TickType max) {
    return (cur >= start) ? (cur - start) : (max - start + cur);
}
StatusType StartScheduleTableRel(uint8_t table_id, TickType offset){
    if(table_id >= OS_MAX_SchedTbl) return E_OS_ID;

    OsSchedCtl *s = &Schedule_Table_List[table_id];
    if(s->state != ST_STOPPED)  return E_OS_STATE;
    if(offset > s->counter->max_allowed_Value) return E_OS_VALUE;

    __disable_irq();
    s->start = (s->counter->current_value + offset) % s->counter->max_allowed_Value;
    s->state = ST_WAITING_START;
    __enable_irq();

    s->current_ep =0;
    // hỗ trợ start hay offset = 0
    for(uint8_t i=0; i < s->num_eps; i++){
        if(s->eps[i].offset == 0){
            switch (s->eps[i].action_type){
            case  SCH_ACTIVATETASK:
                ActivateTask(s->eps[i].action.task_id);
                break;
            case SCH_SETEVENT:
                SetEvent(s->eps[i].action.set_event.task_id, s->eps[i].action.set_event.event);
                break;
            case SCH_CALLBACK:
                s->eps[i].action.func_callback();
                break;
            default:
                break;
            }
        }
    }
    return E_OK;
}
StatusType StartScheduleTableAbs(uint8_t table_id, TickType start){
    if(table_id >= OS_MAX_SchedTbl) return E_OS_ID;

    OsSchedCtl *s = &Schedule_Table_List[table_id];
    if(s->state != ST_STOPPED)  return E_OS_STATE;
    if(start > s->counter->max_allowed_Value) return E_OS_VALUE;

    __disable_irq();
    s->state = ST_WAITING_START;
    s->start = start % s->counter->max_allowed_Value;
    __enable_irq();

    s->current_ep =0;

    for(uint8_t i=0; i < s->num_eps; i++){
        if(s->eps[i].offset == 0){
            switch (s->eps[i].action_type){
            case  SCH_ACTIVATETASK:
                ActivateTask(s->eps[i].action.task_id);
                break;
            case SCH_SETEVENT:
                SetEvent(s->eps[i].action.set_event.task_id, s->eps[i].action.set_event.event);
                break;
            case SCH_CALLBACK:
                s->eps[i].action.func_callback();
                break;
            default:
                break;
            }
        }
    }
    return E_OK;
}

StatusType StopScheduleTable(uint8_t table_id){
    if(table_id >= OS_MAX_SchedTbl) return E_OS_ID;
    OsSchedCtl *s = &Schedule_Table_List[table_id];
    if(s->state == ST_STOPPED)  return E_OS_STATE;

    __disable_irq();
    s->state = ST_STOPPED;
    s->current_ep =0;
    __enable_irq();

    return E_OK;
}

StatusType SyncScheduleTable(uint8_t table_id, TickType new_start_offset){

    if(table_id >= OS_MAX_SchedTbl) return E_OS_ID;
    OsSchedCtl *s = &Schedule_Table_List[table_id];
    if(s->state == ST_STOPPED) return E_OS_STATE;
    if(new_start_offset > s->counter->max_allowed_Value) return E_OS_VALUE;

    __disable_irq();
    s->start = (s->counter->current_value + new_start_offset) % s->counter->max_allowed_Value;
    s->current_ep = 0;
    s->state = ST_WAITING_START;
    __enable_irq();
    return E_OK;

}

void ScheduleTable_tick(CounterTypeId cid){
    OsCounterCtl *c = &Counter_tbl[cid];

    for(int i=0;i < OS_MAX_SchedTbl; i++){
        OsSchedCtl *s = &Schedule_Table_List[cid];
        
        if( s-> counter != c || s-> state == ST_STOPPED ) continue;
        TickType cur = c->current_value;
        TickType max = c->max_allowed_Value;
        TickType elapsed_from_start = diff_wrap(cur, s->start, max);

        if(s -> state == ST_WAITING_START){
            if(elapsed_from_start < s->duration){
                s->state = ST_RUNNING;
                s->current_ep = 0;

                while(s->current_ep < s->num_eps && s->eps[s->current_ep].offset <= elapsed_from_start){
                    Expiry_Point *ep = &s->eps[s->current_ep];
                    switch(ep->action_type){
                        case SCH_ACTIVATETASK:
                            ActivateTask(ep->action.task_id);
                            break;
                        case SCH_SETEVENT:
                            SetEvent(ep->action.set_event.task_id,ep->action.set_event.event);
                            break;
                        case SCH_CALLBACK:
                            ep->action.func_callback();
                            break;
                    }
                    s->current_ep++;
                }

            } else {
                if(s->cyclic){
                    TickType periods_skipped = elapsed_from_start / s->duration;
                    s->start = (s->start + periods_skipped *s->duration) % max;
                    s->current_ep = 0;
                    s->state = ST_WAITING_START;
                } else{
                    s->state = ST_STOPPED;
                    s->current_ep=0;
                }
            }
            continue;
        }
        if(s->state == ST_RUNNING){
            while (s->current_ep < s->num_eps && s->eps[s->current_ep].offset <= elapsed_from_start){
                Expiry_Point *ep = &s->eps[s->current_ep];
                switch(ep->action_type){
                    case SCH_ACTIVATETASK:
                        ActivateTask(ep->action.task_id);
                        break;
                    case SCH_SETEVENT:
                        SetEvent(ep->action.set_event.task_id,ep->action.set_event.event);
                        break;
                    case SCH_CALLBACK:
                        ep->action.func_callback();
                        break;
                }   
                s->current_ep++;
            }
            if(elapsed_from_start >= s->duration){
                if(s->cyclic){
                    TickType Period_skipped = elapsed_from_start / s->duration;
                    s->start = (s->start + Period_skipped * s->duration) %max;
                    s->current_ep = 0;
                    s->state = ST_WAITING_START;

                    TickType e2 = diff_wrap(cur, s->start, max);
                    if (e2 < s->duration) {
                        s->state = ST_RUNNING;
                        while (s->current_ep < s->num_eps && s->eps[s->current_ep].offset <= e2) {
                            Expiry_Point *ep = &s->eps[s->current_ep];
                        switch(ep->action_type){
                            case SCH_ACTIVATETASK:
                                ActivateTask(ep->action.task_id);
                                break;
                            case SCH_SETEVENT:
                                SetEvent(ep->action.set_event.task_id,ep->action.set_event.event);
                                break;
                            case SCH_CALLBACK:
                                ep->action.func_callback();
                                break;
                            }
                            s->current_ep++;
                         }
                    }
                 } else{
                    s->state = ST_STOPPED;
                    s->current_ep = 0;
                 }
            }
        }
    }   
}

void Os_SchedTbl_Init(void){
    OsSchedCtl *t = &Schedule_Table_List[0];

    t->counter = &Counter_tbl[0];
    t->state = ST_STOPPED;
    t->duration = 5000u;
    t->cyclic = 1u;
    t->num_eps = 3u;

    t->eps[0]= (Expiry_Point) {.offset = 0u,    .action_type = SCH_CALLBACK, .action.func_callback = SetMode_Normal};
    t->eps[1]= (Expiry_Point) {.offset = 2000u, .action_type = SCH_CALLBACK, .action.func_callback = SetMode_Warning};
    t->eps[2]= (Expiry_Point) {.offset = 4000u, .action_type = SCH_CALLBACK, .action.func_callback = SetMode_Off};

}
