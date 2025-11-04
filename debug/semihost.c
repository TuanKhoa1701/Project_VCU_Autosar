#include <stddef.h>
#include <stdint.h>
#include "stm32f10x.h"

static inline int dbg_attached(void) {
  /* Debugger attached? bit C_DEBUGEN */
  return (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0;
}

/* SYS_WRITE0: in chuỗi kết thúc bằng '\0' ra host */
static inline void sh_write0(const char *s) {
  register uint32_t r0 __asm__("r0") = 0x04;      // SYS_WRITE0
  register const char *r1 __asm__("r1") = s;      // ptr to 0-terminated string
  __asm__ volatile ("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

/* Retarget printf -> semihosting (GDB/OpenOCD) */
int _write(int fd, const void *buf, size_t len) {
  (void)fd;
  if (!dbg_attached()) return (int)len;          // không treo nếu chạy độc lập
  /* Chuyển buffer sang chuỗi tạm + '\0' (giản lược) */
  /* an toàn/thực tế hơn: lặp chunk, copy vào buffer cục bộ và chèn '\0' */
  static char line[256];
  size_t n = (len < sizeof(line)-1) ? len : (sizeof(line)-1);
  for (size_t i=0; i<n; ++i) line[i] = ((const char*)buf)[i];
  line[n] = '\0';
  sh_write0(line);
  return (int)len;
}
