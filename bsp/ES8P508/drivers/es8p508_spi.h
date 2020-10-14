#ifndef _ES8P508_SPI_H_
#define _ES8P508_SPI_H_

#ifdef configUSING_SPI

#include <device.h>

#include "ES8P508x.h"

struct hr8p296_spi_bus
{
    struct spi_bus parent;
    SPI_TypeDef *spi;
};

struct hr8p296_spi_cs
{
    GPIO_TYPE GPIOx;
    GPIO_TYPE_PIN GPIO_Pin;
};

err_t hr8p296_add_spi(void);
#endif
#endif // hr8p296_SPI_H_INCLUDED
