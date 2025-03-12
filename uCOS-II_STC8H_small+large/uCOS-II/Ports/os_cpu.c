/*
*********************************************************************************************************
*                                              uC/OS-II
*                                        The Real-Time Kernel
*
*                    Copyright 1992-2021 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS

#include "ucos_ii.h"

/*
*********************************************************************************************************
*                                        LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#if      (OS_VERSION >= 281) && (OS_TMR_EN > 0)
  static  INT16U  OSTmrCtr;
#endif                                                                  /* #if (OS_VERSION >= 281) && (OS_TMR_EN > 0)               */

/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSInitHookBegin (void)FunctionProperties
{
#if OS_VERSION >= 281 && OS_TMR_EN > 0
    OSTmrCtr = 0;
#endif
}
#endif

/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                               (END)
*
* Description: This function is called by OSInit() at the end of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSInitHookEnd (void)FunctionProperties
{
}
#endif


/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0
void  OSTaskCreateHook (OS_TCB *ptcb)FunctionProperties
{
#if OS_APP_HOOKS_EN > 0
    App_TaskCreateHook(ptcb);
#else
    if(ptcb);                                                         /* Prevent compiler warning                                 */
#endif
}
#endif


/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0
void  OSTaskDelHook (OS_TCB *ptcb)FunctionProperties
{  
#if OS_APP_HOOKS_EN > 0
    App_TaskDelHook(ptcb);
#else
    if(ptcb);                                                         /* Prevent compiler warning                                 */
#endif
}
#endif

/*
*********************************************************************************************************
*                                             IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do
*              such things as STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0 && OS_VERSION >= 251
void  OSTaskIdleHook (void)FunctionProperties
{
#if OS_APP_HOOKS_EN > 0
    App_TaskIdleHook();
#endif
}
#endif

/*
*********************************************************************************************************
*                                            TASK RETURN HOOK
*
* Description: This function is called if a task accidentally returns.  In other words, a task should
*              either be an infinite loop or delete itself when done.
*
* Arguments  : ptcb      is a pointer to the task control block of the task that is returning.
*
* Note(s)    : none
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0u
void  OSTaskReturnHook (OS_TCB  *ptcb)FunctionProperties
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskReturnHook(ptcb);
#else
    if(ptcb);
#endif
}
#endif

/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0
void  OSTaskStatHook (void)FunctionProperties
{
#if OS_APP_HOOKS_EN > 0
    App_TaskStatHook();
#endif
}
#endif


/*
**********************************************************************************************************
*                                       INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created. This function is highly processor specific.
*
* Arguments  : task          is a pointer to the task code
*
*              p_arg         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.
*
*              ptos          is a pointer to the top of stack. It is assumed that 'ptos' points to the
*                            highest valid address on the stack.
*
*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
*                            (see uCOS_II.H for OS_TASK_OPT_???).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
**********************************************************************************************************
*/

OS_STK *OSTaskStkInit (void (*task)(void *p_arg), void *p_arg, OS_STK *ptos, INT16U opt)FunctionProperties
{
    if(opt);
                                             // ptos即?C_XBP, ?C_XBP不需要保存.  ?C_XBP数据长度=0BYTE 
                                             // ?C_IBP 数据长度 = 0BYTE
    *ptos-- = 0;                             // ?C_IBP
    *ptos-- = 0x00;                          // R7
    *ptos-- = 0x00;                          // R6
    *ptos-- = 0x00;                          // R5
    *ptos-- = 0x00;                          // R4
    *ptos-- = (INT32U)p_arg >> 16;           // R3
    *ptos-- = (INT16U)p_arg >> 8;            // R2
    *ptos-- = (INT16U)p_arg & 0xFF;          // R1
    *ptos-- = 0x00;                          // R0
    *ptos-- = 0x00;                          // PSW
    *ptos-- = 0x00;                          // DPL
    *ptos-- = 0x00;                          // DPH
    *ptos-- = 0x00;                          // B
    *ptos-- = 0x00;                          // ACC
    *ptos-- = (INT16U)task >> 8;             // 任务代码地址高8位
    *ptos-- = (INT16U)task & 0xFF;           // 任务代码地址低8位
    *ptos   = 15;                            // 栈数据长度

    return (ptos);
}

/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0)
void  OSTaskSwHook (void)FunctionProperties
{
#if OS_APP_HOOKS_EN > 0
    App_TaskSwHook();
#endif
}
#endif

/*
*********************************************************************************************************
*                                           OS_TCBInit() HOOK
*
* Description: This function is called by OS_TCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSTCBInitHook (OS_TCB *ptcb)FunctionProperties
{
#if OS_APP_HOOKS_EN > 0
    App_TCBInitHook(ptcb);
#else
    if(ptcb);                                                         /* Prevent compiler warning   */
#endif
}
#endif


/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0) && (OS_TIME_TICK_HOOK_EN > 0)
void  OSTimeTickHook (void)FunctionProperties
{
#if OS_APP_HOOKS_EN > 0
    App_TimeTickHook();
#endif

#if OS_VERSION >= 281 && OS_TMR_EN > 0
    OSTmrCtr++;
    if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
        OSTmrCtr = 0;
        OSTmrSignal();
    }
#endif
}
#endif


/*-----------------------------------------------------------*
* idata公共运行栈检查, 检查方法是查找栈中连续0最大的块
* 此函数在统计任务中自动调用,也可主动调用. 没有返回值,通过全局变量回传数据.
* 使用此功能时,必须在STARTUP.A51中清零所有的IDATA.
* 有多种可能导致检查的结果不准确
* 当剩余的栈容量小余栈正常数据中连续的0时,将得到错误的结果.
* 极端的可能是正常的栈数据中含有大量的连续0
* 所以,只有在栈剩余量较大时才有参考意义
* 虽然结果可能不准确,但是,当栈剩余量较大时仍有参考意义.
*-----------------------------------------------------------*/
void IDStkChk(void)FunctionProperties 
{
    INT8U addr=ID_CH_Start+1,maxFree=0,cnt=0;
    
    for(;addr;addr++){
        if( ((u8 idata *)0)[addr] == 0){
            cnt++;
        }else{
            maxFree = (cnt>maxFree)? cnt:maxFree;
            cnt=0;
        }
    }
    maxFree = (cnt>maxFree)? cnt:maxFree;
    
    if(maxFree<ID_STK_Free){
        ID_STK_Free = maxFree;
        ID_STK_Used = ID_STK_Size-ID_STK_Free;
    }
}
