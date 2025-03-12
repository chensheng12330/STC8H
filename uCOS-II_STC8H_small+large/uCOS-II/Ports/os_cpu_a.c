/*-----------------------------------------------------------*
os_cpu_a.c
*-----------------------------------------------------------*/

#include "config.h"

/*-----------------------------------------------------------*
PendSv�жϳ�ʼ��
*-----------------------------------------------------------*/
static void PendSvInit(void)
{
    //����PendSvΪ����ж����ȼ��Ĵ���
    //����PendSv��ص�����֧�ִ���
    PendSv_ClearFlag();         //�����־
    PendSv_Enable();            //���ж�
}


/*---------------------------------------------------------
�δ��ʼ��
----------------------------------------------------------*/
#define TIM0_DIV  (MAIN_Fosc/OS_TICKS_PER_SEC>=65536UL?12:1)            //��Ƶ
#define TIM0_LOAD ((MAIN_Fosc*2UL/TIM0_DIV/OS_TICKS_PER_SEC+1UL)/2UL)   //װ��ֵ
static void TickInit(void)
{
    AUXR = (TIM0_DIV == 1) ? AUXR|0x80 : AUXR&~0x80;
    TMOD &= 0xF0;                   //���ö�ʱ��ģʽ
    TL0 = 65536UL-TIM0_LOAD;        //���ö�ʱ��ʼֵ
    TH0 = 65536UL-TIM0_LOAD>>8;     //���ö�ʱ��ʼֵ
    TF0 = 0;                        //���TF0��־
    ET0 = 1;                        //ʹ���ж�
    TR0 = 1;                        //��ʱ������
}


/*-----------------------------------------------------------*
C51 ��ַ�ṹ����, ������Ҫ��ռ�õ�ַ (xdata*)NULL �� (xdata*)NULL+1,
�н�С�Ļ�������һ��OS�������䵽 ((xdata*) 0) ʱ ,
�жϿյ�ַ��佫�����!
���� (p==NULL) (ptcp==OS_TCB_RESERVED) �ж�NULL��ַ�������ַ����
*-----------------------------------------------------------*/
static INT16U xdata _bNULL _at_ 0;


/*-----------------------------------------------------------*
�ٽ���ģʽ2Ƕ�׼�����
*-----------------------------------------------------------*/
#if OS_CRITICAL_METHOD == 2
INT8U data _uxNesting_ = 0;
bit _bEA_ = 0;
#endif


/*-----------------------------------------------------------*
ʱ�ӵδ��ж�
*-----------------------------------------------------------*/
void Timer0_ISR_Handler (void) interrupt 1        
{
    OSTimeTick_FROM_ISR();
}


/*-----------------------------------------------------------*
����������
*-----------------------------------------------------------*/
#pragma asm
    ?STACK  SEGMENT     IDATA
    EXTRN   DATA        (?C_XBP)
    EXTRN   DATA        (?C_IBP)
#pragma endasm


/*-----------------------------------------------------------*
�Ĵ�����ջ
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
��ջ���Ĵ���
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
��һ���л���������ȥ
*-----------------------------------------------------------*/
void OSStartHighRdy(void)
{
    EA=0;               //��ֹ��������ǰ�����ж�
    
#pragma asm
    MOV      DPTR,#ID_CH_Start
    MOV      A,#?STACK-1
    MOVX     @DPTR,A
#pragma endasm

    ID_CH_Start += 15;  //�����Ƕ���SP��ֵ�Ա�����ջʹ��, ����15���ֽڱ��õ�,����ֱ�Ӽ�15
    ID_STK_Size = 0XFF-ID_CH_Start+15;  //ջ����
    ID_STK_Used = 15;   //��ʹ��15�ֽ�
    ID_STK_Free = ID_STK_Size-15;   //ʣ���ֽ�

    PendSvInit();       //��ʼ��PendSV
    TickInit();         //��ʼ���δ�,׼������

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
    MOV      R7,A            //ջ�������
    MOV     SP,#?STACK-1
setSTKD:
    INC     DPTR
    MOVX    A,@DPTR
    PUSH    ACC;
    DJNZ    R7,setSTKD      //����ջ����

    INC     DPTR
    MOVX    A,@DPTR
    MOV     ?C_IBP, A       //�ָ�IBP
    JZ      setXBP
    MOV     R0,A
    CPL      A
    INC      A               
    MOV     R7,A            //IBP�������
setIBPD:
    INC     DPTR
    MOVX    A,@DPTR
    MOV     @R0,A
    INC     R0
    DJNZ    R7,setIBPD      //�ָ�IBP����
    
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
    INC      A          //����
    MOV      R7,A           //IBP�������

    CLR      C
    MOV      A,SP
    SUBB     A,#?STACK-1
    MOV      R6,A           //SP���

    ADD      A,R7
    ADD      A,#02H         //�������ֽ�
    CPL     A
    INC     A               //����
    ADD      A,?C_XBP+01H
    MOV     R5,A            //���º��ջ�����ֽ�
    MOV     A,?C_XBP
    ADDC    A,#0FFH
    MOV     R4,A            //���º��ջ�����ֽ�

    MOV      DPL,OSTCBCur+01H
    MOV      DPH,OSTCBCur
    MOVX     @DPTR,A
    INC      DPTR
    MOV      A,R5
    MOVX     @DPTR,A        //��ǰ����OSTCBStkPtr
    
    MOV     DPH,R4          //OSTCBStkPtrд��DPTR
    MOV     DPL,R5
    
    MOV     A,R6
    MOVX    @DPTR,A         //����SP���

    MOV     R0,#?STACK-1
saveSTKD:                   
    INC     DPTR
    INC     R0
    MOV     A,@R0
    MOVX    @DPTR,A
    DJNZ    R6,saveSTKD     //����������
      
    MOV     A,?C_IBP
    INC     DPTR                
    MOVX    @DPTR,A         //����IBP
    
    MOV     A,R7
    JZ      save_Finish     //���û��IBP����
    MOV     R0,?C_IBP
saveIBPD:
    MOV     A,@R0
    INC     R0
    INC     DPTR                
    MOVX    @DPTR,A         //����IBP����
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
    MOV      R7,A            //ջ�������
    MOV     SP,#?STACK-1
recoverSTKD:
    INC     DPTR
    MOVX    A,@DPTR
    PUSH    ACC;
    DJNZ    R7,recoverSTKD  //����ջ����

    INC     DPTR
    MOVX    A,@DPTR
    MOV     ?C_IBP, A       //�ָ�IBP
    JZ      recoverXBP
    MOV     R0,A
    CPL      A
    INC      A
    MOV     R7,A            //IBP�������
recoverIBPD:
    INC     DPTR            
    MOVX    A,@DPTR
    MOV     @R0,A
    INC     R0
    DJNZ    R7,recoverIBPD  //�ָ�IBP����
    
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



