/*---------------------------------------------------------------------*/
/* --- STC MCU Limited ------------------------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.STCAI.com ---------------------------------------------*/
/* --- BBS: www.STCAIMCU.com  -----------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* 液晶屏驱动程序参考wiki技术网站提供的开源源码，仅供学习使用          */
/* 如果要在程序中使用此代码,请在程序中注明使用了STC的资料及程序        */
/*---------------------------------------------------------------------*/

//=========================================电源接线================================================//
//VDD接DC 3.3V电源
//GND接地
//=======================================液晶屏数据线接线==========================================//
//本模块数据总线类型为8位并口
//液晶屏模块             STC8H8K64U
// LCD_D0~D7     接       P60~P67        //液晶屏8位数据线
//=======================================液晶屏控制线接线==========================================//
//液晶屏模块             STC8H8K64U
// LCD_RS        接        P45           //数据/命令选择控制信号
// LCD_RD        接        P44           //读控制信号
// LCD_RST       接        P43           //复位信号
// LCD_WR        接        P42           //写控制信号
// LCD_CS        接        P34           //片选控制信号
//===========================================触摸屏接线============================================//
//不使用触摸或者模块本身不带触摸，则可不连接
//触摸屏使用的数据总线类型为SPI
//触摸屏模块             STC8H8K64U
//  T_CS         接        P52          //触摸屏片选控制信号
//  T_CLK        接        P25          //触摸屏SPI时钟信号
//  T_DO         接        P24          //触摸屏SPI读信号
//  T_DIN        接        P23          //触摸屏SPI写信号
//  T_IRQ        接        P36          //触摸屏响应检查信号
//**************************************************************************************************/	
#ifndef __PIC_H
#define __PIC_H 

//扫描模式：水平扫描，从左到右，从上到下
//存储顺序：低位在前
//宽度高度：40*40
//数据格式：C表格
//输出格式：16位彩色-565
unsigned char code gImage_qq[3200] = 
{
    0xDB,0xDE,0x5D,0xEF,0x5D,0xEF,0x7D,0xEF,0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,
    0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,0xBE,0xF7,
    0xFF,0xFF,0xDE,0xFF,0x38,0xC6,0x92,0x8C,0x8E,0x6B,0x6E,0x6B,0x10,0x7C,0x96,0xAD,
    0x3C,0xE7,0xFF,0xFF,0xFF,0xFF,0x7D,0xEF,0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,
    0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,0x5D,0xEF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5D,0xEF,
    0x15,0x9D,0x4F,0x63,0x6C,0x42,0x0A,0x32,0x88,0x29,0x46,0x19,0x25,0x19,0x45,0x21,
    0xE8,0x31,0x8E,0x6B,0x18,0xBE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xDF,0xFF,0xF5,0x9C,0x0F,0x53,
    0x10,0x4B,0x51,0x53,0x0F,0x4B,0x6C,0x3A,0xE9,0x31,0x67,0x21,0x25,0x19,0xE4,0x10,
    0xA3,0x08,0x62,0x00,0x83,0x08,0x8A,0x4A,0x59,0xC6,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x1C,0xE7,0x4F,0x5B,0xB3,0x63,0xB8,0x7C,
    0xF5,0x63,0x11,0x43,0x4D,0x32,0xEA,0x29,0x88,0x21,0x26,0x19,0x05,0x19,0x05,0x19,
    0x04,0x11,0x04,0x11,0xE4,0x10,0x83,0x00,0xA3,0x08,0x72,0x8C,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xDB,0xDE,0x4B,0x3A,0xF0,0x42,0x35,0x6C,0x54,0x4B,
    0xB1,0x32,0x2E,0x2A,0xEB,0x21,0xA9,0x21,0x67,0x19,0x05,0x19,0x04,0x11,0x04,0x11,
    0x04,0x11,0x04,0x11,0x04,0x11,0x05,0x19,0xE4,0x10,0x42,0x00,0xAF,0x73,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0x5D,0xEF,0x09,0x32,0x4C,0x32,0x10,0x4B,0x8F,0x32,0x4F,0x2A,
    0x2E,0x2A,0xCC,0x19,0x89,0x19,0x89,0x21,0x47,0x19,0x05,0x19,0x04,0x11,0x04,0x11,
    0xC4,0x10,0xC4,0x10,0x04,0x11,0x04,0x11,0x04,0x11,0xE4,0x10,0x42,0x00,0x31,0x84,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xEC,0x52,0x47,0x19,0x4C,0x32,0x0B,0x2A,0xEC,0x21,0xEC,0x21,
    0x0C,0x22,0x91,0x5B,0xEE,0x4A,0x06,0x11,0x26,0x19,0x04,0x19,0xE4,0x10,0xE4,0x10,
    0xA7,0x29,0x66,0x21,0xA3,0x08,0x05,0x19,0x04,0x11,0x04,0x11,0xE4,0x10,0x82,0x00,
    0xF7,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0x35,0xA5,0x83,0x08,0x88,0x21,0x88,0x21,0x89,0x21,0xAA,0x21,0x8A,0x21,
    0x6B,0x42,0x71,0x8C,0xFF,0xFF,0x72,0x8C,0x83,0x08,0x04,0x11,0xC4,0x08,0x29,0x42,
    0xFB,0xDE,0x5D,0xEF,0xEC,0x5A,0x83,0x08,0x04,0x11,0x04,0x11,0x04,0x11,0x83,0x08,
    0xE8,0x31,0xDF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xBE,0xF7,0xC7,0x31,0xC4,0x10,0x25,0x19,0x26,0x19,0x47,0x19,0x47,0x19,0xA8,0x29,
    0x8A,0x52,0x28,0x4A,0x55,0xAD,0xFF,0xFF,0xE8,0x31,0xA3,0x08,0x05,0x19,0x4D,0x6B,
    0x4D,0x6B,0xFF,0xFF,0x7D,0xEF,0x45,0x21,0xC4,0x10,0x04,0x11,0x04,0x11,0x04,0x11,
    0x62,0x00,0x76,0xAD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x96,0xB5,0x62,0x00,0x04,0x11,0x04,0x19,0x05,0x11,0x05,0x19,0xC4,0x08,0x8B,0x4A,
    0xB6,0xB5,0x5D,0xEF,0xF7,0xBD,0xFF,0xFF,0x8E,0x6B,0x62,0x00,0x29,0x42,0xAA,0x5A,
    0x08,0x42,0xFF,0xFF,0xFF,0xFF,0xCC,0x52,0x83,0x08,0x04,0x11,0x04,0x11,0x04,0x11,
    0xA3,0x08,0xAD,0x52,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xBE,0xF7,0xFF,0xFF,0xDF,0xFF,0xDB,0xDE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x4E,0x63,0x62,0x00,0x04,0x11,0x04,0x11,0x04,0x11,0xE4,0x10,0x62,0x00,0x8E,0x63,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xCF,0x73,0x01,0x00,0xF3,0x9C,0x2C,0x63,
    0x96,0xB5,0xFF,0xFF,0xFF,0xFF,0x2D,0x5B,0x83,0x00,0x04,0x11,0x04,0x11,0x04,0x11,
    0xE4,0x10,0x67,0x21,0x3D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xB7,0xB5,0x76,0xAD,0x7D,0xEF,0xCB,0x52,0xB3,0x94,0xFF,0xFF,0xFF,0xFF,0xDF,0xFF,
    0xE8,0x31,0xA3,0x08,0x04,0x11,0x04,0x11,0x04,0x11,0x04,0x11,0xA3,0x08,0x49,0x42,
    0xFF,0xF7,0xFF,0xF7,0xFF,0xFF,0xFF,0xFF,0x6A,0x4A,0x01,0x00,0x72,0x84,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xF7,0xDF,0xEF,0x09,0x3A,0xA3,0x08,0x04,0x11,0x04,0x11,0x04,0x11,
    0x04,0x11,0x05,0x11,0x18,0xBE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xF0,0x7B,0x62,0x00,0xE8,0x31,0xC7,0x31,0x41,0x00,0x35,0xA5,0xFF,0xFF,0x5D,0xEF,
    0x46,0x21,0xC4,0x10,0x04,0x11,0x04,0x11,0x04,0x11,0x04,0x11,0xE4,0x10,0xA3,0x08,
    0x76,0x9D,0xFF,0xF7,0xFF,0xFF,0xB7,0xAD,0xA3,0x08,0xA3,0x08,0xC7,0x31,0x9E,0xE7,
    0xFF,0xF7,0xFF,0xF7,0x76,0xA5,0xA3,0x08,0xE4,0x10,0x04,0x11,0x04,0x11,0x04,0x11,
    0x05,0x11,0x05,0x11,0x35,0xA5,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xDB,0xDE,0xA7,0x29,0x83,0x00,0xC4,0x10,0xC4,0x10,0xC4,0x10,0x1C,0xE7,0x9E,0xEF,
    0x05,0x11,0xE4,0x10,0x04,0x11,0x04,0x11,0x04,0x11,0x04,0x11,0x04,0x19,0xC4,0x08,
    0xE5,0x10,0xD1,0x6B,0xD1,0x6B,0xC5,0x08,0x64,0x00,0xA5,0x08,0x43,0x00,0x2B,0x32,
    0x77,0x9D,0xB3,0x84,0x25,0x19,0xC4,0x10,0x04,0x11,0x04,0x11,0x04,0x11,0x04,0x11,
    0x25,0x19,0x26,0x09,0x35,0x9D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xAF,0x73,0x62,0x00,0x04,0x19,0x05,0x19,0x82,0x00,0x0D,0x5B,0x8E,0x9B,
    0x62,0x10,0x05,0x11,0x04,0x11,0x04,0x11,0x04,0x19,0xE4,0x10,0x85,0x00,0x05,0x11,
    0xC4,0x39,0x81,0x5A,0x40,0x7B,0x22,0x9C,0x43,0xAC,0x03,0xA4,0x83,0x9B,0x82,0x72,
    0x82,0x49,0xC2,0x18,0xA4,0x00,0xC5,0x00,0xE4,0x10,0x04,0x19,0x04,0x11,0x05,0x19,
    0x47,0x19,0x67,0x11,0xEC,0x5A,0xBE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xDF,0xFF,0xDB,0xD6,0xC4,0x10,0xE4,0x10,0x04,0x11,0x05,0x11,0xA4,0x18,0x01,0xC0,
    0x83,0x88,0xE4,0x00,0x05,0x19,0x04,0x19,0xC5,0x08,0x44,0x21,0x43,0x83,0x23,0xD5,
    0x42,0xFE,0xE4,0xFE,0x27,0xFF,0x07,0xFF,0xA4,0xFE,0x64,0xFE,0x03,0xFE,0xA3,0xFD,
    0xE2,0xFC,0x42,0xEC,0x83,0xB3,0x24,0x62,0xE5,0x10,0xC4,0x08,0x04,0x19,0x26,0x19,
    0xA8,0x19,0x87,0x21,0x00,0x90,0xD3,0xBC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xF0,0x7B,0x42,0x00,0x05,0x19,0x05,0x11,0x83,0x28,0x01,0xD0,
    0x44,0xF8,0xA3,0x48,0xE4,0x00,0xC5,0x08,0x44,0x5A,0x02,0xED,0xE2,0xFD,0x02,0xFE,
    0x66,0xFE,0x74,0xFF,0xB8,0xFF,0x73,0xFF,0xE7,0xF6,0xA6,0xF6,0x45,0xF6,0xA4,0xF5,
    0xC3,0xFC,0x62,0xFC,0xC2,0xFC,0xC2,0xFC,0xE3,0xCB,0xC4,0x49,0x06,0x11,0x88,0x19,
    0x87,0x01,0xA4,0x90,0x01,0xF8,0xEC,0x9A,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0x9E,0xF7,0xE8,0x31,0x83,0x00,0x05,0x09,0x82,0x40,0x01,0xC0,
    0x23,0xF8,0x85,0xF0,0xA3,0x48,0xA4,0x00,0x44,0x5A,0x02,0xFD,0x23,0xCC,0xC2,0xDC,
    0x04,0xFE,0x28,0xFE,0x48,0xF6,0x46,0xF6,0x24,0xF6,0xE4,0xF5,0x64,0xFD,0xE3,0xFC,
    0x62,0xFC,0xC2,0xFC,0x02,0xE4,0x02,0xDC,0xE2,0xFC,0xA4,0x7A,0x48,0x01,0x67,0x01,
    0xC4,0x78,0x24,0xF8,0x02,0xF8,0x84,0xB0,0x7D,0xE7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xDB,0xDE,0x25,0x19,0xA3,0x00,0xC4,0x38,0x02,0xE0,
    0x22,0xD8,0x44,0xF8,0xA6,0xF8,0xA4,0x78,0x63,0x00,0x43,0x21,0x83,0x72,0x83,0x39,
    0x82,0x9B,0x21,0xF5,0x61,0xFD,0x22,0xFD,0xE2,0xFC,0xA2,0xFC,0x42,0xFC,0x42,0xFC,
    0x42,0xFC,0x22,0xAB,0x83,0x41,0xC3,0x92,0x04,0x52,0x26,0x01,0x25,0x19,0xA4,0x98,
    0x44,0xF8,0x23,0xF8,0x02,0xF8,0xA4,0xD0,0x9E,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9A,0xD6,0x87,0x29,0xA5,0x00,0x43,0xB8,
    0x22,0xF8,0x23,0xE0,0x65,0xF8,0xE8,0xF8,0x07,0xC9,0x83,0x48,0x42,0x00,0xA3,0x00,
    0x84,0x00,0x63,0x29,0xA2,0x7A,0x62,0xB3,0xA2,0xCB,0x62,0xD3,0x02,0xBB,0x82,0x8A,
    0x83,0x39,0xA4,0x00,0xE5,0x00,0xE5,0x00,0xE5,0x08,0xC4,0x60,0x64,0xD8,0x44,0xF8,
    0x24,0xF8,0x23,0xF8,0x02,0xF8,0x83,0x88,0xDB,0xC6,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3D,0xE7,0x50,0x5B,0x08,0x31,
    0x23,0xE8,0x43,0xF8,0x44,0xF0,0x65,0xF8,0x09,0xF9,0xAB,0xF9,0x89,0xD1,0x06,0x89,
    0xA3,0x48,0x42,0x18,0x02,0x00,0x42,0x00,0x61,0x00,0x82,0x00,0x62,0x00,0x62,0x00,
    0x83,0x00,0xA3,0x20,0xC4,0x50,0xA5,0x88,0x85,0xD8,0x65,0xF8,0x44,0xF8,0x44,0xF8,
    0x23,0xF8,0x23,0xF8,0x03,0xD0,0x82,0x10,0xC7,0x29,0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x6C,0x32,
    0xA5,0x38,0x02,0xD8,0x23,0xF8,0x65,0xF8,0x66,0xF8,0xA7,0xF8,0x4A,0xF9,0x0C,0xFA,
    0x4D,0xFA,0x4C,0xEA,0x0B,0xD2,0xA9,0xB9,0x68,0xB1,0x47,0xA9,0x27,0xB1,0x07,0xB9,
    0x07,0xD1,0xE7,0xE8,0xC7,0xF8,0xA7,0xF8,0x65,0xF8,0x65,0xF8,0x44,0xF8,0x23,0xF8,
    0x03,0xF8,0x02,0xD0,0xA3,0x28,0x05,0x09,0xC4,0x08,0xEC,0x5A,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFB,0xDE,0x05,0x19,
    0xC4,0x00,0xA7,0x41,0xE6,0xC0,0x03,0xF8,0x86,0xF8,0xA7,0xF8,0x87,0xF8,0x86,0xF8,
    0xC7,0xF8,0x29,0xF9,0x8A,0xF9,0xAB,0xF9,0xAB,0xF9,0x8B,0xF9,0x6A,0xF9,0x29,0xF9,
    0x08,0xF9,0xC7,0xF8,0xA6,0xF8,0x86,0xF8,0x65,0xF8,0x64,0xF8,0x23,0xF8,0x02,0xF0,
    0x06,0xB1,0x25,0x29,0xE4,0x00,0xE4,0x10,0x25,0x19,0x25,0x19,0x14,0x9D,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x96,0xAD,0x62,0x00,
    0x82,0x08,0x35,0x95,0xBA,0xCE,0x8B,0xA2,0x44,0xD0,0x25,0xF8,0x87,0xF8,0xA7,0xF8,
    0xC7,0xF8,0xA7,0xF8,0x87,0xF8,0x86,0xF8,0x86,0xF8,0x86,0xF8,0x87,0xF8,0xA7,0xF8,
    0xA7,0xF8,0xA6,0xF8,0x85,0xF8,0x65,0xF8,0x64,0xF8,0x24,0xF0,0x64,0xB8,0x0D,0x93,
    0xBB,0xB6,0xCF,0x63,0x83,0x08,0x04,0x11,0xE4,0x10,0x66,0x21,0x49,0x3A,0x5D,0xEF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xD3,0x94,0x42,0x00,
    0xE4,0x10,0xBB,0xCE,0xFF,0xFF,0xBE,0xE7,0x76,0xB5,0xCC,0xAA,0x07,0xC1,0x45,0xE0,
    0x45,0xF8,0x46,0xF8,0x66,0xF8,0x86,0xF8,0x86,0xF8,0x86,0xF8,0x86,0xF8,0x65,0xF8,
    0x45,0xF8,0x65,0xF8,0x65,0xE8,0x44,0xD0,0x43,0xA8,0x01,0x88,0x82,0x90,0x3C,0xD7,
    0xFF,0xEF,0x55,0x95,0x83,0x08,0x04,0x11,0x04,0x11,0x05,0x19,0x46,0x19,0xB3,0x94,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xB3,0x94,0x41,0x00,
    0x86,0x21,0x5D,0xDF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xDF,0xE7,0x7A,0xC6,0xD3,0xB4,
    0x4E,0xB3,0x2A,0xC2,0x68,0xD1,0xE6,0xE0,0xA6,0xE8,0xA5,0xE8,0xA5,0xE8,0xE6,0xD8,
    0x88,0xC9,0x06,0xA9,0x22,0xA8,0x02,0xA8,0x00,0xA0,0x00,0xC8,0x00,0xD8,0xF7,0xE5,
    0xFF,0xE7,0xF8,0xAD,0xC4,0x10,0xE4,0x10,0x04,0x11,0xE4,0x10,0x05,0x11,0x8B,0x4A,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x55,0xA5,0x41,0x00,
    0xA7,0x29,0x5D,0xDF,0xFF,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xEF,0x7D,0xDF,0xDB,0xCE,0x59,0xCE,0xF8,0xCD,0xD7,0xCD,0xF7,0xC5,0x79,0xCE,
    0xFB,0xBE,0xAB,0xA2,0x03,0xF0,0x45,0xF8,0x42,0xD0,0x43,0xE8,0x00,0xF0,0x72,0xD4,
    0xFF,0xDF,0x39,0xAE,0xE4,0x10,0xE4,0x10,0x04,0x11,0xE4,0x10,0x05,0x11,0x87,0x29,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x59,0xCE,0x83,0x08,
    0x46,0x21,0x1C,0xD7,0xFF,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xEF,0x8B,0xBA,0x04,0xF8,0x45,0xF8,0x62,0xE0,0x44,0xF0,0x00,0xF8,0x8E,0xDB,
    0xFF,0xDF,0xF8,0xA5,0xC4,0x10,0xE4,0x10,0xE4,0x10,0x04,0x11,0xE4,0x10,0x25,0x19,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBE,0xF7,0x87,0x29,
    0x83,0x08,0x39,0xB6,0xFF,0xF7,0xDF,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xBE,0xE7,0x4A,0xBA,0x03,0xF8,0x45,0xF8,0x64,0xF8,0x44,0xF8,0x00,0xF8,0x6E,0xE3,
    0xFF,0xD7,0xF4,0x8C,0x83,0x08,0x04,0x11,0xE4,0x10,0xE4,0x10,0xE4,0x10,0x05,0x19,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xEF,0x73,
    0x00,0x00,0x72,0x84,0xFF,0xEF,0xBE,0xEF,0xDF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xDF,0xE7,0x8B,0xBA,0x03,0xF8,0x45,0xF8,0x45,0xF8,0x23,0xF8,0x00,0xF8,0xD3,0xD4,
    0xFF,0xD7,0x4E,0x5B,0x21,0x00,0x29,0x3A,0x55,0xA5,0x83,0x08,0xC4,0x10,0x25,0x19,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFB,0xDE,
    0xA3,0x08,0xE8,0x31,0x9E,0xDF,0x9E,0xE7,0xBF,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xF7,0x51,0xBC,0x02,0xE0,0x03,0xF8,0x03,0xF0,0x43,0xE0,0xEC,0xC2,0x7E,0xCF,
    0xFC,0xBE,0x46,0x21,0x21,0x00,0xD3,0x94,0xFF,0xFF,0x51,0x84,0x00,0x00,0x87,0x29,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x51,0x84,0x00,0x00,0xF4,0x8C,0xFF,0xEF,0x9E,0xE7,0xBF,0xEF,0xDF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0x3D,0xDF,0x55,0xBD,0x52,0xBC,0x72,0xBC,0xB7,0xB5,0x5D,0xC7,0xFF,0xDF,
    0xF0,0x6B,0x00,0x00,0x09,0x3A,0xBF,0xF7,0xFF,0xFF,0xFF,0xFF,0x14,0x9D,0x55,0xA5,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xAC,0x4A,0xA4,0x08,0xBB,0xBE,0xDF,0xE7,0x7E,0xE7,0xBE,0xEF,0xDF,0xF7,
    0xDF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xDF,0xFF,0xDF,0xF7,0xFF,0xEF,0xDF,0xDF,0xBF,0xD7,0x9E,0xD7,0xDF,0xDF,0xD8,0xA5,
    0x83,0x08,0x26,0x11,0xDB,0xD6,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x79,0xEE,0x8B,0xDC,0x21,0x31,0xA9,0x21,0x3D,0xCF,0xBF,0xDF,0x7E,0xDF,0x9E,0xE7,
    0xBE,0xEF,0xBF,0xEF,0xDF,0xF7,0xDF,0xF7,0xDF,0xF7,0xDF,0xF7,0xDF,0xF7,0xBF,0xEF,
    0xBE,0xEF,0x9E,0xE7,0x7E,0xDF,0x5E,0xD7,0x5E,0xD7,0xDF,0xDF,0x9A,0xB6,0x26,0x19,
    0x42,0x08,0xED,0xA3,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x74,0xDD,
    0xC0,0xDB,0x00,0xFE,0x42,0xEE,0x02,0x42,0x89,0x21,0x7B,0xB6,0xDF,0xDF,0x7E,0xD7,
    0x7E,0xDF,0x7E,0xDF,0x9E,0xE7,0x9E,0xE7,0x9E,0xE7,0x9E,0xE7,0x9E,0xE7,0x7E,0xDF,
    0x7E,0xDF,0x5D,0xD7,0x5D,0xD7,0x9E,0xDF,0xFF,0xE7,0xF8,0xA5,0x07,0x11,0xE3,0x18,
    0x02,0xC5,0x60,0xFD,0xE6,0xD3,0xDB,0xEE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9E,0xF7,0x84,0xBA,
    0xC1,0xFC,0x42,0xFE,0x82,0xFE,0xA2,0xFE,0x81,0x83,0x45,0x21,0x74,0x74,0x5E,0xC7,
    0xDF,0xDF,0x7E,0xD7,0x5E,0xD7,0x5D,0xD7,0x5E,0xD7,0x5E,0xD7,0x5D,0xD7,0x5D,0xD7,
    0x5E,0xD7,0x9E,0xDF,0xFF,0xE7,0x3D,0xC7,0xF1,0x63,0x84,0x08,0x42,0x52,0x26,0xE6,
    0x29,0xFF,0x86,0xFE,0xE0,0xF3,0x6A,0xC3,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x18,0xDE,0xC1,0xD2,
    0xA2,0xFD,0x22,0xFE,0x42,0xFE,0x42,0xFE,0x62,0xFE,0xE2,0xD4,0x41,0x6A,0x49,0x42,
    0x53,0x74,0x3B,0xA6,0x3E,0xC7,0xBF,0xD7,0xBF,0xD7,0xBF,0xD7,0xBF,0xDF,0xBF,0xD7,
    0x3E,0xC7,0x1A,0xA6,0xF2,0x63,0xA7,0x29,0x82,0x41,0x22,0xB4,0x62,0xFE,0x83,0xFE,
    0xAA,0xFE,0x0F,0xFF,0x67,0xFD,0x63,0xBA,0x3C,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9A,0xE6,0x80,0xD2,
    0x21,0xFD,0xC2,0xFD,0xE2,0xF5,0xC2,0xF5,0x82,0xF5,0x82,0xFD,0x62,0xFD,0x61,0xDC,
    0x21,0x9B,0x84,0x6A,0xE9,0x6A,0x2C,0x63,0xAF,0x63,0x11,0x74,0x6E,0x63,0x2C,0x63,
    0x89,0x5A,0x04,0x52,0x81,0x7A,0xC2,0xCB,0xE2,0xFC,0x62,0xFD,0x82,0xFD,0xC2,0xFD,
    0xC2,0xFD,0xE4,0xFD,0x24,0xFD,0x62,0xCA,0x1C,0xE7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xB1,0xCC,
    0x81,0xD2,0xC0,0xF3,0xC1,0xFC,0x02,0xFD,0x02,0xFD,0xE2,0xFC,0xC2,0xFC,0xC2,0xFC,
    0x81,0xFC,0x80,0xFB,0xC0,0xC9,0xA4,0x81,0x35,0xAD,0x59,0xCE,0x71,0x9C,0x21,0x81,
    0x00,0xDA,0xA1,0xFB,0x82,0xFC,0xA2,0xFC,0x82,0xFC,0xA2,0xFC,0x02,0xFD,0x22,0xFD,
    0xE2,0xFC,0x00,0xFC,0x60,0xDA,0x90,0xCC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x59,0xDE,0x0D,0xC4,0x06,0xCB,0xE4,0xD2,0x03,0xDB,0x03,0xDB,0xE3,0xDA,0xC3,0xD2,
    0xA4,0xC2,0x09,0xB3,0xD2,0xBC,0x9E,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFB,0xE6,
    0x0E,0xB4,0xA6,0xBA,0x83,0xD2,0xE3,0xE2,0x02,0xEB,0x22,0xEB,0x22,0xE3,0x03,0xDB,
    0xE4,0xD2,0x6A,0xC3,0xB6,0xD5,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

#endif