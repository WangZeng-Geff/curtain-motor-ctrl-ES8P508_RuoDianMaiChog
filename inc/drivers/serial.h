#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "types.h"
#include "softtimer.h"
#include "alloter.h"

#define BAUD_RATE_2400                  2400
#define BAUD_RATE_4800                  4800
#define BAUD_RATE_9600                  9600
#define BAUD_RATE_19200                 19200
#define BAUD_RATE_38400                 38400
#define BAUD_RATE_57600                 57600
#define BAUD_RATE_115200                115200
#define BAUD_RATE_230400                230400
#define BAUD_RATE_460800                460800
#define BAUD_RATE_921600                921600
#define BAUD_RATE_2000000               2000000
#define BAUD_RATE_3000000               3000000

#define DATA_BITS_5                     5
#define DATA_BITS_6                     6
#define DATA_BITS_7                     7
#define DATA_BITS_8                     8
#define DATA_BITS_9                     9

#define STOP_BITS_1                     0
#define STOP_BITS_2                     1
#define STOP_BITS_3                     2
#define STOP_BITS_4                     3

#define PARITY_NONE                     0
#define PARITY_ODD                      1
#define PARITY_EVEN                     2

#define BIT_ORDER_LSB                   0
#define BIT_ORDER_MSB                   1

#define SERIAL_EVENT_RX_IND          0x01    /* Rx indication */
#define SERIAL_EVENT_TX_RDY          0x02
#define SERIAL_EVENT_RX_DMADONE      0x03
#define SERIAL_EVENT_TX_DMADONE      0x04
#define SERIAL_EVENT_RX_TIMEOUT      0x05    /* Rx timeout    */

#define SERIAL_RX_INT                0x01
#define SERIAL_TX_INT                0x02

#define SERIAL_ERR_OVERRUN           0x01
#define SERIAL_ERR_FRAMING           0x02
#define SERIAL_ERR_PARITY            0x03

#define SERIAL_TX_DATAQUEUE_SIZE     2048
#define SERIAL_TX_DATAQUEUE_LWM      30

#ifndef SERIAL_RB_BUFSZ
# define SERIAL_RB_BUFSZ             64
#endif

/* Default config for serial_configure structure */
#define SERIAL_CONFIG_DEFAULT              \
{                                          \
    BAUD_RATE_9600,   /* 9600 bits/s */  \
    DATA_BITS_8,      /* 8 databits */     \
    STOP_BITS_1,      /* 1 stopbit */      \
    PARITY_NONE,      /* No parity  */     \
    SERIAL_RB_BUFSZ,  /* Buffer size */  \
    0                                      \
}

struct serial_configure
{
    uint32_t baud_rate;

    uint32_t data_bits               : 4;
    uint32_t stop_bits               : 2;
    uint32_t parity                  : 2;
    uint32_t bufsz                   : 16;
    uint32_t reserved                : 8;
};

struct uart_ops;
struct serial_device
{
    struct device          parent;

    const struct uart_ops  *ops;
    struct serial_configure   config;
#ifdef configUSING_FRAME_TIMEOUT_SOFT
    struct soft_timer rxto;
#endif
    chn_slot_t          rx, tx;
    //DECLARE_KFIFO_PTR(serial_rx, unsigned char);
    //DECLARE_KFIFO_PTR(serial_tx, unsigned char);
};
typedef struct rt_serial_device rt_serial_t;

/**
 * uart operators
 */
struct uart_ops
{
    err_t(*cfg)(struct serial_device *dev, struct serial_configure *cfg);
    err_t(*control)(struct serial_device *dev, int cmd, void *arg);

    int (*putc)(struct serial_device *dev, char c);
    int (*getc)(struct serial_device *dev);
};

void serial_device_isr(struct serial_device *serial, int event);

err_t serial_device_register(struct serial_device *serial, const char *name, uint32_t flag, void *data);

#endif
