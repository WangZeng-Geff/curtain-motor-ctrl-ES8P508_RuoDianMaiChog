#ifndef _DEV_SHOW_H_
#define _DEV_SHOW_H_

#include <stdint.h>
#include "proto_smart.h"

#define DEV_SHOW

#ifdef DEV_SHOW

#define TIME_LEN                 0x02
#define TYPE_LEN                 0x04
#define DID_LEN                  0x02
#define CMD_SHOW                 0x03

struct DEV_SEARCH_PARAM
{
	uint8_t   dev_show_flag;
	uint8_t   ack_seq;
	uint16_t  ack_count;
	uint8_t   search_id[AID_LEN];
	uint16_t  silent_time;
    uint16_t  max_delay_time;
	//uint32_t  cur_time;
};
extern struct DEV_SEARCH_PARAM dev_search_param;

uint8_t search_frame_opt(struct SmartFrame *pframe);
void check_dev_show(void);
void check_search_delay(void);

#endif

#endif

