/*---------------------------------------------------------
UART.C
----------------------------------------------------------*/


#include "UART.H"



#define BTL_LOAD   (65536UL-(MAIN_Fosc/BRT+2UL)/4UL)


/*---------------------------------------------------------
//不要调用不可重入的库函数
----------------------------------------------------------*/
#if (UART1_EN  == 1) || (UART2_EN  == 1) || (UART3_EN  == 1) || (UART4_EN  == 1)
static void  memcopy  (u8 *s1, const u8 *s2, u16 n)FunctionProperties
{
    while(n--)*s1++=*s2++;
}
#endif


///////////////////////////UART1//////////////////////////
#if UART1_EN  == 1
/*---------------------------------------------------------
发送部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART1_TX_BUF[UART1_TX_BUF_SIZE];
static u16          UART1_TX_W_ADDR=0;
static u16          UART1_TX_R_ADDR=0;
static bit          UART1_TX_IDLE=1;

/*---------------------------------------------------------
接收部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART1_RX_BUF[UART1_RX_BUF_SIZE];
static u16          UART1_RX_W_ADDR=0;
static u16          UART1_RX_R_ADDR=0;


/*---------------------------------------------------------
串口中断函数
----------------------------------------------------------*/
void UART1_ISR_Handler (void) interrupt UART1_VECTOR
{
    if(TI){
        TI=0;
        if(UART1_TX_W_ADDR!=UART1_TX_R_ADDR){
            SBUF=UART1_TX_BUF[UART1_TX_R_ADDR++];
            if(UART1_TX_R_ADDR>=UART1_TX_BUF_SIZE)UART1_TX_R_ADDR=0;
        }else{
            UART1_TX_IDLE=1; 
            UART1_set485RxMode();
        }
    }
    if(RI){
        RI=0;
        UART1_RX_BUF[UART1_RX_W_ADDR++]=SBUF;
        if(UART1_RX_W_ADDR>=UART1_RX_BUF_SIZE)UART1_RX_W_ADDR=0;
    }
}

/*---------------------------------------------------------
返回发送区容量
----------------------------------------------------------*/
u16 UART1_GetCapacity( void )FunctionProperties
{
    u16 tx_r_addr;
    OS_ENTER_CRITICAL();
    tx_r_addr = UART1_TX_R_ADDR;
    OS_EXIT_CRITICAL();
    return ( (UART1_TX_W_ADDR>=tx_r_addr)? UART1_TX_BUF_SIZE-(UART1_TX_W_ADDR-tx_r_addr)-1 : tx_r_addr-UART1_TX_W_ADDR-1);
}

/*---------------------------------------------------------
串口发送数据,缓冲区容量不足返回1
----------------------------------------------------------*/
u8 UART1_Send( void *pt, u16 Size)FunctionProperties
{    
    u8 *buf = pt;
    u16 tx_w_addr;
    
    if(UART1_GetCapacity()<Size) return 1;
    
    tx_w_addr = UART1_TX_W_ADDR;
    
    if((tx_w_addr+Size)<UART1_TX_BUF_SIZE){
        memcopy(&UART1_TX_BUF[tx_w_addr],buf,Size);
        tx_w_addr+=Size;
    }else{
        u16 len = UART1_TX_BUF_SIZE-tx_w_addr;
        memcopy(&UART1_TX_BUF[tx_w_addr],buf,len);
        buf+=len;
        len=Size-len;
        memcopy(&UART1_TX_BUF[0],buf,len);
        tx_w_addr=len;
    }
    
    OS_ENTER_CRITICAL();
    UART1_TX_W_ADDR = tx_w_addr;
    if(UART1_TX_IDLE){
        if(UART1_TX_W_ADDR!=UART1_TX_R_ADDR){
            UART1_set485TxMode();
            UART1_TX_IDLE=0;
            SBUF=UART1_TX_BUF[UART1_TX_R_ADDR++];
            if(UART1_TX_R_ADDR>=UART1_TX_BUF_SIZE)UART1_TX_R_ADDR=0;
        }
    }
    OS_EXIT_CRITICAL();
        
    return 0;
}

/*---------------------------------------------------------
读串口数据
----------------------------------------------------------*/
u16 UART1_Receive(u8 *buf, u16 Size)FunctionProperties
{
    u16 rx_w_addr,nu=0;
    OS_ENTER_CRITICAL();
    rx_w_addr=UART1_RX_W_ADDR;
    OS_EXIT_CRITICAL();
    nu = (rx_w_addr>=UART1_RX_R_ADDR)? rx_w_addr-UART1_RX_R_ADDR : rx_w_addr+UART1_RX_BUF_SIZE-UART1_RX_R_ADDR;
    nu = (nu<Size)? nu:Size;
    if((UART1_RX_R_ADDR+nu)<UART1_RX_BUF_SIZE){
        memcopy(buf,&UART1_RX_BUF[UART1_RX_R_ADDR],nu);
        UART1_RX_R_ADDR+=nu;
    }else{
        u16 len = UART1_RX_BUF_SIZE-UART1_RX_R_ADDR;
        memcopy(buf,&UART1_RX_BUF[UART1_RX_R_ADDR],len);
        buf+=len;
        len=nu-len;
        memcopy(buf,&UART1_RX_BUF[0],len);
        UART1_RX_R_ADDR=len;
    }
    return nu;
}

/*---------------------------------------------------------
UART1_setBRT
----------------------------------------------------------*/
void UART1_setBRT( u32 BRT, u8 brt_Timer )FunctionProperties
{
    if(brt_Timer==UART1_BRT_Timer1){
        TL1 = BTL_LOAD;
        TH1 = BTL_LOAD>>8;
    }else{
        T2L = BTL_LOAD;
        T2H = BTL_LOAD>>8;
    }
}

/*---------------------------------------------------------
UART1_Init
----------------------------------------------------------*/
void UART1_Init(u32 BRT, u8 brt_Timer, u8 sw)
{
    UART1_SW(sw);
    SCON = 0x50;
    if(brt_Timer==UART1_BRT_Timer1){
        AUXR |= 0x40;
        AUXR &= 0xFE;
        TMOD &= 0x0F;
        TL1 = BTL_LOAD;
        TH1 = BTL_LOAD>>8;
        TR1 = 1;
    }else{
        AUXR |= 0x01;
        AUXR |= 0x04;
        T2L = BTL_LOAD;
        T2H = BTL_LOAD>>8;
        AUXR |= 0x10;
    }
    UART1_Priority(Priority_2);
    UART1_Interrupt(1);
}
#endif


///////////////////////////UART2//////////////////////////
#if UART2_EN  == 1
/*---------------------------------------------------------
发送部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART2_TX_BUF[UART2_TX_BUF_SIZE];
static u16          UART2_TX_W_ADDR=0;
static u16          UART2_TX_R_ADDR=0;
static bit          UART2_TX_IDLE=1;

/*---------------------------------------------------------
接收部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART2_RX_BUF[UART2_RX_BUF_SIZE];
static u16          UART2_RX_W_ADDR=0;
static u16          UART2_RX_R_ADDR=0;


/*---------------------------------------------------------
串口中断函数
----------------------------------------------------------*/
void UART2_ISR_Handler (void) interrupt UART2_VECTOR
{
    if(TI2){
        CLR_TI2();
        if(UART2_TX_W_ADDR!=UART2_TX_R_ADDR){
            S2BUF=UART2_TX_BUF[UART2_TX_R_ADDR++];
            if(UART2_TX_R_ADDR>=UART2_TX_BUF_SIZE)UART2_TX_R_ADDR=0;
        }else{
            UART2_TX_IDLE=1; 
            UART2_set485RxMode();
        }
    }
    if(RI2){
        CLR_RI2();
        UART2_RX_BUF[UART2_RX_W_ADDR++]=S2BUF;
        if(UART2_RX_W_ADDR>=UART2_RX_BUF_SIZE)UART2_RX_W_ADDR=0;
    }
}

/*---------------------------------------------------------
返回发送区容量
----------------------------------------------------------*/
u16 UART2_GetCapacity( void )FunctionProperties
{
    u16 tx_r_addr;
    OS_ENTER_CRITICAL();
    tx_r_addr = UART2_TX_R_ADDR;
    OS_EXIT_CRITICAL();
    return ( (UART2_TX_W_ADDR>=tx_r_addr)? UART2_TX_BUF_SIZE-(UART2_TX_W_ADDR-tx_r_addr)-1 : tx_r_addr-UART2_TX_W_ADDR-1);
}

/*---------------------------------------------------------
串口发送数据,缓冲区容量不足返回1
----------------------------------------------------------*/
u8 UART2_Send( void *pt, u16 Size)FunctionProperties
{    
    u8 *buf = pt;
    u16 tx_w_addr;
    
    if(UART2_GetCapacity()<Size) return 1;
    
    tx_w_addr = UART2_TX_W_ADDR;
    
    if((tx_w_addr+Size)<UART2_TX_BUF_SIZE){
        memcopy(&UART2_TX_BUF[tx_w_addr],buf,Size);
        tx_w_addr+=Size;
    }else{
        u16 len = UART2_TX_BUF_SIZE-tx_w_addr;
        memcopy(&UART2_TX_BUF[tx_w_addr],buf,len);
        buf+=len;
        len=Size-len;
        memcopy(&UART2_TX_BUF[0],buf,len);
        tx_w_addr=len;
    }
    
    OS_ENTER_CRITICAL();
    UART2_TX_W_ADDR = tx_w_addr;
    if(UART2_TX_IDLE){
        if(UART2_TX_W_ADDR!=UART2_TX_R_ADDR){
            UART2_set485TxMode();
            UART2_TX_IDLE=0;
            S2BUF=UART2_TX_BUF[UART2_TX_R_ADDR++];
            if(UART2_TX_R_ADDR>=UART2_TX_BUF_SIZE)UART2_TX_R_ADDR=0;
        }
    }
    OS_EXIT_CRITICAL();
        
    return 0;
}

/*---------------------------------------------------------
读串口数据
----------------------------------------------------------*/
u16 UART2_Receive(u8 *buf, u16 Size)FunctionProperties
{
    u16 rx_w_addr,nu=0;
    OS_ENTER_CRITICAL();
    rx_w_addr=UART2_RX_W_ADDR;
    OS_EXIT_CRITICAL();
    nu = (rx_w_addr>=UART2_RX_R_ADDR)? rx_w_addr-UART2_RX_R_ADDR : rx_w_addr+UART2_RX_BUF_SIZE-UART2_RX_R_ADDR;
    nu = (nu<Size)? nu:Size;
    if((UART2_RX_R_ADDR+nu)<UART2_RX_BUF_SIZE){
        memcopy(buf,&UART2_RX_BUF[UART2_RX_R_ADDR],nu);
        UART2_RX_R_ADDR+=nu;
    }else{
        u16 len = UART2_RX_BUF_SIZE-UART2_RX_R_ADDR;
        memcopy(buf,&UART2_RX_BUF[UART2_RX_R_ADDR],len);
        buf+=len;
        len=nu-len;
        memcopy(buf,&UART2_RX_BUF[0],len);
        UART2_RX_R_ADDR=len;
    }
    return nu;
}

/*---------------------------------------------------------
UART2_setBRT
----------------------------------------------------------*/
void UART2_setBRT( u32 BRT )FunctionProperties
{
    T2L = BTL_LOAD;
    T2H = BTL_LOAD>>8;
}

/*---------------------------------------------------------
UART2_Init
----------------------------------------------------------*/
void UART2_Init(u32 BRT, u8 sw)
{
    UART2_SW(sw);
    
    S2CON = 0x50;
    AUXR |= 0x04;
    T2L = BTL_LOAD;
    T2H = BTL_LOAD>>8;
    AUXR |= 0x10;
    
    UART2_Priority(Priority_2);
    UART2_Interrupt(1);
}
#endif


///////////////////////////UART3//////////////////////////
#if UART3_EN  == 1
/*---------------------------------------------------------
发送部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART3_TX_BUF[UART3_TX_BUF_SIZE];
static u16          UART3_TX_W_ADDR=0;
static u16          UART3_TX_R_ADDR=0;
static bit          UART3_TX_IDLE=1;

/*---------------------------------------------------------
接收部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART3_RX_BUF[UART3_RX_BUF_SIZE];
static u16          UART3_RX_W_ADDR=0;
static u16          UART3_RX_R_ADDR=0;


/*---------------------------------------------------------
串口中断函数
----------------------------------------------------------*/
void UART3_ISR_Handler (void) interrupt UART3_VECTOR
{
    if(TI3){
        CLR_TI3();
        if(UART3_TX_W_ADDR!=UART3_TX_R_ADDR){
            S3BUF=UART3_TX_BUF[UART3_TX_R_ADDR++];
            if(UART3_TX_R_ADDR>=UART3_TX_BUF_SIZE)UART3_TX_R_ADDR=0;
        }else{
            UART3_TX_IDLE=1; 
            UART3_set485RxMode();
        }
    }
    if(RI3){
        CLR_RI3();
        UART3_RX_BUF[UART3_RX_W_ADDR++]=S3BUF;
        if(UART3_RX_W_ADDR>=UART3_RX_BUF_SIZE)UART3_RX_W_ADDR=0;
    }
}

/*---------------------------------------------------------
返回发送区容量
----------------------------------------------------------*/
u16 UART3_GetCapacity( void )FunctionProperties
{
    u16 tx_r_addr;
    OS_ENTER_CRITICAL();
    tx_r_addr = UART3_TX_R_ADDR;
    OS_EXIT_CRITICAL();
    return ( (UART3_TX_W_ADDR>=tx_r_addr)? UART3_TX_BUF_SIZE-(UART3_TX_W_ADDR-tx_r_addr)-1 : tx_r_addr-UART3_TX_W_ADDR-1);
}

/*---------------------------------------------------------
串口发送数据,缓冲区容量不足返回1
----------------------------------------------------------*/
u8 UART3_Send( void *pt, u16 Size)FunctionProperties
{    
    u8 *buf = pt;
    u16 tx_w_addr;
    
    if(UART3_GetCapacity()<Size) return 1;
    
    tx_w_addr = UART3_TX_W_ADDR;
    
    if((tx_w_addr+Size)<UART3_TX_BUF_SIZE){
        memcopy(&UART3_TX_BUF[tx_w_addr],buf,Size);
        tx_w_addr+=Size;
    }else{
        u16 len = UART3_TX_BUF_SIZE-tx_w_addr;
        memcopy(&UART3_TX_BUF[tx_w_addr],buf,len);
        buf+=len;
        len=Size-len;
        memcopy(&UART3_TX_BUF[0],buf,len);
        tx_w_addr=len;
    }
    
    OS_ENTER_CRITICAL();
    UART3_TX_W_ADDR = tx_w_addr;
    if(UART3_TX_IDLE){
        if(UART3_TX_W_ADDR!=UART3_TX_R_ADDR){
            UART3_set485TxMode();
            UART3_TX_IDLE=0;
            S3BUF=UART3_TX_BUF[UART3_TX_R_ADDR++];
            if(UART3_TX_R_ADDR>=UART3_TX_BUF_SIZE)UART3_TX_R_ADDR=0;
        }
    }
    OS_EXIT_CRITICAL();
        
    return 0;
}

/*---------------------------------------------------------
读串口数据
----------------------------------------------------------*/
u16 UART3_Receive(u8 *buf, u16 Size)FunctionProperties
{
    u16 rx_w_addr,nu=0;
    OS_ENTER_CRITICAL();
    rx_w_addr=UART3_RX_W_ADDR;
    OS_EXIT_CRITICAL();
    nu = (rx_w_addr>=UART3_RX_R_ADDR)? rx_w_addr-UART3_RX_R_ADDR : rx_w_addr+UART3_RX_BUF_SIZE-UART3_RX_R_ADDR;
    nu = (nu<Size)? nu:Size;
    if((UART3_RX_R_ADDR+nu)<UART3_RX_BUF_SIZE){
        memcopy(buf,&UART3_RX_BUF[UART3_RX_R_ADDR],nu);
        UART3_RX_R_ADDR+=nu;
    }else{
        u16 len = UART3_RX_BUF_SIZE-UART3_RX_R_ADDR;
        memcopy(buf,&UART3_RX_BUF[UART3_RX_R_ADDR],len);
        buf+=len;
        len=nu-len;
        memcopy(buf,&UART3_RX_BUF[0],len);
        UART3_RX_R_ADDR=len;
    }
    return nu;
}

/*---------------------------------------------------------
UART3_setBRT
----------------------------------------------------------*/
void UART3_setBRT( u32 BRT, u8 brt_Timer )FunctionProperties
{
    if(brt_Timer==UART3_BRT_Timer3){
        TL1 = BTL_LOAD;
        TH1 = BTL_LOAD>>8;
    }else{
        T2L = BTL_LOAD;
        T2H = BTL_LOAD>>8;
    }
}

/*---------------------------------------------------------
UART3_Init
----------------------------------------------------------*/
void UART3_Init(u32 BRT, u8 brt_Timer, u8 sw)
{
    UART3_SW(sw);
    S3CON = 0x10;
    if(brt_Timer==UART3_BRT_Timer3){
        S3CON |= 0x40;
        T4T3M |= 0x02;
        T3L = BTL_LOAD;
        T3H = BTL_LOAD>>8;
        T4T3M |= 0x08;
    }else{
        S3CON &= 0xBF;
        AUXR |= 0x04;
        T2L = BTL_LOAD;
        T2H = BTL_LOAD>>8;
        AUXR |= 0x10;
    }
    UART3_Priority(Priority_2);
    UART3_Interrupt(1);
}
#endif


///////////////////////////UART4//////////////////////////
#if UART4_EN  == 1
/*---------------------------------------------------------
发送部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART4_TX_BUF[UART4_TX_BUF_SIZE];
static u16          UART4_TX_W_ADDR=0;
static u16          UART4_TX_R_ADDR=0;
static bit          UART4_TX_IDLE=1;

/*---------------------------------------------------------
接收部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART4_RX_BUF[UART4_RX_BUF_SIZE];
static u16          UART4_RX_W_ADDR=0;
static u16          UART4_RX_R_ADDR=0;


/*---------------------------------------------------------
串口中断函数
----------------------------------------------------------*/
void UART4_ISR_Handler (void) interrupt UART4_VECTOR
{
    if(TI4){
        CLR_TI4();
        if(UART4_TX_W_ADDR!=UART4_TX_R_ADDR){
            S4BUF=UART4_TX_BUF[UART4_TX_R_ADDR++];
            if(UART4_TX_R_ADDR>=UART4_TX_BUF_SIZE)UART4_TX_R_ADDR=0;
        }else{
            UART4_TX_IDLE=1; 
            UART4_set485RxMode();
        }
    }
    if(RI4){
        CLR_RI4();
        UART4_RX_BUF[UART4_RX_W_ADDR++]=S4BUF;
        if(UART4_RX_W_ADDR>=UART4_RX_BUF_SIZE)UART4_RX_W_ADDR=0;
    }
}

/*---------------------------------------------------------
返回发送区容量
----------------------------------------------------------*/
u16 UART4_GetCapacity( void )FunctionProperties
{
    u16 tx_r_addr;
    OS_ENTER_CRITICAL();
    tx_r_addr = UART4_TX_R_ADDR;
    OS_EXIT_CRITICAL();
    return ( (UART4_TX_W_ADDR>=tx_r_addr)? UART4_TX_BUF_SIZE-(UART4_TX_W_ADDR-tx_r_addr)-1 : tx_r_addr-UART4_TX_W_ADDR-1);
}

/*---------------------------------------------------------
串口发送数据,缓冲区容量不足返回1
----------------------------------------------------------*/
u8 UART4_Send( void *pt, u16 Size)FunctionProperties
{    
    u8 *buf = pt;
    u16 tx_w_addr;
    
    if(UART4_GetCapacity()<Size) return 1;
    
    tx_w_addr = UART4_TX_W_ADDR;
    
    if((tx_w_addr+Size)<UART4_TX_BUF_SIZE){
        memcopy(&UART4_TX_BUF[tx_w_addr],buf,Size);
        tx_w_addr+=Size;
    }else{
        u16 len = UART4_TX_BUF_SIZE-tx_w_addr;
        memcopy(&UART4_TX_BUF[tx_w_addr],buf,len);
        buf+=len;
        len=Size-len;
        memcopy(&UART4_TX_BUF[0],buf,len);
        tx_w_addr=len;
    }
    
    OS_ENTER_CRITICAL();
    UART4_TX_W_ADDR = tx_w_addr;
    if(UART4_TX_IDLE){
        if(UART4_TX_W_ADDR!=UART4_TX_R_ADDR){
            UART4_set485TxMode();
            UART4_TX_IDLE=0;
            S4BUF=UART4_TX_BUF[UART4_TX_R_ADDR++];
            if(UART4_TX_R_ADDR>=UART4_TX_BUF_SIZE)UART4_TX_R_ADDR=0;
        }
    }
    OS_EXIT_CRITICAL();
        
    return 0;
}

/*---------------------------------------------------------
读串口数据
----------------------------------------------------------*/
u16 UART4_Receive(u8 *buf, u16 Size)FunctionProperties
{
    u16 rx_w_addr,nu=0;
    OS_ENTER_CRITICAL();
    rx_w_addr=UART4_RX_W_ADDR;
    OS_EXIT_CRITICAL();
    nu = (rx_w_addr>=UART4_RX_R_ADDR)? rx_w_addr-UART4_RX_R_ADDR : rx_w_addr+UART4_RX_BUF_SIZE-UART4_RX_R_ADDR;
    nu = (nu<Size)? nu:Size;
    if((UART4_RX_R_ADDR+nu)<UART4_RX_BUF_SIZE){
        memcopy(buf,&UART4_RX_BUF[UART4_RX_R_ADDR],nu);
        UART4_RX_R_ADDR+=nu;
    }else{
        u16 len = UART4_RX_BUF_SIZE-UART4_RX_R_ADDR;
        memcopy(buf,&UART4_RX_BUF[UART4_RX_R_ADDR],len);
        buf+=len;
        len=nu-len;
        memcopy(buf,&UART4_RX_BUF[0],len);
        UART4_RX_R_ADDR=len;
    }
    return nu;
}

/*---------------------------------------------------------
UART4_setBRT
----------------------------------------------------------*/
void UART4_setBRT( u32 BRT, u8 brt_Timer )FunctionProperties
{
    if(brt_Timer==UART4_BRT_Timer4){
        TL1 = BTL_LOAD;
        TH1 = BTL_LOAD>>8;
    }else{
        T2L = BTL_LOAD;
        T2H = BTL_LOAD>>8;
    }
}

/*---------------------------------------------------------
UART4_Init
----------------------------------------------------------*/
void UART4_Init(u32 BRT, u8 brt_Timer, u8 sw)
{
    UART4_SW(sw);
    S4CON = 0x10;
    if(brt_Timer==UART4_BRT_Timer4){
        S4CON |= 0x40;
        T4T3M |= 0x20;
        T4L = BTL_LOAD;
        T4H = BTL_LOAD>>8;
        T4T3M |= 0x80;
    }else{
        S4CON &= 0xBF;
        AUXR |= 0x04;
        T2L = BTL_LOAD;
        T2H = BTL_LOAD>>8;
        AUXR |= 0x10;
    }
    UART4_Priority(Priority_2);
    UART4_Interrupt(1);
}
#endif


/*---------------------------------------------------------
putchar
----------------------------------------------------------*/
#if printf_UART == 1
char putchar (char c)  {
    while(UART1_Send(&c,1));
    return c;
}
#endif

#if printf_UART == 2
char putchar (char c)  {
    while(UART2_Send(&c,1));
    return c;
}
#endif

#if printf_UART == 3
char putchar (char c)  {
    while(UART3_Send(&c,1));
    return c;
}
#endif

#if printf_UART == 4
char putchar (char c)  {
    while(UART4_Send(&c,1));
    return c;
}
#endif


/*---------------------------------------------------------
end
----------------------------------------------------------*/

