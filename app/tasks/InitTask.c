/**********************************************************
 * @file    InitTask.c
 * @brief   Task khởi tạo hệ thống (autostart)
 * @details - Khởi tạo RTE, SWC, COM (nếu COM chưa được BSW init)
 *          - Cấu hình các Alarm chu kỳ (10ms/100ms)
 *          - Kết thúc bản thân (TerminateTask)
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/
#include "Os.h"
#include "EcuM.h"
#include "Rte.h"
#include "Com.h"               /* Nếu COM init ở đây */
#include "Swc_PedalAcq.h"
#include "Swc_BrakeAcq.h"
#include "Swc_GearSelector.h"
#include "Swc_DriveModeMgr.h"
#include "Swc_SafetyManager.h"
#include "Swc_CmdComposer.h"
#include "IoHwAb_Digital.h"
#include "IoHwAb_Digital_Cfg.h"
#include "PduR.h"
#include "PduR_Cfg.h"

TASK(Task_Init)
{
    EcuM_StartupTwo();

    /* Khởi tạo I/O Hardware Abstraction */
    IoHwAb_Init1(&IoHwAb1_Config);

    /* Khởi tạo các layer communication*/
    Com_Init();
    PduR_Init(&PduR_Config);
    CanIf_Init(&My_CanIf_Config);
    
     /* Khởi tạo RTE và các SWC */
    Rte_Init();
    Swc_PedalAcq_Init();
    Swc_BrakeAcq_Init();
    Swc_GearSelector_Init();
    Swc_DriveModeMgr_Init();
    Swc_SafetyManager_Init();
    Swc_CmdComposer_Init();

    //Ioc_Init(Ioc_CH_1, 2, Rec_list);
    //StartScheduleTableRel(0,50);
    SetRelAlarm(0u, 10u,  10u);
    SetRelAlarm(1u, 60u,  70u);
    // SetRelAlarm(2u, 700u,  500u);
    TerminateTask();
}
