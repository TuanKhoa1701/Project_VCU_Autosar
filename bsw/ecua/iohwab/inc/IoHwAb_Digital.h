/**********************************************************
 * @file    IoHwAb_Digital.h
 * @brief   I/O Hardware Abstraction (IoHwAb) – Giao diện dịch vụ phần cứng
 * @details Lớp trừu tượng phần cứng theo phong cách AUTOSAR dùng cho dự án:
 *          - Cung cấp API **đồng bộ** để SWC truy cập phần cứng qua RTE
 *            (Client/Server), ẩn chi tiết MCU/driver (ADC/GPIO/SPL/HAL).
 *          - Cung cấp các API trừu tượng để đọc và ghi các kênh I/O kỹ thuật số,
 *            ẩn đi chi tiết của lớp MCAL (DIO Driver). Các hàm này thường được gọi 
 *            bởi các SWC thông qua RTE.
 *
 *          Quy ước:
 *            - Tất cả API trả về Std_ReturnType:
 *                 E_OK     = thành công, *out hợp lệ
 *                 E_NOT_OK = lỗi/tham số NULL/driver chưa sẵn sàng
 *            - Con trỏ đầu ra (*out) **không được NULL**.
 *            - API mặc định **không tái nhập** (non-reentrant) trừ khi
 *              hiện thực có bảo vệ (IRQ lock/mutex/critical section).
 *            - Khuyến nghị gọi từ Task định kỳ (ví dụ 10 ms), không gọi
 *              từ ISR trừ khi driver đảm bảo đủ nhanh/an toàn.
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

 #ifndef __IOHWAB_DIGITAL_H__
#define __IOHWAB_DIGITAL_H__

#include "IoHwAb_Types.h"
#include "IoHwAb_Digital_Cfg.h"
/***********************************************
 * @details: Hàm khởi tạo IoHwAb Digital.
 * @param[in] cfg: Con trỏ cấu hình cho IoHwAb_Digital
 * @return : Void
 ************************************************/

void IoHwAb_Init1(const IoHwAb1_ConfigType *cfg);

/**********************************************
 * @details: Hàm đọc tín hiệu Digital từ chân vào.
 * @param[out] Value: con trỏ nhận giá trị đọc được.
 * @return : trả về giá trị Std_ReturnType (E_OK, E_NOT_OK)
 **********************************************/

Std_ReturnType IoHwAb_Digital_ReadChannel(IoHwAb_SignalType id,Dio_LevelType *Value);

/**********************************************
 * @details: Hàm ghi tín hiệu Digital ra chân
 * @param[in] Value : Giá trị cần ghi ra chân
 * @return : trả về giá trị Std_ReturnType (E_OK, E_NOT_OK)
 **********************************************/
Std_ReturnType IoHwAb_Digital_WriteSignal(IoHwAb_SignalType id,Dio_LevelType Value);
#endif