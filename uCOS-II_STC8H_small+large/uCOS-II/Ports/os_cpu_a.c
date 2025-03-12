/*-----------------------------------------------------------*
os_cpu_a.c
*-----------------------------------------------------------*/

#include "config.h"

/*-----------------------------------------------------------*
PendSv中断初始化
*-----------------------------------------------------------*/
static void PendSvInit(void)
{
    //配置PendSv为最低中断优先级的代码
    //配置PendSv相关的其它支持代码
    PendSv_ClearFlag();         //清除标志
    PendSv_Enable();            //开中断
}


/*---------------------------------------------------------
滴答初始化
----------------------------------------------------------*/
#define TIM0_DIV  (MAIN_Fosc/OS_TICKS_PER_SEC>=65536UL?12:1)            //分频
#define TIM0_LOAD ((MAIN_Fosc*2UL/TIM0_DIV/OS_TICKS_PER_SEC+1UL)/2UL)   //装载值
static void TickInit(void)
{
    AUXR = (TIM0_DIV == 1) ? AUXR|0x80 : AUXR&~0x80;
    TMOD &= 0xF0;                   //设置定时器模式
    TL0 = 65536UL-TIM0_LOAD;        //设置定时初始值
    TH0 = 65536UL-TIM0_LOAD>>8;     //设置定时初始值
    TF0 = 0;                        //清除TF0标志
    ET0 = 1;                        //使能中断
    TR0 = 1;                        //定时器开启
}


/*-----------------------------------------------------------*
C51 地址结构混乱, 这里主要是占用地址 (xdata*)NULL 和 (xdata*)NULL+1,
有较小的机会正好一个OS变量分配到 ((xdata*) 0) 时 ,
判断空地址语句将会出错!
比如 (p==NULL) (ptcp==OS_TCB_RESERVED) 判断NULL地址或特殊地址出错！
*-----------------------------------------------------------*/
static INT16U xdata _bNULL _at_ 0;


/*-----------------------------------------------------------*
临界区模式2嵌套计数器
*-----------------------------------------------------------*/
#if OS_CRITICAL_METHOD == 2
INT8U data _uxNesting_ = 0;
bit _bEA_ = 0;
#endif


/*-----------------------------------------------------------*
时钟滴答中断
*-----------------------------------------------------------*/
void Timer0_ISR_Handler (void) interrupt 1        
{
    OSTimeTick_FROM_ISR();
}


/*-----------------------------------------------------------*
汇编变量申明
*-----------------------------------------------------------*/
#pragma asm
    ?STACK  SEGMENT     IDATA
    EXTRN   DATA        (?C_XBP)
    EXTRN   DATA        (?C_IBP)
#pragma endasm


/*-----------------------------------------------------------*
寄存器入栈
*-----------------------------------------------------------*/
#pragma asm
    PUSHALL MACRO
    PUSH     ACC
    PUSH     B
    PUSH     DPH
    PUSH     DPL
    PUSH     PSW
    PUSH     AR0
    PUSH     AR1
    PUSH     AR2
    PUSH     AR3
    PUSH     AR4
    PUSH     AR5
    PUSH     AR6
    PUSH     AR7
    ENDM
#pragma endasm

/*-----------------------------------------------------------*
出栈到寄存器
*-----------------------------------------------------------*/
#pragma asm
    POPALL  MACRO
    POP      AR7
    POP      AR6
    POP      AR5
    POP      AR4
    POP      AR3
    POP      AR2
    POP      AR1
    POP      AR0
    POP      PSW
    POP      DPL
    POP      DPH
    POP      B
    POP      ACC
    ENDM
#pragma endasm


/*-----------------------------------------------------------*
第一次切换到任务里去
*-----------------------------------------------------------*/
void OSStartHighRdy(void)
{
    EA=0;               //防止进入任务前产生中断
    
#pragma asm
    MOV      DPTR,#ID_CH_Start
    MOV      A,#?STACK-1
    MOVX     @DPTR,A
#pragma endasm

    ID_CH_Start += 15;  //这里是读出SP初值以备检查堆栈使用, 其中15个字节必用到,所以直接加15
    ID_STK_Size = 0XFF-ID_CH_Start+15;  //栈容量
    ID_STK_Used = 15;   //已使用15字节
    ID_STK_Free = ID_STK_Size-15;   //剩余字节

    PendSvInit();       //初始化PendSV
    TickInit();         //初始化滴答,准备运行

#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0)
    OSTaskSwHook();
#endif
    
#pragma asm
    MOV      DPH,OSTCBCur
    MOV      DPL,OSTCBCur+01H
    MOVX     A,@DPTR
    MOV     R4,A
    INC     DPTR
    MOVX     A,@DPTR
    MOV      DPH,R4
    MOV      DPL,A
    MOVX     A,@DPTR
    MOV      R7,A            //栈数据深度
    MOV     SP,#?STACK-1
setSTKD:
    INC     DPTR
    MOVX    A,@DPTR
    PUSH    ACC;
    DJNZ    R7,setSTKD      //复制栈数据

    INC     DPTR
    MOVX    A,@DPTR
    MOV     ?C_IBP, A       //恢复IBP
    JZ      setXBP
    MOV     R0,A
    CPL      A
    INC      A               
    MOV     R7,A            //IBP数据深度
setIBPD:
    INC     DPTR
    MOVX    A,@DPTR
    MOV     @R0,A
    INC     R0
    DJNZ    R7,setIBPD      //恢复IBP数据
    
setXBP:
    INC     DPTR
    MOV     ?C_XBP,DPH
    MOV     ?C_XBP+1,DPL
    
    POPALL
#pragma endasm

    OSRunning=1;
    EA=1;
}

/*-----------------------------------------------------------*
pendSV_ISR__Handler
*-----------------------------------------------------------*/
void pendSV_ISR_Handler (void)
{
#pragma asm
    
    PUSHALL

    MOV      A,?C_IBP
    CPL      A
    INC      A          //补码
    MOV      R7,A           //IBP数据深度

    CLR      C
    MOV      A,SP
    SUBB     A,#?STACK-1
    MOV      R6,A           //SP深度

    ADD      A,R7
    ADD      A,#02H         //额外两字节
    CPL     A
    INC     A               //补码
    ADD      A,?C_XBP+01H
    MOV     R5,A            //更新后的栈顶低字节
    MOV     A,?C_XBP
    ADDC    A,#0FFH
    MOV     R4,A            //更新后的栈顶高字节

    MOV      DPL,OSTCBCur+01H
    MOV      DPH,OSTCBCur
    MOVX     @DPTR,A
    INC      DPTR
    MOV      A,R5
    MOVX     @DPTR,A        //提前更新OSTCBStkPtr
    
    MOV     DPH,R4          //OSTCBStkPtr写入DPTR
    MOV     DPL,R5
    
    MOV     A,R6
    MOVX    @DPTR,A         //保存SP深度

    MOV     R0,#?STACK-1
saveSTKD:                   
    INC     DPTR
    INC     R0
    MOV     A,@R0
    MOVX    @DPTR,A
    DJNZ    R6,saveSTKD     //保存上下文
      
    MOV     A,?C_IBP
    INC     DPTR                
    MOVX    @DPTR,A         //保存IBP
    
    MOV     A,R7
    JZ      save_Finish     //如果没有IBP数据
    MOV     R0,?C_IBP
saveIBPD:
    MOV     A,@R0
    INC     R0
    INC     DPTR                
    MOVX    @DPTR,A         //保存IBP数据
    DJNZ    R7,saveIBPD
save_Finish:
#pragma endasm

    EA=0;
#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0)
    OSTaskSwHook();
#endif
    PendSv_ClearFlag();
    OSPrioCur = OSPrioHighRdy;
    OSTCBCur = OSTCBHighRdy;
    EA=1;

#pragma asm
    MOV      DPH,OSTCBCur
    MOV      DPL,OSTCBCur+01H
    MOVX     A,@DPTR
    MOV     R4,A
    INC     DPTR
    MOVX     A,@DPTR
    MOV      DPH,R4
    MOV      DPL,A
    MOVX     A,@DPTR
    MOV      R7,A            //栈数据深度
    MOV     SP,#?STACK-1
recoverSTKD:
    INC     DPTR
    MOVX    A,@DPTR
    PUSH    ACC;
    DJNZ    R7,recoverSTKD  //复制栈数据

    INC     DPTR
    MOVX    A,@DPTR
    MOV     ?C_IBP, A       //恢复IBP
    JZ      recoverXBP
    MOV     R0,A
    CPL      A
    INC      A
    MOV     R7,A            //IBP数据深度
recoverIBPD:
    INC     DPTR            
    MOVX    A,@DPTR
    MOV     @R0,A
    INC     R0
    DJNZ    R7,recoverIBPD  //恢复IBP数据
    
recoverXBP:
    INC     DPTR
    CLR     EA
    MOV     ?C_XBP,DPH
    MOV     ?C_XBP+1,DPL
    SETB    EA

    POPALL
    RETI
#pragma endasm
}



