/**********************************************************
 * @file Adc.h
 * @brief Khai báo các hàm và kiểu dữ liệu liên quan đến điều khiển ADC
 * @details File này cung cấp các khai báo và định nghĩa cần thiết cho việc khởi tạo,
 *          chuyển đổi, đọc kết quả và quản lý trạng thái của ADC trên vi điều khiển.
 * @version 1.0
 * @date 2025-7-03
 * @author Nguyễn Tuấn Khoa
 **********************************************************/

#ifndef ADC_H
#define ADC_H

#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"
#include "Std_Types.h"

/* ============================= */
/* ========== MACRO =========== */
/* ============================= */
/** @brief Số lượng phần cứng ADC tối đa được hỗ trợ (ví dụ: ADC1, ADC2). */
#define ADC_MAX_HW 1
/** @brief Số lượng nhóm kênh ADC tối đa được cấu hình. */
#define ADC_MAX_GROUPS 2
/** @brief Kích thước bộ đệm cho nhóm ADC 1. */
#define ADC_BUFFER_SIZE_GROUP_1 4
/** @brief Kích thước bộ đệm cho nhóm ADC 2. */
#define ADC_BUFFER_SIZE_GROUP_2 4

/** @brief Định danh cho nhóm ADC 1. */
#define ADC_GROUP_1 0
/** @brief Định danh cho nhóm ADC 2. */
#define ADC_GROUP_2 1

/* ============================= */
/* ==== ENUM & STRUCT TYPE ==== */
/* ============================= */

/** @brief Kiểu dữ liệu cho thời gian chuyển đổi ADC. */
typedef uint32_t Adc_ConversionTimeType;
/** @brief Kiểu dữ liệu định danh một kênh ADC vật lý (ví dụ: ADC_Channel_0). */
typedef uint32_t Adc_ChannelType;
/** @brief Kiểu dữ liệu định danh một nhóm kênh ADC. */
typedef uint32_t Adc_GroupType;
/** @brief Kiểu dữ liệu cho giá trị kết quả của một lần chuyển đổi ADC (thường là 12-bit). */
typedef uint16_t Adc_ValueGroupType;
/** @brief Kiểu dữ liệu cho độ phân giải của ADC (ví dụ: 8, 10, 12 bit). */
typedef uint8_t Adc_ResolutionType;
/** @brief Kiểu dữ liệu cho mức độ ưu tiên của một nhóm ADC. */
typedef uint8_t Adc_GroupPriorityType;
/** @brief Kiểu dữ liệu cho số lượng mẫu trong một lần streaming. */
typedef uint8_t Adc_StreamNumberSampleType;
/** @brief Kiểu dữ liệu cho thời gian lấy mẫu của một kênh ADC. */
typedef uint8_t Adc_SamplingTimeType;

/** @enum Adc_GroupReplacementType
 *  @brief Xác định hành vi khi một yêu cầu chuyển đổi mới đến trong khi nhóm đang bận.
 */
typedef enum
{
    ADC_GROUP_REPL_ABORT_RESTART = 0, /**< Hủy yêu cầu cũ và bắt đầu lại yêu cầu mới. */
    ADC_GROUP_REPL_SUSPEND_RESUME = 1, /**< Tạm dừng yêu cầu cũ, thực hiện yêu cầu mới, sau đó tiếp tục yêu cầu cũ. */
} Adc_GroupReplacementType;

/** @enum Adc_HwTriggerSignalType
 *  @brief Xác định loại cạnh của tín hiệu trigger phần cứng.
 */
typedef enum
{
    ADC_HW_TRIG_RISING_EDGE = 0, /**< Kích hoạt bởi cạnh lên. */
    ADC_HW_TRIG_FALLING_EDGE = 1, /**< Kích hoạt bởi cạnh xuống. */
    ADC_HW_TRIG_BOTH_EDGES = 2    /**< Kích hoạt bởi cả hai cạnh. */
} Adc_HwTriggerSignalType;

/** @enum Adc_GroupAccessModeType
 *  @brief Xác định chế độ truy cập kết quả của một nhóm ADC.
 */
typedef enum
{
    ADC_GROUP_ACCESS_MODE_SINGLE = 0,    /**< Truy cập một lần, chỉ lấy kết quả mới nhất. */
    ADC_GROUP_ACCESS_MODE_STREAMING = 1  /**< Truy cập liên tục, kết quả được lưu vào một buffer. */
} Adc_GroupAccessModeType;

/** @enum Adc_NotificationType
 *  @brief Bật/tắt cơ chế thông báo (callback) khi một nhóm hoàn thành chuyển đổi.
 */
typedef enum
{
    ADC_NOTIFICATION_DISABLED = 0, /**< Tắt thông báo. */
    ADC_NOTIFICATION_ENABLED = 1   /**< Bật thông báo. */
} Adc_NotificationType;

/** @enum Dma_Adc_NotificationType
 *  @brief Bật/tắt cơ chế thông báo (callback) khi DMA hoàn thành truyền dữ liệu cho ADC.
 */
typedef enum
{
    DMA_ADC_NOTIFICATION_DISABLED = 0, /**< Tắt thông báo DMA. */
    DMA_ADC_NOTIFICATION_ENABLED = 1   /**< Bật thông báo DMA. */
} Dma_Adc_NotificationType;

/** @enum Adc_TriggerSourceType
 *  @brief Xác định nguồn kích hoạt cho một nhóm ADC.
 */
typedef enum
{
    ADC_TRIGGER_SOFTWARE = 0, /**< Kích hoạt bằng lời gọi hàm (software trigger). */
    ADC_TRIGGER_HARDWARE = 1  /**< Kích hoạt bằng tín hiệu phần cứng bên ngoài (ví dụ: timer). */
} Adc_TriggerSourceType;

/** @enum Adc_GroupConvModeType
 *  @brief Xác định chế độ chuyển đổi của một nhóm ADC.
 */
typedef enum
{
    ADC_CONV_MODE_SINGLE = 0,     /**< Chuyển đổi một lần sau mỗi lần trigger. */
    ADC_CONV_MODE_CONTINUOUS = 1  /**< Tự động chuyển đổi liên tục sau lần trigger đầu tiên. */
} Adc_GroupConvModeType;

/** @enum Adc_ResultAlignmentType
 *  @brief Xác định cách căn lề cho kết quả chuyển đổi trong thanh ghi dữ liệu.
 */
typedef enum
{
    ADC_RESULT_ALIGNMENT_RIGHT = 0, /**< Căn lề phải. */
    ADC_RESULT_ALIGNMENT_LEFT = 1   /**< Căn lề trái. */
} Adc_ResultAlignmentType;

/** @enum Adc_InstanceType
 *  @brief Định danh cho các ngoại vi ADC vật lý.
 */
typedef enum
{
    ADC_INSTANCE_1 = 0, /**< ADC peripheral 1. */
    ADC_INSTANCE_2 = 1  /**< ADC peripheral 2. */
} Adc_InstanceType;

/** @enum Adc_StatusType
 *  @brief Trạng thái hiện tại của một nhóm ADC.
 */
typedef enum
{
    ADC_IDLE,      /**< Nhóm đang rảnh và sẵn sàng cho một yêu cầu chuyển đổi mới. */
    ADC_BUSY,      /**< Nhóm đang trong quá trình thực hiện chuyển đổi. */
    ADC_COMPLETED, /**< Nhóm đã hoàn thành chuyển đổi và kết quả đã sẵn sàng. */
    ADC_ERROR      /**< Đã xảy ra lỗi trong quá trình chuyển đổi. */
} Adc_StatusType;

/** @enum Adc_StreamBufferModeType
 *  @brief Chế độ hoạt động của bộ đệm trong chế độ streaming.
 */
typedef enum
{
    ADC_STREAM_BUFFER_LINEAR = 0x00,   /**< Bộ đệm tuyến tính, dừng khi đầy. */
    ADC_STREAM_BUFFER_CIRCULAR = 0x01  /**< Bộ đệm vòng, ghi đè lên dữ liệu cũ khi đầy. */
} Adc_StreamBufferModeType;

/** @struct Adc_ChannelConfigType
 *  @brief Cấu hình cho một kênh ADC cụ thể trong một nhóm.
 */
typedef struct
{
    Adc_ChannelType Channel;           /**< Kênh ADC vật lý (ví dụ: ADC_Channel_1). */
    Adc_SamplingTimeType SamplingTime; /**< Thời gian lấy mẫu cho kênh này. */
    uint8_t Rank;                      /**< Thứ tự chuyển đổi của kênh trong nhóm. */
} Adc_ChannelConfigType;

/** @struct Adc_ConfigType
 *  @brief Cấu trúc cấu hình tổng thể cho một ngoại vi ADC.
 */
typedef struct
{
    uint32_t ClockPrescaler;                   /**< Bộ chia tần số cho clock ADC. */
    Adc_ResolutionType Resolution;             /**< Độ phân giải của ADC (không dùng trên STM32F1). */
    Adc_GroupConvModeType ConversionMode;      /**< Chế độ chuyển đổi (một lần/liên tục). */
    Adc_TriggerSourceType TriggerSource;       /**< Nguồn trigger (phần mềm/phần cứng). */
    Adc_NotificationType NotificationEnabled;  /**< Bật/tắt thông báo khi hoàn thành. */
    uint8_t NumChannels;                       /**< Tổng số kênh được cấu hình. */
    Adc_InstanceType AdcInstance;              /**< Ngoại vi ADC được sử dụng (ADC1/ADC2). */
    Adc_ChannelConfigType Channels[16];        /**< Mảng cấu hình cho từng kênh. */
    Adc_ResultAlignmentType ResultAlignment;   /**< Căn lề kết quả (trái/phải). */
    void (*InitCallback)(void);                /**< Con trỏ hàm callback khi có thông báo. */
} Adc_ConfigType;

/** @struct Adc_GroupDefType
 *  @brief Cấu trúc định nghĩa một nhóm kênh ADC.
 */
typedef struct
{
    Adc_GroupType id;                              /**< ID của nhóm kênh ADC */
    Adc_InstanceType AdcInstance;                  /**< ID cơ bản của mô-đun ADC */
    Adc_ChannelType Channels[16];                  /**< Danh sách kênh trong nhóm */
    Adc_GroupPriorityType Priority;                /**< Mức độ ưu tiên của nhóm */
    uint8_t NumChannels;                           /**< Số kênh trong nhóm */
    Adc_StatusType Status;                         /**< Trạng thái hiện tại của nhóm */
    Adc_ValueGroupType *Result;                    /**< Con trỏ buffer kết quả */
    uint8_t Adc_StreamEnableType;                  /**< Có bật DMA hay không (1 = ENABLE) */
    uint8_t Adc_StreamBufferSize;                  /**< Kích thước bộ đệm */
    Adc_StreamBufferModeType Adc_StreamBufferMode; /**< Kiểu hoạt động stream (Linear/Circular) */
    Dma_Adc_NotificationType Dma_Notification;     /**< Có bật thông báo DMA hay không */
} Adc_GroupDefType;

/** @enum Adc_PowerStateType
 *  @brief Các trạng thái nguồn của module ADC.
 */
typedef enum
{
    ADC_POWERSTATE_OFF,      /**< Tắt nguồn hoàn toàn. */
    ADC_POWERSTATE_ON,       /**< Bật nguồn, hoạt động bình thường. */
    ADC_POWERSTATE_LOWPOWER  /**< Chế độ năng lượng thấp. */
} Adc_PowerStateType;
/* ============================= */
/* ========== API ============= */
/* ============================= */

/**********************************************************
 * @brief Khởi tạo mô-đun ADC.
 * @details Hàm này khởi tạo các đơn vị phần cứng ADC và driver theo cấu hình được chỉ định.
 * @param[in] ConfigPtr Con trỏ tới tập cấu hình cho mô-đun ADC.
 * @return None
 **********************************************************/
void Adc_Init(const Adc_ConfigType *ConfigPtr);

/**********************************************************
 * @brief Thiết lập bộ đệm kết quả cho nhóm ADC.
 * @details Hàm này thiết lập địa chỉ bộ đệm kết quả cho các kênh trong nhóm.
 * @param[in] Group ID của nhóm kênh ADC.
 * @param[in] DataBufferPtr Con trỏ tới bộ đệm kết quả.
 * @return Std_ReturnType Trả về E_OK nếu thành công, E_NOT_OK nếu thất bại.
 **********************************************************/
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr);

/**********************************************************
 * @brief Hủy khởi tạo mô-đun ADC.
 * @details Đưa tất cả các đơn vị phần cứng ADC về trạng thái chưa khởi tạo.
 * @return None
 **********************************************************/
void Adc_DeInit(void);

/**********************************************************
 * @brief Bắt đầu chuyển đổi nhóm kênh ADC.
 * @details Bắt đầu quá trình chuyển đổi cho tất cả các kênh trong nhóm.
 * @param[in] Group ID của nhóm kênh ADC.
 * @return None
 **********************************************************/
void Adc_StartGroupConversion(Adc_GroupType Group);

/**********************************************************
 * @brief Dừng quá trình chuyển đổi nhóm ADC.
 * @details Dừng quá trình chuyển đổi của nhóm kênh ADC được chỉ định.
 * @param[in] Group ID của nhóm kênh ADC.
 * @return None
 **********************************************************/
void Adc_StopGroupConversion(Adc_GroupType Group);

/**********************************************************
 * @brief Đọc kết quả chuyển đổi của nhóm ADC.
 * @details Trả về giá trị kết quả của các kênh trong nhóm ADC.
 * @param[in] Group ID của nhóm kênh ADC.
 * @param[out] DataBufferPtr Con trỏ tới bộ đệm lưu trữ kết quả.
 * @return Std_ReturnType Trả về E_OK nếu đọc thành công, E_NOT_OK nếu thất bại.
 **********************************************************/
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr);

/**********************************************************
 * @brief Bật kích hoạt phần cứng cho nhóm ADC.
 * @details Kích hoạt nguồn phần cứng cho nhóm kênh được chỉ định.
 * @param[in] Group ID của nhóm kênh ADC.
 * @return None
 **********************************************************/
void Adc_EnableHardwareTrigger(Adc_GroupType Group);

/**********************************************************
 * @brief Tắt kích hoạt phần cứng cho nhóm ADC.
 * @details Vô hiệu hóa nguồn phần cứng cho nhóm kênh được chỉ định.
 * @param[in] Group ID của nhóm kênh ADC.
 * @return None
 **********************************************************/
void Adc_DisableHardwareTrigger(Adc_GroupType Group);

/**********************************************************
 * @brief Bật thông báo cho nhóm ADC.
 * @details Kích hoạt thông báo nhóm ADC.
 * @param[in] Group ID của nhóm kênh ADC.
 * @return None
 **********************************************************/
void Adc_EnableGroupNotification(Adc_GroupType Group);

/**********************************************************
 * @brief Tắt thông báo cho nhóm ADC.
 * @details Vô hiệu hóa thông báo nhóm ADC.
 * @param[in] Group ID của nhóm kênh ADC.
 * @return None
 **********************************************************/
void Adc_DisableGroupNotification(Adc_GroupType Group);

/**********************************************************
 * @brief Lấy trạng thái nhóm ADC.
 * @details Trả về trạng thái hiện tại của nhóm ADC (IDLE, BUSY, hoặc COMPLETED).
 * @param[in] Group ID của nhóm kênh ADC.
 * @return Adc_StatusType Trạng thái của nhóm kênh ADC.
 **********************************************************/
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group);

/**********************************************************
 * @brief Lấy con trỏ cuối cùng của buffer trong chế độ streaming.
 * @details Trả về địa chỉ của mẫu cuối cùng được ghi vào bộ đệm của nhóm kênh.
 * @param[in] Group ID của nhóm kênh ADC.
 * @param[out] PtrToSampleAddress Con trỏ tới địa chỉ của mẫu cuối cùng.
 * @return Std_ReturnType Trả về E_OK nếu thành công, E_NOT_OK nếu thất bại.
 **********************************************************/
Std_ReturnType Adc_GetStreamLastPointer(Adc_GroupType Group, Adc_ValueGroupType **PtrToSampleAddress);

/**********************************************************
 * @brief Lấy thông tin phiên bản của mô-đun ADC.
 * @details Hàm này trả về thông tin phiên bản của mô-đun ADC.
 * @param[out] VersionInfo Con trỏ tới cấu trúc chứa thông tin phiên bản.
 * @return None
 **********************************************************/

void Adc_GetVersionInfo(Std_VersionInfoType *VersionInfo);

/**********************************************************
 * @brief Thiết lập trạng thái nguồn cho nhóm ADC.
 * @details Hàm này thiết lập trạng thái nguồn (bật, tắt, tiết kiệm năng lượng) cho nhóm ADC.
 * @param[in] Group ID của nhóm kênh ADC.
 * @param[in] PowerState Trạng thái nguồn cần thiết lập.
 * @return Std_ReturnType Trả về E_OK nếu thành công, E_NOT_OK nếu thất bại.
 **********************************************************/
Std_ReturnType Adc_SetPowerState(Adc_GroupType Group, Adc_PowerStateType PowerState);

#endif // ADC_H
