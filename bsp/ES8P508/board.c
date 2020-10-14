#include <config.h>
#include <utils.h>
#include <os.h>
#include "alloter.h"
#include "lib_config.h"
#include <settings.h>
#include "printk.h"
#include "system_ES8P508x.h"
#include "board.h"
#include "init.h"
#include "device.h"
#include "drivers\es8p508_led.h"

/*
 * setup system clock
 */
static int board_setup_rcc(void)
{
	extern unsigned long SystemCoreClock;

	PLLClock_Config(ENABLE, SCU_PLL_HRC, SCU_PLL_48M, ENABLE);  //PLL使能，倍频到48M作为系统时钟 //SCU_PLL_IN16M
    SystemCoreClock = configSYS_CLOCK;
	
	DeviceClockAllEnable();
    return 0;
}
pure_initcall(board_setup_rcc);

static gpio_t pin_blackled ={LED1_PORT, LED1_PIN};

//dev show start, pin to gpio mode
void dev_show_display_start(void)
{	
	GPIO_InitStruType y;
	
	y.GPIO_Signal		= GPIO_Pin_Signal_Digital;
	y.GPIO_Func			= GPIO_Func_0;
	y.GPIO_Direction	= GPIO_Dir_Out;
	y.GPIO_PUEN			= GPIO_PUE_Input_Disable;
	y.GPIO_PDEN			= GPIO_PDE_Input_Disable;
	y.GPIO_OD			= GPIO_ODE_Output_Disable;
	y.GPIO_DS			= GPIO_DS_Output_Normal;
	
	GPIO_Init(pin_blackled.port, pin_blackled.pin, &y);
	GPIO_ToggleBit(pin_blackled.port, pin_blackled.pin);	
}
//dev show end, pin to pwm mode, be care of the T16NX
void dev_show_display_end(void)
{	
    TIM_BaseInitStruType x;
    GPIO_InitStruType y;

    //T16N0
	y.GPIO_Signal		= GPIO_Pin_Signal_Digital;
	y.GPIO_Func			= GPIO_Func_2;
	y.GPIO_Direction	= GPIO_Dir_Out;
	y.GPIO_PUEN			= GPIO_PUE_Input_Disable;
	y.GPIO_PDEN			= GPIO_PDE_Input_Disable;
	y.GPIO_OD			= GPIO_ODE_Output_Disable;
	y.GPIO_DS			= GPIO_DS_Output_Normal;
	
    GPIO_Init(LED1_PORT, LED1_PIN, &y); 

    x.TIM_ClkS = TIM_ClkS_PCLK;                         //选择时钟：Pclk
    x.TIM_SYNC = DISABLE;
    x.TIM_EDGE = TIM_EDGE_Rise;
    x.TIM_Mode = TIM_Mode_PWM ;                         //设为PWM模式
    T16Nx_BaseInit(T16N0,&x);

    T16Nx_MAT0Out0Config(T16N0,TIM_Out_High);            //匹配0：输出高
    T16Nx_MAT1Out0Config(T16N0,TIM_Out_Low);           //匹配1：输出低

	
    T16Nx_SetPREMAT(T16N0,configSYS_CLOCK / 1000000 - 1); //预分频1:16,，16MHz->1MHz
	  T16Nx_SetCNT(T16N0,0);
    T16Nx_SetMAT0(T16N0,LED_PERIOD);                          //控制占空比
    T16Nx_SetMAT1(T16N0,LED_PERIOD);                          //控制频率200us

    NVIC_Init(NVIC_T16N0_IRQn,NVIC_Priority_1,DISABLE);

    T16Nx_MAT0ITConfig(T16N0,TIM_Go_No);               //匹配1：继续计数
    T16Nx_MAT1ITConfig(T16N0,TIM_Clr_Int);               //匹配2：继续计数


    T16Nx_ITConfig(T16N0,TIM_IT_MAT0,DISABLE);          //可选择是否使能中断
    T16Nx_ITConfig(T16N0,TIM_IT_MAT1,DISABLE);

    T16N0_PwmOut0_Enable();                             //使能端口输出0
    T16N0_Enable();

}
//dev refactory display remind user the device will restart...
void dev_restore_display(void)
{
	GPIO_InitStruType y;

	y.GPIO_Signal		= GPIO_Pin_Signal_Digital;
	y.GPIO_Func			= GPIO_Func_0;
	y.GPIO_Direction	= GPIO_Dir_Out;
	y.GPIO_PUEN			= GPIO_PUE_Input_Disable;
	y.GPIO_PDEN			= GPIO_PDE_Input_Disable;
	y.GPIO_OD			= GPIO_ODE_Output_Disable;
	y.GPIO_DS			= GPIO_DS_Output_Normal;

	GPIO_Init(pin_blackled.port, pin_blackled.pin, &y);
	GPIO_ToggleBit(pin_blackled.port, pin_blackled.pin);	

}

static const gpio_t unused_gpios[] = 
{
	{GPIOA, GPIO_Pin_0}, //{GPIOA, GPIO_Pin_10},{GPIOA, GPIO_Pin_18},{GPIOA, GPIO_Pin_17},
	{GPIOA, GPIO_Pin_24},{GPIOA, GPIO_Pin_25},{GPIOB, GPIO_Pin_11}
};

static void gpio_unused_init(void)
{
    int i;
	GPIO_InitStruType y;

	y.GPIO_Signal		= GPIO_Pin_Signal_Digital;
	y.GPIO_Func			= GPIO_Func_0;
	y.GPIO_Direction	= GPIO_Dir_Out;
	y.GPIO_PUEN			= GPIO_PUE_Input_Disable;
	y.GPIO_PDEN			= GPIO_PDE_Input_Disable;
	y.GPIO_OD			= GPIO_ODE_Output_Enable;
	y.GPIO_DS			= GPIO_DS_Output_Normal;

    for (i = 0; i < array_size(unused_gpios); i++) 
    {
        GPIO_Init(unused_gpios[i].port, unused_gpios[i].pin, &y);
    }
}

static int dummy_latest_init(void)
{
    gpio_unused_init();

    return 0;
}
late_initcall(dummy_latest_init);

/*
 * plc reset setup
 */
#define PLC_RESET_PORT   GPIOA
#define PLC_RESET_PIN    GPIO_Pin_14
static void plc_reset_gpio_init(void)
{
    GPIO_InitStruType y;
	
	y.GPIO_Signal		= GPIO_Pin_Signal_Digital;
	y.GPIO_Func			= GPIO_Func_0;
	y.GPIO_Direction	= GPIO_Dir_Out;
	y.GPIO_PUEN			= GPIO_PUE_Input_Enable;
	y.GPIO_PDEN			= GPIO_PDE_Input_Disable;
	y.GPIO_OD			= GPIO_ODE_Output_Disable;
	y.GPIO_DS			= GPIO_DS_Output_Normal;
	
   GPIO_Init(PLC_RESET_PORT, PLC_RESET_PIN, &y);
}
void plc_reset_set(int state)
{
  if(state)
		GPIO_SetBit(PLC_RESET_PORT, PLC_RESET_PIN);
	else
		GPIO_ResetBit(PLC_RESET_PORT, PLC_RESET_PIN);
}

static int board_setup_plc_reset(void)
{
    plc_reset_gpio_init();
    return 0;
}
device_initcall(board_setup_plc_reset);

static void do_init_call(void)
{
#if defined(__CC_ARM)
    extern initcall_t initcall0init$$Base[];
    extern initcall_t initcall7init$$Limit[];

    initcall_t *fn;

    for (fn = initcall0init$$Base;
            fn < initcall7init$$Limit;
            fn++)
    {
        (*fn)();
    }
#endif
}

/*
 * common delay
 */
void board_udelay(unsigned long us)
{
    volatile u32 t1, t2, delta;

    t1 = SysTick->VAL;
    while (1) 
    {
        t2 = SysTick->VAL;
        delta = t2 < t1 ? (t1 - t2) : (SysTick->LOAD - t2 + t1) ;
        if (delta >= us * (SystemCoreClock/1000000U))
            break;
    }
}

void board_mdelay(unsigned long ms)
{
    while (ms > 5) {
        board_udelay(5000);
        ms -= 5;
        IWDT_Clear();
    }
    board_udelay(ms * 1000);
}

static void reboot_timer_cb(struct soft_timer *st)
{
    NVIC_SystemReset();
}
void board_reboot(unsigned long ms100)
{
    static struct soft_timer reboot_timer = 
    {
        .cb = reboot_timer_cb,
    };

    reboot_timer.expires = jiffies + pdMS_TO_TICKS(ms100*10);
    soft_timer_add(&reboot_timer);
}

static void wdg_init(void)
{
    IWDT_InitStruType x;

    IWDT_RegUnLock();
    x.WDT_Tms = 2000;           //period 2s
    x.WDT_IE  = DISABLE;
    x.WDT_Rst = ENABLE;
    x.WDT_Clock = WDT_CLOCK_WDT;
    IWDT_Init(&x);
    IWDT_Enable();
    IWDT_RegLock();
}

void board_feed_wdg(void)
{
    IWDT_Clear();
}

static void feed_wdg_tmr_cb(struct soft_timer *st)
{
	//log_d("feed_wdg\n");
    board_feed_wdg();
    st->expires += configHZ / 2;
    soft_timer_add(st);
}
static int board_setup_feed_wdg(void)
{
    static struct soft_timer st =
    {
        .expires = INITIAL_JIFFIES,
        .cb      = feed_wdg_tmr_cb,
    };

    soft_timer_add(&st);

    return 0;
}
device_initcall(board_setup_feed_wdg);

void board_setup(void)
{	
    do_init_call();
#ifndef configUSING_DEBUG
    wdg_init();
#endif
}
