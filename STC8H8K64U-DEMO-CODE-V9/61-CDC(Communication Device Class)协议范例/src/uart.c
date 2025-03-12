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

#include "stc.h"
#include "uart.h"
#include "usb.h"
#include "usb_req_class.h"

BOOL UartBusy;

void uart_init()
{
    P_SW2 &= ~0x01;     //UART2 switch to: 0: P1.0 P1.1
    P_SW2 |= 1;         //UART2 switch to: 1: P4.6 P4.7

    P4M0 &= ~0xc0;
    P4M1 &= ~0xc0;

    S2CON = 0x50;
    T2L = BR(115200);
    T2H = BR(115200) >> 8;
	AUXR |= 0x14;   //定时器2时钟1T模式,开始计时
    IP2 |= 0x01;    //提高串口2中断优先级，防止数据丢失
    IE2  |= 1;      //允许中断

    LineCoding.dwDTERate = 0x00c20100;  //115200
    LineCoding.bCharFormat = 0;
    LineCoding.bParityType = 0;
    LineCoding.bDataBits = 8;
}

void uart2_isr() interrupt 8
{
    if((S2CON & 2) != 0)
    {
        S2CON &= ~2;    //Clear Tx flag
        UartBusy = 0;
    }

    if((S2CON & 1) != 0)
    {
        S2CON &= ~1;    //Clear Rx flag
        TxBuffer[TxWptr++] = S2BUF;
    }
}

void uart_set_parity(BYTE parity)
{
    switch (parity)
    {
    case NONE_PARITY:
        S2CON = 0x50;
        break;
    case ODD_PARITY:
    case EVEN_PARITY:
    case MARK_PARITY:
        S2CON = 0xda;
        break;
    case SPACE_PARITY:
        S2CON = 0xd2;
        break;
    }
}

void uart_set_baud(DWORD baud)
{
    WORD temp;

    switch (baud)
    {
    case 300:
        T2L = BR(300);
        T2H = BR(300) >> 8;
        break;
    case 600:
        T2L = BR(600);
        T2H = BR(600) >> 8;
        break;
    case 1200:
        T2L = BR(1200);
        T2H = BR(1200) >> 8;
        break;
    case 2400:
        T2L = BR(2400);
        T2H = BR(2400) >> 8;
        break;
    case 4800:
        T2L = BR(4800);
        T2H = BR(4800) >> 8;
        break;
    case 9600:
        T2L = BR(9600);
        T2H = BR(9600) >> 8;
        break;
    case 14400:
        T2L = BR(14400);
        T2H = BR(14400) >> 8;
        break;
    case 19200:
        T2L = BR(19200);
        T2H = BR(19200) >> 8;
        break;
    case 28800:
        T2L = BR(28800);
        T2H = BR(28800) >> 8;
        break;
    case 38400:
        T2L = BR(38400);
        T2H = BR(38400) >> 8;
        break;
    case 57600:
        T2L = BR(57600);
        T2H = BR(57600) >> 8;
        break;
    case 115200:
        T2L = BR(115200);
        T2H = BR(115200) >> 8;
        break;
    case 230400:
        T2L = BR(230400);
        T2H = BR(230400) >> 8;
        break;
    case 460800:
        T2L = BR(460800);
        T2H = BR(460800) >> 8;
        break;
    default:
        temp = (WORD)BR(baud);
        T2L = temp;
        T2H = temp >> 8;
        break;
    }
}

void uart_polling()
{
    BYTE dat;
    BYTE cnt;

    if (DeviceState != DEVSTATE_CONFIGURED)
        return;

    if (!UsbInBusy && (TxRptr != TxWptr))
    {
        IE2 &= ~0x80;   //EUSB = 0;
        UsbInBusy = 1;
        usb_write_reg(INDEX, 1);
        cnt = 0;
        while (TxRptr != TxWptr)
        {
            usb_write_reg(FIFO1, TxBuffer[TxRptr++]);
            cnt++;
            if (cnt == EP1IN_SIZE) break;
        }
        usb_write_reg(INCSR1, INIPRDY);
        IE2 |= 0x80;    //EUSB = 1;
    }

    if (!UartBusy && (RxRptr != RxWptr))
    {
        dat = RxBuffer[RxRptr++];
        UartBusy = 1;
        switch (LineCoding.bParityType)
        {
        case NONE_PARITY:
        case SPACE_PARITY:
            S2CON &= ~0x08;     //S2TB8 = 0;
            break;
        case ODD_PARITY:
            ACC = dat;
            if(P) S2CON &= ~0x08;   //S2TB8 = !P;
            else  S2CON |= 0x08;
            break;
        case EVEN_PARITY:
            ACC = dat;
            if(P) S2CON |= 0x08;    //S2TB8 = P;
            else  S2CON &= ~0x08;
            break;
        case MARK_PARITY:
            S2CON |= 0x08;      //S2TB8 = 1;
            break;
        }
        S2BUF = dat;

        while (UartBusy);
    }

    if (UsbOutBusy)
    {
        IE2 &= ~0x80;   //EUSB = 0;
        if (RxWptr - RxRptr < 256 - EP1OUT_SIZE)
        {
            UsbOutBusy = 0;
            usb_write_reg(INDEX, 1);
            usb_write_reg(OUTCSR1, 0);
        }
        IE2 |= 0x80;    //EUSB = 1;
    }
}
