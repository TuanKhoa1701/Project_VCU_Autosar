/**********************************************************************************************************************
 * @file    Can_GeneralTypes.h
 * @brief   Định nghĩa các kiểu dữ liệu chung cho CAN Driver và các lớp trên.
 * @details File này chứa các định nghĩa về kiểu dữ liệu, cấu trúc, và enum được sử dụng
 *          trên toàn bộ CAN stack (CAN Driver, CAN Interface, v.v.). Các định nghĩa này
 *          được thiết kế theo phong cách tương tự AUTOSAR để đảm bảo tính module hóa và
 *          tái sử dụng.
 *
 *          Bao gồm:
 *          - Các kiểu dữ liệu cơ bản cho ID, Handle phần cứng.
 *          - Cấu trúc mô tả PDU (Protocol Data Unit - `Can_PduType`) để trao đổi dữ liệu giữa
 *            lớp CanIf và CanDrv.
 *          - Cấu trúc mô tả đối tượng phần cứng (`Can_HwType`) để ánh xạ một PDU tới một
 *            mailbox phần cứng cụ thể.
 *          - Các enum định nghĩa trạng thái của CAN controller (`Can_ControllerStateType`),
 *            trạng thái lỗi bus (`Can_ErrorStateType`), và các loại lỗi chi tiết (`Can_ErrorType`).
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 *********************************************************************************************************************/
#ifndef CAN_GENERAL_TYPES_H
#define CAN_GENERAL_TYPES_H
#include "stm32f10x.h"
#include "Std_Types.h"
#include "ComStack_Types.h"

/* Mã trả về khi một mailbox đang bận, không thể thực hiện truyền ngay lập tức */
#define CAN_BUSY 0x02u

/* Kiểu dữ liệu cho ID của một CAN message (Standard hoặc Extended) */
typedef uint32_t Can_IdType;

/* Kiểu dữ liệu cho handle của một đối tượng phần cứng (Hardware Object Handle - HOH) */
typedef uint16_t Can_HwHandleType;

/*************************************************** 
 * @struct Can_PduType
 * @brief  Thông tin về một PDU (Protocol Data Unit) được yêu cầu truyền.
 *         Được lớp trên (CanIf) truyền xuống cho CanDrv.
 **************************************************/
typedef struct
{
    PduIdType swPduHandle; /* ID của PDU ở mức phần mềm (do CanIf quản lý) */
    uint8_t length;        /* Độ dài dữ liệu (DLC - Data Length Code) */
    Can_IdType id;         /* ID của CAN frame */
    uint8_t* sdu;          /* Con trỏ tới buffer chứa dữ liệu (Service Data Unit) */
} Can_PduType;

/************************************************** 
 * @struct Can_HwType
 * @brief  Mô tả một đối tượng phần cứng CAN, dùng để ánh xạ PDU tới phần cứng.
 **************************************************/
typedef struct {
    Can_IdType CanId;         /* ID của CAN frame được cấu hình cho HOH này */
    Can_HwHandleType Hoh;     /* Handle của đối tượng phần cứng (ví dụ, mailbox) */
    uint8_t ControllerId;     /* ID của CAN controller (ví dụ, CAN1, CAN2) */
} Can_HwType;

/************************************************** 
 * @enum  Can_ErrorStateType
 * @brief Trạng thái lỗi của CAN controller theo chuẩn CAN.
 **************************************************/
typedef enum {
    CAN_ERRORSTATE_ACTIVE = 0, /* Controller hoạt động bình thường, có thể gửi Active Error Frame */
    CAN_ERRORSTATE_PASSIVE,    /* Controller bị lỗi, chỉ có thể gửi Passive Error Frame */
    CAN_ERRORSTATE_BUSOFF      /* Controller đã ngắt kết nối khỏi bus do lỗi nghiêm trọng */
} Can_ErrorStateType;

/*************************************************** 
 * @enum  Can_ControllerStateType
 * @brief Trạng thái hoạt động của CAN controller driver.
 **************************************************/
typedef enum {
    CAN_CS_UNINIT,  /* Chưa khởi tạo */
    CAN_CS_STARTED, /* Đã khởi tạo và đang chạy */
    CAN_CS_STOPPED, /* Đã dừng */
    CAN_CS_SLEEP    /* Đang ở chế độ ngủ */
} Can_ControllerStateType;

/****************************************************
 * @enum  Can_ErrorType
 * @brief Liệt kê các loại lỗi phần cứng có thể xảy ra trên bus CAN.
 *****************************************************/
typedef enum {
    CAN_ERROR_BIT_MONITORING1,    /* Lỗi bit monitoring: bit đọc về khác bit gửi đi */
    CAN_ERROR_BIT_MONITORING0,
    CAN_ERROR_BIT,                /* Lỗi bit chung */
    CAN_ERROR_CHECK_ACK_FAILED,   /* Không nhận được ACK */
    CAN_ERROR_ACK_DELIMITER,      /* Lỗi ở ACK delimiter */
    CAN_ERROR_ARBITRATION_LOST,   /* Mất quyền ưu tiên trên bus */
    CAN_ERROR_OVERLOAD,           /* Bus bị quá tải */
    CAN_ERROR_CHECK_FORM_FAILED,  /* Lỗi định dạng frame (form error) */
    CAN_ERROR_CHECK_STUFFING_FAILED, /* Lỗi nhồi bit (stuffing error) */
    CAN_ERROR_CHECK_CRC_FAILED,   /* Lỗi kiểm tra CRC */
    CAN_ERROR_BUS_LOCK            /* Bus bị kẹt ở một trạng thái */
} Can_ErrorType;

#endif