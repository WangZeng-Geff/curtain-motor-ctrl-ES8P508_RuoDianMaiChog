#include <config.h>
#include <init.h>
#include <board.h>
#include "ES8P508x.h"
#include "lib_config.h"
#include "device.h"
#include <settings.h>
#include "es8p508_key.h"

#ifdef configUSING_KEY

static struct key_device key_dev;

/*
 * keys
 */
typedef struct
{
    GPIO_TYPE       port;
    GPIO_TYPE_PIN   pin;
} per_key_t;

static per_key_t keys[] =
{
	{KEY4_PORT, KEY4_PIN},
	{KEY3_PORT, KEY3_PIN},
	{KEY2_PORT, KEY2_PIN},
  {KEY1_PORT, KEY1_PIN},
	{KEY5_PORT, KEY5_PIN}
};

static bool is_key_press(u8 key_no)
{
	if (key_no >= sizeof(keys)) return false;

    return GPIO_ReadBit(keys[key_no].port, keys[key_no].pin) == RESET;
}

static void setup_key_peri(per_key_t *key)
{
    GPIO_InitStruType y;

	y.GPIO_Signal		= GPIO_Pin_Signal_Digital;
	y.GPIO_Func			= GPIO_Func_0;
	y.GPIO_Direction	= GPIO_Dir_In;
	y.GPIO_PUEN			= GPIO_PUE_Input_Enable;
	y.GPIO_PDEN			= GPIO_PDE_Input_Disable;
	y.GPIO_OD			= GPIO_ODE_Output_Disable;
	y.GPIO_DS			= GPIO_DS_Output_Normal;

    GPIO_Init(key->port, key->pin, &y);
}

static const struct key_ops hr8p506_key_ops =
{
    is_key_press,
};

int board_key_init(void)
{
    int i;

    for (i = 0; i < array_size(keys); i++)
    {
        setup_key_peri(&keys[i]);
        set_bit(key_dev.used, i);
    }

    key_dev.ops    = &hr8p506_key_ops;
    key_device_register(&key_dev, "key", 0, NULL);
	
    return 0;
}
device_initcall(board_key_init);
#endif
