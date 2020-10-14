

#ifndef _SETTINGS_H_
#define _SETTINGS_H_
#include <types.h>
#define FLASH_SECTOR_SIZE		(1024u)
#define FLASH_SECTOR_MASK		(FLASH_SECTOR_SIZE - 1)
#define SETTING_MAGIC			(0x55AAu)
#define SETTING_FROM			(70 * 1024u)
#define SETTING_SIZE			(2 * 1024u)
#define SETTING_INVALID_ADDR	(SETTING_FROM + SETTING_SIZE)

//#define MAX_LOW_TEMP_PROTECT 80//低温保护温度  5~8度	使用了 低温回差温度替代
//#define MIN_LOW_TEMP_PROTECT 50//低温保护温度  5~8度
#define SAVE_TIME				600 /*信息保存时间 S*/
#define B_KEEP_TIME 			5  /*背光持续时间 S*/
#define LCD_BLINK_TM			5 /*LCD闪烁的图标保持亮的时间 *100ms  这是500ms*/
#define LCD_DATA_TM 			30 /*LCD闪烁的图标保持亮的时间 *100ms  这是3s*/

//--------------------------------------------------------------------------
//继电器配置
#define H_OPEN					0x03 
#define M_OPEN					0x05
#define L_OPEN					0x09
#define H_CLOSE 				0x02
#define M_CLOSE 				0x04
#define L_CLOSE 				0x08
#define N_CLOSE 				0x00	

//--------------------------------------------------------------------------
typedef struct sensor
{
int16_t 		temp;								//温度 是十倍实际温度 保留1位小数
int16_t 		humi;								//湿度 同上
uint8_t 		poweron 		: 1;				//延时一段时间再进行继电器动作，防止上电后温度未读出时继电器动作
uint8_t 		breakdown;							//传感器故障  1为故障 0为正常  （原采用为标表示 由于上报时需要取字节地址）
} sensor_status_t;


/*若修改按键顺序 修改这里顺序即可*/
enum key_list
{
KEY_SWITCH = 0, 
KEY_DEC, 
KEY_ADD, 
KEY_MODE, 
KEY_SPEED, 
KEY_LOCK,											/*并非实际按键，实现面板锁定后的程序执行*/
};


enum act_flag
{
LOWTEMP = 0,										/*低温保护*/
EN_ACT, 

//	EN_AUTO,/*风速自动*/	
EN_TIME,											/*时段控制标志*/
EN_MAN, 											/*时段内人为操作标志*/
EN_PLC, 
EN_PLC_RUN, 
};


//---------------------------------------------------
//刷新显示的类型
enum 
{
DIS_OTHER, 
DIS_CLOCK, 
};


/*此枚举顺序不可动	跟winspeed相对应*/
enum wind_speed
{
WIN_LOW, 
WIN_MIDDLE, 
WIN_HIGH, 
WIN_AUTO, 
};


/*此枚举顺序不可动	跟mode相对应*/
enum 
{
MODE_COOL = 0, 
MODE_HEAT, 
MODE_VENTILATE, 
};

typedef struct rgb_str
{
uint8_t 		r;
uint8_t 		g;
uint8_t 		b;
} rgb_t;

#ifdef configUSING_LOCALTIME


enum dev_time_list
{
_YY, 
_MM, 
_DD, 
_WW, 
_H, 
_M, 
_S, 
};


typedef struct dev_time
{
uint8_t 		time[7];							/*年月日周时分秒*/
uint8_t 		date_flag_set	: 1;				/*平台下发日期*/
uint8_t 		time_flag_set	: 1;				/*平台下发时间*/
uint8_t 		date_flag_ok	: 1;				/*获取日期成功*/
uint8_t 		time_flag_ok	: 1;				/*获取时间成功*/
} dev_time_t;


#endif

typedef struct special_fun
{
uint8_t 		flag;								//2 3 标志同时长按标志
uint8_t 		cnt0;								/*时间 2 3时间间隔*/
uint8_t 		num;								/*功能*/
uint8_t 		cnt1;								/*长时间不动自动退出*/
} dev_special_fun_t;


typedef struct DevStateInfo
{
uint8_t 		state;								/*继电器状态*/
uint8_t 		state_bak;
int16_t 		set_temp;
uint8_t 		air_coner_switch;
uint8_t 		relay_num;
uint8_t 		need_act;
uint8_t 		act_chn;
uint8_t 		param_save;
uint8_t 		need_restore;
sensor_status_t sensor; 							/*传感器状态*/
uint16_t		act_flag;							/*通过枚举EN_ 进行位使用*/
uint8_t 		low_protect_act;					/*低温保护动作标志*/
uint8_t 		check_online_cnt;					/*检查在线计数*/
uint16_t		power_delay_cnt;					/*检查上电延时计数*/
uint8_t 		lcd_on_cnt; 						/*LCD上电全程时间*/
uint16_t		run_save_cnt;						/*运行状态信息存储计数*/
uint8_t 		plc_run_cnt;						/*PLC通信闪烁计数*/

#ifdef configUSING_LOCALTIME
dev_time_t		dev_time;
uint8_t 		get_time_cnt;
uint8_t 		power_report;
#endif
rgb_t           rgb_data;

uint8_t 		period_invaild_num; 				/*无效时间段*/
dev_special_fun_t special_fun;
} dev_state_info_t;


extern dev_state_info_t dev_state;

typedef struct period_ctrl /*时段控制*/
{
uint8_t 		mode;								/*时段控制模式0：不使用 1：七天 2：5天和2天*/
uint8_t 		week[2];							/*生效时间 bit0-bit6 周1-周天+*/
uint8_t 		num[2]; 							/*时段数*/
uint16_t		start[2][6];						/*起始时间分钟*/
uint16_t		end[2][6];							/*终止时间分钟*/
uint8_t 		onoff[2][6];
int16_t 		set_temp[2][6];
} period_ctrl_t;

typedef struct setting_hdr
{
uint16_t		magic;
uint16_t		crc;
} setting_hdr_t;


typedef struct 
{
uint8_t 		sn[12];
uint8_t 		dkey[8];
uint8_t 		id[4];
uint8_t 		pwd[2];
} encode_t;


typedef struct 
{
uint8_t 		gid[4];
uint8_t 		panid[2];
uint8_t 		sid[2];
} para_t;


typedef struct CtrlInfo
{
uint8_t 		idx;
uint8_t 		aid[4];
uint8_t 		chn;
} ctrl_info_t;


typedef struct DevInfo
{
#if 0
uint32_t		opt_cnt[2]; 						/*动作统计 0为阀门次数 1为开关次数*/

uint32_t		time_cnt[5]; /*继电器动作时间 0 1 2 3 4	分别为阀门 高 中 低 开
	机*/
period_ctrl_t	period_ctrl;						/*时段控制*/
uint8_t 		report;
uint16_t		temp_report_freq;
uint16_t		temp_report_step;
uint16_t		humi_report_freq;
uint16_t		humi_report_step;
uint8_t 		mode;								/*模式  0cool 1heat 2*/
uint8_t 		win_speed;							/*风速*/
int16_t 		set_temp;							/*运行设置温度 可能会由于时段控制修改*/
uint8_t 		air_coner_switch;					/*空调开关*/
uint16_t		return_diff_temp;					/*回差温度 默认1度*/
uint16_t		power_delay;						/*上电传感器延时时间*/
uint8_t 		panel_lock; 						/*面板锁定*/
uint8_t 		backlight_enable;					//背光
uint8_t 		auto_disable;						/*自动风模式下 达到设定温度后是否继续通风*/
uint16_t		low_deadband;						/*低温回差温度 默认*/
uint8_t 		low_temp_switch;					/*低温保护开关*/
uint16_t		low_protect_temp;					//低温保护温度
uint16_t		max_set_temp;						//最大设置温度
uint16_t		min_set_temp;						//最小设置温度
uint16_t		max_heat_temp;						//最大制热温度
uint16_t		min_heat_temp;						//最小制热温度
uint16_t		max_cool_temp;						//最大制冷温度
uint16_t		min_cool_temp;						//最小制冷温度 
int16_t 		humi_compensation;					//湿度补偿
int16_t 		temp_compensation;					//温度补偿
#endif
uint8_t           actuation_time;                                    //继电器脉冲宽度
} dev_info_t;


typedef struct 
{
setting_hdr_t	hdr;

/* device */
encode_t		encode;

/* para */
para_t			para;

/* ctrl */
ctrl_info_t 	ctrl[4];

/* dev */
dev_info_t		dev_infor;
} __attribute__((aligned(4)))

setting_t;


#define SETTING_HDR_SIZE		(sizeof(setting_hdr_t))
#define SETTING_DAT_SIZE		(sizeof(setting)-sizeof(setting_hdr_t))
#define setting_addr(s) 		((uint8_t*)&s + SETTING_HDR_SIZE)
extern setting_t setting;
int disk_erase(uint32_t start, uint32_t size);
int disk_write(uint32_t start, const void * data, uint32_t size);
int disk_read(uint32_t start, void * data, uint32_t size);
void setting_save(void);
void setting_load(void);
void dev_restore_factory(void);

#endif

