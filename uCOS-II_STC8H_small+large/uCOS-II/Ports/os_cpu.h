/*---------------------------------------------------------------------
os_cpu.h
---------------------------------------------------------------------*/


#ifndef    __OS_CPU_H__
#define    __OS_CPU_H__

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

#include    "config.h"


/******************************************************************************
*                    定义与编译器无关的数据类型
******************************************************************************/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;       /* Unsigned  8 bit quantity       */
typedef signed   char  INT8S;       /* Signed    8 bit quantity       */
typedef unsigned int   INT16U;      /* Unsigned 16 bit quantity       */
typedef signed   int   INT16S;      /* Signed   16 bit quantity       */
typedef unsigned long  INT32U;      /* Unsigned 32 bit quantity       */
typedef signed   long  INT32S;      /* Signed   32 bit quantity       */
typedef float          FP32;        /* Single precision floating point */
typedef double         FP64;        /* Double precision floating point */

typedef unsigned char xdata OS_STK; /* Each stack entry is 32-bit wide */
typedef unsigned char  OS_CPU_SR;   /* Define size of CPU status register */


#define  OS_STK_GROWTH        1     //定义栈的增长方向, 对应的是模拟栈, 向下增长

#define  OS_CRITICAL_METHOD   2     //进入临界段的方法(51内核推荐方法2,因为重入时对局部变量的访问效率不高)


//OS_CRITICAL_METHOD = 1 :              //直接打开或关闭EA,不支持嵌套
#if OS_CRITICAL_METHOD == 1             //调用OS服务前一刻, 要保证EA=1
#define  OS_ENTER_CRITICAL()    EA=0
#define  OS_EXIT_CRITICAL()     EA=1
#endif


//OS_CRITICAL_METHOD = 2 :  //嵌套层数存于全局变量,最大嵌套255层
#if OS_CRITICAL_METHOD == 2
extern INT8U data _uxNesting_;
extern bit _bEA_;
#define  OS_ENTER_CRITICAL()    do{ if(!_testbit_(EA)) { if(_uxNesting_==0)_bEA_=0;} else { if(_uxNesting_ == 0)_bEA_=1;}_uxNesting_++; }while(0)
#define  OS_EXIT_CRITICAL()     do{ if(!--_uxNesting_)EA=_bEA_; }while(0)
#endif


//OS_CRITICAL_METHOD = 3 :  状态字保存在局部变量cpu_sr
#if OS_CRITICAL_METHOD == 3
#define  OS_ENTER_CRITICAL()    do{cpu_sr=((!_testbit_(EA))?0X00:0X80);}while(0)    /* 直接用宏插入C语言, 省去调用过程. 可嵌套, 但不支持同一函数内多次进入 */
#define  OS_EXIT_CRITICAL()     do{IE|=cpu_sr;}while(0)
#endif


void    OSStartHighRdy(void);
void    IDStkChk(void) FunctionProperties;



/*---------------------------------------------------------------------
模拟PendSv中断行为的相关宏定义
---------------------------------------------------------------------*/
#define  PendSv_SetFlag()           do{AUXINTIF|=0X04;}while(0)             //设置PendSv标志对应的指令或函数
#define  PendSv_ClearFlag()         do{AUXINTIF&=~0X04 ;}while(0)           //清除PendSv标志对应的指令或函数
#define  PendSv_GetFlag()           ((INT8U)(AUXINTIF&0X04))                //返回PendSv标志的表达式或函数
#define  PendSv_Enable()            do{IE2|=0x40;}while(0)                  //PendSv中断允许
#define  PendSv_Disable()           do{IE2&=~0x40;}while(0)                 //PendSv中断禁止



#define  OS_TASK_SW()               do{PendSv_SetFlag();OS_EXIT_CRITICAL();while(PendSv_GetFlag());return;}while(0)
#define  OS_TASK_SW_FROM_ISR()      do{PendSv_SetFlag();}while(0)







#endif

