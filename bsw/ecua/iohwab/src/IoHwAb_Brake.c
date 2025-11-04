/**********************************************************
 * @file    IoHwAb_Brake.c
 * @brief   IoHwAb – Brake (trạng thái phanh)
 * @details Đọc mức logic qua MCAL Dio và quy về boolean:
 *          - TRUE  : đang đạp phanh
 *          - FALSE : không đạp
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn tuấn Khoa
 **********************************************************/

#include "IoHwAb.h"
#include "Dio.h"
#include <stddef.h>
#define IOHWAB_DIO_BRAKE 16


Std_ReturnType IoHwAb_Brake_Get(boolean* pressed)
{
    if (pressed == NULL) {
        return E_NOT_OK;
    }

    Dio_LevelType lv = DIO_ReadChannel(IOHWAB_DIO_BRAKE);
      // Nếu hệ thống định nghĩa HIGH = đang đạp:
    *pressed = (lv != STD_LOW) ? TRUE : FALSE;
        // Nếu LOW = đang đạp thì đảo lại:
        // *pressed = (lv == STD_LOW) ? TRUE : FALSE;

    return E_OK;
}
