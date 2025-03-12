/*---------------------------------------------------------------------
main.c
---------------------------------------------------------------------*/

#include    "config.h"
#include    "STC8G_H_GPIO.H"
#include    "UART.H"




/*---------------------------------------------------------
GPIO_config
----------------------------------------------------------*/
void GPIO_config(void)
{
    P0_MODE_IO_PU(GPIO_Pin_All);        //P0 ����Ϊ׼˫���
    P1_MODE_IO_PU(GPIO_Pin_All);        //P1 ����Ϊ׼˫���
    P2_MODE_IO_PU(GPIO_Pin_All);        //P2 ����Ϊ׼˫���
    P3_MODE_IO_PU(GPIO_Pin_All);        //P3 ����Ϊ׼˫���
    P4_MODE_IO_PU(GPIO_Pin_All);        //P4 ����Ϊ׼˫���
    P5_MODE_IO_PU(GPIO_Pin_All);        //P5 ����Ϊ׼˫���
    P6_MODE_IO_PU(GPIO_Pin_All);        //P6 ����Ϊ׼˫���
    P7_MODE_IO_PU(GPIO_Pin_All);        //P7 ����Ϊ׼˫���
    P7_MODE_IO_PU(GPIO_Pin_All);        //P7 ����Ϊ׼˫���
}


/*---------------------------------------------------------
�л�ʱ��,ʹ���ⲿ����
----------------------------------------------------------*/
#if USE_Extern_Fosc
void Switch_clock_xosccr(void)
{
    P1_MODE_IN_HIZ(GPIO_Pin_6|GPIO_Pin_7);
    P1_DIGIT_IN_DISABLE(GPIO_Pin_6|GPIO_Pin_7);
    XOSCCR=0XC0;    //�����ⲿ����
    while(!(XOSCCR&0x01));
    CLKDIV=0x01;    //��ƵΪ1
    CLKSEL=0X01;    //�л�
    HIRCCR=0X00;
    IRC32KCR=0;
}
#endif //USE_Extern_Fosc


/*---------------------------------------------------------
main()��һ�����õĺ���,����Ҫ���ϵ��ʼ��IO,ʱ��. 
��Ҫ�������ʼ��Ƭ������, Ƭ������ĳ�ʼ��,���ڵ�һ������Ŀ�ʼ���Ƚ��׵�.
----------------------------------------------------------*/
void main_hook(void)
{
    EAXSFR();           //ʹ��XSFR����
    GPIO_config();      //GPIO ��ʼ��

#if USE_Extern_Fosc
    Switch_clock_xosccr();  //�л�ʱ��
#endif 
}


/*---------------------------------------------------------
main
----------------------------------------------------------*/
void main(void)
{
    main_hook();
    
    OSInit();
    
    OSTaskCreateExt(START_TASK,                                     //������             
                    NULL,                                           //���ݲ���
                    &START_TASK_STK[START_TASK_STK_SIZE-1],         //ջ��
                    APP_CFG_START_TASK_PRIO,                        //���ȼ�
                    APP_CFG_START_TASK_PRIO,                        //����ID
                    &START_TASK_STK[0],                             //ջ��
                    START_TASK_STK_SIZE,                            //��ջ��С
                    NULL,                                           //��չ ����ָ��
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR         //�����ջ��飬��ʼ����ջ����   
                   );

    OSStart();
}

/*---------------------------------------------------------
* STCISP���, ֻҪ��⵽�����ظ������ݵ�һ�������͸�λ
* �����������
* ������ɺ���Ҫ���α�����, �Ա�����������
----------------------------------------------------------*/
#if USE_STCISP_MOMITORING
void isp_monitoring(u8 *buf, u16 len)
{
    static u8 xdata nu=0, key=0;
    while(len--){
        if(*buf==key){
            nu++;
        }else{
            nu=0;
            key=*buf;
        }
        buf++;
        if(nu>=STCISP_KEYWORDS_COUNT) IAP_CONTR=0X60;
    }
}
#endif



