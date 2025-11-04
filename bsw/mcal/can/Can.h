/**********************************************************************************************************************
 * @file    Can.h
 * @brief   Header file cho CAN Driver.
 * @details File này định nghĩa các giao diện (API), kiểu dữ liệu và cấu trúc cấu hình
 *          cho module CAN Driver theo phong cách AUTOSAR. Nó cung cấp một lớp trừu tượng
 *          để các module lớp trên (ví dụ: CanIf) có thể tương tác với phần cứng CAN
 *          mà không cần biết chi tiết về vi điều khiển cụ thể.
 *
 *          Các chức năng chính bao gồm:
 *          - Khởi tạo và hủy khởi tạo CAN controller.
 *          - Gửi và nhận các gói tin CAN (PDU).
 *          - Quản lý trạng thái của CAN controller (Start, Stop, Sleep).
 *          - Đọc trạng thái lỗi và các bộ đếm lỗi của controller.
 *          - Quản lý ngắt.
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 *********************************************************************************************************************/
#ifndef CAN_H
#define CAN_H

#include "Std_Types.h"
#include "Can_GeneralTypes.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_can.h"

#define CAN_1 1
/**********************************************************
 * Macro định nghĩa phiên bản, vendor, module ID cho VersionInfo
 **********************************************************/
#define CAN_VENDOR_ID 1234U
#define CAN_MODULE_ID 81U
#define CAN_SW_MAJOR_VERSION 1U
#define CAN_SW_MINOR_VERSION 0U
#define CAN_SW_PATCH_VERSION 0U
/********************************************************************************
 * @struct Can_ConfigType
 * @brief  Cấu trúc chứa các thông số cấu hình cho một CAN controller.
 * @details Cấu trúc này được truyền vào hàm `Can_Init` để thiết lập các thông số
 *          vận hành cho phần cứng CAN, bao gồm cả timing, các tính năng phụ trợ 
 *          và bộ lọc.
 *******************************************************************************/
typedef struct {
    
    struct{
    /**
     * @brief Cấu hình cơ bản liên quan đến bit timing.
     */
        uint16_t CAN_Prescaler; /*!< Bộ chia tần để tạo ra time quantum. Giá trị từ 1 đến 1024. */
        uint8_t CAN_Mode;       /*!< Chế độ hoạt động: Normal, Loopback, Silent, Silent-Loopback. */
        uint8_t CAN_SJW;        /*!< Synchronization Jump Width: Số time quanta tối đa để bù trừ. */
        uint8_t CAN_BS1;        /*!< Bit Segment 1: Số time quanta trong segment 1. */
        uint8_t CAN_BS2;        /*!< Bit Segment 2: Số time quanta trong segment 2. */
    /****************************************************************************
     * @brief Cấu hình các tính năng nâng cao của CAN controller.
     ****************************************************************************/
  
        FunctionalState CAN_TTCM; /*!< Time Triggered Communication Mode: Bật/tắt chế độ giao tiếp theo thời gian. */
        FunctionalState CAN_ABOM; /*!< Automatic Bus-Off Management: Bật/tắt tự động phục hồi từ trạng thái Bus-Off. */
        FunctionalState CAN_AWUM; /*!< Automatic Wakeup Mode: Bật/tắt chế độ tự động đánh thức. */
        FunctionalState CAN_NART; /*!< No Automatic Retransmission: Bật/tắt chế độ không tự động truyền lại khi có lỗi. */
        FunctionalState CAN_RFLM; /*!< Receive FIFO Locked Mode: Bật/tắt chế độ khóa FIFO khi đầy. */
        FunctionalState CAN_TXFP; /*!< Transmit FIFO Priority: Bật/tắt ưu tiên truyền theo thứ tự yêu cầu (thay vì theo ID). */
    }Basic_Config;

    /****************************************************************************
     * @brief Cấu hình bộ lọc tin nhắn CAN.
     ****************************************************************************/
    struct 
    {
        uint16_t Can_FilterIdHigh;         /*!< Thanh ghi ID bộ lọc (phần cao). */
        uint16_t Can_FilterIdLow;          /*!< Thanh ghi ID bộ lọc (phần thấp). */
        uint16_t Can_FilterMaskIdHigh;     /*!< Thanh ghi mặt nạ hoặc ID thứ hai (phần cao). */
        uint16_t Can_FilterMaskIdLow;      /*!< Thanh ghi mặt nạ hoặc ID thứ hai (phần thấp). */
        uint16_t Can_FilterFIFOAssignment; /*!< Chỉ định FIFO (0 hoặc 1) cho bộ lọc. */
        uint8_t Can_FilterNumber;          /*!< Số thứ tự của bộ lọc (0-13). */
        uint8_t Can_FilterMode;            /*!< Chế độ bộ lọc: Mask (CAN_FilterMode_IdMask) hoặc List (CAN_FilterMode_IdList). */
        uint8_t Can_FilterScale;           /*!< Kích thước bộ lọc: 16-bit (CAN_FilterScale_16bit) hoặc 32-bit (CAN_FilterScale_32bit). */
        uint8_t Can_FilterActivation;      /*!< Trạng thái kích hoạt bộ lọc (ENABLE hoặc DISABLE). */
    } Filter_Config;
    uint8_t NotificationEnable;            /*!< Bật/Tắt ngắt CAN */
}Can_ConfigType;

/*******************************************************************************
 * @brief Khởi tạo CAN driver.
 * @details Hàm này khởi tạo tất cả các CAN controller được định nghĩa trong cấu hình.
 * @param[in] config Con trỏ tới cấu trúc cấu hình của CAN driver.
 * @return void
 *******************************************************************************/
void Can_Init(const Can_ConfigType* config);

/*******************************************************************************
 * @brief Lấy thông tin phiên bản của module.
 * @param[out] versioninfo Con trỏ để lưu thông tin phiên bản.
 ******************************************************************************/
void Can_GetVersionInfo(Std_VersionInfoType* versioninfo);

/******************************************************************************
 * @brief Hủy khởi tạo CAN driver.
 * @details Đưa tất cả các CAN controller về trạng thái chưa khởi tạo.
 ******************************************************************************/
void Can_DeInit(void);

/******************************************************************************
 * @brief Thiết lập baudrate cho một CAN controller.
 * @param[in] Controller ID của controller.
 * @param[in] BaudRateConfigID ID của cấu hình baudrate.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 ******************************************************************************/
Std_ReturnType Can_SetBaudrate(uint8_t Controller, uint16_t BaudRateConfigID);

/******************************************************************************
 * @brief Chuyển đổi trạng thái của một CAN controller.
 * @param[in] Controller ID của controller.
 * @param[in] Transition Trạng thái mong muốn (CAN_CS_STARTED, CAN_CS_STOPPED, CAN_CS_SLEEP).
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 ******************************************************************************/
Std_ReturnType Can_SetControllerMode(uint8_t Controller, Can_ControllerStateType Transition);

/******************************************************************************
 * @brief Vô hiệu hóa ngắt của một CAN controller.
 * @param[in] Controller ID của controller.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 ******************************************************************************/
Std_ReturnType Can_DisableControllerInterrupts(uint8_t Controller);

/******************************************************************************
 * @brief Kích hoạt ngắt của một CAN controller.
 * @param[in] Controller ID của controller.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 ******************************************************************************/
Std_ReturnType Can_EnableControllerInterrupts(uint8_t Controller);

/******************************************************************************
 * @brief Kiểm tra xem có sự kiện wakeup nào xảy ra không.
 * @param[in] Controller ID của controller.
 * @return Std_ReturnType E_OK nếu có sự kiện wakeup, E_NOT_OK nếu không.
 ******************************************************************************/
Std_ReturnType Can_CheckWakeup(uint8_t Controller);

/******************************************************************************
 * @brief Lấy trạng thái lỗi hiện tại của CAN controller (Active, Passive, Bus-Off).
 * @param[in] ControllerID ID của controller.
 * @param[out] ErrorStatePtr Con trỏ để lưu trạng thái lỗi.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 ******************************************************************************/
Std_ReturnType Can_GetControllerErrorState(uint8_t ControllerID, Can_ErrorStateType* ErrorStatePtr);

/******************************************************************************
 * @brief Lấy trạng thái hoạt động hiện tại của CAN controller (Uninit, Started, Stopped, Sleep).
 * @param[in] Controller ID của controller.
 * @param[out] ControllerModePtr Con trỏ để lưu trạng thái hoạt động.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 *****************************************************************************/
Std_ReturnType Can_GetControllerMode(uint8_t Controller, Can_ControllerStateType* ControllerModePtr);

/******************************************************************************
 * @brief Lấy giá trị bộ đếm lỗi nhận (Receive Error Counter - REC).
 * @param[in] Controller ID của controller.
 * @param[out] ErrorCounterPtr Con trỏ để lưu giá trị bộ đếm lỗi nhận.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 *****************************************************************************/
Std_ReturnType CAN_GetControllerRxErrorCounter(uint8_t Controller, uint8_t* RxErrorCounterPtr);

/******************************************************************************
 * @brief Lấy giá trị bộ đếm lỗi truyền (Transmit Error Counter - TEC).
 * @param[in] Controller ID của controller.
 * @param[out] ErrorCounterPtr Con trỏ để lưu giá trị bộ đếm lỗi truyền.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 *****************************************************************************/
Std_ReturnType CAN_GetControllerTxErrorCounter(uint8_t Controller, uint8_t* TxErrorCounterPtr);
/******************************************************************************
 * @brief Ghi một PDU (Protocol Data Unit) để truyền đi.
 * @param[in] Hth Handle của đối tượng phần cứng truyền (Hardware Transmit Handle).
 * @param[in] PduInfo Con trỏ tới cấu trúc PDU chứa thông tin (ID, data, length) cần gửi.
 * @return Std_ReturnType E_OK nếu yêu cầu được chấp nhận, E_NOT_OK nếu Hth không hợp lệ, CAN_BUSY nếu mailbox đang bận.
 *****************************************************************************/
Std_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType* PduInfo);

#endif /* CAN_H */
