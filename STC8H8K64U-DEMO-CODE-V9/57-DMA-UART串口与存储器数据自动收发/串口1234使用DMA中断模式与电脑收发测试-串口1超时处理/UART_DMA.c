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

串口1,2,3,4全双工中断方式收发通讯程序。

通过PC向MCU发送数据, MCU将收到的数据自动存入DMA空间.

当DMA空间存满设置大小的内容后，通过串口的DMA自动发送功能把存储空间的数据原样返回.

利用串口接收中断进行超时判断，超时没有收到新的数据，表示一串数据已经接收完毕.

用定时器做波特率发生器，建议使用1T模式(除非低波特率用12T)，并选择可被波特率整除的时钟频率，以提高精度。

下载时, 选择时钟 22.1184MHz (用户可自行修改频率).

******************************************/

#include "stdio.h"
#include "stc8h.h"

typedef 	unsigned char	u8;
typedef 	unsigned int	u16;
typedef 	unsigned long	u32;

#define MAIN_Fosc       22118400L   //定义主时钟（精确计算115200波特率）
#define Baudrate1       115200L
#define Baudrate2       115200L
#define Baudrate3       115200L
#define Baudrate4       115200L
#define Timer0_Reload   (65536UL -(MAIN_Fosc / 1000))       //Timer 0 中断频率, 1000次/秒

#define DMA_TX_LEN      255  //设置传输总字节数：n+1
#define DMA_RX_LEN      255  //设置传输总字节数：n+1

bit B_1ms;          //1ms标志
bit	DmaTx1Flag;
bit	DmaRx1Flag;
bit	DmaTx2Flag;
bit	DmaRx2Flag;
bit	DmaTx3Flag;
bit	DmaRx3Flag;
bit	DmaTx4Flag;
bit	DmaRx4Flag;

bit	Tx1SendFlag;
u8  RX1_TimeOut;
u16 RX1_Cnt;    //接收计数

u8 xdata DmaBuffer1[256];
u8 xdata DmaBuffer2[256];
u8 xdata DmaBuffer3[256];
u8 xdata DmaBuffer4[256];

void UART1_config(u8 brt);   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
void UART2_config(u8 brt);   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
void UART3_config(u8 brt);   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer3做波特率.
void UART4_config(u8 brt);   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer4做波特率.
void UART1_DMA_Config(void);
void UART2_DMA_Config(void);
void UART3_DMA_Config(void);
void UART4_DMA_Config(void);

void UART1_DMA_Transmit(u8 *pData, u16 Size);
void UART1_DMA_Receive(u8 *pData, u16 Size);

void UartPutc(unsigned char dat)
{
    Tx1SendFlag = 1;
	SBUF = dat; 
	while(Tx1SendFlag); 
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
		DmaBuffer1[i] = i;
		DmaBuffer2[i] = 255-i;
		DmaBuffer3[i] = 0;
		DmaBuffer4[i] = 255;
	}

	AUXR = 0x80;    //Timer0 set as 1T, 16 bits timer auto-reload, 
	TH0 = (u8)(Timer0_Reload / 256);
	TL0 = (u8)(Timer0_Reload % 256);
	ET0 = 1;    //Timer0 interrupt enable
	TR0 = 1;    //Tiner0 run
	
	UART1_config(1);    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
	UART2_config(2);    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
	UART3_config(0);    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer3做波特率.
	UART4_config(0);    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer4做波特率.

	UART1_DMA_Config();
	UART2_DMA_Config();
	UART3_DMA_Config();
	UART4_DMA_Config();
	EA = 1; //允许总中断

	DmaTx1Flag = 0;
	DmaRx1Flag = 0;
	DmaTx2Flag = 0;
	DmaRx2Flag = 0;
	DmaTx3Flag = 0;
	DmaRx3Flag = 0;
	DmaTx4Flag = 0;
	DmaRx4Flag = 0;

//	printf("STC8H UART DMA Test Programme!\r\n");  //UART1发送一个字符串
	while (1)
	{
		if(B_1ms)   //1ms到
		{
			B_1ms = 0;
			if(RX1_TimeOut > 0)     //超时计数
			{
				if(--RX1_TimeOut == 0)
				{
                    //关闭接收DMA，下次接收的数据重新存放在起始地址位置，否则下次接收数据继续往后面存放。
					DMA_UR1R_CR = 0x00;			//bit7 1:使能 UART1_DMA, bit5 1:开始 UART1_DMA 自动接收, bit0 1:清除 FIFO

					printf("\r\nUART1 Timeout!\r\n");  //UART1发送一个字符串
                    UART1_DMA_Transmit(DmaBuffer1,RX1_Cnt);  //设置DMA发送缓冲区，数据长度，并启动发送
                    RX1_Cnt = 0;
                    DmaTx1Flag = 0;
                    DmaRx1Flag = 0;
                    UART1_DMA_Receive(DmaBuffer1,DMA_RX_LEN+1);  //设置DMA接收缓冲区，数据长度，并启动接收
				}
			}
		}

		if((DmaTx1Flag) && (DmaRx1Flag))
		{
			RX1_TimeOut = 0;
			printf("\r\nUART1 DMA FULL!\r\n");  //UART1发送一个字符串
			UART1_DMA_Transmit(DmaBuffer1,RX1_Cnt);  //设置DMA发送缓冲区，数据长度，并启动发送
			RX1_Cnt = 0;
			DmaTx1Flag = 0;
			DmaRx1Flag = 0;
			UART1_DMA_Receive(DmaBuffer1,DMA_RX_LEN+1);  //设置DMA接收缓冲区，数据长度，并启动接收
		}

		if((DmaTx2Flag) && (DmaRx2Flag))
		{
			DmaTx2Flag = 0;
			DMA_UR2T_CR = 0xc0;			//bit7 1:使能 UART2_DMA, bit6 1:开始 UART2_DMA 自动发送
			DmaRx2Flag = 0;
			DMA_UR2R_CR = 0xa1;			//bit7 1:使能 UART2_DMA, bit5 1:开始 UART2_DMA 自动接收, bit0 1:清除 FIFO
		}

		if((DmaTx3Flag) && (DmaRx3Flag))
		{
			DmaTx3Flag = 0;
			DMA_UR3T_CR = 0xc0;			//bit7 1:使能 UART3_DMA, bit6 1:开始 UART3_DMA 自动发送
			DmaRx3Flag = 0;
			DMA_UR3R_CR = 0xa1;			//bit7 1:使能 UART3_DMA, bit5 1:开始 UART3_DMA 自动接收, bit0 1:清除 FIFO
		}

		if((DmaTx4Flag) && (DmaRx4Flag))
		{
			DmaTx4Flag = 0;
			DMA_UR4T_CR = 0xc0;			//bit7 1:使能 UART4_DMA, bit6 1:开始 UART4_DMA 自动发送
			DmaRx4Flag = 0;
			DMA_UR4R_CR = 0xa1;			//bit7 1:使能 UART4_DMA, bit5 1:开始 UART4_DMA 自动接收, bit0 1:清除 FIFO
		}
	}
}

//========================================================================
// 函数: void UART1_DMA_Config(void)
// 描述: UART1 DMA 功能配置.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2021-5-6
//========================================================================
void UART1_DMA_Config(void)
{
	DMA_UR1T_CFG = 0x80;		//bit7 1:Enable Interrupt
	DMA_UR1T_STA = 0x00;
	DMA_UR1T_AMT = DMA_TX_LEN;		//设置传输总字节数：n+1
	DMA_UR1T_TXA = DmaBuffer1;
	DMA_UR1T_CR = 0xc0;			//bit7 1:使能 UART1_DMA, bit6 1:开始 UART1_DMA 自动发送

	DMA_UR1R_CFG = 0x80;		//bit7 1:Enable Interrupt
	DMA_UR1R_STA = 0x00;
	DMA_UR1R_AMT = DMA_RX_LEN;		//设置传输总字节数：n+1
	DMA_UR1R_RXA = DmaBuffer1;
	DMA_UR1R_CR = 0xa1;			//bit7 1:使能 UART1_DMA, bit5 1:开始 UART1_DMA 自动接收, bit0 1:清除 FIFO
}

void UART1_DMA_Transmit(u8 *pData, u16 Size)
{
    if(Size == 0) return;
    Size -= 1;
    DMA_UR1T_AMT = (u8)Size;        //设置传输总字节数：n+1
    DMA_UR1T_TXA = (u16)pData;
    DMA_UR1T_CR = 0xc0;     //bit7 1:使能 UART4_DMA, bit6 1:开始 UART1_DMA 自动发送
}

void UART1_DMA_Receive(u8 *pData, u16 Size)
{
    if(Size == 0) return;
    Size -= 1;
    DMA_UR1R_AMT = (u8)Size;        //设置传输总字节数：n+1
    DMA_UR1R_RXA = (u16)pData;
    DMA_UR1R_CR = 0xa1;     //bit7 1:使能 UART4_DMA, bit5 1:开始 UART4_DMA 自动接收, bit0 1:清除 FIFO
}

//========================================================================
// 函数: void UART2_DMA_Config(void)
// 描述: UART2 DMA 功能配置.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2021-5-6
//========================================================================
void UART2_DMA_Config(void)
{
	DMA_UR2T_CFG = 0x80;		//bit7 1:Enable Interrupt
	DMA_UR2T_STA = 0x00;
	DMA_UR2T_AMT = DMA_TX_LEN;		//设置传输总字节数：n+1
	DMA_UR2T_TXA = DmaBuffer2;
	DMA_UR2T_CR = 0xc0;			//bit7 1:使能 UART2_DMA, bit6 1:开始 UART2_DMA 自动发送

	DMA_UR2R_CFG = 0x80;		//bit7 1:Enable Interrupt
	DMA_UR2R_STA = 0x00;
	DMA_UR2R_AMT = DMA_RX_LEN;		//设置传输总字节数：n+1
	DMA_UR2R_RXA = DmaBuffer2;
	DMA_UR2R_CR = 0xa1;			//bit7 1:使能 UART2_DMA, bit5 1:开始 UART2_DMA 自动接收, bit0 1:清除 FIFO
}

//========================================================================
// 函数: void UART3_DMA_Config(void)
// 描述: UART3 DMA 功能配置.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2021-5-6
//========================================================================
void UART3_DMA_Config(void)
{
	DMA_UR3T_CFG = 0x80;		//bit7 1:Enable Interrupt
	DMA_UR3T_STA = 0x00;
	DMA_UR3T_AMT = DMA_TX_LEN;		//设置传输总字节数：n+1
	DMA_UR3T_TXA = DmaBuffer3;
	DMA_UR3T_CR = 0xc0;			//bit7 1:使能 UART3_DMA, bit6 1:开始 UART3_DMA 自动发送

	DMA_UR3R_CFG = 0x80;		//bit7 1:Enable Interrupt
	DMA_UR3R_STA = 0x00;
	DMA_UR3R_AMT = DMA_RX_LEN;		//设置传输总字节数：n+1
	DMA_UR3R_RXA = DmaBuffer3;
	DMA_UR3R_CR = 0xa1;			//bit7 1:使能 UART3_DMA, bit5 1:开始 UART3_DMA 自动接收, bit0 1:清除 FIFO
}

//========================================================================
// 函数: void UART4_DMA_Config(void)
// 描述: UART4 DMA 功能配置.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2021-5-6
//========================================================================
void UART4_DMA_Config(void)
{
	DMA_UR4T_CFG = 0x80;		//bit7 1:Enable Interrupt
	DMA_UR4T_STA = 0x00;
	DMA_UR4T_AMT = DMA_TX_LEN;		//设置传输总字节数：n+1
	DMA_UR4T_TXA = DmaBuffer4;
	DMA_UR4T_CR = 0xc0;			//bit7 1:使能 UART4_DMA, bit6 1:开始 UART4_DMA 自动发送

	DMA_UR4R_CFG = 0x80;		//bit7 1:Enable Interrupt
	DMA_UR4R_STA = 0x00;
	DMA_UR4R_AMT = DMA_RX_LEN;		//设置传输总字节数：n+1
	DMA_UR4R_RXA = DmaBuffer4;
	DMA_UR4R_CR = 0xa1;			//bit7 1:使能 UART4_DMA, bit5 1:开始 UART4_DMA 自动接收, bit0 1:清除 FIFO
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
void SetTimer2Baudraye(u16 dat)  // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
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
void UART1_config(u8 brt)    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
{
	/*********** 波特率使用定时器2 *****************/
	if(brt == 2)
	{
		AUXR |= 0x01;       //S1 BRT Use Timer2;
		SetTimer2Baudraye(65536UL - (MAIN_Fosc / 4) / Baudrate1);
	}

	/*********** 波特率使用定时器1 *****************/
	else
	{
		TR1 = 0;
		AUXR &= ~0x01;      //S1 BRT Use Timer1;
		AUXR |=  (1<<6);    //Timer1 set as 1T mode
		TMOD &= ~(1<<6);    //Timer1 set As Timer
		TMOD &= ~0x30;      //Timer1_16bitAutoReload;
		TH1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) / 256);
		TL1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) % 256);
		ET1 = 0;    //禁止中断
		TR1  = 1;
	}
	/*************************************************/

	SCON = (SCON & 0x3f) | 0x40;    //UART1模式, 0x00: 同步移位输出, 0x40: 8位数据,可变波特率, 0x80: 9位数据,固定波特率, 0xc0: 9位数据,可变波特率
//  PS  = 1;    //高优先级中断
	ES  = 1;    //允许中断
	REN = 1;    //允许接收
	P_SW1 &= 0x3f;
	P_SW1 |= 0x00;      //UART1 switch to, 0x00: P3.0 P3.1, 0x40: P3.6 P3.7, 0x80: P1.6 P1.7, 0xC0: P4.3 P4.4

	RX1_TimeOut = 0;
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
        SetTimer2Baudraye(65536UL - (MAIN_Fosc / 4) / Baudrate2);

        S2CON &= ~(1<<7);   // 8位数据, 1位起始位, 1位停止位, 无校验
//        IE2   |= 1;         //允许中断
        S2CON |= (1<<4);    //允许接收
        P_SW2 &= ~0x01;     //UART2 switch to: 0: P1.0 P1.1
//        P_SW2 |= 1;         //UART2 switch to: 1: P4.6 P4.7
    }
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
		T4T3M &= 0xf0;
		T4T3M |= 0x0a;
	}
	//IE2 |= 0x08;          //允许UART3中断
	P_SW2 &= ~0x02;       //UART3 switch bit1 to: 0: P0.0 P0.1
	//P_SW2 |= 0x02;      //UART3 switch bit1 to: 1: P5.0 P5.1
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
		T4T3M &= 0x0f;
		T4T3M |= 0xa0;
	}
	//IE2 |= 0x10;          //允许UART4中断
	P_SW2 &= ~0x04;       //UART4 switch bit2 to: 0: P0.2 P0.3
	P_SW2 |= 0x04;      //UART4 switch bit2 to: 1: P5.2 P5.3
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
        RX1_Cnt++;
		RX1_TimeOut = 5;    //如果5ms没收到新的数据，判定一串数据接收完毕
	}

	if(TI)
	{
		TI = 0;
        Tx1SendFlag = 0;
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
// 函数: void UART3_int (void) interrupt UART3_VECTOR
// 描述: UART3中断函数。
// 参数: nine.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
//void UART3_int (void) interrupt 17
//{
//    if((S3CON & 0x01) != 0)
//    {
//        S3CON &= ~0x01;    //Clear Rx flag
//        RX3_Buffer[RX3_Cnt] = S3BUF;
//        if(++RX3_Cnt >= UART3_BUF_LENGTH)   RX3_Cnt = 0;
//    }

//    if((S3CON & 0x02) != 0)
//    {
//        S3CON &= ~0x02;    //Clear Tx flag
//        B_TX3_Busy = 0;
//    }
//}

//========================================================================
// 函数: void UART4_int (void) interrupt UART4_VECTOR
// 描述: UART4中断函数。
// 参数: nine.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
//void UART4_int (void) interrupt 18
//{
//    if((S4CON & 0x01) != 0)
//    {
//        S4CON &= ~0x01;    //Clear Rx flag
//        RX4_Buffer[RX4_Cnt] = S4BUF;
//        if(++RX4_Cnt >= UART4_BUF_LENGTH)   RX4_Cnt = 0;
//    }

//    if((S4CON & 0x02) != 0)
//    {
//        S4CON &= ~0x02;    //Clear Tx flag
//        B_TX4_Busy = 0;
//    }
//}

//========================================================================
// 函数: void timer0 (void) interrupt 1
// 描述: 定时器0中断函数。
// 参数: nine.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void timer0 (void) interrupt 1
{
    B_1ms = 1;      //1ms标志
}

//========================================================================
// 函数: void UART_DMA_Interrupt (void)
// 描述: UART DMA中断函数
// 参数: none.
// 返回: none.
// 版本: VER1.0
// 日期: 2021-5-8
// 备注: 
//========================================================================
void UART_DMA_Interrupt(void) interrupt 13
{
	//======= UART1 DMA ================
	if (DMA_UR1T_STA & 0x01)	//发送完成
	{
		DMA_UR1T_STA &= ~0x01;
		DmaTx1Flag = 1;
	}
	if (DMA_UR1T_STA & 0x04)	//数据覆盖
	{
		DMA_UR1T_STA &= ~0x04;
	}
	
	if (DMA_UR1R_STA & 0x01)	//接收完成
	{
		DMA_UR1R_STA &= ~0x01;
		DmaRx1Flag = 1;
	}
	if (DMA_UR1R_STA & 0x02)	//数据丢弃
	{
		DMA_UR1R_STA &= ~0x02;
	}
	//======= UART2 DMA ================
	if (DMA_UR2T_STA & 0x01)	//发送完成
	{
		DMA_UR2T_STA &= ~0x01;
		DmaTx2Flag = 1;
	}
	if (DMA_UR2T_STA & 0x04)	//数据覆盖
	{
		DMA_UR2T_STA &= ~0x04;
	}
	
	if (DMA_UR2R_STA & 0x01)	//接收完成
	{
		DMA_UR2R_STA &= ~0x01;
		DmaRx2Flag = 1;
	}
	if (DMA_UR2R_STA & 0x02)	//数据丢弃
	{
		DMA_UR2R_STA &= ~0x02;
	}
	//======= UART3 DMA ================
	if (DMA_UR3T_STA & 0x01)	//发送完成
	{
		DMA_UR3T_STA &= ~0x01;
		DmaTx3Flag = 1;
	}
	if (DMA_UR3T_STA & 0x04)	//数据覆盖
	{
		DMA_UR3T_STA &= ~0x04;
	}
	
	if (DMA_UR3R_STA & 0x01)	//接收完成
	{
		DMA_UR3R_STA &= ~0x01;
		DmaRx3Flag = 1;
	}
	if (DMA_UR3R_STA & 0x02)	//数据丢弃
	{
		DMA_UR3R_STA &= ~0x02;
	}
	//======= UART4 DMA ================
	if (DMA_UR4T_STA & 0x01)	//发送完成
	{
		DMA_UR4T_STA &= ~0x01;
		DmaTx4Flag = 1;
	}
	if (DMA_UR4T_STA & 0x04)	//数据覆盖
	{
		DMA_UR4T_STA &= ~0x04;
	}
	
	if (DMA_UR4R_STA & 0x01)	//接收完成
	{
		DMA_UR4R_STA &= ~0x01;
		DmaRx4Flag = 1;
	}
	if (DMA_UR4R_STA & 0x02)	//数据丢弃
	{
		DMA_UR4R_STA &= ~0x02;
	}
}
