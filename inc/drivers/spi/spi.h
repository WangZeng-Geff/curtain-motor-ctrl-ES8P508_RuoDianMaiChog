#ifndef __SPI_H__
#define __SPI_H__

#include <utils.h>
#include <types.h>
#include <device.h>
#include <printk.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KN_SPI_CPHA     (1<<0)                             /* bit[0]:CPHA, clock phase */
#define KN_SPI_CPOL     (1<<1)                             /* bit[1]:CPOL, clock polarity */
/**
 * At CPOL=0 the base value of the clock is zero
 *  - For CPHA=0, data are captured on the clock's rising edge (low¡úhigh transition)
 *    and data are propagated on a falling edge (high¡úlow clock transition).
 *  - For CPHA=1, data are captured on the clock's falling edge and data are
 *    propagated on a rising edge.
 * At CPOL=1 the base value of the clock is one (inversion of CPOL=0)
 *  - For CPHA=0, data are captured on clock's falling edge and data are propagated
 *    on a rising edge.
 *  - For CPHA=1, data are captured on clock's rising edge and data are propagated
 *    on a falling edge.
 */
#define SPI_LSB      (0<<2)                             /* bit[2]: 0-LSB */
#define SPI_MSB      (1<<2)                             /* bit[2]: 1-MSB */

#define SPI_MASTER   (0<<3)                             /* SPI master device */
#define SPI_SLAVE    (1<<3)                             /* SPI slave device */

#define SPI_MODE_0       (0 | 0)                        /* CPOL = 0, CPHA = 0 */
#define SPI_MODE_1       (0 | KN_SPI_CPHA)              /* CPOL = 0, CPHA = 1 */
#define SPI_MODE_2       (KN_SPI_CPOL | 0)              /* CPOL = 1, CPHA = 0 */
#define SPI_MODE_3       (KN_SPI_CPOL | KN_SPI_CPHA)    /* CPOL = 1, CPHA = 1 */

#define SPI_MODE_MASK    (KN_SPI_CPHA | KN_SPI_CPOL | SPI_MSB)

#define SPI_CS_HIGH  (1<<4)                             /* Chipselect active high */
#define SPI_NO_CS    (1<<5)                             /* No chipselect */
#define SPI_3WIRE    (1<<6)                             /* SI/SO pin shared */
#define SPI_READY    (1<<7)                             /* Slave pulls low to pause */

/**
 * SPI message structure
 */
struct spi_message
{
    const void *send_buf;
    void *recv_buf;
    size_t len;
    struct spi_message *next;

    unsigned cs_take    : 1;
    unsigned cs_release : 1;
};

/**
 * SPI configuration structure
 */
struct spi_cfg
{
    uint8_t mode;
    uint8_t data_width;
    uint16_t reserved;

    uint32_t max_hz;
};

struct spi_ops;
struct spi_bus
{
    struct device parent;
    const struct spi_ops *ops;

    struct spi_device *owner;
};

/**
 * SPI operators
 */
struct spi_ops
{
    err_t(*configure)(struct spi_device *device, struct spi_cfg *configuration);
    uint32_t (*xfer)(struct spi_device *device, struct spi_message *message);
};

/**
 * SPI Virtual BUS, one device must connected to a virtual BUS
 */
struct spi_device
{
    struct device parent;
    struct spi_bus *bus;

    struct spi_cfg cfg;
};
#define SPI_DEVICE(dev) ((struct spi_device *)(dev))

/* register a SPI bus */
err_t spi_bus_register(struct spi_bus *bus,
                       const char     *name,
                       const struct spi_ops *ops);

/* attach a device on SPI bus */
err_t spi_bus_attach_device(struct spi_device *device,
                            const char *name,
                            const char *bus_name,
                            void       *user_data);

/**
 * This function takes SPI bus.
 *
 * @param device the SPI device attached to SPI bus
 *
 * @return 0 on taken SPI bus successfully. others on taken SPI bus failed.
 */
err_t spi_take_bus(struct spi_device *device);

/**
 * This function releases SPI bus.
 *
 * @param device the SPI device attached to SPI bus
 *
 * @return 0 on release SPI bus successfully.
 */
err_t spi_release_bus(struct spi_device *device);

/**
 * This function take SPI device (takes CS of SPI device).
 *
 * @param device the SPI device attached to SPI bus
 *
 * @return 0 on release SPI bus successfully. others on taken SPI bus failed.
 */
err_t spi_take(struct spi_device *device);

/**
 * This function releases SPI device (releases CS of SPI device).
 *
 * @param device the SPI device attached to SPI bus
 *
 * @return 0 on release SPI device successfully.
 */
err_t spi_release(struct spi_device *device);

/* set configuration on SPI device */
err_t spi_configure(struct spi_device        *device,
                    struct spi_cfg *cfg);

/* send data then receive data from SPI device */
err_t spi_send_then_recv(struct spi_device *device,
                         const void *send_buf,
                         size_t      send_length,
                         void       *recv_buf,
                         size_t      recv_length);

err_t spi_send_then_send(struct spi_device *device,
                         const void *send_buf1,
                         size_t      send_length1,
                         const void *send_buf2,
                         size_t      send_length2);

/**
 * This function transmits data to SPI device.
 *
 * @param device the SPI device attached to SPI bus
 * @param send_buf the buffer to be transmitted to SPI device.
 * @param recv_buf the buffer to save received data from SPI device.
 * @param length the length of transmitted data.
 *
 * @return the actual length of transmitted.
 */
size_t spi_transfer(struct spi_device *device,
                    const void *send_buf,
                    void       *recv_buf,
                    size_t      length);

/**
 * This function transfers a message list to the SPI device.
 *
 * @param device the SPI device attached to SPI bus
 * @param message the message list to be transmitted to SPI device
 *
 * @return NULL if transmits message list successfully,
 *         SPI message which be transmitted failed.
 */
struct spi_message *spi_transfer_message(struct spi_device  *device,
        struct spi_message *message);

static inline size_t spi_recv(struct spi_device *device,
                              void  *recv_buf,
                              size_t length)
{
    return spi_transfer(device, NULL, recv_buf, length);
}

static inline size_t spi_send(struct spi_device *device,
                              const void        *send_buf,
                              size_t             length)
{
    return spi_transfer(device, send_buf, NULL, length);
}

static inline uint8_t spi_sendrecv8(struct spi_device *device,
                                    uint8_t            data)
{
    uint8_t value;

    spi_send_then_recv(device, &data, 1, &value, 1);

    return value;
}

static inline uint16_t spi_sendrecv16(struct spi_device *device,
                                      uint16_t           data)
{
    uint16_t value;

    spi_send_then_recv(device, &data, 2, &value, 2);

    return value;
}

/**
 * This function appends a message to the SPI message list.
 *
 * @param list the SPI message list header.
 * @param message the message pointer to be appended to the message list.
 */
static inline void spi_message_append(struct spi_message *list,
                                      struct spi_message *message)
{
    if (message == NULL)
        return; /* not append */

    while (list->next != NULL)
    {
        list = list->next;
    }

    list->next = message;
    message->next = NULL;
}

#ifdef __cplusplus
}
#endif

#endif
