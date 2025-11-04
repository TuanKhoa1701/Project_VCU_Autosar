/**********************************************************************************************************************
 * @file    CanIf.h
 * @brief   Header file cho Module CAN Interface (CanIf).
 * @details File này định nghĩa các giao diện (API), kiểu dữ liệu và cấu trúc cấu hình
 *          cho module CanIf theo kiến trúc AUTOSAR. CanIf là một lớp trừu tượng nằm giữa
 *          lớp PDU Router (PduR) và lớp CAN Driver (CanDrv).
 *
 *          Chức năng chính của CanIf:
 *          - Trừu tượng hóa các CAN controller, cho phép lớp trên giao tiếp với bus CAN
 *            mà không cần biết chi tiết về CAN controller vật lý nào đang được sử dụng.
 *          - Định tuyến các PDU (Protocol Data Unit) từ lớp trên (PduR) đến CAN controller
 *            tương ứng để truyền đi (Tx) và ngược lại, từ CAN Driver lên PduR khi nhận được (Rx).
 *          - Quản lý trạng thái của các CAN controller (START, STOP, SLEEP).
 *          - Quản lý chế độ hoạt động của các PDU (ONLINE, OFFLINE).
 *          - Cung cấp cơ chế thông báo (notification) cho lớp trên khi hoàn thành truyền (TxConfirmation)
 *            hoặc khi nhận được dữ liệu mới (RxIndication).
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 *********************************************************************************************************************/
#ifndef CANIF_H
#define CANIF_H

#include "Std_Types.h"
#include "Can.h"
#include "Can_Cfg.h"
/**
 * @defgroup CANIF_VERSION_INFO Thông tin phiên bản của module CanIf
 * @{
 */
#define CANIF_VENDOR_ID        0x123   
#define CANIF_MODULE_ID        0x45    
#define CANIF_SW_MAJOR_VERSION 1
#define CANIF_SW_MINOR_VERSION 0
#define CANIF_SW_PATCH_VERSION 2
/** @} */

/**
 * @defgroup CANIF_CONFIG_LIMITS Giới hạn cấu hình cho module CanIf
 * @brief Các giá trị này xác định kích thước của các mảng cấu hình tĩnh,
 *        giúp cấp phát bộ nhớ tại thời điểm biên dịch.
 * @{
 */
#define CANIF_MAX_CONTROLLERS  2U
#define CANIF_MAX_TX_PDUS      4
#define CANIF_MAX_RX_PDUS      4
#define CANIF_MAX_RX_BUF_SIZE  8
/** @} */

/**
 * @enum  CanIf_PduModeType
 * @brief Định nghĩa các chế độ hoạt động cho một PDU (truyền hoặc nhận).
 */
typedef enum{
    CANIF_OFFLINE,      /**< PDU không hoạt động (cả Tx và Rx đều bị vô hiệu hóa). */
    CANIF_TX_OFFLINE,   /**< Chỉ hướng truyền (Tx) của PDU bị vô hiệu hóa. */
    CANIF_RX_OFFLINE,   /**< Chỉ hướng nhận (Rx) của PDU bị vô hiệu hóa. */
    CANIF_ONLINE,       /**< PDU hoạt động đầy đủ (cả Tx và Rx đều được cho phép). */
    CANIF_TX_ONLINE,    /**< Chỉ hướng truyền (Tx) của PDU được cho phép. */
    CANIF_RX_ONLINE     /**< Chỉ hướng nhận (Rx) của PDU được cho phép. */
} CanIf_PduModeType;

/**
 * @enum  CanIf_ControllerModeType
 * @brief Định nghĩa các trạng thái hoạt động của một CAN controller.
 */
typedef enum{
    CANIF_CONTROLLER_STOPPED,   /**< Controller đã dừng. */
    CANIF_CONTROLLER_STARTED,   /**< Controller đang hoạt động. */
    CANIF_CONTROLLER_SLEEP,     /**< Controller đang ở chế độ ngủ. */
    CANIF_CONTROLLER_WAKEUP     /**< Controller đang trong quá trình thức dậy. */
} CanIf_ControllerModeType;

/**
 * @enum  CanIf_NotifStatusType
 * @brief Định nghĩa trạng thái thông báo (notification) cho một PDU.
 */
typedef enum{
    CANIF_NO_NOTIFICATION,      /**< Không có thông báo nào đang chờ xử lý. */
    CANIF_TX_RX_NOTIFICATION     /**< Có thông báo đang chờ xử lý (Tx hoặc Rx). */
} CanIf_NotifStatusType;

/**
 * @struct CanIf_TxPduState
 * @brief  Lưu trữ trạng thái động của một PDU truyền (Tx PDU).
 * @details Dùng để hỗ trợ các PDU có thể thay đổi CAN ID trong lúc chạy (dynamic ID).
 */
typedef struct{
    uint8_t is_Dynamic;     /**< Cờ cho biết PDU này có phải là dynamic ID hay không. */
    Can_IdType cur_CanID;   /**< CAN ID hiện tại của PDU. */
} CanIf_TxPduState;

/**
 * @enum  CanIf_TxConfirmationStateType
 * @brief Trạng thái xác nhận truyền của một Tx PDU.
 */
typedef enum {
    CANIF_TX_CONF_PENDING = 0,      /**< Đang chờ xác nhận từ CAN Driver. */
    CANIF_TX_CONF_NOT_PENDING = 1   /**< Không có yêu cầu truyền nào đang chờ xác nhận. */
} CanIf_TxConfirmationStateType;

/**
 * @struct CanIf_RxBufferType
 * @brief  Bộ đệm đơn giản để lưu dữ liệu của một PDU nhận (Rx PDU).
 */
typedef struct{
    uint8_t buffer[CANIF_MAX_RX_BUF_SIZE]; /**< Vùng nhớ chứa dữ liệu. */
    uint8_t length;                        /**< Độ dài dữ liệu (DLC). */
    uint8_t hasData;                       /**< Cờ báo hiệu có dữ liệu mới hay không. */
} CanIf_RxBufferType;

/**********************************************************************
 * @brief Con trỏ hàm cho callback xác nhận truyền (Tx Confirmation).
 * @details CanIf sẽ gọi hàm này để thông báo cho lớp trên (PduR) khi một PDU đã được truyền thành công.
 * @param TxPduId ID của PDU đã được truyền.
 **********************************************************************/
typedef void (*CanIf_TxConfirmationCallback)(PduIdType TxPduId);

/**
 * @brief Con trỏ hàm cho callback chỉ báo nhận (Rx Indication).
 * @details CanIf sẽ gọi hàm này để thông báo cho lớp trên (PduR) khi một PDU mới được nhận.
 * @param PduId ID của Pdu nhận được.
 * @param data Con trỏ tới gói tin của PDU.
 */
typedef void (*CanIf_RxIndicationCallback)(PduIdType PduId, const PduInfoType* data);

/**
 * @struct CanIf_RoutingEntry
 * @brief  Một mục trong bảng định tuyến, ánh xạ một PDU ID tới một CAN ID và hướng.
 */
typedef struct{
    PduIdType id;           /**< PDU ID do lớp trên định nghĩa (ví dụ: PduR). */
    Can_IdType CanId;       /**< CAN ID tương ứng trên bus. */
    uint8_t isTX;           /**< Cờ xác định hướng: 1 cho Tx, 0 cho Rx. */
    Can_HwHandleType Hth;    /**< MailBox cho Tx*/
} CanIf_RoutingEntry;

/**
 * @struct CanIf_ConfigType
 * @brief  Cấu trúc cấu hình chính cho module CanIf.
 * @details Cấu trúc này chứa tất cả các thông tin cần thiết để khởi tạo CanIf,
 *          bao gồm cấu hình controller, PDU, bảng định tuyến và các hàm callback.
 */
typedef struct {
    uint8_t numControllers;                                                 /**< Số lượng CAN controller được quản lý. */
    CanIf_ControllerModeType controllerMode[CANIF_MAX_CONTROLLERS];         /**< Mảng trạng thái ban đầu của các controller. */
    uint8_t numTxPdus;                                                      /**< Số lượng Tx PDU được cấu hình. */
    CanIf_PduModeType txPduMode[CANIF_MAX_TX_PDUS];                         /**< Mảng chế độ ban đầu của các Tx PDU. */
    uint8_t numRxPdus;                                                      /**< Số lượng Rx PDU được cấu hình. */
    CanIf_PduModeType rxPduMode[CANIF_MAX_RX_PDUS];                         /**< Mảng chế độ ban đầu của các Rx PDU. */
    uint8_t numRoutingEntries;                                              /**< Số lượng mục trong bảng định tuyến. */
    CanIf_RoutingEntry* routingTable;                                       /**< Con trỏ tới bảng định tuyến PDU. */
    CanIf_TxConfirmationCallback txConfirmationCallback;                    /**< Con trỏ hàm callback cho Tx Confirmation. */
    CanIf_RxIndicationCallback rxIndicationCallback;                        /**< Con trỏ hàm callback cho Rx Indication. */
} CanIf_ConfigType;

/**
 * @brief Khởi tạo module CanIf.
 * @param[in] config Con trỏ tới cấu trúc cấu hình.
 */
void CanIf_Init(const CanIf_ConfigType* config);

/**
 * @brief Hủy khởi tạo module CanIf.
 */
void CanIf_DeInit(void);

/**
 * @brief Thiết lập chế độ hoạt động cho một CAN controller.
 * @param[in] ControllerId ID của controller.
 * @param[in] mode Chế độ mong muốn (STARTED, STOPPED, SLEEP).
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_SetControllerMode(uint8_t ControllerId, CanIf_ControllerModeType mode);

/**
 * @brief Lấy chế độ hoạt động hiện tại của một CAN controller.
 * @param[in] ControllerId ID của controller.
 * @param[out] mode Con trỏ để lưu chế độ hiện tại.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_GetControllerMode(uint8_t ControllerId, CanIf_ControllerModeType* mode);

/**
 * @brief Lấy trạng thái lỗi của một CAN controller.
 * @param[in] ControllerId ID của controller.
 * @param[out] ErrorStatePtr Con trỏ để lưu trạng thái lỗi (ACTIVE, PASSIVE, BUSOFF).
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_GetControllerErrorState(uint8_t ControllerId, Can_ErrorStateType* ErrorStatePtr);

/**
 * @brief Yêu cầu truyền một PDU.
 * @param[in] TxPduId ID của PDU cần truyền.
 * @param[in] PduInfo Con trỏ tới cấu trúc chứa dữ liệu và độ dài của PDU.
 * @return Std_ReturnType E_OK nếu yêu cầu được chấp nhận, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_Transmit(PduIdType TxPduId, const PduInfoType* PduInfo);

/**
 * @brief Đọc dữ liệu từ một Rx PDU đã được nhận.
 * @param[in] RxPduId ID của Rx PDU.
 * @param[out] PduInfo Con trỏ tới cấu trúc để lưu dữ liệu và độ dài.
 * @return Std_ReturnType E_OK nếu đọc thành công, E_NOT_OK nếu không có dữ liệu mới hoặc lỗi.
 */
Std_ReturnType CanIf_ReadRxPduData(PduIdType RxPduId, PduInfoType* PduInfo); 

/**
 * @brief Đọc trạng thái thông báo của một Tx PDU.
 * @param[in] PduId ID của Tx PDU.
 * @return CanIf_NotifStatusType Trạng thái thông báo.
 */
CanIf_NotifStatusType CanIf_ReadTxNotifStatus(PduIdType PduId);

/**
 * @brief Đọc trạng thái thông báo của một Rx PDU.
 * @param[in] PduId ID của Rx PDU.
 * @return CanIf_NotifStatusType Trạng thái thông báo.
 */
CanIf_NotifStatusType CanIf_ReadRxNotifStatus(PduIdType PduId);

/**
 * @brief Thiết lập chế độ hoạt động cho một PDU.
 * @param[in] ControllerID ID của PDU controller.
 * @param[in] modeRequest Chế độ mong muốn (ONLINE, OFFLINE, ...).
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_SetPduMode(PduIdType ControllerID, CanIf_PduModeType modeRequest);

/**
 * @brief Lấy chế độ hoạt động hiện tại của một PDU.
 * @param[in] ControllerID ID của PDU controller.
 * @param[out] modePtr Con trỏ để lưu chế độ hiện tại.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_GetPduMode(PduIdType ControllerID, CanIf_PduModeType* modePtr);

/**
 * @brief Lấy thông tin phiên bản của module CanIf.
 * @param[out] versioninfo Con trỏ để lưu thông tin phiên bản.
 * @return Std_VersionInfoType Thông tin phiên bản.
 */
Std_VersionInfoType CanIf_GetVersionInfo(Std_VersionInfoType* versioninfo);

/**
 * @brief Thiết lập CAN ID động cho một Tx PDU.
 * @param[in] CanTxPduId ID của Tx PDU.
 * @param[in] CanId CAN ID mới cần thiết lập.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu PDU không hỗ trợ ID động.
 */
Std_ReturnType CanIf_SetDynamicTxId(PduIdType CanTxPduId, Can_IdType CanId);

/**
 * @brief Lấy trạng thái xác nhận truyền của một Tx PDU.
 * @param[in] CanIfTxSduId ID của Tx PDU.
 * @param[out] TxConfirmationStatePtr Con trỏ để lưu trạng thái xác nhận.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_GetTxConfirmationState(PduIdType CanIfTxSduId, CanIf_TxConfirmationStateType *TxConfirmationStatePtr);

/**
 * @brief Thiết lập baudrate cho một CAN controller.
 * @param[in] ControllerId ID của controller.
 * @param[in] BaudRateConfigID ID của cấu hình baudrate.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_SetBaudrate(uint8_t ControllerId, uint16_t BaudRateConfigID);

/**
 * @brief Lấy giá trị bộ đếm lỗi nhận (REC) của controller.
 * @param[in] ControllerId ID của controller.
 * @param[out] RxErrorCounterPtr Con trỏ để lưu giá trị REC.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_GetControllerRxErrorCounter(uint8_t ControllerId, uint8_t *RxErrorCounterPtr);

/**
 * @brief Lấy giá trị bộ đếm lỗi truyền (TEC) của controller.
 * @param[in] ControllerId ID của controller.
 * @param[out] TxErrorCounterPtr Con trỏ để lưu giá trị TEC.
 * @return Std_ReturnType E_OK nếu thành công, E_NOT_OK nếu thất bại.
 */
Std_ReturnType CanIf_GetControllerTxErrorCounter(uint8_t ControllerId, uint8_t *TxErrorCounterPtr);

#endif /* CANIF_H */
