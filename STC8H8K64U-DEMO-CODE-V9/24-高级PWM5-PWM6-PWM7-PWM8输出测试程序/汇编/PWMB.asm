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

;高级PWM定时器 PWM5,PWM6,PWM7,PWM8 每个通道都可独立实现PWM输出.

;4个通道PWM根据需要设置对应输出口，可通过示波器观察输出的信号.

;PWM周期和占空比可以根据需要自行设置，最高可达65535.

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
PWM5_Flag       BIT     20H.0
PWM6_Flag       BIT     20H.1
PWM7_Flag       BIT     20H.2
PWM8_Flag       BIT     20H.3

PWM5_Duty_H     DATA    30H
PWM5_Duty_L     DATA    31H
PWM6_Duty_H     DATA    32H
PWM6_Duty_L     DATA    33H
PWM7_Duty_H     DATA    34H
PWM7_Duty_L     DATA    35H
PWM8_Duty_H     DATA    36H
PWM8_Duty_L     DATA    37H

;*******************************************************************
;*******************************************************************
        ORG     0000H               ;reset
        LJMP    F_Main

        ORG     000BH               ;1  Timer0 interrupt
        LJMP    F_Timer0_Interrupt

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
    CLR     PWM5_Flag
    CLR     PWM6_Flag
    CLR     PWM7_Flag
    CLR     PWM8_Flag
    MOV     PWM5_Duty_L, #0
    MOV     PWM5_Duty_H, #0
    MOV     PWM6_Duty_L, #LOW 256
    MOV     PWM6_Duty_H, #HIGH 256
    MOV     PWM7_Duty_L, #LOW 512
    MOV     PWM7_Duty_H, #HIGH 512
    MOV     PWM8_Duty_L, #LOW 1024
    MOV     PWM8_Duty_H, #HIGH 1024

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

;=================== 主循环 ==================================
L_Main_Loop:

    LJMP    L_Main_Loop

;========================================================================
; 函数: F_PWM_Init
; 描述: PWM初始化程序.
; 参数: none
; 返回: none.
; 版本: V1.0, 2021-3-3
;========================================================================
F_PWM_Init:
    CLR     A                   ;写 CCMRx 前必须先清零 CCxE 关闭通道
    MOV     DPTR,#PWMB_CCER1
    MOVX    @DPTR,A
    MOV     DPTR,#PWMB_CCER2
    MOVX    @DPTR,A
    MOV     A,#060H             ;设置 PWMx 模式1 输出
    MOV     DPTR,#PWMB_CCMR1
    MOVX    @DPTR,A
    MOV     DPTR,#PWMB_CCMR2
    MOVX    @DPTR,A
    MOV     DPTR,#PWMB_CCMR3
    MOVX    @DPTR,A
    MOV     DPTR,#PWMB_CCMR4
    MOVX    @DPTR,A
    MOV     A,#033H             ;配置通道输出使能和极性
    MOV     DPTR,#PWMB_CCER1
    MOVX    @DPTR,A
    MOV     DPTR,#PWMB_CCER2
    MOVX    @DPTR,A

    MOV     A,#068H             ;开启PWMB_CCRx预装载功能(需要CCxE=1才可写)
    MOV     DPTR,#PWMB_CCMR1
    MOVX    @DPTR,A
    MOV     DPTR,#PWMB_CCMR2
    MOVX    @DPTR,A
    MOV     DPTR,#PWMB_CCMR3
    MOVX    @DPTR,A
    MOV     DPTR,#PWMB_CCMR4
    MOVX    @DPTR,A

    MOV     A,#3                ;设置周期时间
    MOV     DPTR,#PWMB_ARRH
    MOVX    @DPTR,A
    MOV     A,#0FFH
    MOV     DPTR,#PWMB_ARRL
    MOVX    @DPTR,A

    MOV     A,#055H             ;使能 PWM5~8 输出
    MOV     DPTR,#PWMB_ENO
    MOVX    @DPTR,A
    MOV     A,#00H              ;高级 PWM 通道输出脚选择位, P2
    MOV     DPTR,#PWMB_PS
    MOVX    @DPTR,A
    MOV     A,#080H             ;使能主输出
    MOV     DPTR,#PWMB_BKR
    MOVX    @DPTR,A

    MOV     DPTR,#PWMB_CR1
    MOVX    A,@DPTR
    ORL     A,#01H              ;开始计时
    MOVX    @DPTR,A
    RET

;========================================================================
; 函数: F_UpdatePwm
; 描述: 更新PWM占空比值. 
; 参数: [PWMn_Duty_H PWMn_Duty_L]: pwm占空比值.
; 返回: none.
; 版本: V1.0, 2021-3-3
;========================================================================
F_UpdatePwm:
    MOV     A, PWM5_Duty_H      ;设置占空比时间
    MOV     DPTR, #PWMB_CCR5H
    MOVX    @DPTR, A
    MOV     A, PWM5_Duty_L
    MOV     DPTR, #PWMB_CCR5L
    MOVX    @DPTR, A

    MOV     A, PWM6_Duty_H      ;设置占空比时间
    MOV     DPTR, #PWMB_CCR6H
    MOVX    @DPTR, A
    MOV     A, PWM6_Duty_L
    MOV     DPTR, #PWMB_CCR6L
    MOVX    @DPTR, A

    MOV     A, PWM7_Duty_H      ;设置占空比时间
    MOV     DPTR, #PWMB_CCR7H
    MOVX    @DPTR, A
    MOV     A, PWM7_Duty_L
    MOV     DPTR, #PWMB_CCR7L
    MOVX    @DPTR, A

    MOV     A, PWM8_Duty_H      ;设置占空比时间
    MOV     DPTR, #PWMB_CCR8H
    MOVX    @DPTR, A
    MOV     A, PWM8_Duty_L
    MOV     DPTR, #PWMB_CCR8L
    MOVX    @DPTR, A
    RET

;**************** 中断函数 ***************************************************
F_Timer0_Interrupt: ;Timer0 1ms中断函数
    PUSH    PSW     ;PSW入栈
    PUSH    ACC     ;ACC入栈

    JB      PWM5_Flag, T0_PWM5_SUB
    INC     PWM5_Duty_L       ;PWM5_Duty + 1
    MOV     A, PWM5_Duty_L
    JNZ     $+4
    INC     PWM5_Duty_H
    
    CLR     C
    MOV     A, PWM5_Duty_L
    SUBB    A, #LOW 1023
    MOV     A, PWM5_Duty_H
    SUBB    A, #HIGH 1023
    JC      T0_PWM6_Start
    SETB    PWM5_Flag
    SJMP    T0_PWM6_Start
T0_PWM5_SUB:
    MOV     A, PWM5_Duty_L
    DEC     PWM5_Duty_L       ;PWM5_Duty - 1
    JNZ     $+4
    DEC     PWM5_Duty_H
    
    MOV     A, PWM5_Duty_L
    JNZ     T0_PWM6_Start
    MOV     A, PWM5_Duty_H
    JNZ     T0_PWM6_Start
    CLR     PWM5_Flag

T0_PWM6_Start:
    JB      PWM6_Flag, T0_PWM6_SUB
    INC     PWM6_Duty_L       ;PWM6_Duty + 1
    MOV     A, PWM6_Duty_L
    JNZ     $+4
    INC     PWM6_Duty_H
    
    CLR     C
    MOV     A, PWM6_Duty_L
    SUBB    A, #LOW 1023
    MOV     A, PWM6_Duty_H
    SUBB    A, #HIGH 1023
    JC      T0_PWM7_Start
    SETB    PWM6_Flag
    SJMP    T0_PWM7_Start
T0_PWM6_SUB:
    MOV     A, PWM6_Duty_L
    DEC     PWM6_Duty_L       ;PWM6_Duty - 1
    JNZ     $+4
    DEC     PWM6_Duty_H
    
    MOV     A, PWM6_Duty_L
    JNZ     T0_PWM7_Start
    MOV     A, PWM6_Duty_H
    JNZ     T0_PWM7_Start
    CLR     PWM6_Flag
    
T0_PWM7_Start:
    JB      PWM7_Flag, T0_PWM7_SUB
    INC     PWM7_Duty_L       ;PWM7_Duty + 1
    MOV     A, PWM7_Duty_L
    JNZ     $+4
    INC     PWM7_Duty_H
    
    CLR     C
    MOV     A, PWM7_Duty_L
    SUBB    A, #LOW 1023
    MOV     A, PWM7_Duty_H
    SUBB    A, #HIGH 1023
    JC      T0_PWM8_Start
    SETB    PWM7_Flag
    SJMP    T0_PWM8_Start
T0_PWM7_SUB:
    MOV     A, PWM7_Duty_L
    DEC     PWM7_Duty_L       ;PWM7_Duty - 1
    JNZ     $+4
    DEC     PWM7_Duty_H
    
    MOV     A, PWM7_Duty_L
    JNZ     T0_PWM8_Start
    MOV     A, PWM7_Duty_H
    JNZ     T0_PWM8_Start
    CLR     PWM7_Flag
    
T0_PWM8_Start:
    JB      PWM8_Flag, T0_PWM8_SUB
    INC     PWM8_Duty_L       ;PWM8_Duty + 1
    MOV     A, PWM8_Duty_L
    JNZ     $+4
    INC     PWM8_Duty_H
    
    CLR     C
    MOV     A, PWM8_Duty_L
    SUBB    A, #LOW 1023
    MOV     A, PWM8_Duty_H
    SUBB    A, #HIGH 1023
    JC      F_QuitTimer0
    SETB    PWM8_Flag
    SJMP    F_QuitTimer0
T0_PWM8_SUB:
    MOV     A, PWM8_Duty_L
    DEC     PWM8_Duty_L       ;PWM8_Duty - 1
    JNZ     $+4
    DEC     PWM8_Duty_H
    
    MOV     A, PWM8_Duty_L
    JNZ     F_QuitTimer0
    MOV     A, PWM8_Duty_H
    JNZ     F_QuitTimer0
    CLR     PWM8_Flag
        
F_QuitTimer0:
    LCALL   F_UpdatePwm

    POP     ACC     ;ACC出栈
    POP     PSW     ;PSW出栈
    RETI

;======================================================================

    END

