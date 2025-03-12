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

#include	"STC8G_H_I2C.h"

//========================================================================
//                               本地变量声明
//========================================================================

I2C_IsrTypeDef I2CIsr;
bit DisplayFlag;

//========================================================================
//                            外部函数和变量声明
//========================================================================


//========================================================================
// 函数: I2C_ISR_Handler
// 描述: I2C中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2020-09-23
//========================================================================
void I2C_ISR_Handler() interrupt I2C_VECTOR
{
	// TODO: 在此处添加用户代码
	if (I2CSLST & 0x40)
	{
		I2CSLST &= ~0x40;                       //处理START事件
	}
	else if (I2CSLST & 0x20)
	{
		I2CSLST &= ~0x20;                       //处理RECV事件，SLACKO设置为0
		if (I2CIsr.isda)
		{
			I2CIsr.isda = 0;                           //处理RECV事件（RECV DEVICE ADDR）
		}
		else if (I2CIsr.isma)
		{
			I2CIsr.isma = 0;                           //处理RECV事件（RECV MEMORY ADDR）
			I2CIsr.addr = I2CRXD;
			I2CTXD = I2C_Buffer[I2CIsr.addr];
		}
		else
		{
			I2C_Buffer[I2CIsr.addr++] = I2CRXD;            //处理RECV事件（RECV DATA）
		}
	}
	else if (I2CSLST & 0x10)
	{
		I2CSLST &= ~0x10;                       //处理SEND事件
		if (I2CSLST & 0x02)
		{
			I2CTXD = 0xff;
		}
		else
		{
			I2CTXD = I2C_Buffer[++I2CIsr.addr];
		}
	}
	else if (I2CSLST & 0x08)
	{
		I2CSLST &= ~0x08;                       //处理STOP事件
		I2CIsr.isda = 1;
		I2CIsr.isma = 1;
		DisplayFlag = 1;
	}
}

