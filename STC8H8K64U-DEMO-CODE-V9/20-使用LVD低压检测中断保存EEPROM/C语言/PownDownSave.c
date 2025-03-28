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

用STC的MCU的IO方式驱动8位数码管。

使用Timer0的16位自动重装来产生1ms节拍,程序运行于这个节拍下,用户修改MCU主时钟频率时,自动定时于1ms.

用户可以在"用户定义宏"中修改掉电保存的EEPROM地址。

显示效果为: 上电后显示秒计数, 计数范围为0~10000，显示在右边的5个数码管。

当掉电后，MCU进入低压中断，对秒计数进行保存。MCU上电时读出秒计数继续显示。

用户可以在"用户定义宏"中选择滤波电容大还是小。
大的电容(比如1000uF)，则掉电后保持的时间长，可以在低压中断中擦除后(需要20多ms时间)然后写入。
小的电容，则掉电后保持的时间短, 则必须在主程序初始化时先擦除.

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
注意：下载时，下载界面"硬件选项"中下面的项要固定如下设置:

不勾选  允许低压复位(禁止低压中断)。

设置用户EEPROM大小，并确保"用户定义宏"中设定的地址在EEPROM设置的大小范围之内。

修改过硬件选项，需要给单片机重新上电后才会生效。

选择时钟 24MHZ (用户可自行修改频率)。
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

******************************************/

#include "stc8h.h"       //包含此头文件后，不需要再包含"reg51.h"头文件
#include "intrins.h"

#define     MAIN_Fosc       24000000L   //定义主时钟

typedef     unsigned char   u8;
typedef     unsigned int    u16;
typedef     unsigned long   u32;

#define     Timer0_16bitAutoReload()    TMOD &= ~0x03                   //16位自动重装
#define     Timer0_16bit()              TMOD  = (TMOD & ~0x03) | 0x01   //16位
#define     Timer0_8bitAutoReload()     TMOD  = (TMOD & ~0x03) | 0x02   //8位自动重装
#define     Timer0_16bitAutoRL_NoMask() TMOD |=  0x03       //16位自动重装不可屏蔽中断
#define     Timer0_AsCounterP32()       TMOD |=  4      //时器0用做计数器
#define     Timer0_AsTimer()            TMOD &= ~4      //时器0用做定时器
#define     Timer0_ExtControlP34()      TMOD |=  4      //时器0由外部INT0高电平允许定时计数
#define     Timer0_Run()                TR0 = 1             //允许定时器0计数
#define     Timer0_Stop()               TR0 = 0             //禁止定时器0计数
#define     Timer0_InterruptEnable()    ET0 = 1             //允许Timer1中断.
#define     Timer0_InterruptDisable()   ET0 = 0             //禁止Timer1中断.
#define     Timer0_1T()                 AUXR |=  (1<<7) //Timer0 clodk = fo
#define     Timer0_12T()                AUXR &= ~(1<<7) //Timer0 clodk = fo/12  12分频, default
#define     Timer0_Load(n)      TH0 = (n) / 256,    TL0 = (n) % 256

#define DIS_DOT     0x20
#define DIS_BLACK   0x10
#define DIS_        0x11

/****************************** 用户定义宏 ***********************************/

#define     LargeCapacitor  0   //0: 滤波电容比较小,  1: 滤波电容比较大

#define     EE_ADDRESS  0x0000  //保存的EEPROM起始地址

#define     Timer0_Reload   (65536UL -(MAIN_Fosc / 1000))       //Timer 0 中断频率, 1000次/秒

#define     Tip_Delay   (MAIN_Fosc / 1000000)

/*****************************************************************************/


/*************  本地常量声明    **************/
u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

u8 code T_COM[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};      //位码


u8  LED8[8];        //显示缓冲
u8  display_index;  //显示位索引
bit B_1ms;          //1ms标志
u16 msecond;

u16 Test_cnt;   //测试用的秒计数变量
u8  tmp[2];     //通用数组


void    Display(void);
void    DisableEEPROM(void);
void    EEPROM_read_n(u16 EE_address,u8 *DataAddress,u16 number);
void    EEPROM_write_n(u16 EE_address,u8 *DataAddress,u16 number);
void    EEPROM_SectorErase(u16 EE_address);

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
    for(i=0; i<8; i++)  LED8[i] = DIS_BLACK;    //全部消隐
    
    Timer0_1T();
    Timer0_AsTimer();
    Timer0_16bitAutoReload();
    Timer0_Load(Timer0_Reload);
    Timer0_InterruptEnable();
    Timer0_Run();
    EA = 1;     //打开总中断
    
    for(msecond=0; msecond < 200; ) //延时200ms
    {
        if(B_1ms)   //1ms到
        {
            B_1ms = 0;
            msecond ++;
        }
    }

    EEPROM_read_n(EE_ADDRESS,tmp,2);        //读出2字节
    Test_cnt = ((u16)tmp[0] << 8) + tmp[1]; //秒计数
    if(Test_cnt > 10000)    Test_cnt = 0;   //秒计数范围为0~10000
    
    #if (LargeCapacitor == 0)   //滤波电容比较小，电容保持时间比较短，则先擦除
        EEPROM_SectorErase(EE_ADDRESS); //擦除一个扇区
    #endif
    
    Display();      //显示秒计数
    
    PCON = PCON & ~(1<<5);  //低压检测标志清0
    ELVD = 1;   //低压监测中断允许
    PLVD = 1;   //低压中断 优先级高

    while(1)
    {
        if(B_1ms)   //1ms到
        {
            B_1ms = 0;
            if(++msecond >= 1000)   //1秒到
            {
                msecond = 0;        //清1000ms计数
                Test_cnt++;         //秒计数+1
                if(Test_cnt > 10000)    Test_cnt = 0;   //秒计数范围为0~10000
                Display();          //显示秒计数
            }
        }
    }
} 
/**********************************************/

/********************** 低压中断函数 ************************/
void LVD_Routine(void) interrupt 6
{
    u8  i;

    P7 = 0xff;     //先关闭显示，省电
    
    #if (LargeCapacitor > 0)    //滤波电容比较大，电容保持时间比较长(50ms以上)，则在中断里擦除
        EEPROM_SectorErase(EE_ADDRESS); //擦除一个扇区
    #endif

    tmp[0] = (u8)(Test_cnt >> 8);
    tmp[1] = (u8)Test_cnt;
    EEPROM_write_n(EE_ADDRESS,tmp,2);

    while(PCON & (1<<5))            //检测是否仍然低电压
    {
        PCON = PCON & ~(1<<5);      //低压检测标志清0
        for(i=0; i<100; i++)    ;   //延时一下
    }
}

/********************** 显示计数函数 ************************/
void Display(void)
{
    u8  i;
    
    LED8[3] = Test_cnt / 10000;
    LED8[4] = (Test_cnt % 10000) / 1000;
    LED8[5] = (Test_cnt % 1000) / 100;
    LED8[6] = (Test_cnt % 100) / 10;
    LED8[7] = Test_cnt % 10;
    
    for(i=3; i<7; i++)  //消无效0
    {
        if(LED8[i] > 0) break;
        LED8[i] = DIS_BLACK;
    }
}

//========================================================================
// 函数: void   ISP_Disable(void)
// 描述: 禁止访问ISP/IAP.
// 参数: non.
// 返回: non.
// 版本: V1.0, 2012-10-22
//========================================================================
void DisableEEPROM(void)
{
    IAP_CONTR = 0;          //关闭 IAP 功能
    IAP_CMD = 0;            //清除命令寄存器
    IAP_TRIG = 0;           //清除触发寄存器
    IAP_ADDRH = 0x80;       //将地址设置到非 IAP 区域
    IAP_ADDRL = 0;
}

//========================================================================
// 函数: void EEPROM_read_n(u16 EE_address,u8 *DataAddress,u16 number)
// 描述: 从指定EEPROM首地址读出n个字节放指定的缓冲.
// 参数: EE_address:  读出EEPROM的首地址.
//       DataAddress: 读出数据放缓冲的首地址.
//       number:      读出的字节长度.
// 返回: non.
// 版本: V1.0, 2012-10-22
//========================================================================
void EEPROM_read_n(u16 EE_address,u8 *DataAddress,u16 number)
{
    EA = 0;     //禁止中断

    IAP_CONTR = 0x80; //使能 IAP
    IAP_TPS = Tip_Delay; //设置擦除等待参数 24MHz
    IAP_CMD = 1;  //设置 IAP 读命令
    do
    {
        IAP_ADDRH = EE_address / 256; //设置 IAP 地址高字节
        IAP_ADDRL = EE_address % 256; //设置 IAP 地址低字节
        IAP_TRIG = 0x5a; //写触发命令(0x5a)
        IAP_TRIG = 0xa5; //写触发命令(0xa5)
        _nop_();
        *DataAddress = IAP_DATA; //读 IAP 数据
        EE_address++;
        DataAddress++;
    }while(--number);
    DisableEEPROM();
    EA = 1;     //重新允许中断
}


/******************** 扇区擦除函数 *****************/
//========================================================================
// 函数: void EEPROM_SectorErase(u16 EE_address)
// 描述: 把指定地址的EEPROM扇区擦除.
// 参数: EE_address:  要擦除的扇区EEPROM的地址.
// 返回: non.
// 版本: V1.0, 2013-5-10
//========================================================================
void EEPROM_SectorErase(u16 EE_address)
{
    EA = 0;     //禁止中断

    IAP_CONTR = 0x80; //使能 IAP
    IAP_TPS = Tip_Delay;     //设置擦除等待参数 24MHz
    IAP_CMD = 3;      //设置 IAP 擦除命令
    IAP_ADDRH = EE_address / 256; //设置 IAP 地址高字节
    IAP_ADDRL = EE_address % 256; //设置 IAP 地址低字节
    IAP_TRIG = 0x5a; //写触发命令(0x5a)
    IAP_TRIG = 0xa5; //写触发命令(0xa5)
    _nop_();
    DisableEEPROM();
    EA = 1;     //重新允许中断
}

//========================================================================
// 函数: void EEPROM_write_n(u16 EE_address,u8 *DataAddress,u16 number)
// 描述: 把缓冲的n个字节写入指定首地址的EEPROM.
// 参数: EE_address:  写入EEPROM的首地址.
//       DataAddress: 写入源数据的缓冲的首地址.
//       number:      写入的字节长度.
// 返回: non.
// 版本: V1.0, 2012-10-22
//========================================================================
void EEPROM_write_n(u16 EE_address,u8 *DataAddress,u16 number)
{
    EA = 0;     //禁止中断

    IAP_CONTR = 0x80; //使能 IAP
    IAP_TPS = Tip_Delay; //设置擦除等待参数 24MHz
    IAP_CMD = 2;  //设置 IAP 写命令
    do
    {
        IAP_ADDRH = EE_address / 256; //设置 IAP 地址高字节
        IAP_ADDRL = EE_address % 256; //设置 IAP 地址低字节
        IAP_DATA = *DataAddress; //写 IAP 数据
        IAP_TRIG = 0x5a; //写触发命令(0x5a)
        IAP_TRIG = 0xa5; //写触发命令(0xa5)
        _nop_();
        EE_address++;
        DataAddress++;
    }while(--number);

    DisableEEPROM();
    EA = 1;     //重新允许中断
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
