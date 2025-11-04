#ifndef __IOHWAB_TYPES_H__
#define __IOHWAB_TYPES_H__
#include "Std_Types.h"
#include "DIO.h"
#include "Port.h"
#include "Adc.h"
#include "PWM.h"
/*=============================
     VERSION INFORMATION
===============================*/
#define IoHwAb_VENDOR_ID 1234u
#define IoHwAb_MODULE_ID 567u
#define IoHwAb_SW_MAJOR_VERSION 1u
#define IoHwAb_SW_MINOR_VERSION 0u
#define IoHwAb_SW_PATCH_VERSION 0u
/*=============================
    Kiểu dữ liệu trạng thái chung cho IoHwAb
===============================*/
typedef enum
{
    IOHWAB_UNINIT = 0,
    IOHWAB_IDLE,
    IOHWAB_BUSY,
    IOHWAB_ERROR
} IoHwAb_StatusType;

/**************************************
 * @typedef void
 * @brief: Kiểu dữ liệu chung cho IoHwAb
 * @param[in] versioninfo: Con trỏ tới cấu trúc Std_VersionInfoType để nhận version
 * @return: none
 **************************************/
void IoHwAb_GetVersionInfo(Std_VersionInfoType *versioninfo);
/*************************************
 * @typedef void
 * @brief: Hàm chạy định kỳ để xử lý các tác vụ nền của IoHwAb
 * @param: none
 * @return: none
 ***************************************/
void IoHwAb_MainFunction(void);

#endif /* __IOHWAB_TYPES_H__ */