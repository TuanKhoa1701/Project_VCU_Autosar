/**********************************************************
 * @file    IoHwAb_Gear.c
 * @brief   IoHwAb – Gear (P/R/N/D)
 * @details Đọc mã hoá vị trí cần số qua 2 kênh DIO (B1:B0) bằng MCAL,
 *          rồi ánh xạ: 00=P, 01=R, 10=N, 11=D. Trả kèm cờ hợp lệ.
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#include "IoHwAb.h"
#include <stddef.h>
#include "Dio.h"
#define IOHWAB_DIO_GEAR_B0 8
#define IOHWAB_DIO_GEAR_B1 10

Std_ReturnType IoHwAb_Gear_Get(Gear_e* gear, boolean* valid)
{
    if (gear == NULL || valid == NULL) {
        return E_NOT_OK;
    }

    
    Dio_LevelType b0 = DIO_ReadChannel(IOHWAB_DIO_GEAR_B0);
    Dio_LevelType b1 = DIO_ReadChannel(IOHWAB_DIO_GEAR_B1);
    uint8_t bits = ((b1 != STD_LOW) ? 2u : 0u) | ((b0 != STD_LOW) ? 1u : 0u);
    switch (bits & 0x3u) {
        case 0u: *gear = GEAR_P; *valid = TRUE;  break;
        case 1u: *gear = GEAR_R; *valid = TRUE;  break;
        case 2u: *gear = GEAR_N; *valid = TRUE;  break;
        case 3u: *gear = GEAR_D; *valid = TRUE;  break;
        default: *gear = GEAR_P; *valid = FALSE; break;
    }
    return E_OK;
}
