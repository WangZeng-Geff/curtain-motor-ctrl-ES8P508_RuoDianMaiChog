
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
#include "es8p508_led.h"
static struct device led_dev;
#define LED_GRADUAL_TIME		1500 /*(X10ms)*/
#define GPIO_R					GPIOA
#define GPIO_R_PIN				GPIO_Pin_10 //T16N2-0
#define GPIO_R_AF				GPIO_Func_1
#define GPIO_G					GPIOA
#define GPIO_G_PIN				GPIO_Pin_17//T16N1-0
#define GPIO_G_AF				GPIO_Func_3
#define GPIO_B					GPIOA
#define GPIO_B_PIN				GPIO_Pin_18//T16N1-0
#define GPIO_B_AF				GPIO_Func_3
static uint8_t rgb_data[6] =
{
	0xFF, 0x38, 0x00, 0x00, 0xff, 0xff, 
};
struct led_state
{
uint16_t		time_point;
int16_t 		time_total;
uint8_t 		state;
};

struct led_state blackled =
{
0, 0, 1
};
uint8_t RGB_OFF = 0XFF;
/*颜色不对 改下这里就行*/
static void set_rgb_brightness(uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t pwm_period = LED_PERIOD - 1;

	T16Nx_SetMAT0(T16N1, pwm_period - pwm_period / 255 * r); //r
	T16Nx_SetMAT2(T16N1, 2 * pwm_period - pwm_period / 255 * b); //g
	T16Nx_SetMAT0(T16N2, pwm_period - pwm_period / 255 * g); //b
}

static void set_led_brightness(uint8_t brightness)
{
	brightness = brightness < 100 ? brightness: 100;
	brightness = brightness < 25 ? 25: brightness;
	/*氛围灯熄灭*/
	(brightness == 25) ? (set_rgb_brightness(0,0,0)):0;
	uint16_t pwm_period = LED_PERIOD - 1;
	T16Nx_SetMAT0(T16N0, pwm_period - pwm_period / 100 * brightness);
}

inline uint8_t calculate_brightness(int t)
{
	// 三点确定抛物线方程 y=a(t-t0)(t-t1)
	// (t0,y0)=(0,0)   <---->  (0, 0)//禁止修改
	// (t1,y1)=(800,0) <---->  (LED_GRADUAL_TIME, 0)//可以修改
	// (t2,y2)=(400,100) <---->  (LED_GRADUAL_TIME/2, 100)//最高点
	const float t1 = LED_GRADUAL_TIME;
	const float t2 = LED_GRADUAL_TIME / 2, y2 = 100;
	float a = y2 / (t2 * t2 - t1 * t2);

	return a * t * (t - t1);
}

static void led_tmr_cb(struct soft_timer * st)
{
	st->expires = jiffies + configHZ / 10;			//100ms
	soft_timer_add(st);

	if (blackled.time_total >= 0)
	{
		set_led_brightness(calculate_brightness(blackled.time_point));
		blackled.time_point += 10;
		blackled.time_total -= 10;

		if (blackled.time_point >= LED_GRADUAL_TIME) blackled.time_point -= LED_GRADUAL_TIME;
	}
}

static err_t led_ctrl(device_t * dev, uint8_t cmd, void * args)
{
  	rgb_t *rgb_temp  = (rgb_t *)args; 
	uint8_t *mode	= (uint8_t *)args;
	switch (cmd)
	{
		case CTRL_ON:
			blackled.state = 1;
			blackled.time_point = LED_GRADUAL_TIME / 2;
			blackled.time_total = 0; //不会进行调整

		
			break;

		case CTRL_OFF:
			if (blackled.state)
			{
				blackled.time_point = LED_GRADUAL_TIME / 2;
				blackled.time_total = LED_GRADUAL_TIME / 2;
				blackled.state = 0;
			}

			break;

		case CTRL_ON_OFF:
			if (!blackled.state)
			{
				blackled.time_point = 0;
				blackled.time_total = LED_GRADUAL_TIME;
			}
			else 
			{
				blackled.time_point = LED_GRADUAL_TIME / 2;
				blackled.time_total = LED_GRADUAL_TIME;
			}

			break;

		case CTRL_BACKLIGHT:
			if (!blackled.state)
			{
				set_led_brightness(0);
			}
			else 
			{
				set_led_brightness(100);
			}

			break;
			
         case CTRL_RGB:
             set_rgb_brightness(rgb_temp-> r, rgb_temp-> g, rgb_temp-> b);
		 
			break;
		 case CTRL_MODE_RGB:
			 switch(*mode)
			 {
				 case  MODE_COOL:
					 set_rgb_brightness(rgb_data[3],rgb_data[4], rgb_data[5]);
					 break;
				 case  MODE_HEAT:
					 set_rgb_brightness(rgb_data[0],rgb_data[1], rgb_data[2]);
					 break;
				 default:
					 set_rgb_brightness(0,0, 0);
					 break;
			 }
			break;

		default:
			break;
	}

	return 0;
}

static

struct device_ops led_ops =
{
.ctrl = led_ctrl, 
};

static void rgb_gpio_init(void)
{
	TIM_BaseInitStruType x;
	GPIO_InitStruType y;

	y.GPIO_Signal = GPIO_Pin_Signal_Digital;
	y.GPIO_Func = GPIO_R_AF;
	y.GPIO_Direction = GPIO_Dir_Out;
	y.GPIO_PUEN = GPIO_PUE_Input_Disable;
	y.GPIO_PDEN = GPIO_PDE_Input_Disable;
	y.GPIO_OD = GPIO_ODE_Output_Disable;
	y.GPIO_DS = GPIO_DS_Output_Normal;
	GPIO_Init(GPIO_R, GPIO_R_PIN, &y);
	y.GPIO_Func = GPIO_G_AF;
	GPIO_Init(GPIO_G, GPIO_G_PIN, &y);
	y.GPIO_Func = GPIO_B_AF;
	GPIO_Init(GPIO_B, GPIO_B_PIN, &y);
	x.TIM_ClkS = TIM_ClkS_PCLK; 					//选择时钟：Pclk
	x.TIM_SYNC = DISABLE;
	x.TIM_EDGE = TIM_EDGE_Rise;
	x.TIM_Mode = TIM_Mode_PWM;						//设为PWM模式
	T16Nx_BaseInit(T16N1, &x);
	T16Nx_BaseInit(T16N2, &x);
	T16Nx_MAT0Out0Config(T16N1, TIM_Out_High);		//匹配0：输出高
	T16Nx_MAT1Out0Config(T16N1, TIM_Out_Low);		//匹配1：输出低
	T16Nx_MAT2Out1Config(T16N1, TIM_Out_High);		//匹配0：输出高
	T16Nx_MAT3Out1Config(T16N1, TIM_Out_Low);		//匹配1：输出低
	T16Nx_MAT0Out0Config(T16N2, TIM_Out_High);		//匹配0：输出高
	T16Nx_MAT1Out0Config(T16N2, TIM_Out_Low);		//匹配1：输出低
	T16Nx_SetPREMAT(T16N1, configSYS_CLOCK / 1000000 - 1); //预分频1:16,，16MHz->1MHz
	T16Nx_SetCNT(T16N1, 0);
	T16Nx_SetMAT0(T16N1, LED_PERIOD - 1);			//控制占空比
	T16Nx_SetMAT1(T16N1, LED_PERIOD);				//控制频率200us
	T16Nx_SetMAT2(T16N1, LED_PERIOD * 2 - 1);		//控制占空比
	T16Nx_SetMAT3(T16N1, LED_PERIOD * 2);			//控制频率200us MAT0123有优先级 
	T16Nx_SetPREMAT(T16N2, configSYS_CLOCK / 1000000 - 1); //预分频1:16,，16MHz->1MHz
	T16Nx_SetCNT(T16N2, 0);
	T16Nx_SetMAT0(T16N2, LED_PERIOD - 1);		//控制占空比
	T16Nx_SetMAT1(T16N2, LED_PERIOD );			//控制频率200us
	NVIC_Init(NVIC_T16N1_IRQn, NVIC_Priority_1, DISABLE);
	NVIC_Init(NVIC_T16N2_IRQn, NVIC_Priority_1, DISABLE);
	T16Nx_MAT0ITConfig(T16N1, TIM_Go_No);			//匹配1：继续计数
	T16Nx_MAT1ITConfig(T16N1, TIM_Go_No);			//匹配2：继续计数
	T16Nx_MAT2ITConfig(T16N1, TIM_Go_No);			//匹配3：继续计数
	T16Nx_MAT3ITConfig(T16N1, TIM_Clr_Int); 		//匹配4：清零
	T16Nx_MAT0ITConfig(T16N2, TIM_Go_No);			//匹配1：继续计数
	T16Nx_MAT1ITConfig(T16N2, TIM_Clr_Int); 		//匹配2：继续计数
	T16Nx_ITConfig(T16N1, TIM_IT_MAT0, DISABLE);	//可选择是否使能中断
	T16Nx_ITConfig(T16N1, TIM_IT_MAT1, DISABLE);
	T16Nx_ITConfig(T16N1, TIM_IT_MAT2, DISABLE);	//可选择是否使能中断
	T16Nx_ITConfig(T16N1, TIM_IT_MAT3, DISABLE);
	T16Nx_ITConfig(T16N2, TIM_IT_MAT0, DISABLE);	//可选择是否使能中断
	T16Nx_ITConfig(T16N2, TIM_IT_MAT1, DISABLE);
	T16N1_PwmOut0_Enable(); 						//使能端口输出0
	T16N1_PwmOut1_Enable(); 						//使能端口输出1
	T16N2_PwmOut0_Enable(); 						//使能端口输出0
	T16N1_Enable();
	T16N2_Enable();
}

static void led_gpio_init(void)
{
	TIM_BaseInitStruType x;
	GPIO_InitStruType y;

	//T16N0
	y.GPIO_Signal = GPIO_Pin_Signal_Digital;
	y.GPIO_Func = GPIO_Func_2;
	y.GPIO_Direction = GPIO_Dir_Out;
	y.GPIO_PUEN = GPIO_PUE_Input_Disable;
	y.GPIO_PDEN = GPIO_PDE_Input_Disable;
	y.GPIO_OD = GPIO_ODE_Output_Disable;
	y.GPIO_DS = GPIO_DS_Output_Normal;
	GPIO_Init(LED1_PORT, LED1_PIN, &y);
	x.TIM_ClkS = TIM_ClkS_PCLK; 					//选择时钟：Pclk
	x.TIM_SYNC = DISABLE;
	x.TIM_EDGE = TIM_EDGE_Rise;
	x.TIM_Mode = TIM_Mode_PWM;						//设为PWM模式
	T16Nx_BaseInit(T16N0, &x);
	T16Nx_MAT0Out0Config(T16N0, TIM_Out_High);		//匹配0：输出高
	T16Nx_MAT1Out0Config(T16N0, TIM_Out_Low);		//匹配1：输出低
	T16Nx_SetPREMAT(T16N0, configSYS_CLOCK / 1000000 - 1); //预分频1:16,，16MHz->1MHz
	T16Nx_SetCNT(T16N0, 0);
	T16Nx_SetMAT0(T16N0, LED_PERIOD);				//控制占空比
	T16Nx_SetMAT1(T16N0, LED_PERIOD);				//控制频率200us
	NVIC_Init(NVIC_T16N0_IRQn, NVIC_Priority_1, DISABLE);
	T16Nx_MAT0ITConfig(T16N0, TIM_Go_No);			//匹配1：继续计数
	T16Nx_MAT1ITConfig(T16N0, TIM_Clr_Int); 		//匹配2：继续计数
	T16Nx_ITConfig(T16N0, TIM_IT_MAT0, DISABLE);	//可选择是否使能中断
	T16Nx_ITConfig(T16N0, TIM_IT_MAT1, DISABLE);
	T16N0_PwmOut0_Enable(); 						//使能端口输出0
	T16N0_Enable();
}

static

struct soft_timer led_tmr =
{
.expires = INITIAL_JIFFIES + configHZ / 100, 
.cb = led_tmr_cb, 
};

int board_setup_led(void)
{
	led_gpio_init();
	rgb_gpio_init();
	led_dev.ops = &led_ops;
	device_register(&led_dev, "led", 0);
	soft_timer_add(&led_tmr);
	return 0;
}

device_initcall(board_setup_led);
