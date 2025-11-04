/**********************************************************
 * @file    Com.h
 * @brief   AUTOSAR COM – API rút gọn cho truyền/nhận tín hiệu
 * @details Lớp truyền thông theo phong cách AUTOSAR:
 *          - TX: gửi từng Signal/SignalGroup vào I-PDU và (tuỳ cấu hình)
 *                  kích phát truyền I-PDU (Triggered).
 *          - RX: đọc giá trị Signal đã được cập nhật từ I-PDU nhận.
 *          - Liên kết PduR: Com_RxIndication/Com_TxConfirmation.
 *
 *          Ghi chú triển khai:
 *            • Module nhắm chạy trên MCU thật (không mô phỏng PC).
 *            • Cấu hình (symbolic IDs, mapping signal↔IPDU, packing)
 *              do Com_Cfg.h/.c sinh ra (tool) hoặc viết tay cho demo.
 *            • API dưới đây synchronous, non-reentrant theo mặc định.
 *            • Khi truy cập từ ISR và Task, bảo đảm vùng dữ liệu chia
 *              sẻ có bảo vệ phù hợp (tắt IRQ ngắn/lock).
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/
#ifndef COM_H
#define COM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"        /* Std_ReturnType, boolean, uint8/16... */
#include "ComStack_Types.h"   /* PduIdType, PduInfoType */

/* =========================================================
 * 1) ID typedef (rút gọn)
 *    - Trong dự án thực tế, các kiểu này có thể do tool sinh
 *      ra với độ rộng phù hợp. Ở đây dùng uint16 cho gọn.
 * =======================================================*/
typedef uint16_t Com_SignalIdType;      /**< Kiểu dữ liệu cho ID của Signal. */

#include "Com_Cfg.h"          /* Com_SignalIdType, Com_SignalGroupIdType, symbolic IDs */

typedef enum {
    COM_UNINIT,
    COM_INIT
}Com_StatusType;
/* =========================================================
 * 1) Lifecycle
 * =======================================================*/
/**
 * @brief   Khởi tạo COM module.
 * @details Nạp cấu hình, reset buffer, đưa IPDU về trạng thái mặc định.
 *          Phải được gọi trước khi dùng mọi API COM khác.
 * @note    Gọi từ pha khởi động BSW (ví dụ EcuM/InitTask).
 */
void Com_Init(void);

/**
 * @brief   Dừng COM module.
 * @details Giải phóng tài nguyên/tắt truyền nhận (nếu có).
 */
void Com_DeInit(void);

void Com_MainFunction(void);
/**)
/* =========================================================
 * 2) TX API
 * =======================================================*/
/**
 * @brief   Ghi một Signal vào shadow buffer của COM.
 * @details COM sẽ pack giá trị này vào IPDU tương ứng theo cấu hình
 *          (endianness, bit position, length). Nếu IPDU ở mode
 *          “Triggered”, cần gọi thêm Com_TriggerIPDUSend() để phát ngay.
 *
 * @param   id       ID tượng trưng của Signal (Com_SignalIdType).
 * @param   dataPtr  Con trỏ tới giá trị nguồn (kiểu dữ liệu đúng với cấu hình Signal).
 * @return  E_OK nếu ghi thành công; E_NOT_OK nếu tham số/ID không hợp lệ.
 */
Std_ReturnType Com_SendSignal(Com_SignalIdType id, const void* dataPtr);

/**
 * @brief   Kích phát gửi một I-PDU ngay lập tức.
 * @details Dành cho IPDU cấu hình chế độ truyền “Triggered” hoặc
 *          khi ứng dụng cần phát hiện thời điểm gửi.
 *
 * @param   pduId  ID tượng trưng của I-PDU (TX).
 * @return  E_OK nếu đã xếp lịch/đẩy xuống PduR; E_NOT_OK nếu lỗi tham số/ID.
 */
Std_ReturnType Com_TriggerIPDUSend(PduIdType pduId);

/* =========================================================
 * 3) RX API
 * =======================================================*/
/**
 * @brief   Đọc giá trị một Signal đã được COM cập nhật từ I-PDU nhận.
 * @details COM trả ra “last value” ở bộ nhớ nội bộ sau khi unpack.
 *
 * @param   id       ID tượng trưng của Signal (RX).
 * @param   dataPtr  Con trỏ nhận giá trị (kiểu phải khớp cấu hình).
 * @return  E_OK nếu đọc thành công; E_NOT_OK nếu tham số/ID không hợp lệ.
 */
Std_ReturnType Com_ReceiveSignal(Com_SignalIdType id, void* dataPtr);
/**
 * @brief   Callback được gọi bởi PduR khi có một I-PDU RX được nhận.
 * @details Hàm này có vai trò chính là nhận dữ liệu thô từ lớp dưới (PduR)
 *          .Trong phiên bản demo này, logic được đơn giản hóa bằng cách xử 
 *          lý trực tiếp một số lệnh (command) thay vì thực hiện giải mã tín hiệu chung.
 *
 * @param   ComRxPduId ID của I-PDU nhận được (do PduR cung cấp).
 * @param   PduInfoPtr Con trỏ chứa dữ liệu và độ dài của PDU.
 */
void Com_RxIndication(PduIdType ComRxPduId, const PduInfoType* PduInfoPtr);

void Com_TxConfirmation(PduIdType ComTxPduId);

#ifdef __cplusplus
}
#endif

#endif /* COM_H */
