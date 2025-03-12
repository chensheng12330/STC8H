;/*---------------------------------------------------------------------*/
;/* --- STC MCU Limited ------------------------------------------------*/
;/* --- STC 1T Series MCU Demo Programme -------------------------------*/
;/* --- Mobile: (86)13922805190 ----------------------------------------*/
;/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
;/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
;/* --- Web: www.STCAI.com ---------------------------------------------*/
;/* --- BBS: www.STCAIMCU.com ------------------------------------------*/
;/* --- QQ:  800003751 -------------------------------------------------*/
;/* 如果要在程序中使用此代码,请在程序中注明使用了STC的资料及程序        */
;/*---------------------------------------------------------------------*/


;*************  功能说明    **************

; 本例程基于STC8H8K64U为主控芯片的实验箱9进行编写测试，STC8G、STC8H系列芯片可通用参考.

; 程序使用P6口来演示跑马灯，输出低驱动。

; 当用户使用硬件 USB 对 STC8H8K64U 系列进行 ISP 下载时不能调节内部 IRC 的频率，
; 但用户可用选择内部预置的 16 个频率
; （分别是 5.5296M、 6M、 11.0592M、 12M、 18.432M、 20M、 22.1184M、 
; 24M、27M、 30M、 33.1776M、 35M、 36.864M、 40M、 44.2368M 和 48M）。
; 下载时用户只能从频率下拉列表中进行选择其中之一，而不能手动输入其他频率。
; （使用串口下载则可用输入 4M～48M 之间的任意频率）。

; 下载时, 选择时钟 24MHZ (用户可自行修改频率).

;******************************************

$include (../../comm/STC8H.INC)

;*******************************************************************
;*******************************************************************

Fosc_KHZ    EQU 24000   ;24000KHZ

STACK_POIRTER   EQU     0D0H    ;堆栈开始地址

;*******************************************************************
;*******************************************************************
        ORG     0000H               ;reset
        LJMP    F_Main

        ORG     0003H               ;0 INT0 interrupt
        RETI
        LJMP    F_INT0_Interrupt      

        ORG     000BH               ;1  Timer0 interrupt
        LJMP    F_Timer0_Interrupt

        ORG     0013H               ;2  INT1 interrupt
        LJMP    F_INT1_Interrupt      

        ORG     001BH               ;3  Timer1 interrupt
        LJMP    F_Timer1_Interrupt

        ORG     0023H               ;4  UART1 interrupt
        LJMP    F_UART1_Interrupt

        ORG     002BH               ;5  ADC and SPI interrupt
        LJMP    F_ADC_Interrupt

        ORG     0033H               ;6  Low Voltage Detect interrupt
        LJMP    F_LVD_Interrupt

        ORG     003BH               ;7  PCA interrupt
        LJMP    F_PCA_Interrupt

        ORG     0043H               ;8  UART2 interrupt
        LJMP    F_UART2_Interrupt

        ORG     004BH               ;9  SPI interrupt
        LJMP    F_SPI_Interrupt

        ORG     0053H               ;10  INT2 interrupt
        LJMP    F_INT2_Interrupt

        ORG     005BH               ;11  INT3 interrupt
        LJMP    F_INT3_Interrupt

        ORG     0063H               ;12  Timer2 interrupt
        LJMP    F_Timer2_Interrupt

        ORG     0083H               ;16  INT4 interrupt
        LJMP    F_INT4_Interrupt


;******************** 主程序 **************************/
        ORG     0100H       ;reset
F_Main:
    ORL     P_SW2, #080H    ;使能访问XFR

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
    MOV     P6M1, #00H     ;设置为推挽输出
    MOV     P6M0, #0ffH
    MOV     P7M1, #00H     ;设置为准双向口
    MOV     P7M0, #00H

    MOV     SP, #STACK_POIRTER
    MOV     PSW, #0     ;选择第0组R0~R7

    CLR     P4.0        ;LED Power On
L_MainLoop:
    CLR     P6.0
    LCALL   F_delay_ms      ;延时250ms
    SETB    P6.0

    CLR     P6.1
    LCALL   F_delay_ms      ;延时250ms
    SETB    P6.1

    CLR     P6.2
    LCALL   F_delay_ms      ;延时250ms
    SETB    P6.2

    CLR     P6.3
    LCALL   F_delay_ms      ;延时250ms
    SETB    P6.3

    CLR     P6.4
    LCALL   F_delay_ms      ;延时250ms
    SETB    P6.4

    CLR     P6.5
    LCALL   F_delay_ms      ;延时250ms
    SETB    P6.5

    CLR     P6.6
    LCALL   F_delay_ms      ;延时250ms
    SETB    P6.6

    CLR     P6.7
    LCALL   F_delay_ms      ;延时250ms
    SETB    P6.7

    SJMP    L_MainLoop

;*******************************************************************
;*******************************************************************



;========================================================================
; 函数: F_delay_ms
; 描述: 延时子程序。
; 参数: ACC: 延时ms数.
; 返回: none.
; 版本: VER1.0
; 日期: 2013-4-1
; 备注: 除了ACCC和PSW外, 所用到的通用寄存器都入栈
;========================================================================
F_delay_ms:
    PUSH    02H     ;入栈R2
    PUSH    03H     ;入栈R3
    PUSH    04H     ;入栈R4

    MOV     R2,#250

L_delay_ms_1:
    MOV     R3, #HIGH (Fosc_KHZ / 10)
    MOV     R4, #LOW (Fosc_KHZ / 10)
    
L_delay_ms_2:
    MOV     A, R4           ;1T     Total 10T/loop
    DEC     R4              ;1T
    JNZ     L_delay_ms_3    ;3T
    DEC     R3
L_delay_ms_3:
    DEC     A               ;1T
    ORL     A, R3           ;1T
    JNZ     L_delay_ms_2    ;3T
    
    DJNZ    R2, L_delay_ms_1

    POP     04H     ;出栈R2
    POP     03H     ;出栈R3
    POP     02H     ;出栈R4
    RET


;**************** 中断函数 ***************************************************

F_Timer0_Interrupt: ;Timer0 1ms中断函数
    RETI
    
F_Timer1_Interrupt:
    RETI

F_Timer2_Interrupt:
    RETI

F_INT0_Interrupt:
    RETI
    
F_INT1_Interrupt:
    RETI

F_INT2_Interrupt:
    RETI

F_INT3_Interrupt:
    RETI

F_INT4_Interrupt:
    RETI

F_UART1_Interrupt:
    RETI

F_UART2_Interrupt:
    RETI

F_ADC_Interrupt:
    RETI

F_LVD_Interrupt:
    RETI

F_PCA_Interrupt:
    RETI

F_SPI_Interrupt:
    RETI


    END

