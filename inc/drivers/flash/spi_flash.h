#ifndef SPI_FLASH_H__
#define SPI_FLASH_H__

#include <device.h>

struct spi_flash_device
{
    struct device                flash_device;
    struct device_blk_geometry   geometry;
    struct spi_device           *spi_device;
    void                        *user_data;
};

struct spi_flash_erase_cmd
{
    uint32_t start, size;
};

typedef struct spi_flash_device *spi_flash_device_t;

#ifdef USING_MTD_NOR
struct spi_flash_mtd
{
    struct mtd_nor_device        mtd_device;
    struct spi_device           *spi_device;
    struct mutex                 lock;
    void                        *user_data;
};
#endif

#endif
