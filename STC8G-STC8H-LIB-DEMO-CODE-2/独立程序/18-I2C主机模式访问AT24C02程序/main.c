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
#include	"STC8G_H_I2C.h"
#include	"STC8G_H_Timer.h"
#include	"STC8G_H_UART.h"
#include	"STC8G_H_NVIC.h"
#include	"STC8G_H_Switch.h"

/*************	功能说明	**************

本例程基于STC8H8K64U为主控芯片的实验箱8进行编写测试，STC8G、STC8H系列芯片可通用参考.

使用Timer0的16位自动重装来产生1ms节拍，程序运行于这个节拍下，用户修改MCU主时钟频率时,自动定时于1ms.

通过硬件I2C接口读取AT24C02前8个字节数据，并通过串口打印.

将读取的数据加1后写回AT24C02前8个字节.

重新读取AT24C02前8个字节数据，并通过串口打印.

MCU上电后2秒钟执行1次以上动作.

下载时, 选择时钟 24MHz (可以在配置文件"config.h"中修改).

******************************************/

/*************	本地常量声明	**************/


/*************	本地变量声明	**************/
bit B_1ms;       //1ms标志

u16 msecond;

/*************	本地函数声明	**************/


/*************  外部函数和变量声明 *****************/


/******************** IO口配置 ********************/
void	GPIO_config(void)
{
	P2_MODE_IO_PU(GPIO_Pin_4 | GPIO_Pin_5);		//P2.4,P2.5 设置为准双向口
	P3_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1);		//P3.0,P3.1 设置为准双向口
}

/************************ 定时器配置 ****************************/
void	Timer_config(void)
{
	TIM_InitTypeDef		TIM_InitStructure;						//结构定义
	TIM_InitStructure.TIM_Mode      = TIM_16BitAutoReload;  //指定工作模式,   TIM_16BitAutoReload,TIM_16Bit,TIM_8BitAutoReload,TIM_16BitAutoReloadNoMask
	TIM_InitStructure.TIM_ClkSource = TIM_CLOCK_1T;         //指定时钟源,     TIM_CLOCK_1T,TIM_CLOCK_12T,TIM_CLOCK_Ext
	TIM_InitStructure.TIM_ClkOut    = DISABLE;              //是否输出高速脉冲, ENABLE或DISABLE
	TIM_InitStructure.TIM_Value     = 65536UL - (MAIN_Fosc / 1000UL);   //初值,
	TIM_InitStructure.TIM_Run       = ENABLE;               //是否初始化后启动定时器, ENABLE或DISABLE
	Timer_Inilize(Timer0,&TIM_InitStructure);               //初始化Timer0	  Timer0,Timer1,Timer2,Timer3,Timer4
	NVIC_Timer0_Init(ENABLE,Priority_0);    //中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
}

/****************  I2C初始化函数 *****************/
void	I2C_config(void)
{
	I2C_InitTypeDef		I2C_InitStructure;

	I2C_InitStructure.I2C_Mode      = I2C_Mode_Master;	//主从选择   I2C_Mode_Master, I2C_Mode_Slave
	I2C_InitStructure.I2C_Enable    = ENABLE;			//I2C功能使能,   ENABLE, DISABLE
	I2C_InitStructure.I2C_MS_WDTA   = DISABLE;			//主机使能自动发送,  ENABLE, DISABLE
	I2C_InitStructure.I2C_Speed     = 16;				//总线速度=Fosc/2/(Speed*2+4),      0~63
	I2C_Init(&I2C_InitStructure);
	NVIC_I2C_Init(I2C_Mode_Master,DISABLE,Priority_0);	//主从模式, I2C_Mode_Master, I2C_Mode_Slave; 中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

	I2C_SW(I2C_P24_P25);					//I2C_P14_P15,I2C_P24_P25,I2C_P33_P32
}

/****************  UART初始化函数 *****************/
void	UART_config(void)
{
	COMx_InitDefine		COMx_InitStructure;				//结构定义

	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;	//模式, UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;		//选择波特率发生器, BRT_Timer1, BRT_Timer2 (注意: 串口2固定使用BRT_Timer2)
	COMx_InitStructure.UART_BaudRate  = 115200ul;		//波特率, 一般 110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;			//接收允许,   ENABLE或DISABLE
	COMx_InitStructure.BaudRateDouble = DISABLE;		//波特率加倍, ENABLE或DISABLE
	UART_Configuration(UART1, &COMx_InitStructure);		//初始化串口1 UART1,UART2,UART3,UART4
	NVIC_UART1_Init(ENABLE,Priority_1);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

	UART1_SW(UART1_SW_P30_P31);		//UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17,UART1_SW_P43_P44
}

/******************** task A **************************/
void main(void)
{
    u8 i;

	EAXSFR();		/* 扩展寄存器访问使能 */
	GPIO_config();
	Timer_config();
	I2C_config();
    UART_config();
	EA = 1;
    
	while (1)
	{
		if(B_1ms)
		{
			B_1ms = 0;
            
            if(++msecond >= 2000)   //2秒到
            {
                msecond = 0;        //清计数

                I2C_ReadNbyte(DEV_ADDR, 0, I2C_Buffer, 8);
                printf("Read=");   //读取EEPROM原先的内容
                for(i=0;i<8;i++)
                {
                    printf("0x%02bx ",I2C_Buffer[i]);
                    if(I2C_Buffer[i] == 0xff) I2C_Buffer[i] = i;  //如果内容为空，则写入初始化数据
                    else I2C_Buffer[i]++;    //如果内容非空，在原先基础上加1
                }
                printf("\r\n");

                I2C_WriteNbyte(DEV_ADDR, 0, I2C_Buffer, 8);
            }
		}
	}
}
