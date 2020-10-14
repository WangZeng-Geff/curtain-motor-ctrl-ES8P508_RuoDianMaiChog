#ifndef _COMPILER_H
#define _COMPILER_H

#ifdef __CC_ARM                         /* ARM Compiler */
# include <stdarg.h>
# define SECTION(x)                     __attribute__((section(#x)))
# define UNUSED                         __attribute__((unused))
# define USED                           __attribute__((used))
# define ALIGN(n)                       __attribute__((aligned(n)))
# define WEAK                           __weak
//# define inline                         static __inline

#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
# include <stdarg.h>
# define SECTION(x)                     @ #x
# define UNUSED
# define USED
# define PRAGMA(x)                      _Pragma(#x)
# define ALIGN(n)                       PRAGMA(data_alignment=n)
# define WEAK                           __weak
//# define inline                         static inline

#elif defined (__GNUC__)                /* GNU GCC Compiler */
# ifdef USING_NEWLIB
#  include <stdarg.h>
# else
/* the version of GNU GCC must be greater than 4.x */
typedef __builtin_va_list               __gnuc_va_list;
typedef __gnuc_va_list                  va_list;
#  define va_start(v,l)                 __builtin_va_start(v,l)
#  define va_end(v)                     __builtin_va_end(v)
#  define va_arg(v,l)                   __builtin_va_arg(v,l)
# endif

# define SECTION(x)                     __attribute__((section(#x)))
# define UNUSED                         __attribute__((unused))
# define USED                           __attribute__((used))
# define ALIGN(n)                       __attribute__((aligned(n)))
# define WEAK                           __attribute__((weak))
//# define inline                         static __inline
#else
# error "not supported tool chain"
#endif

#endif
