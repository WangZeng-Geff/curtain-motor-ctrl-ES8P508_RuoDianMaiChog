#ifndef _HDC1080_H_
#define _HDC1080_H_

#include <types.h>


#define HDC1080_CTRL_GET_TH         0xFF
#define HDC1080_CTRL_GET_ID         0xFE

/* Register addresses */
#define HDC1080_TEMPERATURE         0x00
#define HDC1080_HUMIDITY            0x01
#define HDC1080_CONFIG              0x02
#define HDC1080_SERIAL_ID1          0xfb
#define HDC1080_SERIAL_ID2          0xfc
#define HDC1080_SERIAL_ID3          0xfd
#define HDC1080_ID_MANU             0xfe
#define HDC1080_ID_DEV              0xff

#define HDC1080_RH_RES_14           0x00
#define HDC1080_RH_RES_11           0x01
#define HDC1080_RH_RES8             0x02

#define HDC1080_T_RES_14            0x00
#define HDC1080_T_RES_11            0x01

#define HDC1080_HEAT_OFF            0x00
#define HDC1080_HEAT_ON             0x01


err_t hdc1080_init(const char *i2c_bus_name);

#endif
