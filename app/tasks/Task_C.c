#include "Os.h"
#include <stdio.h> 
// #include "Rte.h"

/* Task chu kỳ 100 ms: BSW & SWC ít thường xuyên hơn */
TASK(Task_C)
{
    uint16_t data;

    //Ioc_Receive(Ioc_CH_1, &data, TASK_C);
    // printf("[Task_C] Data Receive:%d\n",data);

    TerminateTask();
}
