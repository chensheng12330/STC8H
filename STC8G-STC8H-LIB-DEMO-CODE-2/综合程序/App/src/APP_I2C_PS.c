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

#include	"APP.h"
#include	"STC8G_H_GPIO.h"
#include	"STC8G_H_I2C.h"
#include	"STC8G_H_Soft_I2C.h"
#include	"STC8G_H_UART.h"
#include	"STC8G_H_NVIC.h"
#include	"STC8G_H_Switch.h"

/*************	功能说明	**************

本例程基于STC8H8K64U为主控芯片的开天斧核心板进行编写测试，STC8G、STC8H系列芯片可通用参考.

内部集成I2C串行总线控制器做从机模式，SCL->P2.5, SDA->P2.4;
IO口模拟I2C做主机模式，SCL->P0.0, SDA->P0.1;
通过外部飞线连接 P0.0->P2.5, P0.1->P2.4，实现I2C自发自收功能。

使用Timer0的16位自动重装来产生1ms节拍,程序运行于这个节拍下,用户修改MCU主时钟频率时,自动定时于1ms.
计数器每秒钟加1, 计数范围为0~199.

上电后主机每秒钟发送一次计数数据，串口打印发送内容；从机接收到数据后通过串口打印接收结果。

需要在"STC8G_H_UART.h"里设置： #define	PRINTF_SELECT  UART1

下载时, 选择时钟 24MHz (可以在配置文件"config.h"中修改).

******************************************/

//========================================================================
//                               本地常量声明	
//========================================================================


//========================================================================
//                               本地变量声明
//========================================================================

u8  temp[4];     //通用数组

//========================================================================
//                               本地函数声明
//========================================================================


//========================================================================
//                            外部函数和变量声明
//========================================================================


//========================================================================
// 函数: I2C_PS_init
// 描述: 用户初始化程序.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-25
//========================================================================
void I2C_PS_init(void)
{
	I2C_InitTypeDef		I2C_InitStructure;
	COMx_InitDefine		COMx_InitStructure;					//结构定义

	P0_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1);		//P0.0,P0.1 设置为准双向口
	P2_MODE_IO_PU(GPIO_Pin_4 | GPIO_Pin_5);		//P2.4,P2.5 设置为准双向口
	P3_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1);		//P3.0,P3.1 设置为准双向口
	I2C_SW(I2C_P24_P25);					//I2C_P14_P15,I2C_P24_P25,I2C_P33_P32
	UART1_SW(UART1_SW_P30_P31);		//UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17,UART1_SW_P43_P44

	I2C_InitStructure.I2C_Mode      = I2C_Mode_Slave;		//主从选择   I2C_Mode_Master, I2C_Mode_Slave
	I2C_InitStructure.I2C_Enable    = ENABLE;						//I2C功能使能,   ENABLE, DISABLE
	I2C_InitStructure.I2C_SL_MA     = ENABLE;						//使能从机地址比较功能,   ENABLE, DISABLE
	I2C_InitStructure.I2C_SL_ADR    = 0x2d;							//从机设备地址,  0~127  (0x2d<<1 = 0x5a)
	I2C_Init(&I2C_InitStructure);
	NVIC_I2C_Init(I2C_Mode_Slave,I2C_ESTAI|I2C_ERXI|I2C_ETXI|I2C_ESTOI,Priority_0);	//主从模式, I2C_Mode_Master, I2C_Mode_Slave; 中断使能, I2C_ESTAI/I2C_ERXI/I2C_ETXI/I2C_ESTOI/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;	//模式, UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;			//选择波特率发生器, BRT_Timer1, BRT_Timer2 (注意: 串口2固定使用BRT_Timer2)
	COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率, 一般 110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;				//接收允许,   ENABLE或DISABLE
	COMx_InitStructure.BaudRateDouble = DISABLE;			//波特率加倍, ENABLE或DISABLE
	UART_Configuration(UART1, &COMx_InitStructure);		//初始化串口1 UART1,UART2,UART3,UART4
	NVIC_UART1_Init(ENABLE,Priority_1);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

	DisplayFlag = 0;
	second = 0;
}

//========================================================================
// 函数: Sample_I2C_PS
// 描述: 用户应用程序.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-25
//========================================================================
void Sample_I2C_PS(void)
{
    DisplayScan();

    if(DisplayFlag)
    {
        DisplayFlag = 0;
        printf("I2C Read: %bd%bd%bd%bd\r\n",I2C_Buffer[0],I2C_Buffer[1],I2C_Buffer[2],I2C_Buffer[3]);
    }

    if(++msecond >= 1000)   //1秒到
    {
        msecond = 0;        //清1000ms计数
        second++;         //秒计数+1
        if(second >= 200)    second = 0;   //秒计数范围为0~199

        printf("SI2C Send: %04bu\r\n",second);
        temp[0] = second / 1000;
        temp[1] = (second % 1000) / 100;
        temp[2] = (second % 100) / 10;
        temp[3] = second % 10;

        SI2C_WriteNbyte(SLAW, 0, temp, 4);
    }
}
