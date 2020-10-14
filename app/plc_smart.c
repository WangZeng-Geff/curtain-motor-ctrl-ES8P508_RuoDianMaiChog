#include <config.h>
#include <utils.h>
#include <os.h>
#include <device.h>
#include <printk.h>
#include <syserr.h>
#include "plc_smart.h"
#include "proto_smart.h"
#include "dev.h"
#include "settings.h"
#include "report\\auto_report_app.h"

#define STATE_TIMER_QUEUE_WAIT  100         /* wait 100 ms max while timer task busy */
#define SIGSTACHG               SIG_USR1    /* state change */

static device_t *serial_plc;
static tcb_t plc_tcb;
static struct soft_timer state_machine_timer;

uint8_t g_frame_buf[1024];
//---------------------------------------------------------------------------------------
enum
{
    STATE_CTR_NEXT  = -1,     /* change to next state */
    STATE_CTR_THIRD = -2,     /* change to next state */
    STATE_CTR_WAIT  = -3,     /* wait for data event */
    STATE_CTR_RETRY = -4,     /* retry current state */
    STATE_CTR_CHAGE = -5,     /* change to other state */
};
//---------------------------------------------------------------------------------------
typedef struct 
{
    state_machine_state_t state, next_state;
    const char *name;
    time_t wait;
    base_t (*op0)(uint8_t *out, size_t maxlen);
    base_t (*op1)(const void *arg);
} state_machine_state_op_t;

typedef struct _state_machine
{
    int cur_state, trycnt;
    uint32_t init;
    const state_machine_state_op_t *op;
} state_machine_t;
static state_machine_t state_machine;
//---------------------------------------------------------------------------------------

static base_t local_ack_opt(struct SmartFrame *pframe)
{
    if (!pframe)    /* time out */
        return STATE_CTR_RETRY;

    if (!smart_frame_is_local(pframe)) /* got a dummy frame */
        return STATE_CTR_WAIT;

    uint8_t cmd = pframe->data[0];
    if (cmd == CMD_ACK || cmd == CMD_NAK)
    {
        return STATE_CTR_NEXT;
    }
    return STATE_CTR_WAIT;
}

/*
 * reset plc
 */
extern void plc_reset_set(int state);
static base_t do_rst_plc0(uint8_t *out, size_t maxlen)
{
	plc_reset_set(0);
    return 0;
}
static base_t do_rst_plc1(const void *arg)
{
	plc_reset_set(1);
    return STATE_CTR_NEXT;
}

/*
 * get eid
 */
static base_t do_get_eid0(uint8_t *out, size_t maxlen)
{
    uint8_t cmd = CMD_GET_EID;

    return code_local_frame(&cmd, sizeof(cmd), out, maxlen);
}

static base_t check_sole_encode(void)
{
	if (!is_all_xx(setting.encode.id, 0x00, AID_LEN) \
        && !is_all_xx(setting.encode.id, 0xFF, AID_LEN))
		return (1);
	return (0);
}

static base_t do_get_eid1(const void *arg)
{
    struct SmartFrame *pframe;

    pframe = (struct SmartFrame *)arg;
    if (pframe && pframe->data[0] == CMD_ACK_EID)
    {
		if (!check_sole_encode())
		{
			memcpy(setting.encode.id, &pframe->data[1], sizeof(setting.encode.id));
			setting_save();
		}
        return STATE_CTR_NEXT;
    }
    return STATE_CTR_RETRY;
}

/*
 * set plc aid
 */
static base_t do_set_aid0(uint8_t *out, size_t maxlen)
{
    uint8_t tmp[5] = {0};
    tmp[0] = CMD_SET_AID;

    memcpy(&tmp[1], setting.encode.id, sizeof(setting.encode.id));
    return code_local_frame(tmp, sizeof(tmp), out, maxlen);
}
static base_t do_set_aid1(const void *arg)
{
    return local_ack_opt((struct SmartFrame *)arg);
}

#ifdef configUSING_UPWAY
/*
 * set update way (put the program in the plc)
 */
static base_t do_set_upway0(uint8_t *out, size_t maxlen)
{
    uint8_t tmp[2] = {0x22, 0x01};

    return code_local_frame(tmp, sizeof(tmp), out, maxlen);
}
static base_t do_set_upway1(const void *arg)
{
    return local_ack_opt((struct SmartFrame *)arg);
}
#endif
/*
 * notify plc to unlink
 */
static base_t do_set_unlink0(uint8_t *out, size_t maxlen)
{
    uint8_t cmd = CMD_UNLINK;

    return code_local_frame(&cmd, sizeof(cmd), out, maxlen);
}
static base_t do_set_unlink1(const void *arg)
{
    return local_ack_opt((struct SmartFrame *)arg);
}

/*
 * wait one second
 */
static base_t do_wait_sec0(uint8_t *out, size_t maxlen)
{
    return 0;
}
static base_t do_wait_sec1(const void *arg)
{
    return STATE_CTR_NEXT;
}

/*
 * set register
 */
struct reg reg;
static base_t do_set_reg0(uint8_t *out, size_t maxlen)
{
    uint8_t tmp[2] = {0};

    tmp[0] = CMD_SET_REG;
    tmp[1] = reg.type | reg.last_status;
    reg.last_status = 0;
    return code_local_frame(tmp, sizeof(tmp), out, maxlen);
}
static base_t do_set_reg1(const void *arg)
{
    return local_ack_opt((struct SmartFrame *)arg);
}

/*
 * set panid
 */
static base_t do_set_panid0(uint8_t *out, size_t maxlen)
{
    if (is_all_xx(setting.para.panid, 0x00, sizeof(setting.para.panid)) \
		|| is_all_xx(setting.para.panid, 0xFF, sizeof(setting.para.panid)))
		return STATE_CTR_NEXT;

    uint8_t tmp[3] = {0};

    tmp[0] = CMD_SET_PANID;
    memcpy(&tmp[1], setting.para.panid, sizeof(setting.para.panid));
    return code_local_frame(tmp, sizeof(tmp), out, maxlen);
}
static base_t do_set_panid1(const void *arg)
{
    return local_ack_opt((struct SmartFrame *)arg);
}

/*
 * get gateway aid
 */
static base_t do_get_gid0(uint8_t *out, size_t maxlen)
{
    uint8_t cmd = CMD_GET_GWAID;

    return code_local_frame(&cmd, sizeof(cmd), out, maxlen);
}
static base_t do_get_gid1(const void *arg)
{
    if (!arg)    /* time out */
        return STATE_CTR_RETRY;

    struct SmartFrame *pframe = (struct SmartFrame *)arg;

    if (smart_frame_is_local(pframe) && pframe->data[0] == CMD_GET_GWAID)
    {
        uint8_t *gid = &pframe->data[1];
        if (memcmp(setting.para.gid, gid, sizeof(setting.para.gid)))
        {
            memcpy(setting.para.gid, gid, sizeof(setting.para.gid));
            setting_save();
        }
        return STATE_CTR_NEXT;
    }
    return STATE_CTR_WAIT;
}
/*
 * get short id
 */
static base_t do_get_sid0(uint8_t *out, size_t maxlen)
{
    uint8_t cmd = CMD_GET_SID;

    return code_local_frame(&cmd, sizeof(cmd), out, maxlen);
}
static base_t do_get_sid1(const void *arg)
{
    if (!arg)    /* time out */
        return STATE_CTR_RETRY;

    struct SmartFrame *pframe = (struct SmartFrame *)arg;

    if (smart_frame_is_local(pframe) && pframe->data[0] == CMD_ACK_SID)
    {
        uint8_t *sid = &pframe->data[1];
        if (memcmp(setting.para.sid, sid, sizeof(setting.para.sid)))
        {
            memcpy(setting.para.sid, sid, sizeof(setting.para.sid));
            setting_save();
        }
				register_report(setting.para.gid);
        return STATE_CTR_NEXT;
    }
    return STATE_CTR_WAIT;
}

/*
 * do idle
 */
static base_t do_end0(uint8_t *out, size_t maxlen)
{
    return 0;
}
static base_t do_end1(const void *arg)
{
    extern int smart_frame_handle(struct SmartFrame * pframe);

    int ret = smart_frame_handle((struct SmartFrame *)arg);
    if (ret > 0)
    {
        device_write(serial_plc, 0, arg, ret);
    }
    return STATE_CTR_WAIT;
}
//---------------------------------------------------------------------------------------
#define state_machine_state(state, next_state, wait, handler) \
{state, next_state, #state, wait, handler##0, handler##1}

static const state_machine_state_op_t state_machine_states[] =
{
    state_machine_state(STATE_RST_PLC, STATE_GET_EID,  pdMS_TO_TICKS(1000), do_rst_plc),
    state_machine_state(STATE_GET_EID, STATE_SET_AID,  pdMS_TO_TICKS(2000), do_get_eid),
#ifdef configUSING_UPWAY
    state_machine_state(STATE_SET_AID, STATE_SET_UWY,  pdMS_TO_TICKS(1000), do_set_aid),
	state_machine_state(STATE_SET_UWY, STATE_UNLINK,   pdMS_TO_TICKS(1000), do_set_upway),
#else
	state_machine_state(STATE_SET_AID, STATE_UNLINK,   pdMS_TO_TICKS(1000), do_set_aid),
#endif
    state_machine_state(STATE_UNLINK,  STATE_WAIT,     pdMS_TO_TICKS(2000), do_set_unlink),
    state_machine_state(STATE_WAIT,    STATE_SET_REG,  pdMS_TO_TICKS(1000), do_wait_sec),
    state_machine_state(STATE_SET_REG, STATE_SET_PID,  pdMS_TO_TICKS(2000), do_set_reg),
    state_machine_state(STATE_SET_PID, STATE_IDLE,     pdMS_TO_TICKS(2000), do_set_panid),

    state_machine_state(STATE_GET_GID, STATE_SET_REG1, pdMS_TO_TICKS(2000), do_get_gid),
    state_machine_state(STATE_SET_REG1,STATE_GET_SID,  pdMS_TO_TICKS(2000), do_set_reg),
    state_machine_state(STATE_GET_SID, STATE_IDLE,     pdMS_TO_TICKS(2000), do_get_sid),

    state_machine_state(STATE_IDLE,    STATE_IDLE,     0,					do_end),
};

static const state_machine_state_op_t *get_state_op(state_machine_state_t state)
{
    int i;

    for (i = 0; i < array_size(state_machine_states); i++) 
    {
        if (state_machine_states[i].state == state)
            return &state_machine_states[i];
    }
    return NULL;
}
//---------------------------------------------------------------------------------------
int is_state_machine_idle(void)
{
    return state_machine.cur_state == STATE_IDLE;
}
//---------------------------------------------------------------------------------------
void state_machine_change(state_machine_state_t state)
{
    const state_machine_state_op_t *op = get_state_op(state);
    if (op)
    {
        state_machine_t *sm = &state_machine;

#ifdef PLC_SMART_DEBUG
        log_d("PLC: change state to %s\r\n", op->name);
#endif

        sm->init = 1;
        if (sm->cur_state != state) 
        {
            sm->cur_state = state;
            sm->trycnt    = 0;
            sm->op        = op;
        }

        task_send_signal(&plc_tcb, SIGSTACHG);
    }
}
//---------------------------------------------------------------------------------------
static void state_machine_timer_handle(struct soft_timer *st)
{
    task_send_signal(&plc_tcb, SIG_ALARM);
}
//---------------------------------------------------------------------------------------
static void state_machine_schedule(void *arg)
{
    int ret;
    state_machine_t *sm = &state_machine;

redo:
    if (sm->init)
    {
        sm->init = 0;
        ret = sm->op->op0(g_frame_buf, sizeof(g_frame_buf));
        if (ret > 0)
        {
            device_write(serial_plc, 0, g_frame_buf, ret);
        }
        if (sm->op->wait == 0) goto redo;

        soft_timer_mod(&state_machine_timer, jiffies + sm->op->wait);
    }
    else
    {
        ret = state_machine.op->op1(arg);
    }

    if (ret == STATE_CTR_NEXT)
    {
        soft_timer_del(&state_machine_timer);
        state_machine_change(state_machine.op->next_state);
    }
    else if (ret == STATE_CTR_RETRY)
    {
        state_machine.trycnt++;
        state_machine_change(state_machine.op->state);
    }
}

//---------------------------------------------------------------------------------------
static err_t serial_plc_open(void)
{
    err_t err;
    struct serial_configure cfg;

    serial_plc = device_find("uart2");
    assert(serial_plc);

    err = device_ctrl(serial_plc, SERIAL_CTRL_GETCFG, &cfg);
    if (err != 0) return err;

    cfg.baud_rate = BAUD_RATE_9600;
    cfg.bufsz     = SIZE_1K;
    cfg.data_bits = DATA_BITS_8;
    cfg.parity    = PARITY_NONE;
    cfg.stop_bits = STOP_BITS_1;
    err = device_ctrl(serial_plc, SERIAL_CTRL_SETCFG, &cfg);
    if (err != 0) return err;

    err = device_open(serial_plc, DEVICE_FLAG_INT_RX | DEVICE_FLAG_INT_TX | DEVICE_FLAG_FASYNC);
    if (err != 0) return err;

    return 0;
}
//----------------------------------------------------------------------------------------

static size_t check_smart_frame(uint8_t *data, size_t len)
{
    smart_frame_t *pframe = get_smart_frame(data, len);
    if (!pframe) return 0;

    len = smart_frame_len(pframe);
#ifdef PLC_SMART_DEBUG
    log_d("got smart frame:\n");
    log_arr_d((uint8_t *)pframe, len);
#endif
    state_machine_schedule(pframe);
    len += ((uint8_t *)pframe - data);

    return len;
}

static void do_uart_data(void)
{
    int len = device_peek(serial_plc, 0, g_frame_buf, sizeof(g_frame_buf));
    if (len)
    {
		int ret = check_smart_frame(g_frame_buf, len);
		if (ret)
		{
			device_read(serial_plc, 0, g_frame_buf, ret);
			if (len > ret)
				task_send_signal(&plc_tcb, SIG_DATA);
		}
		else
		{
			if (len > 0xFF)
				device_read(serial_plc, 0, g_frame_buf, len);
		}
    }
}

void do_send_ctrl_frame(uint8_t idx)
{
	if(!is_all_xx(setting.ctrl[idx].aid, 0x00, AID_LEN))
	{
        uint8_t body[4];

		uint8_t data = 0x3F&setting.ctrl[idx].chn;
        int len = code_body(0xC018, 0, &data, 1, body, sizeof(body));
		len = code_frame(setting.encode.id, setting.ctrl[idx].aid, -1, CMD_SET, body, len, g_frame_buf, sizeof(g_frame_buf));
		device_write(serial_plc, 0, g_frame_buf, len);
	}
	else if(!is_all_xx(setting.para.gid, 0x00, AID_LEN))
	{
		static uint8_t report_state[4] = {0x00,0x00,0x00,0x00};
		uint8_t body[7], data[4] = {0x00,0x00,0x00,0x00};

		report_state[idx] ^= 0x01;
		data[1] = idx + 1;
		data[3] = report_state[idx];

        int len = code_body(0xC062, 0, data, sizeof(data), body, sizeof(body));
		len = code_frame(setting.encode.id, setting.para.gid, -1, CMD_NRELI_REPORT, body, len, g_frame_buf, sizeof(g_frame_buf));
		device_write(serial_plc, 0, g_frame_buf, len);
	}
}

static void plc_task_cb(struct task_ctrl_blk *tcb, ubase_t data)
{
    tSTART(tcb);

    serial_plc_open();
    for (;;)
    {
        task_wait_signal(tcb);

        sig_t sig = task_signal(tcb);

        /* check state change or time out event */
        if (sigget(sig, SIGSTACHG) || sigget(sig, SIG_ALARM))
            state_machine_schedule(NULL);
		/* check recvd data event */
        if (sigget(sig, SIG_DATA))
            do_uart_data();
    }

    tEND();
}

void plc_init(void)
{
    memset(&state_machine_timer, 0, sizeof(state_machine_timer));

    state_machine_timer.cb = state_machine_timer_handle;
    state_machine_change(STATE_RST_PLC);
    task_create(&plc_tcb, plc_task_cb, 0);
}


