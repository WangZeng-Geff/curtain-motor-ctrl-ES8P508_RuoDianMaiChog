#ifndef _BOARD_H_
#define _BOARD_H_

#include "port.h"

#define BOOT_SIZE           (4*1024)
#define APP_SIZE            (64*1024)

/*
 * for internal flash
 */
#define IFLASH_START_ADDR   (0u)
#define APP_ADDR            (IFLASH_START_ADDR + BOOT_SIZE)
#define UPDATE_PARA_ADDR    (APP_ADDR + APP_SIZE)
#define UPDATE_PARA_SIZE    (8u)
#define UPDATE_DATA_ADDR    (UPDATE_PARA_ADDR + UPDATE_PARA_SIZE)


void board_udelay(unsigned long us);
void board_mdelay(unsigned long ms);

void board_feed_wdg(void);

void board_setup(void);
void board_reboot(unsigned long ms100);
void dev_show_display_start(void);
void dev_show_display_end(void);
void dev_restore_display(void);
#endif
