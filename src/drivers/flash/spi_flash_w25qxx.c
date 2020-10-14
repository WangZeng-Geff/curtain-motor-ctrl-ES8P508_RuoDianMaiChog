#include <printk.h>
#include <types.h>
#include <device.h>
#include <utils.h>
#include <syserr.h>

#ifdef configUSING_W25Qxx

#define FLASH_DEBUG

#ifdef FLASH_DEBUG
#define FLASH_TRACE         printk
#else
#define FLASH_TRACE(...)
#endif /* #ifdef FLASH_DEBUG */

#define SECTOR_SIZE         4096
#define PAGE_SIZE           256

/* JEDEC Manufacturer's ID */
#define MF_ID           (0x1C)

/* JEDEC Device ID: Memory type and Capacity */
#define MTC_W25Q80_BV         (0x4014) /* W25Q80BV */
#define MTC_EN25Q16A          (0x3015) /* EN25Q16A  */
#define MTC_W25Q16_DW         (0x6015) /* W25Q16DW  */
#define MTC_W25Q32_BV         (0x4016) /* W25Q32BV */
#define MTC_W25Q32_DW         (0x6016) /* W25Q32DW */
#define MTC_W25Q64_BV_CV      (0x4017) /* W25Q64BV W25Q64CV */
#define MTC_W25Q64_DW         (0x4017) /* W25Q64DW */
#define MTC_W25Q128_BV        (0x4018) /* W25Q128BV */
#define MTC_W25Q256_FV        (TBD)    /* W25Q256FV */

/* command list */
#define CMD_WRSR                    (0x01)  /* Write Status Register */
#define CMD_PP                      (0x02)  /* Page Program */
#define CMD_READ                    (0x03)  /* Read Data */
#define CMD_WRDI                    (0x04)  /* Write Disable */
#define CMD_RDSR1                   (0x05)  /* Read Status Register-1 */
#define CMD_WREN                    (0x06)  /* Write Enable */
#define CMD_FAST_READ               (0x0B)  /* Fast Read */
#define CMD_ERASE_4K                (0x20)  /* Sector Erase:4K */
#define CMD_RDSR2                   (0x35)  /* Read Status Register-2 */
#define CMD_ERASE_32K               (0x52)  /* 32KB Block Erase */
#define CMD_JEDEC_ID                (0x9F)  /* Read JEDEC ID */
#define CMD_ERASE_full              (0xC7)  /* Chip Erase */
#define CMD_ERASE_64K               (0xD8)  /* 64KB Block Erase */

#define DUMMY                       (0xFF)

static struct spi_flash_device  spi_flash_device;

static void flash_lock(struct spi_flash_device *flash_device)
{

}

static void flash_unlock(struct spi_flash_device *flash_device)
{

}

static uint8_t w25qxx_read_status(void)
{
    return spi_sendrecv8(spi_flash_device.spi_device, CMD_RDSR1);
}

static void w25qxx_wait_busy(void)
{
    while (w25qxx_read_status() & (0x01));
}

/** \brief read [size] byte from [offset] to [buffer]
 *
 * \param offset uint32_t unit : byte
 * \param buffer uint8_t*
 * \param size uint32_t   unit : byte
 * \return uint32_t byte for read
 *
 */
static uint32_t w25qxx_read(uint32_t offset, uint8_t *buffer, uint32_t size)
{
    uint8_t send_buffer[4];

    send_buffer[0] = CMD_WRDI;
    spi_send(spi_flash_device.spi_device, send_buffer, 1);

    send_buffer[0] = CMD_READ;
    send_buffer[1] = (uint8_t)(offset >> 16);
    send_buffer[2] = (uint8_t)(offset >> 8);
    send_buffer[3] = (uint8_t)(offset);

    spi_send_then_recv(spi_flash_device.spi_device,
                       send_buffer, 4,
                       buffer, size);

    return size;
}

static uint32_t w25qxx_sector_erase(uint32_t page_addr)
{
    uint8_t send_buffer[4];

    send_buffer[0] = CMD_WREN;
    spi_send(spi_flash_device.spi_device, send_buffer, 1);

    send_buffer[0] = CMD_ERASE_4K;
    send_buffer[1] = (page_addr >> 16);
    send_buffer[2] = (page_addr >> 8);
    send_buffer[3] = (page_addr);
    spi_send(spi_flash_device.spi_device, send_buffer, 4);

    w25qxx_wait_busy(); // wait erase done.

    send_buffer[0] = CMD_WRDI;
    spi_send(spi_flash_device.spi_device, send_buffer, 1);

    return SECTOR_SIZE;
}

static uint32_t w25qxx_erase(device_t *dev, off_t pos, size_t size)
{
    size_t _size, _len;

    flash_lock((struct spi_flash_device *)dev);

    _size = size;
    while (_size > 0)
    {
        _len = min(_size, SECTOR_SIZE);
        w25qxx_sector_erase(pos);
        pos += SECTOR_SIZE;
        _size -= _len;
    }

    flash_unlock((struct spi_flash_device *)dev);

    return size;
}

/** \brief write N page on [page]
 *
 * \param page_addr uint32_t unit : byte (4096 * N,1 page = 4096byte)
 * \param buffer const uint8_t*
 * \return uint32_t
 *
 */
uint32_t w25qxx_page_write(uint32_t page_addr, const uint8_t *buffer, size_t size)
{
    uint8_t send_buffer[4];

    send_buffer[0] = CMD_WREN;
    spi_send(spi_flash_device.spi_device, send_buffer, 1);

    send_buffer[0] = CMD_PP;
    send_buffer[1] = (uint8_t)(page_addr >> 16);
    send_buffer[2] = (uint8_t)(page_addr >> 8);
    send_buffer[3] = (uint8_t)(page_addr);

    spi_send_then_send(spi_flash_device.spi_device,
                       send_buffer,
                       4,
                       buffer,
                       size);

    w25qxx_wait_busy();

    send_buffer[0] = CMD_WRDI;
    spi_send(spi_flash_device.spi_device, send_buffer, 1);

    return size;
}

/* RT-Thread device interface */
static err_t w25qxx_flash_init(device_t *dev)
{
    return 0;
}

static err_t w25qxx_flash_open(device_t *dev, uint16_t oflag)
{
    uint8_t send_buffer[3];

    flash_lock((struct spi_flash_device *)dev);

    send_buffer[0] = CMD_WREN;
    spi_send(spi_flash_device.spi_device, send_buffer, 1);

    send_buffer[0] = CMD_WRSR;
    send_buffer[1] = 0;
    send_buffer[2] = 0;
    spi_send(spi_flash_device.spi_device, send_buffer, 3);

    w25qxx_wait_busy();

    flash_unlock((struct spi_flash_device *)dev);

    return 0;
}

static err_t w25qxx_flash_close(device_t *dev)
{
    return 0;
}

static err_t w25qxx_flash_control(device_t *dev, uint8_t cmd, void *args)
{
    assert(dev != NULL);

    if (cmd == DEVICE_CTRL_BLK_GETGEOME)
    {
        struct device_blk_geometry *geometry;

        geometry = (struct device_blk_geometry *)args;
        if (geometry == NULL) return -EINVAL;

        geometry->bytes_per_sector = spi_flash_device.geometry.bytes_per_sector;
        geometry->sector_count = spi_flash_device.geometry.sector_count;
        geometry->block_size = spi_flash_device.geometry.block_size;
    }
    else if (cmd == DEVICE_CTRL_BLK_ERASE)
    {
        struct spi_flash_erase_cmd *sfec = (struct spi_flash_erase_cmd *)args;

        w25qxx_erase(dev, sfec->start, sfec->size);
    }

    return 0;
}

static size_t w25qxx_flash_read(device_t *dev,
                                off_t pos,
                                void *buffer,
                                size_t size)
{
    flash_lock((struct spi_flash_device *)dev);

    w25qxx_read(pos, buffer, size);

    flash_unlock((struct spi_flash_device *)dev);

    return size;
}

static size_t w25qxx_flash_write(device_t *dev,
                                 off_t pos,
                                 const void *buffer,
                                 size_t size)
{
    size_t _size = size, idx = 0, _len;

    flash_lock((struct spi_flash_device *)dev);

    while (_size > 0)
    {
        _len = pos & 0xFF;
        _len = 0x100 - _len;
        if (_len > _size)
            _len = _size;
        w25qxx_page_write(pos, (uint8_t *)buffer + idx, _len);
        _size -= _len;
        idx   += _len;
        pos   += _len;
    }

    flash_unlock((struct spi_flash_device *)dev);

    return size;
}

static const struct device_ops w25qxx_ops =
{
    .init    = w25qxx_flash_init,
    .open    = w25qxx_flash_open,
    .close   = w25qxx_flash_close,
    .read    = w25qxx_flash_read,
    .write   = w25qxx_flash_write,
    .ctrl = w25qxx_flash_control,
};

err_t w25qxx_init(const char *flash_device_name, const char *spi_device_name)
{
    struct spi_device *spi_device;

    spi_device = (struct spi_device *)device_find(spi_device_name);
    if (spi_device == NULL)
    {
        FLASH_TRACE("spi device %s not found!\r\n", spi_device_name);
        return -ENOSYS;
    }
    spi_flash_device.spi_device = spi_device;

    /* config spi */
    {
        struct spi_cfg cfg;
        cfg.data_width = 8;
        cfg.mode = SPI_MODE_0 | SPI_MSB; /* SPI Compatible: Mode 0 and Mode 3 */
        //cfg.max_hz = 50 * 1000 * 1000; /* 50M */
        cfg.max_hz = 1 * 1000 * 1000; /* 1M */
        spi_configure(spi_flash_device.spi_device, &cfg);
    }

    /* init flash */
    {
        uint8_t cmd;
        uint8_t id_recv[3];
        uint16_t memory_type_capacity;

        flash_lock(&spi_flash_device);

        cmd = 0xFF; /* reset SPI FLASH, cancel all cmd in processing. */
        spi_send(spi_flash_device.spi_device, &cmd, 1);

        cmd = CMD_WRDI;
        spi_send(spi_flash_device.spi_device, &cmd, 1);

        /* read flash id */
        cmd = CMD_JEDEC_ID;
        spi_send_then_recv(spi_flash_device.spi_device, &cmd, 1, id_recv, 3);

        flash_unlock(&spi_flash_device);

        if (id_recv[0] != MF_ID)
        {
            FLASH_TRACE("Manufacturers ID error!\r\n");
            FLASH_TRACE("JEDEC Read-ID Data : %02X %02X %02X\r\n", id_recv[0], id_recv[1], id_recv[2]);
            return -ENOSYS;
        }

        spi_flash_device.geometry.bytes_per_sector = 4096;
        spi_flash_device.geometry.block_size = 4096; /* block erase: 4k */

        /* get memory type and capacity */
        memory_type_capacity = id_recv[1];
        memory_type_capacity = (memory_type_capacity << 8) | id_recv[2];

        if (memory_type_capacity == MTC_EN25Q16A)
        {
            FLASH_TRACE("EN25Q16A detection\r\n");
            spi_flash_device.geometry.sector_count = 512;
        }
        else
        {
            FLASH_TRACE("Memory Capacity error!\r\n");
            return -ENOSYS;
        }
    }

    spi_flash_device.flash_device.ops     = &w25qxx_ops;
    device_register(&spi_flash_device.flash_device, flash_device_name, 0);

    return 0;
}
#endif

