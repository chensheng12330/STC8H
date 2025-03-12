#ifndef __EEPROM_H
#define __EEPROM_H

#include "../comm/STC8h.h"  //包含此头文件后，不需要再包含"reg51.h"头文件
#include "../comm/usb.h"     //USB调试及复位所需头文件
#include "intrins.h"

void IapIdle();
char IapRead(unsigned long addr); 
void IapProgram(unsigned long addr, char dat);
void IapErase(unsigned long addr);


#endif

