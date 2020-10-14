#ifndef _SI7020_H_
#define _SI7020_H_

#include <types.h>
#include "i2c\i2c.h"
#define SI7020_TEMPERATURE      0xF3
#define SI7020_HUMIDITY         0xF5
#define SI7020_SOFT_RESET       0xFE

#define SI7020_CTRL_GET_TH      0x7F
#define SI7020_CTRL_RESET       0x7E
#define SI7020_CTRL_TEMP_CMD   0X7D
#define SI7020_CTRL_GET_TEMP    0X7C
#define SI7020_CTRL_HUMI_CMD    0X7B
#define SI7020_CTRL_GET_HUMI    0X7A


;
struct si7020_data
{
    struct i2c_bus_device *i2c_bus;
};
struct TEMP_HUMI_CTRL
{
	uint8_t temp_humi_read_conter;
	uint8_t start_read_temp_flag;
	uint8_t start_read_humi_flag;
	uint8_t read_temp_flag;
	uint8_t read_humi_flag;
};
err_t si7020_init(const char *i2c_bus_name);
 int si7020_read_data(struct i2c_bus_device *i2c_bus, uint8_t *values, size_t length);
int si7020_get_sensor_value(const struct si7020_data *si7020, uint8_t *val);
void update_temp_humi(void);
#endif
