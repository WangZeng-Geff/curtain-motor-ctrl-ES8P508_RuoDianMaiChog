#ifndef __SHELL_H__
#define __SHELL_H__

#include <compiler.h>
#include <types.h>
#include <os.h>
#include <device.h>

#define SHELL_PRIORITY          (configMAX_PRIORITIES - 1)       /* highest priority */
#define SHELL_STACK_SIZE        (configMINIMAL_STACK_SIZE + 100)

#define SHELL_CMD_SIZE      80
#define SHELL_ARG_NR        10

typedef long(*syscall_func_t)(void);

struct shell_syscall
{
    const char      *name;       /* the name of system call */
    const char      *desc;       /* description of system call */
    syscall_func_t   func;       /* the function address of system call */
};
extern struct shell_syscall *_syscall_table_begin, *_syscall_table_end;

#define SHELL_CMD_EXPORT(name, cmd, desc)      \
    const char __fsym_##cmd##_name[] = #cmd;   \
    const char __fsym_##cmd##_desc[] = desc;  \
    const struct shell_syscall __fsym_##cmd USED SECTION(FSymTab)= \
    {                           \
        __fsym_##cmd##_name,    \
        __fsym_##cmd##_desc,    \
        (syscall_func_t)&name   \
    }

int shell_system_init(void);

#endif
