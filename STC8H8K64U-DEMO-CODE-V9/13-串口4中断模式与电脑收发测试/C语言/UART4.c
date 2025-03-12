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

本例程基于STC8H8K64U为主控芯片的实验箱9进行编写测试，STC8G、STC8H系列芯片可通用参考.

串口4全双工中断方式收发通讯程序。

通过PC向MCU发送数据, MCU收到后通过串口4把收到的数据原样返回.

用定时器做波特率发生器，建议使用1T模式(除非低波特率用12T)，并选择可被波特率整除的时钟频率，以提高精度。

下载时, 选择时钟 22.1184MHz (用户可自行修改频率).

******************************************/

#include "stc8h.h"       //包含此头文件后，不需要再包含"reg51.h"头文件

#define MAIN_Fosc       22118400L   //定义主时钟（精确计算115200波特率）

typedef     unsigned char   u8;
typedef     unsigned int    u16;
typedef     unsigned long   u32;


#define Baudrate4   115200L
#define UART4_BUF_LENGTH    64


u8  TX4_Cnt;    //发送计数
u8  RX4_Cnt;    //接收计数
bit B_TX4_Busy; //发送忙标志

u8  xdata RX4_Buffer[UART4_BUF_LENGTH]; //接收缓冲


void UART4_config(u8 brt);   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer4做波特率.
void PrintString4(u8 *puts);


//========================================================================
// 函数: void main(void)
// 描述: 主函数。
// 参数: none.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void main(void)
{
    P_SW2 |= 0x80;  //扩展寄存器(XFR)访问使能

    P0M1 = 0x30;   P0M0 = 0x30;   //设置P0.4、P0.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P1M1 = 0x30;   P1M0 = 0x30;   //设置P1.4、P1.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P2M1 = 0x3c;   P2M0 = 0x3c;   //设置P2.2~P2.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P3M1 = 0x50;   P3M0 = 0x50;   //设置P3.4、P3.6为漏极开路(实验箱加了上拉电阻到3.3V)
    P4M1 = 0x3c;   P4M0 = 0x3c;   //设置P4.2~P4.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P5M1 = 0x0c;   P5M0 = 0x0c;   //设置P5.2、P5.3为漏极开路(实验箱加了上拉电阻到3.3V)
    P6M1 = 0xff;   P6M0 = 0xff;   //设置为漏极开路(实验箱加了上拉电阻到3.3V)
    P7M1 = 0x00;   P7M0 = 0x00;   //设置为准双向口

    UART4_config(2);    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer4做波特率.
    EA = 1; //允许全局中断
    
    PrintString4("STC8H8K64U UART4 Test Programme!\r\n");  //UART4发送一个字符串

    while (1)
    {
        if((TX4_Cnt != RX4_Cnt) && (!B_TX4_Busy))   //收到数据, 发送空闲
        {
            S4BUF = RX4_Buffer[TX4_Cnt];
            B_TX4_Busy = 1;
            if(++TX4_Cnt >= UART4_BUF_LENGTH)   TX4_Cnt = 0;
        }
    }
}


//========================================================================
// 函数: void PrintString4(u8 *puts)
// 描述: 串口4发送字符串函数。
// 参数: puts:  字符串指针.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void PrintString4(u8 *puts)
{
    for (; *puts != 0;  puts++)     //遇到停止符0结束
    {
        S4BUF = *puts;
        B_TX4_Busy = 1;
        while(B_TX4_Busy);
    }
}

//========================================================================
// 函数: SetTimer2Baudraye(u16 dat)
// 描述: 设置Timer2做波特率发生器。
// 参数: dat: Timer2的重装值.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void SetTimer2Baudraye(u16 dat)  // 使用Timer2做波特率.
{
    AUXR &= ~(1<<4);    //Timer stop
    AUXR &= ~(1<<3);    //Timer2 set As Timer
    AUXR |=  (1<<2);    //Timer2 set as 1T mode
    T2H = dat / 256;
    T2L = dat % 256;
    IE2  &= ~(1<<2);    //禁止T2中断
    AUXR |=  (1<<4);    //Timer run enable
}

//========================================================================
// 函数: void UART4_config(u8 brt)
// 描述: UART4初始化函数。
// 参数: brt: 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer4做波特率.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART4_config(u8 brt)    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer4做波特率.
{
    if(brt == 2)
    {
        SetTimer2Baudraye(65536UL - (MAIN_Fosc / 4) / Baudrate4);
        S4CON = 0x10;       //8位数据, 使用Timer2做波特率发生器, 允许接收
    }
    else
    {
        S4CON = 0x50;       //8位数据, 使用Timer4做波特率发生器, 允许接收
        T4H = (65536UL - (MAIN_Fosc / 4) / Baudrate4) / 256;
        T4L = (65536UL - (MAIN_Fosc / 4) / Baudrate4) % 256;
        T4T3M = 0xa0;
    }
    IE2 |= 0x10;          //允许UART4中断
    P_SW2 &= ~0x04; 
    //P_SW2 |= 0x04;      //UART4 switch bit2 to: 0: P0.2 P0.3,  1: P5.2 P5.3

    B_TX4_Busy = 0;
    TX4_Cnt = 0;
    RX4_Cnt = 0;
}


//========================================================================
// 函数: void UART4_int (void) interrupt UART4_VECTOR
// 描述: UART4中断函数。
// 参数: nine.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART4_int (void) interrupt 18
{
    if((S4CON & 0x01) != 0)
    {
        S4CON &= ~0x01;    //Clear Rx flag
        RX4_Buffer[RX4_Cnt] = S4BUF;
        if(++RX4_Cnt >= UART4_BUF_LENGTH)   RX4_Cnt = 0;
    }

    if((S4CON & 0x02) != 0)
    {
        S4CON &= ~0x02;    //Clear Tx flag
        B_TX4_Busy = 0;
    }
}

