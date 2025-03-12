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

#include	"config.h"
#include	"STC8G_H_GPIO.h"
#include	"STC8G_H_Delay.h"
#include	"STC8G_H_UART.h"
#include	"STC8G_H_SPI.h"
#include	"STC8G_H_NVIC.h"
#include	"STC8G_H_Switch.h"

/*************	功能说明	**************

本例程基于STC8H8K64U为主控芯片的实验箱8进行编写测试，STC8G、STC8H系列芯片可通用参考.

通过串口1(P3.0 P3.1)对PM25LV040/W25X40CL/W25Q80BV进行读写测试。

对FLASH做扇区擦除、写入、读出的操作，命令指定地址。

默认波特率:  115200,8,N,1. 可以在"串口初始化函数"里修改.

串口命令设置: (字母不区分大小写)
    E 0x001234              --> 扇区擦除，指定十六进制地址.
    W 0x001234 1234567890   --> 写入操作，指定十六进制地址，后面为写入内容.
    R 0x001234 10           --> 读出操作，指定十六进制地址，后面为读出字节. 
    C                       --> 如果检测不到PM25LV040/W25X40CL/W25Q80BV/W25Q128FV, 发送C强制允许操作.

注意：为了通用，程序不识别地址是否有效，用户自己根据具体的型号来决定。

下载时, 选择时钟 22.1184MHz (可以在配置文件"config.h"中修改).

******************************************/

/*************	本地常量声明	**************/

sbit SPI_CE  = P2^2;     //PIN1
sbit SPI_SO  = P2^4;     //PIN2
sbit SPI_SI  = P2^3;     //PIN5
sbit SPI_SCK = P2^5;     //PIN6

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

#define EE_BUF_LENGTH   50

/*************	本地变量声明	**************/

u32 Flash_addr;
u8  B_FlashOK;                                //Flash状态
u8  PM25LV040_ID, PM25LV040_ID1, PM25LV040_ID2;

/*************	本地函数声明	**************/

void RX1_Check(void);
void FlashCheckID(void);

/*************  外部函数和变量声明 *****************/


/******************** IO口配置 ********************/
void GPIO_config(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;		//结构定义

	GPIO_InitStructure.Pin  = GPIO_Pin_All;		//指定要初始化的IO, GPIO_Pin_0 ~ GPIO_Pin_7, 或操作
	GPIO_InitStructure.Mode = GPIO_PullUp;		//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P2,&GPIO_InitStructure);	//初始化
}

/****************  串口初始化函数 *****************/
void UART_config(void)
{
	COMx_InitDefine		COMx_InitStructure;					//结构定义

	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;	//模式, UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;			//选择波特率发生器, BRT_Timer1, BRT_Timer2 (注意: 串口2固定使用BRT_Timer2)
	COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率, 一般 110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;				//接收允许,   ENABLE或DISABLE
	COMx_InitStructure.BaudRateDouble = DISABLE;			//波特率加倍, ENABLE或DISABLE
	UART_Configuration(UART1, &COMx_InitStructure);		//初始化串口1 UART1,UART2,UART3,UART4
	NVIC_UART1_Init(ENABLE,Priority_1);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

	UART1_SW(UART1_SW_P30_P31);		//UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17,UART1_SW_P43_P44
}

/****************  SPI初始化函数 *****************/
void SPI_config(void)
{
	SPI_InitTypeDef		SPI_InitStructure;

	SPI_InitStructure.SPI_Enable    = ENABLE;				//SPI启动    ENABLE, DISABLE
	SPI_InitStructure.SPI_SSIG      = ENABLE;				//片选位     ENABLE, DISABLE
	SPI_InitStructure.SPI_FirstBit  = SPI_MSB;				//移位方向   SPI_MSB, SPI_LSB
	SPI_InitStructure.SPI_Mode      = SPI_Mode_Master;		//主从选择   SPI_Mode_Master, SPI_Mode_Slave
	SPI_InitStructure.SPI_CPOL      = SPI_CPOL_High;		//时钟相位   SPI_CPOL_High,   SPI_CPOL_Low
	SPI_InitStructure.SPI_CPHA      = SPI_CPHA_2Edge;		//数据边沿   SPI_CPHA_1Edge,  SPI_CPHA_2Edge
	SPI_InitStructure.SPI_Speed     = SPI_Speed_16;			//SPI速度    SPI_Speed_4,SPI_Speed_16,SPI_Speed_64,SPI_Speed_2/SPI_Speed_32
	SPI_Init(&SPI_InitStructure);
	NVIC_SPI_Init(ENABLE,Priority_3);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
	
	SPI_SW(SPI_P22_P23_P24_P25);	//SPI_P12_P13_P14_P15,SPI_P22_P23_P24_P25,SPI_P54_P40_P41_P43,SPI_P35_P34_P33_P32
    SPI_SCK = 0;    // set clock to low initial state
    SPI_SI = 1;
}

/******************** task A **************************/
void main(void)
{
	EAXSFR();		/* 扩展寄存器访问使能 */
	GPIO_config();
	UART_config();
	SPI_config();
	EA = 1;

    printf("命令设置:\r\n");
    printf("E 0x001234            --> 扇区擦掉  十六进制地址\r\n");
    printf("W 0x001234 1234567890 --> 写入操作  十六进制地址  写入内容\r\n");
    printf("R 0x001234 10         --> 读出操作  十六进制地址  读出字节\r\n");
    printf("C                     --> 如果检测不到SPI Flash, 发送C强制允许操作.\r\n\r\n");

    FlashCheckID();
    FlashCheckID();
    
    if(!B_FlashOK)  printf("未检测到PM25LV040/W25X40CL/W25Q80BV/W25Q128FV!\r\n");
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
        else if(B_FlashOK == 4)
        {
            printf("检测到W25Q128FV!\r\n");
        }
    }
    printf("制造商ID1 = 0x%02bX",PM25LV040_ID1);
    printf("\r\n      ID2 = 0x%02bX",PM25LV040_ID2);
    printf("\r\n   设备ID = 0x%02bX\r\n",PM25LV040_ID);

	while (1)
	{
		delay_ms(1);
				
		if(COM1.RX_TimeOut > 0)
		{
			if(--COM1.RX_TimeOut == 0)
			{
				if(COM1.RX_Cnt > 0)
				{
					RX1_Check();        //串口1处理数据
                    COM1.RX_Cnt = 0;
				}
			}
		}
	}
}

/**************** ASCII码转BIN ****************************/
u8  CheckData(u8 dat)
{
    if((dat >= '0') && (dat <= '9')) return (dat-'0');
    if((dat >= 'A') && (dat <= 'F')) return (dat-'A'+10);
    return 0xff;
}

/**************** 获取写入地址 ****************************/
u32 GetAddress(void)
{
    u32 address;
    u8  i,j;
    
    address = 0;
    if((RX1_Buffer[2] == '0') && (RX1_Buffer[3] == 'X'))
    {
        for(i=4; i<10; i++)
        {
            j = CheckData(RX1_Buffer[i]);
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
    for(i=11; i<COM1.RX_Cnt; i++)
    {
        if(CheckData(RX1_Buffer[i]) >= 10)  break;
        length = length * 10 + CheckData(RX1_Buffer[i]);
    }
    return (length);
}

void FlashCheckID(void)
{
    SPI_CE  = 0;
    SPI_WriteByte(SFC_RDID);        //发送读取ID命令
    SPI_WriteByte(0x00);            //空读3个字节
    SPI_WriteByte(0x00);
    SPI_WriteByte(0x00);
    PM25LV040_ID1 = SPI_ReadByte(); //读取制造商ID1
    PM25LV040_ID  = SPI_ReadByte(); //读取设备ID
    PM25LV040_ID2 = SPI_ReadByte(); //读取制造商ID2
    SPI_CE  = 1;

//    printf("ID1=0x%02bx,ID=0x%02bx,ID2=0x%02bx\r\n",PM25LV040_ID1,PM25LV040_ID,PM25LV040_ID2);

    if((PM25LV040_ID1 == 0x9d) && (PM25LV040_ID2 == 0x7f))  B_FlashOK = 1;  //检测是否为PM25LVxx系列的Flash
    else if(PM25LV040_ID == 0x12)  B_FlashOK = 2;                           //检测是否为W25X4x系列的Flash
    else if(PM25LV040_ID == 0x13)  B_FlashOK = 3;                           //检测是否为W25X8x系列的Flash
    else if(PM25LV040_ID == 0x17)  B_FlashOK = 4;                           //检测是否为W25X128系列的Flash
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

    SPI_CE  = 0;
    SPI_WriteByte(SFC_RDSR);        //发送读取状态命令
    dat = SPI_ReadByte();           //读取状态
    SPI_CE  = 1;

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
    SPI_CE  = 0;
    SPI_WriteByte(SFC_WREN);        //发送写使能命令
    SPI_CE  = 1;
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
        FlashWriteEnable();         //使能Flash写命令
        SPI_CE  = 0;
        SPI_WriteByte(SFC_CHIPER);  //发送片擦除命令
        SPI_CE  = 1;
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
        FlashWriteEnable();     //使能Flash写命令
        SPI_CE  = 0;
        if(B_FlashOK == 1)
        {
            SPI_WriteByte(SFC_SECTORER1);   //发送扇区擦除命令
        }
        else
        {
            SPI_WriteByte(SFC_SECTORER2);   //发送扇区擦除命令
        }
        SPI_WriteByte(((u8 *)&addr)[1]);    //设置起始地址
        SPI_WriteByte(((u8 *)&addr)[2]);
        SPI_WriteByte(((u8 *)&addr)[3]);
        SPI_CE  = 1;
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
static void SPI_Read_Nbytes(u32 addr, u8 *buffer, u16 size)
{
    if(size == 0)   return;
    if(!B_FlashOK)  return;
    while(CheckFlashBusy() > 0);        //Flash忙检测

    SPI_CE  = 0;                        //enable device
    SPI_WriteByte(SFC_READ);            //read command

    SPI_WriteByte(((u8 *)&addr)[1]);    //设置起始地址
    SPI_WriteByte(((u8 *)&addr)[2]);
    SPI_WriteByte(((u8 *)&addr)[3]);

    do{
        *buffer = SPI_ReadByte();       //receive byte and store at buffer
        buffer++;
    }while(--size);                     //read until no_bytes is reached
    SPI_CE  = 1;                        //disable device
}

/************************************************************************
读出n个字节,跟指定的数据进行比较, 错误返回1,正确返回0
************************************************************************/
static u8 SPI_Read_Compare(u32 addr, u8 *buffer, u16 size)
{
    u8  j;
    if(size == 0)   return 2;
    if(!B_FlashOK)  return 2;
    while(CheckFlashBusy() > 0);        //Flash忙检测

    j = 0;
    SPI_CE  = 0;                        //enable device
    SPI_WriteByte(SFC_READ);            //read command
    SPI_WriteByte(((u8 *)&addr)[1]);    //设置起始地址
    SPI_WriteByte(((u8 *)&addr)[2]);
    SPI_WriteByte(((u8 *)&addr)[3]);
    do
    {
        if(*buffer != SPI_ReadByte())   //receive byte and store at buffer
        {
            j = 1;
            break;
        }
        buffer++;
    }while(--size);     //read until no_bytes is reached
    SPI_CE  = 1;        //disable device
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
static void SPI_Write_Nbytes(u32 addr, u8 *buffer, u8 size)
{
    if(size == 0)   return;
    if(!B_FlashOK)  return;
    while(CheckFlashBusy() > 0);        //Flash忙检测


    FlashWriteEnable();                 //使能Flash写命令

    SPI_CE  = 0;                        // enable device
    SPI_WriteByte(SFC_PAGEPROG);        // 发送页编程命令
    SPI_WriteByte(((u8 *)&addr)[1]);    //设置起始地址
    SPI_WriteByte(((u8 *)&addr)[2]);
    SPI_WriteByte(((u8 *)&addr)[3]);
    do{
        SPI_WriteByte(*buffer++);       //连续页内写
        addr++;
        if ((addr & 0xff) == 0) break;
    }while(--size);
    SPI_CE  = 1;                        // disable device
}

/**************** 串口处理函数 ****************************/

void RX1_Check(void)
{
    u8  i,j;
    u8  temp[EE_BUF_LENGTH];

    if((COM1.RX_Cnt == 1) && (RX1_Buffer[0] == 'C'))    //发送C强制允许操作
    {
        B_FlashOK = 1;
        printf("强制允许操作FLASH!\r\n");
    }

    if(!B_FlashOK)
    {
        printf("PM25LV040/W25X40CL/W25Q80BV/W25Q128FV不存在, 不能操作FLASH!\r\n");
        return;
    }
    
    F0 = 0;
    if((COM1.RX_Cnt >= 10) && (RX1_Buffer[1] == ' '))   //最短命令为10个字节
    {
        for(i=0; i<10; i++)
        {
            if((RX1_Buffer[i] >= 'a') && (RX1_Buffer[i] <= 'z'))    RX1_Buffer[i] = RX1_Buffer[i] - 'a' + 'A';//小写转大写
        }
        Flash_addr = GetAddress();
        if(Flash_addr < 0x80000000)
        {
            if(RX1_Buffer[0] == 'E')    //擦除
            {
                FlashSectorErase(Flash_addr);
                printf("已擦掉当前扇区内容!\r\n");
                F0 = 1;
            }

            else if((RX1_Buffer[0] == 'W') && (COM1.RX_Cnt >= 12) && (RX1_Buffer[10] == ' '))   //写入N个字节
            {
                j = COM1.RX_Cnt - 11;
                for(i=0; i<j; i++)  temp[i] = 0xff;      //检测要写入的空间是否为空
                i = SPI_Read_Compare(Flash_addr,temp,j);
                if(i > 0)
                {
                    printf("要写入的地址为非空,不能写入,请先擦掉!\r\n");
                }
                else
                {
                    SPI_Write_Nbytes(Flash_addr,&RX1_Buffer[11],j);     //写N个字节 
                    i = SPI_Read_Compare(Flash_addr,&RX1_Buffer[11],j); //比较写入的数据
                    if(i == 0)
                         printf("已写入%bu字节内容!\r\n",j);
                    else printf("写入错误!\r\n");
                }
                F0 = 1;
            }
            else if((RX1_Buffer[0] == 'R') && (COM1.RX_Cnt >= 12) && (RX1_Buffer[10] == ' '))   //读出N个字节
            {
                j = GetDataLength();
                if((j > 0) && (j < EE_BUF_LENGTH))
                {
                    SPI_Read_Nbytes(Flash_addr,temp,j);
                    printf("读出%bu个字节内容如下：\r\n",j);
                    for(i=0; i<j; i++)  printf("%c",temp[i]);
                    printf("\r\n");
                    F0 = 1;
                }
            }
        }
    }
    if(!F0) printf("命令错误!\r\n");
}
