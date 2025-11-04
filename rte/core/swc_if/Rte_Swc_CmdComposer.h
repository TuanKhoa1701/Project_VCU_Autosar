#ifndef RTE_SWC_CMDCOMPOSER_H
#define RTE_SWC_CMDCOMPOSER_H

#include "Rte.h"
#include "Rte_Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* =========================
     * Ports of SWC: CmdComposer
     *  - RPort: SafeOut (SR Require) -> Safe_s
     *  - RPort: CanIf (Client-Server) -> TransmitVcuCommand()
     * ========================= */

    /* -------- SR Require (đọc SafeOut) -------- */
    Std_ReturnType Rte_Read_Swc_CmdComposer_SafeOut_Cmd(Safe_s *data);
#define Rte_Read_SafeOut_Cmd Rte_Read_Swc_CmdComposer_SafeOut_Cmd

    /* -------- Client-Server (CanIf) -------- */
    Std_ReturnType Rte_Call_Swc_CmdComposer_CanIf_TransmitVcuCommand(uint8_t throttle_pct,
                                                                     uint8_t gear,
                                                                     uint8_t mode,
                                                                     boolean brake,
                                                                     uint8_t alive);
#define Rte_Call_CanIf_TransmitVcuCommand Rte_Call_Swc_CmdComposer_CanIf_TransmitVcuCommand

    /* -------- Runnables -------- */
    void Swc_CmdComposer_Run10ms(void);

#ifdef __cplusplus
}
#endif

#endif /* RTE_SWC_CMDCOMPOSER_H */
