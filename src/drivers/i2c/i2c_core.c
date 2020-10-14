#include <os.h>
#include <device.h>
#include <printk.h>

#ifdef configUSING_I2C
err_t i2c_bus_device_register(struct i2c_bus_device *bus, const char *bus_name)
{
    err_t res = 0;

    if (bus->timeout == 0) bus->timeout = configHZ;

    res = i2c_bus_device_device_init(bus, bus_name);

    i2c_dbg("I2C bus [%s] registered\n", bus_name);

    return res;
}

struct i2c_bus_device *i2c_bus_device_find(const char *bus_name)
{
    struct i2c_bus_device *bus;

    device_t *dev = device_find(bus_name);
    if (!dev)
    {
        i2c_dbg("I2C bus %s not exist\n", bus_name);

        return NULL;
    }

    bus = (struct i2c_bus_device *)dev->user_data;

    return bus;
}

size_t i2c_transfer(struct i2c_bus_device *bus, struct i2c_msg msgs[], uint32_t num)
{
    size_t ret;

    if (bus->ops->master_xfer)
    {
#ifdef I2C_DEBUG
        for (ret = 0; ret < num; ret++)
        {
            i2c_dbg("msgs[%d] %c, addr=0x%02x, len=%d\n", ret,
                    (msgs[ret].flags & I2C_RD) ? 'R' : 'W',
                    msgs[ret].addr, msgs[ret].len);
        }
#endif
        ret = bus->ops->master_xfer(bus, msgs, num);

        return ret;
    }
    else
    {
        i2c_dbg("I2C bus operation not supported\n");

        return 0;
    }
}

size_t i2c_master_send(struct i2c_bus_device *bus,
                       uint16_t               addr,
                       uint16_t               flags,
                       const uint8_t         *buf,
                       uint32_t               count)
{
    err_t ret;
    struct i2c_msg msg;

    msg.addr  = addr;
    msg.flags = flags & I2C_ADDR_10BIT;
    msg.len   = count;
    msg.buf   = (uint8_t *)buf;

    ret = i2c_transfer(bus, &msg, 1);

    return (ret > 0) ? count : ret;
}

size_t i2c_master_recv(struct i2c_bus_device *bus,
                       uint16_t               addr,
                       uint16_t               flags,
                       uint8_t               *buf,
                       uint32_t               count)
{
    err_t ret;
    struct i2c_msg msg;
    assert(bus != NULL);

    msg.addr   = addr;
    msg.flags  = flags & I2C_ADDR_10BIT;
    msg.flags |= I2C_RD;
    msg.len    = count;
    msg.buf    = buf;

    ret = i2c_transfer(bus, &msg, 1);

    return (ret > 0) ? count : ret;
}

int i2c_core_init(void)
{
    return 0;
}
#endif
