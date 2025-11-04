/**** 
 * @file: Std_Types.h
 * @brief  Định nghĩa các kiểu dữ liệu tiêu chuẩn trong AUTOSAR.
 * @details  Đây là tệp tiêu đề cho các kiểu dữ liệu tiêu chuẩn được sử dụng trong AUTOSAR, bao gồm các kiểu dữ liệu nguyên thủy,
 *           kiểu trả về tiêu chuẩn, giá trị logic, và các định nghĩa khác.
 * @note This file is part of the AUTOSAR standard and should be used in compliance with the AUTOSAR guidelines.
 * @author Nguyen Tuan Khoa
 */
#ifndef STD_TYPES_H
#define STD_TYPES_H
// This file is part of the AUTOSAR standard.
#include <stdint.h>
/* ==============================================
    version information
  *=================================================/
 * @brief thông tin về phiên bản của Std_Types.h
 * @details các định nghĩa về việc phiên bản giúp dễ dàng quản lý và bảo trì mã nguồn.
 * @note This file is part of the AUTOSAR standard and should be used in compliance with the AUTOSAR guidelines.
**********************************************/
#define DIO_VENDOR_ID 123U
#define DIO_MODULE_ID 456U
#define STD_TYPES_SW_MAJOR_VERSION 1U
#define STD_TYPES_SW_MINOR_VERSION 0U
#define STD_TYPES_SW_PATCH_VERSION 0U
/* ==============================================================
 * Standard Return Types
 *===============================================================/
 * @brief  kiểu trả về tiêu chuẩn trong AUTOSAR.
 * @details Được sử dụng để báo cáo kết quả của các hàm trong AUTOSAR, với các giá trị như E_OK và E_NOT_OK
 * @note This file is part of the AUTOSAR standard and should be used in compliance with the AUTOSAR guidelines.
 */
typedef uint8_t Std_ReturnType;

#define E_OK 0x00U
#define E_NOT_OK 0x01U
/*============================================================
 * Standard Boolean Values
 *===========================================================/
 *********************************************************
 * @brief  Định nghĩa giá trị logic cao và logic thấp.
 * @details Được sử dụng để biểu diễn trạng thái của các biến, với các giá trị như STD_HIGH và STD_LOW.
 * @note This file is part of the AUTOSAR standard and should be used in compliance with the AUTOSAR guidelines.
 ************************************************************/
#define STD_HIGH 0x01U
#define STD_LOW 0x00U
/* ===========================================================
 * Standard NULL Pointer Definition
 * ===========================================================*/
/*********************************************************
 * @brief  Định nghĩa con trỏ NULL.
 * @details Được sử dụng để biểu thị rằng một con trỏ không trỏ đến bất kỳ địa chỉ hợp lệ nào.
 * @note This file is part of the AUTOSAR standard and should be used in compliance with the AUTOSAR guidelines.
 **********************************************************/
#ifndef NULL
#define NULL ((void *)0)
#endif
/*
 * ===========================================================
 * Standard Type Definitions
 * ===========================================================*/
/***************************************************************
 * @brief  Định nghĩa các kiểu dữ liệu tiêu chuẩn trong AUTOSAR.
 * @details Bao gồm các kiểu dữ liệu nguyên thủy như uint8_t, sint16_t, vuint32_t, vuint64_t, vfloat32_t, vfloat64_t.
 * @note This file is part of the AUTOSAR standard and should be used in compliance with the AUTOSAR guidelines.
 ****************************************************************/
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;
typedef signed char sint8_t;
typedef signed short sint16_t;
typedef signed long sint32_t;
typedef signed long long sint64_t;

typedef float float32_t;
typedef double float64_t;

typedef volatile uint8_t vuint8_t;
typedef volatile uint16_t vuint16_t;
typedef volatile uint32_t vuint32_t;
typedef volatile uint64_t vuint64_t;
typedef volatile sint8_t vsint8_t;
typedef volatile sint16_t vsint16_t;
typedef volatile sint32_t vsint32_t;
typedef volatile sint64_t vsint64_t;
/*
 * ===========================================================
 * Standard Boolean Type Definition
 * ===========================================================
 */
/*******************************************************************
 * @typedef boolean
 * @brief kiểu dữ liệu boolean
 * @details Sử dụng biểu diễn giá trị đúng/sai trong AUTOSAR.
 * @note This file is part of the AUTOSAR standard and should be used in compliance with the AUTOSAR guidelines.
 ********************************************************************/

typedef uint8_t boolean;

#ifndef TRUE
#define TRUE 1U
#endif
#ifndef FALSE
#define FALSE 0U
#endif
/*
 * ===========================================================
 * Standard Version Information Structure
 * ===========================================================
 */
/*******************************************************************
 * @struct Std_VersionInfoType
 * @brief Cấu trúc chứa thông tin phiên bản của một module trong AUTOSAR.
 * @details Bao gồm các trường như vendorID, moduleID, major, minor và patch để quản lý phiên bản.
 * @note This file is part of the AUTOSAR standard and should be used in compliance with the AUTOSAR guidelines.
 *********************************************************************/
typedef struct
{
    uint16_t vendorID;
    uint16_t moduleID;
    uint8_t sw_major_version;
    uint8_t sw_minor_version;
    uint8_t sw_patch_version;
} Std_VersionInfoType;
/*
 * ===========================================================
 * Standard Error Reporting
 * ===========================================================
 */
/*******************************************************************
 * @macro Det_ReportError
 * @brief Macro để báo cáo lỗi trong AUTOSAR.
 * @details Được sử dụng để ghi lại các lỗi xảy ra trong quá trình thực thi của hệ thống, với các tham số như moduleId, instanceId, apiId và errorId.
 * @note This file is part of the AUTOSAR standard and should be used in compliance with the AUTOSAR guidelines.
 ********************************************************************/
#define Det_ReportError(moduleId, instanceId, apiId, errorId)
/*
 * ===========================================================
 * Standard Status Definitions
 * ===========================================================
 */
/*******************************************************************
 * @macro STD_ACTIVE, STD_IDLE, STD_ON, STD_OFF
 * @brief Các định nghĩa trạng thái tiêu chuẩn trong AUTOSAR.
 * @details Được sử dụng để biểu diễn trạng thái hoạt động, nhàn rỗi, bật và tắt của các thành phần trong hệ thống.
 * @note This file is part of the AUTOSAR standard and should be used in compliance with the AUTOSAR guidelines.
 ********************************************************************/

#define STD_ACTIVE 0x01U
#define STD_IDLE 0x00U

#define STD_ON 0x01U
#define STD_OFF 0x00U

#endif
