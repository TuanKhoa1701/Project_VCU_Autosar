/***************************************************************************
 * @file DIO.c
 * @brief têp thực hiện cho module DIO trong AUTOSAR.
 * @details Têp này chứa các hàm để đọc và ghi các kênh GPIO, cũng như các hàm để quản lý nhóm kênh và cổng.
 * @version 1.0.0
 ****************************************************************************/

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "DIO.h"
/***************************************************************************
 * @brief Hàm để ghi mức độ của một kênh DIO.
 * @details Hàm này nhận vào ID của kênh và mức độ cần ghi (STD_HIGH hoặc STD_LOW).
 *          Nó sẽ xác định cổng GPIO tương ứng với kênh và ghi mức độ vào chân GPIO tương ứng.
 * @param[in] ChannelId ID của kênh DIO cần ghi.
 * @param[in] Level Mức độ cần ghi (STD_HIGH hoặc STD_LOW).
 ***************************************************************************/

void DIO_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level)
{
    GPIO_TypeDef *GPIO_Port;
    uint16_t GIPO_Pin;

    GPIO_Port = GPIO_GetPort(ChannelId);
    if (GPIO_Port == NULL)
    {
        Det_ReportError(DIO_MODULE_ID, 0, DIO_WRITECHANNEL_ID, DIO_E_PARAM_INVALID_CHANNEL);
        return; // Handle error appropriately
    }
    GIPO_Pin = GPIO_GetPin(ChannelId);

    if (Level == STD_HIGH)
    {
        GPIO_SetBits(GPIO_Port, GIPO_Pin);
    }
    else
    {
        GPIO_ResetBits(GPIO_Port, GIPO_Pin);
    }
}

/***************************************************************************
 * @brief Hàm để đọc mức độ của một kênh DIO.
 * @details Hàm này nhận vào ID của kênh và trả về mức độ hiện tại của chân GPIO tương ứng.
 * @param[in] ChannelId ID của kênh DIO cần đọc.
 * @return Mức độ hiện tại của kênh (STD_HIGH hoặc STD_LOW).
 ****************************************************************************/

Dio_LevelType DIO_ReadChannel(Dio_ChannelType ChannelId)
{
    GPIO_TypeDef *GPIO_Port;
    uint16_t GIPO_Pin;

    GPIO_Port = GPIO_GetPort(ChannelId);
    if (GPIO_Port == NULL)
    {
        Det_ReportError(DIO_MODULE_ID, 0, DIO_READCHANNEL_ID, DIO_E_PARAM_INVALID_CHANNEL);
        return ; 
    }

    GIPO_Pin = GPIO_GetPin(ChannelId);

    if (GPIO_ReadInputDataBit(GPIO_Port, GIPO_Pin) == STD_HIGH)
    {
         return STD_HIGH;
    }
    else
    {
        return STD_LOW;
    }
}
/***************************************************************************
 * @brief Hàm để đọc mức độ của một cổng DIO.
 * @details Hàm này nhận vào ID của cổng và trả về mức độ hiện tại của toàn bộ cổng GPIO.
 * @param[in] PortId ID của cổng DIO cần đọc.
 * @return Mức độ hiện tại của cổng (Dio_PortLevelType).
 ***************************************************************************/

Dio_PortLevelType DIO_ReadPort(Dio_PortType PortId)
{
    GPIO_TypeDef *GPIO_Port;

    GPIO_Port = GPIO_GetPort(PortId);
    if (GPIO_Port == NULL)
    {
        return 0; // Return a default value or handle error appropriately
    }

    return (Dio_PortLevelType)GPIO_ReadInputData(GPIO_Port);
}
/***************************************************************************
 * @brief Hàm để ghi mức độ của một cổng DIO.
 * @details Hàm này nhận vào ID của cổng và mức độ cần ghi (Dio_PortLevelType).
 *          Nó sẽ xác định cổng GPIO tương ứng với cổng và ghi mức độ vào toàn bộ cổng GPIO.
 * @param[in] PortId ID của cổng DIO cần ghi.
 * @param[in] Level Mức độ cần ghi (Dio_PortLevelType).
 ***************************************************************************/

void DIO_WritePort(Dio_PortType PortId, Dio_PortLevelType Level)
{
    GPIO_TypeDef *GPIO_Port;

    GPIO_Port = GPIO_GetPort(PortId);
    if (GPIO_Port == NULL)
    {
        Det_ReportError(DIO_MODULE_ID, 0, DIO_WRITEPORT_ID, DIO_E_PARAM_INVALID_PORT);
        return; // Handle error appropriately
    }

    GPIO_Write(GPIO_Port, Level);
}

/***************************************************************************
 * @brief Hàm để đọc mức độ của một nhóm kênh DIO.
 * @details Hàm này nhận vào một con trỏ đến cấu trúc Dio_ChannelGroupType, xác định cổng GPIO, mặt nạ và độ lệch của nhóm kênh.
 *          Nó sẽ đọc mức độ của các kênh trong nhóm và trả về giá trị mức độ của nhóm kênh.
 * @param[in] ChannelGroupIdPtr Con trỏ đến cấu trúc Dio_ChannelGroupType chứa thông tin về nhóm kênh.
 * @return Mức độ của nhóm kênh (Dio_PortLevelType).
 ****************************************************************************/
Dio_PortLevelType DIO_ReadChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr)
{
    GPIO_TypeDef *GPIO_Port;
    uint16_t mask;
    uint8_t offset;

    if (ChannelGroupIdPtr == NULL)
    {
        return 0; // Return a default value or handle error appropriately
    }

    GPIO_Port = GPIO_GetPort(ChannelGroupIdPtr->port);
    if (GPIO_Port == NULL)
    {
        return 0; // Return a default value or handle error appropriately
    }

    mask = ChannelGroupIdPtr->mask;
    offset = ChannelGroupIdPtr->offset;

    return (Dio_PortLevelType)((GPIO_ReadInputData(GPIO_Port) & mask) >> offset);
}
/****************************************************************************
 * @brief Hàm để ghi mức độ của một nhóm kênh DIO.
 * @details Hàm này nhận vào một con trỏ đến cấu trúc Dio_ChannelGroupType, xác định cổng GPIO, mặt nạ và độ lệch của nhóm kênh.
 *          Nó sẽ ghi mức độ vào các kênh trong nhóm theo mặt nạ và độ lệch đã chỉ định.
 * @param[in] ChannelGroupIdPtr Con trỏ đến cấu trúc Dio_ChannelGroupType chứa thông tin về nhóm kênh.
 * @param[in] Level Mức độ cần ghi (Dio_PortLevelType).
 * ****************************************************************************/

void DIO_WriteChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr, Dio_PortLevelType Level)
{
    GPIO_TypeDef *GPIO_Port;
    uint16_t mask;
    uint8_t offset;

    if (ChannelGroupIdPtr == NULL)
    {
        return; // Handle error appropriately
    }

    GPIO_Port = GPIO_GetPort(ChannelGroupIdPtr->port);
    if (GPIO_Port == NULL)
    {
        return; // Handle error appropriately
    }

    mask = ChannelGroupIdPtr->mask;
    offset = ChannelGroupIdPtr->offset;

    Dio_PortLevelType currentLevel = GPIO_ReadInputData(GPIO_Port) & ~mask;
    currentLevel |= (Level << offset) & mask;

    GPIO_Write(GPIO_Port, currentLevel);
}

/***************************************************************************
 * @brief Hàm để lấy thông tin phiên bản của module DIO.
 * @details Hàm này điền thông tin phiên bản vào cấu trúc Std_VersionInfoType.
 * @param[out] versioninfo Con trỏ đến cấu trúc Std_VersionInfoType để lưu thông tin phiên bản.
 ***************************************************************************/

void DIO_GetVersionInfo(Std_VersionInfoType *versioninfo)
{
    if (versioninfo == NULL)
    {
        return; // Handle error appropriately
    }

    versioninfo->vendorID = DIO_VENDOR_ID;
    versioninfo->moduleID = DIO_MODULE_ID;
    versioninfo->sw_major_version = STD_TYPES_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = STD_TYPES_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = STD_TYPES_SW_PATCH_VERSION;
}

/***************************************************************************
 * @brief Hàm để lật trạng thái của một kênh DIO.
 * @details Hàm này nhận vào ID của kênh và lật trạng thái của chân GPIO tương ứng (STD_HIGH <-> STD_LOW).
 * @param[in] ChannelId ID của kênh DIO cần lật trạng thái.
 * @return Trạng thái mới của kênh (STD_HIGH hoặc STD_LOW).
 ***************************************************************************/

Dio_LevelType DIO_FlipChannel(Dio_ChannelType ChannelId)
{
    GPIO_TypeDef *GPIO_Port;
    uint16_t GIPO_Pin;

    GPIO_Port = GPIO_GetPort(ChannelId);
    if (GPIO_Port == NULL)
    {
        return STD_LOW; // Return a default value or handle error appropriately
    }
    GIPO_Pin = GPIO_GetPin(ChannelId);

    if (GPIO_ReadInputDataBit(GPIO_Port, GIPO_Pin) == STD_HIGH)
    {
        DIO_WriteChannel(ChannelId, STD_LOW);
        return STD_LOW;
    }
    else
    {
        DIO_WriteChannel(ChannelId, STD_HIGH);
        return STD_HIGH;
    }
}

/***************************************************************************
 * @brief Hàm để ghi mức độ của một cổng DIO với mặt nạ.
 * @details Hàm này nhận vào ID của cổng, mức độ cần ghi và mặt nạ để xác định các chân cần ghi.
 *          Nó sẽ ghi mức độ vào các chân tương ứng với mặt nạ đã chỉ định.
 * @param[in] PortId ID của cổng DIO cần ghi.
 * @param[in] Level Mức độ cần ghi (Dio_PortLevelType).
 * @param[in] Mask Mặt nạ để xác định các chân cần ghi.
 ***************************************************************************/

void DIO_MaskedWritePort(Dio_PortType PortId, Dio_PortLevelType Level, Dio_PortLevelType Mask)
{
    GPIO_TypeDef *GPIO_Port;

    GPIO_Port = GPIO_GetPort(PortId);
    if (GPIO_Port == NULL)
    {
        return; // Handle error appropriately
    }

    Dio_PortLevelType currentLevel = GPIO_ReadInputData(GPIO_Port) & ~Mask;
    currentLevel |= (Level & Mask);

    GPIO_Write(GPIO_Port, currentLevel);
}
