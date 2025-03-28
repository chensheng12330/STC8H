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

;串口3全双工中断方式收发通讯程序.

;串口设置为：115200,8,n,1.

;通过PC向MCU发送数据, MCU收到后通过串口把收到的数据原样返回.

;用定时器做波特率发生器，建议使用1T模式(除非低波特率用12T)，并选择可被波特率整除的时钟频率，以提高精度。

;下载时, 选择时钟 22.1184MHz，如要改变, 请修改下面的"定义主时钟"的值并重新编译.

;******************************************/

$include (../../comm/STC8H.INC)

;/****************************** 用户定义宏 ***********************************/

;UART3_Baudrate EQU     (-4608) ;   600bps @ 11.0592MHz
;UART3_Baudrate EQU     (-2304) ;  1200bps @ 11.0592MHz UART3_Baudrate = 65536UL - ((MAIN_Fosc / 4) / Baudrate)
;UART3_Baudrate EQU     (-1152) ;  2400bps @ 11.0592MHz
;UART3_Baudrate EQU     (-576)  ;  4800bps @ 11.0592MHz
;UART3_Baudrate EQU     (-288)  ;  9600bps @ 11.0592MHz
;UART3_Baudrate EQU     (-144)  ; 19200bps @ 11.0592MHz
;UART3_Baudrate EQU     (-72)   ; 38400bps @ 11.0592MHz
;UART3_Baudrate EQU     (-48)   ; 57600bps @ 11.0592MHz
;UART3_Baudrate EQU     (-24)   ;115200bps @ 11.0592MHz

;UART3_Baudrate EQU     (-7680) ;   600bps @ 18.432MHz
;UART3_Baudrate EQU     (-3840) ;  1200bps @ 18.432MHz
;UART3_Baudrate EQU     (-1920) ;  2400bps @ 18.432MHz
;UART3_Baudrate EQU     (-960)  ;  4800bps @ 18.432MHz
;UART3_Baudrate EQU     (-480)  ;  9600bps @ 18.432MHz
;UART3_Baudrate EQU     (-240)  ; 19200bps @ 18.432MHz
;UART3_Baudrate EQU     (-120)  ; 38400bps @ 18.432MHz
;UART3_Baudrate EQU     (-80)   ; 57600bps @ 18.432MHz
;UART3_Baudrate EQU     (-40)   ;115200bps @ 18.432MHz

;UART3_Baudrate EQU     (-9216) ;   600bps @ 22.1184MHz
;UART3_Baudrate EQU     (-4608) ;  1200bps @ 22.1184MHz
;UART3_Baudrate EQU     (-2304) ;  2400bps @ 22.1184MHz
;UART3_Baudrate EQU     (-1152) ;  4800bps @ 22.1184MHz
;UART3_Baudrate EQU     (-576)  ;  9600bps @ 22.1184MHz
;UART3_Baudrate EQU     (-288)  ; 19200bps @ 22.1184MHz
;UART3_Baudrate EQU     (-144)  ; 38400bps @ 22.1184MHz
;UART3_Baudrate EQU     (-96)   ; 57600bps @ 22.1184MHz
UART3_Baudrate  EQU    (-48)   ;115200bps @ 22.1184MHz

;UART3_Baudrate EQU     (-6912) ; 1200bps @ 33.1776MHz
;UART3_Baudrate EQU     (-3456) ; 2400bps @ 33.1776MHz
;UART3_Baudrate EQU     (-1728) ; 4800bps @ 33.1776MHz
;UART3_Baudrate EQU     (-864)  ; 9600bps @ 33.1776MHz
;UART3_Baudrate EQU     (-432)  ;19200bps @ 33.1776MHz
;UART3_Baudrate EQU     (-216)  ;38400bps @ 33.1776MHz
;UART3_Baudrate EQU     (-144)  ;57600bps @ 33.1776MHz
;UART3_Baudrate EQU     (-72)   ;115200bps @ 33.1776MHz


;*******************************************************************
;*******************************************************************

RX3_Lenth   EQU     32      ; 串口接收缓冲长度

B_TX3_Busy  BIT     20H.0   ; 发送忙标志
TX3_Cnt     DATA    30H     ; 发送计数
RX3_Cnt     DATA    31H     ; 接收计数
RX3_Buffer  DATA    40H     ;40 ~ 5FH 接收缓冲

STACK_POIRTER   EQU     0D0H    ;堆栈开始地址

;*******************************************************************
;*******************************************************************
        ORG     0000H       ;reset
        LJMP    F_Main

        ORG     008BH       ;17  UART3 interrupt
        LJMP    F_UART3_Interrupt


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
    LCALL   F_UART3_config  ; 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer3做波特率.
    SETB    EA      ; 允许全局中断
    
    MOV     DPTR, #TestString3  ;Load string address to DPTR
    LCALL   F_SendString3       ;Send string

;=================== 主循环 ==================================
L_MainLoop:
    MOV     A, TX3_Cnt
    CJNE    A, RX3_Cnt, L_ReturnData
    SJMP    L_QuitCheckRx3
L_ReturnData:                   ; 收到过数据
    JB      B_TX3_Busy, L_QuitCheckRx3      ; 发送忙， 退出
    SETB    B_TX3_Busy          ; 标志发送忙
    MOV     A, #RX3_Buffer
    ADD     A, TX3_Cnt
    MOV     R0, A
    MOV     S3BUF, @R0       ; 发一个字节
    INC     TX3_Cnt
    MOV     A, TX3_Cnt
    CJNE    A, #RX3_Lenth, L_QuitCheckRx3
    MOV     TX3_Cnt, #0     ; 避免溢出处理
L_QuitCheckRx3:
    LJMP    L_MainLoop
;===================================================================


TestString3:                    ;Test string
    DB  "STC8H8K64U Uart3 Test !",0DH,0AH,0

;========================================================================
; 函数: F_SendString3
; 描述: 串口3发送字符串函数。
; 参数: DPTR: 字符串首地址.
; 返回: none.
; 版本: VER1.0
; 日期: 2020-11-05
; 备注: 
;========================================================================
F_SendString3:
    CLR     A
    MOVC    A, @A+DPTR      ;Get current char
    JZ      L_SendEnd3      ;Check the end of the string
    SETB    B_TX3_Busy      ;标志发送忙
    MOV     S3BUF, A         ;发送一个字节
    JB      B_TX3_Busy, $   ;等待发送完成
    INC     DPTR            ;increment string ptr
    SJMP    F_SendString3   ;Check next
L_SendEnd3:
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
; 函数: F_UART3_config
; 描述: UART3初始化函数。
; 参数: ACC: 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer3做波特率.
; 返回: none.
; 版本: VER1.0
; 日期: 2020-11-05
; 备注: 
;========================================================================
F_UART3_config:
    CJNE    A, #2, L_Uart1NotUseTimer2
    ORL     S3CON, #0x10     ; S3 BRT Use Timer2;
    MOV     DPTR, #UART3_Baudrate
    LCALL   F_SetTimer2Baudraye
    SJMP    L_SetupUart1

L_Uart1NotUseTimer2:
    MOV     S3CON, #0x50     ; 8位数据, 使用Timer3做波特率发生器, 允许接收
    MOV     T3H, #HIGH UART3_Baudrate
    MOV     T3L, #LOW  UART3_Baudrate
    MOV     T4T3M, #0x0a

L_SetupUart1:
    ANL     P_SW2, #NOT 0x02  ; UART3 switch to P0.0 P0.1
;    ORL     P_SW2, #0x02     ; UART3 switch to P5.0 P5.1
    ORL     IE2, #0x08        ; 允许UART3中断

    CLR     B_TX3_Busy
    MOV     RX3_Cnt, #0;
    MOV     TX3_Cnt, #0;
    RET


;========================================================================
; 函数: F_UART3_Interrupt
; 描述: UART3中断函数。
; 参数: nine.
; 返回: none.
; 版本: VER1.0
; 日期: 2020-11-05
; 备注: 
;========================================================================
F_UART3_Interrupt:
    PUSH    PSW
    PUSH    ACC
    PUSH    AR0
    
    MOV     A, S3CON
    JNB     ACC.0, L_QuitUartRx
    ANL     S3CON, #NOT 1;
    MOV     A, #RX3_Buffer
    ADD     A, RX3_Cnt
    MOV     R0, A
    MOV     @R0, S3BUF   ;保存一个字节
    INC     RX3_Cnt
    MOV     A, RX3_Cnt
    CJNE    A, #RX3_Lenth, L_QuitUartRx
    MOV     RX3_Cnt, #0     ; 避免溢出处理
L_QuitUartRx:

    MOV     A, S3CON
    JNB     ACC.1, L_QuitUartTx
    ANL     S3CON, #NOT 2;
    CLR     B_TX3_Busy      ; 清除发送忙标志
L_QuitUartTx:

    POP     AR0
    POP     ACC
    POP     PSW
    RETI

    END

