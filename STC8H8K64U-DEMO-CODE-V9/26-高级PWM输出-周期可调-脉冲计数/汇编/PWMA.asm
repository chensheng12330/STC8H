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

;本例程基于STC8H8K64U为主控芯片的实验箱9进行编写测试，STC8H系列芯片可通用参考.

;高级PWM定时器实现高速PWM脉冲输出.

;周期/占空比可调, 通过比较/捕获中断进行脉冲个数计数.

;通过P6口演示输出,每隔10ms输出一次PWM,计数10个脉冲后停止输出.

;定时器每1ms调整PWM周期.

;下载时, 选择时钟 24MHZ (用户可自行修改频率).

;******************************************/

$include (../../comm/STC8H.INC)

;/****************************** 用户定义宏 ***********************************/

Fosc_KHZ    EQU 24000   ;24000KHZ

STACK_POIRTER   EQU     0D0H    ;堆栈开始地址

Timer0_Reload   EQU     (65536 - Fosc_KHZ)  ; Timer 0 中断频率, 1000次/秒

;*******************************************************************
;*******************************************************************


;*******************************************************************
;*******************************************************************

;*************  本地变量声明    **************/
PWM1_Flag       BIT     20H.0
B_1ms           BIT     20H.1
;PWM2_Flag       BIT     20H.1
;PWM3_Flag       BIT     20H.2
;PWM4_Flag       BIT     20H.3

Period_H         DATA    30H
Period_L         DATA    31H

msecond          DATA    32H
Counter          DATA    33H

;*******************************************************************
;*******************************************************************
        ORG     0000H               ;reset
        LJMP    F_Main

        ORG     000BH               ;1  Timer0 interrupt
        LJMP    F_Timer0_Interrupt

        ORG     00D3H               ;26  PWMA interrupt
        LJMP    F_PWMA_Interrupt

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
    CLR     PWM1_Flag
    MOV     Period_L, #LOW 01000H
    MOV     Period_H, #HIGH 01000H

    CLR     TR0
    ORL     AUXR, #(1 SHL 7)    ; Timer0_1T();
    ANL     TMOD, #NOT 04H      ; Timer0_AsTimer();
    ANL     TMOD, #NOT 03H      ; Timer0_16bitAutoReload();
    MOV     TH0, #Timer0_Reload / 256   ;Timer0_Load(Timer0_Reload);
    MOV     TL0, #Timer0_Reload MOD 256
    SETB    ET0         ; Timer0_InterruptEnable();
    SETB    TR0         ; Timer0_Run();
    SETB    EA          ; 打开总中断
    
    LCALL   F_PWM_Init          ; PWM初始化
    LCALL   F_UpdatePwm
    CLR     P4.0

;=================== 主循环 ==================================
L_Main_Loop:
    JNB     B_1ms,  L_Main_Loop     ;1ms未到
    CLR     B_1ms

    INC     msecond       ;msecond + 1
    CLR     C
    MOV     A, msecond    ;msecond - 10
    SUBB    A, #10
    JC      L_Main_Loop     ;if(msecond < 10), jmp
    MOV     msecond, #0   ;if(msecond >= 10)

    LCALL   F_TxPulse
    LJMP    L_Main_Loop

;========================================================================
; 函数: F_TxPulse
; 描述: 发送脉冲函数.
; 参数: none.
; 返回: none.
; 版本: V1.0, 2013-11-22
;========================================================================
F_TxPulse:
    MOV     A,#00                 ;写 CCMRx 前必须先清零 CCxE 关闭通道
    MOV     DPTR,#PWMA_CCER1
    MOVX    @DPTR,A
    MOV     A,#060H               ;设置 PWM1 模式1 输出
    MOV     DPTR,#PWMA_CCMR1
    MOVX    @DPTR,A
    MOV     A,#01H                ;使能 CC1E 通道, 高电平有效
    MOV     DPTR,#PWMA_CCER1
    MOVX    @DPTR,A
    MOV     A,#068H               ;开启PWMA_CCRx预转载功能(需要CCxE=1才可写)
    MOV     DPTR,#PWMA_CCMR1
    MOVX    @DPTR,A

    MOV     A,#00                 ;清标志位
    MOV     DPTR,#PWMA_SR1
    MOVX    @DPTR,A
    MOV     A,#00                 ;清计数器
    MOV     DPTR,#PWMA_CNTRH
    MOVX    @DPTR,A
    MOV     A,#00                 ;清计数器
    MOV     DPTR,#PWMA_CNTRL
    MOVX    @DPTR,A
    MOV     A,#02H                ;使能捕获/比较 1 中断
    MOV     DPTR,#PWMA_IER
    MOVX    @DPTR,A
    RET

;========================================================================
; 函数: F_PWM_Init
; 描述: PWM初始化程序.
; 参数: none
; 返回: none.
; 版本: V1.0, 2021-3-3
;========================================================================
F_PWM_Init:
    MOV     A,#01H              ;使能 PWM1 输出
    MOV     DPTR,#PWMA_ENO
    MOVX    @DPTR,A
    MOV     A,#02H              ;高级 PWM 通道输出脚选择位, P6
    MOV     DPTR,#PWMA_PS
    MOVX    @DPTR,A
    MOV     A,#080H             ;使能主输出
    MOV     DPTR,#PWMA_BKR
    MOVX    @DPTR,A

    MOV     DPTR,#PWMA_CR1
    MOVX    A,@DPTR
    ORL     A,#081H             ;使能ARR预装载，开始计时
    MOVX    @DPTR,A
    RET

;========================================================================
; 函数: F_UpdatePwm
; 描述: PWM周期占空比.
; 参数: [Period_H Period_H]: pwm周期值.
; 返回: none.
; 版本: V1.0, 2021-8-24
;========================================================================
F_UpdatePwm:
    MOV     A, Period_H         ;设置周期时间
    MOV     DPTR, #PWMA_ARRH
    MOVX    @DPTR, A
    MOV     A, Period_L
    MOV     DPTR, #PWMA_ARRL
    MOVX    @DPTR, A

    MOV     A, Period_H         ;设置占空比时间: Period/2
	CLR     C
	RRC     A
    MOV     DPTR, #PWMA_CCR1H
    MOVX    @DPTR, A
    MOV     A, Period_L
	RRC     A
    MOV     DPTR, #PWMA_CCR1L
    MOVX    @DPTR, A
    RET

;**************** 中断函数 ***************************************************
F_Timer0_Interrupt: ;Timer0 1ms中断函数
    PUSH    PSW     ;PSW入栈
    PUSH    ACC     ;ACC入栈

    SETB    B_1ms           ; 1ms标志
    JNB     PWM1_Flag, T0_PWM1_SUB
    INC     Period_L       ;Period + 1
    MOV     A, Period_L
    JNZ     $+4
    INC     Period_H
    
    CLR     C
    MOV     A, Period_L
    SUBB    A, #LOW 01000H
    MOV     A, Period_H
    SUBB    A, #HIGH 01000H
    JC      F_QuitTimer0
    CLR     PWM1_Flag
    SJMP    F_QuitTimer0
T0_PWM1_SUB:
    MOV     A, Period_L
    DEC     Period_L       ;Period - 1
    JNZ     $+4
    DEC     Period_H

    CLR     C
    MOV     A, Period_L
    SUBB    A, #LOW 0100H
    MOV     A, Period_H
    SUBB    A, #HIGH 0100H
    JNC     F_QuitTimer0
    SETB    PWM1_Flag

F_QuitTimer0:
    LCALL   F_UpdatePwm

    POP     ACC     ;ACC出栈
    POP     PSW     ;PSW出栈
    RETI

;========================================================================
; 函数: F_PWMA_Interrupt
; 描述: PWMA中断处理程序.
; 参数: None
; 返回: none.
; 版本: V1.0, 2021-08-23
;========================================================================
F_PWMA_Interrupt:
    PUSH    PSW
    PUSH    ACC

    MOV     DPTR,#PWMA_SR1        ;检测从机状态
    MOVX    A,@DPTR
    JNB     ACC.1,F_PWMA_QuitInt
    CLR     A
    MOVX    @DPTR,A

    INC     Counter               ;Counter + 1
    CLR     C
    MOV     A, Counter            ;Counter - 10
    SUBB    A, #10
    JC      F_PWMA_QuitInt        ;if(Counter < 10), jmp
    MOV     Counter, #0           ;if(Counter >= 10)

    MOV     A,#00                 ;写 CCMRx 前必须先清零 CCxE 关闭通道
    MOV     DPTR,#PWMA_CCER1
    MOVX    @DPTR,A
    MOV     A,#040H               ;设置 PWM1 强制为无效电平
    MOV     DPTR,#PWMA_CCMR1
    MOVX    @DPTR,A
    MOV     A,#01H                ;使能 CC1E 通道, 高电平有效
    MOV     DPTR,#PWMA_CCER1
    MOVX    @DPTR,A
    MOV     A,#00H                ;关闭中断
    MOV     DPTR,#PWMA_IER
    MOVX    @DPTR,A

F_PWMA_QuitInt:
    POP     ACC
    POP     PSW
    RETI

;======================================================================

    END

