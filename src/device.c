#include <types.h>
#include <utils.h>
#include <os.h>
#include <printk.h>
#include <syserr.h>
#include "list.h"
#include "device.h"

device_t *device_find(const char *name)
{
    return (device_t *)object_find(name);
}

void device_register(device_t *dev, const char *name, uint16_t flags)
{
    object_attach((object_t *)dev, name);
}

void device_unregister(device_t *dev)
{
    object_detach((object_t *)dev);
}

err_t device_open(device_t *dev, uint16_t oflag)
{
    err_t ret = 0;

    const struct device_ops *ops = dev->ops;

    /* if device is not initialized, initialize it. */
    if (!(dev->flag & DEVICE_FLAG_ACTIVATED))
    {
        if (ops->init)
        {
            ret = ops->init(dev);
            if (ret) return ret;
        }

        dev->flag |= DEVICE_FLAG_ACTIVATED;
    }

    if (ops->open) ret = ops->open(dev, oflag);

    if (!ret)
    {
        if (oflag & DEVICE_FLAG_FASYNC)
            dev->owner = task_get_current_task_handle();

        dev->open_flag = oflag;
        dev->flag |= DEVICE_FLAG_OPENED;
        dev->ref_count++;
    }

    return ret;
}

err_t device_close(device_t *dev)
{
    err_t ret = 0;

    const struct device_ops *ops = dev->ops;

    if (dev->ref_count == 0) return -EPERM;

    if (--dev->ref_count != 0) return 0;

    if (ops->close) ret = ops->close(dev);

    if (!ret) dev->flag &= ~DEVICE_FLAG_OPENED;

    return ret;
}

void device_set_owner(device_t *dev, const void *owner)
{
    dev->owner = (void *)owner;
}

size_t device_peek(device_t *dev, off_t pos, void *buffer, size_t size)
{
    const struct device_ops *ops = dev->ops;

    if (ops->peek) return ops->peek(dev, pos, buffer, size);

    return 0;
}

size_t device_read(device_t *dev, off_t pos, void *buffer, size_t size)
{
    const struct device_ops *ops = dev->ops;

    if (ops->read) return ops->read(dev, pos, buffer, size);

    return 0;
}

size_t device_write(device_t *dev, off_t pos, const void *buffer, size_t size)
{
    const struct device_ops *ops = dev->ops;

    if (ops->write) return ops->write(dev, pos, buffer, size);

    return 0;
}

err_t device_ctrl(device_t *dev, uint8_t cmd, void *arg)
{
    const struct device_ops *ops = dev->ops;

    if (ops->ctrl) return ops->ctrl(dev, cmd, arg);

    return 0;
}
