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

使用STC8H系列MCU的PWM5_2(P1.7)播放8位WAVE语音.

语音参数: 8k采样 单声道, 码率64kb/s, 时长6.4秒.


                    3K      3K
      PWM5_2(P1.7)  ___     ___    OUTPUT
           --------|___|- -|___|- ------
                         |       |
                        ---     ---
                    103 ---     --- 103
                         |       |
                         |       |
                        ===     ===
                        GND     GND

******************************************/

/*************	本地常量声明	**************/


/*************	本地变量声明	**************/
u16		play_lenth;
u16		LastSample;
u16		PlayCnt;
u8	code *MusicPoint;
bit	B_Play;
u8	DAC_Cnt;



/*************	本地函数声明	**************/
void	LoadMusic(void);
void	PWM_config(void);
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

	PWM_config();	//初始化PWM
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


//========================================================================
// 函数: void	PWM_config(void)
// 描述: PWM配置函数。
// 参数: none.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-8-15
// 备注: 
//========================================================================
void PWM_config(void)
{
	P_SW2 |= 0x80;		//SFR enable   

	PWMB_PSCR = 0x0000;	// 预分频寄存器, 分频 Fck_cnt = Fck_psc/(PSCR[15:0}+1), 边沿对齐PWM频率 = SYSclk/((PSCR+1)*(AAR+1)), 中央对齐PWM频率 = SYSclk/((PSCR+1)*(AAR+1)*2).
	PWMB_DTR  = 00;		// 死区时间配置,   n=0~127: DTR= n T,   0x80 ~(0x80+n), n=0~63: DTR=(64+n)*2T,  
						// 对PWMB没有意义  0xc0 ~(0xc0+n), n=0~31: DTR=(32+n)*8T,   0xE0 ~(0xE0+n), n=0~31: DTR=(32+n)*16T,
	PWMB_ARR = 256;		// 自动重装载寄存器, 控制PWM周期

	PWMB_CCMR1 = 0x68;	// 通道模式配置, PWM模式1, 预装载允许
	PWMB_CCR1 = 128;	// 计数器比较值, 控制占空比
	PWMB_CCER1 = 0x01;	// 配置通道输出使能和极性 PWM6 PWM5

	PWMB_BKR = 0x80;	// 主输出使能 相当于总开关
	PWMB_IER = 0x00;	// 使能中断
	PWMB_CR1 = 0x81;	// 使能计数器, 允许自动重装载寄存器缓冲, 边沿对齐模式, 向上计数, bit7=1:写自动重装载寄存器缓冲(本周期不会被打扰), =0:直接写自动重装载寄存器
	

	PWMB_ENO  = 0x01;	//IO输出PWM允许, bit6: ENO8P,  bit4: ENO7P,  bit2: ENO6P,  bit0: ENO5P
	PWMB_PS   = (0<<6)+(0<<4)+(0<<2)+ 1;	//选择IO, 4项从高到低(从左到右)对应PWM8 PWM7 PWM6 PWM5

//  PWMA_PS    PWM8    PWM7    PWM6    PWM5
//    00       P2.3    P2.2    P2.1    P2.0
//    01       P3.4    P3.3    P5.4    P1.7
//    02       P0.3    P0.2    P0.1    P0.0
//    03       P7.7    P7.6    P7.5    P7.4

//	P_SW2 &= ~0x80;		//XDATA enable   
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
	if(B_Play)
	{
		PWMB_CCR1L = *MusicPoint;	//8位无符号wav数据送PWM占空比
		MusicPoint++;	//指向下一个数据
		if(++PlayCnt >= play_lenth)	B_Play = 0;
	}
}


