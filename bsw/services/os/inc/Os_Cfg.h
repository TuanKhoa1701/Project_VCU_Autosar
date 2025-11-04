#ifndef OS_CFG_H
#define OS_CFG_H

#include <stdint.h>
#include "Std_Types.h"

#define MAX_EXPIRY_POINTS       3
#define IOC_BUFFER_SIZE         4
#define MAX_IOC_CHANNELS        1

#define MAX_RESOURCES           4

#define OS_10MS_TICKS           10u
#define OS_TICK_HZ              1000u   /* 1ms */

#define OS_MAX_TASKS            5u      /* Init, A, B, Idle */
#define OS_MAX_ALARMS           3u      /* AlarmA, AlarmB   */
#define OS_MAX_COUNTERS         2u
#define OS_MAX_SchedTbl         2u
/* Stack size (word = 4 byte) */
#define STACK_WORDS_INIT        256u
#define STACK_WORDS_A           256u
#define STACK_WORDS_B           256u
#define STACK_WORDS_C           256U
#define STACK_WORDS_IDLE        128u
/* ID Task */
typedef enum {
    TASK_INIT = 0,
    TASK_A,
    TASK_B,
    TASK_C,
    TASK_IDLE,
    TASK_COUNT /* = OS_MAX_TASKS */
} TaskId_e;

/* ID Alarm */
typedef enum {
    ALARM_A = 0,
    ALARM_B,
    Alarm_Count
} AlarmId_e;

typedef enum{
    Ioc_CH_1,
    Ioc_CH_COUNT
}Ioc_Channels;



/**********************************************************
 * 2) EVENT MASKS cho Extended Task
 *    Dùng bởi Task_Com:
 *      - EV_RX : có dữ liệu nhận cần xử lý
 *      - EV_TX : có yêu cầu truyền cần xử lý
 **********************************************************/
#define EV_RX   ((EventMaskType)0x0001u)
#define EV_TX   ((EventMaskType)0x0002u)

/**********************************************************
 * 5) PROTOTYPES TASK (cho trình biên dịch biết sớm)
 *    Ứng dụng sẽ định nghĩa thân hàm trong app/tasks/*.c
 **********************************************************/
#define DECLARE_TASK(name)  void name(void)

DECLARE_TASK(Task_Init);
DECLARE_TASK(Task_A);
DECLARE_TASK(Task_B);
DECLARE_TASK(Task_C);
DECLARE_TASK(Task_Idle);


DECLARE_TASK(SetMode_Normal);
DECLARE_TASK(SetMode_Warning);
DECLARE_TASK(SetMode_Off);

#endif /* OS_CFG_H */
