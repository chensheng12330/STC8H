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

#ifndef	__SPI_FLASH_H
#define	__SPI_FLASH_H

extern u32 Flash_addr;
extern bit SpiDmaFlag;

void SPI_init(void);
void SPI_DMA_Config(void);
void FlashChipErase(void);

void SPI_Write_Nbytes(u32 addr, u16 len);
void SPI_Read_Nbytes(u32 addr, u16 len);

#endif
