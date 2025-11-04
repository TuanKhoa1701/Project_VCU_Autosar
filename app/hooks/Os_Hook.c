#include "Os.h"
#include <stdio.h>

/* Các hook này do OS gọi; có thể bỏ nội dung nếu không cần log */

void StartupHook(void)
{
    printf("[OS] StartupHook()\n");
}

void ShutdownHook(StatusType e)
{
    printf("[OS] ShutdownHook(e=%u)\n", (unsigned)e);
}

void PreTaskHook(void)
{
    /* Có thể chèn đo thời gian, trace context switch... */
}

void PostTaskHook(void)
{
    /* Tương tự PreTaskHook */
}
