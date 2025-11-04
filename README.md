# Dự án VCU Debugger với RTOS theo kiến trúc AUTOSAR

Đây là một dự án hệ thống nhúng mô phỏng Bộ điều khiển phương tiện (Vehicle Control Unit - VCU), được xây dựng trên một hệ điều hành thời gian thực (RTOS) tự phát triển theo tiêu chuẩn OSEK/AUTOSAR. Dự án áp dụng kiến trúc phần mềm phân lớp chặt chẽ để tối đa hóa tính module, khả năng tái sử dụng và đơn giản hóa việc bảo trì, nâng cấp.

## Kiến trúc phần mềm

Dự án tuân theo kiến trúc phân lớp của **AUTOSAR Classic Platform**, bao gồm các lớp chính từ trên xuống dưới:

*   **SWC (Software Components):** Lớp ứng dụng, chứa logic nghiệp vụ của hệ thống.
*   **RTE (Runtime Environment):** Lớp trung gian, đóng vai trò là "keo" kết dính các SWC với nhau và với lớp phần mềm cơ sở (BSW).
*   **BSW (Basic Software):** Lớp phần mềm cơ sở, được chia thành nhiều lớp con:
    *   **Services Layer:** Cung cấp các dịch vụ cơ bản như Hệ điều hành (OS).
    *   **ECU Abstraction Layer (ECUAL):** Trừu tượng hóa phần cứng trên bo mạch (ECU), cung cấp giao diện đồng nhất cho các cảm biến, cơ cấu chấp hành.
    *   **Microcontroller Abstraction Layer (MCAL):** Cung cấp các driver cấp thấp để truy cập trực tiếp vào các ngoại vi của vi điều khiển.

## Cấu trúc thư mục

Cấu trúc thư mục của dự án được tổ chức như sau để phản ánh kiến trúc phân lớp:

```
Project_OSEK_VCU_Debugger/
├── bsw/
│   ├── communication/
│   │   └── canif/
│   ├── mcal/
│   │   ├── adc/
│   │   └── ... (pwm, dio, can, etc.)
│   └── services/
│       └── os/
├── rte/
│   └── core/
└── swc/
    ├── Swc_SafetyManager/
    └── Swc_S

```

### 1. `bsw` (Basic Software)

Thư mục này chứa các module phần mềm cơ sở, được chia thành các lớp con:

*   **`mcal` (Microcontroller Abstraction Layer):**
    *   **Mục đích:** Cung cấp các driver cấp thấp để tương tác trực tiếp với phần cứng của vi điều khiển (STM32F103). Lớp này giúp trừu tượng hóa phần cứng, làm cho các lớp bên trên độc lập với vi điều khiển cụ thể.
    *   **Ví dụ:** `adc/` (điều khiển ADC), `pwm/` (điều khiển PWM), `dio/` (điều khiển GPIO).

*   **`services` (Services Layer):**
    *   **Mục đích:** Cung cấp các dịch vụ nền tảng cho hệ thống, không phụ thuộc vào vi điều khiển.
    *   **Ví dụ:** `os/` chứa mã nguồn của hệ điều hành thời gian thực (quản lý Task, Event, Alarm, Scheduler...).

*   **`communication` (Communication Stack):**
    *   **Mục đích:** Chứa các module liên quan đến giao tiếp, ví dụ như ngăn xếp CAN.
    *   **Ví dụ:** `canif/` (CAN Interface), module giao diện giữa lớp trên (PduR) và driver CAN (CanDrv).

### 2. `rte` (Runtime Environment)

*   **Mục đích:** Là lớp trung gian được sinh ra tự động (trong AUTOSAR chuẩn) hoặc viết thủ công để kết nối các thành phần phần mềm (SWC). Nó quản lý việc trao đổi dữ liệu (Sender/Receiver) và gọi hàm (Client/Server) giữa các SWC và giữa SWC với BSW.
*   **Nội dung:** Chứa các file header (`Rte_Types.h`) định nghĩa kiểu dữ liệu chung và các file mã nguồn thực hiện việc định tuyến dữ liệu.

### 3. `swc` (Software Components)

*   **Mục đích:** Đây là nơi chứa logic "não bộ" của ứng dụng. Mỗi thư mục con là một thành phần phần mềm độc lập, thực hiện một chức năng nghiệp vụ cụ thể.
*   **Ví dụ:** `Swc_SafetyManager/` là một SWC chịu trách nhiệm thu thập dữ liệu từ các cảm biến, áp dụng các quy tắc an toàn (ví dụ: kiểm tra điều kiện chuyển số, ưu tiên phanh) và xuất ra các quyết định an toàn cho hệ thống.

## Các thành phần chính đã hiện thực

*   **Hệ điều hành (OS):**
    *   Scheduler preemptive dựa trên độ ưu tiên.
    *   Hỗ trợ Task (Basic & Extended), Event, Alarm, Resource.
    *   Sử dụng SysTick cho time base 1ms và PendSV cho chuyển đổi ngữ cảnh.
*   **Driver (MCAL):**
    *   `Adc`: Driver đọc giá trị analog, hỗ trợ DMA.
*   **Giao tiếp (Communication):**
    *   `CanIf`: Module giao diện CAN, quản lý việc truyền/nhận PDU.
*   **Ứng dụng (SWC):**
    *   `Swc_SafetyManager`: Module giám sát và áp đặt các ràng buộc an toàn cho xe.

---
*Tác giả: Nguyễn Tuấn Khoa*
