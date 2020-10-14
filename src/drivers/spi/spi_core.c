#include <printk.h>
#include <utils.h>
#include <drivers/spi/spi.h>

extern err_t spi_bus_device_init(struct spi_bus *bus, const char *name);
extern err_t spidev_device_init(struct spi_device *dev, const char *name);

err_t spi_bus_register(struct spi_bus       *bus,
                       const char           *name,
                       const struct spi_ops *ops)
{
    err_t result;

    result = spi_bus_device_init(bus, name);
    if (result != 0)
        return result;

    /* set ops */
    bus->ops = ops;
    /* initialize owner */
    bus->owner = NULL;

    return 0;
}

err_t spi_bus_attach_device(struct spi_device *device,
                            const char        *name,
                            const char        *bus_name,
                            void              *user_data)
{
    err_t result;
    device_t *bus;

    /* get physical spi bus */
    bus = device_find(bus_name);
    if (bus)
    {
        device->bus = (struct spi_bus *)bus;

        /* initialize spidev device */
        result = spidev_device_init(device, name);
        if (result != 0)
            return result;

        memset(&device->cfg, 0, sizeof(device->cfg));
        device->parent.user_data = user_data;

        return 0;
    }

    /* not found the host bus */
    return -EINVAL;
}

err_t spi_configure(struct spi_device        *device,
                    struct spi_cfg *cfg)
{
    assert(device != NULL);

    /* set configuration */
    device->cfg.data_width = cfg->data_width;
    device->cfg.mode       = cfg->mode & SPI_MODE_MASK ;
    device->cfg.max_hz     = cfg->max_hz ;

    if (device->bus != NULL)
    {
        if (device->bus->owner == device && device->bus->ops->configure)
        {
            device->bus->ops->configure(device, &device->cfg);
        }
    }

    return 0;
}

err_t spi_send_then_send(struct spi_device *device,
                         const void        *send_buf1,
                         size_t             send_length1,
                         const void        *send_buf2,
                         size_t             send_length2)
{
    err_t result;
    struct spi_message message;

    assert(device != NULL);
    assert(device->bus != NULL);

    {
        if (device->bus->owner != device && device->bus->ops->configure)
        {
            /* not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->cfg);
            if (result == 0)
            {
                /* set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* configure SPI bus failed */
                result = -EINVAL;
                goto __exit;
            }
        }

        /* send data1 */
        message.send_buf   = send_buf1;
        message.recv_buf   = NULL;
        message.len     = send_length1;
        message.cs_take    = 1;
        message.cs_release = 0;
        message.next       = NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = -EINVAL;
            goto __exit;
        }

        /* send data2 */
        message.send_buf   = send_buf2;
        message.recv_buf   = NULL;
        message.len     = send_length2;
        message.cs_take    = 0;
        message.cs_release = 1;
        message.next       = NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = -EINVAL;
            goto __exit;
        }

        result = 0;
    }

__exit:

    return result;
}

err_t spi_send_then_recv(struct spi_device *device,
                         const void        *send_buf,
                         size_t             send_length,
                         void              *recv_buf,
                         size_t             recv_length)
{
    err_t result;
    struct spi_message message;

    assert(device != NULL);
    assert(device->bus != NULL);

    {
        if (device->bus->owner != device && device->bus->ops->configure)
        {
            /* not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->cfg);
            if (result == 0)
            {
                /* set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* configure SPI bus failed */
                result = -EINVAL;
                goto __exit;
            }
        }

        /* send data */
        message.send_buf   = send_buf;
        message.recv_buf   = NULL;
        message.len     = send_length;
        message.cs_take    = 1;
        message.cs_release = 0;
        message.next       = NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = -EINVAL;
            goto __exit;
        }

        /* recv data */
        message.send_buf   = NULL;
        message.recv_buf   = recv_buf;
        message.len     = recv_length;
        message.cs_take    = 0;
        message.cs_release = 1;
        message.next       = NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = -EINVAL;
            goto __exit;
        }

        result = 0;
    }

__exit:

    return result;
}

size_t spi_transfer(struct spi_device *device,
                    const void        *send_buf,
                    void              *recv_buf,
                    size_t             length)
{
    err_t result;
    struct spi_message message;

    assert(device != NULL);
    assert(device->bus != NULL);

    {
        if (device->bus->owner != device && device->bus->ops->configure)
        {
            /* not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->cfg);
            if (result == 0)
            {
                /* set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* configure SPI bus failed */
                result = 0;
                goto __exit;
            }
        }

        /* initial message */
        message.send_buf   = send_buf;
        message.recv_buf   = recv_buf;
        message.len     = length;
        message.cs_take    = 1;
        message.cs_release = 1;
        message.next       = NULL;

        /* transfer message */
        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            goto __exit;
        }
    }

__exit:

    return result;
}

struct spi_message *spi_transfer_message(struct spi_device  *device,
        struct spi_message *message)
{
    err_t result;
    struct spi_message *index;

    assert(device != NULL);

    /* get first message */
    index = message;
    if (index == NULL)
        return index;

    /* configure SPI bus */
    if (device->bus->owner != device && device->bus->ops->configure)
    {
        /* not the same owner as current, re-configure SPI bus */
        result = device->bus->ops->configure(device, &device->cfg);
        if (result == 0)
        {
            /* set SPI bus owner */
            device->bus->owner = device;
        }
        else
        {
            /* configure SPI bus failed */
            result = 0;
            goto __exit;
        }
    }

    /* transmit each SPI message */
    while (index != NULL)
    {
        /* transmit SPI message */
        result = device->bus->ops->xfer(device, index);
        if (result == 0)
        {
            break;
        }

        index = index->next;
    }

__exit:

    return index;
}

err_t spi_take_bus(struct spi_device *device)
{
    err_t result = 0;

    assert(device != NULL);
    assert(device->bus != NULL);

    /* configure SPI bus */
    if (device->bus->owner != device && device->bus->ops->configure)
    {
        /* not the same owner as current, re-configure SPI bus */
        result = device->bus->ops->configure(device, &device->cfg);
        if (result == 0)
        {
            /* set SPI bus owner */
            device->bus->owner = device;
        }
        else
        {
            /* configure SPI bus failed */
            return -EINVAL;
        }
    }

    return result;
}

err_t spi_release_bus(struct spi_device *device)
{
    assert(device != NULL);
    assert(device->bus != NULL);
    assert(device->bus->owner == device);

    return 0;
}

err_t spi_take(struct spi_device *device)
{
    err_t result;
    struct spi_message message;

    assert(device != NULL);
    assert(device->bus != NULL);

    memset(&message, 0, sizeof(message));
    message.cs_take = 1;

    result = device->bus->ops->xfer(device, &message);

    return result;
}

err_t spi_release(struct spi_device *device)
{
    err_t result;
    struct spi_message message;

    assert(device != NULL);
    assert(device->bus != NULL);

    memset(&message, 0, sizeof(message));
    message.cs_release = 1;

    result = device->bus->ops->xfer(device, &message);

    return result;
}
