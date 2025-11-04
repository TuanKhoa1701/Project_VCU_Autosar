#ifndef RTE_SWC_DRIVEMODEMGR_H
#define RTE_SWC_DRIVEMODEMGR_H

#include "Rte.h"
#include "Rte_Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* =========================
     * Ports of SWC: DriveModeMgr
     *  - RPort: DigIf (Client-Server) -> đọc nút/logic chọn chế độ
     *  - Mode Port: DriveMode.Mode (Mode Management)
     * ========================= */

    /* -------- Client-Server (DigIf) -------- */
    Std_ReturnType Rte_Call_Swc_DriveModeMgr_DigIf_ReadChannel(uint8_t channel, boolean *level);
#define Rte_Call_DigIf_ReadChannel Rte_Call_Swc_DriveModeMgr_DigIf_ReadChannel

    /* -------- Mode Management (DriveMode) -------- */
    Std_ReturnType Rte_Switch_Swc_DriveModeMgr_DriveMode_Mode(DriveMode_e mode);
#define Rte_Switch_DriveMode_Mode Rte_Switch_Swc_DriveModeMgr_DriveMode_Mode

    DriveMode_e Rte_Mode_Swc_DriveModeMgr_DriveMode_Mode(void);
#define Rte_Mode_DriveMode_Mode Rte_Mode_Swc_DriveModeMgr_DriveMode_Mode

    /* (tuỳ chọn) ack khi switch */
    Std_ReturnType Rte_SwitchAck_Swc_DriveModeMgr_DriveMode_Mode(void);
#define Rte_SwitchAck_DriveMode_Mode Rte_SwitchAck_Swc_DriveModeMgr_DriveMode_Mode

    /* -------- Runnables -------- */
    void Swc_DriveModeMgr_Run10ms(void);

#ifdef __cplusplus
}
#endif

#endif /* RTE_SWC_DRIVEMODEMGR_H */
