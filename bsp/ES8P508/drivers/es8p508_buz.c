
#include <config.h>
#include <board.h>
#include <init.h>
#include <os.h>
#include <bitmap.h>
#include <bitops.h>
#include <settings.h>
#include "ES8P508x.h"
#include "lib_config.h"
#include "device.h"
#include "printk.h"
#include "es8p508_buz.h"
static struct device buz_dev;


static void buz_tmr_cb(struct soft_timer * st)
{
	log_d("buz_clos Task \r\n");
	T16N3_PwmOut1_Disable();
	T16N3_Disable();
}


static


struct soft_timer buz_tmr =
{
.expires = INITIAL_JIFFIES, 
.cb = buz_tmr_cb, 
};


static void buz_gpio_init(void);


static err_t buz_ctrl(device_t * dev, uint8_t cmd, void * args)
{
	switch (cmd)
	{
		case CTRL_ON:
			{
				buz_gpio_init();
				T16N3_PwmOut1_Enable();
				T16N3_Enable();
				buz_tmr.expires = jiffies + configHZ / 5;
				soft_timer_add(&buz_tmr);
			}
			break;
	}

	return 0;
}


static


struct device_ops buz_ops =
{
.ctrl = buz_ctrl, 
};


static void buz_gpio_init(void)
{
	TIM_BaseInitStruType x;
	GPIO_InitStruType y;

	y.GPIO_Signal = GPIO_Pin_Signal_Digital;
	y.GPIO_Func = GPIO_Func_2;
	y.GPIO_Direction = GPIO_Dir_Out;
	y.GPIO_PUEN = GPIO_PUE_Input_Disable;
	y.GPIO_PDEN = GPIO_PDE_Input_Disable;
	y.GPIO_OD = GPIO_ODE_Output_Disable;
	y.GPIO_DS = GPIO_DS_Output_Normal;
	GPIO_Init(BUZ_PORT, BUZ_PIN, &y);
	x.TIM_ClkS = TIM_ClkS_PCLK; 					//选择时钟：Pclk
	x.TIM_SYNC = DISABLE;
	x.TIM_EDGE = TIM_EDGE_Rise;
	x.TIM_Mode = TIM_Mode_PWM;						//设为PWM模式
	T16Nx_BaseInit(T16N3, &x);
	T16Nx_MAT2Out1Config(T16N3, TIM_Out_High);		//匹配2：输出高
	T16Nx_MAT3Out1Config(T16N3, TIM_Out_Low);		//匹配3：输出低
	T16Nx_SetPREMAT(T16N3, configSYS_CLOCK / 2000000 - 1); //预分频1:8,，8MHz->500kHz
	T16Nx_SetMAT2(T16N3, 1000); 					//控制占空比
	T16Nx_SetMAT3(T16N3, 2000); 					//控制频率
	NVIC_Init(NVIC_T16N3_IRQn, NVIC_Priority_1, DISABLE);
	T16Nx_MAT2ITConfig(T16N3, TIM_Go_No);			//匹配2：继续计数
	T16Nx_MAT3ITConfig(T16N3, TIM_Clr_Int); 		//匹配3：清0
	T16Nx_ITConfig(T16N3, TIM_IT_MAT2, DISABLE);	//可选择是否使能中断
	T16Nx_ITConfig(T16N3, TIM_IT_MAT3, DISABLE);
}


// the	T16N3 is used for encode, please check sn and buzz fuc when buz_init location different
int board_setup_buz(void)
{
	buz_dev.ops = &buz_ops;
	device_register(&buz_dev, "buz", 0);
	return 0;
}


device_initcall(board_setup_buz);
