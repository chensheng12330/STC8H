#include "spi.h"
#define SPIF    0x80        //SPI传输完成标志。写入1清0。
#define WCOL    0x40        //SPI写冲突标志。写入1清0。

#if (SPI_DMA_CFG == 1)
bit	SpiDmaFlag;
uint8_t xdata SpiDmaTxBuffer[256]; //_at_ DMA_TX_ADDR;
uint16_t xdata data_index = 0;
#endif

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

void delay(void)
{
    int xdata i;
    
    for (i=0; i<100; i++);
}
void delay_ms(u8 ms)
{
     u16 i;
     do{
          i = MAIN_Fosc / 10000;
          while(--i);   //10T per loop
     }while(--ms);
}


void SPI_init(void)
{
    SPCTL |=  (1 << 7); //忽略 SS 引脚功能，使用 MSTR 确定器件是主机还是从机
    SPCTL |=  (1 << 6); //使能 SPI 功能
    SPCTL &= ~(1 << 5); //先发送/接收数据的高位（ MSB）
    SPCTL |=  (1 << 4); //设置主机模式
    SPCTL &= ~(1 << 3); //SCLK 空闲时为低电平，SCLK 的前时钟沿为上升沿，后时钟沿为下降沿
    SPCTL &= ~(1 << 2); //数据 SS 管脚为低电平驱动第一位数据并在 SCLK 的后时钟沿改变数据
    SPCTL = (SPCTL & ~3) | 3;   //SPI 时钟频率选择, 0: 4T, 1: 8T,  2: 16T,  3: 2T
    
    P_SW1 = (P_SW1 & ~(3<<2)) | (1<<2);     //IO口切换. 0: P1.2/P5.4 P1.3 P1.4 P1.5, 1: P2.2 P2.3 P2.4 P2.5, 2: P5.4 P4.0 P4.1 P4.3, 3: P3.5 P3.4 P3.3 P3.2
    SPI_SCK = 0;    // set clock to low initial state
    SPI_MOSI = 1;
    SPSTAT = SPIF + WCOL;   //清0 SPIF和WCOL标志
}

/************************************************************************/
void SPI_WriteByte(u8 out)
{
    SPDAT = out;
    while((SPSTAT & SPIF) == 0) ;
    SPSTAT = SPIF + WCOL;   //清0 SPIF和WCOL标志
}

void OLED_WR_Byte(u8 dat,u8 cmd)
{
    u8 xdata udat[1] ;
    udat[0] = dat;
    if(cmd)
    {
        OLED_DC = 1;
    }
    else
    {
        OLED_DC = 0;
    }
    SPI_CS = 0;
    SPI_WriteByte(dat);
    delay();
    SPI_CS = 1;
    OLED_DC = 1;
}

//OLED的初始化
void OLED_Init_SPI(void)
{
    SPI_init();
    OLED_REST = 1;
    delay_ms(200);
    OLED_REST=0;//复位
    delay_ms(200);
    OLED_REST = 1;

    OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
    OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
    OLED_WR_Byte(0xCF,OLED_CMD);// Set SEG Output Current Brightness
    OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
    OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
    OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    OLED_WR_Byte(0x00,OLED_CMD);//-not offset
    OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
    OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
    OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
    OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
    OLED_WR_Byte(0x12,OLED_CMD);
    OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
    OLED_WR_Byte(0x40,OLED_CMD);//Set VCOM Deselect Level
    OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
    OLED_WR_Byte(0x00,OLED_CMD);//
    OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
    OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
    OLED_WR_Byte(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
    OLED_WR_Byte(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7) 
    OLED_WR_Byte(0xAF,OLED_CMD);
}

void OLED_Set_Pos(unsigned char x, unsigned char y) 
{ 
	OLED_WR_Byte((u8)(0xb0+y),OLED_CMD);
	OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
	OLED_WR_Byte((x&0x0f)|0x01,OLED_CMD); 
}   
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{ 	
    unsigned int xdata j=0;
    unsigned char xdata x,y;

    for(y=y0;y<(y1+y0);y++)
    {
        OLED_Set_Pos(x0,y);
        for(x=0;x<x1;x++)
        {      
            OLED_WR_Byte(BMP[j++],OLED_DATA);	    	
        }
    }

    
} 


