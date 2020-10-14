#ifndef _LCD_H
#define _LCD_H

#include <types.h>


#define BU9796_ADDR         (0x7C >> 1)

#define SET_LOAD_DATA_ADDR  0x00            //0x00
#define SET_SOFT_RESET      0xEA            //ICSET
#define SET_IC              0xE8
#define SET_BLINK_OFF       0xF0            //BLINK
#define SET_BLINK_ON        0xF1            //BLINK 0.5Hz
#define SET_AP_OFF          0xFD            //ALL PIX
#define SET_AP_NORMAL       0xFC
#define SET_SHOW_ON         0xC8            //MODE SET
#define SET_SHOW_OFF        0xC0            //MODE SET
#define SET_DISPLAY         0xA2            //DISCTRL

#define LCD_BUF_SIZE        9


#define AA 1
#define PP 2
#define GD 3
#define PD 4
#define SD 5

#define TC 6
#define HC 7
#define RT 8
#define LC 9
#define CL 10
#define LP 11
#define LT 12
#define RS 13
#define ER 14
//
#define SIGNON 15
#define SIGNOFF 16

#define LO 17  //低温小于-9.9

 
struct LCD_ALPHA
{
    unsigned char alpha;
    unsigned char val;
};
struct LCD_SIGN
{
    unsigned char sign;
    unsigned char group;
    unsigned char idx;
    unsigned char val;
};

typedef struct LCD_UPDATE_STR 
{
	int16_t cur_temp;
	uint16_t set_temp;
	int16_t cur_humi;
	uint8_t breakdown;//传感器故障
	uint8_t valve;
	uint8_t plc;
	uint8_t plc_run;
	uint8_t win_speed;
	uint8_t mode;
	uint8_t onoff;
	
	uint8_t lock;
	uint8_t time;

	uint8_t type;	
	uint8_t special_num;
    
} lcd_update_t,*lp_lcd_update_t;
static lcd_update_t lcd_update_temp; 

err_t lcd_init(const char *i2c_bus_name);

#endif
