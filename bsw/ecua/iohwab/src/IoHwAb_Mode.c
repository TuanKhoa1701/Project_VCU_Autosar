/**********************************************************
 * @file    IoHwAb_Mode.c
 * @brief   IoHwAb – Drive Mode (ECO/NORMAL)
 * @details Đọc chế độ lái qua MCAL Dio hoặc lấy từ lớp COM (MCAL/COM),
 *          rồi ánh xạ về DRIVEMODE_ECO / DRIVEMODE_NORMAL.
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#include "IoHwAb.h"
#include <stddef.h>
#include "Dio.h"
#define IOHWAB_DIO_MODE 17

/* Skeleton: trả ECO để chạy được ngay.
 * Khi tích hợp, thay phần TODO bằng đọc DIO hoặc COM qua MCAL.
 */
Std_ReturnType IoHwAb_Mode_Get(DriveMode_e* mode)
{
    if (mode == NULL) {
        return E_NOT_OK;            
    }

     Dio_LevelType lv = DIO_ReadChannel(IOHWAB_DIO_MODE);
    // Nếu HIGH = NORMAL:
     *mode = (lv != STD_LOW) ? DRIVEMODE_NORMAL : DRIVEMODE_ECO;
    // Nếu LOW = NORMAL thì đảo lại.
    return E_OK;
}
