#include <utils.h>
#include <types.h>
#include <bitops.h>
#include <device.h>
#include <printk.h>
#include "board.h"
#include "os.h"


#define HDC1080_ADDR    0x40

#ifdef USING_HDC1080

struct hdc1080_data
{
    struct i2c_bus_device *i2c_bus;
};

static struct hdc1080_data hdc1080_data;

static int hdc1080_read_reg(struct i2c_bus_device *i2c_bus, uint8_t reg, uint8_t *values, size_t length)
{
    struct i2c_msg msgs[2];

    msgs[0].addr  = HDC1080_ADDR;
    msgs[0].flags = I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    msgs[1].addr  = HDC1080_ADDR;
    msgs[1].flags = I2C_RD; /* Read from slave */
    msgs[1].buf   = (uint8_t *)values;
    msgs[1].len   = length;

    if (i2c_transfer(i2c_bus, msgs, 2) == 2)
        return 0;

    return -EIO;
}

static int hdc1080_write_reg(struct i2c_bus_device *i2c_bus, uint8_t reg, uint8_t *values, size_t length)
{
    uint8_t tmp[0x20];
    struct i2c_msg msgs[1];

    tmp[0] = reg;
    memcpy(&tmp[1], values, length);

    msgs[0].addr  = HDC1080_ADDR;
    msgs[0].flags = I2C_WR;
    msgs[0].buf   = tmp;
    msgs[0].len   = length + 1;

    if (i2c_transfer(i2c_bus, msgs, 1) == 1)
        return 0;

    return -EIO;
}

static int hdc1080_read_data(struct i2c_bus_device *i2c_bus, uint8_t *values, size_t length)
{
    struct i2c_msg msgs[1];

    msgs[0].addr  = HDC1080_ADDR;
    msgs[0].flags = I2C_RD;
    msgs[0].buf   = (uint8_t *)values;
    msgs[0].len   = length;

    if (i2c_transfer(i2c_bus, msgs, 1) == 1)
        return 0;

    return -EIO;
}

static int hdc1080_get_sensor_value(const struct hdc1080_data *hdc1080, uint8_t *val)
{
    uint8_t config[2];
    err_t err = hdc1080_read_reg(hdc1080->i2c_bus, HDC1080_CONFIG, config, 2);
    if (err < 0) return (err);

    uint16_t r = get_be_val(config, 2);
    //uint8_t bat_stat = (r >> 11) & 0x0001;
    r |= HDC1080_RH_RES_14 << 10;
    r |= HDC1080_T_RES_14 << 8;
    r |= 1 << 12;     // mode = 1;
    r |= HDC1080_HEAT_OFF << 13;

    put_be_val(r, config, 2);
    // write config
    err = hdc1080_write_reg(hdc1080->i2c_bus, HDC1080_CONFIG, config, 2);
    if (err < 0) return (err);

    err = hdc1080_write_reg(hdc1080->i2c_bus, HDC1080_TEMPERATURE, NULL, 0);
    if (err < 0) return (err);
    //board_mdelay(20); //bug
    task_delay(20 / portTICK_PERIOD_MS);
    err = hdc1080_read_data(hdc1080->i2c_bus, val, 2);
    if (err < 0) return (err);

    err = hdc1080_write_reg(hdc1080->i2c_bus, HDC1080_HUMIDITY, NULL, 0);
    if (err < 0) return (err);
    //board_mdelay(20);
    task_delay(20 / portTICK_PERIOD_MS);
    err = hdc1080_read_data(hdc1080->i2c_bus, &val[2], 2);
    if (err < 0) return (err);

    return 0;
}
static int hdc1080_get_id(const struct hdc1080_data *hdc1080, uint8_t *dev_id)
{
    err_t err = hdc1080_read_reg(hdc1080->i2c_bus, HDC1080_ID_DEV, dev_id, 2);

    return 0;
}

static err_t hdc1080_control(struct device *dev, uint8_t cmd, void *args)
{
    err_t err = 0;
    struct hdc1080_data *hdc1080 = (struct hdc1080_data *)dev->user_data;

    switch (cmd)
    {
    case HDC1080_CTRL_GET_TH:
        err = hdc1080_get_sensor_value(hdc1080, args);
        break;
    case HDC1080_CTRL_GET_ID:
        err = hdc1080_get_id(hdc1080, args);
        break;
    }

    return err;
}

static const struct device_ops hdc1080_dev_ops =
{
    .ctrl = hdc1080_control,
};

static struct device hdc1080_dev;

err_t hdc1080_init(const char *i2c_bus_name)
{
    struct i2c_bus_device *i2c_bus = (struct i2c_bus_device *)device_find(i2c_bus_name);
    if (i2c_bus == NULL)
    {
        log_e(MODULE_APP, "\ni2c_bus %s for hdc1080 not found!\n", i2c_bus_name);
        return -ENOSYS;
    }

    if (device_open(&i2c_bus->parent, NULL) != 0)
    {
        log_e(MODULE_APP, "\ni2c_bus %s for hdc1080 opened failed!\n", i2c_bus_name);
        return -EIO;
    }

    hdc1080_data.i2c_bus  = i2c_bus;
    hdc1080_dev.ops = &hdc1080_dev_ops;
    hdc1080_dev.user_data = &hdc1080_data;

    device_register(&hdc1080_dev, "h1080", 0);
    return 0;
}
#endif
