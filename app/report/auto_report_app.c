#include <stdlib.h>
#include <config.h>
#include <utils.h>
#include "proto_smart.h"
#include "plc_smart.h"
#include "auto_report_app.h"
#include "device.h"
#include "dev.h"
#include "settings.h"
#include "printk.h"


/* please redefine send function */
int uart_write_report(const void *in,int len)
{
	device_t *serial_plc = device_find("uart0");
	set_bit(dev_state.act_flag,EN_PLC_RUN);
	dev_state.plc_run_cnt = 5;
    log_d("auto report send report frame!\n");
	return(device_write(serial_plc, 0, in, len));
}

/* some sensor need report type infor, if you don't use it ,please delete */
int code_intype_body(uint16_t did, int err, const void *data, int len, void *out, int maxlen ,uint8_t type)
{
    int ret = 0;
    uint8_t buf[0x10];
    
    buf[0] = type;
    memcpy(&buf[1], data, len);

    ret = code_body(did, 0, buf, len + 1, out, maxlen);
    return ret;
}
#if 0
void get_report_data_callback(report_type_t r_type)
{
    uint8_t len = 0;
    clear_chn(r_type);
#if MAX_SENSOR_NUM
    uint8_t buf[2] = {0x00,0x00};
    if((POWER_ON == r_type) || (REGISTER == r_type) || (FREQUENCY == r_type) || (FIXED_LENGTH == r_type))
    {  
			/*老设备上报指令*/
			hex2bcd(abs(dev_state.sensor.temp), buf, TEMP_LEN);
			dev_state.sensor.temp < 0 ?  set_bit(buf[1], 7) : clr_bit(buf[1], 7);
			len += code_intype_body(0xB691,0,buf,TEMP_LEN,g_frame_buf+len,sizeof(g_frame_buf), TEMP);
			hex2bcd(dev_state.sensor.humi, buf, HUMI_LEN);
			len += code_intype_body(0xB691,0,buf,HUMI_LEN,g_frame_buf+len,sizeof(g_frame_buf),HUMI);
			/*新设备上报指令*/
			hex2bcd(abs(dev_state.sensor.temp), buf, TEMP_LEN);
			dev_state.sensor.temp < 0 ?  set_bit(buf[1], 7) : clr_bit(buf[1], 7);
			len += code_intype_body(0xB701,0,buf,TEMP_LEN,g_frame_buf+len,sizeof(g_frame_buf), TEMP);
			hex2bcd(dev_state.sensor.humi, buf, HUMI_LEN);
			len += code_intype_body(0xB701,0,buf,HUMI_LEN,g_frame_buf+len,sizeof(g_frame_buf),HUMI);
    }
#endif
/*两种报警上报*/	
#if MAX_ALARM_NUM
    if((POWER_ON == r_type) || (REGISTER == r_type) || (ALARM == r_type))
    {
			len += code_intype_body(0xD105,0,&dev_state.low_protect_act,1,g_frame_buf+len,sizeof(g_frame_buf),LOW_PROT_TEMP_ALARM);
			len += code_intype_body(0xD105,0,&dev_state.sensor.breakdown,1,g_frame_buf+len,sizeof(g_frame_buf),TEMP_HUMI_ALARM);
    }
#endif
	
#if MAX_ACTOR_NUM
    if((POWER_ON == r_type) || (REGISTER == r_type) || (STATE_CHANGE == r_type))
    {
			
	  len+=code_body(0xE013,0,&dev_state.air_coner_switch,1,g_frame_buf+len,sizeof(g_frame_buf));//开关机
      len+=code_body(0xE012,0,&setting.dev_infor.mode,1,g_frame_buf+len,sizeof(g_frame_buf));//模式
      len+=code_body(0xE011,0,&setting.dev_infor.win_speed,1,g_frame_buf+len,sizeof(g_frame_buf));//风速
      len+=code_body(0xE014,0,&setting.dev_infor.panel_lock,1,g_frame_buf+len,sizeof(g_frame_buf));//面板锁定
      len+=code_body(0xE010,0,&setting.dev_infor.low_temp_switch,1,g_frame_buf+len,sizeof(g_frame_buf));//低温保护
      hex2bcd(dev_state.set_temp/10,buf,1);//转为BCD
      len+=code_body(0xE002,0,buf,1,g_frame_buf+len,sizeof(g_frame_buf));//设置温度

			
			
			uint8_t i,j,m_len;
			uint8_t tmp[0x30];
			*tmp = setting.dev_infor.period_ctrl.mode;
			m_len = 1;
			for(j=0;j<setting.dev_infor.period_ctrl.mode;j++)
			{
				*(tmp+m_len) = setting.dev_infor.period_ctrl.week[j];
				*(tmp+m_len+1) = setting.dev_infor.period_ctrl.num[j];
				m_len += 2;
				for(i=0;i<setting.dev_infor.period_ctrl.num[j];i++)
				{
					hex2bcd((setting.dev_infor.period_ctrl.start[j][i]>>8),tmp+m_len,1);
					hex2bcd((setting.dev_infor.period_ctrl.start[j][i]&0xFF),tmp+m_len+1,1);
					hex2bcd((setting.dev_infor.period_ctrl.end[j][i]>>8),tmp+m_len+2,2);
					hex2bcd((setting.dev_infor.period_ctrl.end[j][i]&0XFF),tmp+m_len+3,2);
					*(tmp+m_len+4)=setting.dev_infor.period_ctrl.onoff[j][i];
					hex2bcd(setting.dev_infor.period_ctrl.set_temp[j][i]/10,tmp+m_len+5,1);
					m_len += 6;
				}
			}
			len += code_body(0xE020,0,tmp,m_len,g_frame_buf+len,sizeof(g_frame_buf));
			
			//注意这个要放到最后 组 本地按键的时候会将这个报文删掉
			if((POWER_ON == r_type) || (REGISTER == r_type))
          memcpy(judge_data.taker_id,setting.encode.id,ID_LEN);//保证上电上报和注册上报时使用的是自身的AID
      len += code_body(0xC01A,0,judge_data.taker_id,ID_LEN,g_frame_buf+len,sizeof(g_frame_buf));
    }
#endif
		len = put_chn(r_type,g_frame_buf,len);
    log_d("Report type: %d,put data len:%d\n",r_type,len);
}
#endif

//--------------------------------------------------------------------------------------
#if MAX_SENSOR_NUM
static uint8_t get_sensor_type(struct Body* fbd)
{
    /* B701 ACK, get sensor type */
    if ((fbd->did[0] == 0x01) && (fbd->did[1] == 0xB7))
    {
        return(fbd->data[0]);
    }
	/* please add you did and sensor type */
    return UNKNOWN;
}
#endif


//--------------------------------------------------------------------------------------
//-------------------- receive gateway ack ,refresh sensor baseline --------------------
//--------------------------------------------------------------------------------------
void report_finish_refresh_infor(uint8_t *data, uint8_t len)
{
    #if MAX_SENSOR_NUM
    uint8_t i,type;
    int8_t j = -1;
    struct Body *body;
    for (i = 0; i < len; i += FBD_FRAME_HEAD)
    {
        body = (struct Body*)&data[i];
        type = get_sensor_type(body); 
        switch (type)
        {
            
            case TEMP:
                j = get_sensor_from_type(TEMP);
                if (j >= 0)
                {
                    sensor_data[j].sensor_base = (int32_t)bcd2hex(&body->data[1],TEMP_LEN);
                    sensor_data[j].last_report = sensor_data[j].sensor_base;
                }
                break;
            case HUMI:
                j = get_sensor_from_type(HUMI);
                if (j >= 0)
                {
                   sensor_data[j].sensor_base = (int32_t)bcd2hex(&body->data[1],HUMI_LEN);
                   sensor_data[j].last_report = sensor_data[j].sensor_base;
                }
                
                break;
            default:
                break;
        }
        i += body->ctrl;
    }
    #endif
}
