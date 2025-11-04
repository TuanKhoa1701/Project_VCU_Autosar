/**
 * @file    PduR.c
 * @brief   Triển khai (implementation) cho PDU Router (PduR) Module.
 * @details File này chứa logic hoạt động của các hàm API được định nghĩa trong PduR.h.
 *          Nó đóng vai trò trung gian, nhận các yêu cầu từ một module và định tuyến
 *          chúng đến module khác dựa trên các bảng định tuyến đã được cấu hình.
 *
 *          Luồng hoạt động chính:
 *          - **Luồng truyền (TX)**: `PduR_ComTransmit()` được COM gọi. PduR tra cứu
 *            bảng `ComTxRoutingTable` để tìm PDU ID của lớp dưới (CanIf) và gọi
 *            `CanIf_Transmit()`.
 *          - **Luồng nhận (RX)**: `PduR_CanIfRxIndication()` được CanIf gọi. PduR tra
 *            cứu bảng `CanIfRxRoutingTable` để tìm PDU ID của lớp trên (COM) và gọi
 *            `Com_RxIndication()`.
 *          - **Luồng xác nhận truyền (TX Confirmation)**: `PduR_CanIfTxConfirmation()`
 *            được CanIf gọi. PduR tra cứu bảng `CanIfTxRoutingTable` để tìm PDU ID
 *            của lớp trên (COM) và gọi `Com_TxConfirmation()`.
 *
 * @version 1.1
 * @date    2025-09-12
 * @author  Nguyễn Tuấn Khoa
 */

#include "PduR.h"
/* Các module lớp trên và lớp dưới mà PduR tương tác */
#include "Com.h"   // Lớp trên (Upper Layer)
#include "CanIf.h" // Lớp dưới (Lower Layer)
#include "PduR_Cfg.h"
/* =================================================================================== */
/*                            KHAI BÁO HÀM EXTERNAL                                    */
/* =================================================================================== */

/**
 * @brief Khai báo các hàm API từ các module khác mà PduR sẽ gọi.
 * @details Sử dụng `extern` để báo cho trình biên dịch biết rằng định nghĩa của các
 *          hàm này nằm ở một file khác và sẽ được liên kết (link) sau.
 */
extern Std_ReturnType CanIf_Transmit(PduIdType TxPduId, const PduInfoType* PduInfo);
extern void Com_RxIndication(PduIdType ComRxPduId, const PduInfoType* PduInfoPtr);
extern void Com_TxConfirmation(PduIdType ComTxPduId);
/* =================================================================================== */
/*                               BIẾN TOÀN CỤC TĨNH (STATIC)                           */
/* =================================================================================== */

/** @brief Trạng thái hoạt động hiện tại của module PduR. */
static PduR_StateType PduR_State = PDUR_UNINIT;

/** @brief Con trỏ tới cấu trúc cấu hình post-build của PduR. */
static PduR_PBConfigType* PduR_PBConfig = NULL;

/** @brief Cờ cho phép/vô hiệu hóa toàn bộ hoạt động định tuyến. */
static boolean Routing_Enable = FALSE;

/**
 * @brief   Hàm nội bộ để tìm kiếm một đường định tuyến trong bảng.
 * @details Duyệt qua bảng định tuyến (`tbl`) để tìm mục có `srcPduId` khớp với
 *          ID được cung cấp.
 * @param[in] tbl       Con trỏ tới bảng định tuyến cần tìm kiếm.
 * @param[in] n         Số lượng mục trong bảng.
 * @param[in] srcPduId  PDU ID nguồn cần tìm.
 * @return  Chỉ số của mục tìm thấy trong bảng nếu thành công.
 *          -1 nếu không tìm thấy.
 */
static inline sint32_t prv_find_route(const PduR_Route_1to1_Type* tbl, uint16_t n, PduIdType srcPduId){
    for(uint16_t i = 0; i < n; ++i){
        if(tbl[i].srcPduId == srcPduId){
            return i;
        }
    }
    return -1;
}

/* =================================================================================== */
/*                                  TRIỂN KHAI CÁC HÀM API                               */
/* =================================================================================== */

void PduR_Init(const PduR_PBConfigType* config){
    /* Chỉ thực hiện khởi tạo nếu module đang ở trạng thái UNINIT */
    if (PduR_State != PDUR_UNINIT)
        return;

    /* Nếu không có cấu hình hợp lệ, PduR không thể hoạt động */
    if (config == NULL){
        PduR_State = PDUR_UNINIT;
        PduR_PBConfig = NULL;
        Routing_Enable = FALSE;
        return;
    }

    /* Lưu cấu hình và chuyển sang trạng thái ONLINE */
    PduR_PBConfig = config;
    PduR_State = PDUR_ONLINE;
    Routing_Enable = TRUE;
    //printf("PduR_Init\n");
}

void PduR_EnableRouting(PduR_RoutingPathGroupIdType groupId){
    (void)groupId; // Tham số không được sử dụng trong phiên bản đơn giản này
    if (PduR_State != PDUR_ONLINE){
        return;
    }
    Routing_Enable = TRUE;
    //printf("PduR_EnableRouting\n");
}

void PduR_DisableRouting(PduR_RoutingPathGroupIdType groupId, boolean initialize){
    (void)groupId; // Tham số không được sử dụng trong phiên bản đơn giản này
    if (PduR_State != PDUR_ONLINE){
        return;
    }
    /* Ghi đè giá trị `initialize` và vô hiệu hóa định tuyến.
     * Trong một hệ thống phức tạp, `initialize` có thể được dùng để reset buffer. */
    initialize = FALSE;
    Routing_Enable = initialize;
}

Std_ReturnType PduR_ComTransmit(PduIdType TxPduId, const PduInfoType* Pduinfo){
    /* Kiểm tra điều kiện hoạt động: PduR phải ONLINE, định tuyến được bật, và con trỏ hợp lệ */
    if ((PduR_State != PDUR_ONLINE) || !Routing_Enable || Pduinfo == NULL || Pduinfo->SduDataPtr == NULL)
        return E_NOT_OK;
    PduR_PBConfig = &PduR_Config;
    /* Lấy bảng định tuyến cho luồng COM-TX */
    const PduR_Route_1to1_Type* entry = PduR_PBConfig->ComTxRoutingTable;
    if (entry == NULL) return E_NOT_OK;

    /* Tìm đường định tuyến cho PDU ID từ COM */
    sint32_t idx = prv_find_route(entry, PDUR_NUM_COM_TX_ROUTES, TxPduId);

    if(idx == -1) return E_NOT_OK;

    /* Lấy PDU ID đích (của CanIf) và chuyển tiếp yêu cầu */
    const PduIdType CanIfId = entry[(uint16_t) idx].desPduId;
    return CanIf_Transmit(CanIfId, Pduinfo);
}

void PduR_CanIfRxIndication( PduIdType RxPduId ,const PduInfoType *PduInfoPtr){

    /* Kiểm tra điều kiện hoạt động và các tham số đầu vào */
    if ((PduR_State != PDUR_ONLINE) || !Routing_Enable || (PduInfoPtr == NULL) || (PduInfoPtr->SduDataPtr == NULL)) {
        return;
    }

    /* Lấy bảng định tuyến cho luồng CanIf-RX */
    const PduR_Route_1to1_Type* entry = PduR_Config.CanIfRxRoutingTable;
    if (entry == NULL) return;

    /* Tìm đường định tuyến cho PDU ID từ CanIf */
    sint32_t idx = prv_find_route(entry, PDUR_NUM_CAN_RX_ROUTES, RxPduId);

    if(idx == -1) return;

    /* Lấy PDU ID đích (của COM) và chuyển tiếp dữ liệu nhận được */
    const PduIdType comId = entry[(uint16_t) idx].desPduId;
    Com_RxIndication(comId, PduInfoPtr);
}

void PduR_CanIfTxConfirmation(PduIdType TxPduId){
    /* Kiểm tra điều kiện hoạt động */
    if ((PduR_State != PDUR_ONLINE) || !Routing_Enable) return;

    /* Lấy bảng định tuyến cho luồng xác nhận truyền từ CanIf */
    const PduR_Route_1to1_Type* entry = PduR_PBConfig->CanIfTxRoutingTable;
    if (entry == NULL) return;

    /* Tìm đường định tuyến cho PDU ID từ CanIf */
    sint32_t idx = prv_find_route(entry, PDUR_NUM_CAN_TX_ROUTES, TxPduId);

    if(idx == -1) return;

    /* Lấy PDU ID đích (của COM) và chuyển tiếp thông báo xác nhận */
    const PduIdType comId = entry[(uint16_t) idx].desPduId;

    Com_TxConfirmation(comId);
}

void PduR_GetVersionInfo(Std_VersionInfoType *versioninfo){
    if (versioninfo == NULL) return;

    versioninfo->vendorID = PDUR_VENDOR_ID;
    versioninfo->moduleID = PDUR_MODULE_ID;
    versioninfo->sw_major_version = PDUR_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = PDUR_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = PDUR_SW_PATCH_VERSION;
}