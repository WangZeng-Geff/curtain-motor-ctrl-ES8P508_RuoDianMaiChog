#include <config.h>
#include <croutine.h>
#include "port.h"
#include "jiffies.h"

static LIST_HEAD(task_list);
static tcb_t *current;

#define corINITIAL_STATE    (0)

static void check_delayed_list(void)
{
    tcb_t *iter;
    OS_CPU_SR cpu_sr;

    list_for_each_entry(iter, &task_list, entry)
    {
        if (iter->status == TS_DELAY && time_after_eq(jiffies, iter->expire))
        {
            enter_critical();
            iter->status = TS_READY;
            exit_critical();
        }
    }
}

void task_create(tcb_t *tcb, void (*tcb_cb)(struct task_ctrl_blk *, ubase_t), ubase_t data)
{
    tcb->state  = corINITIAL_STATE;
    tcb->data   = data;
    tcb->tcb_cb = tcb_cb;

    list_add_tail(&tcb->entry, &task_list);
}

void task_add_to_delayed_list(tcb_t *tcb, time_t ticks_to_delay)
{
    OS_CPU_SR cpu_sr;

    enter_critical();
    tcb->expire = jiffies + ticks_to_delay;
    tcb->status = TS_DELAY;
    exit_critical();
}

void task_add_to_block_list(tcb_t *tcb)
{
    OS_CPU_SR cpu_sr;

    enter_critical();
    tcb->signal = 0;
    tcb->status = TS_BLOCK;
    exit_critical();
}

void task_send_signal(tcb_t *tcb, ubase_t sig)
{
    OS_CPU_SR cpu_sr;

    enter_critical();
    tcb->signal |= sig;
    tcb->status  = TS_READY;
    exit_critical();
}

ubase_t task_signal(tcb_t *tcb)
{
    ubase_t sig;
    OS_CPU_SR cpu_sr;

    enter_critical();
    sig = tcb->signal;
    tcb->signal = 0;
    exit_critical();
    return sig;
}

tcb_t *task_get_current_task_handle(void)
{
    return current;
}

void task_schedule(void)
{
    tcb_t *n;
    check_delayed_list();

    list_for_each_entry_safe(current, n, &task_list, entry)
    {
        if (current->status == TS_READY)
        {
            current->tcb_cb(current, current->data);
        }
    }
}

