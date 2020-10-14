#include <types.h>
#include <bitops.h>
#include <device.h>
#include <utils.h>
#include <printk.h>


#define AT24C_ADDR 0x50

#ifdef USING_AT24C

struct at24c_data
{
    struct i2c_bus_device *i2c_bus;
};

static struct at24c_data at24c_data;

static int at24c_sequential_read(struct i2c_bus_device *i2c_bus, off_t pos, void *buffer, size_t size)
{
    struct i2c_msg msgs[2];
    uint8_t _pos = (uint8_t)pos;

    msgs[0].addr  = AT24C_ADDR;
    msgs[0].flags = I2C_WR;
    msgs[0].buf   = &_pos;
    msgs[0].len   = 1;

    msgs[1].addr  = AT24C_ADDR;
    msgs[1].flags = I2C_RD;
    msgs[1].buf   = (uint8_t *)buffer;
    msgs[1].len   = size;

    if (i2c_transfer(i2c_bus, msgs, 2) == 2)
        return 0;

    return -EIO;
}

static int at24c_page_write(struct i2c_bus_device *i2c_bus, off_t pos, const void *buffer, size_t size)
{
    uint8_t tmp[9];
    struct i2c_msg msgs[1];

    assert(size <= 8);

    tmp[0] = pos;
    memcpy(&tmp[1], buffer, size);

    msgs[0].addr  = AT24C_ADDR;
    msgs[0].flags = I2C_WR;
    msgs[0].buf   = tmp;
    msgs[0].len   = size + 1;

    if (i2c_transfer(i2c_bus, msgs, 1) == 1)
        return 0;

    return -EIO;
}

static size_t at24c_read(device_t dev, off_t pos, void *buffer, size_t size)
{
    struct at24c_data *at24c = (struct at24c_data *)dev->user_data;

    err_t err = at24c_sequential_read(at24c->i2c_bus, pos, buffer, size);

    return err == 0 ? size : err;
}
static size_t at24c_write(device_t dev, off_t pos, const void *buffer, size_t size)
{
    size_t _size = size, idx = 0, _len;

    struct at24c_data *at24c = (struct at24c_data *)dev->user_data;

    while (_size > 0)
    {
        _len = pos & 0x07;
        _len = 0x08 - _len;
        if (_len > _size)
            _len = _size;
        at24c_page_write(at24c->i2c_bus, pos, (uint8_t *)buffer + idx, _len);
        _size -= _len;
        idx   += _len;
        pos   += _len;
    }

    return size;
}

static const struct device_ops at24c_dev_ops =
{
    .read = at24c_read,
    .write = at24c_write,
};

static struct device at24c_dev;

err_t at24cxx_init(const char *i2c_bus_name)
{
    struct i2c_bus_device *i2c_bus = (struct i2c_bus_device *)device_find(i2c_bus_name);
    if (i2c_bus == NULL)
    {
        log_e(MODULE_APP, "\ni2c_bus %s for cs43l22 not found!\n", i2c_bus_name);
        return -ENOSYS;
    }

    if (device_open(&i2c_bus->parent, NULL) != 0)
    {
        log_e(MODULE_APP, "\ni2c_bus %s for cs43l22 opened failed!\n", i2c_bus_name);
        return -EPERM;
    }

    at24c_data.i2c_bus  = i2c_bus;
    at24c_dev.ops       = &at24c_dev_ops;
    at24c_dev.user_data = &at24c_data;

    device_register(&at24c_dev, "at24c", 0);
    return 0;
}
#endif
