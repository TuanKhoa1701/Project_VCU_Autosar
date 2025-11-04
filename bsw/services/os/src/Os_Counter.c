/****************************************************
 * @file    Os_Counter.c
 * @brief   Triển khai API Counter của OS
 * @details Hỗ trợ bộ đếm thời gian (TickType) và sử 
 *          dụng các API như:
 *          - GetCounterValue(): Lấy giá trị hiện tại của counter
 *          - IncrementCounter(): Tăng giá trị Counter (gọi từ Systick)
 * @version 1.0
 * @date    2025 -09-10
 * @author  Nguyễn Tuấn Khoa
 ***************************************************/
#include "Os.h"
#include "Os_Arch.h"
#include "Os_Cfg.h"

volatile TickType s_tick = 0;
OsCounterCtl Counter_tbl[OS_MAX_COUNTERS]={
    // counter 0
    {
        .max_allowed_Value = 100,
        .current_value     = 0,
        .min_cycles        = 1,
        .ticks_per_base    = 1  
    },

    // counter 1
    {
        .max_allowed_Value = 5000,
        .current_value     = 0,
        .min_cycles        = 1,
        .ticks_per_base    = 100 
    },

};
StatusType IncrementCounter(CounterTypeId cid){
    if(cid >= OS_MAX_COUNTERS) return E_OS_ID;
    s_tick++;
    // Dòng mã gốc đã được di chuyển vào os_on_tick,nhưng logic đúng để tăng counter nên nằm ở đây.
    // Logic này giả định mỗi lần gọi là một tick.
    OsCounterCtl *c = (OsCounterCtl*)&Counter_tbl[cid];
    if(s_tick  == c->ticks_per_base){
        c->current_value = (c->current_value + 1) % c->max_allowed_Value;\
        s_tick = 0;
    }

    // Logic s_tick cũ có thể không cần thiết nếu mỗi counter có logic riêng
    return E_OK ;
}
StatusType GetCounterValue(CounterTypeId cid, TickRefType value){
    if(cid >= OS_MAX_COUNTERS) return E_OS_ID;

    if (value == NULL) return E_OS_ID;
    OsCounterCtl *c = &Counter_tbl[cid];
    *value = c->current_value;
    return E_OK;
}
