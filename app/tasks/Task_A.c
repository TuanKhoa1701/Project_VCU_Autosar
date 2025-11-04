/**********************************************************
 * @file    Task_10ms.c
 * @brief   Task chu kỳ 10 ms
 * @details Thứ tự khuyến nghị:
 *          1) SWC acquisition: đọc phần cứng → ghi SR vào RTE
 *          2) SWC quản lý chế độ / an toàn
 *          3) SWC tổng hợp lệnh (ghi các signal Tx vào COM qua RTE)
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/
#include "Os.h"
#include "Swc_PedalAcq.h"
#include "Swc_BrakeAcq.h"
#include "Swc_GearSelector.h"
#include "Swc_DriveModeMgr.h"
#include "Swc_SafetyManager.h"
#include "Swc_CmdComposer.h"
#include <stdio.h>
#include "IoHwAb_Digital.h"
#include "IoHwAb_Digital_Cfg.h"
TASK(Task_A)
{
     /* 1) Thu nhận tín hiệu đầu vào từ phần cứng (qua IoHwAb → RTE) */
    Swc_PedalAcq_Run10ms();
    Swc_DriveModeMgr_Run10ms();
    Swc_BrakeAcq_Run10ms();
    Swc_GearSelector_Run10ms();
     /* 2) An toàn: hợp nhất & kiểm tra điều kiện (ghi Safe_s vào RTE) */
    Swc_SafetyManager_Run10ms();

    // IoHwAb_Init1(&IoHwAb1_Config);
    // // if(IoHwAb_Digital_ReadSignal(IoHwAb_CHANNEL_Button, &btn) == E_OK && !btn){
    //     SetEvent(TASK_B, EV_RX);
    // }
    // static uint16_t accA = 0, accB =0;
    // const uint16_t period_normal =150;
    // const uint16_t period_warn   =50;

    // switch (g_mode){
    //     case MODE_NORMAL:
    //         accA += 50;
    //         if(accA == period_normal){
    //             printf("Mode normal\n");
    //             accA=0;
    //         }
    //         break;
    //     case MODE_WARNING:  
    //         accB += 50;
    //         if(accB == period_warn){
    //             printf("Mode warning\n");
    //             accB=0;
    //         }
    //         break;
    //     default:
    //     printf("Task_A running...\n");   
    //     accA = accB = 0;
    //     break;
    // }
    // Ioc_Send(Ioc_CH_1, &speed);
    // printf("Send Speed:%d\n",speed);
    // TerminateTask(
    // printf("Task_A running...\n");
    TerminateTask();
}
