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

#include	"config.h"
//#include "stc.h"
#include "uart.h"
#include "usb.h"

BOOL RxFlag;
BYTE RxCount;
BYTE xdata RxBuffer[64];
BYTE xdata TxBuffer[1024];

void uart_init()
{
    RxFlag = 0;
    RxCount = 0;
}

void uart_send(int size)
{
    BYTE cnt;
    int addr;
    
    IE2 &= ~EUSB;
    usb_write_reg(INDEX, 1);
    
    addr = 0;
    do
    {
        cnt = size > 64 ? 64 : size;
        while (usb_read_reg(INCSR1) & INIPRDY);
        usb_bulk_intr_in(&TxBuffer[addr], cnt, 1);
        addr += cnt;
        size -= cnt;
    } while (cnt >= 64);
    
    IE2 |= EUSB;
}

void uart_recv_done()
{
    IE2 &= ~EUSB;
    
    RxFlag = 0;
    usb_write_reg(INDEX, 1);
    usb_write_reg(OUTCSR1, 0);
    
    IE2 |= EUSB;
}
