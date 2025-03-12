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

本例程基于STC8H8K64U为主控芯片的实验箱9进行编写测试，STC8H系列带DMA模块的芯片可通用参考.

串口2全双工中断方式收发通讯程序。

通过PC向MCU发送数据, MCU将收到的数据自动存入DMA空间.

当DMA空间存满设置大小的内容后，通过串口2的DMA自动发送功能把存储空间的数据原样返回.

用定时器做波特率发生器，建议使用1T模式(除非低波特率用12T)，并选择可被波特率整除的时钟频率，以提高精度。

下载时, 选择时钟 22.1184MHz (用户可自行修改频率).

******************************************/

#include "..\..\comm\STC8H.h"
#include "stdio.h"
#include "intrins.h"

typedef 	unsigned char	u8;
typedef 	unsigned int	u16;
typedef 	unsigned long	u32;

#define MAIN_Fosc   22118400L   //定义主时钟（精确计算115200波特率）
#define Baudrate2   115200L

bit	DmaTxFlag;
bit	DmaRxFlag;

u8 xdata DmaBuffer[256];

void UART2_config(u8 brt);   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
void DMA_Config(void);

void UartPutc(unsigned char dat)
{
	S2BUF = dat; 
	while(!(S2CON & 0x02)); 
	S2CON &= ~0x02;
}
 
char putchar(char c)
{
	UartPutc(c);
	return c;
}

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
	u16 i;

    P_SW2 |= 0x80;  //扩展寄存器(XFR)访问使能

    P0M1 = 0x30;   P0M0 = 0x30;   //设置P0.4、P0.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P1M1 = 0x30;   P1M0 = 0x30;   //设置P1.4、P1.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P2M1 = 0x3c;   P2M0 = 0x3c;   //设置P2.2~P2.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P3M1 = 0x50;   P3M0 = 0x50;   //设置P3.4、P3.6为漏极开路(实验箱加了上拉电阻到3.3V)
    P4M1 = 0x3c;   P4M0 = 0x3c;   //设置P4.2~P4.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P5M1 = 0x0c;   P5M0 = 0x0c;   //设置P5.2、P5.3为漏极开路(实验箱加了上拉电阻到3.3V)
    P6M1 = 0xff;   P6M0 = 0xff;   //设置为漏极开路(实验箱加了上拉电阻到3.3V)
    P7M1 = 0x00;   P7M0 = 0x00;   //设置为准双向口

	for(i=0; i<256; i++)
	{
		DmaBuffer[i] = i;
	}

	UART2_config(2);    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
	printf("STC8H UART2 DMA Test Programme!\r\n");  //UART2发送一个字符串

	DMA_Config();
	EA = 1; //允许总中断

	DmaTxFlag = 0;
	DmaRxFlag = 0;
	
	while (1)
	{
		if((DmaTxFlag) && (DmaRxFlag))
		{
			DmaTxFlag = 0;
			DMA_UR2T_CR = 0xc0;			//bit7 1:使能 UART2_DMA, bit6 1:开始 UART2_DMA 自动发送
			DmaRxFlag = 0;
			DMA_UR2R_CR = 0xa1;			//bit7 1:使能 UART2_DMA, bit5 1:开始 UART2_DMA 自动接收, bit0 1:清除 FIFO
		}
	}
}

//========================================================================
// 函数: void DMA_Config(void)
// 描述: UART DMA 功能配置.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2021-5-6
//========================================================================
void DMA_Config(void)
{
	DMA_UR2T_CFG = 0x80;		//bit7 1:Enable Interrupt
	DMA_UR2T_STA = 0x00;
	DMA_UR2T_AMT = 0xff;		//设置传输总字节数：n+1
	DMA_UR2T_TXAH = (u8)((u16)&DmaBuffer >> 8);
	DMA_UR2T_TXAL = (u8)((u16)&DmaBuffer);
	DMA_UR2T_CR = 0xc0;			//bit7 1:使能 UART2_DMA, bit6 1:开始 UART2_DMA 自动发送

	DMA_UR2R_CFG = 0x80;		//bit7 1:Enable Interrupt
	DMA_UR2R_STA = 0x00;
	DMA_UR2R_AMT = 0xff;		//设置传输总字节数：n+1
	DMA_UR2R_RXAH = (u8)((u16)&DmaBuffer >> 8);
	DMA_UR2R_RXAL = (u8)((u16)&DmaBuffer);
	DMA_UR2R_CR = 0xa1;			//bit7 1:使能 UART2_DMA, bit5 1:开始 UART2_DMA 自动接收, bit0 1:清除 FIFO
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
    IE2  &= ~(1<<2);    //禁止中断
    AUXR |=  (1<<4);    //Timer run enable
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
    /*********** 波特率固定使用定时器2 *****************/
    if(brt == 2)
    {
        SetTimer2Baudraye((u16)(65536UL - (MAIN_Fosc / 4) / Baudrate2));

        S2CON &= ~(1<<7);   //8位数据, 1位起始位, 1位停止位, 无校验
//        IE2   |= 1;         //允许中断
        S2CON |= (1<<4);    //允许接收
        P_SW2 &= ~0x01;     //UART2 switch to: 0: P1.0 P1.1
        P_SW2 |= 1;         //UART2 switch to: 1: P4.6 P4.7
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
//void UART2_int (void) interrupt 8
//{
//    if((S2CON & 1) != 0)
//    {
//        S2CON &= ~1;    //Clear Rx flag
//        RX2_Buffer[RX2_Cnt] = S2BUF;
//        if(++RX2_Cnt >= UART2_BUF_LENGTH)   RX2_Cnt = 0;
//    }

//    if((S2CON & 2) != 0)
//    {
//        S2CON &= ~2;    //Clear Tx flag
//        B_TX2_Busy = 0;
//    }

//}

//========================================================================
// 函数: void UART2_DMA_Interrupt (void) interrupt 52/53
// 描述: UART2 DMA中断函数
// 参数: none.
// 返回: none.
// 版本: VER1.0
// 日期: 2021-5-8
// 备注: 
//========================================================================
void UART2_DMA_Interrupt(void) interrupt 13
{
	if (DMA_UR2T_STA & 0x01)	//发送完成
	{
		DMA_UR2T_STA &= ~0x01;
		DmaTxFlag = 1;
	}
	if (DMA_UR2T_STA & 0x04)	//数据覆盖
	{
		DMA_UR2T_STA &= ~0x04;
	}
	
	if (DMA_UR2R_STA & 0x01)	//接收完成
	{
		DMA_UR2R_STA &= ~0x01;
		DmaRxFlag = 1;
	}
	if (DMA_UR2R_STA & 0x02)	//数据丢弃
	{
		DMA_UR2R_STA &= ~0x02;
	}
}
