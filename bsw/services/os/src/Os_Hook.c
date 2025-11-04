/****************************************************************************************
 * @file    Os_Hook.c
 * @brief   Triển khai mặc định (yếu) cho các hook của OS
 * @details
 *  File này cung cấp các hàm hook rỗng với thuộc tính `__attribute__((weak))`.
 *  Điều này cho phép OS có thể được biên dịch độc lập, nhưng vẫn cho phép
 *  ứng dụng định nghĩa lại (override) các hook này bằng cách cung cấp một
 *  hàm cùng tên (strong symbol) ở bất kỳ đâu trong mã nguồn ứng dụng.
 *
 *  Các hook được OS gọi tại các thời điểm quan trọng trong vòng đời hệ thống:
 *    - StartupHook:  Được gọi một lần trong `StartOS` trước khi task đầu tiên chạy.
 *                    Lý tưởng để khởi tạo driver, module BSW.
 *    - ShutdownHook: Được gọi một lần trong `ShutdownOS` trước khi hệ thống dừng.
 *                    Dùng để dọn dẹp, đưa phần cứng về trạng thái an toàn.
 *    - ErrorHook:    Được gọi khi OS phát hiện lỗi nghiêm trọng.
 *                    Dùng để log lỗi, debug hoặc kích hoạt cơ chế phục hồi.
 *    - PreTaskHook:  Được gọi ngay trước khi một task được chuyển vào trạng thái RUNNING.
 *    - PostTaskHook: Được gọi ngay sau khi một task rời khỏi trạng thái RUNNING.
 *
 * @version  1.1
 * @date     2025-09-10
 * @author   Nguyễn Tuấn Khoa
 ****************************************************************************************/

#include "Os.h"
__attribute__((weak)) void StartupHook(void){}
__attribute__((weak)) void ShutdownHook(StatusType e){ (void)e; }
__attribute__((weak)) void ErrorHook(StatusType e){ (void)e; }
__attribute__((weak)) void PreTaskHook(void){}
__attribute__((weak)) void PostTaskHook(void){}
