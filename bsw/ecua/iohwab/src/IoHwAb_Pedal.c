/**********************************************************
 * @file    IoHwAb_Pedal.c
 * @brief   IoHwAb – Pedal (đọc % đạp ga 0..100)
 * @details Lớp trừu tượng phần cứng cho bàn đạp ga:
 *          - Đọc ADC qua MCAL → chuẩn hoá về phần trăm 0..100.
 *          - Không dùng float, chỉ số nguyên (tuỳ bạn khi tích hợp).
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#include "IoHwAb.h"
#include "Adc.h"

// Định nghĩa các giá trị thô (raw) tối thiểu và tối đa của ADC
// Cần điều chỉnh các giá trị này dựa trên kết quả đo thực tế từ cảm biến và ADC của bạn.
#define PEDAL_RAW_MIN    0u // Ví dụ: giá trị ADC khi bàn đạp không nhấn (0%)
#define PEDAL_RAW_MAX    4029u // Ví dụ: giá trị ADC khi bàn đạp nhấn hết cỡ (100%)


Std_ReturnType IoHwAb_Pedal_ReadPct(uint8_t* pct)
{
    if (pct == NULL) {
        return E_NOT_OK;
    }

    uint16_t raw = 0u;

    // Bắt đầu chuyển đổi cho nhóm ADC của bàn đạp
    Adc_StartGroupConversion(ADC_GROUP_PEDAL);

    // Chờ cho đến khi quá trình chuyển đổi hoàn tất
    //while (Adc_GetGroupStatus(ADC_GROUP_PEDAL) == ADC_BUSY) 

    Adc_ValueGroupType buf[1] = {0};
    if (Adc_ReadGroup(ADC_GROUP_PEDAL, buf) == E_OK) {
        raw = (uint16_t)buf[0];          // Giá trị thô từ ADC (ví dụ: 0..4095 cho 12-bit)

        // Quy đổi giá trị thô về phần trăm (0-100%)
        *pct = (uint8_t)(((uint32_t)(raw - PEDAL_RAW_MIN) * 100u) / (PEDAL_RAW_MAX - PEDAL_RAW_MIN));    }
    return E_OK;
}
