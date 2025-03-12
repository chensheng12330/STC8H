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


/*************  本程序功能说明  **************

本例程基于STC8H8K64U为主控芯片的实验箱9进行编写测试，带MDU16模块的芯片可通用参考.

使用芯片内部集成的16位硬件乘除法器模块进行数学计算.

支持的数据运算如下：
1. 数据规格化；
2. 逻辑左移；
3. 逻辑右移；
4. 16位乘以16位
5. 16位除以16位；
6. 32位除以16位。
所有操作都是基于无符号整形数据类型。

下载时, 选择时钟 24MHZ (用户可自行修改频率).

******************************************/

#include "stc8h.h"       //包含此头文件后，不需要再包含"reg51.h"头文件
#include "intrins.h"
#include "stdio.h"

/****************************** 用户定义宏 ***********************************/

#define MAIN_Fosc     24000000L   //定义主时钟
#define PrintUart     1        //1:printf 使用 UART1; 2:printf 使用 UART2
#define Baudrate      115200L
#define TM            (65536 -(MAIN_Fosc/Baudrate/4))

/*****************************************************************************/

unsigned long res32;
unsigned int res16;
unsigned int dat1,dat2;
signed int dat4,dat5;
signed int sig16;

unsigned long dat3;
unsigned char num;

/*****************************************************************************/

void UartInit(void)
{
#if(PrintUart == 1)
	SCON = (SCON & 0x3f) | 0x40; 
	AUXR |= 0x40;		//定时器时钟1T模式
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TL1  = TM;
	TH1  = TM>>8;
	TR1 = 1;				//定时器1开始计时

//	SCON = (SCON & 0x3f) | 0x40; 
//	T2L  = TM;
//	T2H  = TM>>8;
//	AUXR |= 0x15;   //串口1选择定时器2为波特率发生器
#else
	P_SW2 |= 1;         //UART2 switch to: 0: P1.0 P1.1,  1: P4.6 P4.7
	S2CON &= ~(1<<7);   //8位数据, 1位起始位, 1位停止位, 无校验
	T2L  = TM;
	T2H  = TM>>8;
	AUXR |= 0x14;	      //定时器2时钟1T模式,开始计时
#endif
}
 
void UartPutc(unsigned char dat)
{
#if(PrintUart == 1)
	SBUF = dat; 
	while(TI == 0);
	TI = 0;
#else
	S2BUF  = dat; 
	while((S2CON & 2) == 0);
	S2CON &= ~2;    //Clear Tx flag
#endif
}
 
char putchar(char c)
{
	UartPutc(c);
	return c;
}


void delay_ms(unsigned char ms)
{
    unsigned int i;
    do{
        i = MAIN_Fosc / 10000;
        while(--i);
    }while(--ms);
}

void main(void)
{
    P_SW2 |= 0x80;  //扩展寄存器(XFR)访问使能

    P0M1 = 0x30;   P0M0 = 0x30;   //设置P0.4、P0.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P1M1 = 0x30;   P1M0 = 0x30;   //设置P1.4、P1.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P2M1 = 0x3c;   P2M0 = 0x3c;   //设置P2.2~P2.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P3M1 = 0xf0;   P3M0 = 0x70;   //设置P3.4、P3.5、P3.6为漏极开路(实验箱加了上拉电阻到3.3V), P3.7设置高阻输入
    P4M1 = 0x3c;   P4M0 = 0x3c;   //设置P4.2~P4.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P5M1 = 0x0c;   P5M0 = 0x0c;   //设置P5.2、P5.3为漏极开路(实验箱加了上拉电阻到3.3V)
    P6M1 = 0xff;   P6M0 = 0xff;   //设置为漏极开路(实验箱加了上拉电阻到3.3V)
    P7M1 = 0x00;   P7M0 = 0x00;   //设置为准双向口

	P10 = 0;
	delay_ms(1);
	P10 = 1;
	delay_ms(1);

	UartInit();
	
	P10 = 0;

	//32位左移模式
	res32 = 0x87654321;
	res32 <<= 3;
	printf("<<3=%lx (%lx))\r\n",res32, (0x87654321UL << 3));
	
	//32位右移模式
	res32 = 0x87654321;
	res32 >>= 5;
	printf(">>5=%lx (%lx)\r\n",res32, (0x87654321UL >> 5));
	
	//16位 * 16位
	dat1 = 0x4321;
	dat2 = 0x5678;
	res32 = dat1 * dat2;        //注意此处KEIL会将结果自动截取为16位
	printf("%x*%x=%lx (%lx)\r\n",dat1,dat2,res32, 0x4321UL*0x5678);
	
	//16位 / 16位
	dat1 = 0xabcd;
	dat2 = 0x0123;
	res16 = dat1 / dat2;
	printf("%x/%x=%x (%x)\r\n",dat1,dat2,res16, (int)(0xabcd/0x0123));
	
	//16位 % 16位
	dat1 = 0xabcd;
	dat2 = 0x0123;
	res16 = dat1 % dat2;
	printf("%x%%%x=%x (%x)\r\n",dat1,dat2,res16, (int)(0xabcd%0x0123));

	P10 = 1;

	//16位 / 16位
	dat5 = 9876;
	dat4 = -54;
	sig16 = dat5 / dat4;        //硬件库所有操作都是基于无符号整形数据类型，此计算调用标准函数库
	printf("%x/%x=%x\r\n",dat5,dat4,sig16);
	
	while(1);
}
