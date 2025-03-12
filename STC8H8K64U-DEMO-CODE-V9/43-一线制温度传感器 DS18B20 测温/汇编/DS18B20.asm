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

;通过一个IO口获取一线制温度传感器 DS18B20 温度值.

;使用Timer0的16位自动重装来产生1ms节拍,程序运行于这个节拍下, 用户修改MCU主时钟频率时,自动定时于1ms.

;用STC的MCU的IO方式驱动8位数码管，通过数码管显示测量的温度值.

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
DQ  BIT P3.3                ;DS18B20的数据口位P3.3

;*************  本地变量声明    **************/
Flag0           DATA    20H
B_1ms           BIT     Flag0.0 ;   1ms标志

LED8            DATA    30H     ;   显示缓冲 30H ~ 37H
display_index   DATA    38H     ;   显示位索引

msecond_H       DATA    39H     ;
msecond_L       DATA    3AH     ;

;*******************************************************************
;*******************************************************************

        ORG     0000H               ;reset
        LJMP    F_Main

        ORG     000BH               ;1  Timer0 interrupt
        LJMP    F_Timer0_Interrupt

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
    SETB    EA          ; 打开总中断

;=====================================================

L_Main_Loop:
    JNB     B_1ms,  L_Main_Loop     ;1ms未到
    CLR     B_1ms
    
;=================== 检测300ms是否到 ==================================
    INC     msecond_L       ;msecond + 1
    MOV     A, msecond_L
    JNZ     $+4
    INC     msecond_H
    
    CLR     C
    MOV     A, msecond_L    ;msecond - 300
    SUBB    A, #LOW 300
    MOV     A, msecond_H
    SUBB    A, #HIGH 300
    JC      L_Main_Loop     ;if(msecond < 300), jmp
    
;================= 300ms到 ====================================
    MOV     msecond_L, #0   ;if(msecond >= 1000)
    MOV     msecond_H, #0

    CALL    DS18B20_Reset       ;设备复位
    MOV     A,#0CCH             ;跳过ROM命令
    CALL    DS18B20_WriteByte   ;送出命令
    MOV     A,#044H             ;开始转换
    CALL    DS18B20_WriteByte   ;送出命令
    JNB     DQ,$                ;等待转换完成

    CALL    DS18B20_Reset       ;设备复位
    MOV     A,#0CCH             ;跳过ROM命令
    CALL    DS18B20_WriteByte   ;送出命令
    MOV     A,#0BEH             ;读暂存存储器
    CALL    DS18B20_WriteByte   ;送出命令
    CALL    DS18B20_ReadByte    ;读温度低字节
    MOV     R5,A                ;存储数据
    CALL    DS18B20_ReadByte    ;读温度高字节
    MOV     R4,A                ;存储数据

    MOV     R7, #5          ; 0.0625 * 10，保留1位小数点
    LCALL   F_MUL16x8       ;(R4,R5)* R7 -->(R5,R6,R7)
    MOV     R4, #0
    MOV     R2, #0
    MOV     R3, #8          ; 温度 * 0.625 = 温度 * 5/8
    LCALL   F_ULDIV         ;(R4,R5,R6,R7)/(R2,R3)=(R4,R5,R6,R7),余数在(R2,R3),use R0~R7,B,DPL

    LCALL   F_HEX2_DEC      ;(R6 R7) HEX Change to DEC ---> (R3 R4 R5), use (R2~R7)
    MOV     A, R4
    SWAP    A
    ANL     A, #0x0F
    MOV     LED8+3, A       ;显示温度值
    MOV     A, R4
    ANL     A, #0x0F
    MOV     LED8+2, A
    MOV     A, R5
    SWAP    A
    ANL     A, #0x0F
    ADD     A, #DIS_DOT
    MOV     LED8+1, A
    MOV     A, R5
    ANL     A, #0x0F
    MOV     LED8, A
    MOV     A, LED8+3
    JNZ     L_LED8_3_Not_0
    MOV     LED8+3, #DIS_BLACK      ;千位为0则消隐
L_LED8_3_Not_0:
    JNB     F0, L_QuitRead_Temp
    MOV     LED8+3, #DIS_   ;负温度, 显示-
L_QuitRead_Temp:

    LJMP    L_Main_Loop

;//========================================================================
;// 函数: F_HEX2_DEC
;// 描述: 把双字节十六进制数转成十进制BCD数.
;// 参数: (R6 R7): 要转换的双字节十六进制数.
;// 返回: (R3 R4 R5) = BCD码.
;// 版本: V1.0, 2013-10-22
;//========================================================================
F_HEX2_DEC:         ;(R6 R7) HEX Change to DEC ---> (R3 R4 R5), use (R2~R7)
    MOV     R2,#16
    MOV     R3,#0
    MOV     R4,#0
    MOV     R5,#0

L_HEX2_DEC:
    CLR     C   
    MOV     A,R7
    RLC     A   
    MOV     R7,A
    MOV     A,R6
    RLC     A   
    MOV     R6,A

    MOV     A,R5
    ADDC    A,R5
    DA      A   
    MOV     R5,A

    MOV     A,R4
    ADDC    A,R4
    DA      A   
    MOV     R4,A

    MOV     A,R3
    ADDC    A,R3
    DA      A   
    MOV     R3,A

    DJNZ    R2,L_HEX2_DEC
    RET

;***************************************************************************
F_MUL16x8:               ;(R4,R5)* R7 -->(R5,R6,R7)
    MOV   A,R7      ;1T     1
    MOV   B,R5      ;2T     3
    MUL   AB        ;4T  R3*R7  4
    MOV   R6,B      ;1T     4
    XCH   A,R7      ;2T     3
    
    MOV   B,R4      ;1T     3
    MUL   AB        ;4T  R3*R6  4
    ADD   A,R6      ;1T     2
    MOV   R6,A      ;1T     3
    CLR   A         ;1T     1
    ADDC  A,B       ;1T     3
    MOV   R5,A      ;1T     2
    RET             ;4T     10

;***************************************************************************
F_ULDIV:
F_DIV32:                            ; (R4,R5,R6,R7)/(R2,R3)=(R4,R5,R6,R7),余数在(R2,R3),use R0~R7,B,DPL
    CJNE    R2,#0,F_DIV32_16    ; L_0075
        
F_DIV32_8:                          ;R3非0，(R4,R5,R6,R7)/R3=(R4,R5,R6,R7),余数在 R3,use R0~R7,B
    MOV     A,R4
    MOV     B,R3
    DIV     AB
    XCH     A,R7
    XCH     A,R6
    XCH     A,R5
    MOV     R4,A
    MOV     A,B
    XCH     A,R3
    MOV     R1,A
    MOV     R0,#24
L_0056:
    MOV     A,R7
    ADD     A,R7
    MOV     R7,A
    MOV     A,R6
    RLC     A
    MOV     R6,A
    MOV     A,R5
    RLC     A
    MOV     R5,A
    MOV     A,R4
    RLC     A
    MOV     R4,A
    MOV     A,R3
    RLC     A
    MOV     R3,A
    JBC     CY,L_006B
    SUBB    A,R1
    JC      L_006F
L_006B:
    MOV     A,R3
    SUBB    A,R1
    MOV     R3,A
    INC     R7
L_006F:
    DJNZ    R0,L_0056
    CLR     A
    MOV     R1,A
    MOV     R2,A
    RET

;***************************************************************************
F_DIV32_16:                     ;R2非0，(R4,R5,R6,R7)/(R2,R3)=(R5,R6,R7),余数在 (R2,R3),use R0~R7
L_0075:
    MOV     R0,#24          ;开始R1=0
L_0077:
    MOV     A,R7            ;左移一位
    ADD     A,R7
    MOV     R7,A
    MOV     A,R6
    RLC     A
    MOV     R6,A
    MOV     A,R5
    RLC     A
    MOV     R5,A
    MOV     A,R4
    RLC     A
    MOV     R4,A
    XCH     A,R1
    RLC     A
    XCH     A,R1
    JBC     CY,L_008E   ;如果C=1，肯定够减
    SUBB    A,R3
    MOV     A,R1        ;测试是否够减
    SUBB    A,R2
    JC      L_0095
L_008E:
    MOV     A,R4
    SUBB    A,R3
    MOV     R4,A
    MOV     A,R1
    SUBB    A,R2
    MOV     R1,A
    INC     R7
L_0095:
    DJNZ    R0,L_0077
    CLR     A
    XCH     A,R1
    MOV     R2,A
    CLR     A
    XCH     A,R4
    MOV     R3,A
    RET

;**************************************
;延时X微秒(12M)
;不同的工作环境,需要调整此函数
;入口参数:R7
;出口参数:无
;**************************************
DelayXus:                   ;6 此延时函数是使用1T的指令周期进行计算,与传统的12T的MCU不同
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    NOP                     ;1
    DJNZ R7,DelayXus        ;4
    RET                     ;4

;**************************************
;复位DS18B20,并检测设备是否存在
;入口参数:无
;出口参数:无
;**************************************
DS18B20_Reset:
    CLR     DQ                 ;送出低电平复位信号
    MOV     R7,#240            ;延时至少480us
    CALL    DelayXus
    MOV     R7,#240
    CALL    DelayXus
    SETB    DQ                 ;释放数据线
    MOV     R7,#60             ;等待60us
    CALL    DelayXus
    MOV     C,DQ               ;检测存在脉冲
    MOV     R7,#240            ;等待设备释放数据线
    CALL    DelayXus
    MOV     R7,#180
    CALL    DelayXus
    JC      DS18B20_Reset      ;如果设备不存在,则继续等待
    RET

;**************************************
;从DS18B20读1字节数据
;入口参数:无
;出口参数:ACC
;**************************************
DS18B20_ReadByte:
    CLR     A
    PUSH    0
    MOV     0,#8               ;8位计数器
ReadNext:
    CLR     DQ                 ;开始时间片
    MOV     R7,#1              ;延时等待
    CALL    DelayXus
    SETB    DQ                 ;准备接收
    MOV     R7,#1
    CALL    DelayXus
    MOV     C,DQ               ;读取数据
    RRC     A
    MOV     R7,#60             ;等待时间片结束
    CALL    DelayXus
    DJNZ    0,ReadNext
    POP     0
    RET

;**************************************
;向DS18B20写1字节数据
;入口参数:ACC
;出口参数:无
;**************************************
DS18B20_WriteByte:
    PUSH    0
    MOV     0,#8               ;8位计数器
WriteNext:
    CLR     DQ                 ;开始时间片
    MOV     R7,#1              ;延时等待
    CALL    DelayXus
    RRC     A                  ;输出数据
    MOV     DQ,C
    MOV     R7,#60             ;等待时间片结束
    CALL    DelayXus
    SETB    DQ                 ;准备送出下一位数据
    MOV     R7,#1
    CALL    DelayXus
    DJNZ    0,WriteNext
    POP     0
    RET

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

;**************** 中断函数 ***************************************************

F_Timer0_Interrupt: ;Timer0 1ms中断函数
    PUSH    PSW     ;PSW入栈
    PUSH    ACC     ;ACC入栈

    LCALL   F_DisplayScan   ; 1ms扫描显示一位
    SETB    B_1ms           ; 1ms标志

    POP     ACC     ;ACC出栈
    POP     PSW     ;PSW出栈
    RETI

    END

