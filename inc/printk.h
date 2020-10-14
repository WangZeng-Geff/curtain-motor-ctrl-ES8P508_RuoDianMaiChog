#ifndef _PRINTK_H_
#define _PRINTK_H_

#include <types.h>

#ifdef USING_EASYLOG
#include "elog.h"

/*
 * log_d, log_arr_d: debug
 *
 * note: append newline character automatically
 */

#define log_d(...) elog_d(__VA_ARGS__)
#define log_arr_d(arr, len) elog_arr_d((arr), (len))

#define print_debug_array(arr, len) \
    elog_array(__FILE__, __func__, __LINE__, (arr), (len))

#else

# ifdef configUSING_DEBUG
#define log_d(...) printk(__VA_ARGS__)
#define log_arr_d(arr, len) pr_arr((arr), (len))
# else
#define log_d(...) ((void)0)
#define log_arr_d(arr, len) ((void)0)
# endif

void pr_arr(const void *arr, size_t len);
#endif

void BUG(void);

#ifndef configUSING_DEBUG
# define assert(p) ((void)0)
#else
# define assert(p) do { \
    if (!(p)) { \
        log_d("BUG at assert(%s),%s-line:%d\n", #p, __FILE__, __LINE__); \
        BUG(); \
    }       \
 } while (0)
#endif

int32_t os_snprintf(char *buf, size_t size, const char *fmt, ...);
int32_t os_sprintf(char *buf, const char *format, ...);
int32_t os_vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int32_t os_vsprintf(char *buf, const char *format, va_list arg_ptr);
void printk(const char *fmt, ...);

#define snprintf  os_snprintf
#define sprintf   os_sprintf
#define vsnprintf os_vsnprintf
#define vsprintf  os_vsprintf

/*
 * initialize log
 */
int setup_print(void);
 
#endif
