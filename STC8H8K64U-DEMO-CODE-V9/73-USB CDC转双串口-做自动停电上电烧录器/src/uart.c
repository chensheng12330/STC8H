/*---------------------------------------------------------------------*/
/* --- STC MCU Limited ------------------------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.STCAI.com ---------------------------------------------*/
/* --- BBS: www.STCAIMCU.com  -----------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* 如果要在程序中使用此代码,请在程序中注明使用了STC的资料及程序        */
/*---------------------------------------------------------------------*/

#include "stc.h"
#include "uart.h"
#include "usb.h"
#include "usb_req_class.h"
#include "util.h"

BOOL Uart3TxBusy;
BOOL Uart2TxBusy;

BYTE Uart3RxRptr;
BYTE Uart2RxRptr;
BYTE Uart3RxWptr;
BYTE Uart2RxWptr;

BYTE Uart3TxRptr;
BYTE Uart2TxRptr;
BYTE Uart3TxWptr;
BYTE Uart2TxWptr;

BYTE xdata Uart3RxBuffer[256];
BYTE xdata Uart2RxBuffer[256];
BYTE xdata Uart3TxBuffer[256];
BYTE xdata Uart2TxBuffer[256];

BYTE Count7F;

void uart_init()
{
    S2CON = 0x50;
    T2L = BR(115200);
    T2H = BR(115200) >> 8;
    AUXR |= T2x12;
    AUXR |= T2R;
    IE2 |= ES2;

    S3CON = 0x50;
    T3L = BR(115200);
    T3H = BR(115200) >> 8;
    T4T3M &= 0xf0;
    T4T3M |= 0x0a;
    IE2 |= ES3;

    LineCoding1.dwDTERate = REV4(115200);
    LineCoding1.bCharFormat = 0;
    LineCoding1.bParityType = 0;
    LineCoding1.bDataBits = 8;

    LineCoding2.dwDTERate = REV4(115200);
    LineCoding2.bCharFormat = 0;
    LineCoding2.bParityType = 0;
    LineCoding2.bDataBits = 8;
    
    Uart3TxBusy = 0;
    Uart2TxBusy = 0;
    Uart3RxRptr = 0;
    Uart2RxRptr = 0;
    Uart3RxWptr = 0;
    Uart2RxWptr = 0;
    Uart3TxRptr = 0;
    Uart2TxRptr = 0;
    Uart3TxWptr = 0;
    Uart2TxWptr = 0;
    
    Count7F = 0;
}

void uart2_isr() interrupt UART2_VECTOR
{
    if (S2CON & S2TI)
    {
        S2CON &= ~S2TI;
        Uart2TxBusy = 0;
    }

    if (S2CON & S2RI)
    {
        S2CON &= ~S2RI;
        Uart2TxBuffer[Uart2TxWptr++] = S2BUF;
    }
}

void uart3_isr() interrupt UART3_VECTOR
{
    if (S3CON & S3TI)
    {
        S3CON &= ~S3TI;
        Uart3TxBusy = 0;
    }

    if (S3CON & S3RI)
    {
        S3CON &= ~S3RI;
        Uart3TxBuffer[Uart3TxWptr++] = S3BUF;
    }
}

void uart_set_parity()
{
    if (Interface == 0)
    {
        switch (LineCoding1.bParityType)
        {
        default:
        case NONE_PARITY:
            S3CON = 0x50;
            break;
        case ODD_PARITY:
        case EVEN_PARITY:
        case MARK_PARITY:
            S3CON = 0xd8;
            break;
        case SPACE_PARITY:
            S3CON = 0xd0;
            break;
        }
    }
    else if (Interface == 2)
    {
        switch (LineCoding2.bParityType)
        {
        default:
        case NONE_PARITY:
            S2CON = 0x50;
            break;
        case ODD_PARITY:
        case EVEN_PARITY:
        case MARK_PARITY:
            S2CON = 0xd8;
            break;
        case SPACE_PARITY:
            S2CON = 0xd0;
            break;
        }
    }
}

void uart_set_baud()
{
    DWORD baud;
    WORD temp;
    
    if (Interface == 0)
    {
        baud = LineCoding1.dwDTERate;
    }
    else if (Interface == 2)
    {
        baud = LineCoding2.dwDTERate;
    }
    
    temp = BR(reverse4(baud));
    
    if (Interface == 0)
    {
        T3L = temp;
        T3H = temp >> 8;
    }
    else if (Interface == 2)
    {
        T2L = temp;
        T2H = temp >> 8;
    }
}

void uart_polling()
{
    BYTE dat;
    BYTE cnt;

    if (DeviceState != DEVSTATE_CONFIGURED)
        return;

    if (!Ep4InBusy && (Uart3TxRptr != Uart3TxWptr))
    {
        IE2 &= ~EUSB;
        Ep4InBusy = 1;
        usb_write_reg(INDEX, 4);
        cnt = 0;
        while (Uart3TxRptr != Uart3TxWptr)
        {
            usb_write_reg(FIFO4, Uart3TxBuffer[Uart3TxRptr++]);
            cnt++;
            if (cnt == EP4IN_SIZE) break;
        }
        usb_write_reg(INCSR1, INIPRDY);
        IE2 |= EUSB;
    }

    if (!Ep5InBusy && (Uart2TxRptr != Uart2TxWptr))
    {
        IE2 &= ~EUSB;
        Ep5InBusy = 1;
        usb_write_reg(INDEX, 5);
        cnt = 0;
        while (Uart2TxRptr != Uart2TxWptr)
        {
            usb_write_reg(FIFO5, Uart2TxBuffer[Uart2TxRptr++]);
            cnt++;
            if (cnt == EP5IN_SIZE) break;
        }
        usb_write_reg(INCSR1, INIPRDY);
        IE2 |= EUSB;
    }

    if (!Uart3TxBusy && (Uart3RxRptr != Uart3RxWptr))
    {
        dat = Uart3RxBuffer[Uart3RxRptr++];
        Uart3TxBusy = 1;
        switch (LineCoding1.bParityType)
        {
        case NONE_PARITY:
        case SPACE_PARITY:
            S3CON &= ~S3TB8;
            break;
        case ODD_PARITY:
            ACC = dat;
            if (!P)
            {
                S3CON |= S3TB8;
            }
            else
            {
                S3CON &= ~S3TB8;
            }
            break;
        case EVEN_PARITY:
            ACC = dat;
            if (P)
            {
                S3CON |= S3TB8;
            }
            else
            {
                S3CON &= ~S3TB8;
            }
            break;
        case MARK_PARITY:
            S3CON |= S3TB8;
            break;
        }
        S3BUF = dat;
//        while (Uart3TxBusy);
        
#ifdef ISPPWRCTRL
        //对ISP下载时的握手命令7F进行检测
        {
            extern BYTE IspCount7F;
            
            if (dat == 0x7f)
                IspCount7F++;
            else
                IspCount7F = 0;
        }
#endif
    }

    if (!Uart2TxBusy && (Uart2RxRptr != Uart2RxWptr))
    {
        dat = Uart2RxBuffer[Uart2RxRptr++];
        Uart2TxBusy = 1;
        switch (LineCoding2.bParityType)
        {
        case NONE_PARITY:
        case SPACE_PARITY:
            S2CON &= ~S2TB8;
            break;
        case ODD_PARITY:
            ACC = dat;
            if (!P)
            {
                S2CON |= S2TB8;
            }
            else
            {
                S2CON &= ~S2TB8;
            }
            break;
        case EVEN_PARITY:
            ACC = dat;
            if (P)
            {
                S2CON |= S2TB8;
            }
            else
            {
                S2CON &= ~S2TB8;
            }
            break;
        case MARK_PARITY:
            S2CON |= S2TB8;
            break;
        }
        S2BUF = dat;

//        while (Uart2Busy);
    }

    if (Ep4OutBusy)
    {
        IE2 &= ~EUSB;
        if ((BYTE)(Uart3RxWptr - Uart3RxRptr) < (BYTE)(256 - EP4OUT_SIZE))
        {
            Ep4OutBusy = 0;
            usb_write_reg(INDEX, 4);
            usb_write_reg(OUTCSR1, 0);
        }
        IE2 |= EUSB;
    }

    if (Ep5OutBusy)
    {
        IE2 &= ~EUSB;
        if ((BYTE)(Uart2RxWptr - Uart2RxRptr) < (BYTE)(256 - EP5OUT_SIZE))
        {
            Ep5OutBusy = 0;
            usb_write_reg(INDEX, 5);
            usb_write_reg(OUTCSR1, 0);
        }
        IE2 |= EUSB;
    }
}
