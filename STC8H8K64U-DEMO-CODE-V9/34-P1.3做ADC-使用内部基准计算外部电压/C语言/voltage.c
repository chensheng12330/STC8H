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


/*************  本程序功能说明  **************

本例程基于STC8H8K64U为主控芯片的实验箱9进行编写测试，STC8G、STC8H系列芯片可通用参考.

读ADC测量外部电压，使用内部基准计算电压.

用STC的MCU的IO方式驱动8位数码管。

使用Timer0的16位自动重装来产生1ms节拍,程序运行于这个节拍下, 用户修改MCU主时钟频率时,自动定时于1ms.

右边4位数码管显示测量的电压值.

外部电压从板上测温电阻两端输入, 输入电压0~Vref, 不要超过Vref或低于0V. 

实际项目使用请串一个1K的电阻到ADC输入口, ADC输入口再并一个电容到地.

下载时, 选择时钟 24MHZ (用户可自行修改频率).

******************************************/

#include "stc8h.h"       //包含此头文件后，不需要再包含"reg51.h"头文件
#include "intrins.h"

#define     MAIN_Fosc       24000000L   //定义主时钟

typedef     unsigned char   u8;
typedef     unsigned int    u16;
typedef     unsigned long   u32;

/***********************************************************/

#define DIS_DOT     0x20
#define DIS_BLACK   0x10
#define DIS_        0x11

#define P1n_pure_input(bitn)        P1M1 |=  (bitn),    P1M0 &= ~(bitn)

/****************************** 用户定义宏 ***********************************/

    #define Cal_MODE    0   //每次测量只读1次ADC. 分辨率0.01V
//  #define Cal_MODE    1   //每次测量连续读16次ADC 再平均计算. 分辨率0.01V

#define Timer0_Reload   (65536UL -(MAIN_Fosc / 1000))       //Timer 0 中断频率, 1000次/秒
#define ADCTIM (*(unsigned char volatile xdata *)0xfea8)

/*****************************************************************************/


/*************  本地常量声明    **************/
u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

u8 code T_COM[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};      //位码


/*************  本地变量声明    **************/

u8  LED8[8];        //显示缓冲
u8  display_index;  //显示位索引
bit B_1ms;          //1ms标志

u16 msecond;
u16 Bandgap;    //
u16 Get_ADC12bitResult(u8 channel); //channel = 0~15

/**********************************************/
void main(void)
{
    u8  i;
    u16 j;

    P_SW2 |= 0x80;  //扩展寄存器(XFR)访问使能

    P0M1 = 0x30;   P0M0 = 0x30;   //设置P0.4、P0.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P1M1 = 0x38;   P1M0 = 0x30;   //设置P1.4、P1.5为漏极开路(实验箱加了上拉电阻到3.3V), 设置 P1.3 为 ADC 输入口
    P2M1 = 0x3c;   P2M0 = 0x3c;   //设置P2.2~P2.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P3M1 = 0x50;   P3M0 = 0x50;   //设置P3.4、P3.6为漏极开路(实验箱加了上拉电阻到3.3V)
    P4M1 = 0x3c;   P4M0 = 0x3c;   //设置P4.2~P4.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P5M1 = 0x0c;   P5M0 = 0x0c;   //设置P5.2、P5.3为漏极开路(实验箱加了上拉电阻到3.3V)
    P6M1 = 0xff;   P6M0 = 0xff;   //设置为漏极开路(实验箱加了上拉电阻到3.3V)
    P7M1 = 0x00;   P7M0 = 0x00;   //设置为准双向口
    
    display_index = 0;
    ADCTIM = 0x3f;		//设置 ADC 内部时序，ADC采样时间建议设最大值
    ADCCFG = 0x2f;		//设置 ADC 时钟为系统时钟/2/16
    ADC_CONTR = 0x80; //使能 ADC 模块

    AUXR = 0x80;    //Timer0 set as 1T, 16 bits timer auto-reload, 
    TH0 = (u8)(Timer0_Reload / 256);
    TL0 = (u8)(Timer0_Reload % 256);
    ET0 = 1;    //Timer0 interrupt enable
    TR0 = 1;    //Tiner0 run
    EA = 1;     //打开总中断
    
    for(i=0; i<8; i++)  LED8[i] = 0x10; //上电消隐

    while(1)
    {
        if(B_1ms)   //1ms到
        {
            B_1ms = 0;
            if(++msecond >= 300)    //300ms到
            {
                msecond = 0;

            #if (Cal_MODE == 0)
            //=================== 只读1次ADC, 12bit ADC. 分辨率0.01V ===============================
                Get_ADC12bitResult(15);  //先读一次并丢弃结果, 让内部的采样电容的电压等于输入值.
                Bandgap = Get_ADC12bitResult(15);    //读内部基准ADC, 读15通道
                Get_ADC12bitResult(3);  //先读一次并丢弃结果, 让内部的采样电容的电压等于输入值.
                j = Get_ADC12bitResult(3);  //读外部电压ADC
                j = (u16)((u32)j * 119 / Bandgap);  //计算外部电压, Bandgap为1.19V, 测电压分辨率0.01V
            #endif
            //==========================================================================

            //===== 连续读16次ADC 再平均计算. 分辨率0.01V =========
            #if (Cal_MODE == 1)
                Get_ADC12bitResult(15);  //先读一次并丢弃结果, 让内部的采样电容的电压等于输入值.
                for(j=0, i=0; i<16; i++)
                {
                    j += Get_ADC12bitResult(15); //读内部基准ADC, 读15通道
                }
                Bandgap = j >> 4;   //16次平均
                Get_ADC12bitResult(3);  //先读一次并丢弃结果, 让内部的采样电容的电压等于输入值.
                for(j=0, i=0; i<16; i++)
                {
                    j += Get_ADC12bitResult(3); //读外部电压ADC
                }
                j = j >> 4; //16次平均
                j = (u16)((u32)j * 119 / Bandgap);  //计算外部电压, Bandgap为1.19V, 测电压分辨率0.01V
            #endif
            //==========================================================================

                LED8[5] = j / 100 + DIS_DOT;    //显示外部电压值
                LED8[6] = (j % 100) / 10;
                LED8[7] = j % 10;
/*
                j = Bandgap;
                LED8[0] = j / 1000;     //显示Bandgap ADC值
                LED8[1] = (j % 1000) / 100;
                LED8[2] = (j % 100) / 10;
                LED8[3] = j % 10;
*/
            }
        }
    }
} 
/**********************************************/

//========================================================================
// 函数: u16 Get_ADC12bitResult(u8 channel)
// 描述: 查询法读一次ADC结果.
// 参数: channel: 选择要转换的ADC.
// 返回: 12位ADC结果.
// 版本: V1.0, 2012-10-22
//========================================================================
u16 Get_ADC12bitResult(u8 channel)  //channel = 0~15
{
    ADC_RES = 0;
    ADC_RESL = 0;

    ADC_CONTR = (ADC_CONTR & 0xF0) | 0x40 | channel;    //启动 AD 转换
    _nop_();
    _nop_();
    _nop_();
    _nop_();

    while((ADC_CONTR & 0x20) == 0)  ;   //wait for ADC finish
    ADC_CONTR &= ~0x20;     //清除ADC结束标志
    return  (((u16)ADC_RES << 8) | ADC_RESL);
}

/********************** 显示扫描函数 ************************/
void DisplayScan(void)
{   
    P7 = ~T_COM[7-display_index];
    P6 = ~t_display[LED8[display_index]];
    if(++display_index >= 8)    display_index = 0;  //8位结束回0
}

/********************** Timer0 1ms中断函数 ************************/
void timer0 (void) interrupt 1
{
    DisplayScan();  //1ms扫描显示一位
    B_1ms = 1;      //1ms标志
}

