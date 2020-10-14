#ifndef _TYPES_H
#define _TYPES_H

#include <stddef.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>


#define SIZE_1K         (1024u)
#define SIZE_1M         (1024*1024u)

typedef long            base_t;      /* Nbit CPU related date type */
typedef unsigned long   ubase_t;     /* Nbit unsigned CPU related data type */

typedef ubase_t         sig_t;
typedef ubase_t         time_t;
typedef base_t          err_t;
typedef base_t          off_t;

typedef uint8_t         u8;
typedef uint16_t        u16;
typedef uint32_t        u32;
typedef int8_t          s8;
typedef int16_t         s16;
typedef int32_t         s32;

enum
{
    SIG_ALARM = 1 << 0,
    SIG_DATA  = 1 << 1,
    SIG_KEY   = 1 << 2,

    SIG_USR1  = 1 << 8,
    SIG_USR2  = 1 << 9,
};

typedef enum
{
    COLOR_R,
    COLOR_G,
    COLOR_B,
    COLOR_NR,
} color_t;

typedef enum
{
    DIR_L,          /* left  */
    DIR_R,          /* right */
    DIR_NR,
} dir_t;

typedef struct 
{
    size_t len;
    void *data;
} data_t;

typedef struct list_head
{
    struct list_head *next, *prev;
} list_t;

typedef struct object
{
    const char *name;
    struct list_head entry;
} object_t;

/**
 * block device geometry structure
 */
struct device_blk_geometry
{
    uint32_t sector_count;                           /**< count of sectors */
    uint32_t bytes_per_sector;                       /**< number of bytes per sector */
    uint32_t block_size;                             /**< number of bytes to erase one block */
};
#endif  /* end of _TYPES_H */
