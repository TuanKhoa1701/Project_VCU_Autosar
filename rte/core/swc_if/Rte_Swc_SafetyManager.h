#ifndef RTE_SWC_SAFETYMANAGER_H
#define RTE_SWC_SAFETYMANAGER_H

#include "Rte.h"
#include "Rte_Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* =========================
     * Ports of SWC: SafetyManager
     *  - RPort: PedalOut (SR Require) -> PedalPct
     *  - RPort: BrakeOut (SR Require) -> BrakePressed
     *  - RPort: GearOut (SR Require)  -> Gear
     *  - Mode  : DriveMode.Mode
     *  - PPort: SafeOut (SR Provide)  -> Safe_s
     *  - (tuỳ chọn) Exclusive Area
     * ========================= */

    /* -------- SR Require (đọc input) -------- */
    Std_ReturnType Rte_Read_Swc_SafetyManager_PedalOut_PedalPct(uint8_t *data);
#define Rte_Read_PedalOut_PedalPct Rte_Read_Swc_SafetyManager_PedalOut_PedalPct

    Std_ReturnType Rte_Read_Swc_SafetyManager_BrakeOut_BrakePressed(boolean *data);
#define Rte_Read_BrakeOut_BrakePressed Rte_Read_Swc_SafetyManager_BrakeOut_BrakePressed

    Std_ReturnType Rte_Read_Swc_SafetyManager_GearOut_Gear(Gear_e *data);
#define Rte_Read_GearOut_Gear Rte_Read_Swc_SafetyManager_GearOut_Gear

    /* -------- Mode Read -------- */
    DriveMode_e Rte_Mode_Swc_SafetyManager_DriveMode_Mode(void);
#define Rte_Mode_DriveMode_Mode Rte_Mode_Swc_SafetyManager_DriveMode_Mode

    /* -------- SR Provide (ghi output SafeOut) -------- */
    Std_ReturnType Rte_Write_Swc_SafetyManager_SafeOut_Cmd(const Safe_s *data);
#define Rte_Write_SafeOut_Cmd Rte_Write_Swc_SafetyManager_SafeOut_Cmd

    Std_ReturnType Rte_IsUpdated_Swc_SafetyManager_SafeOut_Cmd(boolean *updated);
#define Rte_IsUpdated_SafeOut_Cmd Rte_IsUpdated_Swc_SafetyManager_SafeOut_Cmd

    /* -------- Exclusive Area (tuỳ chọn) -------- */
    void Rte_Enter_Swc_SafetyManager_EA(void);
    void Rte_Exit_Swc_SafetyManager_EA(void);
#define Rte_Enter_Safety_EA Rte_Enter_Swc_SafetyManager_EA
#define Rte_Exit_Safety_EA Rte_Exit_Swc_SafetyManager_EA

    /* -------- Runnables -------- */
    void Swc_SafetyManager_Run10ms(void);

#ifdef __cplusplus
}
#endif

#endif /* RTE_SWC_SAFETYMANAGER_H */
