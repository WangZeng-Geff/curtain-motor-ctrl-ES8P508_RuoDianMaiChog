#include <printk.h>
#include <device.h>
#include <utils.h>
#include <syserr.h>
#include <board.h>
#include "device.h"
#include "HR8P296.h"
#include "lib_config.h"
#include <os.h>
#include <kfifo.h>

#define W5500_PR(...) printk(##__VA_ARGS__)

#ifdef configUSING_W5500

#define W5500_RESET_PORT    GPIOB
#define W5500_RESET_PIN     GPIO_Pin_18

#define SOCK_ANY_PORT_NUM  0xC000

#define CHECK_SOCK_NUM(sock) \
do { if (sock >= W5500_SOCK_NR) return -EBADF;} while (0)


struct w5500
{
    struct device       dev;
    struct spi_device  *spi;
    tcb_t               tcb;
    net_cfg_t           net_cfg;
    struct soft_timer   scan_tmr;

    u8                  linked   : 1,
                        cfged    : 1,
                        cfg_lost : 1;

    u8                  evt[W5500_SOCK_NR + 1];
};

static struct w5500 w5500;


#pragma pack(1)
struct w5500_hdr
{
    u16 reg;
    u8  ctl;
};
#pragma pack()

static err_t w5500_write_nbyte(u16 reg, const void *val, size_t n)
{
    struct w5500_hdr hdr;

    hdr.reg = cpu_to_be16(reg);
    hdr.ctl = W5500_VDM | W5500_RWB_WRITE | W5500_COMMON_R;

    return spi_send_then_send(w5500.spi, &hdr, sizeof(hdr), val, n);
}
static inline err_t w5500_write_1byte(u16 reg, u8 val)
{
    return w5500_write_nbyte(reg, &val, 1);
}
static inline err_t w5500_write_2byte(u16 reg, u16 val)
{
    val = cpu_to_be16(val);
    return w5500_write_nbyte(reg, &val, 2);
}


static err_t w5500_read_nbyte(u16 reg, void *val, size_t n)
{
    struct w5500_hdr hdr;

    hdr.reg = cpu_to_be16(reg);
    hdr.ctl = W5500_VDM | W5500_RWB_READ | W5500_COMMON_R;

    return spi_send_then_recv(w5500.spi, &hdr, sizeof(hdr), val, n);
}
static inline err_t w5500_read_1byte(u16 reg)
{
    u8 val;

    err_t err = w5500_read_nbyte(reg, &val, 1);
    if (err < 0) return err;

    return val;
}

static err_t w5500_write_sock_nbyte(int sock, u16 reg, const void *val, size_t n)
{
    struct w5500_hdr hdr;

    hdr.reg = cpu_to_be16(reg);
    hdr.ctl = W5500_VDM | W5500_RWB_WRITE | (sock * 0x20 + 0x08);

    return spi_send_then_send(w5500.spi, &hdr, sizeof(hdr), val, n);
}
static inline err_t w5500_write_sock_1byte(int sock, u16 reg, u8 val)
{
    return w5500_write_sock_nbyte(sock, reg, &val, 1);
}
static inline err_t w5500_write_sock_2byte(int sock, u16 reg, u16 val)
{
    val = cpu_to_be16(val);

    return w5500_write_sock_nbyte(sock, reg, &val, 2);
}
static err_t w5500_write_sock_data(int sock, u16 offset, const void *val, size_t n)
{
    struct w5500_hdr hdr;

    hdr.reg = cpu_to_be16(offset);
    hdr.ctl = W5500_VDM | W5500_RWB_WRITE | (sock * 0x20 + 0x10);

    return spi_send_then_send(w5500.spi, &hdr, sizeof(hdr), val, n);
}

static err_t w5500_read_sock_nbyte(int sock, u16 reg, void *val, size_t n)
{
    struct w5500_hdr hdr;

    hdr.reg = cpu_to_be16(reg);
    hdr.ctl = W5500_VDM | W5500_RWB_READ | (sock * 0x20 + 0x08);

    return spi_send_then_recv(w5500.spi, &hdr, sizeof(hdr), val, n);
}
static inline err_t w5500_read_sock_1byte(int sock, u16 reg)
{
    u8 val;

    err_t err = w5500_read_sock_nbyte(sock, reg, &val, 1);
    if (err < 0) return err;

    return val;
}
static inline err_t w5500_read_sock_2byte(int sock, u16 reg)
{
    u16 val;

    err_t err = w5500_read_sock_nbyte(sock, reg, &val, 2);
    if (err < 0) return err;

    return be16_to_cpu(val);
}
static err_t w5500_read_sock_data(int sock, u16 offset, void *val, size_t n)
{
    struct w5500_hdr hdr;

    hdr.reg = cpu_to_be16(offset);
    hdr.ctl = W5500_VDM | W5500_RWB_READ | (sock * 0x20 + 0x18);

    return spi_send_then_recv(w5500.spi, &hdr, sizeof(hdr), val, n);
}

static void w5500_status_reset(void)
{
    w5500.linked    = 0;
    w5500.cfged     = 0;
    w5500.cfg_lost  = 0;

    memset(w5500.evt, 0, sizeof(w5500.evt));
}

static err_t w5500_hard_reset(void)
{
    err_t err;

    printk("W5500 reset...");
    GPIO_set_bit(W5500_RESET_PORT, W5500_RESET_PIN, 0);
    board_mdelay(10);
    GPIO_set_bit(W5500_RESET_PORT, W5500_RESET_PIN, 1);
    board_mdelay(1000);
    printk("done\r\n");

    //w5500_write_1byte(W5500_MR, W5500_RST);
    //board_mdelay(10);

    err = w5500_read_1byte(W5500_VERR);
    if (err < 0) return err;

    if (err != 4) {
        log_i(MODULE_APP, "w5500 is not detected!\r\n");
        return -ENXIO;
    }

    log_i(MODULE_APP, "w5500 ver: %02x\r\n", (u8)err);

    return 0;
}

static err_t w5500_get_phy_addr(phy_addr_t *phy_addr)
{
    w5500_read_nbyte(W5500_SHAR, phy_addr, sizeof(phy_addr_t));
    return 0;
}
static err_t w5500_set_phy_addr(const phy_addr_t *phy_addr)
{
    w5500_write_nbyte(W5500_SHAR, phy_addr, sizeof(phy_addr_t));

#if 1
    phy_addr_t tmp;
    w5500_get_phy_addr(&tmp);
    if (memcmp(&tmp, phy_addr, sizeof(phy_addr_t)))
        return -EINVAL;
#endif
    return 0;
}

static err_t w5500_get_cfg(net_cfg_t *cfg)
{
    w5500_read_nbyte(W5500_GAR,  &cfg->gateway, sizeof(cfg->gateway));
    w5500_read_nbyte(W5500_SUBR, &cfg->submask, sizeof(cfg->submask));
    w5500_read_nbyte(W5500_SIPR, &cfg->ip,      sizeof(cfg->ip));
    return 0;
}
static err_t w5500_chk_cfg(const net_cfg_t *cfg)
{
    net_cfg_t tmp;
    w5500_get_cfg(&tmp);
    if (memcmp(&tmp, cfg, sizeof(net_cfg_t)))
        return -EINVAL;
    return 0;
}
static err_t w5500_set_cfg(const net_cfg_t *cfg)
{
    int i;

    w5500_write_nbyte(W5500_GAR,  &cfg->gateway, sizeof(cfg->gateway));
    w5500_write_nbyte(W5500_SUBR, &cfg->submask, sizeof(cfg->submask));
    w5500_write_nbyte(W5500_SIPR, &cfg->ip,      sizeof(cfg->ip));

    for (i = 0; i < 8; i++)
    {
        w5500_write_sock_1byte(i, W5500_Sn_RXBUF_SIZE, 0x02);
        w5500_write_sock_1byte(i, W5500_Sn_TXBUF_SIZE, 0x02);
    }

    /* retry time and count */
    /* ARP TO: 2000*0.1ms*(8+1) = 1.8s */
    w5500_write_2byte(W5500_RTR, 2000);
    w5500_write_1byte(W5500_RCR, 8);

    w5500_write_1byte(W5500_IMR,  W5500_IM_IR7 | W5500_IM_IR6);
    w5500_write_1byte(W5500_SIMR, W5500_S0_IMR);

    w5500_write_sock_1byte(0, W5500_Sn_IMR, W5500_IMR_SENDOK | W5500_IMR_TIMEOUT | W5500_IMR_RECV | W5500_IMR_DISCON | W5500_IMR_CON);
    return w5500_chk_cfg(cfg);
}

static u8 w5500_is_linked(void) __attribute__((unused));

static u8 w5500_is_linked(void)
{
    u8 val;

    val = w5500_read_1byte(W5500_PHYCFGR);
    return !!(val & W5500_LINK);
}

static err_t close(int sock);

static err_t socket(int sock, socket_proto_t proto, u16 port)
{
    err_t err;
    ip4_addr_t ip;

    CHECK_SOCK_NUM(sock);

    close(sock);

    switch (proto)
    {
    case SOCKET_TCP:
        w5500_read_nbyte(W5500_SIPR, &ip, sizeof(ip));
        if (ip.addr == 0) return -EPERM;
        w5500_write_sock_1byte(sock, W5500_Sn_MR, W5500_MR_TCP);
        break;
    case SOCKET_UDP:
        w5500_write_sock_1byte(sock, W5500_Sn_MR, W5500_MR_UDP);
        break;
    }

    if (port != 0)
    {
        err = w5500_write_sock_2byte(sock, W5500_Sn_PORT, port);
        if (err < 0) return err;
    }

    err = w5500_write_sock_1byte(sock, W5500_Sn_CR, W5500_OPEN);
    if (err < 0) return err;

    board_mdelay(5);

    err = w5500_read_sock_1byte(sock, W5500_Sn_SR);
    if (err < 0 || err == W5500_SOCK_CLOSED) return -EPERM;

    if (proto == SOCKET_TCP && err != W5500_SOCK_INIT)
        return -EPERM;

    return sock;
}

static err_t close(int sock)
{
    err_t err;
    CHECK_SOCK_NUM(sock);

    err = w5500_write_sock_1byte(sock, W5500_Sn_CR, W5500_CLOSE);
    if (err < 0) return err;

    do
    {
        err = w5500_read_sock_1byte(sock, W5500_Sn_CR);
        if (err < 0) return -EPERM;
    }
    while (err);

    err = w5500_write_sock_1byte(sock, W5500_Sn_IR, 0xFF);
    if (err < 0) return err;

    do
    {
        err = w5500_read_sock_1byte(sock, W5500_Sn_SR);
        if (err < 0) return -EPERM;
    }
    while (err != W5500_SOCK_CLOSED);

    return 0;
}

static err_t connect(int sock, const ip4_addr_t *dst, u16 port)
{
    err_t err;

    CHECK_SOCK_NUM(sock);

    //if (!w5500_is_linked()) return -EAGAIN;

    err = w5500_read_sock_1byte(sock, W5500_Sn_MR);
    if (err < 0 || err != W5500_MR_TCP) return -EPERM;

    err = w5500_read_sock_1byte(sock, W5500_Sn_SR);
    if (err < 0 || err != W5500_SOCK_INIT) return -EPERM;

    /* dest ip & port */
    if (port != 0)
    {
        err = w5500_write_sock_2byte(sock, W5500_Sn_DPORTR, port);
        if (err < 0) return err;
    }
    err = w5500_write_sock_nbyte(sock, W5500_Sn_DIPR,   dst, sizeof(ip4_addr_t));
    if (err < 0) return err;

    /* connect to dest ip */
    err = w5500_write_sock_1byte(sock, W5500_Sn_CR, W5500_CONNECT);
    if (err < 0) return err;

    return 0;
}
#if 0
static err_t listen(int sock)
{
    err_t err;

    CHECK_SOCK_NUM(sock);

    err = w5500_read_sock_1byte(sock, W5500_Sn_MR);
    if (err < 0 || err != W5500_MR_TCP) return -EPERM;

    err = w5500_read_sock_1byte(sock, W5500_Sn_SR);
    if (err < 0 || err != W5500_SOCK_INIT) return -EPERM;

    err = w5500_write_sock_1byte(sock, W5500_Sn_CR, W5500_LISTEN);
    if (err < 0) return err;

    return 0;
}
static err_t w5500_wait_for_connect(int sock)
{
    u8 tmp;

    CHECK_SOCK_NUM(sock);

    do
    {
        tmp = w5500_read_sock_1byte(sock, W5500_Sn_SR);
        if (tmp == W5500_SOCK_ESTABLISHED)
            return 0;

        if (tmp == W5500_SOCK_CLOSED)
            return -EINVAL;

        tmp = w5500_read_sock_1byte(sock, W5500_Sn_IR);
        if (tmp & W5500_IR_TIMEOUT)
        {
            w5500_write_sock_1byte(sock, W5500_Sn_IR, W5500_IR_TIMEOUT);
            return -ETIMEDOUT;
        }

        board_mdelay(5);
    }
    while (1);
}
#endif
static err_t send(int sock, const void *buf, size_t len)
{
    err_t err;
    u16 tmp = 0;

    CHECK_SOCK_NUM(sock);

    if (len == 0 || len > W5500_S_TX_SIZE) return -EINVAL;

    err = w5500_read_sock_1byte(sock, W5500_Sn_MR);
    if (err < 0 || err != W5500_MR_TCP) return -EPERM;

    err = w5500_read_sock_1byte(sock, W5500_Sn_SR);
    if (err < 0 || (err != W5500_SOCK_ESTABLISHED && err != W5500_SOCK_CLOSE_WAIT)) return -EPERM;

    /* check free size */
    err = w5500_read_sock_2byte(sock, W5500_Sn_TX_FSR);
    if (err < 0 || len > err) return -EPERM;

    /* send data to w5500 */
    err = w5500_read_sock_2byte(sock, W5500_Sn_TX_WR);
    if (err < 0) return err;

    tmp = err;

    err = w5500_write_sock_data(sock, tmp, buf, len);
    if (err < 0) return err;
    tmp += len;
    err = w5500_write_sock_2byte(sock, W5500_Sn_TX_WR, tmp);
    if (err < 0) return err;

    err = w5500_write_sock_1byte(sock, W5500_Sn_CR, W5500_SEND);
    if (err < 0) return err;

    return len;
}

static err_t recv(int sock, void *buf, size_t len)
{
    err_t err;
    u16 tmp = 0;

    CHECK_SOCK_NUM(sock);

    err = w5500_read_sock_1byte(sock, W5500_Sn_MR);
    if (err < 0 || err != W5500_MR_TCP) return -EPERM;

    err = w5500_read_sock_2byte(sock, W5500_Sn_RX_RSR);
    if (err < 0) return -EPERM;

    if (err == 0) return 0;

    if (err > 1460) err = 1460;
    if (len > err) len = err;

    err = w5500_read_sock_2byte(sock, W5500_Sn_RX_RD);
    if (err < 0) return -EPERM;

    tmp = err;

    err = w5500_read_sock_data(sock, tmp, buf, len);
    if (err < 0) return -EPERM;

    tmp += len;

    err = w5500_write_sock_2byte(sock, W5500_Sn_RX_RD, tmp);
    if (err < 0) return err;

    err = w5500_write_sock_1byte(sock, W5500_Sn_CR, W5500_RECV);
    if (err < 0) return err;

    return len;
}

static err_t disconnect(int sock)
{
    u8 mr;
    CHECK_SOCK_NUM(sock);

    mr = w5500_read_sock_1byte(sock, W5500_Sn_MR);
    if (mr != W5500_MR_TCP) return -EPERM;

    w5500_read_sock_1byte(sock, W5500_DISCON);

    return 0;
}

#if 0
static err_t w5500_gateway_probe(void)
{
    err_t err;

    err = socket(0, SOCKET_TCP, 0);
    if (err < 0) goto out;

    err = connect(0, &w5500.cfg.gateway, 0);
    if (err < 0) goto out;

    do
    {
        err = w5500_read_sock_1byte(0, W5500_Sn_IR);
        if (err < 0) goto out;

        if (err != 0)
            w5500_write_sock_1byte(0, W5500_Sn_IR, (u8)err);
        board_mdelay(5);
        if (err & W5500_IR_TIMEOUT)
        {
            err = -EAGAIN;
            goto out;
        }

        err = w5500_read_sock_1byte(0, W5500_Sn_DHAR);
        if (err < 0) goto out;

        if (err != 0xFF)
        {
            err = 0;
            goto out;
        }
    }
    while (1);

out:
    close(0);
    return err;
}
#endif


static err_t w5500_ctrl(device_t *dev, uint8_t cmd, void *args)
{
    u8 sock;

    switch (cmd)
    {
    case DEV_CTRL_RESET:
        w5500_hard_reset();
        w5500_status_reset();
        return 0;
    case DEV_CTRL_NET_GET_CFG:
        //return w5500_get_cfg((net_cfg_t *)args);
        memcpy(args, &w5500.net_cfg, sizeof(net_cfg_t));
        return 0;
    case DEV_CTRL_NET_SET_CFG:
        memcpy(&w5500.net_cfg, args, sizeof(net_cfg_t));
        w5500.cfged = 1;
        return w5500_set_cfg((net_cfg_t *)args);

    case DEV_CTRL_NET_GET_PHYADDR:
        return w5500_get_phy_addr((phy_addr_t *)args);
    case DEV_CTRL_NET_SET_PHYADDR:
        return w5500_set_phy_addr((phy_addr_t *)args);

    case DEV_CTRL_SOCKET_OPEN:
    {
        socket_open_arg_t *arg = (socket_open_arg_t *)args;
        return socket(arg->sock, arg->proto, arg->port);
    }
    case DEV_CTRL_SOCKET_CONN:
    {
        socket_conn_arg_t *arg = (socket_conn_arg_t *)args;
        return connect(arg->sock, &arg->dst, arg->port);
    }
    case DEV_CTRL_SOCKET_CLOSE:
        sock = *(int *)args;
        if (sock < W5500_SOCK_NR)
        {
            disconnect(sock);
            close(sock);
        }
        break;
    case DEV_CTRL_SOCKET_READ:
    {
        err_t ret;
        socket_read_arg_t *arg = (socket_read_arg_t *)args;
        ret = recv(arg->sock, arg->buf, arg->len);
        if (ret > 0) 
        {
            log_i(MODULE_UART, "w5500 read [%d] bytes from sock[%d]:\r\n", ret, arg->sock);
            log_arr_i(MODULE_UART, arg->buf, ret);
        }
        return ret;
    }
    case DEV_CTRL_SOCKET_WRITE:
    {
        socket_write_arg_t *arg = (socket_write_arg_t *)args;
        log_i(MODULE_UART, "w5500 write [%d] bytes to sock[%d]:\r\n", arg->len, arg->sock);
        log_arr_i(MODULE_UART, arg->buf, arg->len);
        return send(arg->sock, arg->buf, arg->len);
    }
    case DEV_CTRL_GETEVT:
        sock = *(u8 *)args;
        if (sock <= W5500_SOCK_NR)
        {
            *(u8 *)args = w5500.evt[sock];
            w5500.evt[sock] = 0;
        }
        return 0;
    default:
        return -EINVAL;
    }
    return 0;
}
static const struct device_ops w5500_ops =
{
    .ctrl = w5500_ctrl,
};
static int w5500_irq_service(void)
{
    u8 ir;
    u8 evt = 0;

    ir = w5500_read_1byte(W5500_IR);
    w5500_write_1byte(W5500_IR, ir & 0xF0);

    if (ir & W5500_CONFLICT) evt = W5500EVT_IPCONFILCT;
    if (ir & W5500_UNREACH)  evt = W5500EVT_UNREACH;

    if (evt != 0)
    {
        w5500.evt[W5500_SOCK_NR] = evt;
        return 1;
    }
    return 0;
}
static int w5500_sock_irq_service(int sock)
{
    u8 sir;
    u8 evt = 0;

    sir = w5500_read_sock_1byte(sock, W5500_Sn_IR);
    w5500_write_sock_1byte(sock, W5500_Sn_IR, sir);

    if (sir & W5500_IR_CON)     evt = W5500EVT_CONN;
    if (sir & W5500_IR_DISCON)  evt = W5500EVT_CLOSED;
    if (sir & W5500_IR_SEND_OK) evt = W5500EVT_SENT;
    if (sir & W5500_IR_RECV)    evt = W5500EVT_RECVED;
    if (sir & W5500_IR_TIMEOUT) evt = W5500EVT_TIMEOUT;

    if (evt != 0)
    {
        w5500.evt[sock] = evt;
        return 1;
    }
    return 0;
}

static void w5500_thread_cb(struct task_ctrl_blk *tcb, ubase_t data)
{
    int i;
    bool evt_got;
    sig_t sig;

    tSTART(tcb);
    for (;;)
    {
        task_wait_signal(tcb);

        evt_got = false;
        sig = task_signal(tcb);

        if (sigget(sig, SIG_DATA))
        {
            if (w5500_irq_service() > 0)
                evt_got = true;

            u8 ir = w5500_read_1byte(W5500_SIR);
            for (i = 0; i < W5500_SOCK_NR; i++)
                if (tst_bit(ir, i) && w5500_sock_irq_service(i) > 0)
                    evt_got = true;
        }

        if (sigget(sig, SIG_ALARM)) 
        {
            u8 linked = w5500_is_linked();
            if (w5500.linked != linked) {
                w5500.evt[W5500_SOCK_NR] = linked ? W5500EVT_LINKUP : W5500EVT_LINKDOWN;
                evt_got = true;

                w5500.linked = linked;
            }

            u8 lost = w5500_chk_cfg(&w5500.net_cfg) < 0;
            if (w5500.cfged && w5500.cfg_lost != lost) {
                if (lost) {
                    w5500.evt[W5500_SOCK_NR] = W5500EVT_CFGLOSSED;
                    evt_got = true;
                }

                w5500.cfg_lost = lost;
            }
        }

        if (evt_got && w5500.dev.owner)
            task_send_signal(w5500.dev.owner, SIG_DATA);
    }

    tEND();
}

static void scan_tmr_cb(struct soft_timer *st)
{
    task_send_signal(&w5500.tcb, SIG_ALARM);

    st->expires += configHZ * 2;
    soft_timer_add(st);
}

static void w5500_irq_init(void)
{
    GPIO_InitStruType x;

    GPIO_RegUnLock();
    x.GPIO_Func = GPIO_Func_0;
    x.GPIO_Direction = GPIO_Dir_In;
    x.GPIO_PUEN = ENABLE;
    x.GPIO_PDEN = DISABLE;
    x.GPIO_OD = DISABLE;
    GPIO_Init(GPIOB, GPIO_Pin_19, &x);
    GPIO_RegLock();

    PINT_Config(PINT3, PINT_SEL6, PINT_Trig_Fall);
    NVIC_Init(NVIC_PINT3_IRQn, NVIC_Priority_0, ENABLE);
    PINT3_MaskDisable();
    PINT3_Enable();
}

static void w5500_gpio_init(void)
{
    GPIO_InitStruType x;

    GPIO_RegUnLock();
    x.GPIO_Func = GPIO_Func_0;
    x.GPIO_Direction = GPIO_Dir_Out;
    x.GPIO_PUEN = DISABLE;
    x.GPIO_PDEN = DISABLE;
    x.GPIO_OD   = DISABLE;
    GPIO_set_bit(W5500_RESET_PORT, W5500_RESET_PIN, 1);
    GPIO_Init(W5500_RESET_PORT, W5500_RESET_PIN, &x);
    GPIO_RegLock();
}

void PINT3_IRQHandler(void)
{
    if (PINT_GetITStatus(PINT_IT_PINT3) != RESET)
    {
        PINT_ClearITPendingBit(PINT_IT_PINT3);
        task_send_signal(&w5500.tcb, SIG_DATA);
    }
}

err_t w5500_init(const char *name, const char *spi_device_name)
{
    err_t err;
    struct spi_device *spi;

    spi = (struct spi_device *)device_find(spi_device_name);
    if (!spi) return -ENOSYS;

    w5500.spi = spi;

    w5500_gpio_init();
    w5500_irq_init();

    /* config spi */
    {
        struct spi_cfg cfg;
        cfg.data_width = 8;
        cfg.mode = SPI_MODE_0 | SPI_MSB; /* SPI Compatible: Mode 0 and Mode 3 */
        cfg.max_hz = 16 * 1000 * 1000;   /* 16M */
        spi_configure(spi, &cfg);
    }

    err = w5500_hard_reset();
    if (err < 0) return err;

    w5500_status_reset();

    /* soft timer */
    struct soft_timer *st = &w5500.scan_tmr;
    st->cb = scan_tmr_cb;
    st->expires = INITIAL_JIFFIES;
    soft_timer_add(st);

    task_create(&w5500.tcb, w5500_thread_cb, 0);
    w5500.dev.ops = &w5500_ops;
    device_register(&w5500.dev, name, 0);

    //w5500_gateway_probe();

    return 0;
}
#endif

