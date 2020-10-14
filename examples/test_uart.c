#include <config.h>
#include <os.h>
#include <utils.h>
#include <printk.h>
#include "device.h"
#include "test_uart.h"

#ifdef configTEST

# ifdef configUSING_SERIAL
static struct _test_uart
{
    const char *name;
    u32         baud;
    device_t   *dev;
    tcb_t       tcb;
} test_uarts [] =
{
#  ifdef configUSING_UART1
    {.name = "uart1", .baud = BAUD_RATE_115200,},
#  endif
#  ifdef configUSING_UART2
    {.name = "uart2", .baud = BAUD_RATE_115200,},
#  endif
#  ifdef configUSING_UART3
    {.name = "uart3", .baud = BAUD_RATE_115200,},
#  endif
#  ifdef configUSING_VCOM
    {.name = "vcom",  .baud = BAUD_RATE_2400,},
#  endif
};

static device_t *uart_open(const char *fname, uint32_t baud_rate)
{
    err_t err;
    device_t *dev;
    struct serial_configure cfg;

    dev = device_find(fname);
    assert(dev);

    err = device_ctrl(dev, SERIAL_CTRL_GETCFG, &cfg);
    assert(err == 0);

    cfg.baud_rate = baud_rate;
    cfg.bufsz     = 0x100;
    cfg.data_bits = DATA_BITS_8;
    cfg.parity    = PARITY_NONE;
    cfg.stop_bits = STOP_BITS_1;
    err = device_ctrl(dev, SERIAL_CTRL_SETCFG, &cfg);
    assert(err == 0);

    err = device_open(dev, DEVICE_FLAG_INT_RX | DEVICE_FLAG_INT_TX | DEVICE_FLAG_FASYNC);
    assert(err == 0);

    return dev;
}

static void uart_task_callback(struct task_ctrl_blk *tcb, ubase_t data)
{
    u8 tmp[0x100];
    struct _test_uart *uart = (struct _test_uart *)data;

    tSTART(tcb);
    uart->dev = uart_open(uart->name, uart->baud);
    assert(uart->dev);

    for (;;)
    {
        task_wait_signal(tcb);

        sig_t sig = task_signal(tcb);

        if (sigget(sig, SIG_DATA))
        {
            err_t ret = device_peek(uart->dev, 0, tmp, sizeof(tmp));
            if (ret > 0)
            {
                device_write(uart->dev, 0, tmp, ret);
                device_read(uart->dev, 0, tmp, ret);
            }
        }
    }

    tEND();
}

void test_uart(void)
{
    int i;

    for (i = 0; i < array_size(test_uarts); i++)
    {
        task_create(&test_uarts[i].tcb, uart_task_callback, (ubase_t)&test_uarts[i]);
    }
}
# endif
#endif
