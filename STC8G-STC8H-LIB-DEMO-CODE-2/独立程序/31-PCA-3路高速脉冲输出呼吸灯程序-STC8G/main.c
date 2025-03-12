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
#include	"STC8G_PCA.h"
#include	"STC8G_H_GPIO.h"
#include	"STC8G_H_Delay.h"
#include	"STC8G_H_Switch.h"

/*************   功能说明   ***************

本例程基于STC8G1K08-20PIN进行编写测试，STC8G系列芯片可通用参考.

输出3路变化的PWM信号, 类似"呼吸灯"的驱动.

PCA0, PCA1, PCA2 为16位PWM.

下载时, 选择时钟 24MHz (用户可在"config.h"修改频率).

******************************************/

/*************	本地常量声明	**************/

#define		PWM_DUTY		6000		//定义PWM的周期，数值为PCA所选择的时钟脉冲个数。
#define		PWM_HIGH_MIN	80			//限制PWM输出的最小占空比。
#define		PWM_HIGH_MAX	(PWM_DUTY - PWM_HIGH_MIN)	//限制PWM输出的最大占空比。

/*************	本地变量声明	**************/

u16	pwm0,pwm1,pwm2;
bit	B_PWM0_Dir,B_PWM1_Dir,B_PWM2_Dir;	//方向, 0为+, 1为-.

u16	PWM0_low;	//PWM输出低电平的PCA时钟脉冲个数
u16	PWM1_low;	//PWM输出低电平的PCA时钟脉冲个数
u16	PWM2_low;	//PWM输出低电平的PCA时钟脉冲个数

/*************	本地函数声明	**************/

void PWMn_Update(u8 PCA_id, u16 high);

/*************  外部函数和变量声明 *****************/


/******************** IO口配置 ********************/
void	GPIO_config(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;				//结构定义

	GPIO_InitStructure.Pin  = GPIO_Pin_All;			//指定要初始化的IO, GPIO_Pin_0 ~ GPIO_Pin_7, 或操作
	GPIO_InitStructure.Mode = GPIO_PullUp;			//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P1,&GPIO_InitStructure);	//初始化

	GPIO_InitStructure.Pin  = GPIO_Pin_7;				//指定要初始化的IO, GPIO_Pin_0 ~ GPIO_Pin_7, 或操作
	GPIO_InitStructure.Mode = GPIO_PullUp;			//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P3,&GPIO_InitStructure);	//初始化
}

/******************** PCA配置 ********************/
void	PCA_config(void)
{
	PCA_InitTypeDef		PCA_InitStructure;

	PCA_InitStructure.PCA_Clock    = PCA_Clock_1T;		//PCA_Clock_1T, PCA_Clock_2T, PCA_Clock_4T, PCA_Clock_6T, PCA_Clock_8T, PCA_Clock_12T, PCA_Clock_Timer0_OF, PCA_Clock_ECI
	PCA_InitStructure.PCA_RUN      = DISABLE;			//ENABLE, DISABLE
	PCA_Init(PCA_Counter,&PCA_InitStructure);			//设置公用Counter

	PCA_InitStructure.PCA_PWM_Wide = 0;					//PCA_PWM_8bit, PCA_PWM_7bit, PCA_PWM_6bit, PCA_PWM_10bit
	PCA_InitStructure.PCA_Value    = 65535;				//对于软件定时, 为匹配比较值
	PCA_Init(PCA0,&PCA_InitStructure);

	PCA_InitStructure.PCA_PWM_Wide = 0;					//PCA_PWM_8bit, PCA_PWM_7bit, PCA_PWM_6bit, PCA_PWM_10bit
	PCA_InitStructure.PCA_Value    = 65535;				//对于软件定时, 为匹配比较值
	PCA_Init(PCA1,&PCA_InitStructure);

	PCA_InitStructure.PCA_PWM_Wide = 0;					//PCA_PWM_8bit, PCA_PWM_7bit, PCA_PWM_6bit, PCA_PWM_10bit
	PCA_InitStructure.PCA_Value    = 65535;				//对于软件定时, 为匹配比较值
	PCA_Init(PCA2,&PCA_InitStructure);

	NVIC_PCA_Init(PCA_Counter,PCA_Mode_HighPulseOutput,Priority_0);
	NVIC_PCA_Init(PCA0,PCA_Mode_HighPulseOutput,Priority_0);
	NVIC_PCA_Init(PCA1,PCA_Mode_HighPulseOutput,Priority_0);
	NVIC_PCA_Init(PCA2,PCA_Mode_HighPulseOutput,Priority_0);

	PCA_SW(PCA_P12_P11_P10_P37);	//PCA_P12_P11_P10_P37,PCA_P34_P35_P36_P37,PCA_P24_P25_P26_P27
	CR = 1;							//启动PCA
}


/******************** task A **************************/
void main(void)
{
	EAXSFR();		/* 扩展寄存器访问使能 */
    GPIO_config();
    PCA_config();
    pwm0 = (PWM_DUTY / 4 * 1);	//给PWM一个初值
    pwm1 = (PWM_DUTY / 4 * 2);
    pwm2 = (PWM_DUTY / 4 * 3);
    B_PWM0_Dir = 0;
    B_PWM1_Dir = 0;
    B_PWM2_Dir = 0;

    PWMn_Update(PCA0,pwm0);
    PWMn_Update(PCA1,pwm1);
    PWMn_Update(PCA2,pwm2);
    EA = 1;

    while (1)
    {
        delay_ms(1);

        if(B_PWM0_Dir)
        {
            if(--pwm0 <= PWM_HIGH_MIN)	B_PWM0_Dir = 0;
        }
        else if(++pwm0 >= PWM_HIGH_MAX)	B_PWM0_Dir = 1;
        PWMn_Update(PCA0,pwm0);

        if(B_PWM1_Dir)
        {
            if(--pwm1 <= PWM_HIGH_MIN)		B_PWM1_Dir = 0;
        }
        else if(++pwm1 >= PWM_HIGH_MAX)	B_PWM1_Dir = 1;
        PWMn_Update(PCA1,pwm1);

        if(B_PWM2_Dir)
        {
            if(--pwm2 <= PWM_HIGH_MIN)		B_PWM2_Dir = 0;
        }
        else if(++pwm2 >= PWM_HIGH_MAX)	B_PWM2_Dir = 1;
        PWMn_Update(PCA2,pwm2);
    }
}

void PWMn_Update(u8 PCA_id, u16 high)
{
	if(PCA_id == PCA0)
	{
		if(high > PWM_HIGH_MAX)	high = PWM_HIGH_MAX;	//如果写入大于最大占空比数据，强制为最大占空比。
		if(high < PWM_HIGH_MIN)	high = PWM_HIGH_MIN;	//如果写入小于最小占空比数据，强制为最小占空比。
		CR = 0;							//停止PCA。
		PCA_Timer0 = high;				//数据在正确范围，则装入占空比寄存器。
		PWM0_low = PWM_DUTY - high;	//计算并保存PWM输出低电平的PCA时钟脉冲个数。
		CR = 1;							//启动PCA。
	}
	else if(PCA_id == PCA1)
	{
		if(high > PWM_HIGH_MAX)	high = PWM_HIGH_MAX;	//如果写入大于最大占空比数据，强制为最大占空比。
		if(high < PWM_HIGH_MIN)	high = PWM_HIGH_MIN;	//如果写入小于最小占空比数据，强制为最小占空比。
		CR = 0;							//停止PCA。
		PCA_Timer1 = high;				//数据在正确范围，则装入占空比寄存器。
		PWM1_low = PWM_DUTY - high;	//计算并保存PWM输出低电平的PCA时钟脉冲个数。
		CR = 1;							//启动PCA。
	}
	else if(PCA_id == PCA2)
	{
		if(high > PWM_HIGH_MAX)		high = PWM_HIGH_MAX;	//如果写入大于最大占空比数据，强制为最大占空比。
		if(high < PWM_HIGH_MIN)		high = PWM_HIGH_MIN;	//如果写入小于最小占空比数据，强制为最小占空比。
		CR = 0;							//停止PCA。
		PCA_Timer2 = high;				//数据在正确范围，则装入占空比寄存器。
		PWM2_low = PWM_DUTY - high;	//计算并保存PWM输出低电平的PCA时钟脉冲个数。
		CR = 1;							//启动PCA。
	}
}

//========================================================================
// 函数: void PCA_Handler (void) interrupt PCA_VECTOR
// 描述: PCA中断处理程序.
// 参数: None
// 返回: none.
// 版本: V1.0, 2012-11-22
//========================================================================
void PCA_Handler (void) interrupt PCA_VECTOR
{
	if(CCF0)		//PCA模块0中断
	{
		CCF0 = 0;		//清PCA模块0中断标志
		if(P11)	CCAP0_tmp += PCA_Timer0;	//输出为高电平，则给影射寄存器装载高电平时间长度
		else	CCAP0_tmp += PWM0_low;	//输出为低电平，则给影射寄存器装载低电平时间长度
		CCAP0L = (u8)CCAP0_tmp;			//将影射寄存器写入捕获寄存器，先写CCAP0L
		CCAP0H = (u8)(CCAP0_tmp >> 8);	//后写CCAP0H
	}

	if(CCF1)	//PCA模块1中断
	{
		CCF1 = 0;		//清PCA模块1中断标志
		if(P10)	CCAP1_tmp += PCA_Timer1;	//输出为高电平，则给影射寄存器装载高电平时间长度
		else	CCAP1_tmp += PWM1_low;	//输出为低电平，则给影射寄存器装载低电平时间长度
		CCAP1L = (u8)CCAP1_tmp;			//将影射寄存器写入捕获寄存器，先写CCAP0L
		CCAP1H = (u8)(CCAP1_tmp >> 8);	//后写CCAP0H
	}

	if(CCF2)	//PCA模块2中断
	{
		CCF2 = 0;		//清PCA模块1中断标志
		if(P37)	CCAP2_tmp += PCA_Timer2;	//输出为高电平，则给影射寄存器装载高电平时间长度
		else	CCAP2_tmp += PWM2_low;	//输出为低电平，则给影射寄存器装载低电平时间长度
		CCAP2L = (u8)CCAP2_tmp;			//将影射寄存器写入捕获寄存器，先写CCAP0L
		CCAP2H = (u8)(CCAP2_tmp >> 8);	//后写CCAP0H
	}

	if(CF)	//PCA溢出中断
	{
		CF = 0;			//清PCA溢出中断标志
	}

}
