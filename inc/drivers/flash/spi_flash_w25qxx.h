#ifndef SPI_FLASH_W25QXX_H_INCLUDED
#define SPI_FLASH_W25QXX_H_INCLUDED

#include <types.h>

extern err_t w25qxx_init(const char *flash_device_name,
                         const char *spi_device_name);

#endif // SPI_FLASH_W25QXX_H_INCLUDED
