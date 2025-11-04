// OS/src/syscalls_min.c
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

/* ==== Heap/Stack symbols (tùy linker) ==== */
extern char __HeapLimit      __attribute__((weak));
extern char __bss_end__      __attribute__((weak));
extern char _ebss            __attribute__((weak));
extern char end              __attribute__((weak));
extern char _end             __attribute__((weak));
extern char _estack;   // top of stack (phải có trong linker)

static char *heap_end = 0;

static inline char *get_heap_base(void){
    if (&__HeapLimit) return &__HeapLimit;
    if (&__bss_end__) return &__bss_end__;
    if (&_ebss)       return &_ebss;
    if (&end)         return &end;
    if (&_end)        return &_end;
    return 0;
}

void *_sbrk(ptrdiff_t incr){
    if (!heap_end){
        heap_end = get_heap_base();
        if (!heap_end){ errno = ENOMEM; return (void*)-1; }
    }
    const size_t reserve = 1024u;  // giữ lại 1KB cho stack/ISR
    uintptr_t estack_addr = (uintptr_t)&_estack;
    char *limit = (char *)(estack_addr - reserve);
    if (heap_end + incr > limit){
        errno = ENOMEM;
        return (void*)-1;
    }
    char *prev = heap_end;
    heap_end  += incr;
    return prev;
}

/* _write() đã có trong semihost.c → KHÔNG định nghĩa ở đây */
int _close(int fd) { (void)fd; errno = ENOSYS; return -1; }

off_t _lseek(int fd, off_t off, int whence){
    (void)fd; (void)off; (void)whence; errno = ENOSYS; return -1;
}

int _read(int fd, void *buf, size_t cnt){
    (void)fd; (void)buf; (void)cnt; errno = ENOSYS; return -1;
}

int _fstat(int fd, struct stat *st){
    (void)fd; if (st) st->st_mode = S_IFCHR; return 0;
}

int _isatty(int fd){
    (void)fd; return 1;
}
