
#include "..\Cfg\os_cfg.h"


NAME    PENDSV_ISR
    

EXTRN    CODE (pendSV_ISR_Handler)


CSEG AT     PendSv_InterruptNumber*8+3
LJMP        pendSV_ISR_Handler


END
