#include <utils.h>
#include <port.h>
#include <init.h>
#include "alloter.h"

static struct
{
    u8  buffer[BUF_SZ];
    u32 free_bitmap;
} mem_pool_man;

int setup_chn_pool(void)
{
    while ((sizeof(mem_pool_man.buffer) / BLOCK_SIZE) > 8 * sizeof(mem_pool_man.free_bitmap));
    mem_pool_man.free_bitmap = (1 << (sizeof(mem_pool_man.buffer) / BLOCK_SIZE)) - 1;

    return 0;
}
core_initcall(setup_chn_pool);

static ubase_t get_last_bit_seqno(u32 x)
{
    ubase_t k = 0;

    while ((x & 1) == 0)
    {
        k++;
        x >>= 1;
    }
    return k;
}

static uint32_t alloc_a_slot(void)
{
    int k;
    uint32_t bitmap;

    bitmap = mem_pool_man.free_bitmap;
    if (bitmap == 0) return INVALID_BLOCK_NO;

    k = get_last_bit_seqno(bitmap);
    bitmap &= (bitmap - 1);
    mem_pool_man.free_bitmap = bitmap;
    mem_pool_man.buffer[(k << BLOCK_NO_SHIFT) | (BLOCK_SIZE - 1)] = INVALID_BLOCK_NO;
    return k;
}

static void free_a_slot(int blk_no)
{
    mem_pool_man.free_bitmap |= 1 << blk_no;
}

void chn_init(chn_slot_t *chn, u16 maxlen)
{
    chn->tx = chn->rx = INVALID_PTR;
    chn->data_cnt = 0;
    if (maxlen > 0) chn->maxlen = maxlen;
}

static inline void chn_reset(chn_slot_t *chn)
{
    chn->tx = chn->rx = INVALID_PTR;
    chn->data_cnt = 0;
}

#define block_no(ptr)       ((ptr) >> BLOCK_NO_SHIFT)
#define block_valid(blk)    ((blk) != INVALID_BLOCK_NO)

err_t chn_put(chn_slot_t *chn,  const void *data, size_t len)
{
    int k, i = 0;
    OS_CPU_SR cpu_sr;

    if (len == 0) return 0;

    enter_critical();
    while (len > 0)
    {
        /* over limit */
        if (chn->data_cnt > chn->maxlen)
            goto exit;

        if (block_no(chn->rx) == INVALID_BLOCK_NO)
        {
            /* need more room */
            u32 blk = alloc_a_slot();
            if (blk == INVALID_BLOCK_NO)
                goto exit;

            chn->rx = chn->tx = blk << BLOCK_NO_SHIFT;
        }

        if ((chn->rx & BLOCK_MASK) == BLOCK_MASK)
        {
            u32 blk = alloc_a_slot();
            if (blk == INVALID_BLOCK_NO)
                goto exit;

            mem_pool_man.buffer[chn->rx] = blk;
            chn->rx = blk << BLOCK_NO_SHIFT;
        }

        k = chn->rx & BLOCK_MASK;
        k = BLOCK_MASK - k;
        k = min(len, k);
        memcpy(&mem_pool_man.buffer[chn->rx], (u8 *)data + i, k);
        len -= k;
        chn->rx += k;
        i += k;
    }
exit:
    chn->data_cnt += i;
    exit_critical();
    return (i);
}

err_t chn_peek(chn_slot_t *chn, void *data, size_t len)
{
    int n = 0;
    u16 rx, tx;
    OS_CPU_SR cpu_sr;

    enter_critical();
    rx = chn->rx;
    tx = chn->tx;
    exit_critical();

    while (len > 0 && tx != rx)
    {
        if ((tx & BLOCK_MASK) == BLOCK_MASK)
        {
            tx = mem_pool_man.buffer[tx] << BLOCK_NO_SHIFT;
        }
        *((u8 *)data + n) = mem_pool_man.buffer[tx];
        n++;
        tx++;
        len--;
    }
    return n;
}

err_t chn_get(chn_slot_t *chn,  void *data, size_t len)
{
    int k, i = 0;
    OS_CPU_SR cpu_sr;

    if (len == 0) return 0;

    enter_critical();
    while (len > 0)
    {
        if (chn->data_cnt == 0)
            goto exit;

        if ((chn->tx & BLOCK_MASK) == BLOCK_MASK)
        {
            u32 blk;

            free_a_slot(block_no(chn->tx));

            blk = mem_pool_man.buffer[chn->tx];
            if (!block_valid(blk))
            {
                chn_reset(chn);
                goto exit;
            }
            chn->tx = blk << BLOCK_NO_SHIFT;
        }
        //some space free
        k = chn->tx & BLOCK_MASK;
        k = BLOCK_MASK - k;
        k = min(len, k);
        k = min(k, chn->data_cnt);
        memcpy((u8 *)data + i, &mem_pool_man.buffer[chn->tx], k);
        len -= k;
        chn->tx += k;
        i += k;
        chn->data_cnt -= k;
        if (chn->data_cnt == 0)
        {
            free_a_slot(block_no(chn->tx));
            chn->tx = chn->rx = INVALID_PTR;
            goto exit;
        }
    }
exit:
    exit_critical();
    return (i);
}
