/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
*                                            EXAMPLE CODE
*
* Filename : app_cfg.h
*********************************************************************************************************
*/

#ifndef  _APP_CFG_H_
#define  _APP_CFG_H_


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>
#include  <config.h>


/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           TASK PRIORITIES
*********************************************************************************************************
*/

#define     OS_TASK_TMR_PRIO            5


#define     APP_CFG_UART1_MUTEX_PRIO    1


#define     APP_CFG_START_TASK_PRIO     6
#define     APP_CFG_TASK_B_PRIO         7
#define     APP_CFG_TASK_C_PRIO         8

/*
*********************************************************************************************************
*                                          TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define     START_TASK_STK_SIZE         128
#define     TASK_B_STK_SIZE             128
#define     TASK_C_STK_SIZE             128


/*------------------------------------------------------------------------------
全局变量
------------------------------------------------------------------------------*/
extern OS_EVENT    *Uart1Mutex;


/*---------------------------------------------------------
任务声明
----------------------------------------------------------*/
void START_TASK(void *p_arg);
void TASK_B(void *p_arg);
void TASK_C(void *p_arg);


/*---------------------------------------------------------
任务静态栈
----------------------------------------------------------*/
extern OS_STK START_TASK_STK[START_TASK_STK_SIZE];  
extern OS_STK TASK_B_STK[TASK_B_STK_SIZE];
extern OS_STK TASK_C_STK[TASK_C_STK_SIZE];


/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/

#ifndef  TRACE_LEVEL_OFF
#define  TRACE_LEVEL_OFF                    0u
#endif

#ifndef  TRACE_LEVEL_INFO
#define  TRACE_LEVEL_INFO                   1u
#endif

#ifndef  TRACE_LEVEL_DBG
#define  TRACE_LEVEL_DBG                    2u
#endif

#define  APP_TRACE_LEVEL                   TRACE_LEVEL_OFF
#define  APP_TRACE                         printf

#define  APP_TRACE_INFO(x)    ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_TRACE x) : (void)0)
#define  APP_TRACE_DBG(x)     ((APP_TRACE_LEVEL >= TRACE_LEVEL_DBG)   ? (void)(APP_TRACE x) : (void)0)


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of module include.              */
