/**********************************************************************************************************************
 * @file    CanIf.c
 * @brief   Triển khai (implementation) cho module CAN Interface.
 * @details File này chứa logic hoạt động của các hàm API được định nghĩa trong CanIf.h.
 *          Nó quản lý trạng thái, định tuyến PDU, và là cầu nối giữa lớp PduR và CanDrv.
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 *********************************************************************************************************************/
#include "CanIf.h"
#include "CanIf_Cfg.h"
#include "PduR.h" 
#include <stdio.h>  // Dùng cho các hàm debug như printf (nếu có).
#include <string.h> // Dùng cho các hàm xử lý bộ nhớ như memcpy.
/* ================================================================================================================== */
/*                                          BIẾN TOÀN CỤC TĨNH (STATIC GLOBALS)                                       */
/* ================================================================================================================== */
// Các biến này lưu trữ trạng thái và cấu hình của module CanIf trong quá trình chạy.
// Việc sử dụng 'static' giới hạn phạm vi truy cập của chúng chỉ trong file này.

static CanIf_ControllerModeType ControllerMode[CANIF_MAX_CONTROLLERS];
static CanIf_PduModeType ControllerPduMode[CANIF_MAX_CONTROLLERS];
static CanIf_PduModeType TxPduMode[CANIF_MAX_TX_PDUS];
static CanIf_PduModeType RxPduMode[CANIF_MAX_RX_PDUS];
static CanIf_RxBufferType RxBuffer[CANIF_MAX_RX_PDUS];
static CanIf_NotifStatusType txNotifStatus[CANIF_MAX_TX_PDUS];
static CanIf_NotifStatusType rxNotifStatus[CANIF_MAX_RX_PDUS];
static CanIf_TxPduState TxPduState[CANIF_MAX_TX_PDUS];
CanIf_TxConfirmationStateType txConfirmationState[CANIF_MAX_TX_PDUS];

static uint8_t numControllers;
static uint8_t numTxPdus;
static uint8_t numRxPdus;
static uint8_t numRoutingEntries;

CanIf_TxConfirmationCallback txConfirmationCallback = NULL;
CanIf_RxIndicationCallback rxIndicationCallback = NULL;
extern CanIf_RoutingEntry RoutingTable[CANIF_NUM_TX_PDUS+CANIF_NUM_RX_PDUS];

/* ================================================================================================================== */
/*                                          TRIỂN KHAI CÁC HÀM API                                                     */
/* ================================================================================================================== */

/************************************************************
 * @brief Khởi tạo module CanIf với cấu hình được cung cấp.
 * @details Sao chép các thông tin cấu hình (chế độ, bảng định tuyến, callbacks) vào các biến toàn cục của module.
 * @param[in] config Con trỏ tới cấu trúc chứa toàn bộ thông tin cấu hình.
 *****************************************************************/
void CanIf_Init(const CanIf_ConfigType* config){
    // Kiểm tra an toàn: nếu con trỏ cấu hình là NULL thì không làm gì cả.
    if(config == NULL) return;
    // Sao chép các giá trị số lượng từ cấu hình vào biến cục bộ.
    numControllers = config->numControllers;
    numTxPdus = config->numTxPdus;
    numRxPdus = config->numRxPdus;
    numRoutingEntries = config->numRoutingEntries;

    // Sao chép toàn bộ nội dung của các mảng cấu hình.
    memcpy(ControllerMode, config->controllerMode, sizeof(ControllerMode));
    memcpy(TxPduMode, config->txPduMode, sizeof(TxPduMode));
    memcpy(RxPduMode, config->rxPduMode, sizeof(RxPduMode));
    // Lưu lại các con trỏ hàm callback.
    txConfirmationCallback = config->txConfirmationCallback;
    rxIndicationCallback = config->rxIndicationCallback;

    Can_RegisterRxCallback(&CanIf_RxIndication);
    // printf("CanIf_Init\n");
}

/**
 * @brief Hủy khởi tạo module CanIf, đưa về trạng thái ban đầu.
 * @details Reset tất cả các biến trạng thái và cấu hình về giá trị mặc định.
 */
void CanIf_DeInit(void){

    // LỖI LOGIC: Các biến đếm (numControllers, ...) được reset về 0 TRƯỚC khi
    // các vòng lặp for sử dụng chúng. Điều này khiến các vòng lặp không bao giờ chạy.
    // Nên di chuyển việc reset các biến đếm xuống cuối hàm.
    for(int i=0; i< numControllers; i++){
        ControllerMode[i] = CANIF_CONTROLLER_STOPPED;
    }
    
    for(int i=0; i< numTxPdus; i++){
        TxPduMode[i] = CANIF_OFFLINE;
    }

    for(int i=0; i< numRxPdus; i++){
        RxPduMode[i] = CANIF_OFFLINE;
    }   

    for(int i=0; i< numRoutingEntries; i++){
        RoutingTable[i].id = 0;
        RoutingTable[i].CanId = 0;
        RoutingTable[i].isTX = 0;
    }

    // Reset các biến đếm về 0.
    numControllers = 0;
    numTxPdus = 0;
    numRxPdus = 0;
    numRoutingEntries = 0;

    // Reset các con trỏ callback.
    txConfirmationCallback = NULL;
    rxIndicationCallback = NULL;
}

/**
 * @brief Thiết lập chế độ hoạt động cho một CAN controller.
 * @param[in] ControllerId ID của controller.
 * @param[in] mode Chế độ mong muốn (STARTED, STOPPED, SLEEP).
 * @return E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_SetControllerMode(uint8_t ControllerId, CanIf_ControllerModeType mode){
    // Kiểm tra ID controller có hợp lệ không.
    if(ControllerId >= numControllers) return E_NOT_OK;
    
    // Gọi hàm của lớp CanDrv để thực sự thay đổi chế độ của phần cứng.
    if(!Can_SetControllerMode(ControllerId, mode)){
        return E_NOT_OK;
    }
    // Cập nhật lại trạng thái trong mảng của CanIf.
    ControllerMode[ControllerId] = mode;
    return E_OK;
}

/**
 * @brief Lấy chế độ hoạt động hiện tại của một CAN controller.
 * @param[in] ControllerId ID của controller.
 * @param[out] mode Con trỏ để lưu trữ chế độ đọc được.
 * @return E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_GetControllerMode(uint8_t ControllerId, CanIf_ControllerModeType* mode){
    // Kiểm tra tham số đầu vào.
    if(ControllerId >= numControllers|| mode == NULL) return E_NOT_OK;

    // Trả về giá trị đã lưu trong mảng trạng thái của CanIf.
    *mode = ControllerMode[ControllerId];
    return E_OK;
}
/**
 * @brief Lấy trạng thái lỗi của một CAN controller.
 * @param[in] ControllerId ID của controller.
 * @param[out] ErrorStatePtr Con trỏ để lưu trạng thái lỗi (ACTIVE, PASSIVE, BUSOFF).
 * @return E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_GetControllerErrorState(uint8_t ControllerId, Can_ErrorStateType* ErrorStatePtr){
    // Kiểm tra tham số đầu vào.
    if(ControllerId >= numControllers|| ErrorStatePtr == NULL) return E_NOT_OK;

    // Gọi hàm của CanDrv để đọc trực tiếp từ phần cứng.
    if(!Can_GetControllerErrorState(ControllerId, ErrorStatePtr)){
        return E_NOT_OK;
    }
    return E_OK;
}

/**
 * @brief Yêu cầu truyền một PDU.
 * @details Tìm CAN ID tương ứng, sau đó đóng gói và gọi Can_Write() để gửi.
 * @param[in] TxPduId ID của PDU cần truyền.
 * @param[in] PduInfo Con trỏ tới cấu trúc chứa dữ liệu và độ dài.
 * @return E_OK nếu yêu cầu được chấp nhận, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_Transmit(PduIdType TxPduId, const PduInfoType* PduInfo){
    // Kiểm tra các tham số đầu vào có hợp lệ không.
    if(TxPduId >= numTxPdus || PduInfo == NULL || PduInfo->SduDataPtr == NULL) 
        return E_NOT_OK;

    // Nếu PDU đang ở chế độ OFFLINE, không cho phép truyền.
    if(TxPduMode[TxPduId] == CANIF_OFFLINE) 
        return E_NOT_OK;

    Can_IdType CanId = 0;
    Can_HwHandleType Hth = 0;
    // Tìm kiếm CAN ID tương ứng với TxPduId trong bảng định tuyến.
    for(int i=0; i< numRoutingEntries; i++){

        if(RoutingTable[i].id == TxPduId && RoutingTable[i].isTX == 1){
            CanId = RoutingTable[i].CanId;
            Hth = RoutingTable[i].Hth;
            break; // Thoát khỏi vòngh lặp ngay khi tìm thấy.
        }
    }
    // Nếu không tìm thấy CanId hợp lệ, trả về lỗi.
    if(CanId == 0xFFFFFFFF) return E_NOT_OK;

    // Chuẩn bị cấu trúc Can_PduType để truyền xuống cho CanDrv.
    Can_PduType frame;
    frame.id = CanId;
    frame.length = PduInfo->SduLength;
    frame.sdu = PduInfo->SduDataPtr;
    // swPduHandle dùng để CanIf nhận dạng lại PDU khi có callback từ CanDrv.
    frame.swPduHandle = TxPduId;

    // Gọi hàm của CanDrv để ghi PDU vào hàng đợi truyền của phần cứng.
     if(Can_Write(Hth, &frame) != E_OK) return E_NOT_OK;
    
    return E_OK;
}

/**
 * @brief Đọc dữ liệu từ một Rx PDU đã được nhận và lưu trong buffer.
 * @param[in] RxPduId ID của Rx PDU.
 * @param[out] PduInfo Con trỏ tới cấu trúc để lưu dữ liệu và độ dài đọc được.
 * @return E_OK nếu đọc thành công, E_NOT_OK nếu không có dữ liệu mới.
 */
Std_ReturnType CanIf_ReadRxPduData(PduIdType RxPduId, PduInfoType* PduInfo){
    // Kiểm tra tham số đầu vào.
    if(RxPduId >= numRxPdus || PduInfo == NULL || PduInfo->SduDataPtr == NULL)
        return E_NOT_OK;
    
    // Lấy con trỏ tới buffer của Rx PDU tương ứng.
    CanIf_RxBufferType* buf = &RxBuffer[RxPduId];
    // Nếu không có dữ liệu mới (cờ hasData=0) hoặc dữ liệu rỗng, trả về lỗi.
    if(!buf->hasData||buf->length ==0) return E_NOT_OK;

    // Sao chép đúng số byte dữ liệu đã nhận được từ buffer nội bộ sang con trỏ của lớp trên.
    memcpy(PduInfo->SduDataPtr, buf->buffer, buf->length);
    // Cập nhật lại độ dài dữ liệu cho lớp trên biết.
    PduInfo->SduLength = buf->length;

    // Đánh dấu là đã đọc dữ liệu để tránh đọc lại dữ liệu cũ.
    buf->hasData = 0;

    return E_OK;
}
/**
 * @brief Đọc và xóa trạng thái thông báo của một Tx PDU.
 * @details Hàm này được lớp trên dùng để polling xem PDU đã được gửi xong chưa.
 * @param[in] PduId ID của Tx PDU.
 * @return Trạng thái thông báo (có hoặc không).
 */
CanIf_NotifStatusType CanIf_ReadTxNotifStatus(PduIdType PduId){
    if(PduId >= numTxPdus) return CANIF_NO_NOTIFICATION;
    
    // Đọc trạng thái hiện tại.
    CanIf_NotifStatusType status = txNotifStatus[PduId];
    // Reset trạng thái sau khi đọc (cơ chế "read-and-clear").
    txNotifStatus[PduId] = CANIF_NO_NOTIFICATION;
    return status;
}

/**
 * @brief Đọc và xóa trạng thái thông báo của một Rx PDU.
 * @details Hàm này được lớp trên dùng để polling xem có PDU mới nhận được không.
 * @param[in] PduId ID của Rx PDU.
 * @return Trạng thái thông báo (có hoặc không).
 */
CanIf_NotifStatusType CanIf_ReadRxNotifStatus(PduIdType PduId){
    if(PduId >= numRxPdus) return CANIF_NO_NOTIFICATION;
    
    CanIf_NotifStatusType status = rxNotifStatus[PduId];
    // Reset trạng thái sau khi đọc.
    rxNotifStatus[PduId] = CANIF_NO_NOTIFICATION;
    return status;
}
/**
 * @brief Thiết lập chế độ hoạt động cho tất cả các PDU của một controller.
 * @param[in] ControllerID ID của controller.
 * @param[in] modeRequest Chế độ mong muốn.
 * @return E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_SetPduMode(PduIdType ControllerID, CanIf_PduModeType modeRequest){
    if(ControllerID >= CANIF_MAX_CONTROLLERS) return E_NOT_OK;
    ControllerPduMode[ControllerID] = modeRequest;
    return E_OK;
}

/**
 * @brief Lấy chế độ hoạt động PDU chung của một controller.
 * @param[in] ControllerID ID của controller.
 * @param[out] modePtr Con trỏ để lưu chế độ hiện tại.
 * @return E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_GetPduMode(PduIdType ControllerID, CanIf_PduModeType* modePtr){
    if(ControllerID >= CANIF_MAX_CONTROLLERS || modePtr == NULL) return E_NOT_OK;
    *modePtr = ControllerPduMode[ControllerID];
    return E_OK;
}

/**
 * @brief Lấy thông tin phiên bản của module CanIf.
 * @param[out] versioninfo Con trỏ để lưu thông tin phiên bản.
 * @return Không trả về giá trị (void). Cần sửa lại kiểu trả về.
 */
Std_VersionInfoType CanIf_GetVersionInfo(Std_VersionInfoType* versioninfo){

    versioninfo->vendorID = CANIF_VENDOR_ID;
    versioninfo->moduleID = CANIF_MODULE_ID;
    versioninfo->sw_major_version = CANIF_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = CANIF_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = CANIF_SW_PATCH_VERSION;
    return *versioninfo;
}

/**
 * @brief Thiết lập CAN ID động cho một Tx PDU.
 * @details Cho phép thay đổi CAN ID của một PDU trong lúc chạy, hữu ích cho chẩn đoán (UDS).
 * @param[in] CanTxPduId ID của Tx PDU.
 * @param[in] CanId CAN ID mới cần thiết lập.
 * @return E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_SetDynamicTxId(PduIdType CanTxPduId, Can_IdType CanId){
    // Kiểm tra PDU ID có hợp lệ và PDU có được cấu hình là dynamic không.
    if(CanTxPduId >= numTxPdus && !TxPduState[CanTxPduId].is_Dynamic) return E_NOT_OK;

    // Kiểm tra giá trị CanId có nằm trong dải extended ID (29 bit) không.
    if(CanId > 0x1FFFFFFF) return E_NOT_OK;
    
    // Cập nhật CAN ID mới vào mảng trạng thái.
    TxPduState[CanTxPduId].cur_CanID = CanId;
    return E_OK;
}

/**
 * @brief Lấy trạng thái xác nhận truyền của một Tx PDU.
 * @details Hàm này cho phép lớp trên kiểm tra xem một yêu cầu truyền đã được xác nhận hay chưa.
 * @param[in] CanIfTxSduId ID của Tx PDU cần kiểm tra.
 * @param[out] TxConfirmationStatePtr Con trỏ để lưu trạng thái xác nhận đọc được.
 * @return E_OK nếu thành công, E_NOT_OK nếu ID hoặc con trỏ không hợp lệ.
 */
Std_ReturnType CanIf_GetTxConfirmationState(PduIdType CanIfTxSduId, CanIf_TxConfirmationStateType *TxConfirmationStatePtr) {
    if (CanIfTxSduId >= CANIF_MAX_TX_PDUS || TxConfirmationStatePtr == NULL)
        return E_NOT_OK;

    *TxConfirmationStatePtr = txConfirmationState[CanIfTxSduId];
    return E_OK;
}

/**
 * @brief Thiết lập baudrate cho một CAN controller.
 * @details Hàm này chỉ có thể được gọi khi controller đang ở trạng thái STOPPED.
 *          Nó gọi xuống hàm tương ứng của lớp CanDrv để thay đổi cấu hình phần cứng.
 * @param[in] ControllerId ID của controller cần thay đổi baudrate.
 * @param[in] BaudRateConfigID ID của cấu hình baudrate mong muốn (ví dụ: 125, 250, 500 kbps).
 * @return E_OK nếu thành công, E_NOT_OK nếu controller không ở trạng thái STOPPED hoặc có lỗi khác.
 */
Std_ReturnType CanIf_SetBaudrate(uint8_t ControllerId, uint16_t BaudRateConfigID) {
    if (ControllerId >= CANIF_MAX_CONTROLLERS)
        return E_NOT_OK;

    if (ControllerMode[ControllerId] != CANIF_CONTROLLER_STOPPED)
        return E_NOT_OK;

    if (Can_SetBaudrate(ControllerId, BaudRateConfigID) != E_OK)
        return E_NOT_OK;

    return E_OK;
}

