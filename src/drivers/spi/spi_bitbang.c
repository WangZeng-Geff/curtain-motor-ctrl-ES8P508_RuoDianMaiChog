#include <types.h>
#include <drivers/spi/spi.h>
#include <drivers/spi/spi_bitbang.h>

#ifdef configUSING_SPI_BITBANG

static inline void spidelay(struct spi_bitbang_ops *spi)
{
    spi->udelay(spi->delay_us);
}

static inline void active(struct spi_cs_ops *cs_ops, int is_on)
{
    cs_ops->active(is_on);
}

static inline void setsck(struct spi_bitbang_ops *spi, int is_on)
{
    spi->setsck(is_on);
}

static inline void setmosi(struct spi_bitbang_ops *spi, int is_on)
{
    spi->setmosi(is_on);
}

static inline int  getmiso(struct spi_bitbang_ops *spi)
{
    return spi->getmiso();
}
#pragma diag_suppress 61
static uint32_t spi_bitbang_xfer_word(struct spi_bitbang_ops *ops, uint32_t mode, uint32_t word, uint8_t bits)
{
    uint32_t cpol = mode & KN_SPI_CPOL;
    uint32_t cpha = mode & KN_SPI_CPHA;

    uint32_t oldbit = (!(word & (1 << (bits - 1)))) << 31;

    for (word <<= (32 - bits); bits; bits--)
    {
        if (cpha) setsck(ops, !cpol);

        if ((uint32_t)(word & (uint32_t)(1 << 31)) != oldbit)
        {
            setmosi(ops, word & (uint32_t)(1 << 31));
            oldbit = word & (uint32_t)(1 << 31);
        }

        spidelay(ops);

        if (cpha)
        {
            setsck(ops, cpol);
        }
        else
        {
            setsck(ops, !cpol);
        }

        spidelay(ops);

        word <<= 1;
        word |= getmiso(ops);

        if (!cpha) setsck(ops, cpol);
    }
    return word;
}

static uint32_t spi_bitbang_xfer(struct spi_device *dev, struct spi_message *msg)
{
    const uint8_t *send_ptr = msg->send_buf;
    uint8_t *recv_ptr = msg->recv_buf;

    struct spi_bitbang_ops *ops = (struct spi_bitbang_ops *)dev->bus->parent.user_data;
    uint32_t size = msg->len;

    if (msg->cs_take) active((struct spi_cs_ops *)dev->parent.user_data, 1);

    while (size--)
    {
        uint8_t data = 0xFF;

        if (send_ptr) data = *send_ptr++;

        data = spi_bitbang_xfer_word(ops, dev->cfg.mode, data, 8);

        if (recv_ptr) *recv_ptr++ = data;
    }

    if (msg->cs_release) active((struct spi_cs_ops *)dev->parent.user_data, 0);

    return msg->len;
}

static const struct spi_ops bitbang_ops =
{
    .xfer = spi_bitbang_xfer,
};

err_t spi_bit_add_bus(struct spi_bus *bus, const char *bus_name, const struct spi_bitbang_ops *ops)
{
    bus->parent.user_data = (void *)ops;

    return spi_bus_register(bus, bus_name, &bitbang_ops);
}

#endif
