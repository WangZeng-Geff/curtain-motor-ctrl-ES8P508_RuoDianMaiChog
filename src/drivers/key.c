#include <os.h>
#include <utils.h>
#include "device.h"

#ifdef configUSING_KEY

static struct soft_timer key_tmr;

static void key_generate_event(struct key_device *dev)
{
    int i;

    for (i = 0; i < KEY_POOL_MAX; ++i)
    {
        if (!tst_bit(dev->used, i)) continue;

        if (tst_bit(dev->trg, i))   /* key down */
        {
            dev->press_cnt[i] = KEY_LONG_PRESS_TM / KEY_SCAN_PERIOD;
        }

        if (tst_bit(dev->con, i))   /* key continuous pressed */
        {
            if (dev->press_cnt[i] > 0)
            {
                if (--dev->press_cnt[i] == 0)
                {
                    set_bit(dev->evt_lpressed, i);
                }
            }
        }
        else if (tst_bit(dev->last_con, i))     /* key up, pre key down */
        {
            if (dev->press_cnt[i] > 0)
            {
                set_bit(dev->evt_pressed, i);
            }
        }
    }

    dev->last_con = dev->con;

    if (dev->parent.owner && (dev->evt_pressed != 0 || dev->evt_lpressed != 0))
    {
        task_send_signal(dev->parent.owner, SIG_KEY);
    }
}

static void key_tmr_cb(struct soft_timer *st)
{
    int i;
    uint16_t raw = 0;
    struct key_device *dev = (struct key_device *)st->data;

    for (i = 0; i < KEY_POOL_MAX; ++i)
    {
        uint8_t map;
        if (!tst_bit(dev->used, i)) continue;

        map = dev->press_map[i];
        map <<= 1;
        map |= dev->ops->is_pressed(i) ? 1 : 0;

        if ((map & 0x03) == 0x03)
            set_bit(raw, i);
        dev->press_map[i] = map;
    }

    dev->trg = raw & (raw ^ dev->con);
    dev->con = raw;

    key_generate_event(dev);

    key_tmr.expires += pdMS_TO_TICKS(KEY_SCAN_PERIOD);
    soft_timer_add(&key_tmr);
}
static void key_task_init(struct key_device *key)
{
    key_tmr.cb = key_tmr_cb;
    key_tmr.data = (ubase_t)key;
    key_tmr.expires = INITIAL_JIFFIES + pdMS_TO_TICKS(KEY_SCAN_PERIOD);

    soft_timer_add(&key_tmr);
}

static err_t key_init(struct device *dev)
{
    err_t result = 0;
    struct key_device *key = (struct key_device *)dev;

    key->con = key->trg = key->last_con = 0;
    key->evt_pressed = key->evt_lpressed = 0;
    memset(key->press_map, 0, sizeof(key->press_map));

    key_task_init(key);

    return result;
}

static err_t key_control(struct device *dev, uint8_t cmd, void *args)
{
    err_t err = 0;
    struct key_device *key = (struct key_device *)dev;

    switch (cmd)
    {
    case KEY_CTRL_GET_KEY:
    {
        *(uint32_t *)args = ((uint32_t)key->evt_pressed) | ((uint32_t)key->evt_lpressed << 16);
        key->evt_pressed = key->evt_lpressed = 0;
    }
    break;

    default :
        break;
    }

    return err;
}

static const struct device_ops key_ops =
{
    .init        = key_init,
    .ctrl        = key_control,
};

err_t key_device_register(struct key_device *key, const char *name, uint32_t flag, void *data)
{
    struct device *dev = &(key->parent);

    dev->ops         = &key_ops;
    dev->user_data   = data;

    device_register(dev, name, flag);
    return 0;
}
#endif

