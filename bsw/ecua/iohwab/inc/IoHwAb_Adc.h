/**********************************************************
 * @file    IoHwAb_ADC.h
 * @brief   I/O Hardware Abstraction (IoHwAb) – Giao diện dịch vụ phần cứng
 * @details Lớp trừu tượng phần cứng theo phong cách AUTOSAR dùng cho dự án:
 *          - Cung cấp API **đồng bộ** để SWC truy cập phần cứng qua RTE
 *            (Client/Server), ẩn chi tiết MCU/driver (ADC/GPIO/SPL/HAL).
 *          - Cung cấp các API trừu tượng để đọc giá trị từ các kênh ADC,
 *          ẩn đi chi tiết của lớp MCAL (ADC Driver). Các hàm này
 *          thường được gọi bởi các SWC thông qua RTE.
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
#ifndef __IOHWAB_ADC_H__
#define __IOHWAB_ADC_H__

#include "IoHwAb_Types.h"
#include "IoHwAb_Adc_Cfg.h"

/************************************
 * @typedef void
 * @brief: Hàm khởi tạo IoHwAb ADC
 * @param[in] ConfigPtr: Con trỏ tới cấu hình IoHwAb0_ConfigType
 * @return: Không trả về giá trị
 *************************************/
void IoHwAb_Init0(const IoHwAb0_ConfigType *ConfigPtr);

/************************************
 * @typedef Std_ReturnType
 * @brief: Hàm đọc giá trị ADC thô
 * @param[in] Value: Con trỏ tới biến lưu giá trị ADC
 * @return: Giá trị trả về kiểu Std_ReturnType
 ****************************************/
Std_ReturnType IoHwAb_ReadRaw_0(uint16_t *value);

/*************************************
 * @typedef Std_ReturnType
 * @brief: Hàm đọc giá trị ADC đã hiệu chỉnh
 * @param[in] Value: Con trỏ tới biến lưu giá trị ADC
 * @return: Giá trị trả về kiểu Std_ReturnType
 *************************************/
Std_ReturnType IoHwAb_ReadScaleValue_0(uint16_t *temperature);

#endif /* __IOHWAB_ADC_H__ */