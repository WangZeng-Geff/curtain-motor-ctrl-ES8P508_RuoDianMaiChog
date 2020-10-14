
#include <config.h>
#include <compiler.h>
#include <types.h>
#include <utils.h>
#include <printk.h>
#include "dev.h"
#include "settings.h"
#include "board.h"
#include "update.h"
#include "proto_smart.h"
#include "plc_smart.h"
#include "dev_show.h"
#include "report\\auto_report_app.h"
#include "curtain_ctl_gpio.h"
/*
 * frame decode
 */
/**************************************************************************
  * @brief	   : 解析一帧报文
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
smart_frame_t * get_smart_frame(const uint8_t * in, uint32_t len)
{
	int i = 0;


start_lbl:

	while (i < len)
	{
		if (STC == in[i])
			break;

		i++;
	}

	if (len - i < SMART_FRAME_HEAD)
		return NULL;

	smart_frame_t * pframe = (struct SmartFrame *) &in[i];
	int dlen = pframe->len;

	if (i + SMART_FRAME_HEAD + dlen + 1 > len)
	{
		i++;
		goto start_lbl;
	}

	if (pframe->data[dlen] != checksum(pframe, dlen + SMART_FRAME_HEAD))
	{
		i++;
		goto start_lbl;
	}

	pframe = (struct SmartFrame *) &in[i];
	return pframe;
}


/**************************************************************************
  * @brief	   : 组一帧报文
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
int code_frame(const uint8_t * src, const uint8_t * dest, int seq, int cmd, 
	uint8_t * data, int len, void * out, int maxlen)
{
	const uint8_t addr[AID_LEN] =
	{
		0x00, 0x00, 0x00, 0x00
	};
	static uint8_t _seq = 0;

	struct SmartFrame * pframe = (struct SmartFrame *)
	out;
	memmove(&pframe->data[1], data, len);
	pframe->stc = STC;

	if (!src)
		src = addr;

	if (!dest)
		dest = addr;

	memcpy(pframe->said, src, AID_LEN);
	memcpy(pframe->taid, dest, AID_LEN);
	pframe->seq = seq < 0 ? (_seq ++ &0x7F): seq;
	pframe->data[0] = cmd;

	//	  memcpy(&pframe->data[1], data, len);
	pframe->len = len + 1;
	pframe->data[pframe->len] = checksum(pframe, SMART_FRAME_HEAD + pframe->len);
	return SMART_FRAME_HEAD + pframe->len + 1;
}


/**************************************************************************
  * @brief	   : 组一帧本地报文
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
int code_local_frame(const uint8_t * in, int len, void * out, int maxlen)
{
	return code_frame(NULL, NULL, 0, in[0], (uint8_t *) &in[1], len - 1, out, maxlen);
}


/**************************************************************************
  * @brief	   : 组一帧回复报文
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
int code_ret_frame(struct SmartFrame * pframe, int len)
{
	uint8_t tmp[AID_LEN];

	memcpy(tmp, pframe->taid, sizeof(pframe->taid));
	memcpy(pframe->taid, pframe->said, sizeof(pframe->taid));
	memcpy(pframe->said, tmp, sizeof(pframe->said));
	pframe->seq |= 0x80;
	pframe->len = len;
	pframe->data[len] = checksum(pframe, pframe->len + SMART_FRAME_HEAD);
	return pframe->len + SMART_FRAME_HEAD + 1;
}


/**************************************************************************
  * @brief	   : 组用户数据报文FBD
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
int code_body(uint16_t did, int err, const void * data, int len, void * out, int maxlen)
{
	body_t * body = (body_t *)

	out;
	put_le_val(did, body->did, sizeof(body->did));
	body->ctrl = get_bits(len, 0, 6);

	if (err)
		body->ctrl |= 0x80;

	memcpy(body->data, data, len);
	return sizeof(body_t) +len;
}


/*
 * doing protocol 
 */
/**************************************************************************
  * @brief	   : 设备类型
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
static int do_get_dev_type(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen)
{
	return dev_type_get(out, maxlen);
}


/**************************************************************************
  * @brief	   :  读取软件版本号 
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
static int do_get_soft_ver(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen)
{
	return dev_ver_get(out, maxlen);
}


/**************************************************************************
  * @brief	   : 读取DKEY
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
static int get_dev_key(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen)
{
	memcpy(out, setting.encode.dkey, sizeof(setting.encode.dkey));
	return sizeof(setting.encode.dkey);
}


/**************************************************************************
  * @brief	   : SN 
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
static int get_dev_sn(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen)
{
	memcpy(out, setting.encode.sn, sizeof(setting.encode.sn));
	return sizeof(setting.encode.sn);
}


/**************************************************************************
  * @brief	   : 通用 应用层通信协议 及版本
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
static int get_app_comm_ver(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen)
{
	uint8_t ver_len;
	const char ver[] = "EASTSOFT(v1.0)";

	ver_len = (char)
	strlen(ver);

	if (maxlen < ver_len)
		return (-BUFFER_ERR);

	memcpy(out, ver, ver_len);
	return (ver_len);
}


/**************************************************************************
  * @brief	   : 检查秘钥
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
static int check_password(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen)
{
	if (len != 2)
		return (-DATA_ERR);

	out[0] = !memcmp(setting.encode.pwd, in, 2);
	return (1);
}


#ifdef DEV_SHOW

extern struct MOTOR Motor1;
static int set_dev_show(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen)
{
	if (len)
		return (-DATA_ERR);

	dev_search_param.dev_show_flag = 0x01;
//	dev_show_start();

    SET_PIN_H(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
    SET_PIN_L(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
    SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
    relay_time = setting.dev_infor.actuation_time;

	return (NO_ERR);
}


static int get_password(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen)
{
	if (len != 0)
		return (-DATA_ERR);

	memcpy(out, setting.encode.pwd, PW_LEN);
	return (PW_LEN);
}


static int set_silent_time(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen)
{
	if (len != 2)
		return (-DATA_ERR);

	dev_search_param.silent_time = get_le_val(&in[0], TIME_LEN);
	return (NO_ERR);
}

static int get_relay_tim(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen)
{
	if (len != 0)
		return (-DATA_ERR);

	out[0] = setting.dev_infor.actuation_time;
	return 1;
}


static int set_relay_tim(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen)
{
	if (len != 1)
		return (-DATA_ERR);

	setting.dev_infor.actuation_time = in[0];
	dev_state.param_save = 1;
	return (NO_ERR);
}


#endif

typedef int(*func_opt_handler_t) (const uint8_t *, size_t, uint8_t *, size_t);


struct func_ops
{
u16 			did;
const char *	tip;
func_opt_handler_t read;
func_opt_handler_t write;
};
// 	{
// 		DID_REPORT, "report attr", get_report, set_report
// 	},
// 	{
// 		DID_SETTEMP, "set temp", get_temp_set, set_temp_set
// 	},
// 	{
// 		DID_SETRANGE, "set temp range", get_temp_range, set_temp_range
// 	},
// 	{
// 		DID_LOWPROTECT, "lowprotect", get_low_protect, set_low_protect
// 	},
// 	{
// 		DID_DEADBAND, "deadband", get_deadband, set_deadband
// 	},
// 	{
// 		DID_WINSPEED, "win speed", get_winspeed, set_winspeed
// 	},
// 	{
// 		DID_SETMODE, "mode", get_mode, set_mode
// 	},
// 	{
// 		DID_ONOFF, "switch", get_switch, set_switch
// 	},
// 	{
// 		DID_LOCK, "panel lock", get_panel_lock, set_panel_lock
// 	},
// 	{
// 		DID_WINONOFF, "auto win on/off", get_autowin_switch, set_autowin_switch
// 	},
// 	{
// 		DID_RUN_STATUS, "run status", get_run_status, set_run_status
// 	},
// 	{
// 		DID_TIME_CTRL, "time ctrl", get_time_ctrl, set_time_ctrl
// 	},
// 	{
// 		DID_SENSORVALUE, "sensor ", get_sensor_value, NULL
// 	},
// 	/*老指令*/
// 	{
// 		DID_SENSORVALUE2, "sensor2 ", get_sensor_value, NULL
// 	},	
// #ifdef configUSING_LOCALTIME
// 	{
// 		DID_DEV_DATE, "dev date", get_dev_date, set_dev_date
// 	},
// 	{
// 		DID_DEV_TIME, "dev time", get_dev_time, set_dev_time
// 	},
// #endif

// 	{
// 		DID_STEP, "step", get_report_step, set_report_step
// 	},
// 	{
// 		DID_FREQ, "freq", get_report_freq, set_report_freq
// 	},
// 	{
// 		DID_RERPORT_STATUS, "report status", 
// 	},

// 	/**/
// 	{
// 		DID_REPORT_ALARM, "report alarm", get_warning_infor,NULL
// 	},

// 	/**/
// 	{
// 		DID_DELAY, "delay", get_power_on_delay_time, set_power_on_delay_time
// 	},

	//#ifdef configUSING_DEBUG	
// 	{
// 		DID_REBOOT, "reboot", NULL, do_reboot
// 	},
// 	{
// 		DID_RESTORE, "restore", NULL, dbg_do_restore
// 	},

	//#endif
// 	{
// 		DID_DBG_REDUCTION, "rection", get_reduction, set_reduction
// 	},
// 	{
// 		DID_DBG_TEMP_RESET, "temp reset", NULL, set_temp_reset
// 	},


static const struct func_ops func_items[] =
{
#ifdef DEV_SHOW
	{DID_DEVSHOW,           "devshow",      NULL,             set_dev_show},
	{DID_PWD,               "get pwd",      get_password,     NULL},
	{DID_SILENT,            "silent",       NULL,             set_silent_time},
#endif
	{DID_DEVTYPE,           "devtype",      do_get_dev_type,  NULL},
	{DID_COMMVER,           "commver",      get_app_comm_ver, NULL},
	{DID_SOFTVER,           "softver",      do_get_soft_ver,  NULL},
	{DID_DEVKEY,            "devkey",       get_dev_key,      NULL},
	{DID_DEVSN,             "SN",           get_dev_sn,       NULL},
	{DID_CHKPWD,            "chkpwd",       NULL,             check_password},
	{DID_CURTAIN_CTL,       "curtain ctl",        NULL, set_motor_ctrl_inf},
    {DID_CURTAIN_R,         "curtain ctl",        NULL, set_motor_ctrl},
	{DID_CURTAIN_RELAY_TIM, "curtain relay time", get_relay_tim, set_relay_tim},
	
};


static const struct func_ops * get_option(int did)
{
	int i;

	for (i = 0; i < array_size(func_items); i++)
		if (func_items[i].did == did)
			return & func_items[i];

	return NULL;
}


static inline int form_error_body(u16 did, uint8_t err, void * out, size_t maxlen)
{
    uint8_t data[2] = {0x00, 0x00};
	data[0] = err;
	return code_body(did, true, data, sizeof(data), out, maxlen);
}


static inline int smart_frame_body_len(const body_t * body)
{
	return get_bits(body->ctrl, 0, 6);
}


static int do_cmd(int cmd, uint8_t * data, int len)
{
	int outidx = 0, inidx = 0;
	uint8_t tmp[BUF_LEN];

	memcpy(tmp, data, len);

	while (len >= FBD_FRAME_HEAD)
	{
		body_t * body = (body_t *) &tmp[inidx];
		body_t * outbd = (body_t *) &data[outidx];
		uint16_t did = get_le_val(body->did, sizeof(body->did));
		int dlen = smart_frame_body_len(body);

		if (len < FBD_FRAME_HEAD + dlen)
		{
#ifdef PLC_SMART_DEBUG
			log_d("Smart frame, body length error!\n");
#endif

			outidx += form_error_body(did, LEN_ERR, outbd, BUF_LEN);
			break;
		}

		inidx += FBD_FRAME_HEAD + dlen;
		len -= FBD_FRAME_HEAD + dlen;
		const struct func_ops * op = get_option(did);
		int(*handler) (const uint8_t * in, size_t len, uint8_t * out, size_t maxlen) = NULL;
		handler = op ? ((cmd == CMD_GET) ? op->read: op->write): NULL;

		if (handler)
		{
#ifdef PLC_SMART_DEBUG
			log_d("Do cmd[%s]...\n", op->tip);
#endif

			memcpy(outbd, body, sizeof(body_t) +dlen);
			uint8_t maxlen = 128;
			int ret = handler(body->data, dlen, outbd->data, maxlen);

			if (ret < 0)
			{
				if (ret == -DATA_TRANS)
					return - 1;

				if (ret == -NO_RETURN)
					continue;

				form_error_body(did, -ret, outbd, BUF_LEN);
			}
			else if (ret > 0)
			{
				memcpy(outbd->did, body->did, sizeof(body->did));

				if ((outbd->did[0] == 0x18) && (outbd->did[1] == 0XC0)) /*C018的指令回复为C012*/
					outbd->did[0] = 0x12;

				outbd->ctrl = ret;
			}
			else 
			{
				body->ctrl = 0;
			}
		}
		else 
		{
			form_error_body(did, DID_ERR, outbd, BUF_LEN);
		}

		outidx += FBD_FRAME_HEAD + smart_frame_body_len(outbd);
	}

	return outidx;
}


//---------------------------------------------------------------------------------------

/**************************************************************************
  * @brief	   : 判断组地址
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    :  data[0]: bit[0~5]: data length		 
									bit[6~7]: group type
														group_type: 0: bit
																				1: 1byte  group id
																				2: 2bytes group id
***************************************************************************/
bool is_gid_equal(const uint8_t * data, uint16_t mysid)
{
	int dlen = get_bits(data[0], 0, 5);
	int group_type = get_bits(data[0], 6, 7);		//取data[0]的高两位。
	uint16_t gid = get_le_val(setting.para.sid, sizeof(setting.para.sid));

#if MAX_ACTOR_NUM
	uint8_t k, connt_size = 0;
	uint16_t j = 0;

	k = (gid % 8) ? (gid % 8): 8;
#endif

	data++;

	if (group_type == 0) /* bit type */
	{
		gid--;

		if (dlen < get_bits(gid, 3, 7) + 1)
		{
#if MAX_ACTOR_NUM
			goto deal_find_none;

#else

			return false;

#endif
		}

        if (tst_bit(data[get_bits(gid, 3, 7)], gid & 0x07))
		{
#if SUBSCRIBER_NUM

			for (j = 0; j < (gid >> 3); j++)
			{
				connt_size += get_1byte_bit1_number(data[j], 8);
			}

			connt_size += get_1byte_bit1_number(data[(gid >> 3)], k);

			if (0 == judge_data.find_myself)
			{
				judge_data.equipment_gid += connt_size;
			}

			judge_data.find_myself = 1;
#endif

			return true;
		}
	}
	else /* bytes type */
	{
		int i;
		int gid_unit_len = group_type == 1 ? 1: 2;

		for (i = 0; i < dlen; i += gid_unit_len)
		{
			int _gid = get_le_val(data + i, gid_unit_len);

			if (gid == _gid)
			{
#if MAX_ACTOR_NUM

				if (0 == judge_data.find_myself)
				{
					judge_data.equipment_gid += connt_size + 1;
				}

				judge_data.find_myself = 1;
#endif

				return true;
			}
			else if (_gid == 0)
			{
#if MAX_ACTOR_NUM
				judge_data.equipment_gid += gid;
				judge_data.find_myself = 1;
#endif

				return true;
			}

#if MAX_ACTOR_NUM
			connt_size++;
#endif
		}
	}

#if MAX_ACTOR_NUM

deal_find_none:

	if (0 == judge_data.find_myself)
	{
		if (0x00 == group_type)
		{
			for (j = 0; j < dlen; j++)
			{
#if SUBSCRIBER_NUM
				judge_data.equipment_gid += get_1byte_bit1_number(data[j], 8);
#endif
			}
		}
		else if (0x01 == group_type)
		{
			judge_data.equipment_gid += dlen;
		}
		else if (0x02 == group_type)
		{
			judge_data.equipment_gid += dlen / 2;
		}
	}

#endif

	return false;
}


//---------------------------------------------------------------------------------------

/**************************************************************************
  * @brief	   : 广播报文处理
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
int do_group_cmd(uint8_t * data, int len)
{
	int inidx = 0;
	uint16_t mysid = get_le_val(setting.para.sid, SID_LEN);
	uint8_t tmp[BUF_LEN];
	int gid_len = get_bits(data[inidx], 0, 5) + 1;

	while (len >= FBD_FRAME_HEAD + gid_len)
	{
		body_t * body = (body_t *) &data[inidx + gid_len];
		int body_len = gid_len + offset_of(body_t, data) +smart_frame_body_len(body);

		if (len < body_len)
			break;

		if (is_gid_equal(&data[inidx], mysid))
		{
			const struct func_ops * op = get_option(get_le_val(body->did, sizeof(body->did)));

			if (op && op->write)
			{
#ifdef PLC_SMART_DEBUG
				log_d("Do cmd[%s]...", op->tip);
#endif

				op->write(body->data, get_bits(body->ctrl, 0, 6), tmp, 128);
			}
		}

		inidx += body_len;
		len -= body_len;
		gid_len = get_bits(data[inidx], 0, 5) + 1;
	}

	return 0;
}


/**************************************************************************
  * @brief	   : 远程帧动作
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
static int remote_frame_opt(struct SmartFrame * pframe)
{
    int ret = -1;
    struct AppFrame *app = (struct AppFrame *)pframe->data;    
    int len = pframe->len - offset_of(struct AppFrame, data);

	if (smart_frame_is_ack(pframe))
	{
		if (CMD_RELI_REPORT == app->cmd) //可靠用户
		{
			report_frame_ack(pframe->seq, pframe->said, &pframe->data[1]);
		}

		return ret;
	}

#ifdef DEV_SHOW

	if (dev_search_param.dev_show_flag)
		return 0;

#endif

	switch (app->cmd)
	{
		case CMD_SET:
#if MAX_ACTOR_NUM
			clear_equipment_gid_flag();
#endif

			if (smart_frame_is_broadcast(pframe))
			{
				ret = do_group_cmd(app->data, len);

#if MAX_ACTOR_NUM
				save_subscriber_infor(app->cmd, pframe->said, pframe->taid);
#endif

				return 0;
			}

			/* fall through */
		case CMD_GET:
#if MAX_ACTOR_NUM

			if (CMD_SET == app->cmd)
			{
				save_subscriber_infor(app->cmd, pframe->said, pframe->taid);
			}

#endif

			ret = do_cmd(app->cmd, app->data, len);
			break;

		case CMD_UPDATE:
			ret = update_frame_opt(app->data, len);
			break;

#ifdef DEV_SHOW
		case CMD_SHOW:
			ret = search_frame_opt(pframe);
			break;

#endif

		default:
			ret = 0;
			break;
	}

	if (ret > 0)
	{
		ret += sizeof(struct AppFrame);
	}

	return ret;
}


static base_t get_reg(struct SmartFrame * pframe)
{
	if (reg.type == PASSWORD_REG)
	{
		if (is_all_xx(setting.para.panid, 0x00, sizeof(setting.para.panid)))
		{
			// first time register, do not check password
		}
		else if (memcmp(setting.encode.pwd, &pframe->data[PANID_LEN + 1], PW_LEN))
		{
			reg.last_status = PASSWORD_ERR;
			state_machine_change(STATE_UNLINK);
			return 0;
		}
	}

	reg.type = PASSWORD_REG;
	state_machine_change(STATE_GET_GID);

	if (memcmp(setting.para.panid, &pframe->data[1], PANID_LEN))
	{
		memcpy(setting.para.panid, &pframe->data[1], PANID_LEN);
		setting_save();
	}

	return 0;
}


/**************************************************************************
  * @brief	   : 本地帧处理
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
extern void boot_start_update(u8 * data, int len);


void local_frame_opt(struct SmartFrame * pframe)
{
	uint8_t cmd = pframe->data[0];

	switch (cmd)
	{
		case CMD_REGINFOR:
			get_reg(pframe);
			break;

		case CMD_REQ_AID:
			state_machine_change(STATE_SET_AID);
			break;

		case CMD_UNLINK:
			state_machine_change(STATE_SET_REG);
			break;

		case CMD_GET_GWAID:
			memcpy(setting.para.gid, &pframe->data[1], AID_LEN);
			setting_save();
			break;

		case CMD_ACK:
		case CMD_NAK:
			set_bit(dev_state.act_flag, EN_PLC);
			break;

#ifdef configUSING_UPWAY
		case CMD_REQ_UPDATE:
			boot_start_update(&pframe->data[1], pframe->len - 1);
			break;

#endif

		default:
			break;
	}
}


/**************************************************************************
  * @brief	   : 根据帧属性 判断调用本地帧还是远程帧
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
int smart_frame_handle(struct SmartFrame * pframe)
{
	int len = 0;

	if (!pframe)
		return 0;

	if (smart_frame_is_local(pframe))
	{
		local_frame_opt(pframe);
	}
	else 
	{
		len = remote_frame_opt(pframe);

		if (len > 0)
		{
			len = code_ret_frame(pframe, len);
		}

		set_bit(dev_state.act_flag, EN_PLC_RUN);
		dev_state.plc_run_cnt = 5;
	}

	return len;
}


