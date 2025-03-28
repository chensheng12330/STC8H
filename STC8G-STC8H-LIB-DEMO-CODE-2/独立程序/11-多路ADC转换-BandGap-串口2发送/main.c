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
#include	"STC8G_H_ADC.h"
#include	"STC8G_H_GPIO.h"
#include	"STC8G_H_Delay.h"
#include	"STC8G_H_UART.h"
#include	"STC8G_H_NVIC.h"
#include	"STC8G_H_Switch.h"

/*************	功能说明	**************

本例程基于STC8H8K64U为主控芯片的实验箱8进行编写测试，STC8G、STC8H系列芯片可通用参考.

程序演示多路ADC查询采样，通过串口2发送给上位机，波特率115200,N,8,1。

说明:
    ADC的第15通道是用来测试内部BandGap参考电压的,由于内部BandGap参考电
压很稳定,不会随芯片的工作电压的改变而变化,所以可以通过测量内部BandGap
参考电压,然后通过ADC的值便可反推出VCC的电压,从而用户可以实现自己的低
压检测功能.
    ADC的第15通道的测量方法:通过ADC的第15通道读取当前内部BandGap参考电压值.
    用户实现自己的低压检测功能的实现方法:首先用户需要在VCC很精准的情况下
(比如5.0V),测量出内部BandGap参考电压的ADC转换值(比如为BGV5),并这个值保存
到EEPROM中,然后在低压检测的代码中,在实际VCC变化后,所测量出的内部BandGap
参考电压的ADC转换值(比如为BGVx),通过计算公式: 实际VCC = 5.0V * BGV5 / BGVx,
即可计算出实际的VCC电压值,需要注意的是,第一步的BGV5的基准测量一定要精确.

标定内部基准Vref, 提供一个稳定的工作电压Vcc, 读内部基准Nref. 
计算内部基准(10位AD) Vref = Nref * Vcc / 1024.
计算内部基准(12位AD) Vref = Nref * Vcc / 4096.

测量某个电压, 读ADC值Nx, 则电压 Ux = Vref * Nx / Nref. 一般Vref = 1190mV.

下载时, 选择时钟 22.1184MHz (可以在配置文件"config.h"中修改).

******************************************/

/*************	本地常量声明	**************/


/*************	本地变量声明	**************/
u16	Nref;


/*************	本地函数声明	**************/


/*************  外部函数和变量声明 *****************/


/******************* IO配置函数 *******************/
void	GPIO_config(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;		//结构定义
	//AD口设置为输入口
	GPIO_InitStructure.Pin  = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;		//指定要初始化的IO, GPIO_Pin_0 ~ GPIO_Pin_7
	GPIO_InitStructure.Mode = GPIO_HighZ;	//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P1,&GPIO_InitStructure);	//初始化

	GPIO_InitStructure.Pin  = GPIO_Pin_6 | GPIO_Pin_7;		//指定要初始化的IO, GPIO_Pin_0 ~ GPIO_Pin_7
	GPIO_InitStructure.Mode = GPIO_PullUp;	//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P4,&GPIO_InitStructure);	//初始化
}

/******************* AD配置函数 *******************/
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

/***************  串口初始化函数 *****************/
void	UART_config(void)
{
	COMx_InitDefine		COMx_InitStructure;					//结构定义

	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;		//模式,   UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
//	COMx_InitStructure.UART_BRT_Use   = BRT_Timer2;			//选择波特率发生器, BRT_Timer2 (注意: 串口2固定使用BRT_Timer2, 所以不用选择)
	COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率,     110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;				//接收允许,   ENABLE或DISABLE
	UART_Configuration(UART2, &COMx_InitStructure);		//初始化串口2 USART1,USART2,USART3,USART4
	NVIC_UART2_Init(ENABLE,Priority_1);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
	UART2_SW(UART2_SW_P46_P47);		//UART2_SW_P10_P11,UART2_SW_P46_P47
}

/**********************************************/
void main(void)
{
	u8	i;
	u16	j;

	EAXSFR();		/* 扩展寄存器访问使能 */
	GPIO_config();
	UART_config();
	ADC_config();
	EA = 1;

	PrintString2("STC8 AD to UART2 Test Programme!\r\n");	//UART2发送一个字符串

	while (1)
	{
		for(i=0; i<4; i++)
		{
			delay_ms(250);		//为了让发送的速度慢一点，延时250ms

			if(i <3)	//ADC0~ADC2
			{
				j = Get_ADCResult(i);	//参数0~14,查询方式做一次ADC, 返回值就是结果, == 4096 为错误
				TX2_write2buff('A');
				TX2_write2buff('D');
				TX2_write2buff(i+'0');
			}
			else		//内基准
			{
				j = Get_ADCResult(15);	//读内部基准电压, 读15通道
				Nref = j;
				TX2_write2buff('A');
				TX2_write2buff('D');
				TX2_write2buff('r');
				TX2_write2buff('e');
				TX2_write2buff('f');
			}
			TX2_write2buff('=');
			TX2_write2buff(j/1000 + '0');
			TX2_write2buff(j%1000/100 + '0');
			TX2_write2buff(j%100/10 + '0');
			TX2_write2buff(j%10 + '0');
			TX2_write2buff(' ');
			TX2_write2buff(' ');

			TX2_write2buff('V');
			TX2_write2buff('=');
			j = (u32)1190 * (u32)j / Nref;			//Ux = Vref * Nx / Nref. Vref = 1190mV
			TX2_write2buff(j/1000 + '0');
			TX2_write2buff('.');
			TX2_write2buff(j%1000/100 + '0');
			TX2_write2buff(j%100/10 + '0');
			TX2_write2buff(j%10 + '0');
			TX2_write2buff(' ');
			TX2_write2buff(' ');
		}
		PrintString2("\r\n");
		//=====================================================================
	}
}



