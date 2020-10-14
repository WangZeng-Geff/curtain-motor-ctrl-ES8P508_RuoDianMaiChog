#include <drivers/spi/spi.h>


static size_t _spi_bus_device_read(device_t *dev,
                                   off_t    pos,
                                   void    *buffer,
                                   size_t   size)
{
    struct spi_bus *bus;

    bus = (struct spi_bus *)dev;
    assert(bus != NULL);
    assert(bus->owner != NULL);

    return spi_transfer(bus->owner, NULL, buffer, size);
}

static size_t _spi_bus_device_write(device_t *dev,
                                    off_t    pos,
                                    const void *buffer,
                                    size_t   size)
{
    struct spi_bus *bus;

    bus = (struct spi_bus *)dev;
    assert(bus != NULL);
    assert(bus->owner != NULL);

    return spi_transfer(bus->owner, buffer, NULL, size);
}

static err_t _spi_bus_device_control(device_t *dev,
                                     uint8_t  cmd,
                                     void    *args)
{
    /* TODO: add control command handle */
    switch (cmd)
    {
    case 0: /* set device */
        break;
    case 1:
        break;
    }

    return 0;
}

static const struct device_ops spi_bus_ops =
{
    .read    = _spi_bus_device_read,
    .write   = _spi_bus_device_write,
    .ctrl = _spi_bus_device_control,
};

err_t spi_bus_device_init(struct spi_bus *bus, const char *name)
{
    struct device *device;
    assert(bus != NULL);

    device = &bus->parent;

    device->ops     = &spi_bus_ops;
    device_register(device, name, 0);
    return 0;
}

/* SPI Dev device interface, compatible with RT-Thread 0.3.x/1.0.x */
static size_t _spidev_device_read(device_t *dev,
                                  off_t    pos,
                                  void       *buffer,
                                  size_t   size)
{
    struct spi_device *device;

    device = (struct spi_device *)dev;
    assert(device != NULL);
    assert(device->bus != NULL);

    return spi_transfer(device, NULL, buffer, size);
}

static size_t _spidev_device_write(device_t *dev,
                                   off_t    pos,
                                   const void *buffer,
                                   size_t   size)
{
    struct spi_device *device;

    device = (struct spi_device *)dev;
    assert(device != NULL);
    assert(device->bus != NULL);

    return spi_transfer(device, buffer, NULL, size);
}

static err_t _spidev_device_control(device_t *dev,
                                    uint8_t  cmd,
                                    void       *args)
{
    switch (cmd)
    {
    case 0: /* set device */
        break;
    case 1:
        break;
    }

    return 0;
}

static const struct device_ops spi_dev_ops =
{
    .read    = _spidev_device_read,
    .write   = _spidev_device_write,
    .ctrl = _spidev_device_control,
};

err_t spidev_device_init(struct spi_device *dev, const char *name)
{
    struct device *device;
    assert(dev != NULL);

    device = &(dev->parent);

    device->ops     = &spi_dev_ops;
    device_register(device, name, 0);
    return 0;
}
