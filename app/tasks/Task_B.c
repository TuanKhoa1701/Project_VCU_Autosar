#include "Os.h"
#include <stdio.h> 
#include "Swc_CmdComposer.h"
#include "Rte.h"
#include "stm32f10x.h"
#include "stm32f10x_can.h"
/* Task chu kỳ 100 ms: BSW & SWC ít thường xuyên hơn */
TASK(Task_B)
{
    /* Tổng hợp lệnh VCU_Command (ghi từng signal vào COM qua RTE) */
    Swc_CmdComposer_Run10ms(); 
    //Ioc_Receive(Ioc_CH_1, &data, TASK_B);
    // printf("[Task_B] Data Receceive:%d\n",data);
    uint16_t data;
    Swc_CmdComposer_ReadEngineRPM(&data);
    printf("[Task_B] Engine Speed:%d\n",data);
    TerminateTask();
}
