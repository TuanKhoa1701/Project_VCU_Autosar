/**********************************************************
 * @file    Os_Resource.c (hỗ trợ cho kernel preemption)
 * @brief   Resource (mutex tối giản) cho OS trên STM32F103 (Cortex-M3) 
 * @details Cung cấp 3 API khớp prototype:
 *            - void Resource_Init(OsResource* r)
 *            - void GetResource(OsResource* r)
 *            - void ReleaseResource(OsResource* r)
 *
 *          Đặc tả (rút gọn theo tinh thần AUTOSAR/OSEK):
 *            - Resource là mutex không hỗ trợ priority ceiling/boost.
 *            - GetResource KHÔNG trả mã lỗi → lấy Resource 
 *            - Không hỗ trợ re-entrant (nested lock cùng một task).
 *
 *          Đồng bộ:
 *            - Bảo vệ trường locked/owner trong critical section rất ngắn
 *              bằng __DISABLE_IRQ (tắt toàn bộ IRQ trong vài lệnh).
 *            - Chỉ tắt IRQ khi kiểm tra/ghi cờ; KHÔNG tắt IRQ khi chờ,
 *              nhằm tránh làm đơ SysTick và các ISR khác.
 *
 * @version  1.1
 * @date     2025-09-10 
 * @author   Nguyễn Tuấn Khoa
 **********************************************************/

#include "Os.h"
#include "Os_Arch.h"

/* ===== Giá trị owner “không hợp lệ” để đánh dấu resource rỗi ===== */
#ifndef OS_INVALID_TASK_OWNER
#define OS_INVALID_TASK_OWNER ((TaskType)0xFFu)
#endif

extern TCB_t*g_current;
extern TCB_t tcb[OS_MAX_TASKS];
/* =========================================================
 * Resource_Init
 *  - Đưa resource về trạng thái rỗi (unlocked, owner = invalid)
 * =======================================================*/
void Resource_Init(OsResource* r)
{
  if (!r) return;
  r->locked = 0u;
  r->owner  = OS_INVALID_TASK_OWNER;
}
/* =========================================================
 * GetResource
 *  - Chiếm mutex theo kiểu “đợi bận có nhường CPU”.
 *  - Không gọi được từ ISR (thiết kế OS này hướng tới dùng trong Task).
 *  - Không hỗ trợ re-entrant: gọi lặp từ cùng một task là sai cách dùng.
 * =======================================================*/
void GetResource(OsResource* r)
{
  if (!r) return;

    /* Lấy danh tính task hiện hành (nếu chưa có task chạy thì thoát) */

    TaskType me = g_current->id;
    TCB_t *t= &tcb[g_current->id];
    /* Thử chiếm trong vùng găng rất ngắn */
    __disable_irq();
    if (r->locked == 0u) {
      /* Tài nguyên đang rỗi → chiếm ngay */
      r->locked = 1u;
      r->owner  = me;

      if(t->prio < r->ceilingPrio){
        t->prio = r->ceilingPrio;
      }
    __enable_irq();
      return;
    }

    /* Đang bị giữ: nếu chính mình giữ thì KHÔNG hỗ trợ nested → rời vòng
       (giữ nguyên trạng thái). Có thể đổi sang assert nếu muốn phát hiện sớm. */
    if (r->owner == me) {
      __enable_irq();
      return; /* re-entrant không được hỗ trợ, coi như “đã giữ” */
    }

    __enable_irq();
}


/* =========================================================
 * ReleaseResource
 *  - Nhả mutex nếu và chỉ nếu chủ sở hữu hiện tại là task đang chạy.
 *  - Không hỗ trợ nested lock → 1 lần Get ↔ 1 lần Release.
 * =======================================================*/
void ReleaseResource(OsResource* r)
{
  if (!r) return;
 
  TaskType me = g_current->id;

  __disable_irq();

  if ((r->locked != 0u) && (r->owner == me)) {
    r->locked = 0u;
    r->owner  = OS_INVALID_TASK_OWNER;
  }
  /* Nếu không phải chủ sở hữu hiện tại → bỏ qua (không làm gì). */

  __enable_irq();
}
