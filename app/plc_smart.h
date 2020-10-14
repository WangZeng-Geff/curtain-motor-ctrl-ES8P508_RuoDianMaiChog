#ifndef __PLC_SMART_H__
#define __PLC_SMART_H__

#include "types.h"

//#define PLC_SMART_DEBUG

#define STATE_MACHINE_STATE_WAIT        200 //2s
#define STATE_MACHINE_CACHE_SIZE        0x100

typedef enum
{
    STATE_IDLE,
    STATE_RST_PLC,
    STATE_GET_EID,
    STATE_SET_AID,
	STATE_SET_UWY,
    STATE_UNLINK,
    STATE_WAIT,
    STATE_SET_REG,
    STATE_SET_PID,
    STATE_GET_GID,
    STATE_GET_SID,
    STATE_SET_REG1,
} state_machine_state_t;


struct reg
{
    uint8_t type;
    uint8_t last_status;
};
extern struct reg reg;
extern uint8_t g_frame_buf[1024];

void plc_init(void);
void state_machine_change(state_machine_state_t state);

#endif
