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
/* 如果要在程序中使用此代码,请在程序中注明使用了STC的资料及程序            */
/*---------------------------------------------------------------------*/


/*************  功能说明    **************

本例程基于STC8H8K64U为主控芯片的实验箱9进行编写测试，STC8G、STC8H系列芯片可通用参考.

短接实验箱上的J7、J8跳线进行测试.

串口3定时发送一串数据给串口2.

串口2输出从串口3接收到的数据.

下载时, 选择时钟 22.1184MHZ (用户可自行修改频率).

******************************************/

#include "stc8h.h"       //包含此头文件后，不需要再包含"reg51.h"头文件
#include "intrins.h"

#define     MAIN_Fosc       22118400L   //定义主时钟

typedef     unsigned char   u8;
typedef     unsigned int    u16;
typedef     unsigned long   u32;

/****************************** 用户定义宏 ***********************************/

#define Timer0_Reload   (65536UL -(MAIN_Fosc / 1000))       //Timer 0 中断频率, 1000次/秒

#define Baudrate2   115200UL
#define Baudrate3   115200UL

#define UART2_BUF_LENGTH    64
#define UART3_BUF_LENGTH    64

/*****************************************************************************/

/*************  本地变量声明    **************/
bit B_1ms;          //1ms标志
u16 Sec_Cnt;    //1秒计数

u8  TX2_Cnt;    //发送计数
u8  RX2_Cnt;    //接收计数
u8  TX3_Cnt;    //发送计数
u8  RX3_Cnt;    //接收计数
bit B_TX2_Busy; //发送忙标志
bit B_TX3_Busy; //发送忙标志
u8 RX2_TimeOut;
u8 RX3_TimeOut;

u8  xdata RX2_Buffer[UART2_BUF_LENGTH]; //接收缓冲
u8  xdata RX3_Buffer[UART3_BUF_LENGTH]; //接收缓冲

void UART2_config(u8 brt);   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
void UART3_config(u8 brt);   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer3做波特率.
void UART2_TxByte(u8 dat);
void UART3_TxByte(u8 dat);
void PrintString2(u8 *puts);
void PrintString3(u8 *puts);
/******************** 主函数 **************************/
void main(void)
{
    u8 i;
	
    P_SW2 |= 0x80;  //扩展寄存器(XFR)访问使能

    P0M1 = 0x30;   P0M0 = 0x30;   //设置P0.4、P0.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P1M1 = 0x30;   P1M0 = 0x30;   //设置P1.4、P1.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P2M1 = 0x3c;   P2M0 = 0x3c;   //设置P2.2~P2.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P3M1 = 0x50;   P3M0 = 0x50;   //设置P3.4、P3.6为漏极开路(实验箱加了上拉电阻到3.3V)
    P4M1 = 0x3c;   P4M0 = 0x3c;   //设置P4.2~P4.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P5M1 = 0x0c;   P5M0 = 0x0c;   //设置P5.2、P5.3为漏极开路(实验箱加了上拉电阻到3.3V)
    P6M1 = 0xff;   P6M0 = 0xff;   //设置为漏极开路(实验箱加了上拉电阻到3.3V)
    P7M1 = 0x00;   P7M0 = 0x00;   //设置为准双向口

    //  Timer0初始化
    AUXR = 0x80;    //Timer0 set as 1T, 16 bits timer auto-reload, 
    TH0 = (u8)(Timer0_Reload / 256);
    TL0 = (u8)(Timer0_Reload % 256);
    ET0 = 1;    //Timer0 interrupt enable
    TR0 = 1;    //Tiner0 run

    UART2_config(2);    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
    UART3_config(3);    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer3做波特率.
    EA = 1;     //打开总中断

    PrintString2("STC8H8K64U UART2-UART3 Test Programme!\r\n");  //UART2发送一个字符串
    PrintString3("STC8H8K64U UART3-UART2 Test Programme!\r\n");  //UART3发送一个字符串

    while (1)
    {
        if(B_1ms)
        {
            B_1ms = 0;
            Sec_Cnt++;
            if(Sec_Cnt>=1000)
            {
                Sec_Cnt = 0;
                PrintString3("STC8H8K64U UART3-UART2 Test Programme!\r\n");  //UART3发送一个字符串
            }
//            if(RX3_TimeOut > 0)     //超时计数
//            {
//                if(--RX3_TimeOut == 0)
//                {
//                    for(i=0; i<RX3_Cnt; i++)    UART3_TxByte(RX3_Buffer[i]);    //串口3把收到的数据原样返回
//                    RX3_Cnt  = 0;   //清除字节数
//                }
//            }
        
            if(RX2_TimeOut > 0)     //超时计数
            {
                if(--RX2_TimeOut == 0)
                {
                    for(i=0; i<RX2_Cnt; i++)    UART2_TxByte(RX2_Buffer[i]);    //把收到的数据通过串口2输出
                    RX2_Cnt  = 0;   //清除字节数
                }
            }
        }
    }
}


/********************** Timer0 1ms中断函数 ************************/
void timer0(void) interrupt 1
{
	B_1ms = 1;
}

//========================================================================
// 函数: void UART2_TxByte(u8 dat)
// 描述: 发送一个字节.
// 参数: 无.
// 返回: 无.
// 版本: V1.0, 2014-6-30
//========================================================================
void UART2_TxByte(u8 dat)
{
    B_TX2_Busy = 1;
    S2BUF = dat;
    while(B_TX2_Busy);
}

//========================================================================
// 函数: void UART3_TxByte(u8 dat)
// 描述: 发送一个字节.
// 参数: 无.
// 返回: 无.
// 版本: V1.0, 2014-6-30
//========================================================================
void UART3_TxByte(u8 dat)
{
    B_TX3_Busy = 1;
    S3BUF = dat;
    while(B_TX3_Busy);
}

//========================================================================
// 函数: void PrintString2(u8 *puts)
// 描述: 串口2发送字符串函数。
// 参数: puts:  字符串指针.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void PrintString2(u8 *puts)
{
    for (; *puts != 0;  puts++)     //遇到停止符0结束
    {
        UART2_TxByte(*puts);
    }
}

//========================================================================
// 函数: void PrintString3(u8 *puts)
// 描述: 串口3发送字符串函数。
// 参数: puts:  字符串指针.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void PrintString3(u8 *puts)
{
    for (; *puts != 0;  puts++)     //遇到停止符0结束
    {
        UART3_TxByte(*puts);
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
void SetTimer2Baudraye(u16 dat)
{
    AUXR &= ~(1<<4);    //Timer stop
    AUXR &= ~(1<<3);    //Timer2 set As Timer
    AUXR |=  (1<<2);    //Timer2 set as 1T mode
    T2H = dat / 256;
    T2L = dat % 256;
    IE2  &= ~(1<<2);    //禁止中断
    AUXR |=  (1<<4);    //Timer run enable
}

//========================================================================
// 函数: void UART3_config(u8 brt)
// 描述: UART3初始化函数。
// 参数: brt: 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer3做波特率.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART3_config(u8 brt)    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer3做波特率.
{
    if(brt == 2)
    {
        SetTimer2Baudraye(65536UL - (MAIN_Fosc / 4) / Baudrate3);
        S3CON = 0x10;       //8位数据, 使用Timer2做波特率发生器, 允许接收
    }
    else
    {
        S3CON = 0x50;       //8位数据, 使用Timer3做波特率发生器, 允许接收
        T3H = (65536UL - (MAIN_Fosc / 4) / Baudrate3) / 256;
        T3L = (65536UL - (MAIN_Fosc / 4) / Baudrate3) % 256;
        T4T3M &= ~0x0f;
        T4T3M |= 0x0a;
    }
    IE2 |= 0x08;          //允许UART3中断
    P_SW2 &= ~0x02; 
    P_SW2 |= 0x02;      //UART3 switch bit1 to: 0: P0.0 P0.1,  1: P5.0 P5.1

    B_TX3_Busy = 0;
    TX3_Cnt = 0;
    RX3_Cnt = 0;
}

//========================================================================
// 函数: void UART2_config(u8 brt)
// 描述: UART2初始化函数。
// 参数: brt: 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART2_config(u8 brt)    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
{
    if(brt == 2)
    {
        SetTimer2Baudraye(65536UL - (MAIN_Fosc / 4) / Baudrate2);

        S2CON &= ~(1<<7);   // 8位数据, 1位起始位, 1位停止位, 无校验
        IE2   |= 1;         //允许中断
        S2CON |= (1<<4);    //允许接收
        P_SW2 &= ~0x01; 
        P_SW2 |= 1;         //UART2 switch to: 0: P1.0 P1.1,  1: P4.6 P4.7

        B_TX2_Busy = 0;
        TX2_Cnt = 0;
        RX2_Cnt = 0;
    }
}

//========================================================================
// 函数: void UART2_int (void) interrupt UART2_VECTOR
// 描述: UART2中断函数。
// 参数: nine.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART2_int (void) interrupt 8
{
    if((S2CON & 1) != 0)
    {
        S2CON &= ~1;    //Clear Rx flag
        if(RX2_Cnt >= UART2_BUF_LENGTH)   RX2_Cnt = 0;
        RX2_Buffer[RX2_Cnt] = S2BUF;
        RX2_Cnt++;
        RX2_TimeOut = 5;
    }

    if((S2CON & 2) != 0)
    {
        S2CON &= ~2;    //Clear Tx flag
        B_TX2_Busy = 0;
    }
}

//========================================================================
// 函数: void UART3_int (void) interrupt UART3_VECTOR
// 描述: UART3中断函数。
// 参数: nine.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART3_int (void) interrupt 17
{
    if((S3CON & 0x01) != 0)
    {
        S3CON &= ~0x01;    //Clear Rx flag
        RX3_Buffer[RX3_Cnt] = S3BUF;
        if(++RX3_Cnt >= UART3_BUF_LENGTH)   RX3_Cnt = 0;
        RX3_TimeOut = 5;
    }

    if((S3CON & 0x02) != 0)
    {
        S3CON &= ~0x02;    //Clear Tx flag
        B_TX3_Busy = 0;
    }
}
