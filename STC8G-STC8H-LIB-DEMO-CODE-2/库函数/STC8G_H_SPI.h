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

#ifndef	__STC8G_H_SPI_H
#define	__STC8G_H_SPI_H

#include	"config.h"

//========================================================================
//                              寄存器位定义
//========================================================================

//sfr SPSTAT  = 0xCD;	//SPI状态寄存器
//   7       6      5   4   3   2   1   0    	Reset Value
//	SPIF	WCOL	-	-	-	-	-	-
#define	SPIF	0x80		/* SPI传输完成标志。写入1清0。*/
#define	WCOL	0x40		/* SPI写冲突标志。写入1清0。  */

//========================================================================
//                               SPI设置
//========================================================================

#define SPI_SSIG_Set(n)			SPCTL = (SPCTL & ~0x80) | (n << 7)		/* SS引脚功能控制 */
#define SPI_Start(n)			SPCTL = (SPCTL & ~0x40) | (n << 6)		/* SPI使能控制位 */
#define SPI_FirstBit_Set(n)		SPCTL = (SPCTL & ~0x20) | (n << 5)		/* 数据发送/接收顺序 MSB/LSB */
#define SPI_Mode_Set(n)			SPCTL = (SPCTL & ~0x10) | (n << 4)		/* SPI主从模式设置 */
#define SPI_CPOL_Set(n)			SPCTL = (SPCTL & ~0x08) | (n << 3)		/* SPI时钟极性控制 */
#define SPI_CPHA_Set(n)			SPCTL = (SPCTL & ~0x04) | (n << 2)		/* SPI时钟相位控制 */
#define SPI_Clock_Select(n)		SPCTL = (SPCTL & ~0x03) | (n)			/* SPI时钟频率选择 */

#define SPI_ClearFlag()         SPSTAT = (SPIF + WCOL)    /* 写 1 清除 SPIF和WCOL 标志 */

//========================================================================
//                              定义声明
//========================================================================

#define	SPI_BUF_LENTH	128
#define	SPI_BUF_type	xdata

sbit  SPI_SS    = P1^2;
sbit  SPI_MOSI  = P1^3;
sbit  SPI_MISO  = P1^4;
sbit  SPI_SCLK  = P1^5;

sbit  SPI_SS_2    = P2^2;
sbit  SPI_MOSI_2  = P2^3;
sbit  SPI_MISO_2  = P2^4;
sbit  SPI_SCLK_2  = P2^5;

sbit  SPI_SS_3    = P5^4;
sbit  SPI_MOSI_3  = P4^0;
sbit  SPI_MISO_3  = P4^1;
sbit  SPI_SCLK_3  = P4^3;

sbit  SPI_SS_4    = P3^5;
sbit  SPI_MOSI_4  = P3^4;
sbit  SPI_MISO_4  = P3^3;
sbit  SPI_SCLK_4  = P3^2;

#define	SPI_Mode_Master     1
#define	SPI_Mode_Slave      0
#define	SPI_CPOL_High       1
#define	SPI_CPOL_Low        0
#define	SPI_CPHA_1Edge      0
#define	SPI_CPHA_2Edge      1
#define	SPI_Speed_4         0
#define	SPI_Speed_8         1
#define	SPI_Speed_16        2
#define	SPI_Speed_2         3
#define	SPI_Speed_32        3
#define	SPI_MSB             0
#define	SPI_LSB             1

typedef struct
{
	u8	SPI_Enable;     //ENABLE,DISABLE
	u8	SPI_SSIG;       //ENABLE(ignore SS), DISABLE
	u8	SPI_FirstBit;   //SPI_MSB, SPI_LSB
	u8	SPI_Mode;       //SPI_Mode_Master, SPI_Mode_Slave
	u8	SPI_CPOL;       //SPI_CPOL_High,   SPI_CPOL_Low
	u8	SPI_CPHA;       //SPI_CPHA_1Edge,  SPI_CPHA_2Edge
	u8	SPI_Speed;      //SPI_Speed_4,SPI_Speed_16,SPI_Speed_64,SPI_Speed_2/SPI_Speed_32
} SPI_InitTypeDef;


extern	bit B_SPI_Busy; //发送忙标志
extern	u8 	SPI_RxCnt;
extern	u8 	SPI_RxTimerOut;
extern	u8 	SPI_BUF_type SPI_RxBuffer[SPI_BUF_LENTH];


void SPI_Init(SPI_InitTypeDef *SPIx);
void SPI_SetMode(u8 mode);
void SPI_WriteByte(u8 dat);
u8 SPI_ReadByte(void);

#endif

