/**********************************************************
 * @file    Swc_PedalAcq.c
 * @brief   SWC – Pedal Acquisition (đọc & lọc % đạp ga)
 * @details Luồng xử lý:
 *            1) Lấy mẫu % ga thô từ IoHwAb qua RTE:
 *               Rte_Call_PedalAcq_IoHwAb_Pedal_ReadPct(&raw).
 *            2) Lọc EMA (Exponential Moving Average) để giảm nhiễu:
 *                 filt_q = filt_q + ((raw<<K) - filt_q) >> K
 *               (K = 2 → alpha = 1/4; lưu nội bộ dạng QK để tránh float)
 *            3) Rate limit đầu ra theo step %/chu kỳ để tránh “giật”.
 *            4) Publish ra RTE: Rte_Write_PedalAcq_PedalOut(outPct).
 *
 *          Mặc định:
 *            - Chu kỳ:               10 ms.
 *            - EMA alpha = 1/4:      K = 2 (dịch phải 2 bit).
 *            - Rate limit:           2 % mỗi 10 ms (có thể chỉnh).
 *
 *          Ghi chú:
 *            - Nếu IoHwAb không sẵn sàng → bỏ qua chu kỳ, giữ trạng thái cũ.
 *            - Mọi giá trị được clamp về 0..100 trước khi dùng.
 *
 * @version 1.0
 * @date    2025-09-10 
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#include "Swc_PedalAcq.h"
#include "Rte.h"   /* Rte_Call_* & Rte_Write_* */

/* ================== Cấu hình nhanh ================== */
#ifndef PEDAL_TASK_PERIOD_MS
#define PEDAL_TASK_PERIOD_MS     (10u)  /* chu kỳ gọi Run10ms */
#endif

/* K = 2  → alpha = 1/4; K = 3 → alpha = 1/8 (mượt hơn) */
#ifndef PEDAL_EMA_SHIFT_K
#define PEDAL_EMA_SHIFT_K        (2u)
#endif

/* Bước rate-limit (% mỗi chu kỳ). Ví dụ 2% / 10 ms */
#ifndef PEDAL_RATE_LIMIT_STEP
#define PEDAL_RATE_LIMIT_STEP    (2u)
#endif
/* ==================================================== */

/* Trạng thái nội bộ */
typedef struct {
  uint16_t filt_q;     /* giá trị lọc dạng QK (x4 nếu K=2)         */
  uint8_t  outPct;     /* % đã publish lần gần nhất (0..100)       */
  boolean inited;    /* đã seed chưa                              */
} PedalAcq_State_t;

static PedalAcq_State_t s_pedal;

/* Clamp 0..100 */
static uint8_t clamp_0_100(int v)
{
  if (v < 0)   return 0u;
  if (v > 100) return 100u;
  return (uint8_t)v;
}

/* Seed giá trị ban đầu từ phần cứng để tránh “giật” lần 1 */
static void PedalAcq_SeedFromHw(void)
{
  uint8_t raw = 0u;
  if (Rte_Call_PedalAcq_IoHwAb_Pedal_ReadPct(&raw) != E_OK) {
    raw = 0u; /* fallback an toàn */
  }
  raw = clamp_0_100(raw);

  /* lưu ở QK: filt_q = raw << K */
  s_pedal.filt_q = ((uint16_t)raw) << PEDAL_EMA_SHIFT_K;
  s_pedal.outPct = raw;
  s_pedal.inited = TRUE;

  /* Publish ngay seed để các SWC khác có dữ liệu */
  (void)Rte_Write_PedalAcq_PedalOut(s_pedal.outPct);
}

void Swc_PedalAcq_Init(void)
{
  s_pedal.filt_q = 0u;
  s_pedal.outPct = 0u;
  s_pedal.inited = FALSE;

  PedalAcq_SeedFromHw();

  printf("PedalAcq: init done, stable=%d\n", s_pedal.outPct);
}

void Swc_PedalAcq_Run10ms(void)
{
  uint8_t raw = 0u;

  /* 1) Lấy mẫu từ IoHwAb → RTE */
  if (Rte_Call_PedalAcq_IoHwAb_Pedal_ReadPct(&raw) != E_OK) {
    /* Không cập nhật khi IoHwAb lỗi; giữ nguyên output hiện tại */
    return;
  }
  raw = clamp_0_100(raw);

  /* 2) EMA (QK) */
  /* filt_q += ((raw<<K) - filt_q) >> K; */
  uint16_t raw_q = ((uint16_t)raw) << PEDAL_EMA_SHIFT_K;
  s_pedal.filt_q = (uint16_t)(s_pedal.filt_q + ((raw_q - s_pedal.filt_q) >> PEDAL_EMA_SHIFT_K));

  /* 3) Tính mục tiêu nguyên % từ filt_q */
  uint8_t target = (uint8_t)(s_pedal.filt_q >> PEDAL_EMA_SHIFT_K);

  /* 4) Rate limit: chỉ cho thay đổi tối đa PEDAL_RATE_LIMIT_STEP mỗi chu kỳ */
  uint8_t step = PEDAL_RATE_LIMIT_STEP;
  uint8_t out = s_pedal.outPct;

  if (target > out) {
    uint8_t inc = (uint8_t)((target - out) > step ? step : (target - out));
    out = (uint8_t)(out + inc);
  } else if (target < out) {
    uint8_t dec = (uint8_t)((out - target) > step ? step : (out - target));
    out = (uint8_t)(out - dec);
  }

  /* 5) Publish khi có thay đổi (không bắt buộc, nhưng giảm traffic) */
  if (out != s_pedal.outPct) {
    s_pedal.outPct = out;
    (void)Rte_Write_PedalAcq_PedalOut(s_pedal.outPct);
  }
}
