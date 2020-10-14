#include <config.h>
#include <board.h>
#include <init.h>
#include <os.h>
#include <settings.h>
#include <printk.h>
#include "ES8P508x.h"
#include "lib_config.h"
#include "es8p508_rtc.h"
#include "device.h"


#ifdef configUSINGTIME_RTC

static struct device rtc_dev;

/* 年月日周 时分秒*/
static int rtc_get_time( uint8_t *tm)
{
	uint32_t *tmp = 0;/*采用12小时 AM PM标志*/
	*tm = RTC_ReadYear()-2000;
	*(tm+1) = RTC_ReadMonth();
	*(tm+2) = RTC_ReadDay();
	*(tm+3) = RTC_ReadWeek();
	*(tm+4) = RTC_ReadHour(tmp);
	*(tm+5) = RTC_ReadMinute();
	*(tm+6) = RTC_ReadSecond();
#ifdef configRTC_DBG
	log_d("TIME:YEAR:20%02d MONTH:%02d DAY:%02d WEEK:%d  %02d:%02d:%02d\n",*(tm),*(tm+1),*(tm+2),*(tm+3),*(tm+4),*(tm+5),*(tm+6));
#endif
  return 7;
}
static int rtc_set_date(const uint8_t *tm)
{
	RTC_WriteYear(*tm+2000);
	RTC_WriteMonth(*(tm+1));
	RTC_WriteDay(*(tm+2));
	RTC_WriteWeek(*(tm+3));
//	RTC_WriteHour(*(tm+4),0);
//	RTC_WriteMinute(*(tm+5));
//	RTC_WriteSecond(*(tm+6));
#ifdef configRTC_DBG
	log_d("SET_DATE:YEAR:20%02d MONTH:%02d DAY:%02d WEEK:%2d\n",*(tm),*(tm+1),*(tm+2),*(tm+3));
#endif
	return 0;
}
static int rtc_set_time(const uint8_t *tm)
{
//	RTC_WriteYear(*tm);
//	RTC_WriteMonth(*(tm+1));
//	RTC_WriteDay(*(tm+2));
//	RTC_WriteWeek(*(tm+3));
	RTC_WriteHour(*(tm+4),0);
	RTC_WriteMinute(*(tm+5));
	RTC_WriteSecond(*(tm+6));
#ifdef configRTC_DBG
	log_d("SET_TIME:%02d:%02d:%02d\n",*(tm+4),*(tm+5),*(tm+6));
#endif
	return 0;
}
static int rtc_add_alarm(const uint8_t *tm)
{
	return 0;
}
	

static int rtc_del_alarm(const uint8_t *tm)
{
	return 0;
}
	

/**************************************************************************
  * @brief     : 
  * @param[in] : 
  * @param[out]: 
  * @return    : 
  * @others    : 设置传进来的都是 全部参数
***************************************************************************/
static err_t rtc_control(struct device *dev, uint8_t cmd, void *args)
{
    err_t err = 0;

    switch (cmd)
    {
    case RTC_CTRL_GET_TIME:
        rtc_get_time(args);
        break;
    case RTC_CTRL_SET_TIME:
        rtc_set_time(args);
        break;
		case RTC_CTRL_SET_DATE:
        rtc_set_date(args);
        break;
		case RTC_CTRL_ADD_ALARM :
				rtc_add_alarm(args);
				break;
		case RTC_CTRL_DEL_ALARM :
				rtc_del_alarm(args);
				break;
    }
    
    return err;
}

static const struct device_ops rtc_ops =
{
    .ctrl = rtc_control,
};

int board_setup_rtc(void)
{
	  RTC_Init(RTC_LRC,RTC_HOUR24);
    rtc_dev.ops = &rtc_ops;
    device_register(&rtc_dev, "rtc", 0);

    return 0;
}
device_initcall(board_setup_rtc);

#endif
