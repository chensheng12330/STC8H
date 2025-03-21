/*---------------------------------------------------------------------*/
/* --- STC MCU Limited ------------------------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.STCAI.com ---------------------------------------------*/
/* --- Web: www.STCMCUDATA.com  ---------------------------------------*/
/* --- BBS: www.STCAIMCU.com  -----------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* 如果要在程序中使用此代码,请在程序中注明使用了STC的资料及程序        */
/*---------------------------------------------------------------------*/

/*************  功能说明    **************

本例程基于STC8H8K64U为主控芯片的实验箱9进行编写测试.

CDC(Communication Device Class)协议范例

WIN10以下的操作系统需要安装sys目录中的驱动程序，WIN10和WIN11免安装驱动
将代码下载到试验箱后，在PC端可识别为USB转串口的设备
使用试验箱上的DB9接口即可与其它串口进行通讯
串口的数据位只支持8位，停止位只支持1位
校验位可支持：无校验、奇校验、偶校验、1校验和0校验
波特率最高可支持460800，且支持自定义波特率

下载时, 设置IRC工作频率为 24MHz.

******************************************/

#include "stc.h"
#include "usb.h"
#include "uart.h"

void sys_init();

void main()
{
	P_SW2 |= 0x80;  //扩展寄存器(XFR)访问使能

    uart_init();
    usb_init();
    EA = 1;
    
    while (1)
    {
        uart_polling();
    }
}
