;/*---------------------------------------------------------------------*/
;/* --- STC MCU Limited ------------------------------------------------*/
;/* --- STC 1T Series MCU Demo Programme -------------------------------*/
;/* --- Mobile: (86)13922805190 ----------------------------------------*/
;/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
;/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
;/* --- Web: www.STCAI.com ---------------------------------------------*/
;/* --- Web: www.STCMCUDATA.com ----------------------------------------*/
;/* --- BBS: www.STCAIMCU.com ------------------------------------------*/
;/* --- QQ:  800003751 -------------------------------------------------*/
;/* 如果要在程序中使用此代码,请在程序中注明使用了STC的资料及程序        */
;/*---------------------------------------------------------------------*/

;/************* 功能说明    **************

;本例程基于STC8H8K64U为主控芯片的实验箱9进行编写测试，STC8G、STC8H系列芯片可通用参考.

;串口1和串口2全双工中断方式收发通讯程序.

;串口设置为：115200,8,n,1.

;通过PC向MCU发送数据, MCU收到后通过串口把收到的数据原样返回.

;用定时器做波特率发生器，建议使用1T模式(除非低波特率用12T)，并选择可被波特率整除的时钟频率，以提高精度。

;下载时, 选择时钟 22.1184MHz，如要改变, 请修改下面的"定义主时钟"的值并重新编译.

;******************************************/

$include (../../comm/STC8H.INC)

;/****************************** 用户定义宏 ***********************************/

;UART1_Baudrate EQU     (-4608) ;   600bps @ 11.0592MHz
;UART1_Baudrate EQU     (-2304) ;  1200bps @ 11.0592MHz UART1_Baudrate = 65536UL - ((MAIN_Fosc / 4) / Baudrate)
;UART1_Baudrate EQU     (-1152) ;  2400bps @ 11.0592MHz
;UART1_Baudrate EQU     (-576)  ;  4800bps @ 11.0592MHz
;UART1_Baudrate EQU     (-288)  ;  9600bps @ 11.0592MHz
;UART1_Baudrate EQU     (-144)  ; 19200bps @ 11.0592MHz
;UART1_Baudrate EQU     (-72)   ; 38400bps @ 11.0592MHz
;UART1_Baudrate EQU     (-48)   ; 57600bps @ 11.0592MHz
;UART1_Baudrate EQU     (-24)   ;115200bps @ 11.0592MHz

;UART1_Baudrate EQU     (-7680) ;   600bps @ 18.432MHz
;UART1_Baudrate EQU     (-3840) ;  1200bps @ 18.432MHz
;UART1_Baudrate EQU     (-1920) ;  2400bps @ 18.432MHz
;UART1_Baudrate EQU     (-960)  ;  4800bps @ 18.432MHz
;UART1_Baudrate EQU     (-480)  ;  9600bps @ 18.432MHz
;UART1_Baudrate EQU     (-240)  ; 19200bps @ 18.432MHz
;UART1_Baudrate EQU     (-120)  ; 38400bps @ 18.432MHz
;UART1_Baudrate EQU     (-80)   ; 57600bps @ 18.432MHz
;UART1_Baudrate EQU     (-40)   ;115200bps @ 18.432MHz

;UART1_Baudrate EQU     (-9216) ;   600bps @ 22.1184MHz
;UART1_Baudrate EQU     (-4608) ;  1200bps @ 22.1184MHz
;UART1_Baudrate EQU     (-2304) ;  2400bps @ 22.1184MHz
;UART1_Baudrate EQU     (-1152) ;  4800bps @ 22.1184MHz
;UART1_Baudrate EQU     (-576)  ;  9600bps @ 22.1184MHz
;UART1_Baudrate EQU     (-288)  ; 19200bps @ 22.1184MHz
;UART1_Baudrate EQU     (-144)  ; 38400bps @ 22.1184MHz
;UART1_Baudrate EQU     (-96)   ; 57600bps @ 22.1184MHz
UART1_Baudrate  EQU     (-48)   ;115200bps @ 22.1184MHz

;UART1_Baudrate EQU     (-6912) ; 1200bps @ 33.1776MHz
;UART1_Baudrate EQU     (-3456) ; 2400bps @ 33.1776MHz
;UART1_Baudrate EQU     (-1728) ; 4800bps @ 33.1776MHz
;UART1_Baudrate EQU     (-864)  ; 9600bps @ 33.1776MHz
;UART1_Baudrate EQU     (-432)  ;19200bps @ 33.1776MHz
;UART1_Baudrate EQU     (-216)  ;38400bps @ 33.1776MHz
;UART1_Baudrate EQU     (-144)  ;57600bps @ 33.1776MHz
;UART1_Baudrate EQU     (-72)   ;115200bps @ 33.1776MHz

;=======================================================
;UART2_Baudrate EQU     (-4608) ;   600bps @ 11.0592MHz
;UART2_Baudrate EQU     (-2304) ;  1200bps @ 11.0592MHz UART2_Baudrate = 65536UL - ((MAIN_Fosc / 4) / Baudrate)
;UART2_Baudrate EQU     (-1152) ;  2400bps @ 11.0592MHz
;UART2_Baudrate EQU     (-576)  ;  4800bps @ 11.0592MHz
;UART2_Baudrate EQU     (-288)  ;  9600bps @ 11.0592MHz
;UART2_Baudrate EQU     (-144)  ; 19200bps @ 11.0592MHz
;UART2_Baudrate EQU     (-72)   ; 38400bps @ 11.0592MHz
;UART2_Baudrate EQU     (-48)   ; 57600bps @ 11.0592MHz
;UART2_Baudrate EQU     (-24)   ;115200bps @ 11.0592MHz

;UART2_Baudrate EQU     (-7680) ;   600bps @ 18.432MHz
;UART2_Baudrate EQU     (-3840) ;  1200bps @ 18.432MHz
;UART2_Baudrate EQU     (-1920) ;  2400bps @ 18.432MHz
;UART2_Baudrate EQU     (-960)  ;  4800bps @ 18.432MHz
;UART2_Baudrate EQU     (-480)  ;  9600bps @ 18.432MHz
;UART2_Baudrate EQU     (-240)  ; 19200bps @ 18.432MHz
;UART2_Baudrate EQU     (-120)  ; 38400bps @ 18.432MHz
;UART2_Baudrate EQU     (-80)   ; 57600bps @ 18.432MHz
;UART2_Baudrate EQU     (-40)   ;115200bps @ 18.432MHz

;UART2_Baudrate EQU     (-9216) ;   600bps @ 22.1184MHz
;UART2_Baudrate EQU     (-4608) ;  1200bps @ 22.1184MHz
;UART2_Baudrate EQU     (-2304) ;  2400bps @ 22.1184MHz
;UART2_Baudrate EQU     (-1152) ;  4800bps @ 22.1184MHz
;UART2_Baudrate EQU     (-576)  ;  9600bps @ 22.1184MHz
;UART2_Baudrate EQU     (-288)  ; 19200bps @ 22.1184MHz
;UART2_Baudrate EQU     (-144)  ; 38400bps @ 22.1184MHz
;UART2_Baudrate EQU     (-96)   ; 57600bps @ 22.1184MHz
UART2_Baudrate  EQU     (-48)   ;115200bps @ 22.1184MHz

;UART2_Baudrate EQU     (-6912) ; 1200bps @ 33.1776MHz
;UART2_Baudrate EQU     (-3456) ; 2400bps @ 33.1776MHz
;UART2_Baudrate EQU     (-1728) ; 4800bps @ 33.1776MHz
;UART2_Baudrate EQU     (-864)  ; 9600bps @ 33.1776MHz
;UART2_Baudrate EQU     (-432)  ;19200bps @ 33.1776MHz
;UART2_Baudrate EQU     (-216)  ;38400bps @ 33.1776MHz
;UART2_Baudrate EQU     (-144)  ;57600bps @ 33.1776MHz
;UART2_Baudrate EQU     (-72)   ;115200bps @ 33.1776MHz

;*******************************************************************
;*******************************************************************

RX1_Lenth   EQU     32      ; 串口接收缓冲长度
RX2_Lenth   EQU     32      ; 串口接收缓冲长度

B_TX1_Busy  BIT     20H.0   ; 发送忙标志
B_TX2_Busy  BIT     20H.1   ; 发送忙标志
TX1_Cnt     DATA    30H     ; 发送计数
RX1_Cnt     DATA    31H     ; 接收计数
TX2_Cnt     DATA    32H     ; 发送计数
RX2_Cnt     DATA    33H     ; 接收计数

RX1_Buffer  DATA    40H     ;40 ~ 5FH 接收缓冲
RX2_Buffer  DATA    60H     ;60 ~ 7FH 接收缓冲


STACK_POIRTER   EQU     0D0H    ;堆栈开始地址

;*******************************************************************
;*******************************************************************
        ORG     0000H               ;reset
        LJMP    F_Main

        ORG     0023H               ;4  UART1 interrupt
        LJMP    F_UART1_Interrupt

        ORG     0043H               ;8  UART2 interrupt
        LJMP    F_UART2_Interrupt


;******************** 主程序 **************************/
        ORG     0100H       ;reset
F_Main:
    ORL     P_SW2, #080H    ;扩展寄存器(XFR)访问使能

    MOV     P0M1, #30H     ;设置P0.4、P0.5为漏极开路(实验箱加了上拉电阻到3.3V)
    MOV     P0M0, #30H
    MOV     P1M1, #30H     ;设置P1.4、P1.5为漏极开路(实验箱加了上拉电阻到3.3V)
    MOV     P1M0, #30H
    MOV     P2M1, #3cH     ;设置P2.2~P2.5为漏极开路(实验箱加了上拉电阻到3.3V)
    MOV     P2M0, #3cH
    MOV     P3M1, #50H     ;设置P3.4、P3.6为漏极开路(实验箱加了上拉电阻到3.3V)
    MOV     P3M0, #50H
    MOV     P4M1, #3cH     ;设置P4.2~P4.5为漏极开路(实验箱加了上拉电阻到3.3V)
    MOV     P4M0, #3cH
    MOV     P5M1, #0cH     ;设置P5.2、P5.3为漏极开路(实验箱加了上拉电阻到3.3V)
    MOV     P5M0, #0cH
    MOV     P6M1, #0ffH    ;设置为漏极开路(实验箱加了上拉电阻到3.3V)
    MOV     P6M0, #0ffH
    MOV     P7M1, #00H     ;设置为准双向口
    MOV     P7M0, #00H
    
    MOV     SP, #STACK_POIRTER
    MOV     PSW, #0
    USING   0       ;选择第0组R0~R7

;================= 用户初始化程序 ====================================
    MOV     A, #1
    LCALL   F_UART1_config  ; 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
    MOV     A, #2
    LCALL   F_UART2_config  ; 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
    SETB    EA      ; 允许全局中断

    MOV     DPTR, #TestString1  ;Load string address to DPTR
    LCALL   F_SendString1       ;Send string
    MOV     DPTR, #TestString2  ;Load string address to DPTR
    LCALL   F_SendString2       ;Send string

;=================== 主循环 ==================================
L_MainLoop:
    MOV     A, TX1_Cnt
    CJNE    A, RX1_Cnt, L_ReturnData1
    SJMP    L_QuitCheckRx1
L_ReturnData1:                  ; 收到过数据
    JB      B_TX1_Busy, L_QuitCheckRx1      ; 发送忙， 退出
    SETB    B_TX1_Busy          ; 标志发送忙
    MOV     A, #RX1_Buffer
    ADD     A, TX1_Cnt
    MOV     R0, A
    MOV     SBUF, @R0       ; 发一个字节
    INC     TX1_Cnt
    MOV     A, TX1_Cnt
    CJNE    A, #RX1_Lenth, L_QuitCheckRx1
    MOV     TX1_Cnt, #0     ; 避免溢出处理
L_QuitCheckRx1:

    MOV     A, TX2_Cnt
    CJNE    A, RX2_Cnt, L_ReturnData2
    SJMP    L_QuitCheckRx2
L_ReturnData2:                  ; 收到过数据
    JB      B_TX2_Busy, L_QuitCheckRx2      ; 发送忙， 退出
    SETB    B_TX2_Busy          ; 标志发送忙
    MOV     A, #RX2_Buffer
    ADD     A, TX2_Cnt
    MOV     R0, A
    MOV     S2BUF, @R0      ; 发一个字节
    INC     TX2_Cnt
    MOV     A, TX2_Cnt
    CJNE    A, #RX2_Lenth, L_QuitCheckRx2
    MOV     TX2_Cnt, #0     ; 避免溢出处理
L_QuitCheckRx2:

    LJMP    L_MainLoop
;===================================================================

TestString1:                    ;Test string
    DB  "STC8H8K64U Uart1 Test !",0DH,0AH,0
TestString2:                    ;Test string
    DB  "STC8H8K64U Uart2 Test !",0DH,0AH,0

;========================================================================
; 函数: F_SendString1
; 描述: 串口1发送字符串函数。
; 参数: DPTR: 字符串首地址.
; 返回: none.
; 版本: VER1.0
; 日期: 2014-11-28
; 备注: 
;========================================================================
F_SendString1:
    CLR     A
    MOVC    A, @A+DPTR      ;Get current char
    JZ      L_SendEnd1      ;Check the end of the string
    SETB    B_TX1_Busy      ;标志发送忙
    MOV     SBUF, A         ;发送一个字节
    JB      B_TX1_Busy, $   ;等待发送完成
    INC     DPTR            ;increment string ptr
    SJMP    F_SendString1       ;Check next
L_SendEnd1:
    RET

;========================================================================
; 函数: F_SendString2
; 描述: 串口2发送字符串函数。
; 参数: DPTR: 字符串首地址.
; 返回: none.
; 版本: VER1.0
; 日期: 2014-11-28
; 备注: 
;========================================================================
F_SendString2:
    CLR     A
    MOVC    A, @A+DPTR      ;Get current char
    JZ      L_SendEnd2      ;Check the end of the string
    SETB    B_TX2_Busy      ;标志发送忙
    MOV     S2BUF, A        ;发送一个字节
    JB      B_TX2_Busy, $   ;等待发送完成
    INC     DPTR            ;increment string ptr
    SJMP    F_SendString2       ;Check next
L_SendEnd2:
    RET

;========================================================================
; 函数: F_SetTimer2Baudraye
; 描述: 设置Timer2做波特率发生器。
; 参数: DPTR: Timer2的重装值.
; 返回: none.
; 版本: VER1.0
; 日期: 2014-11-28
; 备注: 
;========================================================================
F_SetTimer2Baudraye:    ; 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
    ANL     AUXR, #NOT (1 SHL 4)    ; Timer stop    波特率使用Timer2产生
    ANL     AUXR, #NOT (1 SHL 3)    ; Timer2 set As Timer
    ORL     AUXR, #(1 SHL 2)        ; Timer2 set as 1T mode
    MOV     T2H, DPH
    MOV     T2L, DPL
    ANL     IE2, #NOT (1 SHL 2)     ; 禁止中断
    ORL     AUXR, #(1 SHL 4)        ; Timer run enable
    RET

;========================================================================
; 函数: F_UART1_config
; 描述: UART1初始化函数。
; 参数: ACC: 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
; 返回: none.
; 版本: VER1.0
; 日期: 2014-11-28
; 备注: 
;========================================================================
F_UART1_config:
    CJNE    A, #2, L_Uart1NotUseTimer2
    ORL     AUXR, #0x01     ; S1 BRT Use Timer2;
    MOV     DPTR, #UART1_Baudrate
    LCALL   F_SetTimer2Baudraye
    SJMP    L_SetupUart1

L_Uart1NotUseTimer2:
    CLR     TR1                 ; Timer Stop    波特率使用Timer1产生
    ANL     AUXR, #NOT 0x01     ; S1 BRT Use Timer1;
    ORL     AUXR, #(1 SHL 6)    ; Timer1 set as 1T mode
    ANL     TMOD, #NOT (1 SHL 6); Timer1 set As Timer
    ANL     TMOD, #NOT 0x30     ; Timer1_16bitAutoReload;
    MOV     TH1, #HIGH UART1_Baudrate
    MOV     TL1, #LOW  UART1_Baudrate
    CLR     ET1                 ; 禁止中断
    ANL     INTCLKO, #NOT 0x02  ; 不输出时钟
    SETB    TR1

L_SetupUart1:
    ANL     SCON, #0x3f
    ORL     SCON, #0x40     ; UART1模式, 0x00: 同步移位输出, 0x40: 8位数据,可变波特率, 0x80: 9位数据,固定波特率, 0xc0: 9位数据,可变波特率
;   SETB    PS      ; 高优先级中断
    SETB    REN     ; 允许接收
    SETB    ES      ; 允许中断

    ANL     P_SW1, #0x3f
    ORL     P_SW1, #0x00        ; UART1 switch to, 0x00: P3.0 P3.1, 0x40: P3.6 P3.7, 0x80: P1.6 P1.7, 0xC0: P4.3 P4.4
;   ORL     PCON2, #(1 SHL 4)   ; 内部短路RXD与TXD, 做中继, ENABLE,DISABLE

    CLR     B_TX1_Busy
    MOV     RX1_Cnt, #0;
    MOV     TX1_Cnt, #0;
    RET

;========================================================================
; 函数: F_UART2_config
; 描述: UART2初始化函数。
; 参数: ACC: 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
; 返回: none.
; 版本: VER1.0
; 日期: 2014-11-28
; 备注: 
;========================================================================
F_UART2_config:
    CJNE    A, #2, L_Uart2NotUseTimer2
    MOV     DPTR, #UART2_Baudrate
    LCALL   F_SetTimer2Baudraye

    ANL     S2CON, #NOT (1 SHL 7)   ; 8位数据, 1位起始位, 1位停止位, 无校验
    ORL     IE2,   #0X01            ; 允许中断
    ORL     S2CON, #(1 SHL 4)   ; 允许接收
;   ANL     P_SW2, #NOT 0x01    ; UART2 switch to P1.0 P1.1
    ORL     P_SW2, #0x01        ; UART2 switch to P4.6 P4.7
    CLR     B_TX2_Busy
    MOV     RX2_Cnt, #0;
    MOV     TX2_Cnt, #0;
L_Uart2NotUseTimer2:
    RET

;========================================================================
; 函数: F_UART1_Interrupt
; 描述: UART1中断函数。
; 参数: nine.
; 返回: none.
; 版本: VER1.0
; 日期: 2014-11-28
; 备注: 
;========================================================================
F_UART1_Interrupt:
    PUSH    PSW
    PUSH    ACC
    PUSH    AR0
    
    JNB     RI, L_QuitUartRx1
    CLR     RI
    MOV     A, #RX1_Buffer
    ADD     A, RX1_Cnt
    MOV     R0, A
    MOV     @R0, SBUF   ;保存一个字节
    INC     RX1_Cnt
    MOV     A, RX1_Cnt
    CJNE    A, #RX1_Lenth, L_QuitUartRx1
    MOV     RX1_Cnt, #0     ; 避免溢出处理
L_QuitUartRx1:

    JNB     TI, L_QuitUartTx1
    CLR     TI
    CLR     B_TX1_Busy      ; 清除发送忙标志
L_QuitUartTx1:

    POP     AR0
    POP     ACC
    POP     PSW
    RETI

;========================================================================
; 函数: F_UART2_Interrupt
; 描述: UART2中断函数。
; 参数: nine.
; 返回: none.
; 版本: VER1.0
; 日期: 2014-11-28
; 备注: 
;========================================================================
F_UART2_Interrupt:
    PUSH    PSW
    PUSH    ACC
    PUSH    AR0
    
    MOV     A, S2CON
    JNB     ACC.0, L_QuitUartRx2
    ANL     S2CON, #NOT 1;
    MOV     A, #RX2_Buffer
    ADD     A, RX2_Cnt
    MOV     R0, A
    MOV     @R0, S2BUF  ;保存一个字节
    INC     RX2_Cnt
    MOV     A, RX2_Cnt
    CJNE    A, #RX2_Lenth, L_QuitUartRx2
    MOV     RX2_Cnt, #0     ; 避免溢出处理
L_QuitUartRx2:

    MOV     A, S2CON
    JNB     ACC.1, L_QuitUartTx2
    ANL     S2CON, #NOT 2;
    CLR     B_TX2_Busy      ; 清除发送忙标志
L_QuitUartTx2:

    POP     AR0
    POP     ACC
    POP     PSW
    RETI

    END

