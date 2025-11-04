/**
 * @file      ComStack_Types.h
 * @brief     Định nghĩa các kiểu dữ liệu chung cho Communication Stack (ComStack).
 * @details   File này cung cấp các kiểu dữ liệu cơ bản và cấu trúc được sử dụng
 *            trên toàn bộ các lớp của ComStack (ví dụ: COM, PduR, CanIf),
 *            tuân thủ theo đặc tả của AUTOSAR.
 *            
 *            Theo SWS Communication Stack Types:
 *              - PduInfoType mang con trỏ payload (SduDataPtr), độ dài (SduLength),
 *                và (từ R4.3+) con trỏ MetaData (MetaDataPtr) nếu dùng. 
 *              - PduIdType / PduLengthType là kiểu ID/độ dài PDU dùng xuyên BSW.
 * 
 *            Các định nghĩa chính bao gồm:
 *            - PduIdType: Kiểu dữ liệu cho ID của một PDU.
 *            - PduLengthType: Kiểu dữ liệu cho độ dài của một PDU.
 *            - PduInfoType: Cấu trúc chứa thông tin của một PDU, bao gồm con trỏ
 *              dữ liệu và độ dài.
 *            - ComTxState_t: Cấu trúc quản lý trạng thái truyền của một PDU.
 *
 * @version   1.1
 * @date      2024-07-29
 * @author    Nguyễn Tuấn Khoa
 */

#ifndef COMSTACK_TYPES_H
#define COMSTACK_TYPES_H

/* AUTOSAR Standard Types (Std_ReturnType, uint8, uint16_t, boolean, ...) */
#include "Std_Types.h"
#include <stdbool.h>
/* Cần TickType để quản lý thời gian truyền PDU */
#include "Os_Types.h"

/**
 * @brief   Định danh cho một Protocol Data Unit (PDU).
 * @details Được sử dụng bởi các module trong ComStack (COM, PduR, CanIf, ...)
 *          để xác định một I-PDU một cách duy nhất.
 */
typedef uint16_t PduIdType;

/**
 * @brief   Độ dài của dữ liệu trong một PDU, tính bằng byte.
 */
typedef uint32_t PduLengthType;

/**
 * @struct  ComTxState_t
 * @brief   Cấu trúc quản lý trạng thái và thời gian truyền cho một PDU.
 * @details Được sử dụng bởi module COM để theo dõi các yêu cầu truyền,
 *          đảm bảo tuân thủ các quy tắc về thời gian (ví dụ: minimum delay).
 */
typedef struct
{
    bool txPending;         /**< Cờ báo hiệu có một yêu cầu truyền đang chờ xử lý. */
    TickType nextAllowedTx; /**< Thời điểm (tick) sớm nhất được phép truyền PDU tiếp theo. */
    TickType nextPeriod;    /**< Thời điểm (tick) của lần truyền định kỳ tiếp theo. */
} ComTxState_t;

/**
 * @struct  PduInfoType
 * @brief   Cấu trúc cơ bản để truyền dữ liệu và thông tin của một PDU.
 * @details Theo chuẩn AUTOSAR, cấu trúc này chứa con trỏ đến dữ liệu (SDU),
 *          độ dài dữ liệu và con trỏ tới siêu dữ liệu (tùy chọn).
 */
typedef struct
{
    uint8_t *SduDataPtr;       /**< Con trỏ tới vùng đệm chứa dữ liệu (Service Data Unit). */
    uint8_t *MetaDataPtr;      /**< Con trỏ tới siêu dữ liệu (tùy chọn, có thể là NULL). */
    PduLengthType SduLength; /**< Độ dài của dữ liệu (SduDataPtr) tính bằng byte. */
} PduInfoType;

#endif /* COMSTACK_TYPES_H */
