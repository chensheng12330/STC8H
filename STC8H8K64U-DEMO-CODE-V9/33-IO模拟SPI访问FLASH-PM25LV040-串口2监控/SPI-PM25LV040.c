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

/*************  功能说明    **************

本例程基于STC8H8K64U为主控芯片的实验箱9进行编写测试，STC系列芯片可通用参考.

通过串口2(P4.6 P4.7)对PM25LV040/W25X40CL/W25Q80BV进行读写测试。

对FLASH做扇区擦除、写入、读出的操作，命令指定地址。

默认波特率:  115200,8,N,1. 可以在"串口初始化函数"里修改.

串口命令设置: (字母不区分大小写)
    E 0x001234              --> 对EEPROM进行扇区擦除操作，指定十六进制地址.
    W 0x001234 1234567890   --> 对EEPROM进行扇区写入操作，指定十六进制地址，后面为写入内容.
    R 0x001234 10           --> 对EEPROM进行扇区读出操作，指定十六进制地址. 
    C                       --> 如果检测不到PM25LV040/W25X40CL/W25Q80BV, 发送C强制允许操作.

注意：为了通用，程序不识别地址是否有效，用户自己根据具体的型号来决定。

下载时, 选择时钟 22.1184MHz (用户可自行修改频率).

******************************************/

#include "stc8h.h"       //包含此头文件后，不需要再包含"reg51.h"头文件
#include "intrins.h"

#define     MAIN_Fosc       22118400L   //定义主时钟（精确计算115200波特率）

typedef     unsigned char   u8;
typedef     unsigned int    u16;
typedef     unsigned long   u32;


#define     Baudrate2           115200L
#define     EE_BUF_LENGTH       50          //


/*************  本地常量声明    **************/

/*************  本地变量声明    **************/
u8  xdata   tmp[EE_BUF_LENGTH];
u8  sst_byte;
u32 Flash_addr;

/*************  FLASH相关变量声明   **************/
sbit    P_PM25LV040_CE  = P2^2;     //PIN1
sbit    P_PM25LV040_SO  = P2^4;     //PIN2
sbit    P_PM25LV040_SI  = P2^3;     //PIN5
sbit    P_PM25LV040_SCK = P2^5;     //PIN6

u8  B_FlashOK;                                //Flash状态
u8  PM25LV040_ID, PM25LV040_ID1, PM25LV040_ID2;

#define     UART2_BUF_LENGTH    (EE_BUF_LENGTH+9)   //串口缓冲长度

u8  RX2_TimeOut;
u8  TX2_Cnt;    //发送计数
u8  RX2_Cnt;    //接收计数
bit B_TX2_Busy; //发送忙标志

u8  xdata   RX2_Buffer[UART2_BUF_LENGTH];   //接收缓冲


void    delay_ms(u8 ms);
void    RX2_Check(void);
void    UART2_config(u8 brt);   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
void    PrintString2(u8 *puts);
void    UART2_TxByte(u8 dat);

void    SPI_init(void);
void    FlashCheckID(void);
u8      CheckFlashBusy(void);
void    FlashWriteEnable(void);
void    FlashChipErase(void);
void    FlashSectorErase(u32 addr);
void    SPI_Read_Nbytes( u32 addr, u8 *buffer, u16 size);
u8      SPI_Read_Compare(u32 addr, u8 *buffer, u16 size);
void    SPI_Write_Nbytes(u32 addr, u8 *buffer,  u8 size);


u8  Hex2Ascii(u8 dat)
{
    dat &= 0x0f;
    if(dat < 10)    return (dat+'0');
    return (dat-10+'A');
}

/******************** 主函数 **************************/
void main(void)
{
    P_SW2 |= 0x80;  //扩展寄存器(XFR)访问使能

    P0M1 = 0x30;   P0M0 = 0x30;   //设置P0.4、P0.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P1M1 = 0x30;   P1M0 = 0x30;   //设置P1.4、P1.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P2M1 = 0x00;   P2M0 = 0x00;   //设置为准双向口
    P3M1 = 0x50;   P3M0 = 0x50;   //设置P3.4、P3.6为漏极开路(实验箱加了上拉电阻到3.3V)
    P4M1 = 0x3c;   P4M0 = 0x3c;   //设置P4.2~P4.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P5M1 = 0x0c;   P5M0 = 0x0c;   //设置P5.2、P5.3为漏极开路(实验箱加了上拉电阻到3.3V)
    P6M1 = 0xff;   P6M0 = 0xff;   //设置为漏极开路(实验箱加了上拉电阻到3.3V)
    P7M1 = 0x00;   P7M0 = 0x00;   //设置为准双向口

    delay_ms(10);
    UART2_config(2);
    EA = 1;     //允许总中断

    PrintString2("命令设置:\r\n");
    PrintString2("E 0x001234            --> 扇区擦除  十六进制地址\r\n");
    PrintString2("W 0x001234 1234567890 --> 写入操作  十六进制地址  写入内容\r\n");
    PrintString2("R 0x001234 10         --> 读出操作  十六进制地址  读出字节\r\n");
    PrintString2("C                     --> 如果检测不到PM25LV040/W25X40CL/W25Q80BV, 发送C强制允许操作.\r\n\r\n");

    SPI_init();
    FlashCheckID();
    FlashCheckID();
    
    if(!B_FlashOK)  PrintString2("未检测到PM25LV040/W25X40CL/W25Q80BV!\r\n");
    else
    {
        if(B_FlashOK == 1)
        {
            PrintString2("检测到PM25LV040!\r\n");
        }
        else if(B_FlashOK == 2)
        {
            PrintString2("检测到W25X40CL!\r\n");
        }
        else if(B_FlashOK == 3)
        {
            PrintString2("检测到W25Q80BV!\r\n");
        }
        PrintString2("制造商ID1 = 0x");
        UART2_TxByte(Hex2Ascii(PM25LV040_ID1 >> 4));
        UART2_TxByte(Hex2Ascii(PM25LV040_ID1));
        PrintString2("    ID2 = 0x");
        UART2_TxByte(Hex2Ascii(PM25LV040_ID2 >> 4));
        UART2_TxByte(Hex2Ascii(PM25LV040_ID2));
        PrintString2("    设备ID = 0x");
        UART2_TxByte(Hex2Ascii(PM25LV040_ID >> 4));
        UART2_TxByte(Hex2Ascii(PM25LV040_ID));
        PrintString2("\r\n");
    }

    while (1)
    {
        delay_ms(1);
                
        if(RX2_TimeOut > 0)
        {
            if(--RX2_TimeOut == 0)  //超时,则串口接收结束
            {
                if(RX2_Cnt > 0)
                {
                    RX2_Check();    //串口1处理数据
                }
                RX2_Cnt = 0;
            }
        }
    }
}


//========================================================================
// 函数: void  delay_ms(unsigned char ms)
// 描述: 延时函数。
// 参数: ms,要延时的ms数, 这里只支持1~255ms. 自动适应主时钟.
// 返回: none.
// 版本: VER1.0
// 日期: 2013-4-1
// 备注: 
//========================================================================
void delay_ms(u8 ms)
{
    u16 i;
    do{
        i = MAIN_Fosc / 13000;
        while(--i)    ;   //14T per loop
    }while(--ms);
}


/**************** ASCII码转BIN ****************************/
u8  CheckData(u8 dat)
{
    if((dat >= '0') && (dat <= '9'))        return (dat-'0');
    if((dat >= 'A') && (dat <= 'F'))        return (dat-'A'+10);
    return 0xff;
}

/**************** 获取写入地址 ****************************/
u32 GetAddress(void)
{
    u32 address;
    u8  i,j;
    
    address = 0;
    if((RX2_Buffer[2] == '0') && (RX2_Buffer[3] == 'X'))
    {
        for(i=4; i<10; i++)
        {
            j = CheckData(RX2_Buffer[i]);
            if(j >= 0x10)   return 0x80000000;  //error
            address = (address << 4) + j;
        }
        return (address);
    }
    return  0x80000000; //error
}

/**************** 获取要读出数据的字节数 ****************************/
u8  GetDataLength(void)
{
    u8  i;
    u8  length;
    
    length = 0;
    for(i=11; i<RX2_Cnt; i++)
    {
        if(CheckData(RX2_Buffer[i]) >= 10)  break;
        length = length * 10 + CheckData(RX2_Buffer[i]);
    }
    return (length);
}


/**************** 串口1处理函数 ****************************/

void RX2_Check(void)
{
    u8  i,j;

    if((RX2_Cnt == 1) && (RX2_Buffer[0] == 'C'))    //发送C强制允许操作
    {
        B_FlashOK = 1;
        PrintString2("强制允许操作FLASH!\r\n");
    }

    if(!B_FlashOK)
    {
        PrintString2("PM25LV040/W25X40CL/W25Q80BV不存在, 不能操作FLASH!\r\n");
        return;
    }
    
    F0 = 0;
    if((RX2_Cnt >= 10) && (RX2_Buffer[1] == ' '))   //最短命令为10个字节
    {
        for(i=0; i<10; i++)
        {
            if((RX2_Buffer[i] >= 'a') && (RX2_Buffer[i] <= 'z'))    RX2_Buffer[i] = RX2_Buffer[i] - 'a' + 'A';//小写转大写
        }
        Flash_addr = GetAddress();
        if(Flash_addr < 0x80000000)
        {
            if(RX2_Buffer[0] == 'E')    //擦除
            {
                FlashSectorErase(Flash_addr);
                PrintString2("已擦除一个扇区内容!\r\n");
                F0 = 1;
            }

            else if((RX2_Buffer[0] == 'W') && (RX2_Cnt >= 12) && (RX2_Buffer[10] == ' '))   //写入N个字节
            {
                j = RX2_Cnt - 11;
                for(i=0; i<j; i++)  tmp[i] = 0xff;      //检测要写入的空间是否为空
                i = SPI_Read_Compare(Flash_addr,tmp,j);
                if(i > 0)
                {
                    PrintString2("要写入的地址为非空,不能写入,请先擦除!\r\n");
                }
                else
                {
                    SPI_Write_Nbytes(Flash_addr,&RX2_Buffer[11],j);     //写N个字节 
                    i = SPI_Read_Compare(Flash_addr,&RX2_Buffer[11],j); //比较写入的数据
                    if(i == 0)
                    {
                        PrintString2("已写入");
                        if(j >= 100)    {UART2_TxByte(j/100+'0');   j = j % 100;}
                        if(j >= 10)     {UART2_TxByte(j/10+'0');    j = j % 10;}
                        UART2_TxByte(j%10+'0');
                        PrintString2("字节内容!\r\n");
                    }
                    else        PrintString2("写入错误!\r\n");
                }
                F0 = 1;
            }
            else if((RX2_Buffer[0] == 'R') && (RX2_Cnt >= 12) && (RX2_Buffer[10] == ' '))   //读出N个字节
            {
                j = GetDataLength();
                if((j > 0) && (j < EE_BUF_LENGTH))
                {
                    SPI_Read_Nbytes(Flash_addr,tmp,j);
                    PrintString2("读出");
                    if(j>=100)  UART2_TxByte(j/100+'0');
                    UART2_TxByte(j%100/10+'0');
                    UART2_TxByte(j%10+'0');
                    PrintString2("个字节内容如下：\r\n");
                    for(i=0; i<j; i++)  UART2_TxByte(tmp[i]);
                    UART2_TxByte(0x0d);
                    UART2_TxByte(0x0a);
                    F0 = 1;
                }
            }
        }
    }
    if(!F0) PrintString2("命令错误!\r\n");
}


//========================================================================
// 函数: void UART2_TxByte(u8 dat)
// 描述: 发送一个字节.
// 参数: 无.
// 返回: 无.
// 版本: V1.0, 2014-6-30
//========================================================================
void UART2_TxByte(u8 dat)
{
    S2BUF = dat;
    B_TX2_Busy = 1;
    while(B_TX2_Busy);
}

//========================================================================
// 函数: void PrintString2(u8 *puts)
// 描述: 串口2发送字符串函数。
// 参数: puts:  字符串指针.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void PrintString2(u8 *puts) //发送一个字符串
{
    for (; *puts != 0;  puts++) UART2_TxByte(*puts);    //遇到停止符0结束
}

//========================================================================
// 函数: SetTimer2Baudraye(u16 dat)
// 描述: 设置Timer2做波特率发生器。
// 参数: dat: Timer2的重装值.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void SetTimer2Baudraye(u16 dat)  // 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
{
    AUXR &= ~(1<<4);    //Timer stop
    AUXR &= ~(1<<3);    //Timer2 set As Timer
    AUXR |=  (1<<2);    //Timer2 set as 1T mode
    T2H = dat / 256;
    T2L = dat % 256;
    IE2  &= ~(1<<2);    //禁止中断
    AUXR |=  (1<<4);    //Timer run enable
}

//========================================================================
// 函数: void UART2_config(u8 brt)
// 描述: UART2初始化函数。
// 参数: brt: 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART2_config(u8 brt)    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
{
    /*********** 波特率固定使用定时器2 *****************/
    if(brt == 2)
    {
        SetTimer2Baudraye(65536UL - (MAIN_Fosc / 4) / Baudrate2);

        S2CON &= ~(1<<7);   // 8位数据, 1位起始位, 1位停止位, 无校验
        IE2   |= 1;         //允许中断
        S2CON |= (1<<4);    //允许接收
        P_SW2 &= ~0x01; 
        P_SW2 |= 1;         //UART2 switch to: 0: P1.0 P1.1,  1: P4.6 P4.7

        RX2_TimeOut = 0;
        B_TX2_Busy = 0;
        TX2_Cnt = 0;
        RX2_Cnt = 0;
    }
}


//========================================================================
// 函数: void UART2_int (void) interrupt UART2_VECTOR
// 描述: UART2中断函数。
// 参数: nine.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART2_int (void) interrupt 8
{
    if((S2CON & 1) != 0)
    {
        S2CON &= ~1;    //Clear Rx flag
        RX2_Buffer[RX2_Cnt] = S2BUF;
        if(++RX2_Cnt >= UART2_BUF_LENGTH)   RX2_Cnt = 0;
        RX2_TimeOut = 5;
    }

    if((S2CON & 2) != 0)
    {
        S2CON &= ~2;    //Clear Tx flag
        B_TX2_Busy = 0;
    }
}


/******************* FLASH相关程序 ************************/
#define SFC_WREN        0x06        //串行Flash命令集
#define SFC_WRDI        0x04
#define SFC_RDSR        0x05
#define SFC_WRSR        0x01
#define SFC_READ        0x03
#define SFC_FASTREAD    0x0B
#define SFC_RDID        0xAB
#define SFC_PAGEPROG    0x02
#define SFC_RDCR        0xA1
#define SFC_WRCR        0xF1
#define SFC_SECTORER1   0xD7        //PM25LV040 扇区擦除指令
#define SFC_SECTORER2   0x20        //W25Xxx 扇区擦除指令
#define SFC_BLOCKER     0xD8
#define SFC_CHIPER      0xC7


#define SPI_CE_High()   P_PM25LV040_CE  = 1     // set CE high
#define SPI_CE_Low()    P_PM25LV040_CE  = 0     // clear CE low
#define SPI_Hold()      P_SPI_Hold      = 0     // clear Hold pin
#define SPI_UnHold()    P_SPI_Hold      = 1     // set Hold pin
#define SPI_WP()        P_SPI_WP        = 0     // clear WP pin
#define SPI_UnWP()      P_SPI_WP        = 1     // set WP pin


/************************************************************************/
void SPI_init(void)
{
    SPI_CE_High();
    P_PM25LV040_SCK = 0;    // set clock to low initial state
    P_PM25LV040_SI = 1;
}


/************************************************************************/
void SPI_WriteByte(u8 out)
{
    u8 i;
    i = 8;
    do{
        out <<= 1;
        P_PM25LV040_SI  = CY;
        P_PM25LV040_SCK = 1;
        P_PM25LV040_SCK = 0;
    }while(--i);
    P_PM25LV040_SI = 1;
}

/************************************************************************/
u8 SPI_ReadByte(void)
{
    u8 i, in;
    i = 8;
    do{
        in <<= 1;
        if (P_PM25LV040_SO) in++;
        P_PM25LV040_SCK = 1;
        P_PM25LV040_SCK = 0;
    }while(--i);
    return in;
}


/************************************************
检测Flash是否准备就绪
入口参数: 无
出口参数:
    0 : 没有检测到正确的Flash
    1 : Flash准备就绪
************************************************/
void FlashCheckID(void)
{
    SPI_CE_Low();
    SPI_WriteByte(SFC_RDID);        //发送读取ID命令
    SPI_WriteByte(0x00);            //空读3个字节
    SPI_WriteByte(0x00);
    SPI_WriteByte(0x00);
    PM25LV040_ID1 = SPI_ReadByte();         //读取制造商ID1
    PM25LV040_ID  = SPI_ReadByte();         //读取设备ID
    PM25LV040_ID2 = SPI_ReadByte();         //读取制造商ID2
    SPI_CE_High();

    if((PM25LV040_ID1 == 0x9d) && (PM25LV040_ID2 == 0x7f))  B_FlashOK = 1;  //检测是否为PM25LVxx系列的Flash
    else if(PM25LV040_ID == 0x12)  B_FlashOK = 2;                           //检测是否为W25X4x系列的Flash
    else if(PM25LV040_ID == 0x13)  B_FlashOK = 3;                           //检测是否为W25X8x系列的Flash
    else B_FlashOK = 0;
}


/************************************************
检测Flash的忙状态
入口参数: 无
出口参数:
    0 : Flash处于空闲状态
    1 : Flash处于忙状态
************************************************/
u8  CheckFlashBusy(void)
{
    u8  dat;

    SPI_CE_Low();
    SPI_WriteByte(SFC_RDSR);        //发送读取状态命令
    dat = SPI_ReadByte();           //读取状态
    SPI_CE_High();

    return (dat);                   //状态值的Bit0即为忙标志
}

/************************************************
使能Flash写命令
入口参数: 无
出口参数: 无
************************************************/
void FlashWriteEnable(void)
{
    while(CheckFlashBusy() > 0);    //Flash忙检测
    SPI_CE_Low();
    SPI_WriteByte(SFC_WREN);        //发送写使能命令
    SPI_CE_High();
}

/************************************************
擦除整片Flash
入口参数: 无
出口参数: 无
************************************************/
/*
void FlashChipErase(void)
{
    if(B_FlashOK)
    {
        FlashWriteEnable();             //使能Flash写命令
        SPI_CE_Low();
        SPI_WriteByte(SFC_CHIPER);      //发送片擦除命令
        SPI_CE_High();
    }
}
*/

/************************************************
擦除扇区, 一个扇区4KB
入口参数: 无
出口参数: 无
************************************************/
void FlashSectorErase(u32 addr)
{
    if(B_FlashOK)
    {
        FlashWriteEnable();             //使能Flash写命令
        SPI_CE_Low();
        if(B_FlashOK == 1)
        {
            SPI_WriteByte(SFC_SECTORER1);    //发送扇区擦除命令
        }
        else
        {
            SPI_WriteByte(SFC_SECTORER2);    //发送扇区擦除命令
        }
        SPI_WriteByte(((u8 *)&addr)[1]);           //设置起始地址
        SPI_WriteByte(((u8 *)&addr)[2]);
        SPI_WriteByte(((u8 *)&addr)[3]);
        SPI_CE_High();
    }
}

/************************************************
从Flash中读取数据
入口参数:
    addr   : 地址参数
    buffer : 缓冲从Flash中读取的数据
    size   : 数据块大小
出口参数:
    无
************************************************/
void SPI_Read_Nbytes(u32 addr, u8 *buffer, u16 size)
{
    if(size == 0)   return;
    if(!B_FlashOK)  return;
    while(CheckFlashBusy() > 0);        //Flash忙检测

    SPI_CE_Low();                       //enable device
    SPI_WriteByte(SFC_READ);            //read command

    SPI_WriteByte(((u8 *)&addr)[1]);    //设置起始地址
    SPI_WriteByte(((u8 *)&addr)[2]);
    SPI_WriteByte(((u8 *)&addr)[3]);

    do{
        *buffer = SPI_ReadByte();       //receive byte and store at buffer
        buffer++;
    }while(--size);                     //read until no_bytes is reached
    SPI_CE_High();                      //disable device
}

/************************************************************************
读出n个字节,跟指定的数据进行比较, 错误返回1,正确返回0
************************************************************************/
u8  SPI_Read_Compare(u32 addr, u8 *buffer, u16 size)
{
    u8  j;
    if(size == 0)   return 2;
    if(!B_FlashOK)  return 2;
    while(CheckFlashBusy() > 0);            //Flash忙检测

    j = 0;
    SPI_CE_Low();                           //enable device
    SPI_WriteByte(SFC_READ);                //read command
    SPI_WriteByte(((u8 *)&addr)[1]);        //设置起始地址
    SPI_WriteByte(((u8 *)&addr)[2]);
    SPI_WriteByte(((u8 *)&addr)[3]);
    do
    {
        if(*buffer != SPI_ReadByte())       //receive byte and store at buffer
        {
            j = 1;
            break;
        }
        buffer++;
    }while(--size);         //read until no_bytes is reached
    SPI_CE_High();          //disable device
    return j;
}


/************************************************
写数据到Flash中
入口参数:
    addr   : 地址参数
    buffer : 缓冲需要写入Flash的数据
    size   : 数据块大小
出口参数: 无
************************************************/
void SPI_Write_Nbytes(u32 addr, u8 *buffer, u8 size)
{
    if(size == 0)   return;
    if(!B_FlashOK)  return;
    while(CheckFlashBusy() > 0);        //Flash忙检测


    FlashWriteEnable();                 //使能Flash写命令

    SPI_CE_Low();                       // enable device
    SPI_WriteByte(SFC_PAGEPROG);        // 发送页编程命令
    SPI_WriteByte(((u8 *)&addr)[1]);    //设置起始地址
    SPI_WriteByte(((u8 *)&addr)[2]);
    SPI_WriteByte(((u8 *)&addr)[3]);
    do{
        SPI_WriteByte(*buffer++);       //连续页内写
        addr++;
        if ((addr & 0xff) == 0) break;
    }while(--size);
    SPI_CE_High();                      // disable device
}

