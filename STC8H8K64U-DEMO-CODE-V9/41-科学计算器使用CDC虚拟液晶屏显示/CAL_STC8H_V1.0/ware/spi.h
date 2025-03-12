#ifndef __SPI_H_
#define __SPI_H_

#include "../comm/STC8h.h"  //包含此头文件后，不需要再包含"reg51.h"头文件
#include "../comm/usb.h"     //USB调试及复位所需头文件
#define U8g2_SPI_MODE   1    /* SPI 模式使能 */
#define SPI_DMA_CFG     1    /* SPI DMA 使能 */

sbit    SPI_CS  = P2^2;     //PIN1
sbit    SPI_MISO = P2^4;    //PIN2
sbit    SPI_MOSI = P2^3;    //PIN5
sbit    SPI_SCK = P2^5;     //PIN6

#if (U8g2_SPI_MODE == 1)
sbit    OLED_DC = P2^1;
sbit    OLED_REST = P2^0;
#endif

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

void HAL_SPI_Init();
void HAL_SPI_Transmit(uint8_t *Mem, uint16_t MemSize, uint32_t Timeout);
void delay_ms(u8 ms);

void OLED_WR_Byte(u8 dat,u8 cmd);
void OLED_Init_SPI(void);
 void delay(void);
 void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);
#endif /*__SPI_H__*/



