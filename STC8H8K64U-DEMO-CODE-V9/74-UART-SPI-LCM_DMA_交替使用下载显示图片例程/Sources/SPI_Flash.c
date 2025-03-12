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

#include "STC8H.h"
#include "system.h"
#include "intrins.h"
#include "SPI_Flash.h"
#include "stdio.h"
#include "uart.h"
#include "lcm.h"

/*************  本地常量声明    **************/

#define SPIF    0x80        //SPI传输完成标志。写入1清0。
#define WCOL    0x40        //SPI写冲突标志。写入1清0。

//======== PLL 相关定义 =========
#define HSCK_MCLK       0
#define HSCK_PLL        1
#define HSCK_SEL        HSCK_PLL

#define PLL_96M         0       //PLL时钟 = PLL输入时钟*8
#define PLL_144M        1       //PLL时钟 = PLL输入时钟*12
#define PLL_SEL         PLL_144M

#define CKMS            0x80
#define HSIOCK          0x40
#define MCK2SEL_MSK     0x0c
#define MCK2SEL_SEL1    0x00
#define MCK2SEL_PLL     0x04
#define MCK2SEL_PLLD2   0x08
#define MCK2SEL_IRC48   0x0c
#define MCKSEL_MSK      0x03
#define MCKSEL_HIRC     0x00
#define MCKSEL_XOSC     0x01
#define MCKSEL_X32K     0x02
#define MCKSEL_IRC32K   0x03

#define ENCKM           0x80
#define PCKI_MSK        0x60
#define PCKI_D1         0x00
#define PCKI_D2         0x20
#define PCKI_D4         0x40
#define PCKI_D8         0x60

/*************  本地变量声明    **************/

u32 Flash_addr;
u16 lcdIndex;
u8 xdata DmaBuffer1[DMA_AMT_LEN];
u8 xdata DmaBuffer2[DMA_AMT_LEN];

/*************  FLASH相关变量声明   **************/
sbit    SPI_CS  = P2^2;     //PIN1
sbit    SPI_MISO = P2^4;    //PIN2
sbit    SPI_MOSI = P2^3;    //PIN5
sbit    SPI_SCK = P2^5;     //PIN6

u8  B_FlashOK;                                //Flash状态
u8  PM25LV040_ID, PM25LV040_ID1, PM25LV040_ID2;

bit SpiDmaFlag;
bit DmaBufferSW = 0;

void FlashCheckID(void);

//========================================================================
// 函数: void SPI_DMA_Config(void)
// 描述: SPI DMA 功能配置.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2021-5-6
//========================================================================
void SPI_DMA_Config(void)
{
    //关闭接收DMA，下次接收的数据重新存放在起始地址位置，否则下次接收数据继续往后面存放。
    DMA_SPI_CR = 0x00;		//bit7 1:使能 UART1_DMA, bit5 1:开始 UART1_DMA 自动接收, bit0 1:清除 FIFO

	DMA_SPI_STA = 0x00;
	DMA_SPI_CFG = 0xE5;		//bit7 1:Enable Interrupt, 提供SPI_DMA优先级
	DMA_SPI_AMT = (u8)(DMA_WR_LEN-1);         //设置传输总字节数：n+1

	DMA_SPI_TXAH = (u8)((u16)&DmaBuffer2 >> 8);	//SPI发送数据存储地址
	DMA_SPI_TXAL = (u8)((u16)&DmaBuffer2);
	DMA_SPI_RXAH = (u8)((u16)&DmaBuffer1 >> 8);	//SPI接收数据存储地址
	DMA_SPI_RXAL = (u8)((u16)&DmaBuffer1);

	DMA_SPI_CFG2 = 0x01;	//01:P2.2
	DMA_SPI_CR = 0x81;		//bit7 1:使能 SPI_DMA, bit6 1:开始 SPI_DMA 主机模式, bit0 1:清除 SPI_DMA FIFO
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

#define SPI_CE_High()   SPI_CS  = 1     // set CE high
#define SPI_CE_Low()    SPI_CS  = 0     // clear CE low
#define SPI_Hold()      P_SPI_Hold      = 0     // clear Hold pin
#define SPI_UnHold()    P_SPI_Hold      = 1     // set Hold pin
#define SPI_WP()        P_SPI_WP        = 0     // clear WP pin
#define SPI_UnWP()      P_SPI_WP        = 1     // set WP pin

/************************************************************************/
void SPI_init(void)
{
    P_SW1 = (P_SW1 & ~(3<<2)) | (1<<2);     //IO口切换. 0: P1.2/P5.4 P1.3 P1.4 P1.5, 1: P2.2 P2.3 P2.4 P2.5, 2: P5.4 P4.0 P4.1 P4.3, 3: P3.5 P3.4 P3.3 P3.2

    SPCTL |=  (1 << 7); //忽略 SS 引脚功能，使用 MSTR 确定器件是主机还是从机
    SPCTL |=  (1 << 6); //使能 SPI 功能
    SPCTL &= ~(1 << 5); //先发送/接收数据的高位（ MSB）
    SPCTL |=  (1 << 4); //设置主机模式
    SPCTL &= ~(1 << 3); //SCLK 空闲时为低电平，SCLK 的前时钟沿为上升沿，后时钟沿为下降沿
    SPCTL &= ~(1 << 2); //数据 SS 管脚为低电平驱动第一位数据并在 SCLK 的后时钟沿改变数据
    SPCTL = (SPCTL & ~3) | 0;   //SPI 时钟频率选择, 0: 4T, 1: 8T,  2: 16T,  3: 2T

    SPI_SCK = 0;    // set clock to low initial state
    SPI_MOSI = 1;
    SPSTAT = SPIF + WCOL;   //清0 SPIF和WCOL标志

    FlashCheckID();
    FlashCheckID();
    
    if(!B_FlashOK)  printf("未检测到PM25LV040/W25X40CL/W25Q80BV!\r\n");
    else
    {
        if(B_FlashOK == 1)
        {
            printf("检测到PM25LV040!\r\n");
        }
        else if(B_FlashOK == 2)
        {
            printf("检测到W25X40CL!\r\n");
        }
        else if(B_FlashOK == 3)
        {
            printf("检测到W25Q80BV!\r\n");
        }
        printf("制造商ID1 = 0x%02bX",PM25LV040_ID1);
        printf("\r\n      ID2 = 0x%02bX",PM25LV040_ID2);
        printf("\r\n   设备ID = 0x%02bX\r\n",PM25LV040_ID);
    }
}

/************************************************************************/
void SPI_WriteByte(u8 out)
{
    SPDAT = out;
    while((SPSTAT & SPIF) == 0) ;
    SPSTAT = SPIF + WCOL;   //清0 SPIF和WCOL标志
}

/************************************************************************/
u8 SPI_ReadByte(void)
{
    SPDAT = 0xff;
    while((SPSTAT & SPIF) == 0) ;
    SPSTAT = SPIF + WCOL;   //清0 SPIF和WCOL标志
    return (SPDAT);
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

//	printf("ID1=%bx\r\n",PM25LV040_ID1);
//	printf("ID=%bx\r\n",PM25LV040_ID);
//	printf("ID2=%bx\r\n",PM25LV040_ID2);
	
    if((PM25LV040_ID1 == 0x9d) && (PM25LV040_ID2 == 0x7f))  B_FlashOK = 1;  //检测是否为PM25LVxx系列的Flash
    else if(PM25LV040_ID == 0x12)  B_FlashOK = 2;                           //检测是否为W25X4x系列的Flash
    else if(PM25LV040_ID == 0x13)  B_FlashOK = 3;                           //检测是否为W25X8x系列的Flash
    else                                                    B_FlashOK = 0;
}

/************************************************
检测Flash的忙状态
入口参数: 无
出口参数:
    0 : Flash处于空闲状态
    1 : Flash处于忙状态
************************************************/
u8 CheckFlashBusy(void)
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

/************************************************
擦除扇区, 一个扇区4KB
入口参数: 无
出口参数: 无
************************************************/
//void FlashSectorErase(u32 addr)
//{
//    if(B_FlashOK)
//    {
//        FlashWriteEnable();             //使能Flash写命令
//        SPI_CE_Low();
//        if(B_FlashOK == 1)
//        {
//            SPI_WriteByte(SFC_SECTORER1);    //发送扇区擦除命令
//        }
//        else
//        {
//            SPI_WriteByte(SFC_SECTORER2);    //发送扇区擦除命令
//        }
//        SPI_WriteByte(((u8 *)&addr)[1]);     //设置起始地址
//        SPI_WriteByte(((u8 *)&addr)[2]);
//        SPI_WriteByte(((u8 *)&addr)[3]);
//        SPI_CE_High();
//    }
//}

/************************************************
从Flash中读取数据
入口参数:
    addr   : 地址参数
    buffer : 缓冲从Flash中读取的数据
    size   : 数据块大小
出口参数:
    无
************************************************/
void SPI_Read_Nbytes(u32 addr, u16 len)
{
    if(len == 0)   return;
    if(!B_FlashOK)  return;
    while(SpiDmaFlag);                     //DMA忙检测
    while(CheckFlashBusy() > 0);        //Flash忙检测

    SPI_CE_Low();                       //enable device
    SPI_WriteByte(SFC_READ);            //read command

    SPI_WriteByte(((u8 *)&addr)[1]);    //设置起始地址
    SPI_WriteByte(((u8 *)&addr)[2]);
    SPI_WriteByte(((u8 *)&addr)[3]);

    SpiDmaFlag = 1;
	DMA_SPI_AMT = (u8)(len-1);         //设置传输总字节数：n+1
    DMA_SPI_CR |= 0x40;     //开始SPI_DMA主模式操作
}

/************************************************
写数据到Flash中
入口参数:
    addr   : 地址参数
    size   : 数据块大小
出口参数: 无
************************************************/
void SPI_Write_Nbytes(u32 addr, u16 len)
{
    if(len == 0)   return;
    if(!B_FlashOK)  return;
    while(SpiDmaFlag);                     //DMA忙检测
    while(CheckFlashBusy() > 0);        //Flash忙检测

    FlashWriteEnable();                 //使能Flash写命令

    SPI_CE_Low();                       // enable device
    SPI_WriteByte(SFC_PAGEPROG);        // 发送页编程命令
    SPI_WriteByte(((u8 *)&addr)[1]);    //设置起始地址
    SPI_WriteByte(((u8 *)&addr)[2]);
    SPI_WriteByte(((u8 *)&addr)[3]);

    SpiDmaFlag = 1;
	DMA_SPI_AMT = (u8)(len-1);         //设置传输总字节数：n+1
    DMA_SPI_CR |= 0x40;     //开始SPI_DMA主模式操作
}

//========================================================================
// 函数: void SPI_DMA_Interrupt (void) interrupt 49
// 描述: SPI DMA中断函数
// 参数: none.
// 返回: none.
// 版本: VER1.0
// 日期: 2021-5-8
// 备注: 
//========================================================================
void SPI_DMA_Interrupt(void) interrupt 49   //中断向量号超过31，编译报错的话需安装（例程包根目录下的）Keil中断向量号拓展插件
{
	DMA_SPI_STA = 0;
	SpiDmaFlag = 0;
	SPI_CE_High();
}
