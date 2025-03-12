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
*                    ������������޹ص���������
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


#define  OS_STK_GROWTH        1     //����ջ����������, ��Ӧ����ģ��ջ, ��������

#define  OS_CRITICAL_METHOD   2     //�����ٽ�εķ���(51�ں��Ƽ�����2,��Ϊ����ʱ�Ծֲ������ķ���Ч�ʲ���)


//OS_CRITICAL_METHOD = 1 :              //ֱ�Ӵ򿪻�ر�EA,��֧��Ƕ��
#if OS_CRITICAL_METHOD == 1             //����OS����ǰһ��, Ҫ��֤EA=1
#define  OS_ENTER_CRITICAL()    EA=0
#define  OS_EXIT_CRITICAL()     EA=1
#endif


//OS_CRITICAL_METHOD = 2 :  //Ƕ�ײ�������ȫ�ֱ���,���Ƕ��255��
#if OS_CRITICAL_METHOD == 2
extern INT8U data _uxNesting_;
extern bit _bEA_;
#define  OS_ENTER_CRITICAL()    do{ if(!_testbit_(EA)) { if(_uxNesting_==0)_bEA_=0;} else { if(_uxNesting_ == 0)_bEA_=1;}_uxNesting_++; }while(0)
#define  OS_EXIT_CRITICAL()     do{ if(!--_uxNesting_)EA=_bEA_; }while(0)
#endif


//OS_CRITICAL_METHOD = 3 :  ״̬�ֱ����ھֲ�����cpu_sr
#if OS_CRITICAL_METHOD == 3
#define  OS_ENTER_CRITICAL()    do{cpu_sr=((!_testbit_(EA))?0X00:0X80);}while(0)    /* ֱ���ú����C����, ʡȥ���ù���. ��Ƕ��, ����֧��ͬһ�����ڶ�ν��� */
#define  OS_EXIT_CRITICAL()     do{IE|=cpu_sr;}while(0)
#endif


void    OSStartHighRdy(void);
void    IDStkChk(void) FunctionProperties;



/*---------------------------------------------------------------------
ģ��PendSv�ж���Ϊ����غ궨��
---------------------------------------------------------------------*/
#define  PendSv_SetFlag()           do{AUXINTIF|=0X04;}while(0)             //����PendSv��־��Ӧ��ָ�����
#define  PendSv_ClearFlag()         do{AUXINTIF&=~0X04 ;}while(0)           //���PendSv��־��Ӧ��ָ�����
#define  PendSv_GetFlag()           ((INT8U)(AUXINTIF&0X04))                //����PendSv��־�ı��ʽ����
#define  PendSv_Enable()            do{IE2|=0x40;}while(0)                  //PendSv�ж�����
#define  PendSv_Disable()           do{IE2&=~0x40;}while(0)                 //PendSv�жϽ�ֹ



#define  OS_TASK_SW()               do{PendSv_SetFlag();OS_EXIT_CRITICAL();while(PendSv_GetFlag());return;}while(0)
#define  OS_TASK_SW_FROM_ISR()      do{PendSv_SetFlag();}while(0)







#endif

