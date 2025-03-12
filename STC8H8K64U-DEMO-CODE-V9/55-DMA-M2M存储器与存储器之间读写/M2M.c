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

设置2个存储器空间，一个发送一个接收，分别初始化2个存储器空间内容。

设置M2M DMA，上电后自动将发送存储里的内容写入到接收存储器空间.

根据不同的读取顺序、写入顺序，接收到不同的数据结果.

通过串口2(P4.6 P4.7)打印接收存储器数据(上电打印一次).

下载时, 选择时钟 22.1184MHz (用户可自行修改频率).

******************************************/

#include "STC8H.h"
#include "intrins.h"
#include "stdio.h"

typedef 	unsigned char	u8;
typedef 	unsigned int	u16;
typedef 	unsigned long	u32;

#define MAIN_Fosc     22118400L   //定义主时钟（精确计算115200波特率）
#define Baudrate      115200L
#define TM            (65536 -(MAIN_Fosc/Baudrate/4))

bit	DmaFlag;

u8 xdata DmaTxBuffer[256];
u8 xdata DmaRxBuffer[256];

void DMA_Config(void);

/******************** 串口打印函数 ********************/
void UartInit(void)
{
	P_SW2 |= 1;         //UART2 switch to: 0: P1.0 P1.1,  1: P4.6 P4.7
	S2CON &= ~(1<<7);   //8位数据, 1位起始位, 1位停止位, 无校验
	T2L  = TM;
	T2H  = TM>>8;
	AUXR |= 0x14;	    //定时器2时钟1T模式,开始计时
}

void UartPutc(unsigned char dat)
{
	S2BUF  = dat; 
	while((S2CON & 2) == 0);
	S2CON &= ~2;    //Clear Tx flag
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
		DmaTxBuffer[i] = i;
		DmaRxBuffer[i] = 0;
	}

	UartInit();
	printf("STC8H Memory to Memory DMA Test Programme!\r\n");  //UART发送一个字符串

	DMA_Config();
	EA = 1; //允许总中断

	DmaFlag = 0;

	while (1)
	{
		if(DmaFlag)
		{
			DmaFlag = 0;

			for(i=0; i<256; i++)
			{
				printf("%02bX ", DmaRxBuffer[i]);
				if((i & 0x0f) == 0x0f)
					printf("\r\n");
			}
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
//	DMA_M2M_CFG = 0x80;         //r++ = t++
//	DMA_M2M_STA = 0x00;
//	DMA_M2M_AMT = 0x7f;		//设置传输总字节数：n+1
//	DMA_M2M_TXAH = (u8)((u16)&DmaTxBuffer >> 8);
//	DMA_M2M_TXAL = (u8)((u16)&DmaTxBuffer);
//	DMA_M2M_RXAH = (u8)((u16)&DmaRxBuffer >> 8);
//	DMA_M2M_RXAL = (u8)((u16)&DmaRxBuffer);

//	DMA_M2M_CFG = 0xa0;         //r++ = t--
//	DMA_M2M_STA = 0x00;
//	DMA_M2M_AMT = 0x7f;		//设置传输总字节数：n+1
//	DMA_M2M_TXAH = (u8)((u16)&DmaTxBuffer[255] >> 8);
//	DMA_M2M_TXAL = (u8)((u16)&DmaTxBuffer[255]);
//	DMA_M2M_RXAH = (u8)((u16)&DmaRxBuffer >> 8);
//	DMA_M2M_RXAL = (u8)((u16)&DmaRxBuffer);

//	DMA_M2M_CFG = 0x90;         //r-- = t++
//	DMA_M2M_STA = 0x00;
//	DMA_M2M_AMT = 0x7f;		//设置传输总字节数：n+1
//	DMA_M2M_TXAH = (u8)((u16)&DmaTxBuffer >> 8);
//	DMA_M2M_TXAL = (u8)((u16)&DmaTxBuffer);
//	DMA_M2M_RXAH = (u8)((u16)&DmaRxBuffer[255] >> 8);
//	DMA_M2M_RXAL = (u8)((u16)&DmaRxBuffer[255]);

	DMA_M2M_CFG = 0xb0;         //r-- = t--
	DMA_M2M_STA = 0x00;
	DMA_M2M_AMT = 0x7f;		//设置传输总字节数：n+1
	DMA_M2M_TXAH = (u8)((u16)&DmaTxBuffer[255] >> 8);
	DMA_M2M_TXAL = (u8)((u16)&DmaTxBuffer[255]);
	DMA_M2M_RXAH = (u8)((u16)&DmaRxBuffer[255] >> 8);
	DMA_M2M_RXAL = (u8)((u16)&DmaRxBuffer[255]);
	
//	DMA_M2M_CFG |= 0x0c;
	DMA_M2M_CR = 0xc0;			//bit7 1:使能 M2M_DMA, bit6 1:开始 M2M_DMA 自动接收
}

//========================================================================
// 函数: void M2M_DMA_Interrupt (void) interrupt 47
// 描述: UART1 DMA中断函数
// 参数: none.
// 返回: none.
// 版本: VER1.0
// 日期: 2021-5-8
// 备注: 
//========================================================================
void M2M_DMA_Interrupt(void) interrupt 13
{
	if (DMA_M2M_STA & 0x01)	//发送完成
	{
		DMA_M2M_STA &= ~0x01;
		DmaFlag = 1;
	}
}
