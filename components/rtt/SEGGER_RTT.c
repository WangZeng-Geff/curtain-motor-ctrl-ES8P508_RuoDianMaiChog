#include <printk.h>
#include <utils.h>
#include <stdint.h>
#include <os.h>
#include "SEGGER_RTT.h"
#include "kfifo.h"

#ifdef configUSING_RTT_DBG

#ifndef BUFFER_SIZE_UP
# define BUFFER_SIZE_UP                     (1024)  // Size of the buffer for terminal output of target, up to host
#endif

#ifndef BUFFER_SIZE_DN
# define BUFFER_SIZE_DN                     (16)  // Size of the buffer for terminal input to target from host (Usually keyboard input)
#endif

#ifndef SEGGER_RTT_LOCK
# define SEGGER_RTT_LOCK()
#endif

#ifndef SEGGER_RTT_UNLOCK
# define SEGGER_RTT_UNLOCK()
#endif

#ifndef SEGGER_RTT_IN_RAM
# define SEGGER_RTT_IN_RAM                  (0)
#endif

typedef struct
{
    char   acID[16];
    DECLARE_KFIFO_PTR(fifo_up,   unsigned char);
    DECLARE_KFIFO_PTR(fifo_down, unsigned char);
    uint8_t mode_up;
    uint8_t mode_down;
} SEGGER_RTT_CB;

static SEGGER_RTT_CB _SEGGER_RTT =
{
#if SEGGER_RTT_IN_RAM
    .acID = "SEGGER RTTI",
#else
    .acID = "SEGGER RTT",
#endif
    .mode_up   = SEGGER_RTT_MODE_NO_BLOCK_SKIP,
    .mode_down = SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL,
    //{0, 0, sizeof(_acUpBuf)-1, 1, (char *)_acUpBuf},
    //{0, 0, sizeof(_acDnBuf)-1, 1, (char *)_acDnBuf},
    //SEGGER_RTT_MODE_NO_BLOCK_SKIP,
    //SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL,
};

static u8 buf_up[BUFFER_SIZE_UP], buf_dn[BUFFER_SIZE_DN];
static void _Init(void)
{
    //if (_SEGGER_RTT.acID[0] == 'S') return;

#if SEGGER_RTT_IN_RAM
    if (_SEGGER_RTT.acID[10] == 'I')
    {
        _SEGGER_RTT.acID[10] = '\0';
    }
#endif
    kfifo_init(&_SEGGER_RTT.fifo_up, buf_up, sizeof(buf_up));
    kfifo_init(&_SEGGER_RTT.fifo_down, buf_dn, sizeof(buf_dn));
}

int RTT_read(void *out, size_t maxlen)
{
    size_t ret;

    SEGGER_RTT_LOCK();
    ret = kfifo_out(&_SEGGER_RTT.fifo_down, out, maxlen);
    SEGGER_RTT_UNLOCK();
    return ret;
}

int RTT_write(const char *in, size_t len)
{
    size_t unused;

    SEGGER_RTT_LOCK();

    if (_SEGGER_RTT.mode_up == SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL)
    {
        do
        {
            unused = kfifo_avail(&_SEGGER_RTT.fifo_up);
        }
        while (len > unused);
    }
    else if (_SEGGER_RTT.mode_up == SEGGER_RTT_MODE_NO_BLOCK_SKIP)
    {
        unused = kfifo_avail(&_SEGGER_RTT.fifo_up);
        if (len > unused)
        {
            SEGGER_RTT_UNLOCK();
            return 0;
        }
    }

    len = kfifo_in(&_SEGGER_RTT.fifo_up, (const uint8_t *)in, len);

    SEGGER_RTT_UNLOCK();
    return len;
}

int RTT_write_string(const char *str)
{
    return RTT_write(str, strlen(str));
}

int RTT_get_key(void)
{
    unsigned char c;

    return RTT_read(&c, 1) == 1 ? (int)c : -1;
}

int RTT_wait_key(void)
{
    int r;

    do
    {
        r = RTT_get_key();
    }
    while (r < 0);
    return r;
}

int RTT_has_key(void)
{
    return kfifo_len(&_SEGGER_RTT.fifo_down) > 0;
}

#ifdef configUSING_SHELL
static void rtt_task_callback(struct task_ctrl_blk *tcb, ubase_t data)
{
    tSTART(tcb);

    for (;;)
    {
		extern void shell_get_chr(char ch);
		if (RTT_has_key())
			shell_get_chr(RTT_get_key());
        task_delay(tcb, 1);
    }

    tEND();
}
#endif

void RTT_init(void)
{
    _Init();
#ifdef configUSING_SHELL
	static tcb_t rtt_tcb;
	task_create(&rtt_tcb, rtt_task_callback, 0);
#endif
}

#endif
