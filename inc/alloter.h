#ifndef _ALLOTER_H_
#define _ALLOTER_H_

#include <types.h>

#define BUF_SZ  2048

typedef struct
{
    u16 tx, rx, data_cnt, maxlen;
} chn_slot_t;

#define BLOCK_NO_SHIFT      (7)
#define BLOCK_SIZE          (0x01 << BLOCK_NO_SHIFT)
#define BLOCK_MASK          (BLOCK_SIZE-1)

#define INVALID_BLOCK_NO    (0xFF)
#define INVALID_PTR         (INVALID_BLOCK_NO << BLOCK_NO_SHIFT)

int  setup_chn_pool(void);
void chn_init(chn_slot_t *chn, u16 maxlen);
err_t chn_put(chn_slot_t *chn,  const void *data, size_t len);
err_t chn_get(chn_slot_t *chn,  void *data, size_t len);
err_t chn_peek(chn_slot_t *chn, void *data, size_t len);
#endif
