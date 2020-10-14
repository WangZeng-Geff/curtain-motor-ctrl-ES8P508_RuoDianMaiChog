#ifndef _W5500_H_
#define _W5500_H_

#define W5500_SOCK_NR   8

/***************** Common Register *****************/
#define W5500_MR      0x0000
#define W5500_RST     0x80
#define W5500_WOL     0x20
#define W5500_PB      0x10
#define W5500_PPP     0x08
#define W5500_FARP    0x02

#define W5500_GAR     0x0001
#define W5500_SUBR    0x0005
#define W5500_SHAR    0x0009
#define W5500_SIPR    0x000f

#define W5500_INTLEVEL    0x0013
#define W5500_IR      0x0015
#define W5500_CONFLICT    0x80
#define W5500_UNREACH     0x40
#define W5500_PPPOE       0x20
#define W5500_MP          0x10

#define W5500_IMR     0x0016
#define W5500_IM_IR7      0x80
#define W5500_IM_IR6      0x40
#define W5500_IM_IR5      0x20
#define W5500_IM_IR4      0x10

#define W5500_SIR     0x0017
#define W5500_S7_INT      0x80
#define W5500_S6_INT      0x40
#define W5500_S5_INT      0x20
#define W5500_S4_INT      0x10
#define W5500_S3_INT      0x08
#define W5500_S2_INT      0x04
#define W5500_S1_INT      0x02
#define W5500_S0_INT      0x01

#define W5500_SIMR    0x0018
#define W5500_S7_IMR      0x80
#define W5500_S6_IMR      0x40
#define W5500_S5_IMR      0x20
#define W5500_S4_IMR      0x10
#define W5500_S3_IMR      0x08
#define W5500_S2_IMR      0x04
#define W5500_S1_IMR      0x02
#define W5500_S0_IMR      0x01

#define W5500_RTR     0x0019
#define W5500_RCR     0x001b

#define W5500_PTIMER  0x001c
#define W5500_PMAGIC  0x001d
#define W5500_PHA     0x001e
#define W5500_PSID    0x0024
#define W5500_PMRU    0x0026

#define W5500_UIPR    0x0028
#define W5500_UPORT   0x002c

#define W5500_PHYCFGR 0x002e
#define W5500_RST_PHY     0x80
#define W5500_OPMODE      0x40
#define W5500_DPX         0x04
#define W5500_SPD         0x02
#define W5500_LINK        0x01

#define W5500_VERR    0x0039

/*******W5500_************** Socket Register *******************/
#define W5500_Sn_MR       0x0000
#define W5500_MULTI_MFEN      0x80
#define W5500_BCASTB          0x40
#define W5500_ND_MC_MMB       0x20
#define W5500_UCASTB_MIP6B    0x10
#define W5500_MR_CLOSE        0x00
#define W5500_MR_TCP      0x01
#define W5500_MR_UDP      0x02
#define W5500_MR_MACRAW       0x04

#define W5500_Sn_CR       0x0001
#define W5500_OPEN        0x01
#define W5500_LISTEN      0x02
#define W5500_CONNECT     0x04
#define W5500_DISCON      0x08
#define W5500_CLOSE       0x10
#define W5500_SEND        0x20
#define W5500_SEND_MAC    0x21
#define W5500_SEND_KEEP   0x22
#define W5500_RECV        0x40

#define W5500_Sn_IR       0x0002
#define W5500_IR_SEND_OK      0x10
#define W5500_IR_TIMEOUT      0x08
#define W5500_IR_RECV         0x04
#define W5500_IR_DISCON       0x02
#define W5500_IR_CON          0x01

#define W5500_Sn_SR       0x0003
#define W5500_SOCK_CLOSED     0x00
#define W5500_SOCK_INIT       0x13
#define W5500_SOCK_LISTEN     0x14
#define W5500_SOCK_ESTABLISHED    0x17
#define W5500_SOCK_CLOSE_WAIT     0x1c
#define W5500_SOCK_UDP        0x22
#define W5500_SOCK_MACRAW     0x02

#define W5500_SOCK_SYNSEND    0x15
#define W5500_SOCK_SYNRECV    0x16
#define W5500_SOCK_FIN_WAI    0x18
#define W5500_SOCK_CLOSING    0x1a
#define W5500_SOCK_TIME_WAIT  0x1b
#define W5500_SOCK_LAST_ACK   0x1d

#define W5500_Sn_PORT     0x0004
#define W5500_Sn_DHAR     0x0006
#define W5500_Sn_DIPR     0x000c
#define W5500_Sn_DPORTR   0x0010

#define W5500_Sn_MSSR     0x0012
#define W5500_Sn_TOS      0x0015
#define W5500_Sn_TTL      0x0016

#define W5500_Sn_RXBUF_SIZE   0x001e
#define W5500_Sn_TXBUF_SIZE   0x001f
#define W5500_Sn_TX_FSR   0x0020
#define W5500_Sn_TX_RD    0x0022
#define W5500_Sn_TX_WR    0x0024
#define W5500_Sn_RX_RSR   0x0026
#define W5500_Sn_RX_RD    0x0028
#define W5500_Sn_RX_WR    0x002a

#define W5500_Sn_IMR      0x002c
#define W5500_IMR_SENDOK  0x10
#define W5500_IMR_TIMEOUT 0x08
#define W5500_IMR_RECV    0x04
#define W5500_IMR_DISCON  0x02
#define W5500_IMR_CON     0x01

#define W5500_Sn_FRAG     0x002d
#define W5500_Sn_KPALVTR  0x002f

/*******************************************************************/
/************************ SPI Control Byte *************************/
/*******************************************************************/
/* Operation mode bits */
#define W5500_VDM     0x00
#define W5500_FDM1    0x01
#define W5500_FDM2    0x02
#define W5500_FDM4    0x03

/* Read_Write control bit */
#define W5500_RWB_READ    0x00
#define W5500_RWB_WRITE   0x04

/* Block select bits */
#define W5500_COMMON_R    0x00

/* Socket 0 */
#define W5500_S0_REG      0x08
#define W5500_S0_TX_BUF   0x10
#define W5500_S0_RX_BUF   0x18

/* Socket 1 */
#define W5500_S1_REG      0x28
#define W5500_S1_TX_BUF   0x30
#define W5500_S1_RX_BUF   0x38

/* Socket 2 */
#define W5500_S2_REG      0x48
#define W5500_S2_TX_BUF   0x50
#define W5500_S2_RX_BUF   0x58

/* Socket 3 */
#define W5500_S3_REG      0x68
#define W5500_S3_TX_BUF   0x70
#define W5500_S3_RX_BUF   0x78

/* Socket 4 */
#define W5500_S4_REG      0x88
#define W5500_S4_TX_BUF   0x90
#define W5500_S4_RX_BUF   0x98

/* Socket 5 */
#define W5500_S5_REG      0xa8
#define W5500_S5_TX_BUF   0xb0
#define W5500_S5_RX_BUF   0xb8

/* Socket 6 */
#define W5500_S6_REG      0xc8
#define W5500_S6_TX_BUF   0xd0
#define W5500_S6_RX_BUF   0xd8

/* Socket 7 */
#define W5500_S7_REG      0xe8
#define W5500_S7_TX_BUF   0xf0
#define W5500_S7_RX_BUF   0xf8

#define W5500_S_RX_SIZE   2048    /* rx max size */
#define W5500_S_TX_SIZE   2048    /* tx max size */
#if 0
#ifdef BIG_ENDIAN
/** Set an IP address given by the four byte-parts */
#define IP4_ADDR(ipaddr, a,b,c,d) \
        (ipaddr)->addr = ((u32)((a) & 0xff) << 24) | \
                         ((u32)((b) & 0xff) << 16) | \
                         ((u32)((c) & 0xff) << 8)  | \
                          (u32)((d) & 0xff)
#else
/** Set an IP address given by the four byte-parts.
    Little-endian version that prevents the use of lwip_htonl. */
#define IP4_ADDR(ipaddr, a,b,c,d) \
        (ipaddr)->addr = ((u32)((d) & 0xff) << 24) | \
                         ((u32)((c) & 0xff) << 16) | \
                         ((u32)((b) & 0xff) << 8)  | \
                          (u32)((a) & 0xff)
#endif
#endif

enum
{
    W5500EVT_IPCONFILCT = 1,
    W5500EVT_UNREACH,
    W5500EVT_LINKUP,
    W5500EVT_LINKDOWN,
    W5500EVT_CFGLOSSED,
    W5500EVT_CONN,
    W5500EVT_CLOSED,
    W5500EVT_SENT,
    W5500EVT_RECVED,
    W5500EVT_TIMEOUT,
};

typedef enum socket_proto
{
    SOCKET_TCP,
    SOCKET_UDP,
} socket_proto_t;

typedef struct socket_open_arg
{
    u8              sock;
    socket_proto_t  proto;
    u16             port;
} socket_open_arg_t;

typedef struct socket_conn_arg
{
    u8              sock;
    ip4_addr_t      dst;
    u16             port;
} socket_conn_arg_t;

typedef struct socket_read_arg
{
    u8              sock;
    void           *buf;
    size_t          len;
} socket_read_arg_t;

typedef struct socket_write_arg
{
    u8              sock;
    const void     *buf;
    size_t          len;
} socket_write_arg_t;

typedef struct socket_evt_arg
{
    u8              sock;
} socket_evt_arg_t;

enum
{
    DEV_CTRL_RESET,
    DEV_CTRL_NET_GET_CFG,
    DEV_CTRL_NET_SET_CFG,
    DEV_CTRL_NET_GET_PHYADDR,
    DEV_CTRL_NET_SET_PHYADDR,

    DEV_CTRL_SOCKET_OPEN,
    DEV_CTRL_SOCKET_CONN,
    DEV_CTRL_SOCKET_CLOSE,
    DEV_CTRL_SOCKET_READ,
    DEV_CTRL_SOCKET_WRITE,
    DEV_CTRL_GETEVT,
};

err_t w5500_init(const char *name, const char *spi_device_name);
#endif

