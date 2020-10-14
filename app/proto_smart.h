#ifndef _PROTO_SMART_H_
#define _PROTO_SMART_H_

#include <types.h>
#include <utils.h>


//---------------------------------------------------------------------------------------
/*
 * frame flags
 */
#define STC					0x7E
#define DID_LEN         	0x02
#define SID_LEN         	0x02
#define AID_LEN         	0x04
#define PANID_LEN			0x02
#define BUF_LEN				0xFF
#define EID_LEN				0x08
#define PW_LEN          	0x02
#define PSK_LEN 			0x08
#define SN_LEN          	0x0C
#define DKEY_LEN        	0x08
#define MAGIC_NUM_LEN		0x04
#define WDATA_NUM			0x0A
#define WDATA_LEN			0x40

//---------------------------------------------------------------------------------------
/*
 * frame command
 */
#define CMD_SET_AID     	0x01
#define CMD_GET_AID     	0x03
#define CMD_ACK_AID     	0x13
#define CMD_DEL_AID     	0x04
#define CMD_REQ_AID     	0x05
#define CMD_GET_SID     	0x06
#define CMD_ACK_SID     	0x16
#define CMD_GET_EID     	0x07
#define CMD_ACK_EID     	0x17
#define CMD_SET_BPS     	0x08
#define CMD_SET_REG     	0x09
#define CMD_UNLINK      	0x0A
#define CMD_REGINFOR    	0x0B
#define CMD_SET_PANID   	0x0C
#define CMD_GET_GWAID   	0x0D
#define CMD_GET_VER     	0x0E
#define CMD_ACK_VER     	0x1E
#define CMD_GET_PANID   	0x0F
#define CMD_ACK_PANID   	0x1F
#define CMD_TST_PLC     	0x20
#define CMD_CHG_TONE    	0x21

#define CMD_REQ_UPDATE		0x23

#define CMD_ACK         	0x00
#define CMD_NAK         	0xFF

#define CMD_SET         	0x07
#define CMD_GET        		0x02
#define CMD_UPDATE      	0x05
#define CMD_RELI_REPORT     0x01
#define CMD_NRELI_REPORT    0x00


#define PRESSKEY_REG     	0x01
#define PASSWORD_REG     	0x00
#define PASSWORD_ERR     	0x02
//---------------------------------------------------------------------------------------
/*
 * frame error no
 */
#define NO_ERR        		0x00
#define OTHER_ERR     		0x0F
#define LEN_ERR       		0x01
#define BUFFER_ERR    		0x02
#define DATA_ERR      		0x03
#define DID_ERR       		0x04
#define DEV_BUSY      		0x05
#define NO_RETURN     		0x10
#define DATA_TRANS    		0x12

#define TEMP_SENSOR     0x03
#define HUMI_SENSOR     0x04
#define STEP_LEN        0x02
#define FREQ_LEN        0x02

/*
 * DID definitions
 */
enum
{
    DID_DEVTYPE     =   0x0001,
    DID_COMMVER     =   0x0002,
    DID_SOFTVER     =   0x0003,
    DID_DEVKEY      =   0x0005,
    DID_DEVSN       =   0x0007,
		DID_CHKPWD      =   0xC030,
		DID_REPORT    	=		0xD005,
	
		DID_SETTEMP    =    0xE002,/*设置温度 步长为1*/
		DID_SETRANGE   =		0xE005,
		DID_DEADBAND   =		0xE006,
		DID_LOWPROTECT =    0xE010,
		DID_WINSPEED   =		0xE011,
		DID_SETMODE    =    0xE012,
		DID_ONOFF    	 =		0xE013,
		DID_LOCK    	 =		0xE014,
	 	DID_WINONOFF   =		0xE018,
	  DID_RUN_STATUS =    0xE019,
		DID_TIME_CTRL  =    0xE020,
	
		DID_SENSORVALUE =   0xB701,
		DID_SENSORVALUE2 =   0xB691,
		
		DID_DEV_DATE    =  0xC010,
		DID_DEV_TIME    =  0xC011,
		DID_RERPORT_STATUS   =		0xC01A,
	
		DID_STEP    	  =		0xD103,
		DID_FREQ        =   0xD104,
		DID_REPORT_ALARM     =	0xD105,
	
		DID_DELAY       =   0xD702,
		
		
		DID_DEVSHOW     =		0x0009,
		DID_PWD		    	=		0x000A,
		DID_SILENT	    =		0x000B,
		
		DID_REBOOT    	=   0xFEFD,
		DID_RESTORE    	=   0xFF00,/*恢复出厂*/
		DID_DBG_REDUCTION	=   0xFF08,/*设置温度补偿*/
		DID_DBG_TEMP_RESET	=   0xFF0A,/*设置传感器复位*/
		
		DID_CURTAIN_CTL   = 0x0A01,
		DID_CURTAIN_R     = 0x0A04,
		DID_CURTAIN_RELAY_TIM = 0x0A09,
};

#pragma pack(1)
typedef struct SmartFrame
{
    uint8_t stc;
    uint8_t said[AID_LEN];
    uint8_t taid[AID_LEN];
    uint8_t seq;
    uint8_t len;
    uint8_t data[1];
} smart_frame_t;
#define SMART_FRAME_HEAD offset_of(smart_frame_t, data)

struct AppFrame
{
    uint8_t cmd;
    uint8_t data[0];
};
struct GroupFrame
{
    uint8_t len : 6;
    uint8_t type: 2;
    uint8_t data[1];
};

typedef struct Body
{
    uint8_t did[DID_LEN];
    uint8_t ctrl;
    uint8_t data[0];
} body_t;
#define FBD_FRAME_HEAD  offset_of(body_t, data)

struct RegData
{
    uint8_t aid[AID_LEN];
    uint8_t panid[SID_LEN];
    uint8_t pw[PW_LEN];
    uint8_t gid[AID_LEN];
    uint8_t sid[SID_LEN];
};

#pragma pack()

static inline int smart_frame_len(const smart_frame_t *frame)
{
    return SMART_FRAME_HEAD + frame->len + 1;
}
static inline bool smart_frame_is_broadcast(const smart_frame_t *frame)
{
    return is_all_xx(frame->taid, 0xff, AID_LEN);
}
static inline bool smart_frame_is_local(const smart_frame_t *pframe)
{
    return is_all_xx(pframe->said, 0x00, AID_LEN) && is_all_xx(pframe->taid, 0x00, AID_LEN);
}
static inline bool smart_frame_is_ack(const smart_frame_t *frame)
{
    return tst_bit(frame->seq, 7);
}
static inline int smart_frame_body_len(const body_t *body);

smart_frame_t *get_smart_frame(const uint8_t *in, uint32_t len);
int code_body(uint16_t did, int err, const void *data, int len, void *out, int maxlen);
int code_frame(const uint8_t *src, const uint8_t *dest, int seq, int cmd, \
               uint8_t *data, int len, void *out, int maxlen);
int code_local_frame(const uint8_t *in, int len, void *out, int maxlen);
int code_ret_frame(smart_frame_t *pframe, int len);
bool is_gid_equal(const uint8_t *data, uint16_t mysid);

#endif
