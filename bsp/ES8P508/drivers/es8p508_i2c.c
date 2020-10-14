#include <printk.h>
#include <device.h>
#include <board.h>
#include <init.h>
#include "ES8P508x.h"
#include "lib_config.h"
#include "es8p508_i2c.h"

#ifdef configUSING_I2C

static struct i2c_bus_device es8p508_i2c1, es8p508_i2c2;

#ifdef USING_I2C1
static void i2c1_set_sda(void *data, int32_t state)
{
    if (state)
    {
        GPIO_SetBit(GPIO_PORT_I2C1_SDA, PIN_I2C1_SDA);
    }
    else
    {
        GPIO_ResetBit(GPIO_PORT_I2C1_SDA, PIN_I2C1_SDA);
    }
}
static void i2c1_set_scl(void *data, int32_t state)
{
    if (state == 1)
    {
        GPIO_SetBit(GPIO_PORT_I2C1_SCL, PIN_I2C1_SCL);
    }
    else if (state == 0)
    {
        GPIO_ResetBit(GPIO_PORT_I2C1_SCL, PIN_I2C1_SCL);
    }
}

static int32_t i2c1_get_sda(void *data)
{
    return (int32_t)GPIO_ReadBit(GPIO_PORT_I2C1_SDA, PIN_I2C1_SDA);
}

static int32_t i2c1_get_scl(void *data)
{
    return (int32_t)GPIO_ReadBit(GPIO_PORT_I2C1_SCL, PIN_I2C1_SCL);
}


static const struct i2c_bit_ops es8p508_i2c1_bit_ops =
{
    (void *)0xaa,
    i2c1_set_sda,
    i2c1_set_scl,
    i2c1_get_sda,
    i2c1_get_scl,
    board_udelay,
    100,
    5
};
#endif
#ifdef USING_I2C2
static void i2c2_set_sda(void *data, int32_t state)
{
    if (state)
    {
        GPIO_SetBit(GPIO_PORT_I2C2_SDA, PIN_I2C2_SDA);
    }
    else
    {
        GPIO_ResetBit(GPIO_PORT_I2C2_SDA, PIN_I2C2_SDA);
    }
}
static void i2c2_set_scl(void *data, int32_t state)
{
    if (state == 1)
    {
        GPIO_SetBit(GPIO_PORT_I2C2_SCL, PIN_I2C2_SCL);
    }
    else if (state == 0)
    {
        GPIO_ResetBit(GPIO_PORT_I2C2_SCL, PIN_I2C2_SCL);
    }
}

static int32_t i2c2_get_sda(void *data)
{
    return (int32_t)GPIO_ReadBit(GPIO_PORT_I2C2_SDA, PIN_I2C2_SDA);
}

static int32_t i2c2_get_scl(void *data)
{
    return (int32_t)GPIO_ReadBit(GPIO_PORT_I2C2_SCL, PIN_I2C2_SCL);
}

static const struct i2c_bit_ops es8p508_i2c2_bit_ops =
{
    (void *)0xaa,
    i2c2_set_sda,
    i2c2_set_scl,
    i2c2_get_sda,
    i2c2_get_scl,
    board_udelay,
    20,
    5
};
#endif

static void GPIO_Configuration(void)
{
	GPIO_InitStruType y;
	
	y.GPIO_Signal		= GPIO_Pin_Signal_Digital;
	y.GPIO_Func			= GPIO_Func_0;
	y.GPIO_Direction	= GPIO_Dir_Out;
	y.GPIO_PUEN			= GPIO_PUE_Input_Enable;
	y.GPIO_PDEN			= GPIO_PDE_Input_Disable;
	y.GPIO_OD			= GPIO_ODE_Output_Enable;
	y.GPIO_DS			= GPIO_DS_Output_Normal;
	
	GPIO_Init(GPIO_PORT_I2C1_SCL, PIN_I2C1_SCL, &y);
	GPIO_SetBit(GPIO_PORT_I2C1_SCL, PIN_I2C1_SCL);	
	
	GPIO_Init(GPIO_PORT_I2C1_SDA, PIN_I2C1_SDA, &y);
	GPIO_SetBit(GPIO_PORT_I2C1_SDA, PIN_I2C1_SDA);	
	
		GPIO_Init(GPIO_PORT_I2C2_SCL, PIN_I2C2_SCL, &y);
	GPIO_SetBit(GPIO_PORT_I2C2_SCL, PIN_I2C2_SCL);	
	
	GPIO_Init(GPIO_PORT_I2C2_SDA, PIN_I2C2_SDA, &y);
	GPIO_SetBit(GPIO_PORT_I2C2_SDA, PIN_I2C2_SDA);	

}

int board_i2c_init(void)
{
    GPIO_Configuration();
#ifdef USING_I2C1
    es8p508_i2c1.priv = (void *)&es8p508_i2c1_bit_ops;
    i2c_bit_add_bus(&es8p508_i2c1, "i2c1");
#endif
		
#ifdef USING_I2C2
		es8p508_i2c2.priv = (void *)&es8p508_i2c2_bit_ops;
    i2c_bit_add_bus(&es8p508_i2c2, "i2c2");
#endif
	
    return 0;
}


subsys_initcall(board_i2c_init);

#endif
