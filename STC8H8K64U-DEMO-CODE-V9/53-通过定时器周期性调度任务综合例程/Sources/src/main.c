/*---------------------------------------------------------------------*/
/* --- STC MCU Limited ------------------------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.STCAI.com ---------------------------------------------*/
/* --- Web: www.STCMCUDATA.com  ---------------------------------------*/
/* --- BBS: www.STCAIMCU.com  -----------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* 如果要在程序中使用此代码,请在程序中注明使用了STC的资料及程序            */
/*---------------------------------------------------------------------*/

#include "Task.h"
#include "System_init.h"

/*************	功能说明	**************

本例程基于STC8H为主控芯片的实验箱进行编写测试。

使用Timer0的16位自动重装来产生1ms节拍，程序使用这个节拍进行任务调度.

每个任务设置相应的调度周期，根据设置的周期时间执行任务函数.

下载时, 选择时钟 24MHz (可以在配置文件"config.h"中修改).

******************************************/

//========================================================================
// 函数: void main(void)
// 描述: 主函数程序.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2022-05-26
//========================================================================
void main(void)
{
    SYS_Init();

    while (1)
    {
        Task_Pro_Handler_Callback();
    }
}
