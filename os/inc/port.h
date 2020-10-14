#ifndef _PORT_H_
#define _PORT_H_

#include <types.h>
/* please change the .h file relay on u mcu */
#include "ES8P508x.h"
//#include "HR8P506.h"
//#include "HR8P296.h"

#define enable_irq()      __enable_irq()
#define disable_irq()     __disable_irq()

#define portBYTE_ALIGNMENT      (4)
#define portBYTE_ALIGNMENT_MASK (0x03)

#define OS_CPU_SR   uint32_t
#define enter_critical()        \
    do { cpu_sr = __get_PRIMASK(); disable_irq();} while (0)
#define exit_critical()         \
    do { __set_PRIMASK(cpu_sr);} while (0)

#define portDISABLE_INTERRUPTS __disable_irq

int setup_tick_service(void);

void console_write_bytes(const void *in, size_t n);
void setup_console(void);

#endif

