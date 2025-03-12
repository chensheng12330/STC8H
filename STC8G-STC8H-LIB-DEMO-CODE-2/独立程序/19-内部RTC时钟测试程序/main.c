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
#include	"STC8H_RTC.h"
#include	"STC8G_H_GPIO.h"
#include	"STC8G_H_UART.h"
#include	"STC8G_H_NVIC.h"
#include	"STC8G_H_Switch.h"

/*************	功能说明	**************

读写芯片内部集成的RTC模块.

通过串口1（P3.0,P3.1）打印时间(年-月-日-时-分-秒).

通过 STC8G_H_UART.h 头文件里面的 PRINTF_SELECT 定义，设置使用 UART1 打印 printf 信息。

下载时, 选择时钟 24MHz (可以在配置文件"config.h"中修改).

******************************************/

/*************	本地常量声明	**************/


/*************	本地变量声明	**************/


/*************	本地函数声明	**************/


/*************  外部函数和变量声明 *****************/

extern bit B_1S;
extern bit B_Alarm;

/******************** IO口配置 ********************/
void	GPIO_config(void)
{
	P3_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1);	//P3.0,P3.1 设置为准双向口
}

/***************  串口初始化函数 *****************/
void	UART_config(void)
{
	COMx_InitDefine		COMx_InitStructure;					//结构定义

	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;		//模式,   UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer2;			//选择波特率发生器, BRT_Timer1,BRT_Timer2 (注意: 串口2固定使用BRT_Timer2)
	COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率,     110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;				//接收允许,   ENABLE或DISABLE
	UART_Configuration(UART1, &COMx_InitStructure);		//初始化串口2 USART1,USART2,USART3,USART4
	NVIC_UART1_Init(ENABLE,Priority_1);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

	UART1_SW(UART1_SW_P30_P31);		//UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17,UART1_SW_P43_P44
}

/****************  RTC初始化函数 *****************/
void	RTC_config(void)
{
	RTC_InitTypeDef		RTC_InitStructure;
	RTC_InitStructure.RTC_Clock  = RTC_X32KCR;	//RTC 时钟, RTC_IRC32KCR, RTC_X32KCR
	RTC_InitStructure.RTC_Enable = ENABLE;			//RTC功能使能,   ENABLE, DISABLE
	RTC_InitStructure.RTC_Year   = 23;					//RTC 年, 00~99, 对应2000~2099年
	RTC_InitStructure.RTC_Month  = 12;					//RTC 月, 01~12
	RTC_InitStructure.RTC_Day    = 31;					//RTC 日, 01~31
	RTC_InitStructure.RTC_Hour   = 23;					//RTC 时, 00~23
	RTC_InitStructure.RTC_Min    = 59;					//RTC 分, 00~59
	RTC_InitStructure.RTC_Sec    = 55;					//RTC 秒, 00~59
	RTC_InitStructure.RTC_Ssec   = 00;					//RTC 1/128秒, 00~127

	RTC_InitStructure.RTC_ALAHour= 00;					//RTC 闹钟时, 00~23
	RTC_InitStructure.RTC_ALAMin = 00;					//RTC 闹钟分, 00~59
	RTC_InitStructure.RTC_ALASec = 00;					//RTC 闹钟秒, 00~59
	RTC_InitStructure.RTC_ALASsec= 00;					//RTC 闹钟1/128秒, 00~127
	RTC_Inilize(&RTC_InitStructure);
	NVIC_RTC_Init(RTC_ALARM_INT|RTC_SEC_INT,Priority_0);		//中断使能, RTC_ALARM_INT/RTC_DAY_INT/RTC_HOUR_INT/RTC_MIN_INT/RTC_SEC_INT/RTC_SEC2_INT/RTC_SEC8_INT/RTC_SEC32_INT/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
}

/******************** main **************************/
void main(void)
{
	EAXSFR();		//扩展SFR(XFR)访问使能 

	GPIO_config();
	UART_config();
	RTC_config();
	EA = 1;

	while (1)
	{
		if(B_1S)
		{
			B_1S = 0;
			printf("Year=20%bd,Month=%bd,Day=%bd,Hour=%bd,Minute=%bd,Second=%bd\r\n",YEAR,MONTH,DAY,HOUR,MIN,SEC);
		}

		if(B_Alarm)
		{
			B_Alarm = 0;
			printf("RTC Alarm!\r\n");
		}
	}
}
