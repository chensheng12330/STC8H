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
    P0_MODE_IO_PU(GPIO_Pin_All);        //P0 设置为准双向口
    P1_MODE_IO_PU(GPIO_Pin_All);        //P1 设置为准双向口
    P2_MODE_IO_PU(GPIO_Pin_All);        //P2 设置为准双向口
    P3_MODE_IO_PU(GPIO_Pin_All);        //P3 设置为准双向口
    P4_MODE_IO_PU(GPIO_Pin_All);        //P4 设置为准双向口
    P5_MODE_IO_PU(GPIO_Pin_All);        //P5 设置为准双向口
    P6_MODE_IO_PU(GPIO_Pin_All);        //P6 设置为准双向口
    P7_MODE_IO_PU(GPIO_Pin_All);        //P7 设置为准双向口
    P7_MODE_IO_PU(GPIO_Pin_All);        //P7 设置为准双向口
}


/*---------------------------------------------------------
切换时钟,使用外部晶振
----------------------------------------------------------*/
#if USE_Extern_Fosc
void Switch_clock_xosccr(void)
{
    P1_MODE_IN_HIZ(GPIO_Pin_6|GPIO_Pin_7);
    P1_DIGIT_IN_DISABLE(GPIO_Pin_6|GPIO_Pin_7);
    XOSCCR=0XC0;    //开启外部晶振
    while(!(XOSCCR&0x01));
    CLKDIV=0x01;    //分频为1
    CLKSEL=0X01;    //切换
    HIRCCR=0X00;
    IRC32KCR=0;
}
#endif //USE_Extern_Fosc


/*---------------------------------------------------------
main()第一个调用的函数,仅必要的上电初始化IO,时钟. 
不要在这里初始化片上外设, 片上外设的初始化,放在第一个任务的开始处比较妥当.
----------------------------------------------------------*/
void main_hook(void)
{
    EAXSFR();           //使能XSFR访问
    GPIO_config();      //GPIO 初始化

#if USE_Extern_Fosc
    Switch_clock_xosccr();  //切换时钟
#endif 
}


/*---------------------------------------------------------
main
----------------------------------------------------------*/
void main(void)
{
    main_hook();
    
    OSInit();
    
    OSTaskCreateExt(START_TASK,                                     //任务名             
                    NULL,                                           //传递参数
                    &START_TASK_STK[START_TASK_STK_SIZE-1],         //栈顶
                    APP_CFG_START_TASK_PRIO,                        //优先级
                    APP_CFG_START_TASK_PRIO,                        //任务ID
                    &START_TASK_STK[0],                             //栈底
                    START_TASK_STK_SIZE,                            //堆栈大小
                    NULL,                                           //扩展 备用指针
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR         //允许堆栈检查，初始化堆栈清零   
                   );

    OSStart();
}

/*---------------------------------------------------------
* STCISP监测, 只要监测到连续重复的数据到一定数量就复位
* 不检查命令字
* 调试完成后需要屏蔽本函数, 以避免意外重启
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



