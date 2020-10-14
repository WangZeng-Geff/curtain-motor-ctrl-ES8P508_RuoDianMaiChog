#include <os.h>
#include <device.h>
#include <syserr.h>
#include <printk.h>

#ifdef configUSING_I2C

#ifdef I2C_BIT_DEBUG
#define bit_dbg(fmt, ...)   printk(fmt, ##__VA_ARGS__)
#else
#define bit_dbg(fmt, ...)
#endif

#define SET_SDA(ops, val)   ops->set_sda(ops->data, val)
#define SET_SCL(ops, val)   ops->set_scl(ops->data, val)
#define GET_SDA(ops)        ops->get_sda(ops->data)
#define GET_SCL(ops)        ops->get_scl(ops->data)

static inline void i2c_delay(struct i2c_bit_ops *ops)
{
    ops->udelay((ops->delay_us + 1) >> 1);
}

static inline void i2c_delay2(struct i2c_bit_ops *ops)
{
    ops->udelay(ops->delay_us);
}

#define SDA_L(ops)          SET_SDA(ops, 0)
#define SDA_H(ops)          SET_SDA(ops, 1)
#define SCL_L(ops)          SET_SCL(ops, 0)

/**
 * release scl line, and wait scl line to high.
 */
static err_t SCL_H(struct i2c_bit_ops *ops)
{
    time_t start,tick_now;
    SET_SCL(ops, 1);

    if (!ops->get_scl)
        goto done;

#ifdef USING_FREERTOS
    start = task_get_tick_count();
    while (!GET_SCL(ops))
    {
        tick_now = task_get_tick_count();
        if ((tick_now - start) > ops->timeout)
            return -ETIMEDOUT;
    }
#else
    start = jiffies;
    while (!GET_SCL(ops))
    {
        tick_now = jiffies;
        if ((tick_now - start) > ops->timeout)
            return -ETIMEDOUT;
    }
#endif
#ifdef I2C_BIT_DEBUG
    if (tick_now != start)
    {
        bit_dbg("wait %ld tick for SCL line to go high\n",
                tick_now - start);
    }
#endif

done:
    i2c_delay(ops);

    return 0;
}

static void i2c_start(struct i2c_bit_ops *ops)
{
#ifdef I2C_BIT_DEBUG
    if (ops->get_scl && !GET_SCL(ops))
    {
        bit_dbg("I2C bus error, SCL line low\n");
    }
    if (ops->get_sda && !GET_SDA(ops))
    {
        bit_dbg("I2C bus error, SDA line low\n");
    }
#endif
    SDA_L(ops);
    i2c_delay(ops);
    SCL_L(ops);
}

static void i2c_restart(struct i2c_bit_ops *ops)
{
    SDA_H(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SDA_L(ops);
    i2c_delay(ops);
    SCL_L(ops);
}

static void i2c_stop(struct i2c_bit_ops *ops)
{
    SDA_L(ops);
    i2c_delay(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SDA_H(ops);
    i2c_delay2(ops);
}

static inline bool i2c_waitack(struct i2c_bit_ops *ops)
{
    bool ack;

    SDA_H(ops);
    i2c_delay(ops);

    if (SCL_H(ops) < 0)
    {
        bit_dbg("wait ack timeout\n");

        return -ETIMEDOUT;
    }

    ack = !GET_SDA(ops);    /* ACK : SDA pin is pulled low */
    bit_dbg("%s\n", ack ? "ACK" : "NACK");

    SCL_L(ops);

    return ack;
}

static int32_t i2c_writeb(struct i2c_bus_device *bus, uint8_t data)
{
    int32_t i;
    uint8_t bit;

    struct i2c_bit_ops *ops = bus->priv;

    for (i = 7; i >= 0; i--)
    {
        SCL_L(ops);
        bit = (data >> i) & 1;
        SET_SDA(ops, bit);
        i2c_delay(ops);
        if (SCL_H(ops) < 0)
        {
            bit_dbg("i2c_writeb: 0x%02x, "
                    "wait scl pin high timeout at bit %d\n",
                    data, i);

            return -ETIMEDOUT;
        }
    }
    SCL_L(ops);
    i2c_delay(ops);

    return i2c_waitack(ops);
}

static int32_t i2c_readb(struct i2c_bus_device *bus)
{
    uint8_t i;
    uint8_t data = 0;
    struct i2c_bit_ops *ops = bus->priv;

    SDA_H(ops);
    i2c_delay(ops);
    for (i = 0; i < 8; i++)
    {
        data <<= 1;

        if (SCL_H(ops) < 0)
        {
            bit_dbg("i2c_readb: wait scl pin high "
                    "timeout at bit %d\n", 7 - i);

            return -ETIMEDOUT;
        }

        if (GET_SDA(ops))
            data |= 1;
        SCL_L(ops);
        i2c_delay2(ops);
    }

    return data;
}

static size_t i2c_send_bytes(struct i2c_bus_device *bus,
                             struct i2c_msg        *msg)
{
    int32_t ret;
    size_t bytes = 0;
    const uint8_t *ptr = msg->buf;
    int32_t count = msg->len;
    uint16_t ignore_nack = msg->flags & I2C_IGNORE_NACK;

    while (count > 0)
    {
        ret = i2c_writeb(bus, *ptr);

        if ((ret > 0) || (ignore_nack && (ret == 0)))
        {
            count --;
            ptr ++;
            bytes ++;
        }
        else if (ret == 0)
        {
            i2c_dbg("send bytes: NACK.\n");

            return 0;
        }
        else
        {
            i2c_dbg("send bytes: error %d\n", ret);

            return ret;
        }
    }

    return bytes;
}

static err_t i2c_send_ack_or_nack(struct i2c_bus_device *bus, int ack)
{
    struct i2c_bit_ops *ops = bus->priv;

    if (ack)
        SET_SDA(ops, 0);
    i2c_delay(ops);
    if (SCL_H(ops) < 0)
    {
        bit_dbg("ACK or NACK timeout\n");

        return -ETIMEDOUT;
    }
    SCL_L(ops);

    return 0;
}

static size_t i2c_recv_bytes(struct i2c_bus_device *bus,
                             struct i2c_msg        *msg)
{
    int32_t val;
    int32_t bytes = 0;   /* actual bytes */
    uint8_t *ptr = msg->buf;
    int32_t count = msg->len;
    const uint32_t flags = msg->flags;

    while (count > 0)
    {
        val = i2c_readb(bus);
        if (val >= 0)
        {
            *ptr = val;
            bytes ++;
        }
        else
        {
            break;
        }

        ptr ++;
        count --;

        bit_dbg("recieve bytes: 0x%02x, %s\n",
                val, (flags & I2C_NO_READ_ACK) ?
                "(No ACK/NACK)" : (count ? "ACK" : "NACK"));

        if (!(flags & I2C_NO_READ_ACK))
        {
            val = i2c_send_ack_or_nack(bus, count);
            if (val < 0)
                return val;
        }
    }

    return bytes;
}

static int32_t i2c_send_address(struct i2c_bus_device *bus,
                                uint8_t                addr,
                                int32_t                retries)
{
    struct i2c_bit_ops *ops = bus->priv;
    int32_t i;
    err_t ret = 0;

    for (i = 0; i <= retries; i++)
    {
        ret = i2c_writeb(bus, addr);
        if (ret == 1 || i == retries)
            break;
        bit_dbg("send stop condition\n");
        i2c_stop(ops);
        i2c_delay2(ops);
        bit_dbg("send start condition\n");
        i2c_start(ops);
    }

    return ret;
}

static err_t i2c_bit_send_address(struct i2c_bus_device *bus,
                                  struct i2c_msg        *msg)
{
    uint16_t flags = msg->flags;
    uint16_t ignore_nack = msg->flags & I2C_IGNORE_NACK;
    struct i2c_bit_ops *ops = bus->priv;

    uint8_t addr1, addr2;
    int32_t retries;
    err_t ret;

    retries = ignore_nack ? 0 : bus->retries;

    if (flags & I2C_ADDR_10BIT)
    {
        addr1 = 0xf0 | ((msg->addr >> 7) & 0x06);
        addr2 = msg->addr & 0xff;

        bit_dbg("addr1: %d, addr2: %d\n", addr1, addr2);

        ret = i2c_send_address(bus, addr1, retries);
        if ((ret != 1) && !ignore_nack)
        {
            bit_dbg("NACK: sending first addr\n");

            return -EIO;
        }

        ret = i2c_writeb(bus, addr2);
        if ((ret != 1) && !ignore_nack)
        {
            bit_dbg("NACK: sending second addr\n");

            return -EIO;
        }
        if (flags & I2C_RD)
        {
            bit_dbg("send repeated start condition\n");
            i2c_restart(ops);
            addr1 |= 0x01;
            ret = i2c_send_address(bus, addr1, retries);
            if ((ret != 1) && !ignore_nack)
            {
                bit_dbg("NACK: sending repeated addr\n");

                return -EIO;
            }
        }
    }
    else
    {
        /* 7-bit addr */
        addr1 = msg->addr << 1;
        if (flags & I2C_RD)
            addr1 |= 1;
        ret = i2c_send_address(bus, addr1, retries);
        if ((ret != 1) && !ignore_nack)
            return -EIO;
    }

    return 0;
}

static size_t i2c_bit_xfer(struct i2c_bus_device *bus,
                           struct i2c_msg         msgs[],
                           uint32_t               num)
{
    struct i2c_msg *msg;
    struct i2c_bit_ops *ops = bus->priv;
    int32_t i, ret;
    uint16_t ignore_nack;

    bit_dbg("send start condition\n");
    i2c_start(ops);
    for (i = 0; i < num; i++)
    {
        msg = &msgs[i];
        ignore_nack = msg->flags & I2C_IGNORE_NACK;
        if (!(msg->flags & I2C_NO_START))
        {
            if (i)
            {
                i2c_restart(ops);
            }
            ret = i2c_bit_send_address(bus, msg);
            if ((ret != 0) && !ignore_nack)
            {
                bit_dbg("receive NACK from device addr 0x%02x msg %d\n",
                        msgs[i].addr, i);
                goto out;
            }
        }
        if (msg->flags & I2C_RD)
        {
            ret = i2c_recv_bytes(bus, msg);
            if (ret >= 1)
                bit_dbg("read %d byte%s\n", ret, ret == 1 ? "" : "s");
            if (ret < msg->len)
            {
                if (ret >= 0)
                    ret = -EIO;
                goto out;
            }
        }
        else
        {
            ret = i2c_send_bytes(bus, msg);
            if (ret >= 1)
                bit_dbg("write %d byte%s\n", ret, ret == 1 ? "" : "s");
            if (ret < msg->len)
            {
                if (ret >= 0)
                    ret = -EPERM;
                goto out;
            }
        }
    }
    ret = i;

out:
    bit_dbg("send stop condition\n");
    i2c_stop(ops);

    return ret;
}

static const struct i2c_bus_device_ops i2c_bit_bus_ops =
{
    i2c_bit_xfer,
    NULL,
    NULL
};

err_t i2c_bit_add_bus(struct i2c_bus_device *bus,
                      const char               *bus_name)
{
    bus->ops = &i2c_bit_bus_ops;

    return i2c_bus_device_register(bus, bus_name);
}
#endif
