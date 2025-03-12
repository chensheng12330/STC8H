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

#include	"STC8G_H_Timer.h"
#include	"STC8H_PWM.h"

/*************	本地变量声明	**************/
bit PWM5_Flag;
bit PWM6_Flag;
bit PWM7_Flag;
bit PWM8_Flag;

/*************  外部函数和变量声明 *****************/
extern PWMx_Duty PWMB_Duty;

//========================================================================
// 函数: Timer0_ISR_Handler
// 描述: Timer0中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2020-09-23
//========================================================================
void Timer0_ISR_Handler (void) interrupt TMR0_VECTOR		//进中断时已经清除标志
{
	// TODO: 在此处添加用户代码
	if(!PWM5_Flag)
	{
		PWMB_Duty.PWM5_Duty++;
		if(PWMB_Duty.PWM5_Duty >= 2047) PWM5_Flag = 1;
	}
	else
	{
		PWMB_Duty.PWM5_Duty--;
		if(PWMB_Duty.PWM5_Duty <= 0) PWM5_Flag = 0;
	}

	if(!PWM6_Flag)
	{
		PWMB_Duty.PWM6_Duty++;
		if(PWMB_Duty.PWM6_Duty >= 2047) PWM6_Flag = 1;
	}
	else
	{
		PWMB_Duty.PWM6_Duty--;
		if(PWMB_Duty.PWM6_Duty <= 0) PWM6_Flag = 0;
	}

	if(!PWM7_Flag)
	{
		PWMB_Duty.PWM7_Duty++;
		if(PWMB_Duty.PWM7_Duty >= 2047) PWM7_Flag = 1;
	}
	else
	{
		PWMB_Duty.PWM7_Duty--;
		if(PWMB_Duty.PWM7_Duty <= 0) PWM7_Flag = 0;
	}

	if(!PWM8_Flag)
	{
		PWMB_Duty.PWM8_Duty++;
		if(PWMB_Duty.PWM8_Duty >= 2047) PWM8_Flag = 1;
	}
	else
	{
		PWMB_Duty.PWM8_Duty--;
		if(PWMB_Duty.PWM8_Duty <= 0) PWM8_Flag = 0;
	}
	
	UpdatePwm(PWMB, &PWMB_Duty);
}

//========================================================================
// 函数: Timer1_ISR_Handler
// 描述: Timer1中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2020-09-23
//========================================================================
void Timer1_ISR_Handler (void) interrupt TMR1_VECTOR		//进中断时已经清除标志
{
	// TODO: 在此处添加用户代码
	P66 = ~P66;
}

//========================================================================
// 函数: Timer2_ISR_Handler
// 描述: Timer2中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2020-09-23
//========================================================================
void Timer2_ISR_Handler (void) interrupt TMR2_VECTOR		//进中断时已经清除标志
{
	// TODO: 在此处添加用户代码
	P65 = ~P65;
}

//========================================================================
// 函数: Timer3_ISR_Handler
// 描述: Timer3中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2020-09-23
//========================================================================
void Timer3_ISR_Handler (void) interrupt TMR3_VECTOR		//进中断时已经清除标志
{
	// TODO: 在此处添加用户代码
	P64 = ~P64;
}

//========================================================================
// 函数: Timer4_ISR_Handler
// 描述: Timer4中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2020-09-23
//========================================================================
void Timer4_ISR_Handler (void) interrupt TMR4_VECTOR		//进中断时已经清除标志
{
	// TODO: 在此处添加用户代码
	P63 = ~P63;
}
