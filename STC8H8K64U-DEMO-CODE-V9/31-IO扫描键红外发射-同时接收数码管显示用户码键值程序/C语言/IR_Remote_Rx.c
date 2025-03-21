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
/* 如果要在程序中使用此代码,请在程序中注明使用了STC的资料及程序            */
/*---------------------------------------------------------------------*/


/*************  本程序功能说明  **************

本例程基于STC8H8K64U为主控芯片的实验箱9进行编写测试，STC8H系列芯片可通用参考.

红外收发程序。适用于市场上用量最大的NEC编码。

应用层查询 B_IR_Press标志为,则已接收到一个键码放在IR_code中, 处理完键码后， 用户程序清除B_IR_Press标志.

数码管左起4位显示用户码, 最右边两位显示数据, 均为十六进制.

用户可以在宏定义中指定用户码.

用户底层程序按固定的时间间隔(60~125us)调用 "IR_RX_NEC()"函数.

按下IO行列键（不支持ADC键盘），显示发送、接收到的键值。

下载时, 选择时钟 24MHz (用户可自行修改频率).

******************************************/

#include "stc8h.h"       //包含此头文件后，不需要再包含"reg51.h"头文件
#include "intrins.h"

#define     MAIN_Fosc       24000000UL   //定义主时钟

typedef     unsigned char   u8;
typedef     unsigned int    u16;
typedef     unsigned long   u32;

/****************************** 用户定义宏 ***********************************/

#define SysTick 14225   // 次/秒, 系统滴答频率, 在4000~16000之间
#define Timer0_Reload   (65536UL - ((MAIN_Fosc + SysTick/2) / SysTick))     //Timer 0 中断频率
#define User_code   0xFF00      //定义红外用户码

/***********************************************************/

#define DIS_DOT     0x20
#define DIS_BLACK   0x10
#define DIS_        0x11

/*****************************************************************************/

/*************  本地常量声明    **************/
u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

u8 code T_COM[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};      //位码

/*************  IO口定义    **************/

sbit P_IR_TX = P2^7;    //定义红外发送脚
sbit P_IR_RX = P3^5;    //定义红外接收输入IO口

/*************  本地变量声明    **************/

u8  LED8[8];        //显示缓冲
u8  display_index;  //显示位索引
bit B_1ms;          //1ms标志
u8  cnt_1ms;        //1ms基本计时

u8  IO_KeyState, IO_KeyState1, IO_KeyHoldCnt;   //行列键盘变量
u8  KeyHoldCnt; //键按下计时
u8  KeyCode;    //给用户使用的键码, 1~16有效
u8  cnt_27ms;

/*************  红外发送程序变量声明    **************/

u16 tx_cnt;    //发送或空闲的脉冲计数(等于38KHZ的脉冲数，对应时间), 红外频率为38KHZ, 周期26.3us

/*************  红外接收程序变量声明    **************/

u8  IR_SampleCnt;       //采样计数
u8  IR_BitCnt;          //编码位数
u8  IR_UserH;           //用户码(地址)高字节
u8  IR_UserL;           //用户码(地址)低字节
u8  IR_data;            //数据原码
u8  IR_DataShit;        //数据移位

bit P_IR_RX_temp;       //Last sample
bit B_IR_Sync;          //已收到同步标志
bit B_IR_Press;         //安键动作发生
u8  IR_code;            //红外键码
u16 UserCode;           //用户码

/*************  本地函数声明    **************/
void    IO_KeyScan(void);
void    PWM_config(void);
void    IR_TxPulse(u16 pulse);
void    IR_TxSpace(u16 pulse);
void    IR_TxByte(u8 dat);
void    IR_TxOff(void);
/********************** 主函数 ************************/
void main(void)
{
    u8  i;
    
    P_SW2 |= 0x80;  //扩展寄存器(XFR)访问使能

    P0M1 = 0x30;   P0M0 = 0x30;   //设置P0.4、P0.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P1M1 = 0x30;   P1M0 = 0x30;   //设置P1.4、P1.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P2M1 = 0x3c;   P2M0 = 0x3c;   //设置P2.2~P2.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P3M1 = 0x50;   P3M0 = 0x50;   //设置P3.4、P3.6为漏极开路(实验箱加了上拉电阻到3.3V)
    P4M1 = 0x3c;   P4M0 = 0x3c;   //设置P4.2~P4.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P5M1 = 0x0c;   P5M0 = 0x0c;   //设置P5.2、P5.3为漏极开路(实验箱加了上拉电阻到3.3V)
    P6M1 = 0xff;   P6M0 = 0xff;   //设置为漏极开路(实验箱加了上拉电阻到3.3V)
    P7M1 = 0x00;   P7M0 = 0x00;   //设置为准双向口

    display_index = 0;

    AUXR = 0x80;    //Timer0 set as 1T, 16 bits timer auto-reload, 
    TH0 = (u8)(Timer0_Reload / 256);
    TL0 = (u8)(Timer0_Reload % 256);
    ET0 = 1;    //Timer0 interrupt enable
    TR0 = 1;    //Tiner0 run
    EA = 1;     //打开总中断
    
    PWM_config();
    
    for(i=0; i<8; i++)  LED8[i] = DIS_; //上电显示-
    LED8[4] = DIS_BLACK;
    LED8[5] = DIS_BLACK;

    while(1)
    {
        if(B_1ms)   //1ms到
        {
            B_1ms = 0;
            if(++cnt_27ms >= 27)
            {
                cnt_27ms = 0;
                IO_KeyScan();

                if(KeyCode > 0)     //检测到收到红外键码
                {
                    IR_TxPulse(342);    //对应9ms，同步头       9ms
                    IR_TxSpace(171);    //对应4.5ms，同步头间隔 4.5ms
                    IR_TxPulse(21);     //开始发送数据          0.5625ms

                    IR_TxByte(User_code%256);   //发用户码低字节
                    IR_TxByte(User_code/256);   //发用户码高字节
                    IR_TxByte(KeyCode);         //发数据
                    IR_TxByte(~KeyCode);        //发数据反码
                    IR_TxOff();         //关闭红外发射
									
                    KeyCode = 0;
                }
            }

            if(B_IR_Press)      //检测到收到红外键码
            {
                B_IR_Press = 0;
                
                LED8[0] = (u8)((UserCode >> 12) & 0x0f);    //用户码高字节的高半字节
                LED8[1] = (u8)((UserCode >> 8)  & 0x0f);    //用户码高字节的低半字节
                LED8[2] = (u8)((UserCode >> 4)  & 0x0f);    //用户码低字节的高半字节
                LED8[3] = (u8)(UserCode & 0x0f);            //用户码低字节的低半字节
                LED8[6] = IR_code >> 4;
                LED8[7] = IR_code & 0x0f;
            }
        }
    }
} 

/*****************************************************
    行列键扫描程序
    使用XY查找4x4键的方法，只能单键，速度快

   Y     P04      P05      P06      P07
          |        |        |        |
X         |        |        |        |
P00 ---- K00 ---- K01 ---- K02 ---- K03 ----
          |        |        |        |
P01 ---- K04 ---- K05 ---- K06 ---- K07 ----
          |        |        |        |
P02 ---- K08 ---- K09 ---- K10 ---- K11 ----
          |        |        |        |
P03 ---- K12 ---- K13 ---- K14 ---- K15 ----
          |        |        |        |
******************************************************/

u8 code T_KeyTable[16] = {0,1,2,0,3,0,0,0,4,0,0,0,0,0,0,0};

void IO_KeyDelay(void)
{
    u8 i;
    i = 60;
    while(--i)  ;
}

void    IO_KeyScan(void)    //50ms call
{
    u8  j;

    j = IO_KeyState1;   //保存上一次状态

    P0 = 0xf0;  //X低，读Y
    IO_KeyDelay();
    IO_KeyState1 = P0 & 0xf0;

    P0 = 0x0f;  //Y低，读X
    IO_KeyDelay();
    IO_KeyState1 |= (P0 & 0x0f);
    IO_KeyState1 ^= 0xff;   //取反
    
    if(j == IO_KeyState1)   //连续两次读相等
    {
        j = IO_KeyState;
        IO_KeyState = IO_KeyState1;
        if(IO_KeyState != 0)    //有键按下
        {
            F0 = 0;
            if(j == 0)  F0 = 1; //第一次按下
            else if(j == IO_KeyState)
            {
                if(++IO_KeyHoldCnt >= 37)   //1秒后重键
                {
                    IO_KeyHoldCnt = 33;     //108ms repeat
                    F0 = 1;
                }
            }
            if(F0)
            {
                j = T_KeyTable[IO_KeyState >> 4];
                if((j != 0) && (T_KeyTable[IO_KeyState& 0x0f] != 0)) 
                    KeyCode = (j - 1) * 4 + T_KeyTable[IO_KeyState & 0x0f] + 16;    //计算键码，17~32
            }
        }
        else    IO_KeyHoldCnt = 0;
    }
    P0 = 0xff;
}

/********************** 显示扫描函数 ************************/
void DisplayScan(void)
{   
    P7 = ~T_COM[7-display_index];
    P6 = ~t_display[LED8[display_index]];
    if(++display_index >= 8)    display_index = 0;  //8位结束回0
}


//*******************************************************************
//*********************** IR Remote Module **************************
//*********************** By 梁深 (Coody) 2002-8-24 *****************
//*********************** IR Remote Module **************************
//this programme is used for Receive IR Remote (NEC Code).

//data format: Synchro, AddressH, AddressL, data, /data, (total 32 bit).

//send a frame(85ms), pause 23ms, send synchro of continue frame, pause 94ms

//data rate: 108ms/Frame


//Synchro: low=9ms, high=4.5 / 2.25ms, low=0.5626ms
//Bit0: high=0.5626ms, low=0.5626ms
//Bit1: high=1.6879ms, low=0.5626ms
//frame rate = 108ms ( pause 23 ms or 96 ms)

/******************** 红外采样时间宏定义, 用户不要随意修改  *******************/

#define IR_SAMPLE_TIME      (1000000UL/SysTick)         //查询时间间隔, us, 红外接收要求在60us~250us之间
#if ((IR_SAMPLE_TIME <= 250) && (IR_SAMPLE_TIME >= 60))
    #define D_IR_sample         IR_SAMPLE_TIME      //定义采样时间，在60us~250us之间
#endif

#define D_IR_SYNC_MAX       (15000/D_IR_sample) //SYNC max time
#define D_IR_SYNC_MIN       (9700 /D_IR_sample) //SYNC min time
#define D_IR_SYNC_DIVIDE    (12375/D_IR_sample) //decide data 0 or 1
#define D_IR_DATA_MAX       (3000 /D_IR_sample) //data max time
#define D_IR_DATA_MIN       (600  /D_IR_sample) //data min time
#define D_IR_DATA_DIVIDE    (1687 /D_IR_sample) //decide data 0 or 1
#define D_IR_BIT_NUMBER     32                  //bit number

//*******************************************************************************************
//**************************** IR RECEIVE MODULE ********************************************

void IR_RX_NEC(void)
{
    u8  SampleTime;

    IR_SampleCnt++;                         //Sample + 1

    F0 = P_IR_RX_temp;                      //Save Last sample status
    P_IR_RX_temp = P_IR_RX;                 //Read current status
    if(F0 && !P_IR_RX_temp)                 //Pre-sample is high，and current sample is low, so is fall edge
    {
        SampleTime = IR_SampleCnt;          //get the sample time
        IR_SampleCnt = 0;                   //Clear the sample counter

             if(SampleTime > D_IR_SYNC_MAX)     B_IR_Sync = 0;  //large the Maxim SYNC time, then error
        else if(SampleTime >= D_IR_SYNC_MIN)                    //SYNC
        {
            if(SampleTime >= D_IR_SYNC_DIVIDE)
            {
                B_IR_Sync = 1;                  //has received SYNC
                IR_BitCnt = D_IR_BIT_NUMBER;    //Load bit number
            }
        }
        else if(B_IR_Sync)                      //has received SYNC
        {
            if(SampleTime > D_IR_DATA_MAX)      B_IR_Sync=0;    //data samlpe time too large
            else
            {
                IR_DataShit >>= 1;                  //data shift right 1 bit
                if(SampleTime >= D_IR_DATA_DIVIDE)  IR_DataShit |= 0x80;    //devide data 0 or 1
                if(--IR_BitCnt == 0)                //bit number is over?
                {
                    B_IR_Sync = 0;                  //Clear SYNC
                    if(~IR_DataShit == IR_data)     //判断数据正反码
                    {
                        UserCode = ((u16)IR_UserH << 8) + IR_UserL;
                        IR_code      = IR_data;
                        B_IR_Press   = 1;           //数据有效
                    }
                }
                else if((IR_BitCnt & 7)== 0)        //one byte receive
                {
                    IR_UserL = IR_UserH;            //Save the User code high byte
                    IR_UserH = IR_data;             //Save the User code low byte
                    IR_data  = IR_DataShit;         //Save the IR data byte
                }
            }
        }
    }
}


/********************** Timer0 1ms中断函数 ************************/
void timer0 (void) interrupt 1
{
    IR_RX_NEC();
    if(--cnt_1ms == 0)
    {
        cnt_1ms = SysTick / 1000;
        B_1ms = 1;      //1ms标志
        DisplayScan();  //1ms扫描显示一位
    }
}


/************* 发送脉冲函数 **************/
void IR_TxPulse(u16 pulse)
{
    tx_cnt = pulse;
    PWMA_CCER2 = 0x00; //写 CCMRx 前必须先清零 CCxE 关闭通道
    PWMA_CCMR4 = 0x60; //设置 PWM4 模式1 输出
    PWMA_CCER2 = 0x70; //使能 CC4NE 通道, 低电平有效
    PWMA_IER = 0x10;   //使能捕获/比较 4 中断
    while(tx_cnt);
}

/************* 发送空闲函数 **************/
void IR_TxSpace(u16 pulse)
{
    tx_cnt = pulse;
    PWMA_CCER2 = 0x00; //写 CCMRx 前必须先清零 CCxE 关闭通道
    PWMA_CCMR4 = 0x40; //设置 PWM4 强制为无效电平
    PWMA_CCER2 = 0x70; //使能 CC4NE 通道, 低电平有效
    PWMA_IER = 0x10;   //使能捕获/比较 4 中断
    while(tx_cnt);
}

/************* 发送结束函数 **************/
void IR_TxOff(void)
{
    PWMA_CCER2 = 0x00; //写 CCMRx 前必须先清零 CCxE 关闭通道
    PWMA_CCMR4 = 0x50; //设置 PWM4 强制为有效电平
    PWMA_CCER2 = 0x70; //使能 CC4NE 通道, 低电平有效
}

/************* 发送一个字节函数 **************/
void IR_TxByte(u8 dat)
{
    u8 i;
    for(i=0; i<8; i++)
    {
        if(dat & 1)     IR_TxSpace(63);    //数据1对应 1.6875 + 0.5625 ms 
        else            IR_TxSpace(21);    //数据0对应 0.5625 + 0.5625 ms
        IR_TxPulse(21);         //脉冲都是0.5625ms
        dat >>= 1;              //下一个位
    }
}

//========================================================================
// 函数: void   PWM_config(void)
// 描述: PCA配置函数.
// 参数: None
// 返回: none.
// 版本: V1.0, 2012-11-22
//========================================================================
void PWM_config(void)
{
    PWMA_CCER2 = 0x00; //写 CCMRx 前必须先清零 CCxE 关闭通道
    PWMA_CCMR4 = 0x60; //设置 PWM4 模式1 输出
    //PWMA_CCER2 = 0xB0; //使能 CC4NP 通道, 低电平有效
    //PWMA_CCER2 = 0x70; //使能 CC4NE 通道, 低电平有效

    PWMA_ARRH = 0x02; //设置周期时间
    PWMA_ARRL = 0x77;
    PWMA_CCR4H = 0;
    PWMA_CCR4L = 210; //设置占空比时间

    PWMA_PS = 0x40;  //高级 PWM 通道 4N 输出脚选择位, 0x00:P1.7, 0x40:P2.7, 0x80:P6.7, 0xC0:P3.3
//  PWMA_PS = 0x40;  //高级 PWM 通道 4P 输出脚选择位, 0x00:P1.6, 0x40:P2.6, 0x80:P6.6, 0xC0:P3.4
    PWMA_ENO = 0x80; //使能 PWM4N 输出
//  PWMA_ENO = 0x40; //使能 PWM4P 输出
    PWMA_BKR = 0x80; //使能主输出
//    PWMA_IER = 0x10; // 使能中断
    PWMA_CR1 |= 0x01; //开始计时
}

/******************* PWM中断函数 ********************/
void PWMA_ISR() interrupt 26
{
    if(PWMA_SR1 & 0X10)
    {
        //PWMA_SR1 &=~0X10;
        PWMA_SR1 = 0;
			
        if(--tx_cnt == 0)
        {
            PWMA_CCER2 = 0x00; //写 CCMRx 前必须先清零 CCxE 关闭通道
            PWMA_CCMR4 = 0x40; //设置 PWM4 强制为无效电平
            PWMA_CCER2 = 0x70; //使能 CC4NE 通道, 低电平有效
            PWMA_IER = 0x00;   // 关闭中断
        }
		}
}

