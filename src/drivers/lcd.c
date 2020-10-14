#include <utils.h>
#include <types.h>
#include <device.h>
#include <printk.h>
#include <syserr.h>
#include <os.h>
#include "settings.h"
#include "board.h"

#define _a      0x01
#define _b      0x10
#define _c      0x40
#define _d      0x08
#define _e      0x04
#define _f      0x02
#define _g      0x20
#define _null   0



#ifdef configUSING_LCD

const uint8_t digit_table[22] =
{
    _a + _b + _c + _d + _e + _f      ,  //0
    _b + _c                          ,  //1
    _a + _b + _g + _e + _d           ,  //2
    _a + _b + _c + _d + _g           ,  //3
    _b + _c + _g + _f                ,  //4
    _a + _c + _d + _g + _f           ,  //5
    _a + _c + _d + _e + _f + _g      ,  //6
    _a + _b + _c                     ,  //7
    _a + _b + _c + _d + _e + _f + _g ,  //8
    _a + _b + _c + _f + _g + _d      ,  //9
};
const struct LCD_ALPHA alpha_table[] = {
	{ 'A', _a + _b + _c + _e + _f + _g },
	{ 'B', _c + _d + _e + _f + _g      },
	{ 'C', _a + _d + _e + _f           },
	{ 'D', _b + _c + _d + _e + _g      },
	{ 'E', _a + _d + _e + _f + _g      },
	{ 'F', _a + _e + _f + _g           },
	{ 'G', _a + _c + _e + _f + _d },
	{ 'H', _b + _c + _e + _f + _g  		   },	
	{ 'I', _a + _b + _c + _d                          },
	{ 'L', _d + _e + _f           	   },
	{ 'N', _c + _e + _g          	   },
	{ 'O', _c + _d + _e + _g           },
	{ 'P', _a + _b + _e + _f + _g      },
	{ 'R', _e + _g                     },
	{ 'S', _a + _d + _g      },
	{ 'T', _d + _e + _f + _g      	   },
	{ 'Y', _b + _c + _d + _f + _g      },
	{ '-', _g						   },
	{ ' ', _null			     	   },
};

#define ALPHA_TABLE_SZ    (sizeof(alpha_table) / sizeof(alpha_table[0]))
	
enum signgrop
{
	G_MODE,
	G_OTHER,
	G_SPEED,
	G_ERR,
	G_VALVE,
	G_SET,
	G_HUMI,
	G_TEMP,
	
};
enum  sign
{
	SIGN_COOL,
	SIGN_HEAT,
	SIGN_VENTILATE,
	
	SIGN_FAN,
	SIGN_VALVE,
	SIGN_VALVE_T,
	SIGN_PLC,
	SIGN_LOCK	,
	SIGN_TIME,
	
	SIGN_AUTO,
	SIGN_HIGH_REV,
	SIGN_MIDDLE_REV,
	SIGN_LOW_REV,
	
	SIGN_C_T,
	SIGN_SET,
	SIGN_C,
	SIGN_DIP,
	SIGN_TEMP,
	SIGN_PER_CENT,
	SIGN_RH,	
};
const struct LCD_SIGN sign_table[20] =
{
{SIGN_COOL,			G_MODE,0,0x80},//
{SIGN_HEAT,			G_MODE,0,0x40},//
{SIGN_VENTILATE,G_MODE,0,0x20},//

{SIGN_FAN,			G_OTHER,0,0x08},//
{SIGN_VALVE,		G_VALVE,0,0x02},//
{SIGN_VALVE_T,	G_VALVE,0,0x01},//
{SIGN_PLC,			G_OTHER,1,0x80},//
{SIGN_LOCK	,		G_OTHER,1,0x40},//
{SIGN_TIME,			G_OTHER,1,0x20},//

{SIGN_AUTO,		G_SPEED,		0,0x04},//
{SIGN_HIGH_REV,		G_SPEED			,1,0x02},//
{SIGN_MIDDLE_REV, G_SPEED			,1,0x04},//
{SIGN_LOW_REV,		G_SPEED			,1,0x08},//

{SIGN_C_T,			G_SET			,2,0x80},//
{SIGN_SET,			G_SET			,3,0x80},

{SIGN_C,			G_TEMP			,4,0x80},
{SIGN_DIP,			G_TEMP			,5,0x80},//
{SIGN_TEMP,			G_TEMP			,6,0x80},//
{SIGN_PER_CENT,		G_HUMI			,7,0x80},
{SIGN_RH,			G_HUMI			,8,0x80},

};
uint8_t lcd_buf[LCD_BUF_SIZE];
/**************************************************************************
  * @brief     :按组显示
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    :  
***************************************************************************/
static void dis_group(uint8_t group , uint8_t on)
{
	int i;
  for (i = 0; i < array_size(sign_table); i++)
	{
		if (sign_table[i].group == group)
		{
			if(on)
				lcd_buf[sign_table[i].idx] |= sign_table[i].val;
			else
				lcd_buf[sign_table[i].idx] &= (~sign_table[i].val);
		}
	}
}
/**************************************************************************
  * @brief     :按符号单独显示
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    :  
***************************************************************************/
static void dis_sign(uint8_t sign ,uint8_t on)
{
	int i;
  for (i = 0; i < array_size(sign_table); i++)
	{
		if (sign_table[i].sign == sign)
		{
			if(on)
				lcd_buf[sign_table[i].idx] |= sign_table[i].val;
			else
				lcd_buf[sign_table[i].idx] &= (~sign_table[i].val);
			return;
		}
	}
}
/**************************************************************************
  * @brief     :按闪烁显示
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : VALVE  PLC PLCRUN  TYPE
***************************************************************************/
uint8_t plc_blink_cnt = 0 ,valve_blink_cnt = 0 ;
uint8_t blink_temp[2];
static void dis_blink(uint8_t valve , uint8_t plc, uint8_t plc_run ,uint8_t type)		
{
	if(DIS_CLOCK == type)
	{
		memset(blink_temp,0,2);
		if(valve)
		{
			valve_blink_cnt %= 10;
			valve_blink_cnt++;
			dis_sign(SIGN_VALVE_T ,1);
			if( valve_blink_cnt < LCD_BLINK_TM)
			{
				dis_sign(SIGN_VALVE ,1);
			  blink_temp[0] =1;
			}
			else
				dis_sign(SIGN_VALVE ,0);
		}
		else
		{
			valve_blink_cnt = 0;
			dis_group(G_VALVE ,0);
		}
		
		if(plc)
		{
			if(plc_run)
			{
				plc_blink_cnt %= 10;
				plc_blink_cnt++;
				if( plc_blink_cnt < LCD_BLINK_TM)
				{
					dis_sign(SIGN_PLC ,1);
					blink_temp[1] =1;
				}
				else
					dis_sign(SIGN_PLC ,0);				
			}
			else
			{		
				plc_blink_cnt	= 0;			
				dis_sign(SIGN_PLC ,1);
				blink_temp[1] =1;
			}
		}
		else
			dis_sign(SIGN_PLC , 0);
	}
	else
	{
		(blink_temp[0] == 1)?dis_sign(SIGN_VALVE ,1):0;
		(blink_temp[1] == 1)?dis_sign(SIGN_PLC ,1):0;
	}
}
/**************************************************************************
  * @brief     :显示数据  
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 0湿度  1当前温度  2 设置温度
***************************************************************************/
static void dis_num(int16_t num,uint8_t pos)
{
	switch (pos)
	{

		case 0 :
			lcd_buf[8] |= (num % 1000 / 100 !=0)?(digit_table[num % 1000 / 100]):(_null); //1		
			lcd_buf[7] |= digit_table[num % 100 / 10];   //2	
			break;
		case 1:
			lcd_buf[6] |= (num>=0)?((num % 1000 / 100 !=0)?(digit_table[num % 1000 / 100]):(_null)):(_g);/*负数处理*/
			lcd_buf[5] |= digit_table[_abs(num) % 100 / 10];   //4
			lcd_buf[4] |= digit_table[_abs(num) % 10];         //5
			break;
		case 2 :
			lcd_buf[3] |=  (num % 1000 / 100 !=0)?(digit_table[num % 1000 / 100]):(_null); //6
			lcd_buf[2] |= digit_table[num % 100 / 10];   //7
			break;
	}
}
static void dis_123(int16_t num,uint8_t pos,uint8_t posin)
{
	uint8_t temp = digit_table[num];
	switch (pos)
	{
		case 0 :
			(posin ==1)?(lcd_buf[7] = temp):(lcd_buf[8] = temp);
			break;
		case 1 :
			(posin ==1)?(lcd_buf[2] = temp):(lcd_buf[3] = temp);
			break;	
	  case 2 :
			(posin ==2)?(lcd_buf[4] = temp):((posin ==1)?(lcd_buf[5] = temp):(lcd_buf[6] = temp));
			break;
	}
}
/**************************************************************************
  * @brief     :显示字母  
  * @param[in] : E r -
  * @param[out]: 
  * @return    : 
* @others    : pos (0 湿度区 1设置温度)  posin 0第一位 1第二位
***************************************************************************/
static void dis_abc(char abc,uint8_t pos ,uint8_t posin)
{
	uint8_t temp;
	int i;
		for (i = 0; i < ALPHA_TABLE_SZ; i++)
		{
			if (alpha_table[i].alpha == abc) 
			{
				temp = alpha_table[i].val;
				break;
			}
		}
	switch(pos)
	{
		case 0 :
			(posin ==1)?(lcd_buf[7] = temp):(lcd_buf[8] = temp);
			break;
		case 1 :
			(posin ==1)?(lcd_buf[2] = temp):(lcd_buf[3] = temp);
			break;	
	  case 2 :
			(posin ==2)?(lcd_buf[4] = temp):((posin ==1)?(lcd_buf[5] = temp):(lcd_buf[6] = temp));
			break;
	}
	
}
static void dis_string(uint8_t list)
{
	switch (list)
	{
		case AA ://AA
			dis_abc('A',0 ,0);	
      dis_abc('A',0 ,1);	
			break;
		case PP ://PP
			dis_abc('P',0 ,0);	
      dis_abc('P',0 ,1);	
			break;
		case GD://GD
			dis_abc('G',0 ,0);	
      dis_abc('D',0 ,1);	
			break;
		case PD ://PD
			dis_abc('P',0 ,0);	
      dis_abc('D',0 ,1);	
			break;
		case SD ://SD
			dis_abc('S',0 ,0);	
      dis_abc('D',0 ,1);	
			break;	
//-----------------------------------------
		case TC ://TC  温度补偿
			dis_abc('T',0 ,0);	
      dis_abc('C',0 ,1);	
			break;
		case HC ://HC  湿度补偿 
			dis_abc('H',0 ,0);	
      dis_abc('C',0 ,1);	
			break;
		case RT ://RT  回差温度
			dis_abc('R',0 ,0);	
      dis_abc('T',0 ,1);	
			break;
		case LC ://LC  远程锁定
			dis_abc('L',0 ,0);	
      dis_abc('C',0 ,1);	
			break;
		case CL ://CL  定时开关
			dis_abc('C',0 ,0);	
      dis_abc('L',0 ,1);	
			break;
		case LP ://LP  低温保护
			dis_abc('L',0 ,0);	
      dis_abc('P',0 ,1);	
			break;
		case LT ://LT  低温温度
			dis_abc('L',0 ,0);	
      dis_abc('T',0 ,1);	
			break;
		case RS ://RS  回复出场
			dis_abc('R',0 ,0);	
      dis_abc('S',0 ,1);	
			break;
  	case ER ://ERR  故障
			dis_abc('E',0 ,0);	
      dis_abc('R',0 ,1);	
			break;
    case SIGNON :
      dis_abc('O',2,0);	
			dis_abc('N',2,1);	
			break;
    case SIGNOFF :
      dis_abc('O',2,0);	
			dis_abc('F',2,1);	
			dis_abc('F',2,2);	
			break;	
    case LO :
		dis_abc('L',2,0);
		dis_abc('O',2,1);		
		break;
      		
	}
}
static void dis_id(uint8_t data)
{
	uint8_t temp = data>>4;
	(temp>9)?(dis_abc(temp-10+'A',2,1)):(dis_123(temp,2,1));
	temp = data &0x0f;
	(temp>9)?(dis_abc(temp-10+'A',2,2)):(dis_123(temp,2,2));	
}
/**************************************************************************
  * @brief     :显示风速  
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : WIN_LOW WIN_MIDDLE WIN_HIGH     0：非时间引起的刷新
***************************************************************************/
uint8_t  auto_dis_cnt=0;/*自动风显示 闪烁*/
static void dis_wind(int8_t wind , uint8_t type)
{
	if(wind == WIN_AUTO)
	{
		if(type) auto_dis_cnt++;
		auto_dis_cnt %= 40;
		wind = auto_dis_cnt;
		dis_sign(SIGN_AUTO,1);
	}
	else
	{
		auto_dis_cnt = 0;
		wind *= 10;
	}
	dis_sign(SIGN_FAN,1);
	if(wind >= 30) return;
	if(wind >= WIN_HIGH*10) dis_sign(SIGN_HIGH_REV,1);
	if(wind >= WIN_MIDDLE*10) dis_sign(SIGN_MIDDLE_REV,1);
	if(wind >= WIN_LOW*10) dis_sign(SIGN_LOW_REV,1);	
}

struct bu9796_data
{
    struct i2c_bus_device *i2c_bus;
};

static struct bu9796_data bu9796_data;

static int bu9796_write_data(struct i2c_bus_device *i2c_bus, uint8_t *values, size_t length)
{
    struct i2c_msg msgs[1];

    msgs[0].addr  = BU9796_ADDR;
    msgs[0].flags = I2C_WR;
    msgs[0].buf   = (uint8_t *)values;
    msgs[0].len   = length;

    if (i2c_transfer(i2c_bus, msgs, 1) == 1)
        return 0;

    return -EIO;
}

int8_t special_dis_cnt = 0;
void dis_special(uint8_t num ,int16_t data)
{
	dis_string(num);
	switch(num)
	{
		case AA:	
      dis_id(setting.encode.id[3-special_dis_cnt/10]);			
		  dis_num(special_dis_cnt,2);
			special_dis_cnt++;
			special_dis_cnt %= 40;
			break;
		case PP:
			dis_id(setting.encode.pwd[1-special_dis_cnt/10]);			
		  dis_num(special_dis_cnt,2);
			special_dis_cnt++;
			special_dis_cnt %= 20;
			break;
		case GD:
			dis_id(setting.para.gid[3-special_dis_cnt/10]);			
		  dis_num(special_dis_cnt,2);
			special_dis_cnt++;
			special_dis_cnt %= 40;
			break;
		case PD:
			dis_id(setting.para.panid[1-special_dis_cnt/10]);			
		  dis_num(special_dis_cnt,2);
			special_dis_cnt++;
			special_dis_cnt %= 20;
			break;
		case SD:
			dis_id(setting.para.sid[1-special_dis_cnt/10]);			
		  dis_num(special_dis_cnt,2);
			special_dis_cnt++;
			special_dis_cnt %= 20;
			break;
			case TC://温度补偿
				dis_num(data,1);
			  dis_sign(SIGN_DIP,1);
			  dis_sign(SIGN_C,1);
				break;
			case HC://湿度补偿
        dis_num(data,1);
				dis_sign(SIGN_DIP,1); 
				break;
			case RT://回差温度
        dis_num(data,1);
				dis_sign(SIGN_DIP,1);
				dis_sign(SIGN_C,1);
				break;
			case LC://面板锁定
				(data == 1)?(dis_string(SIGNON)):(dis_string(SIGNOFF));
					break;
			case CL://时段开关 不支持5+2天
         (data == 1)?(dis_string(SIGNON)):(dis_string(SIGNOFF));
					break;
			case LP://低温保护开关
          (data == 1)?(dis_string(SIGNON)):(dis_string(SIGNOFF));
					break;
			case LT://低温保护温度
				dis_num(data,1);
			  dis_sign(SIGN_DIP,1);
			  dis_sign(SIGN_C,1);
					break;
			case RS:
				dis_num(888,1);
				break;	
	}
	
	
}
static void update_lcd_data(uint8_t *buffer, size_t size)
{
  lcd_update_t *lcd_update = (lcd_update_t *) buffer;
	memset(lcd_buf,0,LCD_BUF_SIZE);
	
	if(lcd_update->special_num != 0)
	{
		dis_special(lcd_update->special_num , lcd_update->cur_temp);
		return;
	}
	if(0 != lcd_update->breakdown )
	{
		dis_string(ER);
		return;
	}
	//湿度 温度  这里的显示顺序注意下 没用或等于
	dis_num(lcd_update->cur_humi,0);
	(lcd_update->cur_temp < -99)?(dis_string(LO)):(dis_num(lcd_update->cur_temp,1));	
	dis_num(lcd_update->set_temp,2);	
	dis_group(G_HUMI,1);
	(lcd_update->cur_temp > -99)?(dis_group(G_TEMP,1)):0;
	dis_group(G_SET,1);
	
	//PLC和阀  
	dis_blink(lcd_update->valve,lcd_update->plc,lcd_update->plc_run ,lcd_update->type);
	
	if(lcd_update->lock) dis_sign(SIGN_LOCK,1);
	if(lcd_update->time) dis_sign(SIGN_TIME,1);
	
	//-------------关机-------------------------------------
	if(0 == lcd_update->onoff) return;
	dis_wind(lcd_update->win_speed , lcd_update->type);
	dis_sign(lcd_update->mode ,1);
	
}
static err_t bu9796_init(device_t *dev)
{
   struct bu9796_data *bu9796 = (struct bu9796_data *)dev->user_data;
	uint8_t data[6 + LCD_BUF_SIZE] = { SET_SOFT_RESET, SET_BLINK_OFF, SET_IC, SET_AP_NORMAL, SET_SHOW_ON, SET_LOAD_DATA_ADDR };

	memset(&data[6], 0xFF, LCD_BUF_SIZE);                                   //此行为LCD全显  display_all 
	//memset(&data[6], 0x00, LCD_BUF_SIZE);                                    //此行为LCD什么也不显       display_none
	size_t len = bu9796_write_data(bu9796->i2c_bus, data, sizeof(data));
    return 0;
}
static size_t bu9796_write(device_t *dev, off_t pos, const void *buffer, size_t size)
{
    size_t len = 0;
    struct bu9796_data *bu9796 = (struct bu9796_data *)dev->user_data;
    uint8_t data[5 + LCD_BUF_SIZE] = { SET_BLINK_OFF, SET_IC, SET_AP_NORMAL, SET_SHOW_ON, SET_LOAD_DATA_ADDR };
		
		update_lcd_data((uint8_t *)buffer, size);
		
    memcpy(&data[5], lcd_buf, LCD_BUF_SIZE);
    len = bu9796_write_data(bu9796->i2c_bus, data, sizeof(data));
    return len;
}

static const struct device_ops bu9796_dev_ops =
{
		.init = bu9796_init,
    .write = bu9796_write,
};

static struct device bu9796_dev;
err_t lcd_init(const char *i2c_bus_name)
{
    struct i2c_bus_device *i2c_bus = (struct i2c_bus_device *)device_find(i2c_bus_name);
    if (i2c_bus == NULL)
    {
        log_d("\ni2c_bus %s for bu9796 not found!\n", i2c_bus_name);
        return -ENOSYS;
    }

    if (device_open(&i2c_bus->parent, NULL) != 0)
    {
        log_d("\ni2c_bus %s for bu9796 opened failed!\n", i2c_bus_name);
        return -EIO;
    }

    bu9796_data.i2c_bus  = i2c_bus;
    bu9796_dev.ops = &bu9796_dev_ops;
    bu9796_dev.user_data = &bu9796_data;

    device_register(&bu9796_dev, "b9796", 0);
    return 0;
}

#endif
