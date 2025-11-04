#ifndef RTE_SWC_GEARSELECTOR_H
#define RTE_SWC_GEARSELECTOR_H

#include "Rte.h"
#include "Rte_Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* =========================
     * Ports of SWC: GearSelector (DRNP)
     *  - RPort: DigIf (Client-Server) -> đọc các chân D/R/N/P
     *  - PPort: GearOut (Sender-Receiver, non-queued) -> xuất Gear_e
     * ========================= */

    /* -------- Client-Server (DigIf) -------- */
    Std_ReturnType Rte_Call_Swc_GearSelector_DigIf_ReadChannel(uint8_t channel, boolean *level);
#define Rte_Call_DigIf_ReadChannel Rte_Call_Swc_GearSelector_DigIf_ReadChannel

    /* -------- Sender-Receiver (Provide: GearOut) -------- */
    Std_ReturnType Rte_Write_Swc_GearSelector_GearOut_Gear(Gear_e data);
#define Rte_Write_GearOut_Gear Rte_Write_Swc_GearSelector_GearOut_Gear

    Std_ReturnType Rte_IsUpdated_Swc_GearSelector_GearOut_Gear(boolean *updated);
#define Rte_IsUpdated_GearOut_Gear Rte_IsUpdated_Swc_GearSelector_GearOut_Gear

    /* -------- Runnables -------- */
    void Swc_GearSelector_Run10ms(void);

#ifdef __cplusplus
}
#endif

#endif /* RTE_SWC_GEARSELECTOR_H */
