#ifndef _KEY_H_
#define _KEY_H_

#include <stdint.h>
#include <stdbool.h>
#include "device.h"

#define KEY_POOL_MAX        16

#define KEY_SCAN_PERIOD     (20)    /* 20ms */
#define KEY_LONG_PRESS_TM   (5000)  /* 5s */
#define KEY_LONG_PRESS_CNT  (KEY_LONG_PRESS_TM / KEY_SCAN_PERIOD)  /* 150 count */

struct key_ops
{
    bool (*is_pressed)(uint8_t keyno);
};
struct key_device
{
    struct device          parent;
    const struct key_ops  *ops;

    uint16_t used;
    uint16_t  press_cnt[KEY_POOL_MAX];
    uint16_t  press_map[KEY_POOL_MAX];
    uint16_t trg, con, last_con;
    uint16_t evt_pressed, evt_lpressed;
};

err_t key_device_register(struct key_device *key, const char *name, uint32_t flag, void *data);
#endif
