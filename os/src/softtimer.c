#include <config.h>
#include <utils.h>
#include <port.h>
#include <printk.h>
#include <jiffies.h>
#include <croutine.h>
#include <board.h>
#include <init.h>
#include "softtimer.h"


static LIST_HEAD(timer_list);

void soft_timer_add(struct soft_timer *st)
{
    OS_CPU_SR cpu_sr;

    enter_critical();
	if (!(st->entry.next && st->entry.prev))
		list_add_tail(&st->entry, &timer_list);
    exit_critical();
}

void soft_timer_del(struct soft_timer *st)
{
    OS_CPU_SR cpu_sr;

    enter_critical();
	if (st->entry.next && st->entry.prev)
		list_del(&st->entry);
    exit_critical();
}

void soft_timer_mod(struct soft_timer *st, time_t expires)
{
    OS_CPU_SR cpu_sr;

    enter_critical();
    list_del(&st->entry);
    st->expires = expires;
    list_add_tail(&st->entry, &timer_list);
    exit_critical();
}

static void soft_timer_task_callback(struct task_ctrl_blk *tcb, ubase_t data)
{
    tSTART(tcb);

    for (;;)
    {
        struct soft_timer *iter, *n;

        list_for_each_entry_safe(iter, n, &timer_list, entry)
        {
            if (time_after_eq(jiffies, iter->expires))
            {
                soft_timer_del(iter);

                if (iter->cb) iter->cb(iter);
									
            }
        }
        task_delay(tcb, 1);
    }

    tEND();
}

static tcb_t soft_timer_crcb;

int setup_soft_timer_service(void)
{
    task_create(&soft_timer_crcb, soft_timer_task_callback, 0);

    return 0;
}
core_initcall(setup_soft_timer_service);
