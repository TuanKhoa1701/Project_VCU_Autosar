#ifndef RTE_SWC_PEDALACQ_H
#define RTE_SWC_PEDALACQ_H

#include "Rte.h"
#include "Rte_Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* =========================
     * Ports of SWC: PedalAcq
     *  - RPort: AdcIf (Client-Server) -> đọc kênh ADC
     *  - PPort: PedalOut (Sender-Receiver, non-queued) -> xuất PedalPct
     * ========================= */

    /* -------- Client-Server (AdcIf) -------- */
    Std_ReturnType Rte_Call_Swc_PedalAcq_AdcIf_ReadChannel(uint8_t channel, uint16_t *value);
/* Tên rút gọn theo SWS (macro): */
#define Rte_Call_AdcIf_ReadChannel Rte_Call_Swc_PedalAcq_AdcIf_ReadChannel

    /* -------- Sender-Receiver (Provide: PedalOut) -------- */
    Std_ReturnType Rte_Write_Swc_PedalAcq_PedalOut_PedalPct(uint8_t data);
#define Rte_Write_PedalOut_PedalPct Rte_Write_Swc_PedalAcq_PedalOut_PedalPct

    /* (tuỳ chọn) trạng thái cập nhật DE */
    Std_ReturnType Rte_IsUpdated_Swc_PedalAcq_PedalOut_PedalPct(boolean *updated);
#define Rte_IsUpdated_PedalOut_PedalPct Rte_IsUpdated_Swc_PedalAcq_PedalOut_PedalPct

    /* -------- Runnables của SWC -------- */
    void Swc_PedalAcq_Run10ms(void);

#ifdef __cplusplus
}
#endif

#endif /* RTE_SWC_PEDALACQ_H */
