#ifndef CO_ROUTINE_H
#define CO_ROUTINE_H

#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

enum TASK_STATE
{
    TS_READY,
    TS_BLOCK,
    TS_DELAY,
};

typedef struct task_ctrl_blk
{
    struct list_head entry;
    void (*tcb_cb)(struct task_ctrl_blk *tcb, ubase_t);
    ubase_t          data;
    enum TASK_STATE  status;
    u16              state;
    ubase_t          signal;
    ubase_t          expire;
} tcb_t;

void task_create(tcb_t *tcb, void (*tcb_cb)(struct task_ctrl_blk *, ubase_t), ubase_t data);
tcb_t *task_get_current_task_handle(void);
void task_schedule(void);

#define tSTART(tcb) switch((tcb)->state) {case 0:
#define tEND() }

/*
 * These macros are intended for internal use by the co-routine implementation
 * only.  The macros should not be used directly by application writers.
 */
#define tSET_STATE0(tcb) (tcb)->state = (__LINE__*2); return; case (__LINE__*2):
#define tSET_STATE1(tcb) (tcb)->state = (__LINE__*2+1); return; case (__LINE__*2+1):

#define task_delay(tcb, ticks)                              \
    if((ticks) > 0)                                         \
    {                                                       \
        task_add_to_delayed_list(tcb, ticks);               \
    }                                                       \
    tSET_STATE0(tcb);

#define task_wait_signal(tcb)                               \
    if ((tcb)->signal == 0)                                 \
    {                                                       \
        task_add_to_block_list(tcb);                        \
        tSET_STATE1(tcb);                                   \
    }


ubase_t task_signal(tcb_t *tcb);
#define sigempty(sigmap) ((sigmap) = 0)
#define sigset(sigmap, sig) ((sigmap) |= (sig))
#define sigget(signal, sig) ((signal) &  (sig))

void task_add_to_delayed_list(tcb_t *tcb, time_t ticks_to_delay);
void task_add_to_block_list(tcb_t *tcb);
void task_send_signal(tcb_t *tcb, ubase_t sig);

#ifdef __cplusplus
}
#endif

#endif /* CO_ROUTINE_H */
