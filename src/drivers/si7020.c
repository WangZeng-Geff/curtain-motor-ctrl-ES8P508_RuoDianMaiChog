#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <bitops.h>
#include <device.h>
#include <printk.h>
#include <syserr.h>
#include <settings.h>
#include "lib_config.h"
#include "board.h"
#include "os.h"
#include "dev.h"


#define SI7020_ADDR    0x40

#ifdef configUSING_SI7020


static struct si7020_data si7020_data;

//static int si7020_read_reg(struct i2c_bus_device *i2c_bus, uint8_t reg, uint8_t *values, size_t length)
//{
//    struct i2c_msg msgs[2];

//    msgs[0].addr  = SI7020_ADDR;
//    msgs[0].flags = I2C_WR;
//    msgs[0].buf   = &reg;
//    msgs[0].len   = 1;

//    msgs[1].addr  = SI7020_ADDR;
//    msgs[1].flags = I2C_RD; /* Read from slave */
//    msgs[1].buf   = (uint8_t *)values;
//    msgs[1].len   = length;

//    if (i2c_transfer(i2c_bus, msgs, 2) == 2)
//        return 0;

//    return -EIO;
//}

 int si7020_write_reg(struct i2c_bus_device *i2c_bus, uint8_t reg, uint8_t *values, size_t length)
{
    uint8_t tmp[0x20];
    struct i2c_msg msgs[1];
    
    tmp[0] = reg;
    memcpy(&tmp[1], values, length);

    msgs[0].addr  = SI7020_ADDR;
    msgs[0].flags = I2C_WR;
    msgs[0].buf   = tmp;
    msgs[0].len   = length+1;

    if (i2c_transfer(i2c_bus, msgs, 1) == 1)
        return 0;

    return -EIO;
}

 int si7020_read_data(struct i2c_bus_device *i2c_bus, uint8_t *values, size_t length)
{
    struct i2c_msg msgs[1];

    msgs[0].addr  = SI7020_ADDR;
    msgs[0].flags = I2C_RD;
    msgs[0].buf   = (uint8_t *)values;
    msgs[0].len   = length;

    if (i2c_transfer(i2c_bus, msgs, 1) == 1)
        return 0;

    return -EIO;
}
struct TEMP_HUMI_CTRL temp_humi_ctrl;
//static struct si7020_data *si7020_;
//static uint8_t *val_;
//int si7020_get_sensor_value_(struct si7020_data *si7020,uint8_t *val)
//{
//	si7020_=si7020;
//	val_=val;
//}
//static int si7020_get_sensor_value(const struct si7020_data *si7020, uint8_t *val)
//{
//	si7020_=si7020;
//	val_=val;
//}
int si7020_get_sensor_value(const struct si7020_data *si7020, uint8_t *val)
{
	int err;
	if(temp_humi_ctrl.temp_humi_read_conter==2)
	{
		err		= si7020_write_reg(si7020->i2c_bus, SI7020_TEMPERATURE, NULL, 0);
			if (err < 0) return (err);
	}		
	else if(temp_humi_ctrl.temp_humi_read_conter==3)
	{
		err = si7020_read_data(si7020->i2c_bus, val, 2);
			if (err < 0) return (err);
		
	}
	else if (temp_humi_ctrl.temp_humi_read_conter==4)
	{
		err = si7020_write_reg(si7020->i2c_bus, SI7020_HUMIDITY, NULL, 0);
		if (err < 0) return (err);
	}
	else if (temp_humi_ctrl.temp_humi_read_conter==5)
	{
		err = si7020_read_data(si7020->i2c_bus, &val[2], 2);
		if (err < 0) return (err);
		temp_humi_ctrl.temp_humi_read_conter=0;
		temp_humi_ctrl.start_read_humi_flag=0;
	}
    return 0;
}

int si7020_read_temp_cmd(const struct si7020_data *si7020, uint8_t *val)
{
	int err		= si7020_write_reg(si7020->i2c_bus, SI7020_TEMPERATURE, NULL, 0);
			if (err < 0) return (err);
	return 0;
}
int si7020_read_temp_value(const struct si7020_data *si7020, uint8_t *val)
{
	int err = si7020_read_data(si7020->i2c_bus, val, 2);
			if (err < 0) return (err);
	 err = si7020_write_reg(si7020->i2c_bus, SI7020_HUMIDITY, NULL, 0);
		if (err < 0) return (err);
	return 0;
}
int si7020_read_humi_cmd(const struct si7020_data *si7020, uint8_t *val)
{
//	int err = si7020_write_reg(si7020->i2c_bus, SI7020_HUMIDITY, NULL, 0);
//		if (err < 0) return (err);
//	return 0;
  return 0;
}
int si7020_read_humi_value(const struct si7020_data *si7020, uint8_t *val)
{
	int err = si7020_read_data(si7020->i2c_bus, &val[2], 2);
		if (err < 0) return (err);
	return 0;
}

int si7020_reset(const struct si7020_data *si7020, uint8_t *val)
{
	int err	= si7020_write_reg(si7020->i2c_bus, SI7020_CTRL_RESET, NULL, 0);
			if (err < 0) return (err);
	return 0;
}
	
static err_t si7020_ctrl(struct device *dev, uint8_t cmd, void *args)
{
    err_t err = 0;
    struct si7020_data *si7020 = (struct si7020_data *)dev->user_data;

    switch (cmd)
    {
        case SI7020_CTRL_GET_TH:
        {
            err = si7020_get_sensor_value(si7020, args);
        }break;
        
        case SI7020_CTRL_TEMP_CMD:
        {
            err=si7020_read_temp_cmd(si7020, args);
        }break;
        case SI7020_CTRL_GET_TEMP:
        {
            err=si7020_read_temp_value(si7020, args);
        }break;
        case SI7020_CTRL_HUMI_CMD:
        {
            err=si7020_read_humi_cmd(si7020, args);
        }break;
        case SI7020_CTRL_GET_HUMI:
        {
            err=si7020_read_temp_value(si7020, args);
        }break;
        //err = si7020_get_sensor_value(si7020, args);
     case SI7020_CTRL_RESET:
         err = si7020_reset(si7020, args);
        break;
    }

    return err;
}

static const struct device_ops si7020_dev_ops =
{
    .ctrl = si7020_ctrl,
};

static struct device si7020_dev;

err_t si7020_init(const char *i2c_bus_name)
{
    struct i2c_bus_device *i2c_bus = (struct i2c_bus_device *)device_find(i2c_bus_name);
    if (i2c_bus == NULL)
    {
        log_d("\ni2c_bus %s for si7020 not found!\n", i2c_bus_name);
        return -ENOSYS;
    }

    if (device_open(&i2c_bus->parent, NULL) != 0)
    {
        log_d("\ni2c_bus %s for si7020 opened failed!\n", i2c_bus_name);
        return -EIO;
    }

    si7020_data.i2c_bus  = i2c_bus;
    si7020_dev.ops = &si7020_dev_ops;
    si7020_dev.user_data = &si7020_data;

    device_register(&si7020_dev, "si7020", 0);
    return 0;
}
#endif


int breakdown_temp = 0;
int16_t temp_temp,humi_temp;
void update_temp_humi(void)
{
    uint8_t th_val[4];
    device_t *si7020 = device_find("si7020");
    assert(si7020);
    err_t err;
	IWDT_Clear();
    if(temp_humi_ctrl.temp_humi_read_conter==2) 
    {
      err=device_ctrl(si7020, SI7020_CTRL_TEMP_CMD, th_val);
    } 
    else if(temp_humi_ctrl.temp_humi_read_conter==4) 
    {
			err=device_ctrl(si7020, SI7020_CTRL_GET_TEMP, th_val);
			if (err != 0) 
			{				
				breakdown_temp++;
				if(breakdown_temp>=3)
				{
					dev_state.sensor.breakdown = 1 ;
					breakdown_temp = 0;
					//err=device_ctrl(si7020, SI7020_CTRL_RESET, th_val);
					log_d("si7020 read TEMP ERROR!\n");
				}
			} 
			else 
			{
				breakdown_temp = 0;
				dev_state.sensor.breakdown = 0 ; 
				double tmp = (double)get_be_val(th_val, 2);
				tmp = (tmp * 175.72f / 65536.0f) - 46.85f;
				if(dev_state.sensor.poweron != 1)//无效时间
					dev_state.sensor.temp = (int16_t)(tmp * 10 + setting.dev_infor.temp_compensation);
				else
				{
					temp_temp = (int16_t)(tmp * 10 + setting.dev_infor.temp_compensation);
					if(abs(temp_temp - dev_state.sensor.temp)<50)//波动小于5度
						dev_state.sensor.temp = temp_temp;
	
				}
				
			}
    }
    else if(temp_humi_ctrl.temp_humi_read_conter==6)
    {
			err=device_ctrl(si7020, SI7020_CTRL_GET_HUMI, &th_val[2]);
			if (err != 0) 
			{			
				breakdown_temp++;
				if(breakdown_temp>=3)
				{
					dev_state.sensor.breakdown = 1 ;
					breakdown_temp = 0;
					//err=device_ctrl(si7020, SI7020_CTRL_RESET, th_val);
					log_d("si7020 read HUMI ERROR!\n");
				}
			} 
			else 
			{

				breakdown_temp = 0;
				dev_state.sensor.breakdown = 0 ; 
				double tmp = (double)get_be_val(th_val, 2);
				tmp = (tmp * 175.72f / 65536.0f) - 46.85f;
				tmp = (double)get_be_val(&th_val[2], 2);
				tmp = (tmp * 125.0f / 65536.0f) - 6.0f;//

				tmp = (tmp*10+ setting.dev_infor.humi_compensation);//进行温度补偿
				tmp = tmp < 0 ? 0 : tmp;          //判断补偿会的值
				tmp = tmp > 999 ? 999 : tmp;
				dev_state.sensor.humi = (uint16_t)tmp;        ;
			}
    }
}









//发送测量指令，硬件测量时间,温度≤85ms，湿度≤29ms

/************************ 
 * CRC数据校验
 * 
 ************************/
/*
const uint16_t POLYNOMIAL = 0x131;   //P(x)=x^8+x^5+x^4+1 = 100110001
static uint8_t SHT20_checkcrc(uint8_t data[], uint8_t checksum)
{
    uint8_t crc = 0;
    uint8_t byteCtr;
    uint8_t bit;

    for (byteCtr = 0; byteCtr < 2; ++byteCtr)
    {
        crc ^= (data[byteCtr]);
        for (bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ POLYNOMIAL;
            }
            else
            {
                crc = (crc << 1);
            }
        }
    }
    return crc == checksum;
}
*/




