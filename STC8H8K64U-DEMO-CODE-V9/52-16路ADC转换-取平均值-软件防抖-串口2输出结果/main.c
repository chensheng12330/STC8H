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

#include "STC8h.h"
#include "stdio.h"
#include "intrins.h"

/*************	功能说明	**************

本程序演示15路ADC和bandgap查询采样，串口2(P4.6 P4.7)发送给上位机，波特率：115200,N,8,1.

0~7通道对应P1.0~P1.7, 8~14通道对应P0.0~P0.6(个别芯片在P3口), 15通道为内部1.19V基准电压做输入的ADC值.

初始化时先把要ADC转换的引脚设置为高阻输入.

上电或复位后先返回一串字符串，然后周期性打印ADC采样结果.

程序提供了两种软件防抖的方式供参考选用：
1. 使用冒泡排序，去掉最高值、最低值，求中间平均值（不需要的话屏蔽 BUBBLE_SORT 定义）
2. 采样累加，求平均值（不需要的话可将 SUM_LENGTH 定义值改为 1 ）

此外STC8H4K、STC8H8K系列芯片还支持扩展寄存器设置硬件自动取平均值功能.

下载时, 选择时钟 22.1184MHz (用户可自行修改频率).

******************************************/

typedef     unsigned char   u8;
typedef     unsigned int    u16;
typedef     unsigned long   u32;

#define MAIN_Fosc     22118400L   //定义主时钟（精确计算115200波特率）
#define Baudrate      115200L
#define TM            (65536 -(MAIN_Fosc/Baudrate/4))
#define PrintUart     2        //1:printf 使用 UART1; 2:printf 使用 UART2

/*************	本地常量声明	**************/

#define ADC_START	(1<<6)	/* 自动清0 */
#define ADC_FLAG	(1<<5)	/* 软件清0 */

#define	ADC_SPEED	15		/* 0~15, ADC转换时间(CPU时钟数) = (n+1)*32  ADCCFG */
#define	RES_FMT		(1<<5)	/* ADC结果格式 0: 左对齐, ADC_RES: D11 D10 D9 D8 D7 D6 D5 D4, ADC_RESL: D3 D2 D1 D0 0 0 0 0 */
							/* ADCCFG     1: 右对齐, ADC_RES: 0 0 0 0 D11 D10 D9 D8, ADC_RESL: D7 D6 D5 D4 D3 D2 D1 D0 */

#define BUBBLE_SORT     //设置使用冒泡排序，去掉最高值、最低值，求中间平均值（不需要的话屏蔽此行）

#define SUM_LENGTH	16	/* 平均值采样次数 最大值16（不需要的话可将定义值改为 1 ）*/

/*************	本地变量声明	**************/

#ifdef BUBBLE_SORT  //使用冒泡排序，去掉最高值、最低值，求中间平均值
u16 xdata ADC_Buffer[16];
#endif

/*************	本地函数声明	**************/

u16		Get_ADC12bitResult(u8 channel);
void	delay_ms(u8 ms);
void	ADC_convert(u8 chn);	//chn=0~7对应P1.0~P1.7, chn=8~14对应P0.0~P0.6, chn=15对应BandGap电压

/******************** 串口打印函数 ********************/
void UartInit(void)
{
#if(PrintUart == 1)
	SCON = (SCON & 0x3f) | 0x40; 
	AUXR |= 0x40;		//定时器时钟1T模式
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TL1  = TM;
	TH1  = TM>>8;
	TR1 = 1;			//定时器1开始计时
//	REN = 1;        //允许接收

//	SCON = (SCON & 0x3f) | 0x40; 
//	T2L  = TM;
//	T2H  = TM>>8;
//	AUXR |= 0x15;   //串口1选择定时器2为波特率发生器
//	REN = 1;        //允许接收
#else
	P_SW2 |= 1;         //UART2 switch to: 0: P1.0 P1.1,  1: P4.6 P4.7
	S2CON &= ~(1<<7);   //8位数据, 1位起始位, 1位停止位, 无校验
	T2L  = TM;
	T2H  = TM>>8;
	AUXR |= 0x14;	    //定时器2时钟1T模式,开始计时
//	S2CON |= (1<<4);    //允许接收
#endif
}

void UartPutc(unsigned char dat)
{
#if(PrintUart == 1)
	SBUF = dat; 
	while(TI==0);
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

/**********************************************/
void main(void)
{
	u8	i;

    P_SW2 |= 0x80;  //扩展寄存器(XFR)访问使能

    P0M1 = 0x7f;   P0M0 = 0x00;   //设置为高阻输入
    P1M1 = 0xff;   P1M0 = 0x00;   //设置为高阻输入
    P2M1 = 0x3c;   P2M0 = 0x3c;   //设置P2.2~P2.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P3M1 = 0x7f;   P3M0 = 0x00;   //设置为高阻输入(个别芯片ADC8~14在P3口)
    P4M1 = 0x3c;   P4M0 = 0x3c;   //设置P4.2~P4.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P5M1 = 0x1c;   P5M0 = 0x0c;   //设置P5.2、P5.3为漏极开路(实验箱加了上拉电阻到3.3V)，P5.4设置为高阻输入
    P6M1 = 0xff;   P6M0 = 0xff;   //设置为漏极开路(实验箱加了上拉电阻到3.3V)
    P7M1 = 0x00;   P7M0 = 0x00;   //设置为准双向口

	//ADC模块电源打开后，需等待1ms，MCU内部ADC电源稳定后再进行AD转换
	ADC_CONTR = 0x80 + 0;	//ADC on + channel (注意：STC8A系列芯片ADC模块电源被打开同时，channel对应的IO口会被强制设置为高阻输入，并关闭其数字功能)
	ADCCFG = RES_FMT + ADC_SPEED;

	ADCTIM = 0x3f;  //设置通道选择时间、保持时间、采样时间

#if 1    //STC8H4K、STC8H8K系列芯片支持扩展寄存器设置硬件自动取平均值功能
//	ADCEXCFG = 0x00;    //转换1次并取平均值
	ADCEXCFG = 0x04;    //转换2次并取平均值
//	ADCEXCFG = 0x05;    //转换4次并取平均值
//	ADCEXCFG = 0x06;    //转换8次并取平均值
//	ADCEXCFG = 0x07;    //转换16次并取平均值
#endif

	UartInit();
	EA = 1;
	printf("STC8系列ADC测试程序!\r\n");

	while (1)
	{
		for(i=0; i<16; i++)
		{
			delay_ms(200);
			//ADC_convert(3);		//发送固定通道AD值

			ADC_convert(i);		//发送轮询通道AD值
			if((i & 7) == 7)	//分两行打印
			{
				printf("\r\n");
			}
		}
	}
}

#ifdef BUBBLE_SORT  //使用冒泡排序
//========================================================================
// 函数: void DataSwap(u16* data1, u16* data2)
// 描述: 数据交换函数。
// 参数: data1,data2 要交换的数据.
// 返回: none.
// 版本: VER1.0
// 日期: 2021-9-27
// 备注: 
//========================================================================
void DataSwap(u16* data1, u16* data2)
{
	u16 temp;
	temp = *data1;
	*data1 = *data2;
	*data2 = temp;
}

//========================================================================
// 函数: void BubbleSort(u16* pDataArry, u8 DataNum)
// 描述: 冒泡排序函数。
// 参数: pDataArry需要排序的数组，DataNum需要排序的数据个数.
// 返回: none.
// 版本: VER1.0
// 日期: 2021-9-27
// 备注: 
//========================================================================
void BubbleSort(u16* pDataArry, u8 DataNum)
{
	bit flag;
	u8 i,j;
	for(i=0;i<DataNum-1;i++)
	{
		flag = 0;
		for(j=0;j<DataNum-i-1;j++)
		{
			if(pDataArry[j] > pDataArry[j+1])
			{
				flag = 1;
				DataSwap(&pDataArry[j], &pDataArry[j+1]);
			}
		}
		if(!flag)  //上一轮比较中不存在数据交换，则退出排序
		{
			break;
		}
	}
}
#endif

//========================================================================
// 函数: u16	Get_ADC12bitResult(u8 channel))	//channel = 0~15
// 描述: 查询法读一次ADC结果.
// 参数: channel: 选择要转换的ADC, 0~15.
// 返回: 12位ADC结果.
// 版本: V1.0, 2016-4-28
//========================================================================
u16	Get_ADC12bitResult(u8 channel)	//channel = 0~15
{
	ADC_RES = 0;
	ADC_RESL = 0;

	ADC_CONTR = 0x80 | ADC_START | channel; 
	NOP(10);			//
	while((ADC_CONTR & ADC_FLAG) == 0)	;	//等待ADC结束
	ADC_CONTR &= ~ADC_FLAG;
	return	((u16)ADC_RES * 256 + (u16)ADC_RESL);
}

/***********************************
查询方式做一次ADC, chn为通道号, chn=0~7对应P1.0~P1.7, chn=8~14对应P0.0~P0.6, chn=15对应BandGap电压.
***********************************/
void	ADC_convert(u8 chn)
{
	u16	j;
	u8	k;		//平均值滤波时使用

	Get_ADC12bitResult(chn);		//参数i=0~15,查询方式做一次ADC, 切换通道后第一次转换结果丢弃. 避免采样电容的残存电压影响.
	Get_ADC12bitResult(chn);		//参数i=0~15,查询方式做一次ADC, 切换通道后第二次转换结果丢弃. 避免采样电容的残存电压影响.

#ifdef BUBBLE_SORT  //使用冒泡排序，去掉最高值、最低值，求中间平均值

	for(k=0; k<16; k++)	ADC_Buffer[k] = Get_ADC12bitResult(chn);
	BubbleSort(ADC_Buffer,16);  //冒泡排序
	for(k=4, j=0; k<12; k++) j += ADC_Buffer[k];  //取中间8个数据
	j = j / 8;		// 求平均

#else   //采样累加，求平均值（不需要的话可将 SUM_LENGTH 定义值改为 1 ）

	for(k=0, j=0; k<SUM_LENGTH; k++)	j += Get_ADC12bitResult(chn);	// 采样累加和 参数0~15,查询方式做一次ADC, 返回值就是结果
	j = j / SUM_LENGTH;		// 求平均

#endif

	if(chn == 15)	printf("Bandgap=%04d  ",j);	//内基准1.35V or 1.19V
	else		//ADC0~ADC14
	{
		printf("ADC%02bd=%04d  ",chn,j);
	}
}

//========================================================================
// 函数: void delay_ms(u8 ms)
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
	do
	{
		i = MAIN_Fosc / 10000;
		while(--i);
	}while(--ms);
}
