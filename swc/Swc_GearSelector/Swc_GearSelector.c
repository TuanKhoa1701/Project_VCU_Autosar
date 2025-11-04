/**********************************************************
 * @file    Swc_GearSelector.c
 * @brief   SWC – Gear Selector (đọc & quản lý vị trí cần số)
 * @details Luồng xử lý:
 *            1) Lấy mẫu vị trí số thô từ IoHwAb qua RTE:
 *               Rte_Call_GearSelector_IoHwAb_Gear_Get(&raw, &valid).
 *            2) Nếu valid == TRUE → debounce theo thời gian.
 *            3) Khi trạng thái ổn định thay đổi → publish:
 *               Rte_Write_GearSelector_GearOut(stable).
 *
 *          Mặc định:
 *            - Chu kỳ gọi Run10ms: 10 ms.
 *            - Cửa sổ debounce: 30 ms (3 chu kỳ).
 *
 *          Ghi chú:
 *            - Giá trị hợp lệ: GEAR_P/GEAR_R/GEAR_N/GEAR_D.
 *            - Nếu IoHwAb báo invalid → bỏ qua chu kỳ đó.
 *
 * @version 1.0
 * @date    2025-09-10
 * @author  Nguyễn tuấn Khoa
 **********************************************************/

#include "Swc_GearSelector.h"
#include "Rte.h"        /* Rte_Call_* & Rte_Write_* */
#include "Rte_Types.h"   /* Gear_e */

#ifndef GEARSEL_TASK_PERIOD_MS
#define GEARSEL_TASK_PERIOD_MS   (10u)  /* chu kỳ gọi Run10ms */
#endif

#ifndef GEARSEL_DEBOUNCE_MS
#define GEARSEL_DEBOUNCE_MS      (20u)  /* yêu cầu ổn định 30 ms */
#endif

/* Số mẫu cần liên tiếp giống nhau để coi là ổn định */
#define GEARSEL_DEBOUNCE_TICKS \
  ((uint8_t)((GEARSEL_DEBOUNCE_MS + (GEARSEL_TASK_PERIOD_MS - 1u)) / GEARSEL_TASK_PERIOD_MS))

/* Trạng thái nội bộ cho thuật toán debounce */
typedef struct {
  Gear_e  lastRaw;  /* mẫu vừa đọc (kỳ trước)       */
  Gear_e  stable;   /* vị trí số đã ổn định         */
  uint8_t   cnt;      /* số lần liên tiếp cùng giá trị*/
  boolean inited;   /* đã seed lần đầu chưa         */
} GearSel_State_t;

static GearSel_State_t s_gear;

/* Trả về TRUE nếu g là một trong các giá trị hợp lệ PRND */
static boolean gear_is_valid(Gear_e g)
{
  return (g == GEAR_P) || (g == GEAR_R) || (g == GEAR_N) || (g == GEAR_D);
}

/* Seed giá trị ban đầu từ phần cứng (qua IoHwAb) để tránh “giật” lần 1 */
static void GearSel_SeedFromHw(void)
{
  Gear_e  raw   = GEAR_P;
  boolean valid = FALSE;

  if (Rte_Call_GearSelector_IoHwAb_Gear_Get(&raw, &valid) != E_OK || valid == FALSE || !gear_is_valid(raw)) {
    raw = GEAR_P; /* fallback an toàn */
  }

  s_gear.lastRaw = raw;
  s_gear.stable  = raw;
  s_gear.cnt     = 0u;
  s_gear.inited  = TRUE;

  /* Publish giá trị seed để các SWC khác có dữ liệu ngay */
  (void)Rte_Write_GearSelector_GearOut(s_gear.stable);
}

void Swc_GearSelector_Init(void)
{
  s_gear.lastRaw = GEAR_P;
  s_gear.stable  = GEAR_P;
  s_gear.cnt     = 0u;
  s_gear.inited  = FALSE;

  GearSel_SeedFromHw();

  printf("GearSelector: init done, stable=%d\n", s_gear.stable);
}

void Swc_GearSelector_Run10ms(void)
{
  Gear_e  raw;
  boolean valid;

  /* 1) Lấy mẫu thô từ IoHwAb (qua RTE) */
  if (Rte_Call_GearSelector_IoHwAb_Gear_Get(&raw, &valid) != E_OK) {
    /* Không cập nhật khi IoHwAb lỗi; giữ nguyên trạng thái hiện hành */
    return;
  }
  if (valid == FALSE || !gear_is_valid(raw)) {
    /* Bỏ qua chu kỳ nếu dữ liệu không hợp lệ */
    return;
  }

  /* 2) Debounce: nếu giống mẫu trước → tăng đếm; khác → reset đếm */
  if (raw == s_gear.lastRaw) {
    if (s_gear.cnt < 0xFFu) { s_gear.cnt++; }
  } else {
    s_gear.lastRaw = raw;
    s_gear.cnt     = 1u;  /* bắt đầu chuỗi mới */
  }

  /* 3) Khi đủ số mẫu liên tiếp và khác với stable → cập nhật & publish */
  if ((s_gear.cnt >= GEARSEL_DEBOUNCE_TICKS) && (raw != s_gear.stable)) {
    s_gear.stable = raw;
    (void)Rte_Write_GearSelector_GearOut(s_gear.stable);
    /* Giữ lastRaw/cnt để tiếp tục debounce chuỗi kế tiếp */
  }
}
