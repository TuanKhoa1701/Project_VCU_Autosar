/**********************************************************
 * @file    Rte.c
 * @brief   Runtime Environment (RTE) tối giản cho MCU
 * @details Kết dính SWC ↔ BSW theo phong cách AUTOSAR:
 *          - Sender/Receiver (SR, non-queued) giữa các SWC:
 *              Rte_Write_*() ghi vào “shadow buffers” nội bộ,
 *              Rte_Read_*() đọc lại giá trị hiện hành.
 *          - Client/Server (CS): ủy quyền trực tiếp xuống IoHwAb_*().
 *          - Truyền thông (Tx/Rx) qua AUTOSAR COM:
 *              Com_SendSignal() / Com_TriggerIPDUSend() / Com_ReceiveSignal().
 *
 *          Lưu ý vận hành trên MCU (không mô phỏng PC):
 *            • Khởi tạo COM/MCAL/IoHwAb phải được thực hiện ở lớp BSW
 *              (EcuM/ComM/SchM/InitTask). RTE **không** gọi Com_Init().
 *            • Nếu SR buffers có thể được truy cập từ ISR và Task khác
 *              nhau, cần đảm bảo bảo vệ truy cập (tắt IRQ ngắn / spinlock).
 *            • Các API dưới đây là synchronous, non-reentrant theo mặc định.
 *
 * @version 1.2
 * @date    2025-09-10
 * @author  HALA Academy
 **********************************************************/
/* rte/core/src/Rte.c */
#include "Rte.h"
#include "Rte_Types.h"

/* Kéo vào các Application Headers để bảo đảm prototype khớp */
#include "Rte_Swc_PedalAcq.h"
#include "Rte_Swc_BrakeAcq.h"
#include "Rte_Swc_GearSelector.h"
#include "Rte_Swc_DriveModeMgr.h"
#include "Rte_Swc_SafetyManager.h"
#include "Rte_Swc_CmdComposer.h"

/* Forward tới IoHwAb / CanIf (Client-Server) */
#include "IoHwAb_Adc.h"     /* Std_ReturnType IoHwAb_Adc_ReadChannel(uint8, uint16*) */
#include "IoHwAb_Digital.h" 
#include "CanIf.h" 

/* Forward tới COM (Proxy) */
#include "Com.h"
#include "Com_Cfg.h"
#include "IoHwAb.h"
#ifdef __cplusplus
extern "C"
{
#endif

    /* =======================================================
     *                   RTE INTERNAL BUFFERS
     *  (SR non-queued explicit + IsUpdated flags + Mode value)
     * ======================================================= */

    /* PedalAcq -> PedalOut.PedalPct (uint8) */
    static uint8_t Rte_Buffer_PedalOut_PedalPct = 0u;
    static boolean Rte_IsUpdated_PedalOut_PedalPct_Flag = FALSE;

    /* BrakeAcq -> BrakeOut.BrakePressed (boolean) */
    static boolean Rte_Buffer_BrakeOut_BrakePressed = FALSE;
    static boolean Rte_IsUpdated_BrakeOut_BrakePressed_Flag = FALSE;

    /* GearSelector -> GearOut.Gear (Gear_e) */
    static Gear_e Rte_Buffer_GearOut_Gear = GEAR_P;
    static boolean Rte_IsUpdated_GearOut_Gear_Flag = FALSE;

    /* SafetyManager -> SafeOut.Cmd (Safe_s) */
    static Safe_s Rte_Buffer_SafeOut_Cmd = {0};
    static boolean Rte_IsUpdated_SafeOut_Cmd_Flag = FALSE;

    /* COM Proxy Buffers for VCU_Command Tx PDU */
    static uint8_t Rte_Buffer_VcuCmdTx_Throttle = 0u;
    static uint8_t Rte_Buffer_VcuCmdTx_Gear = 0u;
    static uint8_t Rte_Buffer_VcuCmdTx_Mode = 0u;
    static boolean Rte_Buffer_VcuCmdTx_Brake = FALSE;
    static uint8_t Rte_Buffer_VcuCmdTx_Alive = 0u;

    /* Mode port: DriveMode */
    static DriveMode_e Rte_Mode_DriveMode_Value = DRIVEMODE_ECO;
    static boolean Rte_Mode_DriveMode_SwitchPendingAck = FALSE;

    /* Lifecycle state (đơn giản) */
    static boolean Rte_Core_Started = FALSE;
    static boolean Rte_Timing_Activated = FALSE;

    /* =======================================================
     *                     LIFECYCLE API
     * ======================================================= */

    Std_ReturnType Rte_Start(void)
    {
        /* Reset buffers & flags về trạng thái an toàn */
        Rte_Buffer_PedalOut_PedalPct = 0u;
        Rte_IsUpdated_PedalOut_PedalPct_Flag = FALSE;

        Rte_Buffer_BrakeOut_BrakePressed = FALSE;
        Rte_IsUpdated_BrakeOut_BrakePressed_Flag = FALSE;

        Rte_Buffer_GearOut_Gear = GEAR_P;
        Rte_IsUpdated_GearOut_Gear_Flag = FALSE;

        Rte_Buffer_SafeOut_Cmd.throttle_pct = 0u;
        Rte_Buffer_SafeOut_Cmd.gear = GEAR_P;
        Rte_Buffer_SafeOut_Cmd.driveMode = DRIVEMODE_ECO;
        Rte_Buffer_SafeOut_Cmd.brakeActive = FALSE;
        Rte_IsUpdated_SafeOut_Cmd_Flag = FALSE;

        Rte_Mode_DriveMode_Value = DRIVEMODE_ECO;
        Rte_Mode_DriveMode_SwitchPendingAck = FALSE;

        Rte_Core_Started = TRUE;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_Stop(void)
    {
        Rte_Timing_Activated = FALSE;
        Rte_Core_Started = FALSE;
        return RTE_E_OK;
    }
    void Rte_Tick10ms(void)
    {
    }
    void Rte_Init_Core(void)
    {
        /* Nếu có initialization runnables, gọi ở đây theo thứ tự mong muốn.
           Dự án tối giản hiện không có, nên giữ trống. */
    }

    void Rte_StartTiming(void)
    {
        /* Cho phép OS/Scheduler kích runnable theo tick; RTE bản tối giản không tự lập lịch */
        Rte_Timing_Activated = TRUE;
        (void)Rte_Timing_Activated; /* tránh cảnh báo nếu chưa dùng */
    }

       /**
     * @brief Khởi tạo RTE, được gọi từ InitTask.
     * @details Hàm này là điểm vào công khai để bắt đầu RTE. Nó gọi các
     *          hàm khởi tạo nội bộ như Rte_Start() để reset trạng thái.
     */
    void Rte_Init(void)
    {
        (void)Rte_Start();
        /* Có thể gọi thêm Rte_Init_Core() ở đây nếu cần chạy các init-runnable */
        Rte_StartTiming();
    }
    /* =======================================================
     *               SENDER-RECEIVER — WRITE (Provide)
     * ======================================================= */
    /* PedalAcq -> PedalOut.PedalPct */
    Std_ReturnType Rte_Write_PedalAcq_PedalOut(uint8_t data)
    {
        Rte_Buffer_PedalOut_PedalPct = data;
        Rte_IsUpdated_PedalOut_PedalPct_Flag = TRUE;
        return RTE_E_OK;
    }

    /* BrakeAcq -> BrakeOut.BracePressed */
    Std_ReturnType Rte_Write_BrakeAcq_BrakeOut(boolean data)
    {
        Rte_Buffer_BrakeOut_BrakePressed = data;
        Rte_IsUpdated_BrakeOut_BrakePressed_Flag = TRUE;
        return RTE_E_OK;
    }

    /* GearSelector -> GearOut.Gear */
    Std_ReturnType Rte_Write_GearSelector_GearOut(Gear_e data)
    {
        Rte_Buffer_GearOut_Gear = data;
        Rte_IsUpdated_GearOut_Gear_Flag = TRUE;
        return RTE_E_OK;
    }

    /* SafetyManager -> SafeOut.Cmd */
    Std_ReturnType Rte_Write_SafetyManager_SafeOut(const Safe_s *data)
    {
        if (data == NULL)
        {
            return RTE_E_INVALID;
        }
        Rte_Buffer_SafeOut_Cmd = *data;
        Rte_IsUpdated_SafeOut_Cmd_Flag = TRUE;
        return RTE_E_OK;
    }

    /* =======================================================
     *           SENDER-RECEIVER — READ (Require)
     * (clear IsUpdated flag khi đọc để báo đã tiêu thụ)
     * ======================================================= */
    Std_ReturnType Rte_Read_SafetyManager_PedalOut(uint8_t *data)
    {
        if (data == NULL)
        {
            return RTE_E_INVALID;
        }
        *data = Rte_Buffer_PedalOut_PedalPct;
        Rte_IsUpdated_PedalOut_PedalPct_Flag = FALSE;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_Read_SafetyManager_BrakeOut(boolean *data)
    {
        if (data == NULL)
        {
            return RTE_E_INVALID;
        }
        *data = Rte_Buffer_BrakeOut_BrakePressed;
        Rte_IsUpdated_BrakeOut_BrakePressed_Flag = FALSE;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_Read_SafetyManager_GearOut(Gear_e *data)
    {
        if (data == NULL)
        {
            return RTE_E_INVALID;
        }
        *data = Rte_Buffer_GearOut_Gear;
        Rte_IsUpdated_GearOut_Gear_Flag = FALSE;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_Read_CmdComposer_SafeOut(Safe_s *data)
    {
        if (data == NULL)
        {
            return RTE_E_INVALID;
        }
        *data = Rte_Buffer_SafeOut_Cmd;
        Rte_IsUpdated_SafeOut_Cmd_Flag = FALSE;
        return RTE_E_OK;
    }

    /* =======================================================
     *            SENDER-RECEIVER — IsUpdated helpers
     * ======================================================= */
    Std_ReturnType Rte_IsUpdated_Swc_PedalAcq_PedalOut_PedalPct(boolean *updated)
    {
        if (updated == NULL)
        {
            return RTE_E_INVALID;
        }
        *updated = Rte_IsUpdated_PedalOut_PedalPct_Flag;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_IsUpdated_Swc_BrakeAcq_BrakeOut_BrakePressed(boolean *updated)
    {
        if (updated == NULL)
        {
            return RTE_E_INVALID;
        }
        *updated = Rte_IsUpdated_BrakeOut_BrakePressed_Flag;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_IsUpdated_Swc_GearSelector_GearOut_Gear(boolean *updated)
    {
        if (updated == NULL)
        {
            return RTE_E_INVALID;
        }
        *updated = Rte_IsUpdated_GearOut_Gear_Flag;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_IsUpdated_Swc_SafetyManager_SafeOut_Cmd(boolean *updated)
    {
        if (updated == NULL)
        {
            return RTE_E_INVALID;
        }
        *updated = Rte_IsUpdated_SafeOut_Cmd_Flag;
        return RTE_E_OK;
    }

    /* =======================================================
     *               MODE MANAGEMENT — DriveMode
     * ======================================================= */
    Std_ReturnType Rte_Write_DriveModeMgr_DriveModeOut(DriveMode_e mode)
    {
        /* Validate input (tối giản) */
        if (mode != DRIVEMODE_ECO && mode != DRIVEMODE_NORMAL)
        {
            return RTE_E_LIMIT;
        }
        Rte_Mode_DriveMode_Value = mode;
        Rte_Mode_DriveMode_SwitchPendingAck = TRUE;
        return RTE_E_OK;
    }

    /* Hai hàm Mode_Read cho 2 SWC consumer khác nhau (cùng trả một giá trị) */
    DriveMode_e Rte_Mode_Swc_DriveModeMgr_DriveMode_Mode(void)
    {
        return Rte_Mode_DriveMode_Value;
    }
    Std_ReturnType Rte_Read_SafetyManager_DriveModeOut(DriveMode_e* data)
    {
        if (data == NULL)
        {
            return RTE_E_INVALID;
        }
        *data = Rte_Mode_DriveMode_Value;
        Rte_Mode_DriveMode_SwitchPendingAck = TRUE;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_SwitchAck_Swc_DriveModeMgr_DriveMode_Mode(void)
    {
        if (Rte_Mode_DriveMode_SwitchPendingAck == TRUE)
        {
            Rte_Mode_DriveMode_SwitchPendingAck = FALSE;
            return RTE_E_OK;
        }
        return RTE_E_NO_DATA; /* không có switch đang chờ ACK */
    }

    /* =======================================================
     *          PROXY for COM Tx (VCU_Command)
     * ======================================================= */
    Std_ReturnType Rte_Write_CmdComposer_VcuCmdTx_ThrottleReq_pct(uint8_t v)
    {
        Rte_Buffer_VcuCmdTx_Throttle = v;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_Write_CmdComposer_VcuCmdTx_GearSel(uint8_t gear)
    {
        Rte_Buffer_VcuCmdTx_Gear = gear;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_Write_CmdComposer_VcuCmdTx_DriveMode(uint8_t mode)
    {
        Rte_Buffer_VcuCmdTx_Mode = mode;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_Write_CmdComposer_VcuCmdTx_BrakeActive(boolean b)
    {
        Rte_Buffer_VcuCmdTx_Brake = b;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_Write_CmdComposer_VcuCmdTx_AliveCounter(uint8_t nibble)
    {
        Rte_Buffer_VcuCmdTx_Alive = nibble;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_Trigger_CmdComposer_VcuCmdTx(void)
    {
        Std_ReturnType ret = RTE_E_OK;
        Std_ReturnType op_ret;

        /* 1. Gửi từng signal vào shadow buffer của COM */
        op_ret = Com_SendSignal(ComConf_ComSignal_VCU_ThrottleReq_pct, &Rte_Buffer_VcuCmdTx_Throttle);
        if (op_ret != E_OK) { ret = RTE_E_NOT_OK; }

        op_ret = Com_SendSignal(ComConf_ComSignal_VCU_GearSel, &Rte_Buffer_VcuCmdTx_Gear);
        if (op_ret != E_OK) { ret = RTE_E_NOT_OK; }

        op_ret = Com_SendSignal(ComConf_ComSignal_VCU_DriveMode, &Rte_Buffer_VcuCmdTx_Mode);
        if (op_ret != E_OK) { ret = RTE_E_NOT_OK; }

        op_ret = Com_SendSignal(ComConf_ComSignal_VCU_BrakeActive, &Rte_Buffer_VcuCmdTx_Brake);
        if (op_ret != E_OK) { ret = RTE_E_NOT_OK; }

        op_ret = Com_SendSignal(ComConf_ComSignal_VCU_Alive, &Rte_Buffer_VcuCmdTx_Alive);
        if (op_ret != E_OK) { ret = RTE_E_NOT_OK; }

        /* 2. Yêu cầu COM phát PDU đã được cập nhật */
        op_ret = Com_TriggerIPDUSend(ComConf_ComIPdu_VCU_Command);
        if (op_ret != E_OK) { ret = RTE_E_NOT_OK; }

        return ret;
    }
    void Rte_Com_Update_EngineSpeedFromPdu(const uint16_t* data){
        Com_ReceiveSignal(ComConf_ComSignal_EngineSpeedRpm, data);
    }

    /* =======================================================
     *          CLIENT–SERVER FORWARDING (IoHwAb / CanIf)
     * ======================================================= */
    Std_ReturnType Rte_Call_Swc_PedalAcq_AdcIf_ReadChannel(uint8_t channel, uint16_t *value)
    {
        return IoHwAb_Adc_ReadChannel(channel, value);
    }

    Std_ReturnType Rte_Call_Swc_BrakeAcq_DigIf_ReadChannel(uint8_t channel, boolean *level)
    {
        return IoHwAb_Digital_ReadChannel(channel, level);
    }

    Std_ReturnType Rte_Call_Swc_BrakeAcq_DigIf_GetDebounced(uint8_t channel, boolean *level)
    {
        return IoHwAb_Digital_GetDebounced(channel, level);
    }

    Std_ReturnType Rte_Call_Swc_GearSelector_DigIf_ReadChannel(uint8_t channel, boolean *level)
    {
        return IoHwAb_Digital_ReadChannel(channel, level);
    }

    Std_ReturnType Rte_Call_Swc_DriveModeMgr_DigIf_ReadChannel(uint8_t channel, boolean *level)
    {
        return IoHwAb_Digital_ReadChannel(channel, level);
    }

    Std_ReturnType Rte_Call_Swc_CmdComposer_CanIf_TransmitVcuCommand(uint8_t throttle_pct,
                                                                     uint8_t gear,
                                                                     uint8_t mode,
                                                                     boolean brake,
                                                                     uint8_t alive)
    {
        return CanIf_Transmit_VcuCommand(throttle_pct, gear, mode, brake, alive);
    }

    Std_ReturnType Rte_Call_DriveModeMgr_IoHwAb_Mode_Get(DriveMode_e *mode)
    {
        return IoHwAb_Mode_Get(mode);
    }

    Std_ReturnType Rte_Call_GearSelector_IoHwAb_Gear_Get(Gear_e *gear, boolean *valid)
    {
        return IoHwAb_Gear_Get(gear, valid);
    }

    Std_ReturnType Rte_Call_BrakeAcq_IoHwAb_Brake_Get(boolean *pressed)
    {
        return IoHwAb_Brake_Get(pressed);
    }
    Std_ReturnType Rte_Call_PedalAcq_IoHwAb_Pedal_ReadPct(uint8_t *pct)
    {
        return IoHwAb_Pedal_ReadPct(pct);
    }


#ifdef __cplusplus
}
#endif
