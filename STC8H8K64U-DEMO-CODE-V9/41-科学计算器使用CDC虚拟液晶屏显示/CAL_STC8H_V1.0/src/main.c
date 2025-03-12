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
//#define PRINTF_HID              //printf输出直接重定向到USB HID接口，放在"usb.h"头文件之前

#include "../comm/STC8h.h"  //包含此头文件后，不需要再包含"reg51.h"头文件
#include "../comm/usb.h"     //USB调试及复位所需头文件
#include <intrins.h>
#include <stdio.h>
#include <string.h>
#include "spi.h"
#include "middle.h"
#include "set.h"
#include "menu.h"


char  *USER_DEVICEDESC = NULL;
char  *USER_PRODUCTDESC = NULL;
char  *USER_STCISPCMD = "@STCISP#";                      //设置自动复位到ISP区的用户接口命令

//P3.2口按键复位所需变量
u8 xdata  Key_Flag;
u16 xdata Key_cnt;

void sys_init();
void KeyResetScan(void);


void main()
{
    static u8 xdata Init_flag=0;
    u8 xdata Key_Num=0;    
    P_SW2 |= 0x80;  //XFR访问使能
    
    P0M1 = 0x00;   P0M0 = 0x00;   //设置为准双向口
    P1M1 = 0x00;   P1M0 = 0x00;   //设置为准双向口
    P2M1 = 0x00;   P2M0 = 0x00;   //设置为准双向口
    P3M1 = 0x00;   P3M0 = 0x00;   //设置为准双向口
    P4M1 = 0x00;   P4M0 = 0x00;   //设置为准双向口
    P5M1 = 0x00;   P5M0 = 0x00;   //设置为准双向口
    P6M1 = 0x00;   P6M0 = 0x00;   //设置为准双向口
    P7M1 = 0x00;   P7M0 = 0x00;   //设置为准双向口   

    P3M0 &= ~0x03;
    P3M1 |= 0x03;
 
    IRC48MCR = 0x80;
    while (!(IRC48MCR & 0x01));
    USBCLK = 0x00;
    USBCON = 0x90;

    usb_init();
    Timer0_init();
    Menu_Init();    
    
    /* 开总中断 */
    IE2 |= 0x80;   //IE2相关的中断位操作使能后，需要重新设置EUSB
    EA = 1;
  

    while (1)
    {
        delay_ms(1);
        if(DeviceState != DEVSTATE_CONFIGURED)  //判断USB设备是否配置完成
            continue;
        if (bUsbOutReady)
        {
            if ((UsbOutBuffer[0] == 'K') &&(UsbOutBuffer[1] == 'E') &&(UsbOutBuffer[2] == 'Y') &&(UsbOutBuffer[3] == 'P'))
            {
                if( Init_flag==0 )
                {
                    Init_flag = 1;
                    OLED_LightSet(255);
                    OLED12864_ScrollStop();
                    OLED_DisplayOn();
                    Menu_Display();
                    Screen_Clear();
                } 
                
                Key_Num = CDC_KeyRead( UsbOutBuffer[5] );
            }
            else
            {
                memcpy(UsbInBuffer, UsbOutBuffer, OutNumber);      //原路返回, 用于测试
                usb_IN(OutNumber);
            }
            usb_OUT_done();
        }                  
        delay_ms(9);            
        Menu_Deal(&Key_Num);
        Key_Num = 0;
    }
}