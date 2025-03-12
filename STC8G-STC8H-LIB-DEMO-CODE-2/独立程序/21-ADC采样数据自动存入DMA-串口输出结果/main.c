/*---------------------------------------------------------------------*/
/* --- STC MCU Limited ------------------------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.STCAI.com ---------------------------------------------*/
/* --- BBS: www.STCAIMCU.com  -----------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* 如果要在程序中使用此代码,请在程序中注明使用了STC的资料及程序            */
/*---------------------------------------------------------------------*/

#include	"config.h"
#include	"STC8G_H_GPIO.h"
#include	"STC8G_H_ADC.h"
#include	"STC8G_H_UART.h"
#include	"STC8H_DMA.h"
#include	"STC8G_H_NVIC.h"
#include	"STC8G_H_Delay.h"
#include	"STC8G_H_Switch.h"

/*************   功能说明   ***************

本程序演示多路ADC DMA采样.

初始化时先把要ADC转换的引脚设置为高阻输入.

设置数据批量传输(DMA)功能，所有通道一次循环采集的数据自动存放到DMA定义的xdata空间.

通过串口1(P3.0 P3.1)将DMA定义的xdata空间所收到的数据发送给上位机，波特率115200,N,8,1

用定时器做波特率发生器，建议使用1T模式(除非低波特率用12T)，并选择可被波特率整除的时钟频率，以提高精度.

下载时, 选择时钟 22.1184MHz (可以在配置文件"config.h"中修改).

******************************************/

/*************	本地常量声明	**************/

#define	ADC_CH		16			/* 1~16, ADC转换通道数, 需同步修改转换通道 */
#define	ADC_DATA	12			/* 6~n, 每个通道ADC转换数据总数, 2*转换次数+4, 需同步修改转换次数 */

/*************	本地变量声明	**************/

u8 chn = 0;
u8 xdata DmaAdBuffer[ADC_CH][ADC_DATA];

/*************	本地函数声明	**************/


/*************  外部函数和变量声明 *****************/


/******************** IO口配置 ********************/
void	GPIO_config(void)
{
	P0_MODE_IN_HIZ(GPIO_Pin_LOW | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6);	//P0.0~P0.6 设置为高阻输入
	P1_MODE_IN_HIZ(GPIO_Pin_All);		//P1.0~P1.7 设置为高阻输入
}

/******************** UART配置 ********************/
void	UART_config(void)
{
	COMx_InitDefine		COMx_InitStructure;		//结构定义

	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;	//模式,   UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;		//选择波特率发生器, BRT_Timer1,BRT_Timer2 (注意: 串口2固定使用BRT_Timer2)
	COMx_InitStructure.UART_BaudRate  = 115200ul;		//波特率,     110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;			//接收允许,   ENABLE 或 DISABLE
	UART_Configuration(UART1, &COMx_InitStructure);		//初始化串口 UART1,UART2,UART3,UART4
	NVIC_UART1_Init(ENABLE,Priority_1);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

	UART1_SW(UART1_SW_P30_P31);		//UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17,UART1_SW_P43_P44
}

/******************** ADC 配置 ********************/
void	ADC_config(void)
{
	ADC_InitTypeDef		ADC_InitStructure;		//结构定义

	ADC_InitStructure.ADC_SMPduty   = 31;		//ADC 模拟信号采样时间控制, 0~31（注意： SMPDUTY 一定不能设置小于 10）
	ADC_InitStructure.ADC_CsSetup   = 0;		//ADC 通道选择时间控制 0(默认),1
	ADC_InitStructure.ADC_CsHold    = 1;		//ADC 通道选择保持时间控制 0,1(默认),2,3
	ADC_InitStructure.ADC_Speed     = ADC_SPEED_2X16T;		//设置 ADC 工作时钟频率	ADC_SPEED_2X1T~ADC_SPEED_2X16T
	ADC_InitStructure.ADC_AdjResult = ADC_RIGHT_JUSTIFIED;	//ADC结果调整,	ADC_LEFT_JUSTIFIED,ADC_RIGHT_JUSTIFIED
	ADC_Inilize(&ADC_InitStructure);		//初始化
	ADC_PowerControl(ENABLE);				//ADC电源开关, ENABLE或DISABLE
	NVIC_ADC_Init(DISABLE,Priority_0);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
}

/******************** DMA 配置 ********************/
void	DMA_config(void)
{
	DMA_ADC_InitTypeDef		DMA_ADC_InitStructure;		//结构定义

	DMA_ADC_InitStructure.DMA_Enable = ENABLE;			//DMA使能  	ENABLE,DISABLE
	DMA_ADC_InitStructure.DMA_Channel = 0xffff;			//ADC通道使能寄存器, 1:使能, bit15~bit0 对应 ADC15~ADC0
	DMA_ADC_InitStructure.DMA_Buffer = (u16)DmaAdBuffer;	//ADC转换数据存储地址
	DMA_ADC_InitStructure.DMA_Times = ADC_4_Times;	//每个通道转换次数, ADC_1_Times,ADC_2_Times,ADC_4_Times,ADC_8_Times,ADC_16_Times,ADC_32_Times,ADC_64_Times,ADC_128_Times,ADC_256_Times
	DMA_ADC_Inilize(&DMA_ADC_InitStructure);		//初始化
	NVIC_DMA_ADC_Init(ENABLE,Priority_0,Priority_0);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0~Priority_3; 总线优先级(低到高) Priority_0~Priority_3
	DMA_ADC_TRIG();		//触发启动转换
}

/******************** task A **************************/
void main(void)
{
	u8	i,n;

	EAXSFR();		//扩展SFR(XFR)访问使能 
	GPIO_config();
	UART_config();
	ADC_config();
	DMA_config();
	EA = 1;
	
	while (1)
	{
		delay_ms(200);	//200ms输出一次采样结果，方便观察
		
		if(DmaADCFlag)  //判断ADC DMA采样是否完成
		{
			DmaADCFlag = 0; //清除完成标志
			for(i=0; i<ADC_CH; i++)     //打印每个设置通道的采样结果
			{
				for(n=0; n<ADC_DATA; n++)   //打印当前通道的采样数据
				{
					printf("0x%02bx ",DmaAdBuffer[i][n]);	//第1组数据,...,第n组数据,AD通道,平均余数,平均值
				}
				printf("\r\n"); //串口输出回车换行符
			}
			printf("\r\n");     //串口输出回车换行符
			DMA_ADC_TRIG();		//重新触发启动下一次转换
		}
	}
}



