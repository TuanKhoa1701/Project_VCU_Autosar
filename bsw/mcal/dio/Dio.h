/*******************************************************
 * @file: DIO.H
 * @brief: Header file for Digital Input/Output (DIO) module in AUTOSAR.
 * @details: Đây là tệp tiêu đề cho module DIO trong AUTOSAR, định nghĩa các kiểu dữ liệu và hàm liên quan đến việc đọc và ghi các kênh GPIO.
 * @version: 1.0.0
 * @author: Nguyen Tuan Khoa
 ********************************************************/

#ifndef DIO_H
#define DIO_H

// This file is part of the AUTOSAR standard.
#include "Std_Types.h"

/*******************************************************
 * ========================================================
 * DIO Port Definitions
 * ========================================================
 * Macro xác định cổng GPIO dựa trên ID kênh.
 ********************************************************/

#define GPIO_GetPort(ChannelId)                          \
    ((ChannelId) < 16 ? GPIOA : (ChannelId) < 32 ? GPIOB \
                            : (ChannelId) < 48   ? GPIOC \
                            : (ChannelId) < 64   ? GPIOD \
                            : (ChannelId) < 80   ? GPIOE \
                            : (ChannelId) < 96   ? GPIOF \
                            : (ChannelId) < 112  ? GPIOG \
                                                 : NULL)

/**********************************************************
 * ========================================================
 * DIO Pin Definitions
 * ========================================================
 * Macro xác định chân GPIO dựa trên ID kênh
 **********************************************************/

#define GPIO_GetPin(ChannelId) (1 << (ChannelId) % 16)

/****************************************************
 * ========================================================
 * DIO Channel Definitions
 * ========================================================
 * Macro xác định ID kênh DIO dựa trên cổng GPIO và chân.
 ********************************************************/

#define DIO_CHANNEL(GPIO_Port, Pin) \
    ((GPIO_Port) * 16 + (Pin))

/**********************************************************
 * ========================================================
 * DIO Port Definitions
 * ========================================================
 * Các định nghĩa cổng DIO.
 **********************************************************/

#define DIO_PORT_A 0
#define DIO_PORT_B 1
#define DIO_PORT_C 2
#define DIO_PORT_D 3

/*******************************************************
 * ========================================================
 * DIO Channel Definitions for Specific Channels
 * ========================================================
 * Các định nghĩa kênh DIO cụ thể cho từng chân GPIO.
 ********************************************************/

#define DIO_CHANNEL_A0 (DIO_CHANNEL(DIO_PORT_A, 0))
#define DIO_CHANNEL_A1 (DIO_CHANNEL(DIO_PORT_A, 1))
#define DIO_CHANNEL_A2 (DIO_CHANNEL(DIO_PORT_A, 2))
#define DIO_CHANNEL_A3 (DIO_CHANNEL(DIO_PORT_A, 3))
#define DIO_CHANNEL_A4 (DIO_CHANNEL(DIO_PORT_A, 4))
#define DIO_CHANNEL_A5 (DIO_CHANNEL(DIO_PORT_A, 5))
#define DIO_CHANNEL_A6 (DIO_CHANNEL(DIO_PORT_A, 6))
#define DIO_CHANNEL_A7 (DIO_CHANNEL(DIO_PORT_A, 7))
#define DIO_CHANNEL_A8 (DIO_CHANNEL(DIO_PORT_A, 8))
#define DIO_CHANNEL_A9 (DIO_CHANNEL(DIO_PORT_A, 9))
#define DIO_CHANNEL_A10 (DIO_CHANNEL(DIO_PORT_A, 10))
#define DIO_CHANNEL_A11 (DIO_CHANNEL(DIO_PORT_A, 11))
#define DIO_CHANNEL_A12 (DIO_CHANNEL(DIO_PORT_A, 12))
#define DIO_CHANNEL_A13 (DIO_CHANNEL(DIO_PORT_A, 13))
#define DIO_CHANNEL_A14 (DIO_CHANNEL(DIO_PORT_A, 14))
#define DIO_CHANNEL_A15 (DIO_CHANNEL(DIO_PORT_A, 15))

#define DIO_CHANNEL_B0 (DIO_CHANNEL(DIO_PORT_B, 0))
#define DIO_CHANNEL_B1 (DIO_CHANNEL(DIO_PORT_B, 1))
#define DIO_CHANNEL_B2 (DIO_CHANNEL(DIO_PORT_B, 2))
#define DIO_CHANNEL_B3 (DIO_CHANNEL(DIO_PORT_B, 3))
#define DIO_CHANNEL_B4 (DIO_CHANNEL(DIO_PORT_B, 4))
#define DIO_CHANNEL_B5 (DIO_CHANNEL(DIO_PORT_B, 5))
#define DIO_CHANNEL_B6 (DIO_CHANNEL(DIO_PORT_B, 6))
#define DIO_CHANNEL_B7 (DIO_CHANNEL(DIO_PORT_B, 7))
#define DIO_CHANNEL_B8 (DIO_CHANNEL(DIO_PORT_B, 8))
#define DIO_CHANNEL_B9 (DIO_CHANNEL(DIO_PORT_B, 9))
#define DIO_CHANNEL_B10 (DIO_CHANNEL(DIO_PORT_B, 10))
#define DIO_CHANNEL_B11 (DIO_CHANNEL(DIO_PORT_B, 11))
#define DIO_CHANNEL_B12 (DIO_CHANNEL(DIO_PORT_B, 12))
#define DIO_CHANNEL_B13 (DIO_CHANNEL(DIO_PORT_B, 13))
#define DIO_CHANNEL_B14 (DIO_CHANNEL(DIO_PORT_B, 14))
#define DIO_CHANNEL_B15 (DIO_CHANNEL(DIO_PORT_B, 15))

#define DIO_CHANNEL_C0 (DIO_CHANNEL(DIO_PORT_C, 0))
#define DIO_CHANNEL_C1 (DIO_CHANNEL(DIO_PORT_C, 1))
#define DIO_CHANNEL_C2 (DIO_CHANNEL(DIO_PORT_C, 2))
#define DIO_CHANNEL_C3 (DIO_CHANNEL(DIO_PORT_C, 3))
#define DIO_CHANNEL_C4 (DIO_CHANNEL(DIO_PORT_C, 4))
#define DIO_CHANNEL_C5 (DIO_CHANNEL(DIO_PORT_C, 5))
#define DIO_CHANNEL_C6 (DIO_CHANNEL(DIO_PORT_C, 6))
#define DIO_CHANNEL_C7 (DIO_CHANNEL(DIO_PORT_C, 7))
#define DIO_CHANNEL_C8 (DIO_CHANNEL(DIO_PORT_C, 8))
#define DIO_CHANNEL_C9 (DIO_CHANNEL(DIO_PORT_C, 9))
#define DIO_CHANNEL_C10 (DIO_CHANNEL(DIO_PORT_C, 10))
#define DIO_CHANNEL_C11 (DIO_CHANNEL(DIO_PORT_C, 11))
#define DIO_CHANNEL_C12 (DIO_CHANNEL(DIO_PORT_C, 12))
#define DIO_CHANNEL_C13 (DIO_CHANNEL(DIO_PORT_C, 13))
#define DIO_CHANNEL_C14 (DIO_CHANNEL(DIO_PORT_C, 14))
#define DIO_CHANNEL_C15 (DIO_CHANNEL(DIO_PORT_C, 15))

#define DIO_CHANNEL_D0 (DIO_CHANNEL(DIO_PORT_D, 0))
#define DIO_CHANNEL_D1 (DIO_CHANNEL(DIO_PORT_D, 1))
#define DIO_CHANNEL_D2 (DIO_CHANNEL(DIO_PORT_D, 2))
#define DIO_CHANNEL_D3 (DIO_CHANNEL(DIO_PORT_D, 3))
#define DIO_CHANNEL_D4 (DIO_CHANNEL(DIO_PORT_D, 4))
#define DIO_CHANNEL_D5 (DIO_CHANNEL(DIO_PORT_D, 5))
#define DIO_CHANNEL_D6 (DIO_CHANNEL(DIO_PORT_D, 6))
#define DIO_CHANNEL_D7 (DIO_CHANNEL(DIO_PORT_D, 7))
#define DIO_CHANNEL_D8 (DIO_CHANNEL(DIO_PORT_D, 8))
#define DIO_CHANNEL_D9 (DIO_CHANNEL(DIO_PORT_D, 9))
#define DIO_CHANNEL_D10 (DIO_CHANNEL(DIO_PORT_D, 10))
#define DIO_CHANNEL_D11 (DIO_CHANNEL(DIO_PORT_D, 11))
#define DIO_CHANNEL_D12 (DIO_CHANNEL(DIO_PORT_D, 12))
#define DIO_CHANNEL_D13 (DIO_CHANNEL(DIO_PORT_D, 13))
#define DIO_CHANNEL_D14 (DIO_CHANNEL(DIO_PORT_D, 14))
#define DIO_CHANNEL_D15 (DIO_CHANNEL(DIO_PORT_D, 15))

/**********************************************************
 * ========================================================
 * Định nghĩa kiểu dữ liệu cho DIO Driver
 * ========================================================
 * @typedef Dio_ChannelType
 * @brief kieu dữ liệu đại diện cho một kênh DIO.
 * @details kiểu được định danh cho một pin cụ thể.
 **********************************************************/

typedef uint16_t Dio_ChannelType;

/**********************************************************
 * ========================================================
 * Định nghĩa kiểu dữ liệu cho Port DIO
 * ========================================================
 * @typedef Dio_PortType
 * @brief kieu dữ liệu đại diện cho một cổng DIO.
 * @details kiểu này được sử dụng để xác định cổng GPIO mà kênh DIO thuộc về.
 **********************************************************/

typedef uint8_t Dio_PortType;

/**********************************************************
 * ========================================================
 * Định nghĩa kiểu dữ liệu cho DIO Port Level
 * ========================================================
 * @typedef Dio_PortLevelType
 * @brief kieu dữ liệu đại diện cho mức độ của một cổng DIO.
 * @details kiểu này được sử dụng để đọc hoặc ghi mức độ của toàn bộ cổng GPIO.
 **********************************************************/

typedef uint8_t Dio_LevelType;

/**********************************************************
 * ========================================================
 * Định nghĩa kiểu dữ liệu cho DIO Channel Group
 * ========================================================
 * @typedef Dio_ChannelGroupType
 * @brief kiểu dữ liệu đại diện cho một nhóm kênh DIO.
 * @details kiểu này được sử dụng để xác định một nhóm các kênh DIO trong cùng một cổng.
 **********************************************************/

typedef struct
{
    Dio_PortType port;
    uint16_t mask;
    uint8_t offset;
} Dio_ChannelGroupType;

/**********************************************************
 * ========================================================
 * Định nghĩa kiểu dữ liệu cho DIO Port Level
 * ========================================================
 * @typedef Dio_PortLevelType
 * @brief kieu dữ liệu đại diện cho mức độ của một cổng DIO.
 * @details kiểu này được sử dụng để đọc hoặc ghi mức độ của toàn bộ cổng GPIO.
 **********************************************************/

typedef uint16_t Dio_PortLevelType;

/**
 * @brief Ghi một mức logic tới một kênh DIO.
 * @param[in] ChannelId ID của kênh DIO cần ghi.
 * @param[in] Level Mức logic cần ghi (STD_HIGH hoặc STD_LOW).
 */
void DIO_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level);

/**
 * @brief Đọc mức logic của một kênh DIO.
 * @param[in] ChannelId ID của kênh DIO cần đọc.
 * @return Mức logic hiện tại của kênh (STD_HIGH hoặc STD_LOW).
 */
Dio_LevelType DIO_ReadChannel(Dio_ChannelType ChannelId);

/**
 * @brief Đọc giá trị của toàn bộ một cổng DIO.
 * @param[in] PortId ID của cổng DIO cần đọc.
 * @return Giá trị hiện tại của cổng.
 */
Dio_PortLevelType DIO_ReadPort(Dio_PortType PortId);

/**
 * @brief Ghi một giá trị tới toàn bộ một cổng DIO.
 * @param[in] PortId ID của cổng DIO cần ghi.
 * @param[in] Level Giá trị cần ghi cho cổng.
 */
void DIO_WritePort(Dio_PortType PortId, Dio_PortLevelType Level);

/**
 * @brief Đọc giá trị của một nhóm kênh DIO.
 * @param[in] ChannelGroupIdPtr Con trỏ tới cấu trúc định nghĩa nhóm kênh (port, mask, offset).
 * @return Giá trị của nhóm kênh sau khi áp dụng mask và offset.
 */
Dio_PortLevelType DIO_ReadChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr);

/**
 * @brief Ghi một giá trị tới một nhóm kênh DIO.
 * @param[in] ChannelGroupIdPtr Con trỏ tới cấu trúc định nghĩa nhóm kênh (port, mask, offset).
 * @param[in] Level Giá trị cần ghi cho nhóm kênh.
 */
void DIO_WriteChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr, Dio_PortLevelType Level);

/**
 * @brief Lấy thông tin phiên bản của module DIO.
 * @param[out] versioninfo Con trỏ tới cấu trúc Std_VersionInfoType để nhận thông tin phiên bản.
 */
void DIO_GetVersionInfo(Std_VersionInfoType *versioninfo);

/**
 * @brief Đảo mức logic của một kênh DIO.
 * @details Đọc trạng thái hiện tại của kênh và ghi giá trị ngược lại.
 * @param[in] ChannelId ID của kênh DIO cần đảo.
 * @return Mức logic mới của kênh sau khi đảo.
 */
Dio_LevelType DIO_FlipChannel(Dio_ChannelType ChannelId);

/**
 * @brief Ghi một giá trị tới một cổng DIO thông qua một mặt nạ.
 * @details Chỉ các bit được set trong `Mask` mới bị ảnh hưởng.
 *          Thao tác này là read-modify-write.
 * @param[in] PortId ID của cổng DIO cần ghi.
 * @param[in] Level Giá trị cần ghi.
 * @param[in] Mask Mặt nạ xác định các bit sẽ được ghi.
 */
void DIO_MaskedWritePort(Dio_PortType PortId, Dio_PortLevelType Level, Dio_PortLevelType Mask);

#endif
