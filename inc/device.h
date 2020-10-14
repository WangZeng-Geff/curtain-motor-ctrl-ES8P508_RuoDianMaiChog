#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <types.h>
#include <config.h>
#include <kfifo.h>
#include "object.h"

/**
 * device (I/O) class type
 */
enum device_class_type
{
    Device_Class_Char = 0,                  /**< character device */
    Device_Class_Block,                     /**< block device */
    Device_Class_SPIBUS,
    Device_Class_SPIDevice,
    Device_Class_I2CBUS,                    /**< i2c device */
    Device_Class_Unknown                    /**< unknown device */
};
/**
 * device flags defitions
 */
#define DEVICE_FLAG_ACTIVATED        0x0001     /* device is activated */
#define DEVICE_FLAG_OPENED           0x0002     /* device is opened */
#define DEVICE_FLAG_FASYNC           0x0100

#define DEVICE_FLAG_INT_RX           0x01       /* INT mode on Rx */
#define DEVICE_FLAG_DMA_RX           0x02       /* DMA mode on Rx */
#define DEVICE_FLAG_INT_TX           0x04       /* INT mode on Tx */
#define DEVICE_FLAG_DMA_TX           0x08       /* DMA mode on Tx */

#define SERIAL_CTRL_GETCFG           0x01
#define SERIAL_CTRL_SETCFG           0x02
#define DEVICE_CTRL_SET_INT          0x10       /* enable receive irq */
#define DEVICE_CTRL_CLR_INT          0x11       /* disable receive irq */
#define DEVICE_CTRL_GET_INT          0x12
#define DEVICE_CTRL_SET_TX_INT       0x13       /* enable transmit irq */
#define DEVICE_CTRL_CLR_TX_INT       0x14       /* disable transmit irq */

#define DEVICE_CTRL_BLK_GETGEOME     0x10       /**< get geometry information   */
#define DEVICE_CTRL_BLK_SYNC         0x11       /**< flush data to block device */
#define DEVICE_CTRL_BLK_ERASE        0x12       /**< erase block on block device */
#define DEVICE_CTRL_BLK_AUTOREFRESH  0x13       /**< block device : enter/exit auto refresh mode */

#define RTC_CTRL_GET_TIME            0x01
#define RTC_CTRL_SET_TIME            0x02
#define RTC_CTRL_SET_DATE            0x03
#define RTC_CTRL_ADD_ALARM            0x04
#define RTC_CTRL_DEL_ALARM            0x05




enum KEY_CTRL
{
    KEY_CTRL_GET_KEY,
};

enum SWITCH_CTRL
{
  CTRL_ON,
	CTRL_OFF,
	CTRL_ON_OFF,
	CTRL_BACKLIGHT,
	CTRL_RGB,
	CTRL_MODE_RGB,
};

enum RELAY_CTRL
{
  CTRL_RELAY_ON,
	CTRL_RELAY_OFF,
	KEY_PRESS,
	KEY_UP,
};

typedef struct device device_t;

struct device_ops
{
    err_t(*init)(device_t *dev);
    err_t(*open)(device_t *dev, uint16_t oflag);
    err_t(*close)(device_t *dev);
    size_t (*read)(device_t *dev, off_t pos, void *buffer, size_t size);
    size_t (*peek)(device_t *dev, off_t pos, void *buffer, size_t size);
    size_t (*write)(device_t *dev, off_t pos, const void *buffer, size_t size);
    err_t(*ctrl)(device_t *dev, uint8_t cmd, void *args);
};

#define ARGS2U80(args) (get_bits((u32)(args), 0,  7))
#define ARGS2U81(args) (get_bits((u32)(args), 8,  15))
#define ARGS2U82(args) (get_bits((u32)(args), 16, 23))
#define ARGS2U83(args) (get_bits((u32)(args), 24, 31))

struct device
{
    struct object  parent;

    u16  flag;
    u16  open_flag;

    u8   ref_count;

    const struct device_ops *ops;

    void  *owner;
    void  *user_data;
};


device_t *device_find(const char *name);
void     device_register(device_t *dev, const char *name, uint16_t flags);
void     device_unregister(device_t *dev);
err_t    device_open(device_t *dev, uint16_t oflag);
err_t    device_close(device_t *dev);
size_t   device_peek(device_t *dev, off_t pos, void *buffer, size_t size);
size_t   device_read(device_t *dev, off_t pos, void *buffer, size_t size);
size_t   device_write(device_t *dev, off_t pos, const void *buffer, size_t size);
err_t    device_ctrl(device_t *dev, uint8_t cmd, void *arg);
void     device_set_owner(device_t *dev, const void *owner);

#ifndef pdMS_TO_TICKS
# define pdMS_TO_TICKS(xTimeInMs) \
    ((time_t)(((time_t)(xTimeInMs)*(time_t)configHZ)/(time_t)1000))
#endif

#ifdef configUSING_SERIAL
#include "drivers/serial.h"
#endif

#ifdef configUSING_KEY
#include "drivers/key.h"
#endif

#ifdef configUSING_SPI
#include "drivers/spi/spi.h"
# ifdef configUSING_SPI_BITBANG
#include "drivers/spi/spi_bitbang.h"
# endif
#endif

#ifdef configUSING_W25Qxx
#include "drivers/flash/spi_flash.h"
#include "drivers/flash/spi_flash_w25qxx.h"
#endif

#ifdef configUSING_W5500
#include "drivers/net/w5500.h"
#endif

#ifdef configUSING_I2C
#include "drivers/i2c/i2c.h"
#include "drivers/i2c/i2c_dev.h"
#include "drivers/i2c/i2c-bit-ops.h"
#endif

#ifdef configUSING_RX8025
#include "drivers/rtc/rx8025.h"
#endif

#ifdef configUSING_AT24C
#include "drivers/eeprom/at24cxx.h"
#endif

#ifdef configUSING_HDC1080
#include "drivers/hdc1080.h"
#endif

#ifdef configUSING_SI7020
#include "drivers/si7020.h"
#endif

#ifdef configUSING_LCD
#include "drivers/lcd.h"
#endif

struct flash_erase_cmd
{
    u32 start, size;
};

#endif /* __DEVICE_H__ */

