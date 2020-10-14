#ifndef __I2C_BIT_OPS_H__
#define __I2C_BIT_OPS_H__

#ifdef __cplusplus
extern "C" {
#endif


//#define I2C_BIT_DEBUG

struct i2c_bit_ops
{
    void *data;            /* private data for lowlevel routines */
    void (*set_sda)(void *data, int32_t state);
    void (*set_scl)(void *data, int32_t state);
    int32_t (*get_sda)(void *data);
    int32_t (*get_scl)(void *data);

    void (*udelay)(unsigned long us);

    uint32_t delay_us;  /* scl and sda line delay */
    uint32_t timeout;   /* in tick */
};

err_t i2c_bit_add_bus(struct i2c_bus_device *bus,
                      const char            *bus_name);

#ifdef __cplusplus
}
#endif

#endif
