/*---------------------------------------------------------------------
task.c
---------------------------------------------------------------------*/

#include "config.h"
#include "STC8G_H_Delay.h"
#include "UART.H"

void isp_monitoring(u8 *buf, u16 len);

/*---------------------------------------------------------
����̬ջ
----------------------------------------------------------*/
OS_STK START_TASK_STK[START_TASK_STK_SIZE];  
OS_STK TASK_B_STK[TASK_B_STK_SIZE];
OS_STK TASK_C_STK[TASK_C_STK_SIZE];


/*------------------------------------------------------------------------------
ȫ�ֱ���
------------------------------------------------------------------------------*/
OS_EVENT    *Uart1Mutex;

OS_EVENT    *testSem;        
OS_EVENT    *testMbox;        
OS_FLAG_GRP *testFlag;
void * xdata MessageStorage[8];
OS_EVENT    *testQ;
OS_TMR      *testTmr;
u32 xdata   MEM_32X4[32];
OS_MEM      *testMem32;


/*-----------------------------------------------------------*
TMR �ص�
*-----------------------------------------------------------*/
void MyCallback (void *ptmr, void *p_arg)FunctionProperties
{
    P24=~P24;
    if(ptmr);
    if(p_arg);
}


/*-----------------------------------------------------------*
START_TASK
*-----------------------------------------------------------*/
void START_TASK(void *p_arg)
{
    u8 perr;
    
#if OS_TASK_STAT_EN > 0u
    OSStatInit();
#endif
    
    UART1_Init(9600,UART1_BRT_Timer1,UART1_SW_P30_P31);

    testMem32 = OSMemCreate (MEM_32X4,4,32,&perr);  //�����ڴ����
    
    testSem = OSSemCreate (0);
    
    testMbox = OSMboxCreate(NULL);
    
    testFlag = OSFlagCreate(0,&perr);
    
    testQ = OSQCreate(MessageStorage,8);
    
    testTmr  = OSTmrCreate(0,25,OS_TMR_OPT_PERIODIC,MyCallback,NULL,NULL,&perr);
    
    OSTmrStart(testTmr,&perr);
    
    Uart1Mutex=OSMutexCreate(APP_CFG_UART1_MUTEX_PRIO,&perr);
    
    OSTaskCreateExt(TASK_B,                                     //������             
                    NULL,                                       //���ݲ���
                    &TASK_B_STK[TASK_B_STK_SIZE-1],             //ջ��
                    APP_CFG_TASK_B_PRIO,                        //���ȼ�
                    APP_CFG_TASK_B_PRIO,                        //����ID
                    &TASK_B_STK[0],                             //ջ��
                    TASK_B_STK_SIZE,                            //��ջ��С
                    NULL,                                       //��չ ����ָ��
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR     //�����ջ��飬��ʼ����ջ����   
                   );
                   
    OSTaskCreateExt(TASK_C,                                     //������             
                    NULL,                                       //���ݲ���
                    &TASK_C_STK[TASK_C_STK_SIZE-1],             //ջ��
                    APP_CFG_TASK_C_PRIO,                        //���ȼ�
                    APP_CFG_TASK_C_PRIO,                        //����ID
                    &TASK_C_STK[0],                             //ջ��
                    TASK_C_STK_SIZE,                            //��ջ��С
                    NULL,                                       //��չ ����ָ��
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR     //�����ջ��飬��ʼ����ջ����   
                   );

    
    while(OSRunning)
    {
        u8 uart_dat[16],len,perr;
        
        OSMutexPend (Uart1Mutex,0,&perr);
        do{
            len = UART1_Receive(uart_dat,16);   //����������
            UART1_Send(uart_dat,len);           //ԭ·����
#if USE_STCISP_MOMITORING
            isp_monitoring(uart_dat,len);
#endif
        }while(len==16);
        OSMutexPost (Uart1Mutex);

        P27=!P27;
        OSTimeDly(50);
    }
    if(p_arg);
}


/*-----------------------------------------------------------*
Task_B
*-----------------------------------------------------------*/
void TASK_B(void *p_arg)
{
    u8 perr;
    while(OSRunning)
    {
        OSSchedLock();
        OSSchedUnlock();
        
        OSSemPost(testSem);
        OSMboxPost(testMbox,testMbox);
        OSFlagPost(testFlag,0x01,OS_FLAG_SET,&perr);
        OSQPost(testQ,testQ);
        OSTaskSuspend(OS_PRIO_SELF);
        
        P23=~P23;
    }
    if(p_arg);
}


/*-----------------------------------------------------------*
Task_C
*-----------------------------------------------------------*/
void TASK_C(void *p_arg)
{
    u8 perr;
    
    while(OSRunning)
    {
        OSSemPend (testSem,0,&perr);
        OSMboxPend(testMbox,0,&perr);
        OSQPend(testQ,0,&perr);
        OSFlagPend (testFlag,0x01,OS_FLAG_WAIT_SET_ANY+OS_FLAG_CONSUME,0,&perr);

        OSTimeDly(500);
        P22=~P22;

        OSTaskResume(APP_CFG_TASK_B_PRIO);
    }
    if(p_arg);
}


/*------------------------------------------------------------------------------
VisualizationHook,  ��Լÿ��ᱻ����һ��
------------------------------------------------------------------------------*/
void VisualizationHook(void)
{
    u8 perr,*testPt;
    testPt = OSMemGet (testMem32,&perr);
    if(perr){
        printf("OSMemGet Error; %u\r\n", (u16)perr);   
    }else{
        printf("MemAddr;    0X%lX\r\n", (u32)testPt );
        OSMemPut(testMem32,testPt);
    }
    
    P20=~P20;
}
