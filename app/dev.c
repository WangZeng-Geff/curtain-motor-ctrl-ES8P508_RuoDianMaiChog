
#include <config.h>
#include <utils.h>
#include <os.h>
#include <device.h>
#include <printk.h>
#include <settings.h>
#include <syserr.h>
#include "board.h"
#include "plc_smart.h"
#include "encode.h"
#include "dev.h"
#include "dev_show.h"
#include "curtain_ctl_gpio.h"
#include "report\\auto_report_app.h"
static uint8_t dev_type[8] =
{
	0xFF, 0xFF, 0x04, 0x00, 0x03, 0x00, 0x00, 0x00
};

static char * soft_ver = "P-ACT-CC1-AC-B(V1.1)-20190719";
//static tcb_t app_tcb;
device_t * dev_key, *dev_led, *dev_buz, *dev_relay, *bu9796, *dev_rtc;

void dev_show(void)
{
	static char str[0x20];

	dev_ver_get(str, sizeof(str));
	log_d("VER: %s\n", str, sizeof(str));
	log_d("AID: %s\n", arr2str(setting.encode.id, sizeof(setting.encode.id), str, sizeof(str)));
	log_d("PWD: %s\n", arr2str(setting.encode.pwd, sizeof(setting.encode.pwd), str, sizeof(str)));
	log_d("SN: %s\n", arr2str(setting.encode.sn, sizeof(setting.encode.sn), str, sizeof(str)));
	log_d("DKey: %s\n", arr2str(setting.encode.dkey, sizeof(setting.encode.dkey), str, sizeof(str)));
	log_d("PID: %s\n", arr2str(setting.para.panid, sizeof(setting.para.panid), str, sizeof(str)));
	log_d("GID: %s\n", arr2str(setting.para.gid, sizeof(setting.para.gid), str, sizeof(str)));
	log_d("SID: %s\n", arr2str(setting.para.sid, sizeof(setting.para.sid), str, sizeof(str)));
}

int dev_type_get(void * out, int maxlen)
{
	memcpy(out, dev_type, sizeof(dev_type));
	return sizeof(dev_type);
}

int dev_type_cmp(const void * dt, size_t len) //相同返回1
{
	return len == sizeof(dev_type) && !memcmp(dt, dev_type, sizeof(dev_type));
}

int dev_ver_get(void * out, int maxlen)
{
	int len = strlen(soft_ver);

	memcpy(out, soft_ver, len);
	return len;
}

int dev_ver_cmp(const void * ver, size_t len)
{
	char softver[0x20];
	int verlen = dev_ver_get(softver, sizeof(softver));

	return len == verlen && !memcmp(ver, softver, verlen);
}

#if 0
/* dev restore diaplay remind user the device will refactory */
static void dev_resotre_opt(void)
{
	if (dev_state.need_restore)
	{
		dev_restore_display();
	}
}
#endif

/**************************************************************************
  * @brief	   : 参数保存
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
static void flash_param_save(void)
{
	if (dev_state.param_save)
	{
		log_d("flash_param_save\n");
		dev_state.param_save = 0;
		dev_state.run_save_cnt = 0; 				/*存储计数*/
		setting_save();
	}
}

/**************************************************************************
  * @brief	   : 在协议那边修改 发送一帧数据 看是否有ack
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
void check_online_handle()
{
	uint8_t tmp[2] =
	{
		0
	};
	uint8_t ret = 0;
	device_t * serial_plc = device_find("uart0");

	tmp[0] = CMD_SET_REG;
	tmp[1] = reg.type | reg.last_status;
	reg.last_status = 0;
	ret = code_local_frame(tmp, sizeof(tmp), g_frame_buf, sizeof(g_frame_buf)); //组报文
	device_write(serial_plc, 0, g_frame_buf, ret);
	log_d("send plc_online check!\n");
}

/**************************************************************************
  * @brief	   : 检查PLC是否在线 只设置标志位 运行EN_PLC_RUN 在线EN_PLC
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
void check_online(void)
{
	/*这里发送一条PLC语句 并在反馈中 将标志位置1*/
	dev_state.check_online_cnt++;
	if (dev_state.check_online_cnt > 60)
	{
		dev_state.check_online_cnt = 0;
		check_online_handle();						/*在协议那边修改 发送一帧数据 看是否有ack*/
	}

	if (tst_bit(dev_state.act_flag, EN_PLC_RUN))
	{
		if (dev_state.plc_run_cnt == 0) clr_bit(dev_state.act_flag, EN_PLC_RUN);
		else dev_state.plc_run_cnt--;
	}
}

/**************************************************************************
  * @brief	   : 1s循环任务
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 自动上报 数据存储 检查PLC在线 上电传感器延时 显示刷新 运行时间统计
***************************************************************************/
void special_time_del(void);

static void sec_tmr_cb(struct soft_timer * st)
{
	st->expires += configHZ;
	soft_timer_add(st);
//	auto_report_sec_task();
	flash_param_save();
//	check_online();
//	sensor_power_delay();							/*传感器上电延时*/
//	run_time_cnt(); 								/*运行时间统计*/
//	rtc_time_del(); 								/*读取RTC  时段处理*/
//	special_time_del();

#ifdef DEV_SHOW
	check_search_delay();
#endif
//	log_d("1s Task\r\n");
}

static int sec_tmr_init(void)
{
	static

	struct soft_timer st =
	{
		.cb = sec_tmr_cb, 
	};

	st.expires = jiffies + configHZ;
	soft_timer_add(&st);
	return 0;
}

#if 0
/**************************************************************************
  * @brief	   : 200ms循环任务
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    :  读取温湿度数据
***************************************************************************/
extern struct TEMP_HUMI_CTRL temp_humi_ctrl;

static void on_40ms_tmr_cb(struct soft_timer * st)
{
	//故障后执行时间为120ms
	(dev_state.sensor.breakdown == 1)?(st->expires += configHZ / 5):(st->expires += configHZ / 25);
	soft_timer_add(st);
	temp_humi_ctrl.temp_humi_read_conter++;
	if (temp_humi_ctrl.temp_humi_read_conter > 10) temp_humi_ctrl.temp_humi_read_conter = 0;
	update_temp_humi();
//	log_d("200ms Task\r\n");
	
}

static int _40msec_tmr_init(void)
{
	static

	struct soft_timer st =
	{
		.cb = on_40ms_tmr_cb, 
	};

	st.expires = jiffies + configHZ / 25;
	soft_timer_add(&st);
	return 0;
}
#endif 
/**************************************************************************
  * @brief	   : 10ms循环任务
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 窗帘电机控制
***************************************************************************/
extern struct MOTOR Motor1;
static void on_10ms_tmr_cb(struct soft_timer * st)
{
    static int cnt = 0;

	st->expires += configHZ / 100;
	soft_timer_add(st);
    if (relay_time > 0) 
    {
        relay_time--;
        if (relay_time == 0) 
        {
          SET_PIN_L(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
          SET_PIN_L(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
          SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
        }
    }

    if (dev_search_param.dev_show_flag)
    {
        cnt += 1;
        if (cnt > 600)
        {
            SET_PIN_H(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
            SET_PIN_L(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
            SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
            relay_time = setting.dev_infor.actuation_time;

            cnt = 0;
            dev_search_param.dev_show_flag = 0;
        }
    }
}

static int _10msec_tmr_init(void)
{
	static

	struct soft_timer st =
	{
		.cb = on_10ms_tmr_cb, 
	};

	st.expires = jiffies + configHZ / 100;
	soft_timer_add(&st);
	return 0;
}


/**************************************************************************
  * @brief	   : 设备显示时间任务 200ms  6s后结束
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 设备搜索显示
***************************************************************************/
static void dev_show_timer_cb(struct soft_timer * st)
{
	static uint8_t cnt = 0;

	cnt++;
	if (cnt < 30)
	{
		st->expires += configHZ / 5;
		soft_timer_add(st);
		dev_show_display_start();
	}
	else 
	{
		cnt = 0;
		dev_search_param.dev_show_flag = 0x00;
		dev_show_display_end();
		device_ctrl(dev_led, CTRL_ON, 0);
		device_ctrl(dev_led, CTRL_OFF, 0);
	}
}

void dev_show_start(void)
{
	static

	struct soft_timer dev_show_timer =
	{
		.cb = dev_show_timer_cb, 
	};

	dev_show_timer.expires = jiffies + configHZ / 5; //200ms
	soft_timer_add(&dev_show_timer);
}

/**************************************************************************
  * @brief	   : 建立APP任务
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 
***************************************************************************/
void setup_app(void)
{
	setting_load();
	device_encode_opt();
	dev_show();
	plc_init();
//	si7020_init("i2c2");
//	lcd_init("i2c1");
//	_40msec_tmr_init();
	_10msec_tmr_init();
	sec_tmr_init();

//	task_create(&app_tcb, app_task_cb, 0);

#ifndef configUSING_DEBUG
//	auto_report_init(60, setting.para.gid, setting.encode.id, setting.para.sid, &setting.dev_infor.report);

#else

//	auto_report_init(0, setting.para.gid, setting.encode.id, setting.para.sid, &setting.dev_infor.report);
#endif
}

