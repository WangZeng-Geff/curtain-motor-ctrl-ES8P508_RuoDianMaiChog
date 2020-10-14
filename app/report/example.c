int code_frame(const uint8_t *src, const uint8_t *dest, int seq, int cmd,
			   const uint8_t *data, int len, void *out, int maxlen)
{
	const uint8_t addr[ID_LEN] = { 0x00, 0x00, 0x00, 0x00 };

	static uint8_t _seq      = 0;
	struct SHS_frame *pframe = (struct SHS_frame *)out;

    memmove(&pframe->infor[1], data, len);
	pframe->stc = STC;
	if (!src)
		src = addr;
	if (!dest)
		dest = addr;
	memcpy(pframe->said, src, ID_LEN);
	memcpy(pframe->taid, dest, ID_LEN);
	pframe->seq      = seq < 0 ? (_seq++ & 0x7F) : seq;
	pframe->infor[0] = cmd;
	
	pframe->len                = len + 1;
	pframe->infor[pframe->len] = checksum(pframe, SHS_FRAME_HEAD + pframe->len);
	return (SHS_FRAME_HEAD + pframe->len + 1);
}

uint8_t remote_frame_opt(struct SHS_frame *pframe)
{
	uint8_t ret = 0;
	uint8_t cmd, dlen, *body;

	if (pframe->len == 0)
		return (0);
    if(pframe->seq & 0x80)
	{
		if(RELIABLE == (pframe->infor[0] & 0x07))
		{
			report_frame_ack(pframe->seq,pframe->said,&pframe->infor[1]); 
		}
		return(0);
	}

	dlen = pframe->len - 1;
	cmd  = pframe->infor[0] & 0x07;
	body = pframe->infor + 1;
	switch (cmd)
    {
    case CMD_SET:
		#if MAX_ACTOR_NUM
        clear_equipment_gid_flag();
        #endif
    case CMD_GET:
        if (CMD_SET == cmd && is_all_xx(pframe->taid, 0xFF, ID_LEN))
		{
            set_group_parameter(body, dlen);
            #if MAX_ACTOR_NUM
            save_subscriber_infor(cmd,pframe->said,pframe->taid);
            #endif
		}
        else
            ret = do_cmd(cmd, body, dlen);
            #if MAX_ACTOR_NUM
            save_subscriber_infor(cmd,pframe->said,pframe->taid);
            #endif
        break;
	case CMD_UPDATE:
		ret = update_frame_opt(body, dlen);
		break;
	}
	return (ret + 1);             //add cmd
}

static uint8_t is_gid_equal(const uint8_t *data)
{
    uint8_t dlen       = get_bits(data[0], 0, 5);
    uint8_t group_type = get_bits(data[0], 6, 7);
    uint16_t gid       = get_le_val(eep_param.sid, sizeof(eep_param.sid));
    
	#if MAX_ACTOR_NUM
    uint8_t k,connt_size = 0;
    uint16_t j = 0;
    k = (gid%8) ? (gid%8):8;
	#endif
	
    data++;

    if (group_type == 0) /* bit type */
    {
        gid--;
        if (dlen < (gid >> 3) + 1)
	    {
			#if MAX_ACTOR_NUM
				goto deal_find_none;
			#else
			    return (0);
			#endif
		}         
        if (tst_bit(data[gid >> 3], gid & 0x07))
		{
		#if MAX_ACTOR_NUM
			for (j = 0;j < (gid>>3);j++)
			{
				connt_size += get_1byte_bit1_number(data[j],8);
			}
			connt_size += get_1byte_bit1_number(data[(gid>>3)],k);
			if (0 == judge_data.find_myself) 
			{
				judge_data.equipment_gid = connt_size; 
			}
			judge_data.find_myself = 1;
		#endif
			return (1); 
		}
    }
    else                 /* bytes type */
    {
        uint8_t i;
        uint8_t gid_unit_len = (group_type == 1 ? 1 : 2);

        for (i = 0; i < dlen; i += gid_unit_len)
        {
            uint16_t _gid = get_le_val(data + i, gid_unit_len);
            if (_gid == gid)
			{
			#if MAX_ACTOR_NUM
				if(0 == judge_data.find_myself)
				{
					judge_data.equipment_gid = connt_size + 1;
				}
				judge_data.find_myself = 1;
			#endif
				return (1);
			} 
            else if(_gid == 0)			
			{
            #if MAX_ACTOR_NUM
			    judge_data.equipment_gid += gid; 
			    judge_data.find_myself = 1;
		    #endif
				return(1);
			}
            #if MAX_ACTOR_NUM
                connt_size++;
            #endif
        }		
    }
#if MAX_ACTOR_NUM
deal_find_none:
	if(0 == judge_data.find_myself)
	{
	    if(0x00 == group_type)
	    {
			for (j = 0;j < dlen;j++)
			{
				judge_data.equipment_gid += get_1byte_bit1_number(data[j],8);
			}
		}
		else if(0x01 == group_type)
		{
			judge_data.equipment_gid += dlen;
		}
		else if(0x02 == group_type)
		{
			judge_data.equipment_gid += dlen/2;
		}
	}
#endif
  return (0);
}