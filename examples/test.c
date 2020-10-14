#include <config.h>
#include <utils.h>
#include <printk.h>
#include <os.h>
#include "kfifo.h"
#include "device.h"
#include "board.h"
#include "test_uart.h"
#include "cJSON.h"
//#include "mem.h"

#ifdef configTEST

# ifdef USING_KFIFO
static void test_kfifo(void)
{
    int ret;
    uint8_t d[0x08];

    static DECLARE_KFIFO(myfifo, unsigned char, 0x100);

    INIT_KFIFO(myfifo);

    static const struct
    {
        int a, b;
    } c = {.a = 1, .b = 2};

    assert(kfifo_len(&myfifo) == 0);
    assert(kfifo_avail(&myfifo) == 0x100);

    ret = kfifo_out_peek(&myfifo, d, sizeof(d));
    assert(ret == 0);

    ret = kfifo_out(&myfifo, d, sizeof(d));
    assert(ret == 0);

    ret = kfifo_in(&myfifo, (void *)&c, sizeof(c));
    assert(ret == sizeof(c));

    memset(d, 0, sizeof(d));
    ret = kfifo_out_peek(&myfifo, d, sizeof(d));
    assert(ret == sizeof(c));
    assert(!memcmp(d, &c, sizeof(c)));

    memset(d, 0, sizeof(d));
    ret = kfifo_out(&myfifo, d, sizeof(d));
    assert(ret == sizeof(c));
    assert(!memcmp(d, &c, sizeof(c)));
}

static void test_kfifo_record(void)
{
    int ret;
    uint8_t d[0x08];

    static STRUCT_KFIFO_REC_1(0x100) myfifo;

    INIT_KFIFO(myfifo);

    static const struct
    {
        int a, b;
    } c = {.a = 1, .b = 2};

    ret = kfifo_out_peek(&myfifo, d, sizeof(d));
    assert(ret == 0);

    ret = kfifo_out(&myfifo, d, sizeof(d));
    assert(ret == 0);

    ret = kfifo_in(&myfifo, &c, sizeof(c));
    assert(ret == sizeof(c));

    memset(d, 0, sizeof(d));
    ret = kfifo_out_peek(&myfifo, d, sizeof(d));
    assert(ret == sizeof(c));
    assert(!memcmp(d, &c, sizeof(c)));

    memset(d, 0, sizeof(d));
    ret = kfifo_out(&myfifo, d, sizeof(d));
    assert(ret == sizeof(c));
    assert(!memcmp(d, &c, sizeof(c)));
}
# endif

# ifdef configTEST_SOFTTIMER
/*
 * soft timer test
 */

static void soft_timer_test_cb(struct soft_timer *st)
{
    log_d("data = %d\r\n", st->data);

    switch (st->data)
    {
    case 1:
        st->expires += configHZ / 2;
        soft_timer_add(st);
        break;
    case 2:
        st->expires += configHZ * 1;
        soft_timer_add(st);
        break;
    }
}
void test_soft_timer(void)
{
    static struct soft_timer test_timer[3] =
    {
        [0] = {
            .expires = INITIAL_JIFFIES + configHZ * 0,
            .cb      = soft_timer_test_cb,
            .data    = 0,
        },
        [1] = {
            .expires = INITIAL_JIFFIES + configHZ / 2,
            .cb      = soft_timer_test_cb,
            .data    = 1,
        },
        [2] = {
            .expires = INITIAL_JIFFIES + configHZ * 1,
            .cb      = soft_timer_test_cb,
            .data    = 2,
        },
    };

    soft_timer_add(&test_timer[0]);
    soft_timer_add(&test_timer[1]);
    soft_timer_add(&test_timer[2]);
	soft_timer_add(&test_timer[2]);
	soft_timer_add(&test_timer[2]);
	soft_timer_del(&test_timer[0]);
	soft_timer_del(&test_timer[0]);
	soft_timer_del(&test_timer[0]);
}
# endif

# ifdef configTEST_W25Qxx
static void test_25q16a(void)
{
    err_t err;
    uint32_t val = 0x55AAAA55, dummy = 0;

    device_t *q16a = device_find("25q16");
    assert(q16a);

    err = device_open(q16a, 0);
    assert(err == 0);

    struct spi_flash_erase_cmd sfec;
    sfec.start = 0;
    sfec.size  = 10;
    err = device_ctrl(q16a, DEVICE_CTRL_BLK_ERASE, &sfec);
    assert(err == 0);

    err = device_read(q16a, 10, &dummy, sizeof(dummy));
    assert(err == 4);
    assert(dummy == 0xFFFFFFFF);

    err = device_write(q16a, 10, &val, sizeof(val));
    assert(err == 4);

    err = device_read(q16a, 10, &dummy, sizeof(dummy));
    assert(err == 4);
    assert(dummy == val);
}
# endif

# ifdef USING_JSON
void test_json(void)
{
    char pframe[] = "{ \"deviceKey\": \"12345678\", \
                    \"cmd\": \"addDevice\", \
                    \"channel\": \"plc-rs485\", \
                    \"data\": { \
                        \"aid\": \"12345678\", \
                        \"password\": \"4660\", \
                        \"plcMeterAddr\": \"112233445566\" \
                    }, \
                    \"sno\": \"1234567890\" }";

    cJSON *root = cJSON_Parse(pframe);
    assert(root);

    cJSON_Delete(root);
}
# endif

# ifdef USING_RX8025
static device_t rx8025;
static void rx8025_timer_cb(struct timer *st)
{
    err_t err = 0;
    uint8_t bcdtm[6];

    err = device_ctrl(rx8025, RTC_CTRL_GET_TIME, bcdtm);
    assert(err == 0);

    log_arr_d(MODULE_APP, bcdtm, sizeof(bcdtm));

    st->expires += 5 * configHZ;
    timer_add(st);
}
void test_rx8025(void)
{
    err_t err;
    static struct timer st =
    {
        .expires = INITIAL_JIFFIES + 5 * configHZ,
        .timer_cb = rx8025_timer_cb,
    };

    rx8025 = device_find("r8025");
    assert(rx8025);

    err = device_open(rx8025, 0);
    assert(err == 0);

    timer_add(&st);
}
# endif

# ifdef USING_AT24C
static void test_at24c(void)
{
    err_t err;
    uint32_t val = 0x55AAAA55, dummy = 0;

    device_t at24c = device_find("at24c");
    assert(at24c);

    err = device_open(at24c, 0);
    assert(err == 0);

    err = device_write(at24c, 0, &val, sizeof(val));
    assert(err == 4);

    err = device_read(at24c, 0, &dummy, sizeof(dummy));
    assert(err == 4);
    assert(dummy == val);
}
# endif

# ifdef configTEST_LED
static void led_tmr_cb(struct soft_timer *st)
{
    err_t err;
    u8 tmp[100];
    int i;
    static int on = 0;
    device_t *dev = (device_t *)st->data;

    on = 1 - on;

    led_cmd_t *led = (led_cmd_t *)tmp;
    led->n = 32;
    for (i = 0; i < 16; i += 2)
    {
        led_point_init(&led->point[i], i, 0, 0, 0, 1 - on);
        led_point_init(&led->point[i + 1], i + 1, 0, 0, 0, on);
    }
    for (i = 16; i < 32; i += 2)
    {
        led_point_init(&led->point[i], i, 1, 0, 0, 1 - on);
        led_point_init(&led->point[i + 1], i + 1, 1, 0, 0, on);
    }
    err = device_ctrl(dev, LED_CTRL_SET, led);
    assert(err == 0);



    st->expires += configHZ;
    soft_timer_add(st);
}
static void test_led(void)
{
    err_t err;
    static device_t *dev;

    dev = device_find("led");
    assert(dev);

    err = device_open(dev, 0);
    assert(err == 0);

    static struct soft_timer led_tmr =
    {
        .expires = INITIAL_JIFFIES,
        .cb      = led_tmr_cb,
    };
    led_tmr.data = (ubase_t)dev;
    soft_timer_add(&led_tmr);
}
# endif
# ifdef configTEST_KEY
static tcb_t key_tcb;
static void key_task_callback(struct task_ctrl_blk *tcb, ubase_t data)
{
    err_t err;
    uint32_t keymap = 0;
    static device_t *dev;

    tSTART(tcb);
    dev = device_find("key");
    assert(dev);

    err = device_open(dev, DEVICE_FLAG_FASYNC);
    assert(err == 0);

    for (;;)
    {
        task_wait_signal(tcb);

        sig_t sig = task_signal(tcb);

        if (sigget(sig, SIG_KEY))
        {
            err = device_ctrl(dev, KEY_CTRL_GET_KEY, &keymap);
            if (keymap & 0x01)
            {
                log_d(MODULE_APP, "short pressed\n!");
            }

            keymap >>= 16;
            if (keymap & 0x01)
            {
                log_d(MODULE_APP, "long pressed\n!");
            }
        }
    }

    tEND();
}
static void test_key(void)
{
    task_create(&key_tcb, key_task_callback, 0);
}
# endif

# ifdef configTEST_W5500
static tcb_t test_w5500_tcb;
static void w5500_conn(device_t *dev)
{
    err_t err;
    socket_open_arg_t open_arg = {.sock = 0, .proto = SOCKET_TCP, .port = 6000,};


    {
        phy_addr_t phy_addr;

        err = device_ctrl(dev, DEV_CTRL_NET_GET_PHYADDR, &phy_addr);
        assert(err == 0);
        phy_addr.addr[0] = 0x0C;
        phy_addr.addr[5] = 0x01;
        err = device_ctrl(dev, DEV_CTRL_NET_SET_PHYADDR, &phy_addr);
        assert(err == 0);
    }
    {
        net_cfg_t cfg;

        err = device_ctrl(dev, DEV_CTRL_NET_GET_CFG, &cfg);
        assert(err == 0);
        IP4_ADDR(&cfg.ip,      192, 168, 1, 11);
        IP4_ADDR(&cfg.submask, 255, 255, 255, 0);
        IP4_ADDR(&cfg.gateway, 192, 168, 1, 254);
        err = device_ctrl(dev, DEV_CTRL_NET_SET_CFG, &cfg);
        assert(err == 0);
    }


    err = device_ctrl(dev, DEV_CTRL_SOCKET_OPEN, &open_arg);
    assert(err == 0);

    socket_conn_arg_t conn_arg = {.sock = 0, .port = 7000};
    IP4_ADDR(&conn_arg.dst, 192, 168, 1, 235);

    err = device_ctrl(dev, DEV_CTRL_SOCKET_CONN, &conn_arg);
    assert(err == 0);
}
static void w5500_task_callback(struct task_ctrl_blk *tcb, ubase_t data)
{
    err_t err;
    static device_t *dev;

    tSTART(tcb);
    dev = device_find("w5500");
    assert(dev);

    err = device_open(dev, DEVICE_FLAG_FASYNC);
    assert(err == 0);

    w5500_conn(dev);

    for (;;)
    {
        task_wait_signal(tcb);

        sig_t sig = task_signal(tcb);

        if (sigget(sig, SIG_DATA))
        {
            {
                net_event_t evt;
                err = device_ctrl(dev, DEV_CTRL_GETEVT, &evt);
                switch (evt)
                {
                case NE_IPCONFILCT:
                {
                    log_i(MODULE_APP, "ip conflict\n");
                    break;
                }
                default:
                    break;
                }
            }

            {
                socket_event_t evt;
                err = device_ctrl(dev, DEV_CTRL_SOCKET_GETEVT, &evt);
                switch (evt)
                {
                case SE_CONN:
                {
                    u8 tmp[] = {0x01, 0x02, 0x03, 0x04};
                    socket_write_arg_t arg = {.sock = 0, .buf = tmp, .len = sizeof(tmp),};
                    device_ctrl(dev, DEV_CTRL_SOCKET_WRITE, &arg);
                    break;
                }
                case SE_CLOSED:
                {
                    log_i(MODULE_APP, "socket closed\n");
                    socket_close_arg_t arg = {.sock = 0,};
                    device_ctrl(dev, DEV_CTRL_SOCKET_CLOSE, &arg);
                    break;
                }
                case SE_SENT:
                {
                    log_i(MODULE_APP, "send done\n");
                    break;
                }
                case SE_RECVED:
                {
                    u8 tmp[100] = {0};
                    socket_read_arg_t arg = {.sock = 0, .buf = tmp, .len = sizeof(tmp),};

                    err = device_ctrl(dev, DEV_CTRL_SOCKET_READ, &arg);
                    if (err > 0)
                    {
                        socket_write_arg_t arg = { .sock = 0, .buf = tmp, .len = err, };
                        device_ctrl(dev, DEV_CTRL_SOCKET_WRITE, &arg);
                    }

                    break;
                }
                case SE_TO:
                {
                    log_i(MODULE_APP, "time out\n");
                    break;
                }
                default:
                    break;
                }
            }
        }
    }

    tEND();
}

static void test_w5500(void)
{
    task_create(&test_w5500_tcb, w5500_task_callback, 0);
}
# endif
# ifdef USING_HEAP
void test_mem(void)
{
	int i;
	char *ptr[15];
	os_heap_init();
	uint8_t *addr  = (uint8_t *)os_malloc(50);
	for (i = 0; i < 15; i ++) ptr[i] = NULL;
	for (i = 0; i < 15; i++)
	{
		ptr[i] = os_malloc(1 << (i+5));
		if (ptr[i] != NULL)
		{
			log_d("get memory: 0x%x\n", ptr[i]);
			ptr[i] = os_realloc(ptr[i], 1 << (i+4));
//				os_free(ptr[i]);
//				ptr[i] = NULL;
		}
	}
	addr = (uint8_t *)os_malloc(50);
}
#endif
#endif

void test_setup(void)
{
#ifdef configTEST

# ifdef USING_KFIFO
    test_kfifo();
    test_kfifo_record();
# endif

# ifdef configTEST_SOFTTIMER
    test_soft_timer();
# endif

# ifdef USING_JSON
    test_json();
# endif

# ifdef configUSING_SERIAL
    test_uart();
# endif

# ifdef configTEST_W25Qxx
    test_25q16a();
# endif

# ifdef USING_RX8025
    test_rx8025();
# endif

# ifdef USING_AT24C
    test_at24c();
# endif

# ifdef configTEST_LED
    test_led();
# endif

# ifdef configTEST_KEY
    test_key();
# endif

# ifdef configTEST_W5500
    test_w5500();
# endif
# ifdef USING_HEAP
	test_mem();
# endif
#endif
}
