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

;程序使用P6口做跑马灯，演示系统时钟源切换效果。

;跑马灯每跑一轮切换一次时钟源，分别是默认IRC主频，主频48分频，内部32K IRC。

;下载时, 选择时钟 24MHZ (用户可自行修改频率).

;******************************************/

$include (../../comm/STC8H.INC)

;/****************************** 用户定义宏 ***********************************/

Fosc_KHZ    EQU 24000   ;24000KHZ

STACK_POIRTER   EQU     0D0H    ;堆栈开始地址

;*******************************************************************
;*******************************************************************

;*******************************************************************

Mode        DATA    21H

;*******************************************************************
            ORG     0000H
            LJMP    MAIN

            ORG     0100H
MAIN:
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
    MOV     PSW, #0     ;选择第0组R0~R7

    CLR     P4.0        ;LED Power On
    MOV     Mode, #0
    MOV     R0, #0XFE
L_MainLoop:
    MOV     P6,R0
    
    MOV     A,R0
	RL      A
    MOV     R0,A

    JB      ACC.0,L_MainDelay
    LCALL   F_MCLK_Sel

L_MainDelay:
    MOV     A, #10
    LCALL   F_delay_ms      ;延时10ms
    SJMP    L_MainLoop

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

    MOV     R2,A

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

;========================================================================
; 函数: F_MCLK_Sel
; 描述: 主频设置程序。
; 参数: none.
; 返回: none.
; 版本: VER1.0
; 日期: 2013-4-1
;========================================================================
F_MCLK_Sel:
    MOV     A,Mode
	JNZ     F_MCLK_1
F_MCLK_0:
    MOV     A,#080H                    ;启动内部 IRC
    MOV     DPTR,#HIRCCR
    MOVX    @DPTR,A
    MOVX    A,@DPTR
    JNB     ACC.0,$-1                  ;等待时钟稳定

    MOV     A,#00H                     ;时钟不分频
    MOV     DPTR,#CLKDIV
    MOVX    @DPTR,A
    MOV     A,#00H                     ;选择内部IRC(默认)
    MOV     DPTR,#CLKSEL
    MOVX    @DPTR,A
	INC     Mode
    RET

F_MCLK_1:
    MOV     A,Mode
    XRL     A,#1
	JNZ     F_MCLK_2
    MOV     A,#080H                    ;启动内部 IRC
    MOV     DPTR,#HIRCCR
    MOVX    @DPTR,A
    MOVX    A,@DPTR
    JNB     ACC.0,$-1                  ;等待时钟稳定

    MOV     A,#48                      ;时钟48分频
    MOV     DPTR,#CLKDIV
    MOVX    @DPTR,A
    MOV     A,#00H                     ;选择内部IRC(默认)
    MOV     DPTR,#CLKSEL
    MOVX    @DPTR,A
	INC     Mode
    RET

F_MCLK_2:
    MOV     A,Mode
    XRL     A,#2
	JNZ     F_MCLK_CLR
    MOV     A,#080H                    ;启动内部 32K IRC
    MOV     DPTR,#IRC32KCR
    MOVX    @DPTR,A
    MOVX    A,@DPTR
    JNB     ACC.0,$-1                  ;等待时钟稳定

    MOV     A,#00H                     ;时钟不分频
    MOV     DPTR,#CLKDIV
    MOVX    @DPTR,A
    MOV     A,#03H                     ;选择内部 32K
    MOV     DPTR,#CLKSEL
    MOVX    @DPTR,A
F_MCLK_CLR:
    MOV     Mode, #0
    RET

;F_MCLK_3:
    ;MOV     A,Mode
    ;XRL     A,#3
	;JNZ     F_MCLK_CLR
    ;MOV     A,#0C0H                    ;启动外部晶振
    ;MOV     DPTR,#XOSCCR
    ;MOVX    @DPTR,A
    ;MOVX    A,@DPTR
    ;JNB     ACC.0,$-1                  ;等待时钟稳定

    ;MOV     A,#00H                     ;时钟不分频
    ;MOV     DPTR,#CLKDIV
    ;MOVX    @DPTR,A
    ;MOV     A,#01H                     ;选择外部晶振
    ;MOV     DPTR,#CLKSEL
    ;MOVX    @DPTR,A
    ;MOV     Mode, #0
    ;RET

    END

