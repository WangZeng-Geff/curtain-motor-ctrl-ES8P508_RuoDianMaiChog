#include <utils.h>
#include <config.h>
#include <types.h>
#include <os.h>
#include "ES8P508x.h"
#include "lib_config.h"
#include "encode.h"
#include "board.h"
#include "settings.h"

static void encode_timer_setmat(uint16_t value)
{
	T16Nx_SetMAT0(T16N3,value);
    T16Nx_SetCNT(T16N3, 0);
    T16Nx_ClearIFPendingBit(T16N3, TIM_IF_MAT0);
}

static void encode_timer_init(void)
{
	TIM_BaseInitStruType x;
	
	x.TIM_ClkS = TIM_ClkS_PCLK;
    x.TIM_SYNC = DISABLE;
    x.TIM_EDGE = TIM_EDGE_Rise;
    x.TIM_Mode = TIM_Mode_TC0;
    T16Nx_BaseInit(T16N3,&x);
	
	T16Nx_SetPREMAT(T16N3,0);
    T16Nx_SetMAT0(T16N3,ENCODE_PERIOD);
    T16Nx_MAT0ITConfig(T16N3,TIM_Clr_Int);
	T16N3_Enable();
}

static void wait_t(void)
{
	while (!T16Nx_GetFlagStatus(T16N3, TIM_IF_MAT0));
	T16Nx_ClearIFPendingBit(T16N3, TIM_IF_MAT0);
}

static void encode_release(void)
{
	T16N3_Disable();
}

static uint16_t _send_fmt(uint8_t x)
{
	uint16_t databit = x;

	databit |= 1 << 8;          //stop bit
	return (databit);
}

static uint8_t recv_chk(uint16_t x, uint16_t (*fun)(uint8_t))
{
	uint8_t data = (uint8_t)(x & 0xFF);

	return (x == fun(data));
}

#define MAX_BIT_CNT     (1 + 8 + 1)
static void put_char(uint8_t ch)
{
	uint16_t data;
	uint8_t i;

	data   = _send_fmt(ch);
	data <<= 0x01;
	data  &= (1 << 10) - 1;

	wait_t();
	for (i = 0; i < MAX_BIT_CNT; i++)
	{
		if (data & 0x01)
			TX_SDA_SET();
		else
			TX_SDA_CLR();
		data >>= 0x01;
		wait_t();
	}
}

static void send_to_sda(uint8_t buf[], uint8_t len)
{
	uint8_t i;

	ENCODE_SDA_OUT();
	wait_t();
	for (i = 0; i < len; i++)
	{
		put_char(buf[i]);
	}
}

#define MAX_TIME_OVER   (500)   /*n*100us debug modify */
static uint8_t recv_from_sda(uint8_t *c)
{
	uint16_t data         = 0, time_over = 0;
	volatile uint16_t bit = 1;
	uint8_t i;

	ENCODE_SDA_IN();
	while (RX_SDA_VALUE())
	{
		if (T16Nx_GetFlagStatus(T16N3, TIM_IF_MAT0) != RESET)
		{
			T16Nx_ClearIFPendingBit(T16N3, TIM_IF_MAT0);
			time_over++;
			if (time_over >= MAX_TIME_OVER)
				return (0);
		}
		board_feed_wdg();
	}
	encode_timer_setmat(ENCODE_PERIOD >> 1);
	wait_t();
	encode_timer_setmat(ENCODE_PERIOD);
	for (i = 0; i < MAX_BIT_CNT - 1; i++)
	{
		wait_t();
		if (RX_SDA_VALUE())
			data |= bit;
		bit <<= 1;
	}
	if (recv_chk(data, _send_fmt))
	{
		*c = (uint8_t)(data & 0xFF);
		return (1);
	}
	return (0);
}

static struct CODE_FRAME *get_code_frame(uint8_t buf[], uint8_t len)
{
	uint8_t i, length;
	struct CODE_FRAME *pframe = NULL;

	for (i = 0; i < len; i++)
	{
start_lbl:
		if (STC == buf[i])
			break;
	}
	if (i >= len)
		return (NULL);

	pframe = (struct CODE_FRAME *)&buf[i];
	length = pframe->len;
	if ((length > len) || (pframe->data[length] != checksum(&buf[i], length + 2)))
	{
		i++;
		goto start_lbl;
	}
	return (pframe);
}

static uint8_t get_device_infor(uint8_t buf[], uint8_t len)
{
	struct CODE_FRAME *pframe;

	if (len < 2)
		return (0);

	pframe = get_code_frame(buf, len);
	if (!pframe || (ENCODE_LEN != pframe->len)) return (0);

	memcpy(&setting.encode, pframe->data, pframe->len);
	setting_save();
	setting_load();
	memcpy(pframe->data, &setting.encode, pframe->len);
	pframe->data[pframe->len] = checksum((uint8_t *)pframe, pframe->len + 2);
	send_to_sda((uint8_t *)pframe, pframe->len + 3);
	return (1);
}

extern uint8_t g_frame_buf[1024];
void device_encode_opt(void)
{
    static uint8_t tmp_buf[4] = { 0x7E, 0x01, 0x00, 0x7F };
    uint8_t try_cnt           = 3, ret, len;

    encode_timer_init();

    while (try_cnt--)
    {
        send_to_sda(tmp_buf, sizeof((tmp_buf)));

        len = 0;
        do
        {
            board_feed_wdg();
            ret = recv_from_sda(&g_frame_buf[len]);
            len++;
        }
        while (ret);

        if (get_device_infor(g_frame_buf, len)) break;
        board_feed_wdg();
    }
    encode_release();
}


