#ifndef RTE_SWC_BRAKEACQ_H
#define RTE_SWC_BRAKEACQ_H

#include "Rte.h"
#include "Rte_Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* =========================
     * Ports of SWC: BrakeAcq
     *  - RPort: DigIf (Client-Server) -> đọc DIO
     *  - PPort: BrakeOut (Sender-Receiver, non-queued) -> xuất BrakePressed
     * ========================= */

    /* -------- Client-Server (DigIf) -------- */

    Std_ReturnType Rte_Call_Swc_BrakeAcq_DigIf_ReadChannel(uint8_t channel, boolean *level);
#define Rte_Call_DigIf_ReadChannel Rte_Call_Swc_BrakeAcq_DigIf_ReadChannel

    /* (tuỳ chọn) CS debounce cao cấp */
    Std_ReturnType Rte_Call_Swc_BrakeAcq_DigIf_GetDebounced(uint8_t channel, boolean *level);
#define Rte_Call_DigIf_GetDebounced Rte_Call_Swc_BrakeAcq_DigIf_GetDebounced

    /* -------- Sender-Receiver (Provide: BrakeOut) -------- */

    Std_ReturnType Rte_Write_Swc_BrakeAcq_BrakeOut_BrakePressed(boolean data);
#define Rte_Write_BrakeOut_BrakePressed Rte_Write_Swc_BrakeAcq_BrakeOut_BrakePressed

    Std_ReturnType Rte_IsUpdated_Swc_BrakeAcq_BrakeOut_BrakePressed(boolean *updated);
#define Rte_IsUpdated_BrakeOut_BrakePressed Rte_IsUpdated_Swc_BrakeAcq_BrakeOut_BrakePressed

    /* -------- Runnables -------- */
    void Swc_BrakeAcq_Run10ms(void);

#ifdef __cplusplus
}
#endif

#endif /* RTE_SWC_BRAKEACQ_H */
