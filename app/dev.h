#ifndef _DEV_H_
#define _DEV_H_

#include <device.h>


extern device_t *dev_key, *dev_led, *dev_buz, *dev_relay;

#define ON 1
#define OFF 0


void dev_show(void);

int dev_type_get(void *out, int maxlen);
int dev_type_cmp(const void *dt, size_t len);
int dev_ver_get(void *out, int maxlen);
int dev_ver_cmp(const void *ver, size_t len);
void data_recovery(void);
void dev_show_start(void);
uint8_t relay_status_save(uint8_t state);
#endif
