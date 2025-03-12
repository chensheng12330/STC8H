#ifndef __TIM_H
#define __TIM_H

#include "../comm/STC8h.h"  //包含此头文件后，不需要再包含"reg51.h"头文件
#include "../comm/usb.h"     //USB调试及复位所需头文件

extern u32 Sys_Time_10ms ;
#define Timer0_Reload   (MAIN_Fosc / 100)      //Timer 0 中断频率, 10次/秒 100hz的速度
#define Timer1_Reload   (MAIN_Fosc / 100)      //Timer 1 中断频率, 2000次/秒

void Timer0_init(void);
    
#endif

