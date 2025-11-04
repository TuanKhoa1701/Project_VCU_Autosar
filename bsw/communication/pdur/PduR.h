/**
 * @file    PduR.h
 * @brief   Header file cho PDU Router (PduR) Module.
 * @details File này định nghĩa các giao diện (API), kiểu dữ liệu, và cấu trúc
 *          cấu hình cho module PduR theo kiến trúc AUTOSAR. PduR đóng vai trò
 *          là một "cổng trung chuyển" cho các đơn vị dữ liệu giao thức (PDU)
 *          giữa các module giao tiếp khác nhau.
 *
 *          Chức năng chính của PduR:
 *          - **Định tuyến PDU**: Nhận PDU từ một module và chuyển tiếp nó đến
 *            một hoặc nhiều module đích dựa trên bảng định tuyến đã cấu hình.
 *            Ví dụ:
 *              - Định tuyến yêu cầu truyền từ COM xuống CanIf (đường TX).
 *              - Định tuyến dữ liệu nhận được từ CanIf lên COM (đường RX).
 *              - Định tuyến thông báo xác nhận truyền (TxConfirmation) từ CanIf
 *                lên COM.
 *          - **Gateway**: Có thể thực hiện gateway giữa các bus khác nhau (ví dụ:
 *            CAN sang FlexRay), mặc dù trong phiên bản này chỉ tập trung vào
 *            định tuyến trong cùng một stack CAN.
 *          - **Quản lý trạng thái**: Cho phép bật/tắt các đường định tuyến (routing paths).
 *
 * @version 1.0
 * @date    2025-09-12
 * @author  Nguyễn Tuấn Khoa
 */
#ifndef PDUR_H
#define PDUR_H

#include "Std_Types.h"
#include "ComStack_Types.h" /* Cung cấp các kiểu dữ liệu chung như PduIdType, PduInfoType */

/**
 * @defgroup PDUR_VERSION_INFO Thông tin phiên bản của module PduR
 * @{
 */
#define PDUR_VENDOR_ID 0x100u
#define PDUR_MODULE_ID 0x200u
#define PDUR_SW_MAJOR_VERSION 1u
#define PDUR_SW_MINOR_VERSION 0u
#define PDUR_SW_PATCH_VERSION 0u
/** @} */

/**
 * @brief Kiểu dữ liệu cho ID của cấu hình post-build.
 */
typedef uint16_t PduR_PBConfigIdType;
/**
 * @brief Kiểu dữ liệu để định danh một nhóm các đường định tuyến (routing path).
 *        Dùng để bật/tắt định tuyến cho cả một nhóm.
 */
typedef uint16_t PduR_RoutingPathGroupIdType;
/**
 * @enum  PduR_StateType
 * @brief Các trạng thái hoạt động của module PduR.
 */
typedef enum{
    PDUR_UNINIT,    /**< Module chưa được khởi tạo. */
    PDUR_ONLINE     /**< Module đã được khởi tạo và sẵn sàng định tuyến. */
}PduR_StateType;

/**
 * @struct PduR_Route_1to1_Type
 * @brief  Định nghĩa một đường định tuyến 1-1.
 * @details Ánh xạ một PDU ID nguồn (từ module gửi) sang một PDU ID đích (cho module nhận).
 */
typedef struct {
    PduIdType srcPduId; /**< PDU ID tại module nguồn. */
    PduIdType desPduId; /**< PDU ID tại module đích. */
}PduR_Route_1to1_Type;

/**
 * @struct PduR_PBConfigType
 * @brief  Cấu trúc cấu hình chính (post-build) cho PduR.
 * @details Chứa các con trỏ tới các bảng định tuyến khác nhau.
 */
typedef struct{
    const PduR_Route_1to1_Type* CanIfRxRoutingTable; /**< Bảng định tuyến cho PDU nhận từ CanIf. */
    const PduR_Route_1to1_Type* CanIfTxRoutingTable; /**< Bảng định tuyến cho xác nhận truyền từ CanIf. */
    const PduR_Route_1to1_Type* ComTxRoutingTable;   /**< Bảng định tuyến cho PDU truyền từ COM. */
}PduR_PBConfigType;
/* =================================================================================== */
/*                                  KHAI BÁO HÀM API                                   */
/* =================================================================================== */

/**
 * @brief   Khởi tạo module PduR.
 * @details Hàm này khởi tạo module PduR, thiết lập trạng thái ban đầu và nạp
 *          cấu hình định tuyến. Nó phải được gọi trước khi bất kỳ API PduR nào khác
 *          được sử dụng.
 * @param[in] config Con trỏ tới cấu trúc cấu hình post-build của PduR.
 *                   Nếu `config` là NULL, module sẽ không được khởi tạo và
 *                   các hoạt động định tuyến sẽ bị vô hiệu hóa.
 * @pre     Module PduR phải ở trạng thái `PDUR_UNINIT`.
 * @post    Module PduR sẽ chuyển sang trạng thái `PDUR_ONLINE` nếu cấu hình hợp lệ,
 *          và các đường định tuyến sẽ được kích hoạt.
 */
void PduR_Init(const PduR_PBConfigType* config);

/**
 * @brief   Lấy thông tin phiên bản của module PduR.
 * @details Hàm này cung cấp thông tin về nhà cung cấp, ID module, và phiên bản
 *          phần mềm của module PduR.
 * @param[out] versioninfo Con trỏ tới cấu trúc `Std_VersionInfoType` để lưu
 *                         thông tin phiên bản. Con trỏ này không được NULL.
 * @pre     Không có yêu cầu đặc biệt về trạng thái module.
 * @post    Cấu trúc `versioninfo` sẽ được điền đầy đủ thông tin phiên bản.
 */
void PduR_GetVersionInfo(Std_VersionInfoType *versioninfo);

/* --- Giao diện với lớp dưới (Lower Layer Interface - LLI) --- */
/**
 * @brief   Chỉ báo nhận PDU từ module CAN Interface (CanIf).
 * @details Hàm này được module CanIf gọi khi một PDU đã được nhận thành công
 *          từ bus CAN. PduR sẽ sử dụng `RxPduId` để tra cứu bảng định tuyến
 *          `CanIfRxRoutingTable` và chuyển tiếp PDU này lên module lớp trên
 *          tương ứng (ví dụ: COM) thông qua hàm `Com_RxIndication`.
 * @param[in] RxPduId ID của PDU đã nhận, được định nghĩa bởi CanIf.
 * @param[in] PduInfoPtr Con trỏ tới cấu trúc `PduInfoType` chứa dữ liệu
 *                       (payload) và độ dài của PDU đã nhận.
 * @pre     Module PduR phải ở trạng thái `PDUR_ONLINE` và định tuyến phải được kích hoạt.
 * @post    Nếu tìm thấy đường định tuyến, PDU sẽ được chuyển tiếp lên lớp trên.
 */
void PduR_CanIfRxIndication( PduIdType RxPduId,const PduInfoType *PduInfoPtr);

/**
 * @brief   Xác nhận truyền PDU từ module CAN Interface (CanIf).
 * @details Hàm này được module CanIf gọi khi một PDU đã được truyền thành công
 *          trên bus CAN. PduR sẽ sử dụng `TxPduId` để tra cứu bảng định tuyến
 *          `CanIfTxRoutingTable` và chuyển tiếp thông báo xác nhận này lên
 *          module lớp trên tương ứng (ví dụ: COM) thông qua hàm `Com_TxConfirmation`.
 * @param[in] TxPduId ID của PDU đã được truyền, được định nghĩa bởi CanIf.
 * @pre     Module PduR phải ở trạng thái `PDUR_ONLINE` và định tuyến phải được kích hoạt.
 * @post    Nếu tìm thấy đường định tuyến, thông báo xác nhận sẽ được chuyển tiếp lên lớp trên.
 */
void PduR_CanIfTxConfirmation(PduIdType TxPduId);

/* --- Giao diện với lớp trên (Upper Layer Interface - ULI) --- */
/***********************************************************************
 * @brief   Yêu cầu truyền PDU từ module COM.
 * @details Hàm này được module COM gọi khi nó muốn truyền một PDU ra bus.
 *          PduR sẽ sử dụng `TxPduId` để tra cứu bảng định tuyến `ComTxRoutingTable`
 *          và chuyển tiếp yêu cầu truyền này xuống module lớp dưới tương ứng
 *          (ví dụ: CanIf) thông qua hàm `CanIf_Transmit`.
 * @param[in] TxPduId ID của PDU cần truyền, được định nghĩa bởi COM.
 * @param[in] Pduinfo Con trỏ tới cấu trúc `PduInfoType` chứa dữ liệu
 *                    (payload) và độ dài của PDU cần truyền.
 * @return  `E_OK` nếu yêu cầu truyền được chấp nhận và định tuyến thành công.
 *          `E_NOT_OK` nếu module chưa được khởi tạo, định tuyến bị vô hiệu hóa,
 *          tham số không hợp lệ, hoặc không tìm thấy đường định tuyến.
 */
Std_ReturnType PduR_ComTransmit(PduIdType TxPduId, const PduInfoType* Pduinfo);

/* --- Giao diện quản lý định tuyến --- */
/**
 * @brief   Kích hoạt định tuyến cho một nhóm đường định tuyến cụ thể.
 * @details Hàm này cho phép bật lại các đường định tuyến đã bị vô hiệu hóa
 *          trước đó. Trong triển khai hiện tại, nó chỉ bật cờ `Routing_Enable`
 *          cho toàn bộ module.
 * @param[in] groupId ID của nhóm đường định tuyến cần kích hoạt.
 * @pre     Module PduR phải ở trạng thái `PDUR_ONLINE`.
 * @post    Các đường định tuyến trong nhóm `groupId` sẽ được kích hoạt.
 */
void PduR_EnableRouting(PduR_RoutingPathGroupIdType groupId);

/**
 * @brief   Vô hiệu hóa định tuyến cho một nhóm đường định tuyến cụ thể.
 * @details Hàm này tạm dừng việc định tuyến cho các PDU thuộc nhóm `groupId`.
 *          Trong triển khai hiện tại, nó chỉ tắt cờ `Routing_Enable` cho toàn bộ module.
 * @param[in] groupId ID của nhóm đường định tuyến cần vô hiệu hóa.
 * @param[in] initialize Cờ này (nếu là `TRUE`) có thể được dùng để reset
 *                       trạng thái của các đường định tuyến trong nhóm.
 *                       Trong triển khai hiện tại, nó luôn được đặt thành `FALSE`
 *                       và chỉ ảnh hưởng đến cờ `Routing_Enable`.
 * @pre     Module PduR phải ở trạng thái `PDUR_ONLINE`.
 * @post    Các đường định tuyến trong nhóm `groupId` sẽ bị vô hiệu hóa.
 */
void PduR_DisableRouting(PduR_RoutingPathGroupIdType groupId, boolean initialize);
#endif /* PDUR_H */
