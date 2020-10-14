#include <types.h>
#include <bitops.h>
#include <device.h>
#include <utils.h>
#include <printk.h>

/* Register definitions */
#define RX8025_REG_SEC          0x00
#define RX8025_REG_MIN          0x01
#define RX8025_REG_HOUR         0x02
#define RX8025_REG_WDAY         0x03
#define RX8025_REG_MDAY         0x04
#define RX8025_REG_MONTH        0x05
#define RX8025_REG_YEAR         0x06
#define RX8025_REG_DIGOFF       0x07
#define RX8025_REG_ALWMIN       0x08
#define RX8025_REG_ALWHOUR      0x09
#define RX8025_REG_ALWWDAY      0x0a
#define RX8025_REG_ALDMIN       0x0b
#define RX8025_REG_ALDHOUR      0x0c
/* 0x0d is reserved */
#define RX8025_REG_CTRL1        0x0e
#define RX8025_REG_CTRL2        0x0f

#define RX8025_BIT_CTRL1_CT     (7 << 0)
/* 1 Hz periodic level irq */
#define RX8025_BIT_CTRL1_VDET   BIT(0)
#define RX8025_BIT_CTRL1_VLF    BIT(1)


/* Clock precision adjustment */
#define RX8025_ADJ_RESOLUTION   3050 /* in ppb */
#define RX8025_ADJ_DATA_MAX     62
#define RX8025_ADJ_DATA_MIN     -62

#define RX8025_ADDR 0x32

#ifdef USING_RX8025

struct rx8025_data
{
    struct i2c_bus_device *i2c_bus;
};

static struct rx8025_data rx8025_data;

static const uint8_t tmdef[6] = {0x00, 0x00, 0x00, 0x01, 0x01, 0x17};

static int rx8025_read_regs(struct i2c_bus_device *i2c_bus, uint8_t reg, uint8_t *values, size_t length)
{
    struct i2c_msg msgs[2];

    msgs[0].addr  = RX8025_ADDR;
    msgs[0].flags = I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    msgs[1].addr  = RX8025_ADDR;
    msgs[1].flags = I2C_RD; /* Read from slave */
    msgs[1].buf   = (uint8_t *)values;
    msgs[1].len   = length;

    if (i2c_transfer(i2c_bus, msgs, 2) == 2)
        return 0;

    return -EIO;
}

static int rx8025_write_regs(struct i2c_bus_device *i2c_bus, uint8_t reg, uint8_t *values, size_t length)
{
    uint8_t tmp[17];
    struct i2c_msg msgs[1];

    assert(length <= 16);

    tmp[0] = reg;
    memcpy(&tmp[1], values, length);

    msgs[0].addr  = RX8025_ADDR;
    msgs[0].flags = I2C_WR;
    msgs[0].buf   = tmp;
    msgs[0].len   = length + 1;

    if (i2c_transfer(i2c_bus, msgs, 1) == 1)
        return 0;

    return -EIO;
}

static int rx8025_get_time(const struct rx8025_data *rx8025, uint8_t *bcdtm)
{
    uint8_t tm[7] = {0};

    rx8025_read_regs(rx8025->i2c_bus, RX8025_REG_SEC, tm, sizeof(tm));
    memcpy(&bcdtm[0], &tm[0], 3);
    memcpy(&bcdtm[3], &tm[4], 3);

    return 6;
}
static int rx8025_set_time(const struct rx8025_data *rx8025, const uint8_t *bcdtm)
{
    uint8_t tm[16] = {0};

    if (!is_bcd_time_valid(bcdtm))
        bcdtm = tmdef;

    memcpy(&tm[0], &bcdtm[0], 3);
    memcpy(&tm[4], &bcdtm[3], 3);
    //tm[3] = week(bcdtm[5], bcdtm[4], bcdtm[3]);

    tm[15] = 0x60;

    return rx8025_write_regs(rx8025->i2c_bus, 0, tm, sizeof(tm));
}

static err_t rx8025_control(struct device *dev, uint8_t cmd, void *args)
{
    err_t err = 0;
    struct rx8025_data *rx8025 = (struct rx8025_data *)dev->user_data;

    switch (cmd)
    {
    case RTC_CTRL_GET_TIME:
        rx8025_get_time(rx8025, args);
        break;
    case RTC_CTRL_SET_TIME:
        rx8025_set_time(rx8025, args);
        break;
    }

    return err;
}

static const struct device_ops rx8025_ops =
{
    .ctrl = rx8025_control,
};

static struct device rx8025_dev;

static int rx8025_check_validity(const struct rx8025_data *rx8025)
{
    uint8_t val = 0;

    int ret = rx8025_read_regs(rx8025->i2c_bus, RX8025_REG_CTRL2, &val, sizeof(val));
    if (ret < 0) return ret;

    if (val != 0x60)
    {
        rx8025_set_time(rx8025, tmdef);
    }
    return 0;
}

err_t rx8025_init(const char *i2c_bus_name)
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

    rx8025_data.i2c_bus  = i2c_bus;
    rx8025_dev.ops       = &rx8025_ops;
    rx8025_dev.user_data = &rx8025_data;

    rx8025_check_validity(&rx8025_data);

    device_register(&rx8025_dev, "r8025", 0);
    return 0;
}
#endif
