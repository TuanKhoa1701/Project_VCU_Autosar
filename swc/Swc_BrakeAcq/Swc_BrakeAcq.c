/**********************************************************
 * @file    Swc_BrakeAcq.c
 * @brief   SWC – Brake Acquisition (đọc & lọc tín hiệu phanh)
 * @details Luồng xử lý:
 *            1) Lấy mẫu trạng thái phanh thô từ IoHwAb qua RTE:
 *               Rte_Call_BrakeAcq_IoHwAb_Brake_Get(&raw).
 *            2) Debounce theo thời gian (bỏ qua nhiễu chập chờn).
 *            3) Khi trạng thái ổn định thay đổi → publish:
 *               Rte_Write_BrakeAcq_BrakeOut(stable).
 *
 *          Mặc định:
 *            - Chu kỳ gọi Run10ms: 10 ms.
 *            - Cửa sổ debounce: 20 ms (2 chu kỳ).
 *
 *          Ghi chú an toàn:
 *            - Không chặn lâu trong Run10ms; mọi gọi xuống IoHwAb là
 *              đồng bộ ngắn gọn.
 *            - Nếu IoHwAb không sẵn sàng, giữ nguyên trạng thái trước.
 *
 * @version  1.0
 * @date     2025-09-10 
 * @author   Nguyễn Tuấn Khoa
 **********************************************************/

#include "Swc_BrakeAcq.h"
#include "Rte.h"   /* Rte_Call_* & Rte_Write_* */

#ifndef BRAKEACQ_TASK_PERIOD_MS
#define BRAKEACQ_TASK_PERIOD_MS   (10u)  /* chu kỳ gọi Run10ms */
#endif

#ifndef BRAKEACQ_DEBOUNCE_MS
#define BRAKEACQ_DEBOUNCE_MS      (20u)  /* yêu cầu ổn định 20 ms */
#endif

/* Số mẫu cần liên tiếp giống nhau để coi là ổn định */
#define BRAKEACQ_DEBOUNCE_TICKS   ((uint8_t)((BRAKEACQ_DEBOUNCE_MS + (BRAKEACQ_TASK_PERIOD_MS-1u)) / BRAKEACQ_TASK_PERIOD_MS))

/* Trạng thái nội bộ cho thuật toán debounce */
typedef struct {
  boolean lastRaw;     /* mẫu vừa đọc (kỳ trước)       */
  boolean stable;      /* trạng thái đã ổn định        */
  uint8_t   cnt;         /* số lần liên tiếp cùng giá trị*/
  boolean inited;      /* đã seed lần đầu chưa         */
} BrakeAcq_State_t;

static BrakeAcq_State_t s_brake;

/* Seed giá trị ban đầu từ phần cứng (qua IoHwAb) để tránh “giật” lần 1 */
static void BrakeAcq_SeedFromHw(void)
{
  boolean raw = FALSE;
  if (Rte_Call_BrakeAcq_IoHwAb_Brake_Get(&raw) != E_OK) {
    raw = FALSE; /* fallback an toàn */
  }
  s_brake.lastRaw = raw;
  s_brake.stable  = raw;
  s_brake.cnt     = 0u;
  s_brake.inited  = TRUE;

  /* phát hành giá trị seed để các SWC khác có dữ liệu ngay */
  (void)Rte_Write_BrakeAcq_BrakeOut(s_brake.stable);
}

void Swc_BrakeAcq_Init(void)
{
  s_brake.lastRaw = FALSE;
  s_brake.stable  = FALSE;
  s_brake.cnt     = 0u;
  s_brake.inited  = FALSE;

  BrakeAcq_SeedFromHw();

  printf("BrakeAcq: init done, stable=%d\n", s_brake.stable);
}

void Swc_BrakeAcq_Run10ms(void)
{
  boolean raw = FALSE;

  /* 1) Lấy mẫu thô từ phần cứng (qua RTE → IoHwAb) */
  if (Rte_Call_BrakeAcq_IoHwAb_Brake_Get(&raw) != E_OK) {
    /* Không cập nhật khi IoHwAb lỗi; giữ nguyên trạng thái hiện hành */
    return;
  }

  /* 2) Debounce: nếu giống mẫu trước → tăng đếm; khác → reset đếm */
  if (raw == s_brake.lastRaw) {
    if (s_brake.cnt < 0xFFu) { s_brake.cnt++; }
  } else {
    s_brake.lastRaw = raw;
    s_brake.cnt     = 1u;  /* bắt đầu chuỗi mới */
  }

  /* 3) Khi đủ số mẫu liên tiếp và khác với stable → cập nhật & publish */
  if ((s_brake.cnt >= BRAKEACQ_DEBOUNCE_TICKS) && (raw != s_brake.stable)) {
    s_brake.stable = raw;
    (void)Rte_Write_BrakeAcq_BrakeOut(s_brake.stable);
    /* Giữ lastRaw/cnt để tiếp tục debounce chuỗi kế tiếp */
  }
}
