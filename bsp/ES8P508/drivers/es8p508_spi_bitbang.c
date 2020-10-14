#include <device.h>
#include <init.h>
#include "HR8P296.h"
#include "lib_config.h"
#include "hr8p296_spi_bitbang.h"
#include "board.h"

#if defined(configUSING_SPI) && defined(configUSING_SPI_BITBANG)

/*
 * GPIO define
 * SPI1_MOSI: PB3
 * SPI1_MISO: PB1
 * SPI1_SCK : PB2
 * CS0: PB0
 */
#define GPIO_PORT_SPI_CS    GPIOB
#define PIN_SPI_CS          GPIO_Pin_0

#define GPIO_PORT_SPI_SCK   GPIOB
#define PIN_SPI_SCK         GPIO_Pin_2

#define GPIO_PORT_SPI_MOSI  GPIOB
#define PIN_SPI_MOSI        GPIO_Pin_3

#define GPIO_PORT_SPI_MISO  GPIOB
#define PIN_SPI_MISO        GPIO_Pin_1

static struct spi_bus hr8p296_spi;
static struct spi_device bitbang_device;

static void hr8p296_spi_set_cs(int is_on)
{
    if (is_on)
    {
        GPIO_ResetBit(GPIO_PORT_SPI_CS, PIN_SPI_CS);
    }
    else
    {
        GPIO_SetBit(GPIO_PORT_SPI_CS, PIN_SPI_CS);
    }
}
static void hr8p296_spi_set_sck(int is_on)
{
    if (is_on)
    {
        GPIO_SetBit(GPIO_PORT_SPI_SCK, PIN_SPI_SCK);
    }
    else
    {
        GPIO_ResetBit(GPIO_PORT_SPI_SCK, PIN_SPI_SCK);
    }
}
static void hr8p296_spi_set_mosi(int is_on)
{
    if (is_on)
    {
        GPIO_SetBit(GPIO_PORT_SPI_MOSI, PIN_SPI_MOSI);
    }
    else
    {
        GPIO_ResetBit(GPIO_PORT_SPI_MOSI, PIN_SPI_MOSI);
    }
}
static int hr8p296_spi_get_miso(void)
{
    return (int)GPIO_ReadBit(GPIO_PORT_SPI_MISO, PIN_SPI_MISO);
}


static const struct spi_bitbang_ops ops =
{
    .setsck  = hr8p296_spi_set_sck,
    .setmosi = hr8p296_spi_set_mosi,
    .getmiso = hr8p296_spi_get_miso,

    .udelay  = board_udelay,
    .delay_us = 0,
};
static const struct spi_cs_ops cs_ops0 =
{
    .active = hr8p296_spi_set_cs,
};

static void RCC_Configuration(void)
{
}

static void GPIO_Configuration(void)
{
    GPIO_InitStruType x;

    GPIO_RegUnLock();
    x.GPIO_Func = GPIO_Func_0;
    x.GPIO_Direction = GPIO_Dir_Out;
    x.GPIO_PUEN = DISABLE;
    x.GPIO_PDEN = DISABLE;
    x.GPIO_OD   = DISABLE;

    GPIO_Init(GPIO_PORT_SPI_CS,  PIN_SPI_CS,  &x);
    GPIO_Init(GPIO_PORT_SPI_SCK, PIN_SPI_SCK, &x);
    GPIO_Init(GPIO_PORT_SPI_MOSI, PIN_SPI_MOSI, &x);

    GPIO_SetBit(GPIO_PORT_SPI_CS, PIN_SPI_CS);
    GPIO_ResetBit(GPIO_PORT_SPI_SCK, PIN_SPI_SCK);

    x.GPIO_Direction = GPIO_Dir_In;
    x.GPIO_PDEN      = ENABLE;
    GPIO_Init(GPIO_PORT_SPI_MISO, PIN_SPI_MISO, &x);
    GPIO_RegLock();
}

int hw_spi_bitbang_init(void)
{
    RCC_Configuration();
    GPIO_Configuration();

    spi_bit_add_bus(&hr8p296_spi, "spib", &ops);

    spi_bus_attach_device(&bitbang_device, "spib0", "spib", (void *)&cs_ops0);

    return 0;
}
device_initcall(hw_spi_bitbang_init);
#endif
