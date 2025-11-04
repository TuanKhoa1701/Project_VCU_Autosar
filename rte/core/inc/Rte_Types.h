/**********************************************************
 * @file    Rte_Type.h
 * @brief   Định nghĩa kiểu dữ liệu domain dùng bởi RTE/SWC
 * @details Gom các kiểu “business” dùng chung giữa các SWC
 *          (Sender/Receiver, Client/Server) và BSW/COM.
 *          File này KHÔNG chứa API; chỉ là kiểu và mô tả ý nghĩa.
 *
 *          Quy ước:
 *            - Dùng kiểu chuẩn AUTOSAR từ Std_Types.h (uint8, uint16,
 *              boolean, Std_ReturnType, E_OK/E_NOT_OK, TRUE/FALSE).
 *            - Các enum biểu diễn giá trị rời rạc (gear, drive mode…).
 *            - Struct Safe_s là gói dữ liệu an toàn tổng hợp để
 *              chuyển giữa các SWC (ví dụ SafetyManager → CmdComposer).
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#ifndef RTE_TYPE_H
#define RTE_TYPE_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================
 * 1) Kiểu liệt kê vị trí cần số tự động
 *    - Dùng cho các SWC: GearSelector, SafetyManager, CmdComposer…
 *    - Mapping sang tín hiệu trên bus có thể mã hoá 0..3 tương ứng.
 * =======================================================*/
/**
 * @enum  Gear_e
 * @brief Vị trí cần số tự động
 */
typedef enum {
  GEAR_P = 0,  /**< Parking  – khoá truyền động, xe đứng yên    */
  GEAR_R = 1,  /**< Reverse  – lùi                              */
  GEAR_N = 2,  /**< Neutral  – mo                               */
  GEAR_D = 3   /**< Drive    – tiến                             */
} Gear_e;

/* =========================================================
 * 2) Kiểu liệt kê chế độ lái
 *    - Tuỳ dự án có thể mở rộng thêm SPORT, SNOW… (giá trị mới).
 * =======================================================*/
/**
 * @enum  DriveMode_e
 * @brief Chế độ vận hành của hệ thống truyền lực
 */
typedef enum {
  DRIVEMODE_ECO    = 0,  /**< Ưu tiên tiết kiệm                     */
  DRIVEMODE_NORMAL = 1   /**< Vận hành thông thường                 */
} DriveMode_e;

/* =========================================================
 * 3) Gói dữ liệu an toàn tổng hợp
 *    - Được SafetyManager kết xuất sau khi kiểm tra/giới hạn
 *      các tín hiệu thô (pedal, brake, gear, mode…).
 *    - Dùng làm đầu vào cho bộ tạo lệnh (CmdComposer) hoặc
 *      điều khiển động cơ/biến tần (MotorCtrl) tuỳ kiến trúc.
 * =======================================================*/
/**
 * @struct Safe_s
 * @brief  Gói dữ liệu an toàn dùng trao đổi giữa SWC
 */
typedef struct {
  uint8_t     throttle_pct;  /**< % đạp ga 0..100 (đã clamp)            */
  Gear_e      gear;          /**< P/R/N/D (đã hợp lệ)                    */
  DriveMode_e driveMode;     /**< ECO/NORMAL                             */
  boolean     brakeActive;   /**< TRUE nếu phanh đang tác dụng           */
} Safe_s;
/* (tùy chọn) Kiểu ADC/DIO chuẩn hóa */
    typedef uint16_t Adc_mV_t; /* mV đọc từ ADC sau khi scale */
    typedef uint8_t Percent_t; /* 0..100% */
/* =========================================================
 * 4) Tốc độ động cơ (rpm)
 *    - Giá trị đến từ COM (CAN/LIN) hoặc đo trong hệ thống.
 *    - Phạm vi/độ phân giải do yêu cầu chẩn đoán/hệ thống quyết định.
 * =======================================================*/
/** @brief Kiểu dữ liệu tốc độ động cơ theo phút (rpm) */
typedef uint16_t EngineSpeedRpm_t;
/* (tùy chọn) Result code riêng của cổng dịch vụ IoHwAb/CanIf */
#ifndef RTE_E_OK
#define RTE_E_OK ((Std_ReturnType)E_OK)
#endif
#ifndef RTE_E_NOT_OK
#define RTE_E_NOT_OK ((Std_ReturnType)E_NOT_OK)
#endif
#define RTE_E_INVALID ((Std_ReturnType)E_NOT_OK)
#define RTE_E_NO_DATA ((Std_ReturnType)E_NOT_OK)
#define RTE_E_LIMIT ((Std_ReturnType)E_NOT_OK)
#ifdef __cplusplus
}
#endif

#endif /* RTE_TYPE_H */
