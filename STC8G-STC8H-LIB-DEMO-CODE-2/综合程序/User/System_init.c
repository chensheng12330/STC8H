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
#include	"STC8H_PWM.h"
#include	"STC8G_H_ADC.h"
#include	"STC8G_PCA.h"
#include	"STC8G_H_Exti.h"
#include	"STC8G_H_I2C.h"
#include	"STC8G_H_SPI.h"
#include	"STC8G_H_Timer.h"
#include	"STC8G_H_UART.h"
#include	"STC8G_H_Compare.h"
#include	"STC8G_PWM15bit.h"
#include	"STC8G_H_Delay.h"
#include	"STC8G_H_Switch.h"
#include	"STC8G_H_WDT.h"
#include	"STC8G_H_NVIC.h"
#include	"app.h"

//========================================================================
//                                IO口配置
//========================================================================
void	GPIO_config(void)
{
	P0_MODE_IO_PU(GPIO_Pin_All);		//P0 设置为准双向口
	P1_MODE_IN_HIZ(GPIO_Pin_LOW);		//P1.0~P1.3 设置为高阻输入
	P1_MODE_OUT_OD(GPIO_Pin_4 | GPIO_Pin_5);		//P1.4,P1.5 设置为漏极开路
	P2_MODE_IO_PU(GPIO_Pin_All);		//P2 设置为准双向口
	P3_MODE_IO_PU(GPIO_Pin_LOW);		//P3.0~P3.3 设置为准双向口
	P3_MODE_IO_PU(GPIO_Pin_HIGH);		//P3.4~P3.7 设置为准双向口
//	P3_MODE_IN_HIZ(GPIO_Pin_6 | GPIO_Pin_7);		//P3.6,P3.7 设置为高阻输入
	P4_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_6 | GPIO_Pin_7);	//P4.0,P4.6,P4.7 设置为准双向口
	P6_MODE_IO_PU(GPIO_Pin_All);		//P6 设置为准双向口
	P7_MODE_IO_PU(GPIO_Pin_All);		//P7 设置为准双向口
	
	P1_PULL_UP_ENABLE(GPIO_Pin_4 | GPIO_Pin_5);	//P1.4,P1.5 开启内部上拉
}

//========================================================================
//                               定时器配置
//========================================================================
void	Timer_config(void)
{
	TIM_InitTypeDef		TIM_InitStructure;                  //结构定义
	TIM_InitStructure.TIM_Mode      = TIM_16BitAutoReload;  //指定工作模式,   TIM_16BitAutoReload,TIM_16Bit,TIM_8BitAutoReload,TIM_16BitAutoReloadNoMask
	TIM_InitStructure.TIM_ClkSource = TIM_CLOCK_1T;         //指定时钟源,     TIM_CLOCK_1T,TIM_CLOCK_12T,TIM_CLOCK_Ext
	TIM_InitStructure.TIM_ClkOut    = DISABLE;              //是否输出高速脉冲, ENABLE或DISABLE
	TIM_InitStructure.TIM_Value     = 65536UL - (MAIN_Fosc / 1000UL);   //初值,
	TIM_InitStructure.TIM_Run       = ENABLE;               //是否初始化后启动定时器, ENABLE或DISABLE
	Timer_Inilize(Timer0,&TIM_InitStructure);               //初始化Timer0	  Timer0,Timer1,Timer2,Timer3,Timer4
	NVIC_Timer0_Init(ENABLE,Priority_0);    //中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

//	//定时器1做16位自动重装, 中断频率为20000HZ，中断函数从P6.6取反输出10KHZ方波信号.
//	TIM_InitStructure.TIM_Mode      = TIM_16BitAutoReload;	//指定工作模式,   TIM_16BitAutoReload,TIM_16Bit,TIM_8BitAutoReload,TIM_T1Stop
//	TIM_InitStructure.TIM_ClkSource = TIM_CLOCK_1T;		//指定时钟源, TIM_CLOCK_1T,TIM_CLOCK_12T,TIM_CLOCK_Ext
//	TIM_InitStructure.TIM_ClkOut    = DISABLE;				//是否输出高速脉冲, ENABLE或DISABLE
//	TIM_InitStructure.TIM_Value     = 65536UL - (MAIN_Fosc / 20000);			//初值,
//	TIM_InitStructure.TIM_Run       = ENABLE;				//是否初始化后启动定时器, ENABLE或DISABLE
//	Timer_Inilize(Timer1,&TIM_InitStructure);				//初始化Timer1	  Timer0,Timer1,Timer2,Timer3,Timer4
//	NVIC_Timer1_Init(ENABLE,Priority_0);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

//	//定时器2做16位自动重装, 中断频率为10000HZ，中断函数从P6.5取反输出5KHZ方波信号.
//	TIM_InitStructure.TIM_ClkSource = TIM_CLOCK_1T;		//指定时钟源,     TIM_CLOCK_1T,TIM_CLOCK_12T,TIM_CLOCK_Ext
//	TIM_InitStructure.TIM_ClkOut    = DISABLE;				//是否输出高速脉冲, ENABLE或DISABLE
//	TIM_InitStructure.TIM_Value     = 65536UL - (MAIN_Fosc / 10000);				//初值
//	TIM_InitStructure.TIM_PS        = 0;					//8位预分频器(n+1), 0~255, (注意:并非所有系列都有此寄存器,详情请查看数据手册)
//	TIM_InitStructure.TIM_Run       = ENABLE;				//是否初始化后启动定时器, ENABLE或DISABLE
//	Timer_Inilize(Timer2,&TIM_InitStructure);				//初始化Timer2	  Timer0,Timer1,Timer2,Timer3,Timer4
//	NVIC_Timer2_Init(ENABLE,NULL);		//中断使能, ENABLE/DISABLE; 无优先级

//	//定时器3做16位自动重装, 中断频率为100HZ，中断函数从P6.4取反输出50HZ方波信号.
//	TIM_InitStructure.TIM_ClkSource = TIM_CLOCK_12T;	//指定时钟源,     TIM_CLOCK_1T,TIM_CLOCK_12T,TIM_CLOCK_Ext
//	TIM_InitStructure.TIM_ClkOut    = ENABLE;				//是否输出高速脉冲, ENABLE或DISABLE
//	TIM_InitStructure.TIM_Value     = 65536UL - (MAIN_Fosc / (100*12));		//初值
//	TIM_InitStructure.TIM_PS        = 0;					//8位预分频器(n+1), 0~255, (注意:并非所有系列都有此寄存器,详情请查看数据手册)
//	TIM_InitStructure.TIM_Run       = ENABLE;				//是否初始化后启动定时器, ENABLE或DISABLE
//	Timer_Inilize(Timer3,&TIM_InitStructure);				//初始化Timer3	  Timer0,Timer1,Timer2,Timer3,Timer4
//	NVIC_Timer3_Init(ENABLE,NULL);		//中断使能, ENABLE/DISABLE; 无优先级

//	//定时器4做16位自动重装, 中断频率为50HZ，中断函数从P6.3取反输出25HZ方波信号.
//	TIM_InitStructure.TIM_ClkSource = TIM_CLOCK_12T;	//指定时钟源,     TIM_CLOCK_1T,TIM_CLOCK_12T,TIM_CLOCK_Ext
//	TIM_InitStructure.TIM_ClkOut    = ENABLE;				//是否输出高速脉冲, ENABLE或DISABLE
//	TIM_InitStructure.TIM_Value     = 65536UL - (MAIN_Fosc / (50*12));		//初值
//	TIM_InitStructure.TIM_PS        = 0;					//8位预分频器(n+1), 0~255, (注意:并非所有系列都有此寄存器,详情请查看数据手册)
//	TIM_InitStructure.TIM_Run       = ENABLE;				//是否初始化后启动定时器, ENABLE或DISABLE
//	Timer_Inilize(Timer4,&TIM_InitStructure);				//初始化Timer4	  Timer0,Timer1,Timer2,Timer3,Timer4
//	NVIC_Timer4_Init(ENABLE,NULL);		//中断使能, ENABLE/DISABLE; 无优先级
}

//========================================================================
//                              ADC初始化
//========================================================================
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

//========================================================================
//                              串口初始化
//========================================================================
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

//	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;	//模式,   UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
////	COMx_InitStructure.UART_BRT_Use   = BRT_Timer2;	//选择波特率发生器, BRT_Timer2 (注意: 串口2固定使用BRT_Timer2, 所以不用选择)
//	COMx_InitStructure.UART_BaudRate  = 115200ul;		//波特率,     110 ~ 115200
//	COMx_InitStructure.UART_RxEnable  = ENABLE;			//接收允许,   ENABLE或DISABLE
//	UART_Configuration(UART2, &COMx_InitStructure);		//初始化串口2 UART1,UART2,UART3,UART4
//	NVIC_UART2_Init(ENABLE,Priority_1);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
}

//========================================================================
//                               I2C初始化
//========================================================================
void	I2C_config(void)
{
	I2C_InitTypeDef		I2C_InitStructure;
	I2C_InitStructure.I2C_Mode      = I2C_Mode_Master;	//主从选择   I2C_Mode_Master, I2C_Mode_Slave
	I2C_InitStructure.I2C_Enable    = ENABLE;			//I2C功能使能,   ENABLE, DISABLE
	I2C_InitStructure.I2C_MS_WDTA   = DISABLE;			//主机使能自动发送,  ENABLE, DISABLE
	I2C_InitStructure.I2C_Speed     = 16;				//总线速度=Fosc/2/(Speed*2+4),      0~63
	I2C_Init(&I2C_InitStructure);
	NVIC_I2C_Init(I2C_Mode_Master,DISABLE,Priority_0);	//主从模式, I2C_Mode_Master, I2C_Mode_Slave; 中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
}

//========================================================================
//                               SPI初始化
//========================================================================
void	SPI_config(void)
{
	SPI_InitTypeDef		SPI_InitStructure;

	SPI_InitStructure.SPI_Enable    = ENABLE;				//SPI启动    ENABLE, DISABLE
	SPI_InitStructure.SPI_SSIG      = ENABLE;				//片选位     ENABLE, DISABLE
	SPI_InitStructure.SPI_FirstBit  = SPI_MSB;				//移位方向   SPI_MSB, SPI_LSB
	SPI_InitStructure.SPI_Mode      = SPI_Mode_Slave;		//主从选择   SPI_Mode_Master, SPI_Mode_Slave
	SPI_InitStructure.SPI_CPOL      = SPI_CPOL_Low;			//时钟相位   SPI_CPOL_High,   SPI_CPOL_Low
	SPI_InitStructure.SPI_CPHA      = SPI_CPHA_2Edge;		//数据边沿   SPI_CPHA_1Edge,  SPI_CPHA_2Edge
	SPI_InitStructure.SPI_Speed     = SPI_Speed_4;			//SPI速度    SPI_Speed_4, SPI_Speed_8, SPI_Speed_16, SPI_Speed_2/SPI_Speed_32
	SPI_Init(&SPI_InitStructure);
	NVIC_SPI_Init(ENABLE,Priority_3);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

//	SPI_InitStructure.SPI_Enable    = ENABLE;				//SPI启动    ENABLE, DISABLE
//	SPI_InitStructure.SPI_SSIG      = DISABLE;				//片选位     ENABLE, DISABLE
//	SPI_InitStructure.SPI_FirstBit  = SPI_MSB;				//移位方向   SPI_MSB, SPI_LSB
//	SPI_InitStructure.SPI_Mode      = SPI_Mode_Master;		//主从选择   SPI_Mode_Master, SPI_Mode_Slave
//	SPI_InitStructure.SPI_CPOL      = SPI_CPOL_Low;			//时钟相位   SPI_CPOL_High,   SPI_CPOL_Low
//	SPI_InitStructure.SPI_CPHA      = SPI_CPHA_2Edge;		//数据边沿   SPI_CPHA_1Edge,  SPI_CPHA_2Edge
//	SPI_InitStructure.SPI_Speed     = SPI_Speed_4;			//SPI速度    SPI_Speed_4, SPI_Speed_8, SPI_Speed_16, SPI_Speed_2/SPI_Speed_32
//	SPI_Init(&SPI_InitStructure);
//	NVIC_SPI_Init(ENABLE,Priority_3);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
}

//========================================================================
//                             比较器初始化
//========================================================================
void	CMP_config(void)
{
	CMP_InitDefine CMP_InitStructure;				//结构定义
	CMP_InitStructure.CMP_EN = ENABLE;				//允许比较器		ENABLE,DISABLE
	CMP_InitStructure.CMP_P_Select     = CMP_P_P37;	//比较器输入正极选择, CMP_P_P37,CMP_P_P50,CMP_P_P51,CMP_P_ADC.
	CMP_InitStructure.CMP_N_Select     = CMP_N_GAP;	//比较器输入负极选择, CMP_N_GAP: 选择内部BandGap经过OP后的电压做负输入, CMP_N_P36: 选择P3.6做负输入.
	CMP_InitStructure.CMP_InvCMPO      = DISABLE;	//比较器输出取反, 	ENABLE,DISABLE
	CMP_InitStructure.CMP_100nsFilter  = ENABLE;	//内部0.1us滤波,  	ENABLE,DISABLE
	CMP_InitStructure.CMP_Outpt_En     = ENABLE;	//允许比较结果输出,ENABLE,DISABLE
	CMP_InitStructure.CMP_OutDelayDuty = 16;		//比较结果变化延时周期数, 0~63
	CMP_InitStructure.CMP_CHYS = CMP_CHYS_10mV;		//DC迟滞输入选择,  CMP_CHYS_0mV,CMP_CHYS_10mV,CMP_CHYS_20mV,CMP_CHYS_30mV
	CMP_Inilize(&CMP_InitStructure);				//初始化比较器
	NVIC_CMP_Init(RISING_EDGE|FALLING_EDGE,Priority_0);	//中断使能, RISING_EDGE/FALLING_EDGE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
}
//========================================================================
//                             外部中断初始化
//========================================================================
void	Exti_config(void)
{
	EXTI_InitTypeDef	Exti_InitStructure;				//结构定义

	Exti_InitStructure.EXTI_Mode      = EXT_MODE_Fall;  //中断模式,   EXT_MODE_RiseFall,EXT_MODE_Fall
	Ext_Inilize(EXT_INT0,&Exti_InitStructure);			//初始化
	NVIC_INT0_Init(ENABLE,Priority_0);	//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

	Exti_InitStructure.EXTI_Mode      = EXT_MODE_Fall;  //中断模式,   EXT_MODE_RiseFall,EXT_MODE_Fall
	Ext_Inilize(EXT_INT1,&Exti_InitStructure);			//初始化
	NVIC_INT1_Init(ENABLE,Priority_0);	//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

	NVIC_INT2_Init(ENABLE,NULL);		//中断使能, ENABLE/DISABLE; 无优先级
	NVIC_INT3_Init(ENABLE,NULL);		//中断使能, ENABLE/DISABLE; 无优先级
	NVIC_INT4_Init(ENABLE,NULL);		//中断使能, ENABLE/DISABLE; 无优先级
}

//========================================================================
//                             外部中断初始化
//========================================================================

//========================================================================
//                               功能脚切换
//========================================================================
void	Switch_config(void)
{
	UART1_SW(UART1_SW_P30_P31);		//UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17,UART1_SW_P43_P44
	UART2_SW(UART2_SW_P46_P47);		//UART2_SW_P10_P11,UART2_SW_P46_P47
//	UART3_SW(UART3_SW_P00_P01);		//UART3_SW_P00_P01,UART3_SW_P50_P51
//	UART4_SW(UART4_SW_P02_P03);		//UART4_SW_P02_P03,UART4_SW_P52_P53
//	I2C_SW(I2C_P14_P15);			//I2C_P14_P15,I2C_P24_P25,I2C_P33_P32
//	COMP_SW(CMP_OUT_P34);			//CMP_OUT_P34,CMP_OUT_P41
	SPI_SW(SPI_P22_P23_P24_P25);	//SPI_P12_P13_P14_P15,SPI_P22_P23_P24_P25,SPI_P54_P40_P41_P43,SPI_P35_P34_P33_P32

	PWM1_SW(PWM1_SW_P60_P61);		//PWM1_SW_P10_P11,PWM1_SW_P20_P21,PWM1_SW_P60_P61
	PWM2_SW(PWM2_SW_P62_P63);		//PWM2_SW_P12_P13,PWM2_SW_P22_P23,PWM2_SW_P62_P63
	PWM3_SW(PWM3_SW_P64_P65);		//PWM3_SW_P14_P15,PWM3_SW_P24_P25,PWM3_SW_P64_P65
	PWM4_SW(PWM4_SW_P66_P67);		//PWM4_SW_P16_P17,PWM4_SW_P26_P27,PWM4_SW_P66_P67,PWM4_SW_P34_P33

	PWM5_SW(PWM5_SW_P00);			//PWM5_SW_P20,PWM5_SW_P17,PWM5_SW_P00,PWM5_SW_P74
	PWM6_SW(PWM6_SW_P01);			//PWM6_SW_P21,PWM6_SW_P54,PWM6_SW_P01,PWM6_SW_P75
	PWM7_SW(PWM7_SW_P02);			//PWM7_SW_P22,PWM7_SW_P33,PWM7_SW_P02,PWM7_SW_P76
	PWM8_SW(PWM8_SW_P03);			//PWM8_SW_P23,PWM8_SW_P34,PWM8_SW_P03,PWM8_SW_P77

//	PCA_SW(PCA_P12_P11_P10_P37);	//PCA_P12_P11_P10_P37,PCA_P34_P35_P36_P37,PCA_P24_P25_P26_P27
}

//========================================================================
//                                系统初始化
//========================================================================
void	SYS_Init(void)
{
//	GPIO_config();
	Timer_config();
//	ADC_config();
//	UART_config();
//	Exti_config();
//	I2C_config();
//	SPI_config();
//	CMP_config();
	Switch_config();
	EA = 1;
	
	APP_config();
}

