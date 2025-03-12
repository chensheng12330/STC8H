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

;高级PWM定时器 PWM1P/PWM1N,PWM2P/PWM2N,PWM3P/PWM3N,PWM4P/PWM4N 每个通道都可独立实现PWM输出，或者两两互补对称输出.

;8个通道PWM设置对应P6的8个端口.

;通过P6口上连接的8个LED灯，利用PWM实现呼吸灯效果.

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
PWM1_Flag       BIT     20H.0
PWM2_Flag       BIT     20H.1
PWM3_Flag       BIT     20H.2
PWM4_Flag       BIT     20H.3

PWM1_Duty_H     DATA    30H
PWM1_Duty_L     DATA    31H
PWM2_Duty_H     DATA    32H
PWM2_Duty_L     DATA    33H
PWM3_Duty_H     DATA    34H
PWM3_Duty_L     DATA    35H
PWM4_Duty_H     DATA    36H
PWM4_Duty_L     DATA    37H

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
    CLR     PWM1_Flag
    CLR     PWM2_Flag
    CLR     PWM3_Flag
    CLR     PWM4_Flag
    MOV     PWM1_Duty_L, #0
    MOV     PWM1_Duty_H, #0
    MOV     PWM2_Duty_L, #LOW 256
    MOV     PWM2_Duty_H, #HIGH 256
    MOV     PWM3_Duty_L, #LOW 512
    MOV     PWM3_Duty_H, #HIGH 512
    MOV     PWM4_Duty_L, #LOW 1024
    MOV     PWM4_Duty_H, #HIGH 1024

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
    CLR     P4.0

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
    MOV     DPTR,#PWMA_CCER1
    MOVX    @DPTR,A
    MOV     DPTR,#PWMA_CCER2
    MOVX    @DPTR,A
    MOV     A,#060H             ;设置 PWMx 模式1 输出
    MOV     DPTR,#PWMA_CCMR1
    MOVX    @DPTR,A
    MOV     DPTR,#PWMA_CCMR2
    MOVX    @DPTR,A
    MOV     DPTR,#PWMA_CCMR3
    MOVX    @DPTR,A
    MOV     DPTR,#PWMA_CCMR4
    MOVX    @DPTR,A
    MOV     A,#055H             ;配置通道输出使能和极性
    MOV     DPTR,#PWMA_CCER1
    MOVX    @DPTR,A
    MOV     DPTR,#PWMA_CCER2
    MOVX    @DPTR,A

    MOV     A,#068H             ;开启PWMA_CCRx预装载功能(需要CCxE=1才可写)
    MOV     DPTR,#PWMA_CCMR1
    MOVX    @DPTR,A
    MOV     DPTR,#PWMA_CCMR2
    MOVX    @DPTR,A
    MOV     DPTR,#PWMA_CCMR3
    MOVX    @DPTR,A
    MOV     DPTR,#PWMA_CCMR4
    MOVX    @DPTR,A

    MOV     A,#3                ;设置周期时间
    MOV     DPTR,#PWMA_ARRH
    MOVX    @DPTR,A
    MOV     A,#0FFH
    MOV     DPTR,#PWMA_ARRL
    MOVX    @DPTR,A

    MOV     A,#0FFH             ;使能 PWM1~4 输出
    MOV     DPTR,#PWMA_ENO
    MOVX    @DPTR,A
    MOV     A,#0AAH             ;高级 PWM 通道输出脚选择位, P6
    MOV     DPTR,#PWMA_PS
    MOVX    @DPTR,A
    MOV     A,#080H             ;使能主输出
    MOV     DPTR,#PWMA_BKR
    MOVX    @DPTR,A

    MOV     DPTR,#PWMA_CR1
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
    MOV     A, PWM1_Duty_H      ;设置占空比时间
    MOV     DPTR, #PWMA_CCR1H
    MOVX    @DPTR, A
    MOV     A, PWM1_Duty_L
    MOV     DPTR, #PWMA_CCR1L
    MOVX    @DPTR, A

    MOV     A, PWM2_Duty_H      ;设置占空比时间
    MOV     DPTR, #PWMA_CCR2H
    MOVX    @DPTR, A
    MOV     A, PWM2_Duty_L
    MOV     DPTR, #PWMA_CCR2L
    MOVX    @DPTR, A

    MOV     A, PWM3_Duty_H      ;设置占空比时间
    MOV     DPTR, #PWMA_CCR3H
    MOVX    @DPTR, A
    MOV     A, PWM3_Duty_L
    MOV     DPTR, #PWMA_CCR3L
    MOVX    @DPTR, A

    MOV     A, PWM4_Duty_H      ;设置占空比时间
    MOV     DPTR, #PWMA_CCR4H
    MOVX    @DPTR, A
    MOV     A, PWM4_Duty_L
    MOV     DPTR, #PWMA_CCR4L
    MOVX    @DPTR, A
    RET

;**************** 中断函数 ***************************************************
F_Timer0_Interrupt: ;Timer0 1ms中断函数
    PUSH    PSW     ;PSW入栈
    PUSH    ACC     ;ACC入栈

    JB      PWM1_Flag, T0_PWM1_SUB
    INC     PWM1_Duty_L       ;PWM1_Duty + 1
    MOV     A, PWM1_Duty_L
    JNZ     $+4
    INC     PWM1_Duty_H
    
    CLR     C
    MOV     A, PWM1_Duty_L
    SUBB    A, #LOW 1023
    MOV     A, PWM1_Duty_H
    SUBB    A, #HIGH 1023
    JC      T0_PWM2_Start
    SETB    PWM1_Flag
    SJMP    T0_PWM2_Start
T0_PWM1_SUB:
    MOV     A, PWM1_Duty_L
    DEC     PWM1_Duty_L       ;PWM1_Duty - 1
    JNZ     $+4
    DEC     PWM1_Duty_H
    
    MOV     A, PWM1_Duty_L
    JNZ     T0_PWM2_Start
    MOV     A, PWM1_Duty_H
    JNZ     T0_PWM2_Start
    CLR     PWM1_Flag

T0_PWM2_Start:
    JB      PWM2_Flag, T0_PWM2_SUB
    INC     PWM2_Duty_L       ;PWM2_Duty + 1
    MOV     A, PWM2_Duty_L
    JNZ     $+4
    INC     PWM2_Duty_H
    
    CLR     C
    MOV     A, PWM2_Duty_L
    SUBB    A, #LOW 1023
    MOV     A, PWM2_Duty_H
    SUBB    A, #HIGH 1023
    JC      T0_PWM3_Start
    SETB    PWM2_Flag
    SJMP    T0_PWM3_Start
T0_PWM2_SUB:
    MOV     A, PWM2_Duty_L
    DEC     PWM2_Duty_L       ;PWM2_Duty - 1
    JNZ     $+4
    DEC     PWM2_Duty_H
    
    MOV     A, PWM2_Duty_L
    JNZ     T0_PWM3_Start
    MOV     A, PWM2_Duty_H
    JNZ     T0_PWM3_Start
    CLR     PWM2_Flag
    
T0_PWM3_Start:
    JB      PWM3_Flag, T0_PWM3_SUB
    INC     PWM3_Duty_L       ;PWM3_Duty + 1
    MOV     A, PWM3_Duty_L
    JNZ     $+4
    INC     PWM3_Duty_H
    
    CLR     C
    MOV     A, PWM3_Duty_L
    SUBB    A, #LOW 1023
    MOV     A, PWM3_Duty_H
    SUBB    A, #HIGH 1023
    JC      T0_PWM4_Start
    SETB    PWM3_Flag
    SJMP    T0_PWM4_Start
T0_PWM3_SUB:
    MOV     A, PWM3_Duty_L
    DEC     PWM3_Duty_L       ;PWM3_Duty - 1
    JNZ     $+4
    DEC     PWM3_Duty_H
    
    MOV     A, PWM3_Duty_L
    JNZ     T0_PWM4_Start
    MOV     A, PWM3_Duty_H
    JNZ     T0_PWM4_Start
    CLR     PWM3_Flag
    
T0_PWM4_Start:
    JB      PWM4_Flag, T0_PWM4_SUB
    INC     PWM4_Duty_L       ;PWM4_Duty + 1
    MOV     A, PWM4_Duty_L
    JNZ     $+4
    INC     PWM4_Duty_H
    
    CLR     C
    MOV     A, PWM4_Duty_L
    SUBB    A, #LOW 1023
    MOV     A, PWM4_Duty_H
    SUBB    A, #HIGH 1023
    JC      F_QuitTimer0
    SETB    PWM4_Flag
    SJMP    F_QuitTimer0
T0_PWM4_SUB:
    MOV     A, PWM4_Duty_L
    DEC     PWM4_Duty_L       ;PWM4_Duty - 1
    JNZ     $+4
    DEC     PWM4_Duty_H
    
    MOV     A, PWM4_Duty_L
    JNZ     F_QuitTimer0
    MOV     A, PWM4_Duty_H
    JNZ     F_QuitTimer0
    CLR     PWM4_Flag
        
F_QuitTimer0:
    LCALL   F_UpdatePwm

    POP     ACC     ;ACC出栈
    POP     PSW     ;PSW出栈
    RETI

;======================================================================

    END

