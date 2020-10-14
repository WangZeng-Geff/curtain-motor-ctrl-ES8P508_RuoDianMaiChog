#include <config.h>
#include <board.h>
#include <init.h>
#include <os.h>
#include "ES8P508x.h"
#include "lib_config.h"
#include "device.h"
#include "printk.h"

#ifdef configUSING_IFLASH

#define PAGE_SIZE (1024u)

typedef struct flash_dev
{
    struct device                dev;
    struct device_blk_geometry   geometry;
    void                        *user_data;
} flash_dev_t;

static flash_dev_t iflash;

/*
 * lock and unlock
 */
static void flash_lock(flash_dev_t *flash)
{
	FlashIAP_RegLock();
    FlashIAP_Disable();
    FlashIap_OpenAll_WPROT();
}
static void flash_unlock(flash_dev_t *flash)
{
	FlashIAP_RegUnLock();
    FlashIAP_Enable();
    FlashIap_CloseAll_WPROT();
}

/*
 * erase flash
 */
static u32 es8p508_erase(u32 page_addr)
{
	IAP_PageErase(page_addr);

    return PAGE_SIZE;
}
static uint32_t iflash_erase(device_t *dev, off_t pos, size_t size)
{
    size_t _size, _len;

    flash_unlock((flash_dev_t *)dev);

    _size = size;
    while (_size > 0)
    {
        _len = min(_size, PAGE_SIZE);
        es8p508_erase(pos);
        pos += PAGE_SIZE;
        _size -= _len;
    }

    flash_lock((flash_dev_t *)dev);

    return size;
}

/*
 * read flash
 */
static size_t iflash_read(device_t *dev, off_t pos, void *buffer, size_t size)
{
    flash_unlock((flash_dev_t *)dev);

    memcpy(buffer, (void *)pos, size);

	flash_lock((flash_dev_t *)dev);

    return size;
}

/*
 * write flash
 */
static int es8p508_flash_write(uint32_t addr, const void *data, uint32_t len)
{
    int ret = -1;

    if ((len == 0) || (len & 0x03) || ((uint32_t)data & 0x03))
	{
		log_d("flash_write error len: %d, data: %d\n", len, data);
        goto out;
	}

	int i;
	for (i = 0; i < len; i += 4)
    {
        if (IAP_WordProgram(addr, *((uint32_t *)data + i / 4)) == ERROR)
            goto out;
		addr += 4;
    }
    ret = len;
out:
    return ret;
}
static size_t iflash_write(device_t *dev, off_t pos, const void *data, size_t size)
{
    flash_unlock((flash_dev_t *)dev);
	
    if (es8p508_flash_write(pos, data, size) < 0) size = 0;

    flash_lock((flash_dev_t *)dev);

    return size;
}

/*
 * control flash
 */
static err_t iflash_control(device_t *dev, uint8_t cmd, void *args)
{
    switch (cmd)
    {
    case DEVICE_CTRL_BLK_ERASE:
    {
        struct flash_erase_cmd *fec = (struct flash_erase_cmd *)args;

        iflash_erase(dev, fec->start, fec->size);
        break;
    }
    default:
        break;
    }

    return 0;
}

static const struct device_ops iflash_ops =
{
    .read    = iflash_read,
    .write   = iflash_write,
    .ctrl    = iflash_control,
};

//static void test_iflash(void) USED;
//static void test_iflash(void)
//{
//    u8 tmp[0x20];

//    memset(tmp, 0, sizeof(tmp));

//    iflash_erase(&iflash.dev, 64 * 1024, 1024);

//    iflash_read(&iflash.dev, 64 * 1024, tmp, sizeof(tmp));

//	int i;
//	for (i = 0; i < sizeof(tmp); i++)
//		tmp[i] = i;
//    iflash_write(&iflash.dev, 64 * 1024, tmp, sizeof(tmp));

//    memset(tmp, 0, sizeof(tmp));
//    iflash_read(&iflash.dev, 64 * 1024, tmp, sizeof(tmp));

////    assert();
//}
static int board_add_iflash(void) USED;
static int board_add_iflash(void)
{
//	test_iflash();

    iflash.dev.ops = &iflash_ops;
    device_register(&iflash.dev, "iflash", 0);
    return 0;
}
device_initcall(board_add_iflash);
#endif

