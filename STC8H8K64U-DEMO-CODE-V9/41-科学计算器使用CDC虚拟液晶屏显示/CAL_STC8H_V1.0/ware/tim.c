#include "tim.h"

u32 Sys_Time_10ms = 0;
//========================================================================
// 函数: void Timer0_init(void)
// 描述: timer0初始化函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2015-1-12
//========================================================================
void Timer0_init(void)
{
        TR0 = 0;    //停止计数

    #if (Timer0_Reload < 64)    // 如果用户设置值不合适， 则不启动定时器
        #error "Timer0设置的中断过快!"

    #elif ((Timer0_Reload/12) < 65536UL)    // 如果用户设置值不合适， 则不启动定时器
        ET0 = 1;    //允许中断
    //  PT0 = 1;    //高优先级中断
        TMOD &= ~0x03;
        TMOD |= 0;  //工作模式, 0: 16位自动重装, 1: 16位定时/计数, 2: 8位自动重装, 3: 16位自动重装, 不可屏蔽中断
    //  TMOD |=  0x04;  //对外计数或分频
        TMOD &= ~0x04;  //定时
    //  INTCLKO |=  0x01;  //输出时钟
        INTCLKO &= ~0x01;  //不输出时钟

        #if (Timer0_Reload < 65536UL)
            AUXR |=  0x80;  //1T mode
            TH0 = (u8)((65536UL - Timer0_Reload) / 256);
            TL0 = (u8)((65536UL - Timer0_Reload) % 256);
        #else
            AUXR &= ~0x80;  //12T mode
            TH0 = (u8)((65536UL - Timer0_Reload/12) / 256);
            TL0 = (u8)((65536UL - Timer0_Reload/12) % 256);
        #endif

        TR0 = 1;    //开始运行

    #else
        #error "Timer0设置的中断过慢!"
    #endif
}


//========================================================================
// 函数: void timer0_int (void) interrupt TIMER0_VECTOR
// 描述:  timer0中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2015-1-12
//========================================================================
void timer0_int (void) interrupt 1
{
    static u8 xdata count = 0;
    TR0=0;
    Sys_Time_10ms++;
	TR0=1;
    if( count++ >= 100 )
    {
        count= 0;
    }
}
