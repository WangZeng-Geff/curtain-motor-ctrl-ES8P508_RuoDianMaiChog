#include <config.h>
#include <types.h>
#include <utils.h>
#include <os.h>
#include <printk.h>
#include "dev.h"
#include "settings.h"
#include "update.h"
#include "board.h"

#ifndef configUSING_UPWAY
typedef struct 
{
    u32 size;
    u16 crc;
    u16 flag;
} update_param_t;
#endif

#define UPDATE_FINISHED     0xFFFF
#define UPDATE_PROG_FLAG    0xAA55


#ifndef configUSING_UPWAY
static update_cache_t update_cache;

static void update_catche(update_file_t *file, size_t len)
{
    update_cache_t *uc = &update_cache;

    uc->size        = file->size;
    uc->block_size  = file->block_size;
    uc->max_blk_cnt = (uc->size + uc->block_size - 1) / uc->block_size;
    uc->crc         = file->crc;
    uc->block_no    = 1;
    memcpy(uc->ver, file->data, len);
}
#endif
static int update_hframe_opt(up_hdr_t *u)
{
    int ret = 0;
	
#ifdef configUSING_UPWAY
    u16 seq_no         = 0;
#else
	update_cache_t *uc = &update_cache;
   u16 seq_no         = uc->block_no;
#endif
	
    update_file_t *uf  = (update_file_t *)u->data;
		u8 len = u->len;

    if (len <= sizeof(update_file_t) 
        || !dev_type_cmp(uf->type, sizeof(uf->type))
        || uf->size > APP_SIZE)  /*长度不对 设备类型不同 文件大小超过32K  返回错误报文*/
    {
        dev_type_get(uf->type, sizeof(uf->type));
        ret    = sizeof(update_file_t);
        ret   += dev_ver_get(uf->data, 0x40);
        seq_no = 0;
        goto end_lbl;
    }

    len -= sizeof(update_file_t);

    /* check device software version */
    if (dev_ver_cmp(uf->data, len))
    {
        seq_no = UPDATE_FINISHED;
        goto end_lbl;
    }
#ifndef configUSING_UPWAY
    /* breakpoint resume */
    if ((uc->size == uf->size) && (uc->crc == uf->crc) && (uc->block_size == uf->block_size)
        && !memcmp(uc->ver, uf->data, len))
        goto end_lbl;

    update_catche(uf, len);
    disk_erase(UPDATE_PARA_ADDR, UPDATE_PARA_SIZE + uc->size);
#endif
    seq_no = 1;

end_lbl:
    u->seq = seq_no;
    return ret;
}

#ifndef configUSING_UPWAY
static bool check_file_crc(void)
{
    u16 crc  = 0;
    u32 addr = UPDATE_DATA_ADDR, len = update_cache.size;
    u8 tmp[0x80];

    while (len > 0)
    {
        int _len = min(len, sizeof(tmp));

        disk_read(addr, tmp, _len);
        crc = crc16(crc, tmp, _len);

        len  -= _len;
        addr += _len;

        board_feed_wdg();
    }
    return crc == update_cache.crc;
}

static void do_save_update_param(update_cache_t *uc)
{
    update_param_t up;

    up.size = uc->size;
    up.crc  = uc->crc;
    up.flag = UPDATE_PROG_FLAG;
    disk_write(UPDATE_PARA_ADDR, &up, sizeof(up));
}
static int update_dframe_opt(up_hdr_t *u)
{
    u32 off;
    update_cache_t *uc = &update_cache;

    if (uc->crc != u->crc || uc->block_size != u->len)
    {
        u->crc = uc->crc;
        goto end;
    }

    if (uc->block_no != u->seq)
        goto end;

	uint8_t udata[0xFF];
	memcpy(udata, u->data, u->len);
	off = (uc->block_no - 1) * uc->block_size + UPDATE_DATA_ADDR;
    disk_write(off, udata, u->len);

    ++uc->block_no;
    if (uc->block_no > uc->max_blk_cnt)
    {
        if (!check_file_crc())
        {
            uc->block_no = 1;
            disk_erase(UPDATE_PARA_ADDR, UPDATE_PARA_SIZE + uc->size);
            goto end;
        }

        uc->block_no = UPDATE_FINISHED;

        /* save update param for bootloader */
        do_save_update_param(uc);
        board_reboot(100);
    }

end:
    u->seq = uc->block_no;
    return 0;
}
#endif

int update_frame_opt(u8 *data, int len)
{
    int ret = 0;

    up_hdr_t *u = (up_hdr_t *)data;

    if (len != sizeof(up_hdr_t) + u->len) return 0;
    if (u->seq == 0)
        ret = update_hframe_opt(u);
#ifndef configUSING_UPWAY
    else
        ret = update_dframe_opt(u);
#endif
    u->len = ret;
    ret    = u->ack ? sizeof(up_hdr_t) + u->len : 0;
    u->ack = 0;
    return (ret);
}

#ifdef configUSING_UPWAY
void boot_start_update(u8 *data, int len)
{
    uint32_t flag = UPDATE_PROG_FLAG;

	extern int code_local_frame(const uint8_t *in, int len, void *out, int maxlen);
	const u8 in[2] = {0x23, 0x01};
	u8 tmp[0x20];
	len = code_local_frame(in, sizeof(in), tmp, sizeof(tmp));
	device_t *serial_plc = device_find("uart2");
	device_write(serial_plc, 0, tmp, len);
	disk_erase(UPDATE_PARA_ADDR, sizeof(flag));
    disk_write(UPDATE_PARA_ADDR, &flag, sizeof(flag));
    board_reboot(100);
}
#endif


