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
#include "es8p508_relay.h"

static struct device relay_dev;

typedef struct
{
    GPIO_TYPE       port;
    GPIO_TYPE_PIN   pin;
} per_relay_t;
/*修改这里 进行阀门和风继电器的控制*/
static per_relay_t relay_pin[] =
{
  {RELAY1_PORT, 	RELAY1_PIN},      //阀门
	{RELAY2_PORT, 	RELAY2_PIN},      //高
	{RELAY3_PORT, 	RELAY3_PIN},      //中
	{RELAY4_PORT, 	RELAY4_PIN},      //低
};

static void setup_relay_peri(per_relay_t *relay)
{
  GPIO_InitStruType y;
	
	y.GPIO_Signal		= GPIO_Pin_Signal_Digital;
	y.GPIO_Func			= GPIO_Func_0;
	y.GPIO_Direction	= GPIO_Dir_Out;
	y.GPIO_PUEN			= GPIO_PUE_Input_Enable;
	y.GPIO_PDEN			= GPIO_PDE_Input_Disable;
	y.GPIO_OD			= GPIO_ODE_Output_Disable;
	y.GPIO_DS			= GPIO_DS_Output_Normal;
	
    GPIO_Init(relay->port, relay->pin, &y);
}

static void relay_gpio_init(void)
{ 
	int i;
    for (i = 0; i < array_size(relay_pin); i++)
	{
        setup_relay_peri(&relay_pin[i]);
		GPIO_ResetBit(relay_pin[i].port, relay_pin[i].pin);
    }
}

struct relay_map
{
		uint8_t id;
    const char *tip;
		uint8_t ctrl_map;
};

static void relay_on(uint8_t chn)
{
	int i;
	for(i = 0; i < dev_state.relay_num; i++)
	{
		if(tst_bit(chn,i))
		{
			SET_PIN_H(relay_pin[i].port, relay_pin[i].pin);//ON_PIN
			if(i == 0)/*阀门动作次数统计*/
			{
//				setting.dev_infor.opt_cnt[0]++;
				dev_state.param_save = 0x01;
			}
		}
	}
}
static void relay_off(uint8_t chn)
{
	int i;
	for(i = 0; i < dev_state.relay_num; i++)
	{
		if(tst_bit(chn,i))
		{
			SET_PIN_L(relay_pin[i].port, relay_pin[i].pin);
		}
	}
}
static err_t relay_ctrl(device_t *dev, uint8_t cmd, void *args)
{
    uint8_t *chn = (uint8_t *)args;
	
	switch (cmd)
    {
    case CTRL_RELAY_ON:
		relay_on(*chn);
		break;
			case CTRL_RELAY_OFF:
		relay_off(*chn);
		break;
		default:
		break;
    }
    return 0;
}
static struct device_ops relay_ops =
{
	.ctrl = relay_ctrl,
};

int board_setup_relay(void)
{
    dev_state.relay_num =	array_size(relay_pin);
		relay_gpio_init();

    relay_dev.ops = &relay_ops;
    device_register(&relay_dev, "relay", 0);

    return 0;
}
device_initcall(board_setup_relay);

