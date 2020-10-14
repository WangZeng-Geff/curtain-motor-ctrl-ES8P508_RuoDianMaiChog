#ifndef __I2C_DEV_H__
#define __I2C_DEV_H__

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_DEV_CTRL_10BIT        0x20
#define I2C_DEV_CTRL_ADDR         0x21
#define I2C_DEV_CTRL_TIMEOUT      0x22
#define I2C_DEV_CTRL_RW           0x23

struct i2c_priv_data
{
    struct i2c_msg  *msgs;
    size_t number;
};

struct i2c_bus_device;
err_t i2c_bus_device_device_init(struct i2c_bus_device *bus,
                                 const char            *name);

#ifdef __cplusplus
}
#endif

#endif
