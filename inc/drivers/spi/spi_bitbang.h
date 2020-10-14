#ifndef __SPI_BITBANG_H__
#define __SPI_BITBANG_H__

#include "types.h"
#include "spi.h"

struct spi_bitbang_ops
{
    void (*setsck)(int is_on);
    void (*setmosi)(int is_on);
    int (*getmiso)(void);

    void (*udelay)(unsigned long us);

    unsigned delay_us;  /* scl and sda line delay */
};

struct spi_cs_ops
{
    void (*active)(int is_on);
};

struct spi_bus;

err_t spi_bit_add_bus(struct spi_bus *bus, const char *bus_name, const struct spi_bitbang_ops *ops);
#endif
