#include "Os.h"
#include "stm32f10x.h"
#include <stdio.h> 


/* Task chu kỳ 100 ms: BSW & SWC ít thường xuyên hơn */
TASK(Task_Idle)
{
    for (;;)
    {
        __WFI();
    }
}
