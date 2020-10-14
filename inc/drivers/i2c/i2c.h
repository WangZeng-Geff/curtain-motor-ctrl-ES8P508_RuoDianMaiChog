#ifndef __I2C_H__
#define __I2C_H__

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_WR                0x0000
#define I2C_RD               (1u << 0)
#define I2C_ADDR_10BIT       (1u << 2)  /* this is a ten bit chip address */
#define I2C_NO_START         (1u << 4)
#define I2C_IGNORE_NACK      (1u << 5)
#define I2C_NO_READ_ACK      (1u << 6)  /* when I2C reading, we do not ACK */

struct i2c_msg
{
    uint16_t addr;
    uint16_t flags;
    uint16_t len;
    uint8_t  *buf;
};

struct i2c_bus_device;

struct i2c_bus_device_ops
{
    size_t (*master_xfer)(struct i2c_bus_device *bus,
                          struct i2c_msg msgs[],
                          uint32_t num);
    size_t (*slave_xfer)(struct i2c_bus_device *bus,
                         struct i2c_msg msgs[],
                         uint32_t num);
    err_t(*i2c_bus_control)(struct i2c_bus_device *bus,
                            uint32_t,
                            uint32_t);
};

/*for i2c bus driver*/
struct i2c_bus_device
{
    struct device parent;
    const struct i2c_bus_device_ops *ops;
    uint16_t  flags;
    uint16_t  addr;
    uint32_t  timeout;
    uint32_t  retries;
    void *priv;
};

//#define I2C_DEBUG

#ifdef I2C_DEBUG
#define i2c_dbg(fmt, ...)   printk(fmt, ##__VA_ARGS__)
#else
#define i2c_dbg(fmt, ...)
#endif

err_t  i2c_bus_device_register(struct i2c_bus_device *bus, const char *bus_name);
struct i2c_bus_device *i2c_bus_device_find(const char *bus_name);
size_t i2c_transfer(struct i2c_bus_device *bus, struct i2c_msg msgs[], uint32_t num);
size_t i2c_master_send(struct i2c_bus_device *bus,
                       uint16_t               addr,
                       uint16_t               flags,
                       const uint8_t         *buf,
                       uint32_t               count);
size_t i2c_master_recv(struct i2c_bus_device *bus,
                       uint16_t               addr,
                       uint16_t               flags,
                       uint8_t               *buf,
                       uint32_t               count);
int    i2c_core_init(void);

#ifdef __cplusplus
}
#endif

#endif
