#ifndef __RTC_H
#define __RTC_H

#include "../comm/STC8h.h"  //包含此头文件后，不需要再包含"reg51.h"头文件
#include "../comm/usb.h"     //USB调试及复位所需头文件
#include "intrins.h"
#include "stdio.h"


extern xdata u8 B1S_Flag;

void RTC_config(void);


#endif


