#include <types.h>
#include <utils.h>
#include <os.h>
#include <device.h>
#include <printk.h>

#ifdef configUSING_SERIAL

#ifdef configUSING_FRAME_TIMEOUT_SOFT
static void serial_tmr_cb(struct soft_timer *st)
{
    struct serial_device *serial = (struct serial_device *)st->data;

    if (serial->parent.owner)
    {
        task_send_signal(serial->parent.owner, SIG_DATA);
    }
}
#endif
static err_t serial_init(struct device *dev)
{
    err_t result = 0;
    struct serial_device *serial = (struct serial_device *)dev;

    /* apply configuration */
    if (serial->ops->cfg) result = serial->ops->cfg(serial, &serial->config);

#ifdef configUSING_FRAME_TIMEOUT_SOFT
    {
        struct soft_timer *st = &serial->rxto;

        st->cb      = serial_tmr_cb;
        st->data    = (ubase_t)serial;
        st->expires = INITIAL_JIFFIES;
    }
#endif

    return result;
}

static err_t serial_open(struct device *dev, uint16_t oflag)
{
    struct serial_device *serial = (struct serial_device *)dev;

    /* get open flags */
    dev->open_flag = oflag & 0xff;

    if (oflag & DEVICE_FLAG_INT_RX)
    {
        chn_init(&serial->rx, serial->config.bufsz);
        dev->open_flag |= DEVICE_FLAG_INT_RX;
        serial->ops->control(serial, DEVICE_CTRL_SET_INT, NULL);
    }

    if (oflag & DEVICE_FLAG_INT_TX)
    {
        chn_init(&serial->tx, serial->config.bufsz);
        dev->open_flag |= DEVICE_FLAG_INT_TX;
    }

    return 0;
}

static err_t serial_close(struct device *dev)
{
    struct serial_device *serial = (struct serial_device *)dev;

    if (dev->open_flag & DEVICE_FLAG_INT_RX)
    {
        dev->open_flag &= ~DEVICE_FLAG_INT_RX;
        serial->ops->control(serial, DEVICE_CTRL_CLR_INT, (void *)0);
    }

    if (dev->open_flag & DEVICE_FLAG_INT_TX)
    {
        dev->open_flag &= ~DEVICE_FLAG_INT_TX;
        serial->ops->control(serial, DEVICE_CTRL_CLR_INT, (void *)0);
    }

    return 0;
}

static size_t serial_read(struct device *dev, off_t pos, void *buffer, size_t size)
{
    struct serial_device *serial = (struct serial_device *)dev;

    if (dev->open_flag & DEVICE_FLAG_INT_RX)
    {
        return chn_get(&serial->rx, buffer, size);
    }

    return 0;
}

static size_t serial_peek(struct device *dev, off_t pos, void *buffer, size_t size)
{
    struct serial_device *serial = (struct serial_device *)dev;

    if (dev->open_flag & DEVICE_FLAG_INT_RX)
    {
        return chn_peek(&serial->rx, buffer, size);
    }

    return 0;
}

static size_t serial_write(struct device *dev, off_t pos, const void *buffer, size_t size)
{
    size_t ret = 0;
    struct serial_device *serial = (struct serial_device *)dev;

    log_d("send data to %s\r\n", dev->parent.name);
    log_arr_d(buffer, size);

    if (dev->open_flag & DEVICE_FLAG_INT_TX)
    {
        ret = chn_put(&serial->tx, buffer, size);
        if (ret > 0)
        {
            serial->ops->control(serial, DEVICE_CTRL_SET_TX_INT, NULL);
        }
    }
    return ret;
}

static err_t serial_control(struct device *dev, uint8_t cmd, void *args)
{
    err_t err = 0;
    struct serial_device *serial = (struct serial_device *)dev;

    switch (cmd)
    {
    case SERIAL_CTRL_GETCFG:
        *(struct serial_configure *)args = serial->config;
        break;
    case SERIAL_CTRL_SETCFG:
    {
        struct serial_configure *pconfig = (struct serial_configure *)args;

        err = serial->ops->cfg(serial, pconfig);
        if (err == 0) serial->config = *pconfig;
    }
    break;

    default:
        err = serial->ops->control(serial, cmd, args);
        break;
    }

    return err;
}

static const struct device_ops serial_ops =
{
    .init  = serial_init,
    .open  = serial_open,
    .close = serial_close,
    .read  = serial_read,
    .peek  = serial_peek,
    .write = serial_write,
    .ctrl  = serial_control,
};

err_t serial_device_register(struct serial_device *serial, const char *name, uint32_t flag, void *data)
{
    struct device *dev = &(serial->parent);

    dev->ops       = &serial_ops;
    dev->user_data = data;

    device_register(dev, name, flag);
    return 0;
}

void serial_device_isr(struct serial_device *dev, int event)
{
    u8 c;

    switch (event & 0xff)
    {
    case SERIAL_EVENT_TX_RDY:
        if (chn_get(&dev->tx, &c, 1))
        {
            dev->ops->putc(dev, c);
        }
        else
        {
            dev->ops->control(dev, DEVICE_CTRL_CLR_TX_INT, NULL);
        }
        break;

    case SERIAL_EVENT_RX_IND:
    {
        int ch = -1;

        while (1)
        {
            ch = dev->ops->getc(dev);
            if (ch == -1) break;

            c = ch;
            chn_put(&dev->rx, &c, 1);
        }

#ifdef configUSING_FRAME_TIMEOUT_SOFT
        {
            struct soft_timer *st = &dev->rxto;
            soft_timer_del(st);
            st->expires = jiffies + configSERIAL_RX_TO;
            soft_timer_add(st);
        }
#endif
        break;
    }
    }
}
#endif

