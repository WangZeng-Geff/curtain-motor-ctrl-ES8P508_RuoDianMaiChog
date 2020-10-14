
#include <config.h>
#include <utils.h>
#include <device.h>
#include "board.h"
#include "settings.h"
#include "dev.h"
#include "printk.h"
setting_t setting;
dev_state_info_t dev_state;
static uint32_t setting_start_addr = SETTING_INVALID_ADDR;
static device_t * iflash;

/*
 * for disk use
 */
static void check_disk_open(void)
{
	if (iflash)
		return;

	iflash = device_find("iflash");
	assert(iflash);
	device_open(iflash, 0);
}


int disk_erase(uint32_t off, uint32_t size)
{
	check_disk_open();
	struct flash_erase_cmd sfec;
	sfec.start = off;
	sfec.size = size;
	return device_ctrl(iflash, DEVICE_CTRL_BLK_ERASE, &sfec);
}


int disk_write(uint32_t off, const void * data, uint32_t size)
{
	check_disk_open();
	return device_write(iflash, off, data, size);
}


int disk_read(uint32_t off, void * data, uint32_t size)
{
	check_disk_open();
	return device_read(iflash, off, data, size);
}


void dev_restore_factory(void)
{
	memset(&setting.para, 0x00, sizeof(setting.para));
	memset(setting.ctrl, 0x00, sizeof(setting.ctrl));
	int i;

	for (i = 0; i < 4; i++)
		setting.ctrl[i].idx = i + 1;
	if(setting.dev_infor.actuation_time == 0)
		setting.dev_infor.actuation_time = 120;//changed by wz 20200612
#if 0
	memset(&setting.dev_infor, 0x00, sizeof(setting.dev_infor));
	setting.dev_infor.report = 0x03;
	setting.dev_infor.temp_report_freq = 0; 		/*关闭定频上报*/
	setting.dev_infor.temp_report_step = 10;		/*默认步长为1度*/
	setting.dev_infor.humi_report_freq = 0; 		/*关闭定频上报*/
	setting.dev_infor.humi_report_step = 50;		/*默认步长为5%*/
	setting.dev_infor.mode = MODE_VENTILATE;		/*默认通风模式*/
	setting.dev_infor.win_speed = WIN_AUTO; 		/*默认低风*/
	setting.dev_infor.set_temp = 200;				/*默认20.0度*/
	setting.dev_infor.return_diff_temp = 10;		/*回差温度 默认1度*/
	setting.dev_infor.low_protect_temp = 50;
	setting.dev_infor.low_deadband = 30;			/*低温回差温度 默认3度  即 5-8度*/
	setting.dev_infor.power_delay = 60; 			/*上电传感器延时时间*/
	setting.dev_infor.low_temp_switch = 1;			/*低温保护开关*/
	setting.dev_infor.panel_lock = 0;				/*面板锁定*/
	setting.dev_infor.air_coner_switch = 0; 		/*空调开关*/
	setting.dev_infor.backlight_enable = 1; 		/*背光*/
	setting.dev_infor.auto_disable = 1; 			/*自动风模式下 达到设定温度后是否继续通风*/
	setting.dev_infor.max_heat_temp = 300;			//最大制热温度
	setting.dev_infor.min_heat_temp = 160;			//最小制热温度
	setting.dev_infor.max_cool_temp = 300;			//最大制冷温度
	setting.dev_infor.min_cool_temp = 160;			//最小制冷温度 
	setting.dev_infor.humi_compensation = 30;		//湿度补偿
	setting.dev_infor.temp_compensation = -15;		//温度补偿
	setting.dev_infor.min_set_temp = 50;			//最小设置温度
	setting.dev_infor.max_set_temp = 300;			//最大设置温度
#endif
}


static void setting_default(void)
{
	memset(&setting, 0, sizeof(setting));
	dev_restore_factory();
}


void setting_save(void)
{

setting_save_start:
	check_disk_open();
	setting_hdr_t hdr;
	uint32_t addr = setting_start_addr + sizeof(setting_t);

	//		log_d("the setting size : 0x%06X!\r\n", sizeof(setting_t));
	bool new_sec = false;

	if (addr + sizeof(setting_t) >= SETTING_FROM + SETTING_SIZE)
	{
		addr = SETTING_FROM;
		new_sec = true;
	}

	if ((addr & FLASH_SECTOR_MASK) +sizeof(setting_t) > FLASH_SECTOR_SIZE)
	{
		addr = (addr + FLASH_SECTOR_SIZE) & (~FLASH_SECTOR_MASK);
		new_sec = true;
	}

	if (! (addr & FLASH_SECTOR_MASK))
	{
		new_sec = true;
	}

	/* erase next sector if a new sector */
	if (new_sec)
	{
		struct flash_erase_cmd sfec;
		sfec.start = addr;
		sfec.size = FLASH_SECTOR_SIZE;
		device_ctrl(iflash, DEVICE_CTRL_BLK_ERASE, &sfec);
	}

	/* check memory */
	setting_t tmp;

	device_read(iflash, addr, (uint8_t *) &tmp, sizeof(tmp));
	log_d("the next valid addr: 0x%06X!\r\n", addr);

	/***********************防止数据不全是FF造成的 数据位写不上的问题******************************
		*********/
	if (! (is_all_xx((uint8_t *) &tmp, 0xFF, sizeof(tmp))))
	{
		assert(is_all_xx((uint8_t *) &tmp, 0xFF, sizeof(tmp)));
		setting_start_addr = addr;
		goto setting_save_start;
	}

	/*****************************************************************************************************/
	if (!device_write(iflash, addr + sizeof(setting_hdr_t), setting_addr(setting), SETTING_DAT_SIZE))
		log_d("setting_save write error!\r\n");

	hdr.crc = crc16(0, setting_addr(setting), SETTING_DAT_SIZE);
	hdr.magic = SETTING_MAGIC;
	device_write(iflash, addr, &hdr, sizeof(hdr));

	if (setting_start_addr < SETTING_FROM + SETTING_SIZE)
	{
		memset(&hdr, 0, sizeof(hdr));
		device_write(iflash, setting_start_addr, &hdr, sizeof(hdr));
	}

	setting_start_addr = addr;
}


void setting_load(void)
{
	check_disk_open();
	setting_hdr_t hdr;
	uint32_t addr = SETTING_FROM;

	for (addr = SETTING_FROM; addr < SETTING_FROM + SETTING_SIZE; addr += sizeof(setting_t))
	{
		if ((addr & FLASH_SECTOR_MASK) +sizeof(setting_t) > FLASH_SECTOR_SIZE)
		{
			addr = (addr + FLASH_SECTOR_SIZE) & (~FLASH_SECTOR_MASK);
		}

		board_feed_wdg();

		/* check magic no. */
		device_read(iflash, addr, &hdr, sizeof(setting_hdr_t));

		if (hdr.magic != SETTING_MAGIC)
			continue;

		/* check crc */
		device_read(iflash, addr + SETTING_HDR_SIZE, setting_addr(setting), SETTING_DAT_SIZE);

		if (hdr.crc != crc16(0, setting_addr(setting), SETTING_DAT_SIZE))
			continue;

		/* got really settings */
		setting_start_addr = addr;

		if(setting.dev_infor.actuation_time == 0)
			setting.dev_infor.actuation_time = 120;//changed by wz 20200612
		log_d("setting_load here has data addr: 0x%06X!\r\n", addr);
		return;
	}

	setting_default();
	setting_save();
}


