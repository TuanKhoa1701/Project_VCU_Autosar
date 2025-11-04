/**********************************************************
 * @file    Os_Ioc.c
 * @brief   Hàng đợi IOC (byte queue) tối giản cho OS (STM32F103)
 * @details Dùng Enable và Disable_irq để bảo vệ head/tail 
 *          khi truy cập đồng thời giữa ISR và Task.
 *
 *  API:
 *            - void Ioc_Init(OsIocQueue* q, uint8* storage, uint16 size)
 *            - int  Ioc_Send(OsIocQueue* q, uint8 b)       // 0=OK, -1=FULL/err
 *            - int  Ioc_Receive(OsIocQueue* q, uint8* out) // 0=OK, -1=EMPTY/err
 *
 * Quy ước:
 *            - Dùng 1 ô trống để phân biệt FULL/EMPTY:
 *                next(head) == tail  ⇒ FULL
 *                head == tail        ⇒ EMPTY
 *            - size tối thiểu = 2 (nếu <2 → vô hiệu queue).
 *
 * @version  1.0
 * @date     2025-09-10 
 * @author   Nguyễn Tuấn Khoa
 **********************************************************/

#include "Os.h"   /* OsIocQueue, uint8/uint16/uint32 ... */

TaskType Rec_list[2]= {TASK_B, TASK_C};

static OsIocCtrl IocChannel[MAX_IOC_CHANNELS];
/* =========================================================
 * Ioc_Init
 *  - Khởi tạo queue với bộ nhớ ngoài do ứng dụng cấp.
 *  - size phải ≥ 2 (vì dùng 1 ô trống để phân biệt full/empty).
 *  - Nếu tham số lỗi → vô hiệu queue (size=0).
 * =======================================================*/
void Ioc_Init(uint8_t channel, uint8_t num, TaskType* receivers)
{
  if(channel >= MAX_IOC_CHANNELS || !receivers) return;
  
  /* Init thường gọi trước khi bật IRQ → không cần vùng găng */
  IocChannel[channel].used = 1;
  IocChannel[channel].num_receivers = num;
  IocChannel[channel].head = 0;
  IocChannel[channel].count = 0;

  for(int i=0; i<num; i++) {
    IocChannel[channel].receivers[i] = receivers[i];
    IocChannel[channel].tail[i] = 0;
  }
}

/* =========================================================
 * Ioc_Send
 *  - Ghi 1 byte vào queue.
 *  - Trả  0 nếu OK, -1 nếu FULL hoặc chưa init.
 *  - Bảo vệ head/tail bằng vùng găng ngắn (PRIMASK).
 * =======================================================*/
uint8_t Ioc_Send(uint8_t channel, uint16_t* data)
{
  if (channel >= MAX_IOC_CHANNELS || !IocChannel[channel].used|| !data) return -1;
    OsIocCtrl *I = &IocChannel[channel];
  __disable_irq();

    I->buffer[channel][I->head] = *data;
    I->head = (I->head + 1u) % IOC_BUFFER_SIZE;

    if(I->count < IOC_BUFFER_SIZE) I->count++;
    else {
      for (int i=0; i<I->num_receivers; i++){
        I->tail[i] = (I->tail[i] + 1u) % IOC_BUFFER_SIZE;
      }
    }
    __enable_irq();
    for(int i=0; i<I->num_receivers; i++) {
      SetEvent(I->receivers[i], EV_RX);
    }
    return 0;
  
}

/* =========================================================
 * Ioc_Receive
 *  - Đọc 1 byte từ queue.
 *  - Trả  0 nếu OK (*out nhận dữ liệu), -1 nếu EMPTY/err.
 * =======================================================*/
uint8_t Ioc_Receive(uint8_t channel, uint16_t* data, TaskType receiver)
{
  if (channel >= MAX_IOC_CHANNELS|| !IocChannel[channel].used || !receiver || !data)
   return -1;

  OsIocCtrl *I = &IocChannel[channel];
  EventMaskType ev;

  WaitEvent(EV_RX);
  GetEvent(receiver, &ev);

  if(ev & EV_RX){
    if (I->tail[receiver] == I->head) { /* Buffer rỗng */
      return -1;
    }
    ClearEvent(EV_RX);
    __disable_irq();

    *data = I->buffer[channel][I->tail[receiver]];
    I->tail[receiver] = (I->tail[receiver] + 1u) % IOC_BUFFER_SIZE;
    __enable_irq();
  }
    return 0;
  
}
