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
;/* 如果要在程序中使用此代码,请在程序中注明使用了STC的资料及程序            */
;/*---------------------------------------------------------------------*/


;*************  功能说明    **************

;本例程基于STC8H8K64U为主控芯片的实验箱9进行编写测试，STC8H系列带RTC模块的芯片可通用参考.

;读写芯片内部集成的RTC模块.

;用STC的MCU的IO方式驱动8位数码管。

;使用Timer0的16位自动重装来产生1ms节拍,程序运行于这个节拍下, 用户修改MCU主时钟频率时,自动定时于1ms.

;8位数码管显示时间(小时-分钟-秒).

;行列扫描按键键码为25~32.

;按键只支持单键按下, 不支持多键同时按下, 那样将会有不可预知的结果.

;键按下超过1秒后,将以10键/秒的速度提供重键输出. 用户只需要检测KeyCode是否非0来判断键是否按下.

;调整时间键:
;键码25: 小时+.
;键码26: 小时-.
;键码27: 分钟+.
;键码28: 分钟-.

;下载时, 选择时钟 24MHZ (用户可自行修改频率).

;******************************************/

$include (../../comm/STC8H.INC)

;/****************************** 用户定义宏 ***********************************/

Fosc_KHZ    EQU 24000   ;24000KHZ

STACK_POIRTER   EQU     0D0H    ;堆栈开始地址

Timer0_Reload   EQU     (65536 - Fosc_KHZ)  ; Timer 0 中断频率, 1000次/秒

DIS_DOT         EQU     020H
DIS_BLACK       EQU     010H
DIS_            EQU     011H

;*******************************************************************
;*******************************************************************


;*************  IO口定义    **************/


;*************  本地变量声明    **************/
Flag0           DATA    20H
B_1ms           BIT     Flag0.0 ;   1ms标志

LED8            DATA    30H     ;   显示缓冲 30H ~ 37H
display_index   DATA    38H     ;   显示位索引

usrhour         DATA    39H     ;RTC变量
usrminute       DATA    3AH
usrsecond       DATA    3BH     ;
msecond_H       DATA    3CH     ;
msecond_L       DATA    3DH     ;

IO_KeyState     DATA    3EH ; IO行列键状态变量
IO_KeyState1    DATA    3FH
IO_KeyHoldCnt   DATA    40H ; IO键按下计时
KeyCode         DATA    41H ; 给用户使用的键码, 1~16为ADC键， 17~32为IO键

cnt50ms         DATA    45H;


;*******************************************************************
;*******************************************************************
        ORG     0000H               ;reset
        LJMP    F_Main


        ORG     000BH               ;1  Timer0 interrupt
        LJMP    F_Timer0_Interrupt

        ORG     0123H               ;36 RTC interrupt
        LJMP    F_RTC_Interrupt


;*******************************************************************
;*******************************************************************


;******************** 主程序 **************************/
        ORG     0200H       ;reset
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
    MOV     PSW, #0     ;选择第0组R0~R7
    USING   0       ;选择第0组R0~R7

;================= 用户初始化程序 ====================================

    MOV     display_index, #0
    MOV     R0, #LED8
    MOV     R2, #8
L_ClearLoop:
    MOV     @R0, #DIS_BLACK     ;上电消隐
    INC     R0
    DJNZ    R2, L_ClearLoop
    
    CLR     TR0
    ORL     AUXR, #(1 SHL 7)    ; Timer0_1T();
    ANL     TMOD, #NOT 04H      ; Timer0_AsTimer();
    ANL     TMOD, #NOT 03H      ; Timer0_16bitAutoReload();
    MOV     TH0, #Timer0_Reload / 256   ;Timer0_Load(Timer0_Reload);
    MOV     TL0, #Timer0_Reload MOD 256
    SETB    ET0         ; Timer0_InterruptEnable();
    SETB    TR0         ; Timer0_Run();

    LCALL   F_RTC_Init
    SETB    EA          ; 打开总中断

    LCALL   F_DisplayRTC

    CLR     A
    MOV     IO_KeyState, A
    MOV     IO_KeyState1, A
    MOV     IO_KeyHoldCnt, A
    MOV     KeyCode, A      ; 给用户使用的键码, 17~32有效
    MOV     cnt50ms, #50

;=====================================================

;=================== 主循环 ==================================
L_Main_Loop:
    JNB     B_1ms,  L_Main_Loop     ;1ms未到
    CLR     B_1ms
    
;=================== 检测1000ms是否到 ==================================
    INC     msecond_L       ;msecond + 1
    MOV     A, msecond_L
    JNZ     L_Check1000ms
    INC     msecond_H
L_Check1000ms:
    CLR     C
    MOV     A, msecond_L    ;msecond - 1000
    SUBB    A, #LOW 1000
    MOV     A, msecond_H
    SUBB    A, #HIGH 1000
    JC      L_Quit_Check_1000ms     ;if(msecond < 1000), jmp
    
;================= 1000ms到， 处理RTC ====================================
    MOV     msecond_L, #0   ;if(msecond >= 1000)
    MOV     msecond_H, #0

    LCALL   F_DisplayRTC
L_Quit_Check_1000ms:

;======================= 50ms扫描一次行列键盘 ==============================
L_Read_IO_Key:
    DJNZ    cnt50ms, L_Quit_Read_IO_Key     ; (cnt50ms - 1) != 0, jmp
    MOV     cnt50ms, #50    ;
    LCALL   F_IO_KeyScan    ;50ms扫描一次行列键盘
L_Quit_Read_IO_Key:
;=====================================================


;=====================================================
L_KeyProcess:
    MOV     A, KeyCode
    JZ      L_Quit_KeyProcess
                        ;有键按下
    MOV     A, KeyCode
    CJNE    A, #25, L_Quit_K25
    INC     usrhour        ;if(KeyCode == 17)  hour +1
    MOV     A, usrhour
    CLR     C
    SUBB    A, #24
    JC      L_K25_Display
    MOV     usrhour, #0
L_K25_Display:
    LCALL   F_WriteRTC
    LCALL   F_DisplayRTC
L_Quit_K25:

    MOV     A, KeyCode
    CJNE    A, #26, L_Quit_K26
    DEC     usrhour    ; hour - 1
    MOV     A, usrhour
    CJNE    A, #255, L_K26_Display
    MOV     usrhour, #23
L_K26_Display:
    LCALL   F_WriteRTC
    LCALL   F_DisplayRTC
L_Quit_K26:

    MOV     A, KeyCode
    CJNE    A, #27, L_Quit_K27
    INC     usrminute  ; minute + 1
    MOV     A, usrminute
    CLR     C
    SUBB    A, #60
    JC      L_K27_Display
    MOV     usrminute, #0
L_K27_Display:
    LCALL   F_WriteRTC
    LCALL   F_DisplayRTC
L_Quit_K27:

    MOV     A, KeyCode
    CJNE    A, #28, L_Quit_K28
    DEC     usrminute  ; minute - 1
    MOV     A, usrminute
    CJNE    A, #255, L_K28_Display
    MOV     usrminute, #59
L_K28_Display:
    LCALL   F_WriteRTC
    LCALL   F_DisplayRTC
L_Quit_K28:

    MOV     KeyCode, #0
L_Quit_KeyProcess:

    LJMP    L_Main_Loop

;**********************************************/

;========================================================================
; 函数: F_RTC_Init
; 描述: RTC初始化程序.
; 参数: none
; 返回: none.
; 版本: V1.0, 2021-3-4
;========================================================================
F_RTC_Init:
    ;-------时间设置------
    MOV     A,#21
    MOV     DPTR,#INIYEAR       ;Y:2021
    MOVX    @DPTR,A

    MOV     A,#12
    MOV     DPTR,#INIMONTH      ;M:12
    MOVX    @DPTR,A

    MOV     A,#31
    MOV     DPTR,#INIDAY        ;D:31
    MOVX    @DPTR,A

    MOV     A,#23
    MOV     DPTR,#INIHOUR       ;H:23
    MOVX    @DPTR,A

    MOV     A,#59
    MOV     DPTR,#INIMIN        ;M:59
    MOVX    @DPTR,A

    MOV     A,#50
    MOV     DPTR,#INISEC        ;S:50
    MOVX    @DPTR,A

    MOV     A,#0
    MOV     DPTR,#INISSEC       ;S/128:0
    MOVX    @DPTR,A

    ;-------闹钟设置------
    MOV     A,#0
    MOV     DPTR,#ALAHOUR       ;闹钟小时
    MOVX    @DPTR,A

    MOV     A,#0
    MOV     DPTR,#ALAMIN        ;闹钟分钟
    MOVX    @DPTR,A

    MOV     A,#0
    MOV     DPTR,#ALASEC        ;闹钟秒
    MOVX    @DPTR,A

    MOV     A,#0
    MOV     DPTR,#ALASSEC       ;闹钟1/128秒
    MOVX    @DPTR,A

    ;-------时钟设置------
    ;STC8H8K64U B版本芯片使用内部32K时钟，休眠无法唤醒
;    MOV     A,#80H	            ;启动内部32K IRC
;    MOV     DPTR,#IRC32KCR
;    MOVX    @DPTR,A
;    MOVX    A,@DPTR
;    JNB     ACC.0,$-1	        ;等待时钟稳定
;    MOV     A,#3
;    MOV     DPTR,#RTCCFG        ;选择内部32K时钟源，触发RTC寄存器初始化
;    MOVX    @DPTR,A

    MOV     A,#0C0H
    MOV     DPTR,#X32KCR        ;启动外部32K晶振, 低增益+0x00, 高增益+0x40.
    MOVX    @DPTR,A
    MOVX    A,@DPTR
    JNB     ACC.0,$-1	        ;等待时钟稳定
    MOV     A,#1
    MOV     DPTR,#RTCCFG        ;选择外部32K时钟源，触发RTC寄存器初始化
    MOVX    @DPTR,A

    ;-------中断设置------
    MOV     A,#0
    MOV     DPTR,#RTCIF         ;清中断标志
    MOVX    @DPTR,A

    MOV     A,#088H
    MOV     DPTR,#RTCIEN        ;中断使能, 0x80:闹钟中断, 0x40:日中断, 0x20:小时中断, 0x10:分钟中断, 0x08:秒中断, 0x04:1/2秒中断, 0x02:1/8秒中断, 0x01:1/32秒中断
    MOVX    @DPTR,A

    ;-------RTC使能-------
    MOV     A,#1
    MOV     DPTR,#RTCCR         ;RTC使能
    MOVX    @DPTR,A

    MOV     DPTR,#RTCCFG
    MOVX    A,@DPTR
    JB      ACC.0,$-1	        ;等待初始化完成,需要在 "RTC使能" 之后判断. 
    ;设置RTC时间需要32768Hz的1个周期时间,大约30.5us. 由于同步, 所以实际等待时间是0~30.5us.
    ;如果不等待设置完成就睡眠, 则RTC会由于设置没完成, 停止计数, 唤醒后才继续完成设置并继续计数.
    RET


;/*****************************************************
;   行列键扫描程序
;   使用XY查找4x4键的方法，只能单键，速度快
;
;   Y     P04      P05      P06      P07
;          |        |        |        |
;X         |        |        |        |
;P00 ---- K00 ---- K01 ---- K02 ---- K03 ----
;          |        |        |        |
;P01 ---- K04 ---- K05 ---- K06 ---- K07 ----
;          |        |        |        |
;P02 ---- K08 ---- K09 ---- K10 ---- K11 ----
;          |        |        |        |
;P03 ---- K12 ---- K13 ---- K14 ---- K15 ----
;          |        |        |        |
;******************************************************/


T_KeyTable:  DB 0,1,2,0,3,0,0,0,4,0,0,0,0,0,0,0

F_IO_KeyDelay:
    PUSH    03H     ;R3入栈
    MOV     R3, #60
    DJNZ    R3, $   ; (n * 4) T
    POP     03H     ;R3出栈
    RET

F_IO_KeyScan:   ;50ms call
    PUSH    06H     ;R6入栈
    PUSH    07H     ;R7入栈

    MOV     R6, IO_KeyState1    ; 保存上一次状态

    MOV     P0, #0F0H       ;X低，读Y
    LCALL   F_IO_KeyDelay       ;delay about 250T
    MOV     A, P0
    ANL     A, #0F0H
    MOV     IO_KeyState1, A     ; IO_KeyState1 = P0 & 0xf0

    MOV     P0, #0FH        ;Y低，读X
    LCALL   F_IO_KeyDelay       ;delay about 250T
    MOV     A, P0
    ANL     A, #0FH
    ORL     A, IO_KeyState1         ; IO_KeyState1 |= (P0 & 0x0f)
    MOV     IO_KeyState1, A
    XRL     IO_KeyState1, #0FFH     ; IO_KeyState1 ^= 0xff; 取反

    MOV     A, R6                   ;if(j == IO_KeyState1), 连续两次读相等
    CJNE    A, IO_KeyState1, L_QuitCheckIoKey   ;不相等, jmp
    
    MOV     R6, IO_KeyState     ;暂存IO_KeyState
    MOV     IO_KeyState, IO_KeyState1
    MOV     A, IO_KeyState
    JZ      L_NoIoKeyPress      ; if(IO_KeyState != 0), 有键按下

    MOV     A, R6   
    JZ      L_CalculateIoKey    ;if(R6 == 0)    F0 = 1; 第一次按下
    MOV     A, R6   
    CJNE    A, IO_KeyState, L_QuitCheckIoKey    ; if(j != IO_KeyState), jmp
    
    INC     IO_KeyHoldCnt   ; if(++IO_KeyHoldCnt >= 20),    1秒后重键
    MOV     A, IO_KeyHoldCnt
    CJNE    A, #20, L_QuitCheckIoKey
    MOV     IO_KeyHoldCnt, #18;
L_CalculateIoKey:
    MOV     A, IO_KeyState
    SWAP    A       ;R6 = T_KeyTable[IO_KeyState >> 4];
    ANL     A, #0x0F
    MOV     DPTR, #T_KeyTable
    MOVC    A, @A+DPTR
    MOV     R6, A
    
    JZ      L_QuitCheckIoKey    ; if(R6 == 0)
    MOV     A, IO_KeyState
    ANL     A, #0x0F
    MOVC    A, @A+DPTR
    MOV     R7, A
    JZ      L_QuitCheckIoKey    ; if(T_KeyTable[IO_KeyState& 0x0f] == 0)
    
    MOV     A, R6       ;KeyCode = (j - 1) * 4 + T_KeyTable[IO_KeyState & 0x0f] + 16;   //计算键码，17~32
    ADD     A, ACC
    ADD     A, ACC
    MOV     R6, A
    MOV     A, R7
    ADD     A, R6
    ADD     A, #12
    MOV     KeyCode, A
    SJMP    L_QuitCheckIoKey
    
L_NoIoKeyPress:
    MOV     IO_KeyHoldCnt, #0

L_QuitCheckIoKey:
    MOV     P0, #0xFF
    POP     07H     ;R7出栈
    POP     06H     ;R6出栈
    REt



; *********************** 显示相关程序 ****************************************
T_Display:                      ;标准字库
;    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
DB  03FH,006H,05BH,04FH,066H,06DH,07DH,007H,07FH,06FH,077H,07CH,039H,05EH,079H,071H
;  black  -    H    J    K    L    N    o    P    U    t    G    Q    r    M    y
DB  000H,040H,076H,01EH,070H,038H,037H,05CH,073H,03EH,078H,03dH,067H,050H,037H,06EH
;    0.   1.   2.   3.   4.   5.   6.   7.   8.   9.   -1
DB  0BFH,086H,0DBH,0CFH,0E6H,0EDH,0FDH,087H,0FFH,0EFH,046H

T_COM:
DB  001H,002H,004H,008H,010H,020H,040H,080H     ;   位码


;//========================================================================
;// 函数: F_DisplayScan
;// 描述: 显示扫描子程序。
;// 参数: none.
;// 返回: none.
;// 版本: VER1.0
;// 日期: 2013-4-1
;// 备注: 除了ACCC和PSW外, 所用到的通用寄存器都入栈
;//========================================================================
F_DisplayScan:
    PUSH    DPH     ;DPH入栈
    PUSH    DPL     ;DPL入栈
    PUSH    00H     ;R0 入栈
    
    MOV     DPTR, #T_COM
    MOV     A, display_index
    MOVC    A, @A+DPTR
    CPL     A
    MOV     P7,A
    
    MOV     DPTR, #T_Display
    MOV     A, display_index
    ADD     A, #LED8
    MOV     R0, A
    MOV     A, @R0
    MOVC    A, @A+DPTR
    CPL     A
    MOV     P6,A

    INC     display_index
    MOV     A, display_index
    ANL     A, #0F8H            ; if(display_index >= 8)
    JZ      L_QuitDisplayScan
    MOV     display_index, #0;  ;8位结束回0
L_QuitDisplayScan:
    POP     00H     ;R0 出栈
    POP     DPL     ;DPL出栈
    POP     DPH     ;DPH出栈
    RET

;**************** 中断函数 ***********************************************

F_Timer0_Interrupt: ;Timer0 1ms中断函数
    PUSH    PSW     ;PSW入栈
    PUSH    ACC     ;ACC入栈

    LCALL   F_DisplayScan   ; 1ms扫描显示一位
    SETB    B_1ms           ; 1ms标志

    POP     ACC     ;ACC出栈
    POP     PSW     ;PSW出栈
    RETI

;========================================================================
F_RTC_Interrupt:    ;RTC中断函数
    PUSH    PSW     ;PSW入栈
    PUSH    ACC     ;ACC入栈

    MOV     DPTR,#RTCIF       ;检测从机状态
    MOVX    A,@DPTR
    JB      ACC.7,ALARMIF
    JB      ACC.3,SECONDIF
    CLR     A
    MOVX    @DPTR,A
    JMP     ISREXIT

ALARMIF:
    ANL     A,#NOT 080H       ;清中断标志
    MOVX    @DPTR,A
    CPL     P4.6              ;测试端口
    JMP     ISREXIT

SECONDIF:
    ANL     A,#NOT 08H        ;清中断标志
    MOVX    @DPTR,A
    CPL     P4.7              ;测试端口
    JMP     ISREXIT

ISREXIT:
    POP     ACC     ;ACC出栈
    POP     PSW     ;PSW出栈
    RETI

;========================================================================
; 函数: F_DisplayRTC
; 描述: 显示时钟子程序。
; 参数: none.
; 返回: none.
; 版本: VER1.0
; 日期: 2013-4-1
; 备注: 除了ACCC和PSW外, 所用到的通用寄存器都入栈
;========================================================================
F_DisplayRTC:
    PUSH    B       ;B入栈

    MOV     DPTR,#HOUR
    MOVX    A,@DPTR
    MOV     usrhour, A
    MOV     B, #10
    DIV     AB
    MOV     LED8+7, A
    MOV     LED8+6, B

    MOV     LED8+5, #DIS_

    MOV     DPTR,#MIN
    MOVX    A,@DPTR
    MOV     usrminute, A
    MOV     B, #10
    DIV     AB
    MOV     LED8+4, A;
    MOV     LED8+3, B;

    MOV     LED8+2, #DIS_

    MOV     DPTR,#SEC
    MOVX    A,@DPTR
    MOV     B, #10
    DIV     AB
    MOV     LED8+1, A;
    MOV     LED8+0, B;
L_QuitDisplayRTC:
    POP     B       ;B出栈
    RET

;========================================================================
; 函数: F_WriteRTC
; 描述: 写RTC函数。
; 参数: none.
; 返回: none.
; 版本: VER1.0
; 日期: 2013-4-1
; 备注: 
;========================================================================
F_WriteRTC:
    MOV     DPTR,#YEAR
    MOVX    A,@DPTR
    MOV     DPTR,#INIYEAR    ;继承当前年
    MOVX    @DPTR,A

    MOV     DPTR,#MONTH
    MOVX    A,@DPTR
    MOV     DPTR,#INIMONTH   ;继承当前月
    MOVX    @DPTR,A

    MOV     DPTR,#DAY
    MOVX    A,@DPTR
    MOV     DPTR,#INIDAY     ;继承当前日
    MOVX    @DPTR,A

    MOV     A,usrhour
    MOV     DPTR,#INIHOUR    ;修改时
    MOVX    @DPTR,A

    MOV     A,usrminute
    MOV     DPTR,#INIMIN     ;修改分
    MOVX    @DPTR,A

    MOV     A,#0
    MOV     DPTR,#INISEC     ;修改秒
    MOVX    @DPTR,A

    MOV     A,#0
    MOV     DPTR,#INISSEC    ;修改秒
    MOVX    @DPTR,A

    MOV     DPTR,#RTCCFG
    MOVX    A,@DPTR
    ORL     A,#01H
    MOVX    @DPTR,A

    RET



    END

