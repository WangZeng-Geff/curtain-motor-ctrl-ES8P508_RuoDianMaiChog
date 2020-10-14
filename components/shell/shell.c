#include <utils.h>
#include <os.h>
#include <printk.h>
#include <init.h>
#include "cmds.h"
#include "shell.h"

#ifdef configUSING_SHELL

#define PROMPT "sh >"
typedef int (*cmd_function_t)(int argc, char **argv);

#if defined(__ICCARM__)
# pragma section="FSymTab"
#endif

/* shell thread */
static tcb_t shell_tcb;

static char line[SHELL_CMD_SIZE];
static u8 line_position;

struct shell_syscall *_syscall_table_begin  = NULL;
struct shell_syscall *_syscall_table_end    = NULL;

static int shell_args_split(char *cmd, size_t length, char *argv[SHELL_ARG_NR])
{
    char *ptr;
    size_t position, argc;

    ptr = cmd;
    position = 0;
    argc = 0;

    while (position < length)
    {
        /* strip bank and tab */
        while ((*ptr == ' ' || *ptr == '\t') && position < length)
        {
            *ptr = '\0';
            ptr ++;
            position ++;
        }
        if (position >= length) break;

        /* handle string */
        if (*ptr == '"')
        {
            ptr ++;
            position ++;
            argv[argc] = ptr;
            argc ++;

            /* skip this string */
            while (*ptr != '"' && position < length)
            {
                if (*ptr == '\\')
                {
                    if (*(ptr + 1) == '"')
                    {
                        ptr ++;
                        position ++;
                    }
                }
                ptr ++;
                position ++;
            }
            if (position >= length) break;

            /* skip '"' */
            *ptr = '\0';
            ptr ++;
            position ++;
        }
        else
        {
            argv[argc] = ptr;
            argc ++;
            while ((*ptr != ' ' && *ptr != '\t') && position < length)
            {
                ptr ++;
                position ++;
            }
            if (position >= length) break;
        }
    }

    return argc;
}

static cmd_function_t shell_get_cmd(char *cmd, int size)
{
    struct shell_syscall *index;

    for (index = _syscall_table_begin; index < _syscall_table_end; index++)
    {
        if (strncmp(index->name, cmd, size) == 0 && index->name[size] == '\0')
            return (cmd_function_t)index->func;
    }

    return NULL;
}

static int shell_exec_cmd(char *cmd, size_t length, int *retp)
{
    int argc;
    int cmd0_size = 0;
    cmd_function_t cmd_func;
    char *argv[SHELL_ARG_NR];

    /* find the size of first command */
    while ((cmd[cmd0_size] != ' ' && cmd[cmd0_size] != '\t') && cmd0_size < length)
        cmd0_size ++;

    if (cmd0_size == 0) return -EINVAL;

    cmd_func = shell_get_cmd(cmd, cmd0_size);
    if (!cmd_func) return -EINVAL;

    /* split arguments */
    memset(argv, 0x00, sizeof(argv));
    argc = shell_args_split(cmd, length, argv);
    if (argc == 0) return -EINVAL;

    /* exec this command */
    *retp = cmd_func(argc, argv);
    return 0;
}

static int shell_exec(char *cmd, size_t length)
{
    int cmd_ret;

    /* strim the beginning of command */
    while (*cmd  == ' ' || *cmd == '\t')
    {
        cmd++;
        length--;
    }

    if (length == 0) return 0;

    if (shell_exec_cmd(cmd, length, &cmd_ret) == 0)
        return cmd_ret;

    /* truncate the cmd at the first space. */
    {
        char *tcmd = cmd;
        while (*tcmd != ' ' && *tcmd != '\0')
            tcmd++;
        *tcmd = '\0';
    }
    printk("%s: command not found.\r\n", cmd);
    return -1;
}

static int str_common(const char *str1, const char *str2)
{
    const char *str = str1;

    while ((*str != 0) && (*str2 != 0) && (*str == *str2))
    {
        str ++;
        str2 ++;
    }

    return (str - str1);
}
static void do_auto_complete(char *prefix)
{
    int length, min_length;
    const char *name_ptr, *cmd_name;
    struct shell_syscall *index;

    min_length = 0;
    name_ptr = NULL;

    if (*prefix == '\0')
    {
        msh_help(0, NULL);
        return;
    }

    for (index = _syscall_table_begin; index < _syscall_table_end; index++)
    {
        cmd_name = (const char *)index->name;
        if (!strncmp(prefix, cmd_name, strlen(prefix)))
        {
            if (min_length == 0)
            {
                name_ptr = cmd_name;
                min_length = strlen(name_ptr);
            }

            length = str_common(name_ptr, cmd_name);
            if (length < min_length)
                min_length = length;

            printk("%s\r\n", cmd_name);
        }
    }

    if (name_ptr) strncpy(prefix, name_ptr, min_length);
}

static void shell_auto_complete(char *prefix)
{
    printk("\r\n");
    do_auto_complete(prefix);
    printk("%s%s", PROMPT, prefix);
}

void shell_get_chr(char ch)
{
    switch (ch)
    {
    case '\t':   /* TAB */
    {
        int i;

        for (i = 0; i < line_position; i++)
            printk("\b");

        shell_auto_complete(&line[0]);
        line_position = strlen(line);

        break;
    }
    case 0x08:  /* Backspace */
    case 0x7F:   /* Delete */
    {
        if (line_position == 0)
            break;

        line_position--;

        printk("\b \b");
        line[line_position] = 0;
        break;
    }
    case '\n':
        break;
    case '\r':
    {
        printk("\r\n");

        task_send_signal(&shell_tcb, SIG_DATA);
        break;
    }
    default:
    {
        /* it's a large line, discard it */
        if (line_position >= SHELL_CMD_SIZE)
            line_position = 0;

        /* normal character */
        line[line_position] = ch;
        printk("%c", ch);

        ch = 0;
        line_position++;
        if (line_position >= SHELL_CMD_SIZE)
            line_position = 0;
        break;
    }
    } /* switch(ch) */
}

static void shell_entry(tcb_t *tcb, ubase_t data)
{
    tSTART(tcb);

    printk(PROMPT);

    for (;;)
    {
        task_wait_signal(tcb);

        sig_t sig = task_signal(tcb);

        if (sigget(sig, SIG_DATA))
        {
            shell_exec(line, line_position);
            printk(PROMPT);
            memset(line, 0, sizeof(line));
            line_position = 0;
        }
    }
    tEND();
}

static void shell_system_function_init(const void *begin, const void *end)
{
    _syscall_table_begin = (struct shell_syscall *)begin;
    _syscall_table_end   = (struct shell_syscall *)end;
}

int shell_system_init(void)
{
#if defined(__CC_ARM)
    extern const int FSymTab$$Base;
    extern const int FSymTab$$Limit;
    shell_system_function_init(&FSymTab$$Base, &FSymTab$$Limit);
#elif defined(__ICCARM__)
    shell_system_function_init((const void *)__section_begin("FSymTab"),
                               (const void *)__section_end("FSymTab"));
#endif

    task_create(&shell_tcb, shell_entry, 0);

    return 0;
}
device_initcall(shell_system_init);
#endif
