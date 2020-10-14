#include <compiler.h>
#include <types.h>
#include <utils.h>
#include <printk.h>
#include <stdlib.h>
#include <os.h>
#include "dev.h"
#include "settings.h"
#include "board.h"
#include "update.h"
#include "proto_smart.h"
#include "plc_smart.h"
#include "dev_show.h"
#include "report\\auto_report_app.h"

#ifdef DEV_SHOW

struct DEV_SEARCH_PARAM dev_search_param;

//--------------------------------------------------------------------------------------------------
static void send_search_ack_frame(void)
{
    uint8_t len=0,dev_len,dev_infor[0x40],body[0x80];;

	if (is_all_xx((void*)setting.encode.id, 0x00, AID_LEN)
        || is_all_xx((void*)dev_search_param.search_id, 0x00, AID_LEN))
    {
        return;
    }
    dev_len = dev_ver_get(dev_infor,0x40);

    len = code_body(0x0007,0,setting.encode.sn, SN_LEN,body,sizeof(body));
    len += code_body(0x0005,0,setting.encode.dkey, DKEY_LEN,body+len,sizeof(body));
    len += code_body(0x000A,0,setting.encode.pwd, PW_LEN,body+len,sizeof(body));
    len += code_body(0x0003,0,dev_infor, dev_len,body+len,sizeof(body));
    
    len = code_frame(setting.encode.id,dev_search_param.search_id,dev_search_param.ack_seq,CMD_SHOW,body,len,g_frame_buf,sizeof(g_frame_buf));
    
	device_t *serial_plc = device_find("uart2");
	device_write(serial_plc, 0, g_frame_buf, len);
	set_bit(dev_state.act_flag,EN_PLC_RUN);
		dev_state.plc_run_cnt = 5;
}

void check_search_delay(void)
{
    if (dev_search_param.silent_time > 0)  dev_search_param.silent_time--;
	if (dev_search_param.ack_count > 0) 
    {
        dev_search_param.ack_count--;
        if (dev_search_param.ack_count > 0) return;
        send_search_ack_frame();
    }
}

#if 0
//---- please modify this function according u device -------
void check_dev_show(void)
{
	
}
#endif
//--------------------------------------------------------------------------------------------------
static uint8_t sn_type_check(uint8_t data[],uint8_t sn_data[])
{
    if (is_all_xx(data, 0xFF, TYPE_LEN))
    {
        return 1;
    }
    else if (((0x0f & data[1]) == (0x0f & sn_data[1])) \
            && ((0xf0 & data[3]) == (0xf0 & sn_data[3])) \
            && (data[2] == sn_data[2]))
    {
        return 1;
    }
    return 0;
}

//7E XX XX XX XX FF FF FF FF 00 0D 03  41 00  08 00   07   00 06 00 XX XX XX XX CS
uint8_t search_frame_opt(struct SmartFrame *pframe)
{
    struct Body * body = (struct Body *)&pframe->data[3];   
    uint8_t search_type,search_did[DID_LEN] = {0x08,0x00};


    if(!(is_all_xx(pframe->taid, 0xff, AID_LEN)) \
        || (memcmp(body->did,search_did,DID_LEN)) \
        || (pframe->len < (body->ctrl&0x7F) + FBD_FRAME_HEAD))   
        goto end;
        
        
    if((dev_search_param.silent_time > 0) || (!is_all_xx(setting.para.panid, 0x00, PANID_LEN)))   
        goto end;   
        
    search_type = body->data[0];//00,dev search, 01 key search
    if (is_gid_equal(&pframe->data[1],get_le_val(setting.para.sid, SID_LEN))) //41 00
    {
        clear_equipment_gid_flag();
		if((!search_type) && (sn_type_check(&body->data[1+TIME_LEN],setting.encode.sn))) 
        {
            memcpy(dev_search_param.search_id, pframe->said, AID_LEN); //save
			dev_search_param.max_delay_time = get_le_val(&body->data[1], TIME_LEN);;
            if(!dev_search_param.max_delay_time) dev_search_param.max_delay_time = 10;//default time
            dev_search_param.ack_seq = pframe->seq | 0x80;
            srand(jiffies + get_le_val(setting.encode.id,AID_LEN));
            dev_search_param.ack_count = (rand() % (dev_search_param.max_delay_time)) + 1;
        }
    }

end:
    return (0); //no ack     
}

//--------------------------------------------------------------------------------------------------

#endif

