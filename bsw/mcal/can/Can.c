/**********************************************************************************************************************
 * @file    Can.c
 * @brief   Triển khai (implementation) cho CAN Driver.
 * @details File này chứa phần thân của các hàm đã được định nghĩa trong Can.h.
 *          Nó tương tác trực tiếp với các thanh ghi của CAN controller (thông qua
 *          thư viện SPL) để thực hiện các chức năng như khởi tạo, gửi/nhận dữ liệu,
 *          và quản lý trạng thái.
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 *********************************************************************************************************************/
#include "Can.h"
#include <stdio.h>
#include "Can_Cfg.h"
#include "stm32f10x.h"
/**
 * @brief Các biến quản lý trạng thái của các mailbox truyền (Tx).
 * @details `mbx_busy` theo dõi mailbox nào đang bận. `swPduHandle` lưu PDU ID của lớp trên 
 *          tương ứng với mỗi mailbox, dùng cho việc xác nhận truyền (Tx Confirmation).
 */
static uint8_t mbx;
static uint8_t mbx_busy[CAN_MAX_TX_MAILBOX];
static PduIdType* swPduHandle[CAN_MAX_TX_MAILBOX];
/******************************************************************************
 * @brief Khởi tạo CAN controller với cấu hình được cung cấp.
 * @param[in] config Con trỏ tới cấu trúc cấu hình.
 *****************************************************************************/
void Can_Init(const Can_ConfigType* config) {
    CAN_InitTypeDef Can_InitStructure;
    // Kích hoạt Clock cho CAN1 trên bus APB1
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    
    // Cấu hình các tính năng nâng cao
    Can_InitStructure.CAN_TTCM = config->Basic_Config.CAN_TTCM;
    Can_InitStructure.CAN_ABOM = config->Basic_Config.CAN_ABOM;
    Can_InitStructure.CAN_AWUM = config->Basic_Config.CAN_AWUM;
    Can_InitStructure.CAN_NART = config->Basic_Config.CAN_NART;
    Can_InitStructure.CAN_RFLM = config->Basic_Config.CAN_RFLM;
    Can_InitStructure.CAN_TXFP = config->Basic_Config.CAN_TXFP;

    // Cấu hình timing và chế độ hoạt động cơ bản
    Can_InitStructure.CAN_Mode = config->Basic_Config.CAN_Mode; // Hoặc CAN_Mode_LoopBack nếu bạn đang test
    Can_InitStructure.CAN_SJW = config->Basic_Config.CAN_SJW;
    Can_InitStructure.CAN_BS1 = config->Basic_Config.CAN_BS1;
    Can_InitStructure.CAN_BS2 = config->Basic_Config.CAN_BS2;
    Can_InitStructure.CAN_Prescaler = config->Basic_Config.CAN_Prescaler;

    if(CAN_Init(CAN1, &Can_InitStructure) != CAN_InitStatus_Success){
        // printf("CAN Init failed!\n");
    };

    CAN_FilterInitTypeDef Can_FilterInitStructure;
    Can_FilterInitStructure.CAN_FilterNumber = 0;
    Can_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    Can_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    Can_FilterInitStructure.CAN_FilterIdHigh = config->Filter_Config.Can_FilterIdHigh;
    Can_FilterInitStructure.CAN_FilterIdLow = config->Filter_Config.Can_FilterIdLow;
    Can_FilterInitStructure.CAN_FilterMaskIdHigh = config->Filter_Config.Can_FilterMaskIdHigh;
    Can_FilterInitStructure.CAN_FilterMaskIdLow = config->Filter_Config.Can_FilterMaskIdLow;
    Can_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
    Can_FilterInitStructure.CAN_FilterActivation = ENABLE;

    CAN_FilterInit(&Can_FilterInitStructure);
    if(config->NotificationEnable == ENABLE){
        CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
        NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    }
}

/******************************************************************************
 * @brief Hủy khởi tạo CAN controller.
 *****************************************************************************/
void Can_DeInit(void) {
    CAN_DeInit(CAN1);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN1, ENABLE);
}

/******************************************************************************
 * @brief Lấy thông tin phiên bản của module.
 *****************************************************************************/
void Can_GetVersionInfo(Std_VersionInfoType* versioninfo){
    versioninfo->vendorID = CAN_VENDOR_ID;
    versioninfo->moduleID = CAN_MODULE_ID;
    versioninfo->sw_major_version = CAN_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = CAN_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = CAN_SW_PATCH_VERSION;
}

/******************************************************************************
 * @brief Thiết lập baudrate cho CAN controller.
 * @details Hàm này đưa controller vào chế độ Initialization, cấu hình lại
 *          các thông số timing, sau đó thoát khỏi chế độ Initialization.
 * @param[in] Controller ID của controller (chỉ hỗ trợ CAN_1).
 * @param[in] BaudRateConfigID ID của cấu hình baudrate (ví dụ: 125, 250, 500, 1000).
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 *****************************************************************************/
Std_ReturnType Can_SetBaudrate(uint8_t Controller, uint16_t BaudRateConfigID)
{
    if (Controller != CAN_1) return E_NOT_OK;  // CAN1 = controller 1
    CAN_TypeDef *CANx = CAN1;

    /* 1. Vào chế độ Initialization (tạm dừng CAN) */
    CAN_OperatingModeRequest(CANx, CAN_OperatingMode_Initialization);
    // Đợi cho đến khi vào chế độ Init thành công
    while ((CANx->MSR & CAN_MSR_INAK) == 0);

    /* 2. Cấu hình lại các thông số timing cho baudrate mới */
    CAN_InitTypeDef CAN_InitStructure;
    CAN_StructInit(&CAN_InitStructure);

    // giả sử bạn chọn baudrate qua ID
    switch (BaudRateConfigID)
    {
        case 125:  // 125 kbps
            CAN_InitStructure.CAN_Prescaler = 18; // 36MHz / (18 * (1+12+3)) = 125kbps
            CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
            CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
            break;  
        case 250:  // 250 kbps
            CAN_InitStructure.CAN_Prescaler = 9; // 36MHz / (9 * (1+12+3)) = 250kbps
            CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
            CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
            break;
        case 500:  // 500 kbps
            CAN_InitStructure.CAN_Prescaler = 4; // 36MHz / (4 * (1+13+4)) = 500kbps
            CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_4tq;
            CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
            break;
        case 1000:  // 1000 kbps
            CAN_InitStructure.CAN_Prescaler = 3;
            CAN_InitStructure.CAN_BS1 = CAN_BS1_7tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_4tq;
            CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
            break;
        default:
            return E_NOT_OK;
    }

    if(CAN_Init(CANx, &CAN_InitStructure) == CAN_InitStatus_Failed){
        return E_NOT_OK;
    };

    /* 3. Thoát khỏi chế độ Initialization để áp dụng cấu hình mới */
    CAN_OperatingModeRequest(CANx, CAN_OperatingMode_Normal);

    return E_OK;
}

/******************************************************************************
 * @brief Chuyển đổi trạng thái của CAN controller.
 * @param[in] Controller ID của controller (chỉ hỗ trợ CAN_1).
 * @param[in] Transition Trạng thái mong muốn (STARTED, STOPPED, SLEEP).
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 *****************************************************************************/
Std_ReturnType Can_SetControllerMode(uint8_t Controller, Can_ControllerStateType Transition)
{
    if (Controller != CAN_1)
        return E_NOT_OK;

    CAN_TypeDef *CANx = CAN1;

    switch (Transition)
    {
        /* === START (vào Normal mode) === */
        case CAN_CS_STARTED:
            CANx->MCR &= ~CAN_MCR_INRQ;
            if(CANx->MSR & CAN_MSR_INAK) return E_NOT_OK;
            break;
        /* === STOP (vào Init mode) === */
        case CAN_CS_STOPPED:
            CANx->MCR |= CAN_MCR_INRQ;
            if(!(CANx->MSR & CAN_MSR_INAK)) return E_NOT_OK;
            break;
        /* === SLEEP mode === */
        case CAN_CS_SLEEP:
            CANx->MCR |= CAN_MCR_SLEEP;  
            if(!(CANx->MSR & CAN_MSR_SLAK)) return E_NOT_OK;
            break;
        default:
            return E_NOT_OK;
    }

    return E_OK;
}

/******************************************************************************
 * @brief Vô hiệu hóa tất cả các ngắt của CAN controller.
 * @param[in] Controller ID của controller (chỉ hỗ trợ CAN_1).
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 *****************************************************************************/
Std_ReturnType Can_DisableControllerInterrupts(uint8_t Controller){
    if(Controller != CAN_1) return E_NOT_OK;

    CAN_ITConfig(CAN1, CAN_IT_FMP0, DISABLE);
    CAN_ITConfig(CAN1, CAN_IT_FMP1, DISABLE);
    CAN_ITConfig(CAN1, CAN_IT_TME, DISABLE);
    CAN_ITConfig(CAN1, CAN_IT_WKU, DISABLE);
    CAN_ITConfig(CAN1, CAN_IT_ERR, DISABLE);
    CAN_ITConfig(CAN1, CAN_IT_SLK, DISABLE);
    
    CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    CAN_ClearITPendingBit(CAN1, CAN_IT_FMP1);
    CAN_ClearITPendingBit(CAN1, CAN_IT_TME);
    CAN_ClearITPendingBit(CAN1, CAN_IT_WKU);
    CAN_ClearITPendingBit(CAN1, CAN_IT_ERR);
    CAN_ClearITPendingBit(CAN1, CAN_IT_SLK);
    
    return E_OK;
}

/******************************************************************************
 * @brief Kích hoạt tất cả các ngắt của CAN controller.
 * @param[in] Controller ID của controller (chỉ hỗ trợ CAN_1).
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 *****************************************************************************/
Std_ReturnType Can_EnableControllerInterrupts(uint8_t Controller){
    if(Controller != CAN_1) return E_NOT_OK;

    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
    CAN_ITConfig(CAN1, CAN_IT_FMP1, ENABLE);
    CAN_ITConfig(CAN1, CAN_IT_TME, ENABLE);
    CAN_ITConfig(CAN1, CAN_IT_WKU, ENABLE);
    CAN_ITConfig(CAN1, CAN_IT_ERR, ENABLE);
    CAN_ITConfig(CAN1, CAN_IT_SLK, ENABLE);
    
    return E_OK;
    
}

/******************************************************************************
 * @brief Kiểm tra xem có sự kiện wakeup nào xảy ra không.
 * @param[in] Controller ID của controller (chỉ hỗ trợ CAN_1).
 * @return Std_ReturnType E_OK nếu có sự kiện wakeup, E_NOT_OK nếu không.
 *****************************************************************************/
Std_ReturnType Can_CheckWakeup(uint8_t Controller){
    if(Controller != CAN_1) return E_NOT_OK;

    if(CAN_GetFlagStatus(CAN1, CAN_IT_WKU) == SET){
        return E_OK;
    }
    return E_NOT_OK;
}

/******************************************************************************
 * @brief Lấy trạng thái lỗi hiện tại của CAN controller.
 * @param[in] ControllerID ID của controller (chỉ hỗ trợ CAN_1).
 * @param[out] ErrorStatePtr Con trỏ để lưu trạng thái lỗi (Active, Passive, Bus-Off).
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 *****************************************************************************/
Std_ReturnType Can_GetControllerErrorState(uint8_t ControllerID, Can_ErrorStateType* ErrorStatePtr){
    if(ControllerID != CAN_1 || !ErrorStatePtr) return E_NOT_OK;

    uint32_t error = CAN1->ESR;
    if(error & CAN_ESR_BOFF)
    {
     *ErrorStatePtr = CAN_ERRORSTATE_BUSOFF;

    }else if(error & CAN_ESR_EPVF)
    {
     *ErrorStatePtr = CAN_ERRORSTATE_PASSIVE;
    }else
    {
     *ErrorStatePtr = CAN_ERRORSTATE_ACTIVE;
    }
    return E_OK;
}

/******************************************************************************
 * @brief Lấy trạng thái hoạt động hiện tại của CAN controller.
 * @param[in] Controller ID của controller (chỉ hỗ trợ CAN_1).
 * @param[out] ControllerModePtr Con trỏ để lưu trạng thái (Uninit, Started, Stopped, Sleep).
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 *****************************************************************************/
Std_ReturnType Can_GetControllerMode(uint8_t Controller, Can_ControllerStateType* ControllerModePtr){
    if(Controller != CAN_1 || !ControllerModePtr) return E_NOT_OK;

    uint32_t status = CAN1->MSR;
    
    if(status & CAN_MSR_INAK)
    {
        *ControllerModePtr = CAN_CS_STOPPED;
    }else if(status & CAN_MSR_SLAK)
    {
        *ControllerModePtr = CAN_CS_SLEEP;
    }else{
        *ControllerModePtr = CAN_CS_STARTED;
    }
    return E_OK;
}

/******************************************************************************
 * @brief Lấy giá trị bộ đếm lỗi nhận (Receive Error Counter - REC).
 * @param[in] Controller ID của controller (chỉ hỗ trợ CAN_1).
 * @param[out] RxErrorCounterPtr Con trỏ để lưu giá trị bộ đếm lỗi nhận.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 *****************************************************************************/
Std_ReturnType CAN_GetControllerRxErrorCounter(uint8_t Controller, uint8_t* RxErrorCounterPtr)
{
    if (Controller != CAN_1 || RxErrorCounterPtr == NULL)
    {
        return E_NOT_OK;
    }
    *RxErrorCounterPtr = CAN_GetReceiveErrorCounter(CAN1);
    return E_OK;
}

/******************************************************************************
 * @brief Lấy giá trị bộ đếm lỗi truyền (Transmit Error Counter - TEC).
 * @param[in] Controller ID của controller (chỉ hỗ trợ CAN_1).
 * @param[out] TxErrorCounterPtr Con trỏ để lưu giá trị bộ đếm lỗi truyền.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 *****************************************************************************/
Std_ReturnType CAN_GetControllerTxErrorCounter(uint8_t Controller, uint8_t* TxErrorCounterPtr)
{
    if (Controller != CAN_1 || TxErrorCounterPtr == NULL)
    {
        return E_NOT_OK;
    }
    *TxErrorCounterPtr = CAN_GetLSBTransmitErrorCounter(CAN1);
    return E_OK;
}   

/******************************************************************************
 * @brief Ghi một PDU (Protocol Data Unit) để truyền đi.
 * @param[in] Hth Handle của đối tượng phần cứng truyền (không dùng trong phiên bản này).
 * @param[in] PduInfo Con trỏ tới cấu trúc PDU chứa thông tin cần gửi.
 * @return Std_ReturnType E_OK nếu yêu cầu được chấp nhận, E_NOT_OK nếu lỗi, CAN_BUSY nếu không có mailbox rỗi.
 *****************************************************************************/
Std_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType* PduInfo){
    if(PduInfo == NULL) return E_NOT_OK;

    // Kiểm tra xem có ít nhất một mailbox truyền đang rỗi không
    if((CAN1->TSR & CAN_TSR_TME0) == 0 &&
       (CAN1->TSR & CAN_TSR_TME1) == 0 &&
       (CAN1->TSR & CAN_TSR_TME2) == 0
    ) return CAN_BUSY;

    // Chuẩn bị message để truyền đi theo cấu trúc của thư viện SPL
    CanTxMsg TxMessage;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.IDE = (PduInfo->id > 0x7FFu) ? CAN_ID_EXT : CAN_ID_STD;
    
    // Gán ID (Standard hoặc Extended)
    if(TxMessage.IDE == CAN_ID_STD){
        TxMessage.StdId = PduInfo->id;
    }else{
        TxMessage.ExtId = PduInfo->id;
    }

    TxMessage.DLC = PduInfo->length;
    for(uint8_t i = 0; i < PduInfo->length; i++){
        TxMessage.Data[i] = PduInfo->sdu[i]; // Sao chép dữ liệu
    }
    
    // Gọi hàm của SPL để truyền message
    mbx = CAN_Transmit(CAN1, &TxMessage);
    // CAN_Transmit trả về số mailbox (0, 1, 2) hoặc CAN_TxStatus_NoMailBox (4)
    if(mbx == CAN_TxStatus_NoMailBox) return CAN_BUSY;

    if(mbx < CAN_MAX_TX_MAILBOX){
        mbx_busy[mbx] = 1;
        swPduHandle[mbx] = PduInfo->swPduHandle;
    };
   
    while(CAN_TransmitStatus(CAN1, mbx) == CAN_TxStatus_Failed) ;

    return E_OK;   
}

