#ifndef __UPDATE_H__
#define __UPDATE_H__

#include <types.h>

#pragma pack(1)
typedef struct
{
    u16 seq;
    u8  ack;
    u16 crc;
    u8  len;
    u8  data[0];
} up_hdr_t;

typedef struct
{
    u32 size;
    u16 crc;
    u8  block_size;
    u8  type[8];
    u8  data[0];
} update_file_t;

#pragma pack()

typedef struct
{
    u32 size;
    u16 max_blk_cnt;
    u16 crc;
    u16 block_no;
    u16 block_size;
    u8 ver[0x20];
} update_cache_t;

int update_frame_opt(u8 *data, int len);
#endif
