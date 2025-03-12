/*---------------------------------------------------------------------*/
/* --- STC MCU Limited ------------------------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.STCAI.com ---------------------------------------------*/
/* --- Web: www.STCMCUDATA.com  ---------------------------------------*/
/* --- BBS: www.STCAIMCU.com  -----------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* 如果要在程序中使用此代码,请在程序中注明使用了STC的资料及程序        */
/*---------------------------------------------------------------------*/

#define 	MAIN_Fosc		24000000L	//定义主时钟
#include	"STC8Hxxx.h"

#include	"music-8bit-8K-wave.h"


/*************	功能说明	**************

使用STC8H系列MCU外接立体声DAC PT8211(TM8211) 播放8位WAVE语音.

语音参数: 8k采样 单声道, 码率64kb/s, 时长6.4秒.

******************************************/

/*************	本地常量声明	**************/


/*************	本地变量声明	**************/
sbit	P_TM8211_BCK = P0^2;	//PIN1--BCK		8--RCH		4--GND
sbit	P_TM8211_WS  = P0^1;	//PIN2--WS		7--NC		5--VDD
sbit	P_TM8211_DIN = P0^0;	//PIN3--DIN		6--LCH

u16		play_lenth;
u16		LastSample;
u16		PlayCnt;
u8	code *MusicPoint;
bit	B_Play;
u8	DAC_Cnt;



/*************	本地函数声明	**************/
void	LoadMusic(void);
u8		Timer0_Config(u16 reload);	//reload是重装值, 返回0正确, 返回1装载值过大错误.



/*************  外部函数和变量声明 *****************/




/******************** 主函数**************************/
void main(void)
{
	P0n_standard(0xff);	//设置为准双向口
	P1n_standard(0xff);	//设置为准双向口
	P2n_standard(0xff);	//设置为准双向口
	P3n_standard(0xff);	//设置为准双向口
	P4n_standard(0xff);	//设置为准双向口
	P5n_standard(0xff);	//设置为准双向口

	Timer0_Config(MAIN_Fosc / 8000);	//t=0: reload值是主时钟周期数,  (中断频率, 8000次/秒)

	EA = 1;			//允许全局中断
	
	
	while (1)
	{
		if(!B_Play)	LoadMusic();
		NOP(10);
	}
}


//========================================================================
// 函数: void	LoadWave(void)
// 描述: 装载一段wav
// 参数: none.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-8-15
// 备注: 
//========================================================================
void	LoadMusic(void)
{
	MusicPoint = Music;

	play_lenth = ((u16)MusicPoint[41] << 8)	+ MusicPoint[40];
	MusicPoint += 44;

	PlayCnt = 0;
	B_Play = 1;
}


void TM8211_Wbyte(u8 dat)
{
	ACC = dat;

	P_TM8211_DIN = ACC7;	NOP(1);	P_TM8211_BCK = 1;	NOP(1);	P_TM8211_BCK = 0;	//6*8+7=55T
	P_TM8211_DIN = ACC6;	NOP(1);	P_TM8211_BCK = 1;	NOP(1);	P_TM8211_BCK = 0;	
	P_TM8211_DIN = ACC5;	NOP(1);	P_TM8211_BCK = 1;	NOP(1);	P_TM8211_BCK = 0;	
	P_TM8211_DIN = ACC4;	NOP(1);	P_TM8211_BCK = 1;	NOP(1);	P_TM8211_BCK = 0;	
	P_TM8211_DIN = ACC3;	NOP(1);	P_TM8211_BCK = 1;	NOP(1);	P_TM8211_BCK = 0;	
	P_TM8211_DIN = ACC2;	NOP(1);	P_TM8211_BCK = 1;	NOP(1);	P_TM8211_BCK = 0;	
	P_TM8211_DIN = ACC1;	NOP(1);	P_TM8211_BCK = 1;	NOP(1);	P_TM8211_BCK = 0;	
	P_TM8211_DIN = ACC0;	NOP(1);	P_TM8211_BCK = 1;	NOP(1);	P_TM8211_BCK = 0;	
}


//========================================================================
// 函数:u8	Timer0_Config(u8 t, u32 reload)
// 描述: timer0初始化函数.
// 参数: reload: 重装值.
// 返回: 0: 初始化正确, 1: 重装值过大, 初始化错误.
// 版本: V1.0, 2018-12-20
//========================================================================
u8	Timer0_Config(u16 reload)	//reload是重装值
{
	TR0 = 0;	//停止计数

	if(reload >= (65536UL * 12))	return 1;	//值过大, 返回错误
	if(reload < 65536UL)	AUXR |= 0x80;		//1T mode
	else
	{
		AUXR &= ~0x80;	//12T mode
		reload = reload / 12;
	}
	reload = 65536UL - reload;
	TH0 = (u8)(reload >> 8);
	TL0 = (u8)(reload);

	ET0 = 1;	//允许中断
//	PT0 = 1;	//高优先级中断
	TMOD = (TMOD & ~0x03) | 0;	//工作模式, 0: 16位自动重装, 1: 16位定时/计数, 2: 8位自动重装, 3: 16位自动重装, 不可屏蔽中断
//	TMOD |=  0x04;	//对外计数或分频
//	INT_CLKO |=  0x01;	//输出时钟
	TR0 = 1;	//开始运行
	return 0;
}

//========================================================================
// 函数: void timer0_int (void) interrupt TIMER0_VECTOR
// 描述:  timer0中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2018-12-20
//========================================================================
void timer0_ISR (void) interrupt TIMER0_VECTOR
{
	u16	j;
	
	if(B_Play)
	{
		j = ((u16)*MusicPoint << 8) + 32768;	//8位无符号wav数据转16位有符号整数
		TM8211_Wbyte((u8)(j >> 8));	//输出DAC
		TM8211_Wbyte((u8)j);
		P_TM8211_WS = 0;
		NOP(2);					//另一个声道, 如果没有数据, 则两个声道输出相同(单声道)
		P_TM8211_WS = 1;

		MusicPoint++;	//指向下一个数据
		if(++PlayCnt >= play_lenth)	B_Play = 0;
	}
}


