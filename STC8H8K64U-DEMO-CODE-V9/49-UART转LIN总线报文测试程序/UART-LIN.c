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

通过UART接口连接LIN收发器实现LIN总线信号收发测试例程.

UART1通过串口工具连接电脑.

UART2外接LIN收发器(TJA1020/1), 连接LIN总线.

将电脑串口发送的数据转发到LIN总线; 从LIN总线接收到的数据转发到电脑串口.

默认传输速率：9600波特率，发送LIN数据前切换波特率，发送13个显性间隔信号.

下载时, 选择时钟 22.1184MHz (用户可自行修改频率).

******************************************/

#include "stc8h.h"       //包含此头文件后，不需要再包含"reg51.h"头文件
#include "intrins.h"

#define MAIN_Fosc       22118400L   //定义主时钟（精确计算波特率）

typedef     unsigned char   u8;
typedef     unsigned int    u16;
typedef     unsigned long   u32;

sbit SLP_N  = P2^4;     //0: Sleep

/****************************** 用户定义宏 ***********************************/

#define Baudrate1           (65536UL - (MAIN_Fosc / 4) / 9600UL)
#define Baudrate2           (65536UL - (MAIN_Fosc / 4) / 9600UL)  //发送数据传输波特率

#define Baudrate_Break      (65536UL - (MAIN_Fosc / 4) / 6647UL)  //发送显性间隔信号波特率

#define UART1_BUF_LENGTH    32
#define UART2_BUF_LENGTH    32

#define LIN_ID    0x31

u8  TX1_Cnt;    //发送计数
u8  RX1_Cnt;    //接收计数
u8  TX2_Cnt;    //发送计数
u8  RX2_Cnt;    //接收计数
bit B_TX1_Busy; //发送忙标志
bit B_TX2_Busy; //发送忙标志
u8  RX1_TimeOut;
u8  RX2_TimeOut;

u8  xdata RX1_Buffer[UART1_BUF_LENGTH]; //接收缓冲
u8  xdata RX2_Buffer[UART2_BUF_LENGTH]; //接收缓冲

void UART1_config(u8 brt);   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
void UART2_config(u8 brt);   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
void PrintString1(u8 *puts);
void delay_ms(u8 ms);
void UART1_TxByte(u8 dat);
void UART2_TxByte(u8 dat);
void Lin_Send(u8 *puts);
void SetTimer2Baudraye(u16 dat);
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

    UART1_config(1);    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
    UART2_config(2);    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
    EA = 1;             //允许全局中断
    SLP_N = 1;

    PrintString1("STC8H8K64U UART1 Test Programme!\r\n");  //UART1发送一个字符串

    while (1)
    {
        delay_ms(1);
        if(RX1_TimeOut > 0)
        {
            if(--RX1_TimeOut == 0)  //超时,则串口接收结束
            {
                if(RX1_Cnt > 0)
                {
                    Lin_Send(RX1_Buffer);  //将UART1收到的数据发送到LIN总线上
                }
                RX1_Cnt = 0;
            }
        }
				
        if(RX2_TimeOut > 0)
        {
            if(--RX2_TimeOut == 0)  //超时,则串口接收结束
            {
                if(RX2_Cnt > 0)
                {
                    for (i=0; i < RX2_Cnt; i++)     //遇到停止符0结束
                    {
                        UART1_TxByte(RX2_Buffer[i]);  //从LIN总线收到的数据发送到UART1
                    }
                }
                RX2_Cnt = 0;
            }
        }
    }
}


//========================================================================
// 函数: void delay_ms(unsigned char ms)
// 描述: 延时函数。
// 参数: ms,要延时的ms数, 这里只支持1~255ms. 自动适应主时钟.
// 返回: none.
// 版本: VER1.0
// 日期: 2013-4-1
// 备注: 
//========================================================================
void delay_ms(u8 ms)
{
    u16 i;
    do{
        i = MAIN_Fosc / 10000;
        while(--i)    ;   //10T per loop
    }while(--ms);
}

//========================================================================
// 函数: u8 Lin_CheckPID(u8 id)
// 描述: ID码加上校验符，转成PID码。
// 参数: ID码.
// 返回: PID码.
// 版本: VER1.0
// 日期: 2020-12-2
// 备注: 
//========================================================================
u8 Lin_CheckPID(u8 id)
{
	u8 returnpid ;
	u8 P0 ;
	u8 P1 ;
	
	P0 = (((id)^(id>>1)^(id>>2)^(id>>4))&0x01)<<6 ;
	P1 = ((~((id>>1)^(id>>3)^(id>>4)^(id>>5)))&0x01)<<7 ;
	
	returnpid = id|P0|P1 ;
	
	return returnpid ;
}

//========================================================================
// 函数: u8 LINCalcChecksum(u8 *dat)
// 描述: 计算校验码。
// 参数: 数据场传输的数据.
// 返回: 校验码.
// 版本: VER1.0
// 日期: 2020-12-2
// 备注: 
//========================================================================
static u8 LINCalcChecksum(u8 *dat)
{
  u16 sum = 0;
  u8 i;

  for(i = 0; i < 8; i++)
  {
    sum += dat[i];
    if(sum & 0xFF00)
    {
      sum = (sum & 0x00FF) + 1;
    }
  }
  sum ^= 0x00FF;
  return (u8)sum;
}

//========================================================================
// 函数: void Lin_SendBreak(void)
// 描述: 发送显性间隔信号。
// 参数: none.
// 返回: none.
// 版本: VER1.0
// 日期: 2020-12-2
// 备注: 
//========================================================================
void Lin_SendBreak(void)
{
    SetTimer2Baudraye(Baudrate_Break);
    UART2_TxByte(0);
    SetTimer2Baudraye(Baudrate2);
}

//========================================================================
// 函数: void Lin_Send(u8 *puts)
// 描述: 发送LIN总线报文。
// 参数: 待发送的数据场内容.
// 返回: none.
// 版本: VER1.0
// 日期: 2020-12-2
// 备注: 
//========================================================================
void Lin_Send(u8 *puts)
{
    u8 i;

    Lin_SendBreak();			//Break
    UART2_TxByte(0x55);		//SYNC
    UART2_TxByte(Lin_CheckPID(LIN_ID));		//LIN ID
    for(i=0;i<8;i++)
    {
        UART2_TxByte(puts[i]);
    }
    UART2_TxByte(LINCalcChecksum(puts));
}

//========================================================================
// 函数: void UART1_TxByte(u8 dat)
// 描述: 发送一个字节.
// 参数: 无.
// 返回: 无.
// 版本: V1.0, 2014-6-30
//========================================================================
void UART1_TxByte(u8 dat)
{
    SBUF = dat;
    B_TX1_Busy = 1;
    while(B_TX1_Busy);
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
    S2BUF = dat;
    B_TX2_Busy = 1;
    while(B_TX2_Busy);
}

//========================================================================
// 函数: void PrintString1(u8 *puts)
// 描述: 串口1发送字符串函数。
// 参数: puts:  字符串指针.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void PrintString1(u8 *puts)
{
    for (; *puts != 0;  puts++)     //遇到停止符0结束
    {
        SBUF = *puts;
        B_TX1_Busy = 1;
        while(B_TX1_Busy);
    }
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
//void PrintString2(u8 *puts)
//{
//    for (; *puts != 0;  puts++)     //遇到停止符0结束
//    {
//        S2BUF = *puts;
//        B_TX2_Busy = 1;
//        while(B_TX2_Busy);
//    }
//}

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
// 函数: void UART1_config(u8 brt)
// 描述: UART1初始化函数。
// 参数: brt: 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART1_config(u8 brt)
{
    /*********** 波特率使用定时器2 *****************/
    if(brt == 2)
    {
        AUXR |= 0x01;       //S1 BRT Use Timer2;
        SetTimer2Baudraye(Baudrate1);
    }

    /*********** 波特率使用定时器1 *****************/
    else
    {
        TR1 = 0;
        AUXR &= ~0x01;      //S1 BRT Use Timer1;
        AUXR |=  (1<<6);    //Timer1 set as 1T mode
        TMOD &= ~(1<<6);    //Timer1 set As Timer
        TMOD &= ~0x30;      //Timer1_16bitAutoReload;
        TH1 = (u8)(Baudrate1 / 256);
        TL1 = (u8)(Baudrate1 % 256);
        ET1 = 0;    //禁止中断
        INTCLKO &= ~0x02;   //不输出时钟
        TR1  = 1;
    }
    /*************************************************/

    SCON = (SCON & 0x3f) | 0x40;    //UART1模式, 0x00: 同步移位输出, 0x40: 8位数据,可变波特率, 0x80: 9位数据,固定波特率, 0xc0: 9位数据,可变波特率
//  PS  = 1;    //高优先级中断
    ES  = 1;    //允许中断
    REN = 1;    //允许接收
    P_SW1 &= 0x3f;
//  P_SW1 |= 0x80;      //UART1 switch to, 0x00: P3.0 P3.1, 0x40: P3.6 P3.7, 0x80: P1.6 P1.7, 0xC0: P4.3 P4.4

    B_TX1_Busy = 0;
    TX1_Cnt = 0;
    RX1_Cnt = 0;
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
        SetTimer2Baudraye(Baudrate2);

        S2CON &= ~(1<<7);   // 8位数据, 1位起始位, 1位停止位, 无校验
        IE2   |= 1;         //允许中断
        S2CON |= (1<<4);    //允许接收
        P_SW2 &= ~0x01; 
//        P_SW2 |= 1;         //UART2 switch to: 0: P1.0 P1.1,  1: P4.6 P4.7

        B_TX2_Busy = 0;
        TX2_Cnt = 0;
        RX2_Cnt = 0;
    }
}

//========================================================================
// 函数: void UART1_int (void) interrupt UART1_VECTOR
// 描述: UART1中断函数。
// 参数: nine.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART1_int (void) interrupt 4
{
    if(RI)
    {
        RI = 0;
        if(RX1_Cnt >= UART1_BUF_LENGTH) RX1_Cnt = 0;
        RX1_Buffer[RX1_Cnt] = SBUF;
        RX1_Cnt++;
        RX1_TimeOut = 5;
    }

    if(TI)
    {
        TI = 0;
        B_TX1_Busy = 0;
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
        if(RX2_Cnt >= UART2_BUF_LENGTH) RX2_Cnt = 0;
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
