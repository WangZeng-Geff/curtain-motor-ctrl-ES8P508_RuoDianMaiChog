#include <device.h>
#include <printk.h>
#include "drivers/i2c/i2c_dev.h"
#include <syserr.h>

#ifdef configUSING_I2C
static size_t i2c_bus_device_read(device_t *dev,
                                  off_t    pos,
                                  void    *buffer,
                                  size_t   count)
{
    uint16_t addr;
    uint16_t flags;
    struct i2c_bus_device *bus = (struct i2c_bus_device *)dev->user_data;

    assert(bus != NULL);
    assert(buffer != NULL);

    i2c_dbg("I2C bus dev [%s] reading %u bytes.\n", dev->parent.name, count);

    addr = pos & 0xffff;
    flags = (pos >> 16) & 0xffff;

    return i2c_master_recv(bus, addr, flags, buffer, count);
}

static size_t i2c_bus_device_write(device_t *dev,
                                   off_t    pos,
                                   const void *buffer,
                                   size_t   count)
{
    uint16_t addr;
    uint16_t flags;
    struct i2c_bus_device *bus = (struct i2c_bus_device *)dev->user_data;

    assert(bus != NULL);
    assert(buffer != NULL);

    i2c_dbg("I2C bus dev %s writing %u bytes.\n", dev->parent.name, count);

    addr = pos & 0xffff;
    flags = (pos >> 16) & 0xffff;

    return i2c_master_send(bus, addr, flags, buffer, count);
}

static err_t i2c_bus_device_control(device_t *dev,
                                    uint8_t  cmd,
                                    void    *args)
{
    err_t ret;
    struct i2c_priv_data *priv_data;
    struct i2c_bus_device *bus = (struct i2c_bus_device *)dev->user_data;

    assert(bus != NULL);

    switch (cmd)
    {
        /* set 10-bit addr mode */
    case I2C_DEV_CTRL_10BIT:
        bus->flags |= I2C_ADDR_10BIT;
        break;
    case I2C_DEV_CTRL_ADDR:
        bus->addr = *(uint16_t *)args;
        break;
    case I2C_DEV_CTRL_TIMEOUT:
        bus->timeout = *(uint32_t *)args;
        break;
    case I2C_DEV_CTRL_RW:
        priv_data = (struct i2c_priv_data *)args;
        ret = i2c_transfer(bus, priv_data->msgs, priv_data->number);
        if (ret < 0)
        {
            return -EIO;
        }
        break;
    default:
        break;
    }

    return 0;
}

static const struct device_ops i2c_bus_ops =
{
    .read    = i2c_bus_device_read,
    .write   = i2c_bus_device_write,
    .ctrl = i2c_bus_device_control,
};

err_t i2c_bus_device_device_init(struct i2c_bus_device *bus,
                                 const char               *name)
{
    struct device *device;
    assert(bus != NULL);

    device = &bus->parent;

    device->user_data = bus;
    device->ops       = &i2c_bus_ops;
    device_register(device, name, 0);

    return 0;
}
#endif
