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

#include "STC8G_H_Compare.h"

//========================================================================
// 函数: CMP_ISR_Handler
// 描述: 比较器中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2020-09-23
//========================================================================
void CMP_ISR_Handler (void) interrupt CMP_VECTOR
{
	CMPCR1 &= ~CMPIF;			//清除中断标志
	
	// TODO: 在此处添加用户代码
	P47 = CMPCR1 & 0x01;	//中断方式读取比较器比较结果
}
