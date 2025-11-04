/**********************************************************
 * @file    Swc_SafetyManager.c
 * @brief   SWC – Safety Manager (tổng hợp & áp ràng buộc an toàn)
 * @details Luồng xử lý mỗi chu kỳ:
 *   1) Đọc các tín hiệu từ RTE (SR-Require):
 *      - PedalOut (%), BrakeOut (bool), GearOut (PRND), DriveModeOut.
 *      - Kiểm tra E_OK/E_NOT_OK để phát hiện thiếu cập nhật.
 *   2) Ràng buộc an toàn:
 *      - Interlock chuyển số (thuật toán yêu cầu):
 *          **Từ P sang R/D hoặc từ R/D về P đều yêu cầu đạp phanh.**
 *        Các chuyển đổi khác (R↔N, N↔D, R↔D, P↔N) theo quy tắc này
 *        **không bắt buộc** phanh (có thể thay đổi nếu dự án yêu cầu).
 *      - Brake override: đang phanh → hạn chế % ga tối đa.
 *      - Timeout: nguồn dữ liệu quá hạn → fallback an toàn (ví dụ
 *        throttle=0, gear=P, mode=ECO).
 *   3) Đóng gói Safe_s và xuất qua RTE (SR-Provide).
 *
 * @version 1.1
 * @date    2025-09-10 
 * @author  Nguyễn Tuấn Khoa 
 **********************************************************/

#include "Swc_SafetyManager.h"
#include "Rte.h"
#include "Rte_Types.h"

/* ========= Cấu hình nhanh (điều chỉnh tuỳ yêu cầu hệ thống) ========= */

/* Chu kỳ gọi (ms) – dùng để suy ra số tick cho timeout */
#ifndef SAFETY_TASK_PERIOD_MS
#define SAFETY_TASK_PERIOD_MS             (10u)
#endif

/* Giới hạn % ga tối đa khi đang đạp phanh (brake override) */
#ifndef SAFETY_THR_MAX_WHEN_BRAKE
#define SAFETY_THR_MAX_WHEN_BRAKE         (10u)  /* ví dụ 10% */
#endif

/* Timeout dữ liệu (ms) – quá thời gian mà chưa có update → fallback */
#ifndef SAFETY_TIMEOUT_MS
#define SAFETY_TIMEOUT_MS                 (100u) /* 100 ms */
#endif

/* Số tick tương ứng timeout */
#define SAFETY_TIMEOUT_TICKS \
  ((uint8_t)((SAFETY_TIMEOUT_MS + (SAFETY_TASK_PERIOD_MS - 1u)) / SAFETY_TASK_PERIOD_MS))

/* ========= Trạng thái nội bộ ========= */

typedef struct {
  /* Bản nhớ “an toàn” lần trước để làm tham chiếu và fallback */
  Safe_s   lastSafe;

  /* Bộ đếm “miss update” cho từng nguồn (tích số chu kỳ E_NOT_OK liên tiếp) */
  uint8_t    missPedal;
  uint8_t    missBrake;
  uint8_t    missGear;
  uint8_t    missMode;

  /* Seed hoàn tất chưa */
  boolean  inited;
} Safety_State_t;

static Safety_State_t s_safety;

/* ========= Tiện ích clamp/kiểm tra ========= */

static uint8_t clamp_0_100(int v)
{
  if (v < 0)   return 0u;
  if (v > 100) return 100u;
  return (uint8_t)v;
}

static boolean gear_is_valid(Gear_e g)
{
  return (g == GEAR_P) || (g == GEAR_R) || (g == GEAR_N) || (g == GEAR_D);
}

/* ========= Seed giá trị ban đầu =========
 * Đặt mặc định an toàn: throttle=0, brake=FALSE, gear=P, mode=ECO.
 * (Tuỳ dự án có thể cố đọc từ RTE, nhưng nhiều hệ sẽ E_NOT_OK ở chu kỳ đầu.)
 */
static void Safety_Seed(void)
{
  s_safety.lastSafe.throttle_pct = 0u;
  s_safety.lastSafe.brakeActive  = FALSE;
  s_safety.lastSafe.gear         = GEAR_P;
  s_safety.lastSafe.driveMode    = DRIVEMODE_ECO;

  s_safety.missPedal = 0u;
  s_safety.missBrake = 0u;
  s_safety.missGear  = 0u;
  s_safety.missMode  = 0u;

  s_safety.inited    = TRUE;

  /* Xuất ngay để các SWC downstream có dữ liệu */
  (void)Rte_Write_SafetyManager_SafeOut(&s_safety.lastSafe);
}

/* ========= Thuật toán Interlock chuyển số =========
 * Quy tắc yêu cầu:
 *   - TỪ P sang R hoặc D → bắt buộc brake = TRUE, nếu không GIỮ nguyên P.
 *   - TỪ R hoặc D về P → bắt buộc brake = TRUE, nếu không GIỮ nguyên R/D.
 *   - Các chuyển đổi khác (R↔N, N↔D, R↔D, P↔N) theo quy tắc này
 *     không bắt buộc phanh (có thể bổ sung theo yêu cầu dự án).
 */
static Gear_e Safety_ApplyGearInterlock(Gear_e requested, boolean brake, Gear_e currentSafe)
{
  /* Nếu không thay đổi thì giữ nguyên */
  if (requested == currentSafe) {
    return currentSafe;
  }

  /* Trường hợp 1: P -> (R hoặc D) cần phanh */
  if (currentSafe == GEAR_P && (requested == GEAR_R || requested == GEAR_D)) {
    return brake ? requested : currentSafe;
  }

  /* Trường hợp 2: (R hoặc D) -> P cần phanh */
  if ((currentSafe == GEAR_R || currentSafe == GEAR_D) && requested == GEAR_P) {
    return brake ? requested : currentSafe;
  }

  /* Các chuyển đổi còn lại (R<->N, N<->D, R<->D, P<->N) không bắt buộc phanh */
  (void)brake; /* không dùng trong nhánh này */
  return requested;
}

/* ========= Áp brake override cho % ga =========
 * - Khi brake đang active, giới hạn cực đại % ga.
 */
static uint8_t Safety_ApplyBrakeOverride(uint8_t throttlePct, boolean brake)
{
  if (brake && throttlePct > SAFETY_THR_MAX_WHEN_BRAKE) {
    return SAFETY_THR_MAX_WHEN_BRAKE;
  }
  return throttlePct;
}

/* ========= API ========= */

void Swc_SafetyManager_Init(void)
{
  s_safety.inited = FALSE;
  Safety_Seed(); /* seed mặc định an toàn */

  printf("SafetyManager: init done, throttle=%d, gear=%d, mode=%d, brake=%d\n",
         s_safety.lastSafe.throttle_pct, s_safety.lastSafe.gear,
         s_safety.lastSafe.driveMode, s_safety.lastSafe.brakeActive);
}

void Swc_SafetyManager_Run10ms(void)
{
  if (!s_safety.inited) {
    Safety_Seed();
  }

  /* 1) Đọc nguồn dữ liệu từ RTE */
  uint8_t     pedalPctTmp;
  boolean     brakeTmp;
  Gear_e      gearTmp;
  DriveMode_e modeTmp;

  boolean havePedal = (Rte_Read_SafetyManager_PedalOut(&pedalPctTmp) == E_OK);
  boolean haveBrake = (Rte_Read_SafetyManager_BrakeOut(&brakeTmp)   == E_OK);
  boolean haveGear  = (Rte_Read_SafetyManager_GearOut(&gearTmp)     == E_OK);
  boolean haveMode  = (Rte_Read_SafetyManager_DriveModeOut(&modeTmp)== E_OK);

  /* 2) Quản lý timeout nguồn dữ liệu */
  s_safety.missPedal = (havePedal ? 0u : (uint8_t)((s_safety.missPedal < 0xFFu) ? s_safety.missPedal + 1u : 0xFFu));
  s_safety.missBrake = (haveBrake ? 0u : (uint8_t)((s_safety.missBrake < 0xFFu) ? s_safety.missBrake + 1u : 0xFFu));
  s_safety.missGear  = (haveGear  ? 0u : (uint8_t)((s_safety.missGear  < 0xFFu) ? s_safety.missGear  + 1u : 0xFFu));
  s_safety.missMode  = (haveMode  ? 0u : (uint8_t)((s_safety.missMode  < 0xFFu) ? s_safety.missMode  + 1u : 0xFFu));

  const boolean pedalTimeout = (s_safety.missPedal >= SAFETY_TIMEOUT_TICKS);
  const boolean brakeTimeout = (s_safety.missBrake >= SAFETY_TIMEOUT_TICKS);
  const boolean gearTimeout  = (s_safety.missGear  >= SAFETY_TIMEOUT_TICKS);
  const boolean modeTimeout  = (s_safety.missMode  >= SAFETY_TIMEOUT_TICKS);

  /* 3) Xây dựng giá trị “yêu cầu” (requested) từ nguồn/hoặc fallback */
  uint8_t     reqThrottle = pedalTimeout ? 0u : (havePedal ? clamp_0_100((int)pedalPctTmp) : s_safety.lastSafe.throttle_pct);
  boolean     reqBrake    = brakeTimeout ? FALSE : (haveBrake ? (brakeTmp ? TRUE : FALSE)    : s_safety.lastSafe.brakeActive);
  Gear_e      reqGear     = gearTimeout  ? GEAR_P : (haveGear  && gear_is_valid(gearTmp) ? gearTmp : s_safety.lastSafe.gear);
  DriveMode_e reqMode     = modeTimeout  ? DRIVEMODE_ECO : (haveMode ? modeTmp : s_safety.lastSafe.driveMode);

  /* 4) Áp các ràng buộc an toàn */
  /* 4.1) Brake override với throttle */
  uint8_t safeThrottle = Safety_ApplyBrakeOverride(reqThrottle, reqBrake);

  /* 4.2) Interlock chuyển số P<->R/D bắt buộc phanh */
  Gear_e safeGear = Safety_ApplyGearInterlock(reqGear, reqBrake, s_safety.lastSafe.gear);

  /* 5) Gói Safe_s và xuất qua RTE */
  Safe_s out;
  out.throttle_pct = safeThrottle;
  out.brakeActive  = reqBrake;
  out.gear         = safeGear;
  out.driveMode    = reqMode;

  (void)Rte_Write_SafetyManager_SafeOut(&out);

  /* 6) Lưu lại bản “an toàn” làm tham chiếu cho chu kỳ sau */
  s_safety.lastSafe = out;
}
