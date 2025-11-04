/**********************************************************
 * @file    Swc_DriveModeMgr.c
 * @brief   SWC – Drive Mode Manager (đọc & quản lý chế độ lái)
 * @details Luồng xử lý:
 *            1) Lấy mẫu chế độ lái thô từ IoHwAb qua RTE:
 *               Rte_Call_DriveModeMgr_IoHwAb_Mode_Get(&raw).
 *            2) Debounce theo thời gian để chống rung/nhiễu công tắc.
 *            3) Khi trạng thái ổn định thay đổi → publish:
 *               Rte_Write_DriveModeMgr_DriveModeOut(stable).
 *
 *          Mặc định:
 *            - Chu kỳ gọi Run10ms: 10 ms.
 *            - Cửa sổ debounce: 30 ms (3 chu kỳ).
 *
 *          Ghi chú:
 *            - Nếu IoHwAb không sẵn sàng → bỏ qua chu kỳ, giữ trạng thái cũ.
 *            - Chỉ cho phép giá trị thuộc tập {DRIVEMODE_ECO, DRIVEMODE_NORMAL}.
 *
 * @version 1.0
 * @date    2025-09-10 
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#include "Swc_DriveModeMgr.h"
#include "Rte.h"       /* Rte_Call_* & Rte_Write_* */
#include "Rte_Types.h"  /* DriveMode_e */

#ifndef DRIVEMODE_TASK_PERIOD_MS
#define DRIVEMODE_TASK_PERIOD_MS   (10u)  /* chu kỳ gọi Run10ms */
#endif

#ifndef DRIVEMODE_DEBOUNCE_MS
#define DRIVEMODE_DEBOUNCE_MS      (20u)  /* yêu cầu ổn định 30 ms */
#endif

/* Số mẫu cần liên tiếp giống nhau để coi là ổn định */
#define DRIVEMODE_DEBOUNCE_TICKS \
  ((uint8_t)((DRIVEMODE_DEBOUNCE_MS + (DRIVEMODE_TASK_PERIOD_MS - 1u)) / DRIVEMODE_TASK_PERIOD_MS))

/* Trạng thái nội bộ cho thuật toán debounce */
typedef struct {
  DriveMode_e lastRaw;  /* mẫu vừa đọc (kỳ trước)         */
  DriveMode_e stable;   /* trạng thái lái đã ổn định       */
  uint8_t       cnt;      /* số lần liên tiếp cùng giá trị   */
  boolean     inited;   /* đã seed lần đầu chưa            */
} DriveMode_State_t;

static DriveMode_State_t s_mode;

/* Giới hạn giá trị hợp lệ, trả về ECO nếu ngoài phạm vi */
static DriveMode_e clamp_mode(DriveMode_e m)
{
  if (m != DRIVEMODE_ECO && m != DRIVEMODE_NORMAL) {
    return DRIVEMODE_ECO;
  }
  return m;
}

/* Seed giá trị ban đầu từ phần cứng (qua IoHwAb) để tránh “giật” lần 1 */
static void DriveMode_SeedFromHw(void)
{
  DriveMode_e raw = DRIVEMODE_ECO;
  if (Rte_Call_DriveModeMgr_IoHwAb_Mode_Get(&raw) != E_OK) {
    raw = DRIVEMODE_ECO; /* fallback an toàn */
  }
  raw = clamp_mode(raw);

  s_mode.lastRaw = raw;
  s_mode.stable  = raw;
  s_mode.cnt     = 0u;
  s_mode.inited  = TRUE;

  /* Publish giá trị seed để các SWC khác có dữ liệu ngay */
  (void)Rte_Write_DriveModeMgr_DriveModeOut(s_mode.stable);
}

void Swc_DriveModeMgr_Init(void)
{
  s_mode.lastRaw = DRIVEMODE_ECO;
  s_mode.stable  = DRIVEMODE_ECO;
  s_mode.cnt     = 0u;
  s_mode.inited  = FALSE;

  DriveMode_SeedFromHw();

  printf("DriveModeMgr: init done, stable=%d\n", s_mode.stable);
}

void Swc_DriveModeMgr_Run10ms(void)
{
  DriveMode_e raw;

  /* 1) Lấy mẫu thô từ IoHwAb (qua RTE) */
  if (Rte_Call_DriveModeMgr_IoHwAb_Mode_Get(&raw) != E_OK) {
    /* Không cập nhật khi IoHwAb lỗi; giữ nguyên trạng thái hiện hành */
    return;
  }
  raw = clamp_mode(raw);

  /* 2) Debounce: nếu giống mẫu trước → tăng đếm; khác → reset đếm */
  if (raw == s_mode.lastRaw) {
    if (s_mode.cnt < 0xFFu) { s_mode.cnt++; }
  } else {
    s_mode.lastRaw = raw;
    s_mode.cnt     = 1u;  /* bắt đầu chuỗi mới */
  }

  /* 3) Khi đủ số mẫu liên tiếp và khác với stable → cập nhật & publish */
  if ((s_mode.cnt >= DRIVEMODE_DEBOUNCE_TICKS) && (raw != s_mode.stable)) {
    s_mode.stable = raw;
    (void)Rte_Write_DriveModeMgr_DriveModeOut(s_mode.stable);
    /* Giữ lastRaw/cnt để tiếp tục debounce chuỗi kế tiếp */
  }
}
