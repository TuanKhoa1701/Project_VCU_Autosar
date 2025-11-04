/**********************************************************
 * @file    Port.c
 * @brief   Port Driver Source File (AUTOSAR Standard)
 * @details File này hiện thực các API chuẩn AUTOSAR cho Port Driver,
 *          dành cho STM32F103 sử dụng SPL. Driver cho phép cấu hình,
 *          đổi mode, đổi chiều, refresh và đọc thông tin version của port/pin.
 *
 * @version 1.0
 * @date    2025-06-19
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

/* ===============================
 *           Includes
 * =============================== */
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include <stddef.h>
#include "Port.h"
#include "Port_Cfg.h"
/* ===============================
 *     Static/Internal Variables
 * =============================== */
static uint8_t Port_Initialized = 0; /* Biến trạng thái xác định Port đã init chưa */

/* Kích thước mảng runtime nên đủ lớn cho mọi cấu hình, không phụ thuộc vào số pin hiện tại */
#ifndef PORT_MAX_CONFIGURABLE_PINS
#define PORT_MAX_CONFIGURABLE_PINS 16U
#endif
static Port_PinConfigType Port_RuntimePins[PORT_MAX_CONFIGURABLE_PINS];
/* ===============================
 *      Internal Helper Function
 * =============================== */

/**********************************************************
 * @brief Cấu hình 1 pin GPIO dựa trên thông số AUTOSAR
 * @param[in] pinCfg Con trỏ đến cấu trúc cấu hình pin
 **********************************************************/
static void Port_ApplyPinConfig(const Port_PinConfigType *pinCfg)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    uint16_t pinMask = PORT_GET_PIN_MASK(pinCfg->PinNum);

    /* Kích hoạt clock cho PORT */
    switch (pinCfg->PortNum)
    {
    case PORT_ID_A:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        break;
    case PORT_ID_B:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        break;
    case PORT_ID_C:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        break;
    case PORT_ID_D:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
        break;
    default:
        return;
    }

    GPIO_InitStruct.GPIO_Pin = pinMask;

    /* Cấu hình tốc độ chân GPIO */
    switch (pinCfg->Speed)
    {
    case PORT_SPEED_2Mhz:
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
        break;
    case PORT_SPEED_10Mhz:
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
        break;
    case PORT_SPEED_50Mhz:
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        break;
    default:
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
        break; // Mặc định 2MHz
    }

    switch (pinCfg->Mode)
    {
    case PORT_PIN_MODE_DIO:
        if (pinCfg->Direction == PORT_PIN_OUT)
        {
            GPIO_InitStruct.GPIO_Mode = (pinCfg->Pull == PORT_PIN_PULL_UP) ? GPIO_Mode_Out_PP : GPIO_Mode_Out_OD;
        }
        else
        {
            if (pinCfg->Pull == PORT_PIN_PULL_UP)
                GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
            else if (pinCfg->Pull == PORT_PIN_PULL_DOWN)
                GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
            else
                GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        }
        break;
    case PORT_PIN_MODE_PWM:
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP; // Chế độ Alternate Function Push Pull
        break;
    case PORT_PIN_MODE_SPI:
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP; // Chế độ Alternate Function Push Pull
        break;
    case PORT_PIN_MODE_ADC:
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN; // Chế độ Analog Input
        break;
    case PORT_PIN_MODE_CAN:
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP; // Chế độ Alternate Function Push Pull
        break;
    default:
        break;
    }
    /* Các mode khác như ADC, PWM, SPI ... mở rộng thêm tùy MCU */

    /* Khởi tạo chân */
    GPIO_Init(PORT_GET_PORT(pinCfg->PortNum), &GPIO_InitStruct);

    /* Nếu là output, set level ban đầu */
    if (pinCfg->Direction == PORT_PIN_OUT)
    {
        if (pinCfg->Level == PORT_PIN_LEVEL_HIGH)
            GPIO_SetBits(PORT_GET_PORT(pinCfg->PortNum), pinMask);
        else
            GPIO_ResetBits(PORT_GET_PORT(pinCfg->PortNum), pinMask);
    }
}

/* ===============================
 *     Function Definitions
 * =============================== */

/**********************************************************
 * @brief Khởi tạo toàn bộ các Port/Pin theo cấu hình
 * @details Hàm này sẽ gọi cấu hình từng pin theo bảng config.
 * @param[in] ConfigPtr Con trỏ đến cấu hình Port
 **********************************************************/
void Port_Init(const Port_ConfigType *ConfigPtr)
{
    if (ConfigPtr == NULL)
        return;

    /* Đảm bảo số pin cấu hình không vượt quá kích thước mảng runtime */
    if (ConfigPtr->PinCount > PORT_MAX_CONFIGURABLE_PINS) {
        /* Có thể báo lỗi ở đây, ví dụ qua DET (Development Error Tracer) */
        return;
    }
    for (uint16_t i = 0; i < ConfigPtr->PinCount; i++)
    {
        Port_RuntimePins[i] = ConfigPtr->PinConfigs[i];
        Port_ApplyPinConfig(&ConfigPtr->PinConfigs[i]);
    }
    Port_Initialized = 1;
}

/**********************************************************
 * @brief Đổi chiều một chân Port (nếu cho phép runtime)
 * @details
 * Hàm sẽ đổi chiều (IN/OUT) của pin, nếu cho phép ở config.
 * @param[in] Pin Số hiệu pin (0..n-1)
 * @param[in] Direction Chiều mong muốn
 **********************************************************/
void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction)
{
    if (!Port_Initialized)
        return;
    /* Sửa lỗi: Kiểm tra với kích thước mảng runtime và số pin đã cấu hình */
    if (Pin >= portConfig.PinCount || Pin >= PORT_MAX_CONFIGURABLE_PINS)
        return;
    /* Sửa lỗi: Truy cập cấu hình gốc thông qua portConfig thay vì biến extern trực tiếp */
    if (!portConfig.PinConfigs[Pin].DirectionChangeable)
        return;

    Port_RuntimePins[Pin].Direction = Direction;
    Port_ApplyPinConfig(&Port_RuntimePins[Pin]);
}

/**********************************************************
 * @brief Làm tươi lại chiều các pin không cho đổi runtime
 * @details Chỉ các pin cấu hình DirectionChangeable=0 sẽ được làm tươi lại chiều về giá trị config
 **********************************************************/
void Port_RefreshPortDirection(void)
{
    if (!Port_Initialized)
        return;
    for (uint16_t i = 0; i < portConfig.PinCount; i++)
    {
        if (!portConfig.PinConfigs[i].DirectionChangeable)
        {
            Port_ApplyPinConfig(&portConfig.PinConfigs[i]);
        }
    }
}

/**********************************************************
 * @brief Lấy thông tin phiên bản của Port Driver
 * @param[out] versioninfo Con trỏ đến Std_VersionInfoType để nhận version
 **********************************************************/
void Port_GetVersionInfo(Std_VersionInfoType *versioninfo)
{
    if (versioninfo == NULL)
        return;
    versioninfo->vendorID = PORT_VENDOR_ID;
    versioninfo->moduleID = PORT_MODULE_ID;
    versioninfo->sw_major_version = PORT_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = PORT_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = PORT_SW_PATCH_VERSION;
}

/**********************************************************
 * @brief Đổi mode chức năng của một chân pin (nếu cho phép runtime)
 * @param[in] Pin Số hiệu pin
 * @param[in] Mode Mode chức năng cần chuyển sang
 **********************************************************/
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode)
{
    if (!Port_Initialized)
        return;
    /* Sửa lỗi: Kiểm tra với kích thước mảng runtime và số pin đã cấu hình */
    if (Pin >= portConfig.PinCount || Pin >= PORT_MAX_CONFIGURABLE_PINS)
        return;
    /* Sửa lỗi: Truy cập cấu hình gốc thông qua portConfig */
    if (!portConfig.PinConfigs[Pin].ModeChangeable)
        return;

    /* SỬA LỖI LOGIC NGHIÊM TRỌNG: Không được ghi đè lên mảng const. Phải sửa trên mảng runtime. */
    Port_RuntimePins[Pin].Mode = Mode;
    Port_ApplyPinConfig(&Port_RuntimePins[Pin]);
}
