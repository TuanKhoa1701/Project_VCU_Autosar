#include "os.h"
#include "stm32f10x.h"
#include <stdio.h>
#include "EcuM.h"   
#include "core_cm3.h" 

int main(void)
{   

    EcuM_Init();
    /* Vào OS → autostart InitTask */
    StartOS(OSDEFAULTAPPMODE) ;// chế độ bình thường;

    /* Không bao giờ quay lại đây trong mô phỏng */
    for(;;) { }
}
