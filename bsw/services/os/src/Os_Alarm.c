/**********************************************************
 * @file    Os_Alarm.c
 * @brief   Bộ điều phối Alarm (ms) cho OS (subset AUTOSAR/OSEK) trên STM32F103
 * @details Cung cấp:
 *          - Bộ đếm Alarm theo tick (ms) gọi từ SysTick/ISR: os_alarm_tick()
 *          - API SetRelAlarm()/CancelAlarm() theo phong cách AUTOSAR
 *          - Ánh xạ Alarm → Task chu kỳ (ActivateTask)
 *
 *          Mô hình thời gian:
 *            - SysTick cấu hình 1 kHz (1 ms/tick)
 *            - Mỗi tick gọi os_alarm_tick() để giảm bộ đếm remain
 *            - Khi remain về 0 → kích hoạt Task được map với Alarm
 *            - cycle = 0 → one-shot; cycle > 0 → periodic (reload remain=cycle)
 *
 *          Tính đồng bộ:
 *            - os_alarm_tick() thường chạy trong ngữ cảnh ISR (SysTick)
 *            - SetRelAlarm()/CancelAlarm() có thể gọi từ TASK hoặc ISR
 *            - Cần đảm bảo vùng găng (critical section) khi cập nhật s_alarms
 *              nếu OS hỗ trợ tiền xử lý song song giữa ISR và TASK.
 *
 * @version  1.0
 * @date     2025-09-10 11:25
 * @author   Nguyễn Tuấn Khoa
 **********************************************************/
#include "Os.h"
#include "Os_Arch.h"
#include "Os_Cfg.h"

//extern alarm_to_counter[OS_MAX_ALARMS];
OsAlarmCtl alarm_tbl[OS_MAX_ALARMS];
extern TCB_t tcb[OS_MAX_TASKS];

extern OsCounterCtl Counter_tbl[OS_MAX_COUNTERS];

static inline uint32_t ms_to_tick(uint32_t ms){
    if(ms == 0u)
        return 0u;
    uint64_t t = ((uint64_t)ms * (uint64_t)OS_TICK_HZ +999ull) / 1000ull;

    if(t ==0) t = 1ull;
    if(t > 0xFFFFFFFFull) t = 0xFFFFFFFFull;
    return (uint32_t)t;
}
/*
 * 1) SetRelAlarm()
 */

 StatusType SetRelAlarm(AlarmType aid, TickType offset, TickType cycle){

    if(aid >= OS_MAX_ALARMS) return E_OS_LIMIT;

    //OsCounterCtl *c = alarm_to_counter[aid];

    uint32_t inc_ticks = ms_to_tick(offset);
    uint32_t cyc_ticks = ms_to_tick(cycle);

    __disable_irq();

    OsAlarmCtl *a = &alarm_tbl[aid];
    a->active = 1u;
    a->Expiry_tick = (a->counter->current_value + inc_ticks) % a->counter->max_allowed_Value;

    if(cyc_ticks >= a->counter->min_cycles){
        a->cycle = cyc_ticks % a->counter->max_allowed_Value;
    } else{
      return E_OS_VALUE;
    }
    __enable_irq();
    return E_OK;
 }

 /*
  * 2) SetAbsAlarm
  */
StatusType SetAbsAlarm(AlarmType aid, TickType start, TickType cycle){

     if(aid >= OS_MAX_ALARMS) return E_OS_LIMIT;

   // OsCounterCtl *c = alarm_to_counter[aid];
    uint32_t inc_ticks = ms_to_tick(start);
    uint32_t cyc_ticks = ms_to_tick(cycle);
    if((cyc_ticks > 0u) && (cyc_ticks < 1u)){
        cyc_ticks = 1u;
    }

    __disable_irq();
    OsAlarmCtl *a = &alarm_tbl[aid];
    a->active = 1u;
    a->Expiry_tick = inc_ticks & a->counter->max_allowed_Value;

    if(cyc_ticks >= a->counter->min_cycles){
        a->cycle = cyc_ticks % a->counter->max_allowed_Value;
    } else{
      return E_OS_VALUE;
    }

    __enable_irq();
    return E_OK;
}

/*
 * CancleAlarm()
*/
StatusType CancelAlarm(AlarmType alarm){

   OsAlarmCtl *a = &alarm_tbl[alarm];
  if (alarm >= OS_MAX_ALARMS) {
    return E_OS_ID;
  }

  if (a->active) {
    return E_OS_STATE;
  }

  /* Huỷ hoạt động alarm. (Có thể cần critical section tuỳ kiến trúc.) */
  a->active = 0u;
  return E_OK;
}

void Os_Alarm_Init(void){
    alarm_tbl[0].counter      = &Counter_tbl[0];
    alarm_tbl[0].action_type  = ALARMACTION_ACTIVATETASK;
    alarm_tbl[0].action.task_id = TASK_A;

    alarm_tbl[1].counter      = &Counter_tbl[0];
    alarm_tbl[1].action_type  = ALARMACTION_ACTIVATETASK;
    alarm_tbl[1].action.task_id = TASK_B;

    alarm_tbl[2].counter      = &Counter_tbl[0];
    alarm_tbl[2].action_type  = ALARMACTION_ACTIVATETASK;
    alarm_tbl[2].action.task_id = TASK_C;
}
void os_alarm_tick(void){
   for(int i=0; i< OS_MAX_ALARMS; i++){
        OsAlarmCtl *a = &alarm_tbl[i];
        
        if (!a->active)
            continue;

        if (a->Expiry_tick == a->counter->current_value)
        {
            // ActivateTask(a->action.task_id);
            switch(a->action_type){
                case ALARMACTION_ACTIVATETASK:
                    /* Kích hoạt task đích */
                    ActivateTask(a->action.task_id);
                    break;
                case ALARMACTION_SETEVENT:
                    SetEvent(a->action.Set_event.task_id, a->action.Set_event.mask);
                    break;
                case ALARMACTION_CALLBACK:
                    a->action.callback();
                    break;
            }
            /* Lặp hay one-shot */
            if ((a->cycle > 0u) && (a-> cycle >= a->counter->min_cycles)) {
                a->Expiry_tick = a->cycle; /* nạp lại chu kỳ */
            } else {
                a->active = 0u; /* one-shot → tắt */
            }   
        }
  }
}
