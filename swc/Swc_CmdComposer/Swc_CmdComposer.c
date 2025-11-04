/**********************************************************
 * @file    Swc_CmdComposer.c
 * @brief   SWC – Command Composer (biên soạn lệnh VCU)
 * @details Luồng xử lý mỗi chu kỳ:
 *   1) Đọc gói Safe_s từ RTE (Rte_Read_CmdComposer_SafeOut).
 *   2) Chuẩn hoá/ánh xạ:
 *        - throttle_pct → ThrottleReq_pct (0..100, clamp).
 *        - gear (P/R/N/D) → GearSel (0..3).
 *        - driveMode (ECO/NORMAL) → 0/1.
 *        - brakeActive (boolean).
 *        - alive counter (4-bit) tăng dần 0..15, quấn vòng.
 *   3) Ghi từng signal qua RTE_Write_* và gọi Rte_Trigger_* để COM phát.
 *
 *   Ghi chú:
 *     - Không tính CRC ở đây (COM sẽ tính và chèn nếu yêu cầu giao thức).
 *     - Nếu Safe_s chưa sẵn sàng (E_NOT_OK) thì giữ lệnh trước đó
 *       hoặc fallback an toàn (throttle=0, giữ gear/mode/brake cũ).
 *
 * @version 1.0
 * @date    2025-09-10 
 * @author  Nguyễn Tuấn Khoa
 **********************************************************/

#include "Swc_CmdComposer.h"
#include "Rte.h"
#include "Rte_Types.h"

/* ================== Cấu hình nhanh ================== */
#ifndef CMDC_TASK_PERIOD_MS
#define CMDC_TASK_PERIOD_MS      (10u)  /* chu kỳ gọi Run10ms */
#endif

/* Nếu muốn chặn thay đổi %ga quá nhanh tại lớp lệnh, đặt bước giới hạn.
   0 = không áp rate-limit ở CmdComposer (đã lọc ở Pedal/Safety). */
#ifndef CMDC_THR_RATE_LIMIT_STEP
#define CMDC_THR_RATE_LIMIT_STEP (0u)
#endif
/* ==================================================== */

/* Trạng thái nội bộ để nhớ lệnh lần trước & alive counter */
typedef struct {
  uint8_t   lastThrottle;  /* 0..100 */
  uint8_t   lastGearU8;    /* 0..3   */
  uint8_t   lastModeU8;    /* 0..1   */
  boolean   lastBrake;     /* TRUE/FALSE */
  uint8_t   aliveNibble;   /* 0..15 */
  boolean   inited;
} CmdComposer_State_t;

static CmdComposer_State_t s_cmd;

/* ===== Tiện ích clamp/ánh xạ ===== */
static uint8_t clamp_0_100(int v)
{
  if (v < 0)   return 0u;
  if (v > 100) return 100u;
  return (uint8_t)v;
}

static uint8_t gear_to_u8(Gear_e g)
{
  switch (g) {
    case GEAR_P: return 0u;
    case GEAR_R: return 1u;
    case GEAR_N: return 2u;
    case GEAR_D: return 3u;
    default:     return 0u; /* fallback an toàn */
  }
}

static uint8_t mode_to_u8(DriveMode_e m)
{
  switch (m) {
    case DRIVEMODE_ECO:    return 0u;
    case DRIVEMODE_NORMAL: return 1u;
    default:               return 0u;
  }
}

static uint8_t rate_limit_u8(uint8_t target, uint8_t current, uint8_t step)
{
  if (step == 0u) return target;
  if (target > current) {
    uint8_t inc = (uint8_t)((target - current) > step ? step : (target - current));
    return (uint8_t)(current + inc);
  } else if (target < current) {
    uint8_t dec = (uint8_t)((current - target) > step ? step : (current - target));
    return (uint8_t)(current - dec);
  }
  return current;
}

/* ===== Seed trạng thái lệnh ban đầu ===== */
static void CmdComposer_Seed(void)
{
  s_cmd.lastThrottle = 0u;
  s_cmd.lastGearU8   = gear_to_u8(GEAR_P);
  s_cmd.lastModeU8   = mode_to_u8(DRIVEMODE_ECO);
  s_cmd.lastBrake    = FALSE;
  s_cmd.aliveNibble  = 0u;
  s_cmd.inited       = TRUE;

  /* Ghi initial xuống shadow buffer RTE để phía COM có dữ liệu sớm */
  (void)Rte_Write_CmdComposer_VcuCmdTx_ThrottleReq_pct(s_cmd.lastThrottle);
  (void)Rte_Write_CmdComposer_VcuCmdTx_GearSel(s_cmd.lastGearU8);
  (void)Rte_Write_CmdComposer_VcuCmdTx_DriveMode(s_cmd.lastModeU8);
  (void)Rte_Write_CmdComposer_VcuCmdTx_BrakeActive(s_cmd.lastBrake);
  (void)Rte_Write_CmdComposer_VcuCmdTx_AliveCounter(s_cmd.aliveNibble);

}

void Swc_CmdComposer_Init(void)
{
  s_cmd.inited = FALSE;
  CmdComposer_Seed();

  printf("CmdComposer: init done, throttle=%d, gear=%d, mode=%d, brake=%d\n",
         s_cmd.lastThrottle, s_cmd.lastGearU8, s_cmd.lastModeU8, s_cmd.lastBrake);
}

void Swc_CmdComposer_Run10ms(void)
{
  if (!s_cmd.inited) {
    CmdComposer_Seed();
  }

  Safe_s safe;
  const boolean haveSafe = (Rte_Read_CmdComposer_SafeOut(&safe) == E_OK);

  /* 1) Lấy giá trị mục tiêu từ Safe_s hoặc fallback từ last */
  uint8_t  thr  = haveSafe ? clamp_0_100((int)safe.throttle_pct) : s_cmd.lastThrottle;
  uint8_t  gear = haveSafe ? gear_to_u8(safe.gear)               : s_cmd.lastGearU8;
  uint8_t  mode = haveSafe ? mode_to_u8(safe.driveMode)          : s_cmd.lastModeU8;
  boolean brk = haveSafe ? (safe.brakeActive ? TRUE : FALSE)   : s_cmd.lastBrake;

  /* 2) (Tuỳ chọn) rate-limit riêng ở CmdComposer (thường không cần) */
  thr = rate_limit_u8(thr, s_cmd.lastThrottle, CMDC_THR_RATE_LIMIT_STEP);

  /* 3) Ghi xuống shadow buffer RTE từng signal */

  (void)Rte_Write_CmdComposer_VcuCmdTx_ThrottleReq_pct(thr);
  (void)Rte_Write_CmdComposer_VcuCmdTx_GearSel(gear);
  (void)Rte_Write_CmdComposer_VcuCmdTx_DriveMode(mode);
  (void)Rte_Write_CmdComposer_VcuCmdTx_BrakeActive(brk);

  /* 4) Tăng alive counter (4-bit) và ghi */
  s_cmd.aliveNibble = (uint8_t)((s_cmd.aliveNibble + 1u) & 0x0Fu);
  (void)Rte_Write_CmdComposer_VcuCmdTx_AliveCounter(s_cmd.aliveNibble);


  /* 5) Yêu cầu COM phát frame */
  Rte_Trigger_CmdComposer_VcuCmdTx();
  /* 6) Lưu lại để làm “last” cho chu kỳ sau */
  s_cmd.lastThrottle = thr;
  s_cmd.lastGearU8   = gear;
  s_cmd.lastModeU8   = mode;
  s_cmd.lastBrake    = brk;
}
void Swc_CmdComposer_ReadEngineRPM(const uint16_t* data){
  Rte_Com_Update_EngineSpeedFromPdu(data);
}

